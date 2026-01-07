#include "export_formats.h"
#include "rfid_sniffer.h"
#include <furi_hal.h>
#include <stdarg.h>

#define TAG "ExportFormats"

// Helper function to write formatted string to file
static bool write_formatted(File* file, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if(written < 0 || written >= (int)sizeof(buffer)) {
        return false;
    }
    
    return storage_file_write(file, buffer, written) == (size_t)written;
}

// Convert signal data to hex string
static void data_to_hex_string(const uint8_t* data, size_t len, char* output, size_t output_size) {
    size_t pos = 0;
    for(size_t i = 0; i < len && pos < output_size - 3; i++) {
        pos += snprintf(output + pos, output_size - pos, "%02X", data[i]);
    }
}

// Get protocol name
static const char* get_protocol_name(RFIDProtocol protocol) {
    switch(protocol) {
        case RFIDProtocolEM4100: return "EM4100";
        case RFIDProtocolHIDProx: return "HID Prox";
        case RFIDProtocolIndala: return "Indala";
        case RFIDProtocolMifareClassic: return "Mifare Classic";
        case RFIDProtocolISO14443A: return "ISO14443A";
        default: return "Unknown";
    }
}

// Get protocol frequency
static uint32_t get_protocol_frequency(RFIDProtocol protocol) {
    switch(protocol) {
        case RFIDProtocolEM4100:
        case RFIDProtocolHIDProx:
        case RFIDProtocolIndala:
            return 125; // kHz
        case RFIDProtocolMifareClassic:
        case RFIDProtocolISO14443A:
            return 13560; // kHz (13.56 MHz)
        default:
            return 0;
    }
}

// Export to Flipper format (.sub file)
ExportResult export_to_flipper_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal) {
    
    ExportResult result = {.success = false, .bytes_written = 0};
    
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to open file for writing");
        storage_file_free(file);
        return result;
    }
    
    // Write Flipper SubGhz format header
    bool success = true;
    success = success && write_formatted(file, "Filetype: Flipper SubGhz RAW File\n");
    success = success && write_formatted(file, "Version: 1\n");
    success = success && write_formatted(file, "Frequency: %lu\n", 
                                         (unsigned long)get_protocol_frequency(protocol));
    success = success && write_formatted(file, "Preset: FuriHalSubGhzPresetOok650Async\n");
    success = success && write_formatted(file, "Protocol: %s\n", get_protocol_name(protocol));
    
    // Write signal data as hex
    char hex_data[256];
    data_to_hex_string(signal->data, signal->data_length, hex_data, sizeof(hex_data));
    success = success && write_formatted(file, "RAW_Data: %s\n", hex_data);
    
    // Write metadata
    success = success && write_formatted(file, "RSSI: %d\n", signal->rssi);
    success = success && write_formatted(file, "Timestamp: %lu\n", 
                                         (unsigned long)signal->timestamp);
    
    result.bytes_written = storage_file_tell(file);
    result.success = success;
    
    if(!success) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to write data to file");
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

// Export to CSV format
ExportResult export_to_csv_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal,
    bool append) {
    
    ExportResult result = {.success = false, .bytes_written = 0};
    
    File* file = storage_file_alloc(storage);
    
    // Check if file exists to determine if header is needed
    bool file_exists = storage_file_exists(storage, file_path);
    
    FsOpenMode mode = append ? FSOM_OPEN_APPEND : FSOM_CREATE_ALWAYS;
    if(!storage_file_open(file, file_path, FSAM_WRITE, mode)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to open file for writing");
        storage_file_free(file);
        return result;
    }
    
    bool success = true;
    
    // Write CSV header if file is new
    if(!file_exists || !append) {
        success = success && write_formatted(file, 
            "Timestamp,Protocol,Frequency_kHz,Data_Hex,Data_Length,RSSI_dBm\n");
    }
    
    // Convert signal data to hex string
    char hex_data[256];
    data_to_hex_string(signal->data, signal->data_length, hex_data, sizeof(hex_data));
    
    // Write CSV row
    success = success && write_formatted(file, "%lu,%s,%lu,%s,%zu,%d\n",
        (unsigned long)signal->timestamp,
        get_protocol_name(protocol),
        (unsigned long)get_protocol_frequency(protocol),
        hex_data,
        signal->data_length,
        signal->rssi);
    
    result.bytes_written = storage_file_tell(file);
    result.success = success;
    
    if(!success) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to write data to file");
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

// Export to Wiegand format
ExportResult export_to_wiegand_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal) {
    
    ExportResult result = {.success = false, .bytes_written = 0};
    
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to open file for writing");
        storage_file_free(file);
        return result;
    }
    
    bool success = true;
    
    // Write Wiegand format header
    success = success && write_formatted(file, "# Wiegand Format Export\n");
    success = success && write_formatted(file, "# Protocol: %s\n", get_protocol_name(protocol));
    success = success && write_formatted(file, "# Frequency: %lu kHz\n", 
                                         (unsigned long)get_protocol_frequency(protocol));
    success = success && write_formatted(file, "\n");
    
    // Determine Wiegand bit length (26, 34, or 37 bit are common)
    size_t bit_count = signal->data_length * 8;
    
    // For protocols like HID Prox, extract facility code and card number
    if(protocol == RFIDProtocolHIDProx && signal->data_length >= 4) {
        // Standard 26-bit Wiegand format
        uint32_t card_data = 0;
        for(size_t i = 0; i < 4 && i < signal->data_length; i++) {
            card_data = (card_data << 8) | signal->data[i];
        }
        
        uint8_t facility_code = (card_data >> 17) & 0xFF;
        uint16_t card_number = (card_data >> 1) & 0xFFFF;
        
        success = success && write_formatted(file, "Format: 26-bit\n");
        success = success && write_formatted(file, "Facility Code: %u\n", facility_code);
        success = success && write_formatted(file, "Card Number: %u\n", card_number);
    } else {
        // Generic bit string format
        success = success && write_formatted(file, "Bit Length: %zu\n", bit_count);
        
        // Write binary representation
        success = success && write_formatted(file, "Binary: ");
        for(size_t i = 0; i < signal->data_length; i++) {
            for(int bit = 7; bit >= 0; bit--) {
                char bit_char = (signal->data[i] & (1 << bit)) ? '1' : '0';
                success = success && write_formatted(file, "%c", bit_char);
            }
        }
        success = success && write_formatted(file, "\n");
    }
    
    // Write hex representation
    char hex_data[256];
    data_to_hex_string(signal->data, signal->data_length, hex_data, sizeof(hex_data));
    success = success && write_formatted(file, "Hex: %s\n", hex_data);
    
    result.bytes_written = storage_file_tell(file);
    result.success = success;
    
    if(!success) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to write data to file");
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

// Export to Proxmark3 format
ExportResult export_to_proxmark3_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal) {
    
    ExportResult result = {.success = false, .bytes_written = 0};
    
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to open file for writing");
        storage_file_free(file);
        return result;
    }
    
    bool success = true;
    
    // Write script header
    success = success && write_formatted(file, "# Proxmark3 Command Script\n");
    success = success && write_formatted(file, "# Protocol: %s\n", get_protocol_name(protocol));
    success = success && write_formatted(file, "# Generated by Flipper Zero RFID Sniffer\n");
    success = success && write_formatted(file, "\n");
    
    // Convert data to hex string
    char hex_data[256];
    data_to_hex_string(signal->data, signal->data_length, hex_data, sizeof(hex_data));
    
    // Generate protocol-specific commands
    switch(protocol) {
        case RFIDProtocolEM4100:
            success = success && write_formatted(file, "# EM4100 Read/Clone Commands\n");
            success = success && write_formatted(file, "lf em 410x_read\n");
            success = success && write_formatted(file, "lf em 410x_sim %s\n", hex_data);
            break;
            
        case RFIDProtocolHIDProx:
            success = success && write_formatted(file, "# HID Prox Read/Clone Commands\n");
            success = success && write_formatted(file, "lf hid read\n");
            success = success && write_formatted(file, "lf hid sim %s\n", hex_data);
            break;
            
        case RFIDProtocolIndala:
            success = success && write_formatted(file, "# Indala Read/Clone Commands\n");
            success = success && write_formatted(file, "lf indala read\n");
            success = success && write_formatted(file, "lf indala sim %s\n", hex_data);
            break;
            
        case RFIDProtocolMifareClassic:
            success = success && write_formatted(file, "# Mifare Classic Commands\n");
            success = success && write_formatted(file, "hf mf rdbl 0 A FFFFFFFFFFFF\n");
            success = success && write_formatted(file, "hf mf restore %s\n", hex_data);
            break;
            
        case RFIDProtocolISO14443A:
            success = success && write_formatted(file, "# ISO14443A Commands\n");
            success = success && write_formatted(file, "hf 14a read\n");
            success = success && write_formatted(file, "hf 14a sim t 1 u %s\n", hex_data);
            break;
            
        default:
            success = success && write_formatted(file, "# Generic commands\n");
            success = success && write_formatted(file, "# Data: %s\n", hex_data);
            break;
    }
    
    success = success && write_formatted(file, "\n# Raw data: %s\n", hex_data);
    
    result.bytes_written = storage_file_tell(file);
    result.success = success;
    
    if(!success) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to write data to file");
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

// Export to JSON format
ExportResult export_to_json_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal) {
    
    ExportResult result = {.success = false, .bytes_written = 0};
    
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to open file for writing");
        storage_file_free(file);
        return result;
    }
    
    bool success = true;
    
    // Convert data to hex string
    char hex_data[256];
    data_to_hex_string(signal->data, signal->data_length, hex_data, sizeof(hex_data));
    
    // Write JSON structure
    success = success && write_formatted(file, "{\n");
    success = success && write_formatted(file, "  \"protocol\": \"%s\",\n", 
                                         get_protocol_name(protocol));
    success = success && write_formatted(file, "  \"frequency_khz\": %lu,\n", 
                                         (unsigned long)get_protocol_frequency(protocol));
    success = success && write_formatted(file, "  \"timestamp\": %lu,\n", 
                                         (unsigned long)signal->timestamp);
    success = success && write_formatted(file, "  \"rssi_dbm\": %d,\n", signal->rssi);
    success = success && write_formatted(file, "  \"data\": {\n");
    success = success && write_formatted(file, "    \"hex\": \"%s\",\n", hex_data);
    success = success && write_formatted(file, "    \"length\": %zu,\n", signal->data_length);
    
    // Write individual bytes as array
    success = success && write_formatted(file, "    \"bytes\": [");
    for(size_t i = 0; i < signal->data_length; i++) {
        if(i > 0) {
            success = success && write_formatted(file, ", ");
        }
        success = success && write_formatted(file, "%u", signal->data[i]);
    }
    success = success && write_formatted(file, "]\n");
    
    success = success && write_formatted(file, "  },\n");
    success = success && write_formatted(file, "  \"metadata\": {\n");
    success = success && write_formatted(file, "    \"source\": \"Flipper Zero RFID Sniffer\",\n");
    success = success && write_formatted(file, "    \"version\": \"1.0\"\n");
    success = success && write_formatted(file, "  }\n");
    success = success && write_formatted(file, "}\n");
    
    result.bytes_written = storage_file_tell(file);
    result.success = success;
    
    if(!success) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to write data to file");
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

// Get file extension for format
const char* export_format_get_extension(ExportFormat format) {
    switch(format) {
        case ExportFormatFlipper: return ".sub";
        case ExportFormatCSV: return ".csv";
        case ExportFormatWiegand: return ".wiegand";
        case ExportFormatProxmark3: return ".pm3";
        case ExportFormatJSON: return ".json";
        default: return ".txt";
    }
}

// Get format name
const char* export_format_get_name(ExportFormat format) {
    switch(format) {
        case ExportFormatFlipper: return "Flipper Format";
        case ExportFormatCSV: return "CSV Database";
        case ExportFormatWiegand: return "Wiegand Format";
        case ExportFormatProxmark3: return "Proxmark3 Script";
        case ExportFormatJSON: return "JSON Metadata";
        default: return "Unknown";
    }
}
