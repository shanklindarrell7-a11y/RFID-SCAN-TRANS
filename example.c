/**
 * RFID Sniffer Example and Performance Test Application
 * Demonstrates all features and measures performance
 */

#include "rfid_sniffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void print_separator(void) {
    printf("\n========================================\n");
}

void test_ringbuffer_performance(void) {
    print_separator();
    printf("Testing Ring Buffer Performance\n");
    print_separator();
    
    RFIDRingBuffer* rb = rfid_ringbuffer_create(RFID_BUFFER_SIZE);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return;
    }
    
    printf("Ring buffer created with capacity: %u\n", rfid_ringbuffer_get_capacity(rb));
    
    // Performance test: push operations
    clock_t start = clock();
    uint32_t push_count = 0;
    
    for (uint32_t i = 0; i < 10000; i++) {
        RFIDSignal signal = {0};
        signal.id = i;
        signal.timestamp = (uint64_t)time(NULL);
        signal.type = RFID_TYPE_125KHZ;
        signal.frequency = 125;
        signal.data_length = 64;
        
        if (rfid_ringbuffer_push(rb, &signal)) {
            push_count++;
        }
        
        // Pop every 10th signal to prevent buffer from filling
        if (i % 10 == 0) {
            RFIDSignal temp;
            rfid_ringbuffer_pop(rb, &temp);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Pushed %u signals in %.3f seconds\n", push_count, elapsed);
    printf("Throughput: %.0f signals/second\n", push_count / elapsed);
    
    rfid_ringbuffer_destroy(rb);
}

void test_database_performance(void) {
    print_separator();
    printf("Testing Database Performance\n");
    print_separator();
    
    // Remove existing test database
    remove("test_rfid.db");
    
    RFIDDatabase* db = rfid_db_open("test_rfid.db");
    if (!db) {
        printf("Failed to open database\n");
        return;
    }
    
    printf("Database opened successfully\n");
    
    // Test 1: Individual insert performance
    printf("\nTest 1: Individual inserts (100 signals)\n");
    clock_t start = clock();
    
    for (uint32_t i = 0; i < 100; i++) {
        RFIDSignal signal = {0};
        signal.timestamp = (uint64_t)time(NULL) + i;
        signal.type = (i % 2) ? RFID_TYPE_125KHZ : RFID_TYPE_13_56MHZ;
        signal.frequency = signal.type == RFID_TYPE_125KHZ ? 125 : 13560;
        signal.data_length = 64;
        
        for (int j = 0; j < signal.data_length; j++) {
            signal.data[j] = (uint8_t)(i + j);
        }
        
        signal.checksum = rfid_calculate_checksum(signal.data, signal.data_length);
        snprintf(signal.tag, sizeof(signal.tag), "test_%u", i);
        
        if (rfid_db_store_signal(db, &signal) != 0) {
            printf("Failed to store signal %u\n", i);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Individual insert time: %.3f seconds\n", elapsed);
    printf("Average per signal: %.3f ms\n", (elapsed * 1000) / 100);
    
    // Test 2: Batch insert performance
    printf("\nTest 2: Batch insert (1000 signals)\n");
    RFIDSignal* batch_signals = (RFIDSignal*)calloc(1000, sizeof(RFIDSignal));
    
    for (uint32_t i = 0; i < 1000; i++) {
        batch_signals[i].timestamp = (uint64_t)time(NULL) + i + 100;
        batch_signals[i].type = (i % 2) ? RFID_TYPE_125KHZ : RFID_TYPE_13_56MHZ;
        batch_signals[i].frequency = batch_signals[i].type == RFID_TYPE_125KHZ ? 125 : 13560;
        batch_signals[i].data_length = 64;
        
        for (int j = 0; j < batch_signals[i].data_length; j++) {
            batch_signals[i].data[j] = (uint8_t)(i + j);
        }
        
        batch_signals[i].checksum = rfid_calculate_checksum(batch_signals[i].data, 
                                                            batch_signals[i].data_length);
        snprintf(batch_signals[i].tag, sizeof(batch_signals[i].tag), "batch_%u", i);
    }
    
    start = clock();
    rfid_db_store_signals_batch(db, batch_signals, 1000);
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Batch insert time: %.3f seconds\n", elapsed);
    printf("Average per signal: %.3f ms\n", (elapsed * 1000) / 1000);
    printf("Throughput: %.0f signals/second\n", 1000 / elapsed);
    
    free(batch_signals);
    
    // Test 3: Query performance
    printf("\nTest 3: Query performance\n");
    int total_signals = rfid_db_get_count(db);
    printf("Total signals in database: %d\n", total_signals);
    
    // Test cache performance
    start = clock();
    for (uint32_t i = 1; i <= 50; i++) {
        RFIDSignal* signal = rfid_db_get_signal(db, i);
        if (signal) {
            free(signal);
        }
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("First 50 queries (cold cache): %.3f ms\n", elapsed * 1000);
    
    // Query again to test cache
    start = clock();
    for (uint32_t i = 1; i <= 50; i++) {
        RFIDSignal* signal = rfid_db_get_signal(db, i);
        if (signal) {
            free(signal);
        }
    }
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Second 50 queries (warm cache): %.3f ms\n", elapsed * 1000);
    
    printf("Cache hit rate: %.1f%%\n", 
           (db->cache_hits * 100.0) / (db->cache_hits + db->cache_misses));
    
    // Test 4: Filtered query performance
    printf("\nTest 4: Filtered query performance\n");
    RFIDFilter filter = {0};
    filter.type_filter = RFID_TYPE_125KHZ;
    filter.use_type_filter = true;
    
    RFIDSignal* results = NULL;
    uint32_t count = 0;
    
    start = clock();
    rfid_db_get_signals_filtered(db, &filter, &results, &count, 1000);
    end = clock();
    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Filtered query returned %u signals in %.3f ms\n", count, elapsed * 1000);
    free(results);
    
    rfid_db_close(db);
}

void test_capture_performance(void) {
    print_separator();
    printf("Testing Real-time Capture Performance\n");
    print_separator();
    
    if (rfid_sniffer_init() != 0) {
        printf("Failed to initialize sniffer\n");
        return;
    }
    
    printf("Starting signal capture...\n");
    
    if (rfid_capture_start() != 0) {
        printf("Failed to start capture\n");
        rfid_sniffer_cleanup();
        return;
    }
    
    // Capture signals for 2 seconds
    printf("Capturing signals for 2 seconds...\n");
    uint32_t captured_count = 0;
    time_t start_time = time(NULL);
    
    while (time(NULL) - start_time < 2) {
        RFIDSignal* signal = rfid_capture_get_signal(10);
        if (signal) {
            captured_count++;
            free(signal);
        }
    }
    
    rfid_capture_stop();
    
    RFIDPerformanceMetrics metrics;
    rfid_get_performance_metrics(&metrics);
    
    printf("\nCapture Performance:\n");
    printf("Signals captured: %u\n", captured_count);
    printf("Capture rate: %.0f signals/second\n", captured_count / 2.0);
    printf("Average capture latency: %u microseconds\n", metrics.avg_capture_latency_us);
    
    rfid_sniffer_cleanup();
}

void test_filtering_features(void) {
    print_separator();
    printf("Testing Advanced Filtering Features\n");
    print_separator();
    
    RFIDDatabase* db = rfid_db_open("test_rfid.db");
    if (!db) {
        printf("Failed to open database\n");
        return;
    }
    
    // Test type filtering
    printf("\nTest 1: Filter by type (125kHz)\n");
    RFIDSignal* results = NULL;
    uint32_t count = 0;
    
    if (rfid_filter_by_type(db, RFID_TYPE_125KHZ, &results, &count) == 0) {
        printf("Found %u signals with type 125kHz\n", count);
        free(results);
    }
    
    // Test frequency filtering
    printf("\nTest 2: Filter by frequency range (100-200 Hz)\n");
    results = NULL;
    count = 0;
    
    if (rfid_filter_by_frequency(db, 100, 200, &results, &count) == 0) {
        printf("Found %u signals in frequency range\n", count);
        free(results);
    }
    
    // Test tag filtering
    printf("\nTest 3: Filter by tag (batch_*)\n");
    results = NULL;
    count = 0;
    
    if (rfid_filter_by_tag(db, "batch", &results, &count) == 0) {
        printf("Found %u signals matching tag pattern\n", count);
        free(results);
    }
    
    rfid_db_close(db);
}

void test_export_features(void) {
    print_separator();
    printf("Testing Batch Export Features\n");
    print_separator();
    
    RFIDDatabase* db = rfid_db_open("test_rfid.db");
    if (!db) {
        printf("Failed to open database\n");
        return;
    }
    
    printf("\nExporting to JSON...\n");
    RFIDFilter filter = {0};
    filter.type_filter = RFID_TYPE_125KHZ;
    filter.use_type_filter = true;
    
    clock_t start = clock();
    if (rfid_export_to_json(db, "export_125khz.json", &filter) == 0) {
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("JSON export completed in %.3f seconds\n", elapsed);
    }
    
    printf("\nExporting to CSV...\n");
    start = clock();
    if (rfid_export_to_csv(db, "export_all.csv", NULL) == 0) {
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("CSV export completed in %.3f seconds\n", elapsed);
    }
    
    printf("\nExporting to binary format...\n");
    start = clock();
    if (rfid_export_to_binary(db, "export_all.bin", NULL) == 0) {
        clock_t end = clock();
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Binary export completed in %.3f seconds\n", elapsed);
    }
    
    rfid_db_close(db);
}

void test_transmission_modes(void) {
    print_separator();
    printf("Testing Transmission Modes\n");
    print_separator();
    
    RFIDSignal test_signal = {0};
    test_signal.id = 1;
    test_signal.timestamp = (uint64_t)time(NULL);
    test_signal.type = RFID_TYPE_125KHZ;
    test_signal.frequency = 125;
    test_signal.data_length = 64;
    
    for (int i = 0; i < test_signal.data_length; i++) {
        test_signal.data[i] = (uint8_t)i;
    }
    
    test_signal.checksum = rfid_calculate_checksum(test_signal.data, test_signal.data_length);
    strcpy(test_signal.tag, "transmission_test");
    
    printf("\nTest 1: Clone mode\n");
    rfid_transmit_clone(&test_signal);
    
    printf("\nTest 2: Emulate mode (1000ms)\n");
    rfid_transmit_emulate(&test_signal, 1000);
    
    printf("\nTest 3: Replay mode (5 times)\n");
    rfid_transmit_replay(&test_signal, 5);
    
    printf("\nTest 4: Generate custom signal\n");
    uint8_t custom_data[32] = {0xAA, 0xBB, 0xCC, 0xDD};
    rfid_transmit_generate(RFID_TYPE_13_56MHZ, custom_data, 32);
}

void test_visualization_features(void) {
    print_separator();
    printf("Testing Visualization Features\n");
    print_separator();
    
    RFIDDatabase* db = rfid_db_open("test_rfid.db");
    if (!db) {
        printf("Failed to open database\n");
        return;
    }
    
    printf("\nGenerating visualization data...\n");
    clock_t start = clock();
    RFIDVisualizationData* vis_data = rfid_generate_visualization(db, NULL);
    clock_t end = clock();
    
    if (vis_data) {
        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Visualization data generated in %.3f ms\n", elapsed * 1000);
        
        printf("\nHistogram (signal type distribution):\n");
        printf("  125kHz signals: %u\n", vis_data->histogram_data[RFID_TYPE_125KHZ]);
        printf("  13.56MHz signals: %u\n", vis_data->histogram_data[RFID_TYPE_13_56MHZ]);
        printf("  Unknown signals: %u\n", vis_data->histogram_data[RFID_TYPE_UNKNOWN]);
        
        printf("\nTimeline data points: %u\n", vis_data->timeline_points);
        
        rfid_visualization_destroy(vis_data);
    }
    
    rfid_db_close(db);
}

void print_overall_metrics(void) {
    print_separator();
    printf("Overall Performance Metrics\n");
    print_separator();
    
    RFIDPerformanceMetrics metrics;
    rfid_get_performance_metrics(&metrics);
    
    printf("Total signals captured: %u\n", metrics.signals_captured);
    printf("Total signals stored: %u\n", metrics.signals_stored);
    printf("Total signals transmitted: %u\n", metrics.signals_transmitted);
    printf("Average capture latency: %u us\n", metrics.avg_capture_latency_us);
    printf("Average storage latency: %u us\n", metrics.avg_storage_latency_us);
}

int main(void) {
    printf("RFID Sniffer - Performance Test Suite\n");
    printf("======================================\n");
    
    // Run all tests
    test_ringbuffer_performance();
    test_database_performance();
    test_capture_performance();
    test_filtering_features();
    test_export_features();
    test_transmission_modes();
    test_visualization_features();
    
    print_overall_metrics();
    
    print_separator();
    printf("All tests completed!\n");
    print_separator();
    
    // Cleanup test files
    printf("\nCleaning up test files...\n");
    remove("test_rfid.db");
    remove("test_rfid.db-wal");
    remove("test_rfid.db-shm");
    remove("export_125khz.json");
    remove("export_all.csv");
    remove("export_all.bin");
    
    return 0;
}
