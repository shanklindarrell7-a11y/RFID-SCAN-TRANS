#include "../rfid_scan_trans.h"
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>

// Export to Flipper .sub format
bool rfid_export_flipper(RfidApp* app, RfidSignal* signal, const char* path) {
    furi_assert(app);
    furi_assert(signal);
    furi_assert(path);
    
    File* file = storage_file_alloc(app->storage);
    
    bool success = false;
    
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* content = furi_string_alloc();
        
        // Flipper SubGHz format header
        furi_string_cat_printf(content, "Filetype: Flipper SubGHz RAW File\n");
        furi_string_cat_printf(content, "Version: 1\n");
        furi_string_cat_printf(content, "Frequency: %lu\n", signal->frequency ? signal->frequency : 125000);
        furi_string_cat_printf(content, "Preset: FuriHalSubGhzPresetOok650Async\n");
        furi_string_cat_printf(content, "Protocol: %s\n", signal->protocol_name);
        
        // Write UID as hex
        furi_string_cat_printf(content, "Key: ");
        for(uint8_t i = 0; i < signal->uid_len; i++) {
            furi_string_cat_printf(content, "%02X ", signal->uid[i]);
        }
        furi_string_cat_printf(content, "\n");
        
        storage_file_write(file, furi_string_get_cstr(content), furi_string_size(content));
        storage_file_close(file);
        success = true;
        
        furi_string_free(content);
        FURI_LOG_I(TAG, "Exported to Flipper format: %s", path);
    }
    
    storage_file_free(file);
    
    return success;
}

// Export all signals to CSV
bool rfid_export_csv(RfidApp* app, const char* path) {
    furi_assert(app);
    furi_assert(path);
    
    File* file = storage_file_alloc(app->storage);
    
    bool success = false;
    
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* content = furi_string_alloc();
        
        // CSV header
        furi_string_cat_printf(content, "Name,Protocol,UID,Frequency,Signal Strength,Favorite,Timestamp,Notes\n");
        
        // Export each signal
        for(uint32_t i = 0; i < app->state->signal_count; i++) {
            RfidSignal* sig = &app->state->signals[i];
            
            furi_string_cat_printf(content, "\"%s\",\"%s\",\"", sig->name, sig->protocol_name);
            
            // UID as hex string
            for(uint8_t j = 0; j < sig->uid_len; j++) {
                furi_string_cat_printf(content, "%02X", sig->uid[j]);
            }
            
            furi_string_cat_printf(
                content,
                "\",%lu,%u,%s,%lu,\"%s\"\n",
                sig->frequency,
                sig->signal_strength,
                sig->is_favorite ? "Yes" : "No",
                sig->timestamp,
                sig->notes
            );
        }
        
        storage_file_write(file, furi_string_get_cstr(content), furi_string_size(content));
        storage_file_close(file);
        success = true;
        
        furi_string_free(content);
        FURI_LOG_I(TAG, "Exported %lu signals to CSV: %s", app->state->signal_count, path);
    }
    
    storage_file_free(file);
    
    return success;
}

// Export to Wiegand format
bool rfid_export_wiegand(RfidApp* app, RfidSignal* signal, const char* path) {
    furi_assert(app);
    furi_assert(signal);
    furi_assert(path);
    
    File* file = storage_file_alloc(app->storage);
    
    bool success = false;
    
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* content = furi_string_alloc();
        
        // Wiegand format (industry standard for access control)
        furi_string_cat_printf(content, "# Wiegand Format Export\n");
        furi_string_cat_printf(content, "# Card Name: %s\n", signal->name);
        furi_string_cat_printf(content, "# Protocol: %s\n", signal->protocol_name);
        furi_string_cat_printf(content, "\n");
        
        // Calculate Wiegand 26-bit format if applicable
        if(signal->uid_len >= 3) {
            // Standard 26-bit Wiegand: Facility Code (8 bits) + Card Number (16 bits) + Parity (2 bits)
            uint8_t facility_code = signal->uid[0];
            uint16_t card_number = (signal->uid[1] << 8) | signal->uid[2];
            
            furi_string_cat_printf(content, "Format: Wiegand 26-bit\n");
            furi_string_cat_printf(content, "Facility Code: %u\n", facility_code);
            furi_string_cat_printf(content, "Card Number: %u\n", card_number);
            furi_string_cat_printf(content, "\n");
        }
        
        // Raw hex data
        furi_string_cat_printf(content, "Raw Hex: ");
        for(uint8_t i = 0; i < signal->uid_len; i++) {
            furi_string_cat_printf(content, "%02X", signal->uid[i]);
        }
        furi_string_cat_printf(content, "\n");
        
        // Binary representation
        furi_string_cat_printf(content, "Binary: ");
        for(uint8_t i = 0; i < signal->uid_len; i++) {
            for(int8_t bit = 7; bit >= 0; bit--) {
                furi_string_cat_printf(content, "%d", (signal->uid[i] >> bit) & 1);
            }
        }
        furi_string_cat_printf(content, "\n");
        
        storage_file_write(file, furi_string_get_cstr(content), furi_string_size(content));
        storage_file_close(file);
        success = true;
        
        furi_string_free(content);
        FURI_LOG_I(TAG, "Exported to Wiegand format: %s", path);
    }
    
    storage_file_free(file);
    
    return success;
}

// Export to Proxmark3 command format
bool rfid_export_proxmark3(RfidApp* app, RfidSignal* signal, const char* path) {
    furi_assert(app);
    furi_assert(signal);
    furi_assert(path);
    
    File* file = storage_file_alloc(app->storage);
    
    bool success = false;
    
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* content = furi_string_alloc();
        
        // Proxmark3 script header
        furi_string_cat_printf(content, "# Proxmark3 Commands for %s\n", signal->name);
        furi_string_cat_printf(content, "# Protocol: %s\n", signal->protocol_name);
        furi_string_cat_printf(content, "# Generated by RFID Scan & Trans\n");
        furi_string_cat_printf(content, "\n");
        
        // Generate appropriate command based on protocol
        switch(signal->protocol) {
            case RfidProtocolEM4100:
                furi_string_cat_printf(content, "# Read EM4100 card\n");
                furi_string_cat_printf(content, "lf em 410x_read\n\n");
                furi_string_cat_printf(content, "# Clone to T55xx\n");
                furi_string_cat_printf(content, "lf em 410x_clone ");
                for(uint8_t i = 0; i < signal->uid_len; i++) {
                    furi_string_cat_printf(content, "%02x", signal->uid[i]);
                }
                furi_string_cat_printf(content, "\n");
                break;
                
            case RfidProtocolHIDProx:
                furi_string_cat_printf(content, "# Read HID Prox card\n");
                furi_string_cat_printf(content, "lf hid read\n\n");
                furi_string_cat_printf(content, "# Clone HID Prox\n");
                furi_string_cat_printf(content, "lf hid clone ");
                for(uint8_t i = 0; i < signal->uid_len; i++) {
                    furi_string_cat_printf(content, "%02x", signal->uid[i]);
                }
                furi_string_cat_printf(content, "\n");
                break;
                
            case RfidProtocolIndala:
                furi_string_cat_printf(content, "# Read Indala card\n");
                furi_string_cat_printf(content, "lf indala read\n\n");
                furi_string_cat_printf(content, "# Clone Indala\n");
                furi_string_cat_printf(content, "lf indala clone ");
                for(uint8_t i = 0; i < signal->uid_len; i++) {
                    furi_string_cat_printf(content, "%02x", signal->uid[i]);
                }
                furi_string_cat_printf(content, "\n");
                break;
                
            default:
                furi_string_cat_printf(content, "# Generic LF read\n");
                furi_string_cat_printf(content, "lf search\n");
                break;
        }
        
        storage_file_write(file, furi_string_get_cstr(content), furi_string_size(content));
        storage_file_close(file);
        success = true;
        
        furi_string_free(content);
        FURI_LOG_I(TAG, "Exported to Proxmark3 format: %s", path);
    }
    
    storage_file_free(file);
    
    return success;
}

// Export to JSON format
bool rfid_export_json(RfidApp* app, RfidSignal* signal, const char* path) {
    furi_assert(app);
    furi_assert(signal);
    furi_assert(path);
    
    File* file = storage_file_alloc(app->storage);
    
    bool success = false;
    
    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* content = furi_string_alloc();
        
        furi_string_cat_printf(content, "{\n");
        furi_string_cat_printf(content, "  \"name\": \"%s\",\n", signal->name);
        furi_string_cat_printf(content, "  \"protocol\": \"%s\",\n", signal->protocol_name);
        furi_string_cat_printf(content, "  \"protocol_id\": %d,\n", signal->protocol);
        
        // UID as hex array
        furi_string_cat_printf(content, "  \"uid\": [");
        for(uint8_t i = 0; i < signal->uid_len; i++) {
            if(i > 0) furi_string_cat_printf(content, ", ");
            furi_string_cat_printf(content, "\"0x%02X\"", signal->uid[i]);
        }
        furi_string_cat_printf(content, "],\n");
        
        furi_string_cat_printf(content, "  \"uid_length\": %u,\n", signal->uid_len);
        furi_string_cat_printf(content, "  \"frequency\": %lu,\n", signal->frequency);
        furi_string_cat_printf(content, "  \"signal_strength\": %u,\n", signal->signal_strength);
        furi_string_cat_printf(content, "  \"is_favorite\": %s,\n", signal->is_favorite ? "true" : "false");
        furi_string_cat_printf(content, "  \"timestamp\": %lu,\n", signal->timestamp);
        furi_string_cat_printf(content, "  \"encrypted\": %s,\n", signal->encrypted ? "true" : "false");
        furi_string_cat_printf(content, "  \"notes\": \"%s\"\n", signal->notes);
        furi_string_cat_printf(content, "}\n");
        
        storage_file_write(file, furi_string_get_cstr(content), furi_string_size(content));
        storage_file_close(file);
        success = true;
        
        furi_string_free(content);
        FURI_LOG_I(TAG, "Exported to JSON format: %s", path);
    }
    
    storage_file_free(file);
    
    return success;
}
