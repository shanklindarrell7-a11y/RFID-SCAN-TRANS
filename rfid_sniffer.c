/**
 * RFID Sniffer Implementation
 * Optimized for high performance and low latency
 */

#include "rfid_sniffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// SQLite for efficient database management
#include <sqlite3.h>

// Helper function for safe data copying with bounds checking
static inline size_t safe_data_copy(uint8_t* dest, size_t dest_size, 
                                     const void* src, size_t src_size) {
    size_t copy_size = (src_size < dest_size) ? src_size : dest_size;
    if (copy_size > 0) {
        memcpy(dest, src, copy_size);
    }
    return copy_size;
}

// Global state
static RFIDRingBuffer* g_capture_buffer = NULL;
static pthread_t g_capture_thread;
static volatile bool g_capture_running = false;
static RFIDPerformanceMetrics g_metrics = {0};
static pthread_mutex_t g_metrics_mutex = PTHREAD_MUTEX_INITIALIZER;

// Timer utility for performance measurement
static inline uint64_t get_time_microseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// ============================================================================
// Ring Buffer Implementation (Lock-free for single producer/consumer)
// ============================================================================

RFIDRingBuffer* rfid_ringbuffer_create(uint32_t capacity) {
    RFIDRingBuffer* rb = (RFIDRingBuffer*)malloc(sizeof(RFIDRingBuffer));
    if (!rb) return NULL;
    
    rb->buffer = (RFIDSignal*)calloc(capacity, sizeof(RFIDSignal));
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }
    
    rb->head = 0;
    rb->tail = 0;
    rb->capacity = capacity;
    
    return rb;
}

void rfid_ringbuffer_destroy(RFIDRingBuffer* rb) {
    if (rb) {
        free(rb->buffer);
        free(rb);
    }
}

bool rfid_ringbuffer_is_empty(RFIDRingBuffer* rb) {
    return rb->head == rb->tail;
}

bool rfid_ringbuffer_is_full(RFIDRingBuffer* rb) {
    return ((rb->head + 1) % rb->capacity) == rb->tail;
}

bool rfid_ringbuffer_push(RFIDRingBuffer* rb, const RFIDSignal* signal) {
    if (rfid_ringbuffer_is_full(rb)) {
        return false;
    }
    
    memcpy(&rb->buffer[rb->head], signal, sizeof(RFIDSignal));
    rb->head = (rb->head + 1) % rb->capacity;
    
    return true;
}

bool rfid_ringbuffer_pop(RFIDRingBuffer* rb, RFIDSignal* signal) {
    if (rfid_ringbuffer_is_empty(rb)) {
        return false;
    }
    
    memcpy(signal, &rb->buffer[rb->tail], sizeof(RFIDSignal));
    rb->tail = (rb->tail + 1) % rb->capacity;
    
    return true;
}

// ============================================================================
// Database Operations with LRU Cache
// ============================================================================

RFIDDatabase* rfid_db_open(const char* db_path) {
    RFIDDatabase* db = (RFIDDatabase*)calloc(1, sizeof(RFIDDatabase));
    if (!db) return NULL;
    
    sqlite3* sqlite_db = NULL;
    int rc = sqlite3_open(db_path, &sqlite_db);
    
    if (rc != SQLITE_OK) {
        free(db);
        return NULL;
    }
    
    db->db_handle = sqlite_db;
    db->cache_size = 0;
    db->cache_hits = 0;
    db->cache_misses = 0;
    db->index_enabled = false;
    
    // Create optimized table schema with indexes
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS signals ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "timestamp INTEGER NOT NULL, "
        "type INTEGER NOT NULL, "
        "frequency INTEGER NOT NULL, "
        "data_length INTEGER NOT NULL, "
        "data BLOB NOT NULL, "
        "checksum INTEGER NOT NULL, "
        "tag TEXT"
        ");";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(sqlite_db, create_table_sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(sqlite_db);
        free(db);
        return NULL;
    }
    
    // Create indexes for common queries
    const char* create_indexes_sql = 
        "CREATE INDEX IF NOT EXISTS idx_timestamp ON signals(timestamp);"
        "CREATE INDEX IF NOT EXISTS idx_type ON signals(type);"
        "CREATE INDEX IF NOT EXISTS idx_frequency ON signals(frequency);"
        "CREATE INDEX IF NOT EXISTS idx_tag ON signals(tag);";
    
    sqlite3_exec(sqlite_db, create_indexes_sql, NULL, NULL, NULL);
    db->index_enabled = true;
    
    // Enable WAL mode for better concurrency
    sqlite3_exec(sqlite_db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    
    // Optimize for performance
    sqlite3_exec(sqlite_db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);
    sqlite3_exec(sqlite_db, "PRAGMA cache_size=10000;", NULL, NULL, NULL);
    sqlite3_exec(sqlite_db, "PRAGMA temp_store=MEMORY;", NULL, NULL, NULL);
    
    return db;
}

void rfid_db_close(RFIDDatabase* db) {
    if (!db) return;
    
    rfid_cache_clear(db);
    
    if (db->db_handle) {
        sqlite3_close((sqlite3*)db->db_handle);
    }
    
    free(db);
}

// LRU Cache implementation
void rfid_cache_put(RFIDDatabase* db, const RFIDSignal* signal) {
    if (!db || !signal) return;
    
    // Check if signal already in cache
    CacheNode* node = db->cache_head;
    while (node) {
        if (node->signal_id == signal->id) {
            // Move to front
            if (node != db->cache_head) {
                if (node->prev) node->prev->next = node->next;
                if (node->next) node->next->prev = node->prev;
                if (node == db->cache_tail) db->cache_tail = node->prev;
                
                node->next = db->cache_head;
                node->prev = NULL;
                db->cache_head->prev = node;
                db->cache_head = node;
            }
            memcpy(&node->signal, signal, sizeof(RFIDSignal));
            node->access_count++;
            return;
        }
        node = node->next;
    }
    
    // Add new node
    CacheNode* new_node = (CacheNode*)malloc(sizeof(CacheNode));
    if (!new_node) return;
    
    new_node->signal_id = signal->id;
    memcpy(&new_node->signal, signal, sizeof(RFIDSignal));
    new_node->access_count = 1;
    new_node->prev = NULL;
    new_node->next = db->cache_head;
    
    if (db->cache_head) {
        db->cache_head->prev = new_node;
    }
    db->cache_head = new_node;
    
    if (!db->cache_tail) {
        db->cache_tail = new_node;
    }
    
    db->cache_size++;
    
    // Evict least recently used if cache is full
    if (db->cache_size > RFID_DB_CACHE_SIZE) {
        CacheNode* lru = db->cache_tail;
        db->cache_tail = lru->prev;
        if (db->cache_tail) {
            db->cache_tail->next = NULL;
        }
        free(lru);
        db->cache_size--;
    }
}

RFIDSignal* rfid_cache_get(RFIDDatabase* db, uint32_t signal_id) {
    if (!db) return NULL;
    
    CacheNode* node = db->cache_head;
    while (node) {
        if (node->signal_id == signal_id) {
            db->cache_hits++;
            node->access_count++;
            
            // Move to front (LRU update)
            if (node != db->cache_head) {
                if (node->prev) node->prev->next = node->next;
                if (node->next) node->next->prev = node->prev;
                if (node == db->cache_tail) db->cache_tail = node->prev;
                
                node->next = db->cache_head;
                node->prev = NULL;
                db->cache_head->prev = node;
                db->cache_head = node;
            }
            
            return &node->signal;
        }
        node = node->next;
    }
    
    db->cache_misses++;
    return NULL;
}

void rfid_cache_clear(RFIDDatabase* db) {
    if (!db) return;
    
    CacheNode* node = db->cache_head;
    while (node) {
        CacheNode* next = node->next;
        free(node);
        node = next;
    }
    
    db->cache_head = NULL;
    db->cache_tail = NULL;
    db->cache_size = 0;
}

int rfid_db_store_signal(RFIDDatabase* db, const RFIDSignal* signal) {
    if (!db || !signal) return -1;
    
    uint64_t start_time = get_time_microseconds();
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    sqlite3_stmt* stmt = NULL;
    
    const char* insert_sql = 
        "INSERT INTO signals (timestamp, type, frequency, data_length, data, checksum, tag) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    
    int rc = sqlite3_prepare_v2(sqlite_db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_int64(stmt, 1, signal->timestamp);
    sqlite3_bind_int(stmt, 2, signal->type);
    sqlite3_bind_int(stmt, 3, signal->frequency);
    sqlite3_bind_int(stmt, 4, signal->data_length);
    sqlite3_bind_blob(stmt, 5, signal->data, signal->data_length, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, signal->checksum);
    sqlite3_bind_text(stmt, 7, signal->tag, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return -1;
    }
    
    // Add to cache
    RFIDSignal cached_signal = *signal;
    cached_signal.id = (uint32_t)sqlite3_last_insert_rowid(sqlite_db);
    rfid_cache_put(db, &cached_signal);
    
    uint64_t end_time = get_time_microseconds();
    
    pthread_mutex_lock(&g_metrics_mutex);
    g_metrics.signals_stored++;
    g_metrics.avg_storage_latency_us = 
        (g_metrics.avg_storage_latency_us + (end_time - start_time)) / 2;
    pthread_mutex_unlock(&g_metrics_mutex);
    
    return 0;
}

int rfid_db_store_signals_batch(RFIDDatabase* db, const RFIDSignal* signals, uint32_t count) {
    if (!db || !signals || count == 0) return -1;
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    
    // Begin transaction for batch insert
    sqlite3_exec(sqlite_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    
    sqlite3_stmt* stmt = NULL;
    const char* insert_sql = 
        "INSERT INTO signals (timestamp, type, frequency, data_length, data, checksum, tag) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)";
    
    int rc = sqlite3_prepare_v2(sqlite_db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_exec(sqlite_db, "ROLLBACK", NULL, NULL, NULL);
        return -1;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        sqlite3_bind_int64(stmt, 1, signals[i].timestamp);
        sqlite3_bind_int(stmt, 2, signals[i].type);
        sqlite3_bind_int(stmt, 3, signals[i].frequency);
        sqlite3_bind_int(stmt, 4, signals[i].data_length);
        sqlite3_bind_blob(stmt, 5, signals[i].data, signals[i].data_length, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, signals[i].checksum);
        sqlite3_bind_text(stmt, 7, signals[i].tag, -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_exec(sqlite_db, "ROLLBACK", NULL, NULL, NULL);
            return -1;
        }
        
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_exec(sqlite_db, "COMMIT", NULL, NULL, NULL);
    
    return 0;
}

RFIDSignal* rfid_db_get_signal(RFIDDatabase* db, uint32_t signal_id) {
    if (!db) return NULL;
    
    // Check cache first
    RFIDSignal* cached = rfid_cache_get(db, signal_id);
    if (cached) {
        RFIDSignal* result = (RFIDSignal*)malloc(sizeof(RFIDSignal));
        if (result) {
            memcpy(result, cached, sizeof(RFIDSignal));
        }
        return result;
    }
    
    // Query database
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    sqlite3_stmt* stmt = NULL;
    
    const char* select_sql = 
        "SELECT id, timestamp, type, frequency, data_length, data, checksum, tag "
        "FROM signals WHERE id = ?";
    
    int rc = sqlite3_prepare_v2(sqlite_db, select_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }
    
    sqlite3_bind_int(stmt, 1, signal_id);
    
    RFIDSignal* signal = NULL;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        signal = (RFIDSignal*)malloc(sizeof(RFIDSignal));
        if (signal) {
            signal->id = sqlite3_column_int(stmt, 0);
            signal->timestamp = sqlite3_column_int64(stmt, 1);
            signal->type = sqlite3_column_int(stmt, 2);
            signal->frequency = sqlite3_column_int(stmt, 3);
            signal->data_length = sqlite3_column_int(stmt, 4);
            
            const void* data = sqlite3_column_blob(stmt, 5);
            int data_size = sqlite3_column_bytes(stmt, 5);
            safe_data_copy(signal->data, RFID_SIGNAL_MAX_LENGTH, data, data_size);
            
            signal->checksum = sqlite3_column_int(stmt, 6);
            
            const char* tag = (const char*)sqlite3_column_text(stmt, 7);
            if (tag) {
                strncpy(signal->tag, tag, sizeof(signal->tag) - 1);
                signal->tag[sizeof(signal->tag) - 1] = '\0';
            }
            
            // Add to cache
            rfid_cache_put(db, signal);
        }
    }
    
    sqlite3_finalize(stmt);
    return signal;
}

int rfid_db_get_signals_filtered(RFIDDatabase* db, const RFIDFilter* filter, 
                                  RFIDSignal** results, uint32_t* count, uint32_t max_results) {
    if (!db || !results || !count) return -1;
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    sqlite3_stmt* stmt = NULL;
    
    // Build dynamic query based on filter using parameterized queries
    char query[2048] = "SELECT id, timestamp, type, frequency, data_length, data, checksum, tag FROM signals WHERE 1=1";
    int param_index = 1;
    
    if (filter) {
        if (filter->use_type_filter) {
            strcat(query, " AND type = ?");
        }
        if (filter->use_freq_filter) {
            strcat(query, " AND frequency BETWEEN ? AND ?");
        }
        if (filter->use_time_filter) {
            strcat(query, " AND timestamp BETWEEN ? AND ?");
        }
        if (filter->use_tag_filter && strlen(filter->tag_filter) > 0) {
            strcat(query, " AND tag LIKE ?");
        }
    }
    
    // Add LIMIT using parameter binding
    strcat(query, " LIMIT ?");
    
    int rc = sqlite3_prepare_v2(sqlite_db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    // Bind parameters safely
    param_index = 1;
    if (filter) {
        if (filter->use_type_filter) {
            sqlite3_bind_int(stmt, param_index++, filter->type_filter);
        }
        if (filter->use_freq_filter) {
            sqlite3_bind_int(stmt, param_index++, filter->freq_min);
            sqlite3_bind_int(stmt, param_index++, filter->freq_max);
        }
        if (filter->use_time_filter) {
            sqlite3_bind_int64(stmt, param_index++, filter->time_start);
            sqlite3_bind_int64(stmt, param_index++, filter->time_end);
        }
        if (filter->use_tag_filter && strlen(filter->tag_filter) > 0) {
            char tag_pattern[64];
            snprintf(tag_pattern, sizeof(tag_pattern), "%%%s%%", filter->tag_filter);
            sqlite3_bind_text(stmt, param_index++, tag_pattern, -1, SQLITE_TRANSIENT);
        }
    }
    
    // Bind limit parameter
    sqlite3_bind_int(stmt, param_index, max_results);
    
    // Allocate results array
    *results = (RFIDSignal*)calloc(max_results, sizeof(RFIDSignal));
    if (!*results) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *count < max_results) {
        RFIDSignal* signal = &(*results)[*count];
        
        signal->id = sqlite3_column_int(stmt, 0);
        signal->timestamp = sqlite3_column_int64(stmt, 1);
        signal->type = sqlite3_column_int(stmt, 2);
        signal->frequency = sqlite3_column_int(stmt, 3);
        signal->data_length = sqlite3_column_int(stmt, 4);
        
        const void* data = sqlite3_column_blob(stmt, 5);
        int data_size = sqlite3_column_bytes(stmt, 5);
        safe_data_copy(signal->data, RFID_SIGNAL_MAX_LENGTH, data, data_size);
        
        signal->checksum = sqlite3_column_int(stmt, 6);
        
        const char* tag = (const char*)sqlite3_column_text(stmt, 7);
        if (tag) {
            strncpy(signal->tag, tag, sizeof(signal->tag) - 1);
            signal->tag[sizeof(signal->tag) - 1] = '\0';
        }
        
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int rfid_db_delete_signal(RFIDDatabase* db, uint32_t signal_id) {
    if (!db) return -1;
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    sqlite3_stmt* stmt = NULL;
    
    const char* delete_sql = "DELETE FROM signals WHERE id = ?";
    
    int rc = sqlite3_prepare_v2(sqlite_db, delete_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, signal_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int rfid_db_get_count(RFIDDatabase* db) {
    if (!db) return -1;
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    sqlite3_stmt* stmt = NULL;
    
    const char* count_sql = "SELECT COUNT(*) FROM signals";
    
    int rc = sqlite3_prepare_v2(sqlite_db, count_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

void rfid_db_optimize(RFIDDatabase* db) {
    if (!db) return;
    
    sqlite3* sqlite_db = (sqlite3*)db->db_handle;
    
    // Run VACUUM to reclaim space and optimize database
    sqlite3_exec(sqlite_db, "VACUUM", NULL, NULL, NULL);
    
    // Analyze tables for query optimization
    sqlite3_exec(sqlite_db, "ANALYZE", NULL, NULL, NULL);
    
    // Clear cache to free memory
    rfid_cache_clear(db);
}

// ============================================================================
// Signal Capture (Simulated for demonstration - replace with actual hardware)
// ============================================================================

static void* capture_thread_func(void* arg) {
    (void)arg;
    
    uint32_t signal_counter = 0;
    
    while (g_capture_running) {
        uint64_t start_time = get_time_microseconds();
        
        // Simulate signal capture (replace with actual RFID hardware interface)
        RFIDSignal signal;
        signal.id = 0;
        signal.timestamp = start_time;
        signal.type = (signal_counter % 2) ? RFID_TYPE_125KHZ : RFID_TYPE_13_56MHZ;
        signal.frequency = signal.type == RFID_TYPE_125KHZ ? 125 : 13560;
        signal.data_length = 64 + (signal_counter % 64);
        
        // Generate sample data
        for (int i = 0; i < signal.data_length; i++) {
            signal.data[i] = (uint8_t)(signal_counter + i);
        }
        
        signal.checksum = rfid_calculate_checksum(signal.data, signal.data_length);
        snprintf(signal.tag, sizeof(signal.tag), "captured_%u", signal_counter);
        
        // Push to ring buffer
        if (!rfid_ringbuffer_push(g_capture_buffer, &signal)) {
            // Buffer full - signal dropped
        }
        
        uint64_t end_time = get_time_microseconds();
        
        pthread_mutex_lock(&g_metrics_mutex);
        g_metrics.signals_captured++;
        g_metrics.avg_capture_latency_us = 
            (g_metrics.avg_capture_latency_us + (end_time - start_time)) / 2;
        pthread_mutex_unlock(&g_metrics_mutex);
        
        signal_counter++;
        
        // Simulate capture rate (adjust based on actual hardware)
        usleep(1000);  // 1ms between captures for simulation
    }
    
    return NULL;
}

int rfid_sniffer_init(void) {
    g_capture_buffer = rfid_ringbuffer_create(RFID_BUFFER_SIZE);
    if (!g_capture_buffer) {
        return -1;
    }
    
    memset(&g_metrics, 0, sizeof(g_metrics));
    
    return 0;
}

void rfid_sniffer_cleanup(void) {
    if (g_capture_running) {
        rfid_capture_stop();
    }
    
    if (g_capture_buffer) {
        rfid_ringbuffer_destroy(g_capture_buffer);
        g_capture_buffer = NULL;
    }
}

int rfid_capture_start(void) {
    if (g_capture_running) {
        return -1;
    }
    
    g_capture_running = true;
    
    if (pthread_create(&g_capture_thread, NULL, capture_thread_func, NULL) != 0) {
        g_capture_running = false;
        return -1;
    }
    
    return 0;
}

int rfid_capture_stop(void) {
    if (!g_capture_running) {
        return -1;
    }
    
    g_capture_running = false;
    pthread_join(g_capture_thread, NULL);
    
    return 0;
}

RFIDSignal* rfid_capture_get_signal(uint32_t timeout_ms) {
    if (!g_capture_buffer) {
        return NULL;
    }
    
    uint64_t start_time = get_time_microseconds();
    uint64_t timeout_us = (uint64_t)timeout_ms * 1000;
    
    while (get_time_microseconds() - start_time < timeout_us) {
        RFIDSignal* signal = (RFIDSignal*)malloc(sizeof(RFIDSignal));
        if (!signal) {
            return NULL;
        }
        
        if (rfid_ringbuffer_pop(g_capture_buffer, signal)) {
            return signal;
        }
        
        free(signal);
        usleep(100);  // Short sleep to prevent busy-waiting
    }
    
    return NULL;
}

// ============================================================================
// Signal Transmission (Simulated - replace with actual hardware)
// ============================================================================

int rfid_transmit_clone(const RFIDSignal* signal) {
    if (!signal) return -1;
    
    // Simulate cloning operation
    printf("Cloning signal ID %u (Type: %d, Freq: %u Hz)\n", 
           signal->id, signal->type, signal->frequency);
    
    pthread_mutex_lock(&g_metrics_mutex);
    g_metrics.signals_transmitted++;
    pthread_mutex_unlock(&g_metrics_mutex);
    
    return 0;
}

int rfid_transmit_emulate(const RFIDSignal* signal, uint32_t duration_ms) {
    if (!signal) return -1;
    
    printf("Emulating signal ID %u for %u ms\n", signal->id, duration_ms);
    
    pthread_mutex_lock(&g_metrics_mutex);
    g_metrics.signals_transmitted++;
    pthread_mutex_unlock(&g_metrics_mutex);
    
    return 0;
}

int rfid_transmit_replay(const RFIDSignal* signal, uint32_t repeat_count) {
    if (!signal) return -1;
    
    printf("Replaying signal ID %u (%u times)\n", signal->id, repeat_count);
    
    for (uint32_t i = 0; i < repeat_count; i++) {
        // Simulate replay
        usleep(10000);  // 10ms per replay
    }
    
    pthread_mutex_lock(&g_metrics_mutex);
    g_metrics.signals_transmitted += repeat_count;
    pthread_mutex_unlock(&g_metrics_mutex);
    
    return 0;
}

int rfid_transmit_generate(RFIDType type, const uint8_t* custom_data, uint16_t data_length) {
    if (!custom_data || data_length == 0) return -1;
    
    printf("Generating custom signal (Type: %d, Length: %u bytes)\n", type, data_length);
    
    pthread_mutex_lock(&g_metrics_mutex);
    g_metrics.signals_transmitted++;
    pthread_mutex_unlock(&g_metrics_mutex);
    
    return 0;
}

// ============================================================================
// Filtering Operations
// ============================================================================

int rfid_filter_by_type(RFIDDatabase* db, RFIDType type, RFIDSignal** results, uint32_t* count) {
    RFIDFilter filter = {0};
    filter.type_filter = type;
    filter.use_type_filter = true;
    
    return rfid_db_get_signals_filtered(db, &filter, results, count, 1000);
}

int rfid_filter_by_frequency(RFIDDatabase* db, uint16_t freq_min, uint16_t freq_max,
                             RFIDSignal** results, uint32_t* count) {
    RFIDFilter filter = {0};
    filter.freq_min = freq_min;
    filter.freq_max = freq_max;
    filter.use_freq_filter = true;
    
    return rfid_db_get_signals_filtered(db, &filter, results, count, 1000);
}

int rfid_filter_by_time_range(RFIDDatabase* db, uint64_t start_time, uint64_t end_time,
                              RFIDSignal** results, uint32_t* count) {
    RFIDFilter filter = {0};
    filter.time_start = start_time;
    filter.time_end = end_time;
    filter.use_time_filter = true;
    
    return rfid_db_get_signals_filtered(db, &filter, results, count, 1000);
}

int rfid_filter_by_tag(RFIDDatabase* db, const char* tag, RFIDSignal** results, uint32_t* count) {
    if (!tag) return -1;
    
    RFIDFilter filter = {0};
    strncpy(filter.tag_filter, tag, sizeof(filter.tag_filter) - 1);
    filter.use_tag_filter = true;
    
    return rfid_db_get_signals_filtered(db, &filter, results, count, 1000);
}

// ============================================================================
// Export Operations
// ============================================================================

int rfid_export_to_json(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter) {
    if (!db || !output_path) return -1;
    
    FILE* fp = fopen(output_path, "w");
    if (!fp) return -1;
    
    RFIDSignal* signals = NULL;
    uint32_t count = 0;
    
    if (rfid_db_get_signals_filtered(db, filter, &signals, &count, 10000) != 0) {
        fclose(fp);
        return -1;
    }
    
    fprintf(fp, "[\n");
    for (uint32_t i = 0; i < count; i++) {
        fprintf(fp, "  {\n");
        fprintf(fp, "    \"id\": %u,\n", signals[i].id);
        fprintf(fp, "    \"timestamp\": %llu,\n", (unsigned long long)signals[i].timestamp);
        fprintf(fp, "    \"type\": %d,\n", signals[i].type);
        fprintf(fp, "    \"frequency\": %u,\n", signals[i].frequency);
        fprintf(fp, "    \"data_length\": %u,\n", signals[i].data_length);
        fprintf(fp, "    \"checksum\": %u,\n", signals[i].checksum);
        fprintf(fp, "    \"tag\": \"%s\"\n", signals[i].tag);
        fprintf(fp, "  }%s\n", (i < count - 1) ? "," : "");
    }
    fprintf(fp, "]\n");
    
    free(signals);
    fclose(fp);
    
    return 0;
}

int rfid_export_to_csv(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter) {
    if (!db || !output_path) return -1;
    
    FILE* fp = fopen(output_path, "w");
    if (!fp) return -1;
    
    fprintf(fp, "id,timestamp,type,frequency,data_length,checksum,tag\n");
    
    RFIDSignal* signals = NULL;
    uint32_t count = 0;
    
    if (rfid_db_get_signals_filtered(db, filter, &signals, &count, 10000) != 0) {
        fclose(fp);
        return -1;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        fprintf(fp, "%u,%llu,%d,%u,%u,%u,%s\n",
                signals[i].id,
                (unsigned long long)signals[i].timestamp,
                signals[i].type,
                signals[i].frequency,
                signals[i].data_length,
                signals[i].checksum,
                signals[i].tag);
    }
    
    free(signals);
    fclose(fp);
    
    return 0;
}

int rfid_export_to_binary(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter) {
    if (!db || !output_path) return -1;
    
    FILE* fp = fopen(output_path, "wb");
    if (!fp) return -1;
    
    RFIDSignal* signals = NULL;
    uint32_t count = 0;
    
    if (rfid_db_get_signals_filtered(db, filter, &signals, &count, 10000) != 0) {
        fclose(fp);
        return -1;
    }
    
    // Write header
    fwrite("RFID", 1, 4, fp);
    fwrite(&count, sizeof(uint32_t), 1, fp);
    
    // Write signals
    fwrite(signals, sizeof(RFIDSignal), count, fp);
    
    free(signals);
    fclose(fp);
    
    return 0;
}

// ============================================================================
// Visualization Data Generation
// ============================================================================

RFIDVisualizationData* rfid_generate_visualization(RFIDDatabase* db, const RFIDFilter* filter) {
    if (!db) return NULL;
    
    RFIDVisualizationData* vis_data = (RFIDVisualizationData*)calloc(1, sizeof(RFIDVisualizationData));
    if (!vis_data) return NULL;
    
    RFIDSignal* signals = NULL;
    uint32_t count = 0;
    
    if (rfid_db_get_signals_filtered(db, filter, &signals, &count, 10000) != 0) {
        free(vis_data);
        return NULL;
    }
    
    // Generate histogram (signal types)
    vis_data->histogram_bins = 3;  // 125kHz, 13.56MHz, Unknown
    vis_data->histogram_data = (uint32_t*)calloc(vis_data->histogram_bins, sizeof(uint32_t));
    
    for (uint32_t i = 0; i < count; i++) {
        if (signals[i].type < vis_data->histogram_bins) {
            vis_data->histogram_data[signals[i].type]++;
        }
    }
    
    // Generate frequency spectrum
    vis_data->spectrum_size = 256;
    vis_data->frequency_spectrum = (uint16_t*)calloc(vis_data->spectrum_size, sizeof(uint16_t));
    
    // Generate timeline
    vis_data->timeline_points = count < 100 ? count : 100;
    vis_data->timeline_data = (uint64_t*)calloc(vis_data->timeline_points, sizeof(uint64_t));
    
    for (uint32_t i = 0; i < vis_data->timeline_points; i++) {
        uint32_t idx = (count * i) / vis_data->timeline_points;
        vis_data->timeline_data[i] = signals[idx].timestamp;
    }
    
    free(signals);
    return vis_data;
}

void rfid_visualization_destroy(RFIDVisualizationData* vis_data) {
    if (!vis_data) return;
    
    free(vis_data->histogram_data);
    free(vis_data->frequency_spectrum);
    free(vis_data->timeline_data);
    free(vis_data);
}

// ============================================================================
// Performance Monitoring
// ============================================================================

void rfid_get_performance_metrics(RFIDPerformanceMetrics* metrics) {
    if (!metrics) return;
    
    pthread_mutex_lock(&g_metrics_mutex);
    memcpy(metrics, &g_metrics, sizeof(RFIDPerformanceMetrics));
    pthread_mutex_unlock(&g_metrics_mutex);
}

void rfid_reset_performance_metrics(void) {
    pthread_mutex_lock(&g_metrics_mutex);
    memset(&g_metrics, 0, sizeof(g_metrics));
    pthread_mutex_unlock(&g_metrics_mutex);
}

// ============================================================================
// Utility Functions
// ============================================================================

uint8_t rfid_calculate_checksum(const uint8_t* data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

bool rfid_verify_signal(const RFIDSignal* signal) {
    if (!signal) return false;
    
    uint8_t calculated_checksum = rfid_calculate_checksum(signal->data, signal->data_length);
    return calculated_checksum == signal->checksum;
}

void rfid_print_signal(const RFIDSignal* signal) {
    if (!signal) return;
    
    printf("Signal ID: %u\n", signal->id);
    printf("Timestamp: %llu\n", (unsigned long long)signal->timestamp);
    printf("Type: %d\n", signal->type);
    printf("Frequency: %u Hz\n", signal->frequency);
    printf("Data Length: %u bytes\n", signal->data_length);
    printf("Checksum: 0x%02X\n", signal->checksum);
    printf("Tag: %s\n", signal->tag);
    printf("Data: ");
    for (int i = 0; i < signal->data_length && i < 32; i++) {
        printf("%02X ", signal->data[i]);
    }
    if (signal->data_length > 32) {
        printf("...");
    }
    printf("\n");
}
