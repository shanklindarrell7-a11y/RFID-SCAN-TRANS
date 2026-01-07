#include "export_formats.h"
#include "rfid_sniffer.h"
#include <furi.h>
#include <storage/storage.h>

#define TAG "ExportExample"

/**
 * Example: Export a captured signal to all supported formats
 */
void export_signal_all_formats(Storage* storage, const SignalData* signal, RFIDProtocol protocol) {
    // Create export directory if it doesn't exist
    storage_common_mkdir(storage, "/ext/rfid_exports");
    
    // Generate base filename with timestamp
    char base_path[128];
    snprintf(base_path, sizeof(base_path), "/ext/rfid_exports/signal_%lu", 
             (unsigned long)signal->timestamp);
    
    // Export to Flipper format
    {
        char file_path[128];
        snprintf(file_path, sizeof(file_path), "%s.sub", base_path);
        
        ExportResult result = export_to_flipper_format(storage, file_path, protocol, signal);
        if(result.success) {
            FURI_LOG_I(TAG, "Flipper export: %zu bytes written to %s", 
                       result.bytes_written, file_path);
        } else {
            FURI_LOG_E(TAG, "Flipper export failed: %s", result.error_message);
        }
    }
    
    // Export to CSV format (append to daily log)
    {
        FuriHalRtcDateTime datetime;
        furi_hal_rtc_get_datetime(&datetime);
        
        char file_path[128];
        snprintf(file_path, sizeof(file_path), 
                 "/ext/rfid_exports/log_%04d%02d%02d.csv",
                 datetime.year, datetime.month, datetime.day);
        
        // Append to existing log file
        ExportResult result = export_to_csv_format(storage, file_path, protocol, signal, true);
        if(result.success) {
            FURI_LOG_I(TAG, "CSV export: appended to %s", file_path);
        } else {
            FURI_LOG_E(TAG, "CSV export failed: %s", result.error_message);
        }
    }
    
    // Export to Wiegand format
    {
        char file_path[128];
        snprintf(file_path, sizeof(file_path), "%s.wiegand", base_path);
        
        ExportResult result = export_to_wiegand_format(storage, file_path, protocol, signal);
        if(result.success) {
            FURI_LOG_I(TAG, "Wiegand export: %zu bytes written to %s", 
                       result.bytes_written, file_path);
        } else {
            FURI_LOG_E(TAG, "Wiegand export failed: %s", result.error_message);
        }
    }
    
    // Export to Proxmark3 format
    {
        char file_path[128];
        snprintf(file_path, sizeof(file_path), "%s.pm3", base_path);
        
        ExportResult result = export_to_proxmark3_format(storage, file_path, protocol, signal);
        if(result.success) {
            FURI_LOG_I(TAG, "Proxmark3 export: %zu bytes written to %s", 
                       result.bytes_written, file_path);
        } else {
            FURI_LOG_E(TAG, "Proxmark3 export failed: %s", result.error_message);
        }
    }
    
    // Export to JSON format
    {
        char file_path[128];
        snprintf(file_path, sizeof(file_path), "%s.json", base_path);
        
        ExportResult result = export_to_json_format(storage, file_path, protocol, signal);
        if(result.success) {
            FURI_LOG_I(TAG, "JSON export: %zu bytes written to %s", 
                       result.bytes_written, file_path);
        } else {
            FURI_LOG_E(TAG, "JSON export failed: %s", result.error_message);
        }
    }
}

/**
 * Example: Export single signal to specific format
 */
bool export_signal_format(
    Storage* storage,
    const SignalData* signal,
    RFIDProtocol protocol,
    ExportFormat format,
    const char* custom_path) {
    
    char file_path[256];
    
    // Generate file path if not provided
    if(custom_path) {
        strncpy(file_path, custom_path, sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    } else {
        // Auto-generate path with timestamp and format
        storage_common_mkdir(storage, "/ext/rfid_exports");
        snprintf(file_path, sizeof(file_path), "/ext/rfid_exports/signal_%lu%s",
                 (unsigned long)signal->timestamp,
                 export_format_get_extension(format));
    }
    
    ExportResult result;
    
    switch(format) {
        case ExportFormatFlipper:
            result = export_to_flipper_format(storage, file_path, protocol, signal);
            break;
        case ExportFormatCSV:
            result = export_to_csv_format(storage, file_path, protocol, signal, false);
            break;
        case ExportFormatWiegand:
            result = export_to_wiegand_format(storage, file_path, protocol, signal);
            break;
        case ExportFormatProxmark3:
            result = export_to_proxmark3_format(storage, file_path, protocol, signal);
            break;
        case ExportFormatJSON:
            result = export_to_json_format(storage, file_path, protocol, signal);
            break;
        default:
            FURI_LOG_E(TAG, "Unknown export format");
            return false;
    }
    
    if(result.success) {
        FURI_LOG_I(TAG, "Exported %zu bytes to %s", result.bytes_written, file_path);
    } else {
        FURI_LOG_E(TAG, "Export failed: %s", result.error_message);
    }
    
    return result.success;
}

/**
 * Example: Batch export multiple signals to CSV
 */
void export_batch_to_csv(
    Storage* storage,
    const SignalData* signals,
    size_t signal_count,
    RFIDProtocol protocol,
    const char* file_path) {
    
    FURI_LOG_I(TAG, "Starting batch export of %zu signals", signal_count);
    
    size_t success_count = 0;
    size_t total_bytes = 0;
    
    for(size_t i = 0; i < signal_count; i++) {
        bool append = (i > 0); // First one creates file, rest append
        ExportResult result = export_to_csv_format(
            storage, file_path, protocol, &signals[i], append);
        
        if(result.success) {
            success_count++;
            total_bytes += result.bytes_written;
        } else {
            FURI_LOG_W(TAG, "Failed to export signal %zu: %s", 
                       i, result.error_message);
        }
    }
    
    FURI_LOG_I(TAG, "Batch export complete: %zu/%zu signals, %zu total bytes",
               success_count, signal_count, total_bytes);
}

/**
 * Example: Create sample signal data for testing
 */
SignalData create_sample_signal(RFIDProtocol protocol) {
    SignalData signal;
    memset(&signal, 0, sizeof(signal));
    
    // Get current timestamp
    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    signal.timestamp = furi_hal_rtc_datetime_to_timestamp(&datetime);
    
    // Set sample RSSI
    signal.rssi = -45;
    
    // Generate protocol-specific sample data
    switch(protocol) {
        case RFIDProtocolEM4100:
            // EM4100: 5 bytes (40 bits)
            signal.data[0] = 0xA1;
            signal.data[1] = 0xB2;
            signal.data[2] = 0xC3;
            signal.data[3] = 0xD4;
            signal.data[4] = 0xE5;
            signal.data_length = 5;
            break;
            
        case RFIDProtocolHIDProx:
            // HID Prox: 4 bytes (26-bit Wiegand typically)
            signal.data[0] = 0x12;
            signal.data[1] = 0x34;
            signal.data[2] = 0x56;
            signal.data[3] = 0x78;
            signal.data_length = 4;
            break;
            
        case RFIDProtocolIndala:
            // Indala: 8 bytes (64 bits)
            for(size_t i = 0; i < 8; i++) {
                signal.data[i] = 0x10 + i;
            }
            signal.data_length = 8;
            break;
            
        case RFIDProtocolMifareClassic:
            // Mifare Classic: 4 byte UID
            signal.data[0] = 0xDE;
            signal.data[1] = 0xAD;
            signal.data[2] = 0xBE;
            signal.data[3] = 0xEF;
            signal.data_length = 4;
            break;
            
        case RFIDProtocolISO14443A:
            // ISO14443A: 4 or 7 byte UID
            signal.data[0] = 0x04;
            signal.data[1] = 0x12;
            signal.data[2] = 0x34;
            signal.data[3] = 0x56;
            signal.data[4] = 0x78;
            signal.data[5] = 0x9A;
            signal.data[6] = 0xBC;
            signal.data_length = 7;
            break;
            
        default:
            // Generic data
            signal.data[0] = 0xFF;
            signal.data_length = 1;
            break;
    }
    
    return signal;
}

/**
 * Example: Test all export formats
 */
void test_export_formats(Storage* storage) {
    FURI_LOG_I(TAG, "Testing all export formats...");
    
    // Test each protocol
    RFIDProtocol protocols[] = {
        RFIDProtocolEM4100,
        RFIDProtocolHIDProx,
        RFIDProtocolIndala,
        RFIDProtocolMifareClassic,
        RFIDProtocolISO14443A
    };
    
    for(size_t i = 0; i < sizeof(protocols) / sizeof(protocols[0]); i++) {
        RFIDProtocol protocol = protocols[i];
        SignalData signal = create_sample_signal(protocol);
        
        FURI_LOG_I(TAG, "Testing protocol: %d", protocol);
        export_signal_all_formats(storage, &signal, protocol);
    }
    
    FURI_LOG_I(TAG, "Export format testing complete");
}
