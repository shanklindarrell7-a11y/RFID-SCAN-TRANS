/**
 * RFID Sniffer - High Performance RFID Signal Capture and Analysis
 * Optimized for Flipper Zero
 * 
 * Features:
 * - Real-time signal sniffing with reduced latency
 * - Efficient storage and database management for 1000+ signals
 * - Multiple transmission modes: Clone, Emulate, Replay, Generate
 * - Advanced signal visualization, filtering, and batch exports
 */

#ifndef RFID_SNIFFER_H
#define RFID_SNIFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Configuration constants optimized for performance
#define RFID_BUFFER_SIZE 512          // Optimized buffer size for real-time capture
#define RFID_SIGNAL_MAX_LENGTH 256    // Maximum signal data length
#define RFID_DB_CACHE_SIZE 100        // LRU cache size for frequently accessed signals
#define RFID_BATCH_EXPORT_SIZE 50     // Batch size for bulk operations
#define RFID_FILTER_CACHE_SIZE 32     // Cache size for filter results

// Signal types
typedef enum {
    RFID_TYPE_125KHZ,
    RFID_TYPE_13_56MHZ,
    RFID_TYPE_UNKNOWN
} RFIDType;

// Transmission modes
typedef enum {
    RFID_MODE_CLONE,
    RFID_MODE_EMULATE,
    RFID_MODE_REPLAY,
    RFID_MODE_GENERATE
} RFIDTransmitMode;

// Signal structure with metadata for efficient processing
typedef struct {
    uint32_t id;
    uint64_t timestamp;
    RFIDType type;
    uint16_t frequency;
    uint16_t data_length;
    uint8_t data[RFID_SIGNAL_MAX_LENGTH];
    uint8_t checksum;
    char tag[32];  // User-defined tag for filtering
} RFIDSignal;

// Ring buffer for lock-free real-time capture
typedef struct {
    RFIDSignal* buffer;
    volatile uint32_t head;
    volatile uint32_t tail;
    uint32_t capacity;
} RFIDRingBuffer;

// LRU cache node for database optimization
typedef struct CacheNode {
    uint32_t signal_id;
    RFIDSignal signal;
    struct CacheNode* prev;
    struct CacheNode* next;
    uint32_t access_count;
} CacheNode;

// Database context with optimizations
typedef struct {
    void* db_handle;
    CacheNode* cache_head;
    CacheNode* cache_tail;
    uint32_t cache_size;
    uint32_t cache_hits;
    uint32_t cache_misses;
    bool index_enabled;
} RFIDDatabase;

// Filter configuration for efficient signal filtering
typedef struct {
    RFIDType type_filter;
    uint16_t freq_min;
    uint16_t freq_max;
    uint64_t time_start;
    uint64_t time_end;
    char tag_filter[32];
    bool use_type_filter;
    bool use_freq_filter;
    bool use_time_filter;
    bool use_tag_filter;
} RFIDFilter;

// Performance metrics
typedef struct {
    uint32_t signals_captured;
    uint32_t signals_stored;
    uint32_t signals_transmitted;
    uint32_t avg_capture_latency_us;
    uint32_t avg_storage_latency_us;
    uint32_t db_cache_hit_rate;
    uint64_t total_processing_time_us;
} RFIDPerformanceMetrics;

// Core API Functions

// Initialization and cleanup
int rfid_sniffer_init(void);
void rfid_sniffer_cleanup(void);

// Real-time signal capture (optimized for low latency)
int rfid_capture_start(void);
int rfid_capture_stop(void);
RFIDSignal* rfid_capture_get_signal(uint32_t timeout_ms);

// Ring buffer operations (lock-free)
RFIDRingBuffer* rfid_ringbuffer_create(uint32_t capacity);
void rfid_ringbuffer_destroy(RFIDRingBuffer* rb);
bool rfid_ringbuffer_push(RFIDRingBuffer* rb, const RFIDSignal* signal);
bool rfid_ringbuffer_pop(RFIDRingBuffer* rb, RFIDSignal* signal);
bool rfid_ringbuffer_is_empty(RFIDRingBuffer* rb);
bool rfid_ringbuffer_is_full(RFIDRingBuffer* rb);

// Database operations (optimized for large datasets)
RFIDDatabase* rfid_db_open(const char* db_path);
void rfid_db_close(RFIDDatabase* db);
int rfid_db_store_signal(RFIDDatabase* db, const RFIDSignal* signal);
int rfid_db_store_signals_batch(RFIDDatabase* db, const RFIDSignal* signals, uint32_t count);
RFIDSignal* rfid_db_get_signal(RFIDDatabase* db, uint32_t signal_id);
int rfid_db_get_signals_filtered(RFIDDatabase* db, const RFIDFilter* filter, 
                                  RFIDSignal** results, uint32_t* count, uint32_t max_results);
int rfid_db_delete_signal(RFIDDatabase* db, uint32_t signal_id);
int rfid_db_get_count(RFIDDatabase* db);
void rfid_db_optimize(RFIDDatabase* db);  // Database maintenance and optimization

// LRU cache operations
void rfid_cache_put(RFIDDatabase* db, const RFIDSignal* signal);
RFIDSignal* rfid_cache_get(RFIDDatabase* db, uint32_t signal_id);
void rfid_cache_clear(RFIDDatabase* db);

// Signal transmission modes
int rfid_transmit_clone(const RFIDSignal* signal);
int rfid_transmit_emulate(const RFIDSignal* signal, uint32_t duration_ms);
int rfid_transmit_replay(const RFIDSignal* signal, uint32_t repeat_count);
int rfid_transmit_generate(RFIDType type, const uint8_t* custom_data, uint16_t data_length);

// Advanced filtering and search (with result caching)
int rfid_filter_by_type(RFIDDatabase* db, RFIDType type, RFIDSignal** results, uint32_t* count);
int rfid_filter_by_frequency(RFIDDatabase* db, uint16_t freq_min, uint16_t freq_max, 
                             RFIDSignal** results, uint32_t* count);
int rfid_filter_by_time_range(RFIDDatabase* db, uint64_t start_time, uint64_t end_time,
                              RFIDSignal** results, uint32_t* count);
int rfid_filter_by_tag(RFIDDatabase* db, const char* tag, RFIDSignal** results, uint32_t* count);

// Batch export operations
int rfid_export_to_json(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter);
int rfid_export_to_csv(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter);
int rfid_export_to_binary(RFIDDatabase* db, const char* output_path, const RFIDFilter* filter);

// Signal visualization data generation
typedef struct {
    uint32_t* histogram_data;
    uint32_t histogram_bins;
    uint16_t* frequency_spectrum;
    uint32_t spectrum_size;
    uint64_t* timeline_data;
    uint32_t timeline_points;
} RFIDVisualizationData;

RFIDVisualizationData* rfid_generate_visualization(RFIDDatabase* db, const RFIDFilter* filter);
void rfid_visualization_destroy(RFIDVisualizationData* vis_data);

// Performance monitoring
void rfid_get_performance_metrics(RFIDPerformanceMetrics* metrics);
void rfid_reset_performance_metrics(void);

// Utility functions
uint8_t rfid_calculate_checksum(const uint8_t* data, uint16_t length);
bool rfid_verify_signal(const RFIDSignal* signal);
void rfid_print_signal(const RFIDSignal* signal);

#endif // RFID_SNIFFER_H
