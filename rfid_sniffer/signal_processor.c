#include "signal_processor.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>
#include <stdlib.h>
#include <string.h>

#define TAG "SignalProcessor"
#define MAX_SIGNAL_SIZE 4096
#define CAPTURE_TIMEOUT_MS 5000
#define RFID_FREQUENCY_125KHZ 125000

struct SignalProcessor {
    uint8_t* signal_data;
    size_t signal_size;
    size_t signal_capacity;
    SignalFormat format;
    ProtocolType protocol;
    bool is_capturing;
    FuriMutex* mutex;
    FuriThread* capture_thread;
};

static int32_t signal_processor_capture_thread(void* context);

SignalProcessor* signal_processor_alloc(void) {
    SignalProcessor* processor = malloc(sizeof(SignalProcessor));
    
    processor->signal_capacity = MAX_SIGNAL_SIZE;
    processor->signal_data = malloc(processor->signal_capacity);
    processor->signal_size = 0;
    processor->format = SignalFormatASK;
    processor->protocol = ProtocolEM4100;
    processor->is_capturing = false;
    processor->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    processor->capture_thread = NULL;
    
    FURI_LOG_I(TAG, "Signal processor allocated");
    
    return processor;
}

void signal_processor_free(SignalProcessor* processor) {
    furi_assert(processor);
    
    if(processor->is_capturing && processor->capture_thread) {
        signal_processor_stop(processor);
    }
    
    if(processor->signal_data) {
        free(processor->signal_data);
    }
    
    furi_mutex_free(processor->mutex);
    free(processor);
    
    FURI_LOG_I(TAG, "Signal processor freed");
}

static bool signal_processor_read_rfid_signal(SignalProcessor* processor) {
    furi_assert(processor);
    
    FURI_LOG_D(TAG, "Reading RFID signal...");
    
    // NOTE: This is a simulated implementation for demonstration purposes.
    // In a production implementation, this would interface with the RFID hardware:
    //   - Initialize RFID peripheral (furi_hal_rfid_*)
    //   - Configure antenna and timing
    //   - Read raw samples from ADC
    //   - Decode based on selected protocol
    //   - Validate signal integrity
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    // Simulate captured data (replace with actual RFID hardware calls)
    // Example: furi_hal_rfid_read(processor->signal_data, &processor->signal_size);
    processor->signal_size = 64; // Example: 64 bytes of data
    
    // Simulate EM4100 card ID pattern
    // Format: Header + ID + Parity
    uint8_t sample_id[] = {
        0xFF, 0x00, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC,
        0xDE, 0xF0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66
    };
    
    memcpy(processor->signal_data, sample_id, sizeof(sample_id));
    
    for(size_t i = sizeof(sample_id); i < processor->signal_size; i++) {
        processor->signal_data[i] = (uint8_t)(i & 0xFF);
    }
    
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "RFID signal captured: %zu bytes", processor->signal_size);
    
    return processor->signal_size > 0;
}

static int32_t signal_processor_capture_thread(void* context) {
    SignalProcessor* processor = context;
    
    FURI_LOG_I(TAG, "Capture thread started");
    
    uint32_t start_time = furi_get_tick();
    bool signal_found = false;
    
    while(processor->is_capturing) {
        if(signal_processor_read_rfid_signal(processor)) {
            signal_found = true;
            FURI_LOG_I(TAG, "Signal detected!");
            break;
        }
        
        if(furi_get_tick() - start_time > CAPTURE_TIMEOUT_MS) {
            FURI_LOG_W(TAG, "Capture timeout");
            break;
        }
        
        furi_delay_ms(100);
    }
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    processor->is_capturing = false;
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "Capture thread stopped. Signal found: %s", signal_found ? "yes" : "no");
    
    return 0;
}

bool signal_processor_start(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    if(processor->is_capturing) {
        furi_mutex_release(processor->mutex);
        FURI_LOG_W(TAG, "Already capturing");
        return false;
    }
    
    processor->is_capturing = true;
    processor->signal_size = 0;
    
    furi_mutex_release(processor->mutex);
    
    processor->capture_thread = furi_thread_alloc();
    furi_thread_set_name(processor->capture_thread, "RFIDCapture");
    furi_thread_set_stack_size(processor->capture_thread, 2048);
    furi_thread_set_context(processor->capture_thread, processor);
    furi_thread_set_callback(processor->capture_thread, signal_processor_capture_thread);
    furi_thread_start(processor->capture_thread);
    
    FURI_LOG_I(TAG, "Signal capture started");
    
    return true;
}

void signal_processor_stop(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    processor->is_capturing = false;
    furi_mutex_release(processor->mutex);
    
    if(processor->capture_thread) {
        furi_thread_join(processor->capture_thread);
        furi_thread_free(processor->capture_thread);
        processor->capture_thread = NULL;
    }
    
    FURI_LOG_I(TAG, "Signal capture stopped");
}

bool signal_processor_has_data(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    bool has_data = processor->signal_size > 0;
    furi_mutex_release(processor->mutex);
    
    return has_data;
}

bool signal_processor_save(SignalProcessor* processor, const char* file_path) {
    furi_assert(processor);
    furi_assert(file_path);
    
    FURI_LOG_I(TAG, "Saving signal to: %s", file_path);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    if(processor->signal_size == 0) {
        furi_mutex_release(processor->mutex);
        FURI_LOG_E(TAG, "No signal data to save");
        return false;
    }
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    
    if(storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        // Write file header
        const char* header = "Filetype: Flipper RFID key\n";
        storage_file_write(file, header, strlen(header));
        
        // Write version
        const char* version = "Version: 1\n";
        storage_file_write(file, version, strlen(version));
        
        // Write format
        char format_line[64];
        snprintf(format_line, sizeof(format_line), "Format: %d\n", processor->format);
        storage_file_write(file, format_line, strlen(format_line));
        
        // Write protocol
        char protocol_line[64];
        snprintf(protocol_line, sizeof(protocol_line), "Protocol: %d\n", processor->protocol);
        storage_file_write(file, protocol_line, strlen(protocol_line));
        
        // Write data size
        char size_line[64];
        snprintf(size_line, sizeof(size_line), "Data size: %zu\n", processor->signal_size);
        storage_file_write(file, size_line, strlen(size_line));
        
        // Write data
        const char* data_header = "Data: ";
        storage_file_write(file, data_header, strlen(data_header));
        
        for(size_t i = 0; i < processor->signal_size; i++) {
            char byte_str[4];
            snprintf(byte_str, sizeof(byte_str), "%02X ", processor->signal_data[i]);
            storage_file_write(file, byte_str, strlen(byte_str));
        }
        storage_file_write(file, "\n", 1);
        
        storage_file_close(file);
        success = true;
        FURI_LOG_I(TAG, "Signal saved successfully");
    } else {
        FURI_LOG_E(TAG, "Failed to open file for writing");
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    
    furi_mutex_release(processor->mutex);
    
    return success;
}

bool signal_processor_load(SignalProcessor* processor, const char* file_path) {
    furi_assert(processor);
    furi_assert(file_path);
    
    FURI_LOG_I(TAG, "Loading signal from: %s", file_path);
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    
    if(storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        furi_mutex_acquire(processor->mutex, FuriWaitForever);
        
        // Reset current data
        processor->signal_size = 0;
        
        // Read file line by line and parse
        // This is a simplified implementation
        // A complete implementation would parse the full format
        char buffer[256];
        while(storage_file_read(file, buffer, sizeof(buffer)) > 0) {
            // Parse data (simplified - would need full parser)
            if(strncmp(buffer, "Data: ", 6) == 0) {
                // Signal that data was found
                processor->signal_size = 64; // Example size
                success = true;
                break;
            }
        }
        
        furi_mutex_release(processor->mutex);
        storage_file_close(file);
        
        if(success) {
            FURI_LOG_I(TAG, "Signal loaded successfully");
        } else {
            FURI_LOG_E(TAG, "Failed to parse signal data");
        }
    } else {
        FURI_LOG_E(TAG, "Failed to open file for reading");
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    
    return success;
}

bool signal_processor_clone(SignalProcessor* processor) {
    furi_assert(processor);
    
    FURI_LOG_I(TAG, "Cloning signal...");
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    if(processor->signal_size == 0) {
        furi_mutex_release(processor->mutex);
        FURI_LOG_E(TAG, "No signal data to clone");
        return false;
    }
    
    // Simulate cloning process
    furi_delay_ms(500);
    
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "Signal cloned successfully");
    
    return true;
}

bool signal_processor_emulate(SignalProcessor* processor) {
    furi_assert(processor);
    
    FURI_LOG_I(TAG, "Emulating signal...");
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    if(processor->signal_size == 0) {
        furi_mutex_release(processor->mutex);
        FURI_LOG_E(TAG, "No signal data to emulate");
        return false;
    }
    
    // Simulate emulation process
    furi_delay_ms(1000);
    
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "Signal emulation completed");
    
    return true;
}

bool signal_processor_replay(SignalProcessor* processor) {
    furi_assert(processor);
    
    FURI_LOG_I(TAG, "Replaying signal...");
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    if(processor->signal_size == 0) {
        furi_mutex_release(processor->mutex);
        FURI_LOG_E(TAG, "No signal data to replay");
        return false;
    }
    
    // Simulate replay process
    furi_delay_ms(800);
    
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "Signal replay completed");
    
    return true;
}

bool signal_processor_generate(SignalProcessor* processor) {
    furi_assert(processor);
    
    FURI_LOG_I(TAG, "Generating signal...");
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    
    // Generate a new signal based on protocol
    processor->signal_size = 32;
    
    for(size_t i = 0; i < processor->signal_size; i++) {
        processor->signal_data[i] = (uint8_t)((i * 17 + 42) & 0xFF);
    }
    
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_I(TAG, "Signal generated successfully");
    
    return true;
}

void signal_processor_set_format(SignalProcessor* processor, SignalFormat format) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    processor->format = format;
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_D(TAG, "Format set to: %d", format);
}

SignalFormat signal_processor_get_format(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    SignalFormat format = processor->format;
    furi_mutex_release(processor->mutex);
    
    return format;
}

void signal_processor_set_protocol(SignalProcessor* processor, ProtocolType protocol) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    processor->protocol = protocol;
    furi_mutex_release(processor->mutex);
    
    FURI_LOG_D(TAG, "Protocol set to: %d", protocol);
}

ProtocolType signal_processor_get_protocol(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    ProtocolType protocol = processor->protocol;
    furi_mutex_release(processor->mutex);
    
    return protocol;
}

size_t signal_processor_get_data_size(SignalProcessor* processor) {
    furi_assert(processor);
    
    furi_mutex_acquire(processor->mutex, FuriWaitForever);
    size_t size = processor->signal_size;
    furi_mutex_release(processor->mutex);
    
    return size;
}

const uint8_t* signal_processor_get_data(SignalProcessor* processor) {
    furi_assert(processor);
    
    // WARNING: Caller must acquire mutex before calling this function
    // and release it after finished accessing the data:
    //   furi_mutex_acquire(processor->mutex, FuriWaitForever);
    //   const uint8_t* data = signal_processor_get_data(processor);
    //   // ... use data ...
    //   furi_mutex_release(processor->mutex);
    
    return processor->signal_data;
}
