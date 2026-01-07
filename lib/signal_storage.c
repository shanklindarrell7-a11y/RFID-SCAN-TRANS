#include "../rfid_scan_trans.h"
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/path.h>

// Protocol names
const char* rfid_protocol_to_string(RfidProtocol protocol) {
    switch(protocol) {
        case RfidProtocolEM4100:
            return "EM4100";
        case RfidProtocolHIDProx:
            return "HID Prox";
        case RfidProtocolIOProx:
            return "IO Prox";
        case RfidProtocolIndala:
            return "Indala";
        case RfidProtocolT5577:
            return "T5577";
        default:
            return "Unknown";
    }
}

// Detect protocol from UID
RfidProtocol rfid_detect_protocol(uint8_t* uid, uint8_t uid_len) {
    // Simple heuristic-based detection
    if(uid_len == 5) {
        // EM4100 typically uses 5 bytes
        return RfidProtocolEM4100;
    } else if(uid_len == 6 || uid_len == 8) {
        // HID Prox can use 6 or 8 bytes
        return RfidProtocolHIDProx;
    } else if(uid_len == 4) {
        // Indala uses 4 bytes
        return RfidProtocolIndala;
    }
    return RfidProtocolUnknown;
}

// Validate signal integrity
bool rfid_validate_signal(RfidSignal* signal) {
    if(!signal) return false;
    if(signal->uid_len == 0 || signal->uid_len > MAX_UID_SIZE) return false;
    if(signal->name[0] == '\0') return false;
    
    // Basic checksum validation for EM4100
    if(signal->protocol == RfidProtocolEM4100 && signal->uid_len == 5) {
        // EM4100 has parity bits we can validate
        // For now, just return true - full validation would require bit-level parsing
        return true;
    }
    
    return true;
}

// Generate variant signal (increment UID)
void rfid_generate_variant(RfidSignal* source, RfidSignal* dest, int32_t offset) {
    memcpy(dest, source, sizeof(RfidSignal));
    
    // Increment the UID by offset
    int32_t carry = offset;
    for(int i = source->uid_len - 1; i >= 0 && carry != 0; i--) {
        int32_t new_val = dest->uid[i] + carry;
        dest->uid[i] = new_val & 0xFF;
        carry = new_val >> 8;
    }
    
    // Update name
    snprintf(dest->name, MAX_SIGNAL_NAME, "%s_var%ld", source->name, offset);
    dest->timestamp = furi_get_tick();
}

// Save signal to file
bool rfid_signal_save(RfidApp* app, RfidSignal* signal) {
    furi_assert(app);
    furi_assert(signal);
    
    FuriString* path = furi_string_alloc();
    furi_string_printf(path, "%s/%s%s", RFID_APP_FOLDER, signal->name, RFID_APP_EXTENSION);
    
    Storage* storage = app->storage;
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    
    if(storage_file_open(file, furi_string_get_cstr(path), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        // Write signal data
        storage_file_write(file, signal, sizeof(RfidSignal));
        storage_file_close(file);
        success = true;
        FURI_LOG_I(TAG, "Saved signal: %s", signal->name);
    } else {
        FURI_LOG_E(TAG, "Failed to save signal: %s", signal->name);
    }
    
    storage_file_free(file);
    furi_string_free(path);
    
    return success;
}

// Load signal from file
bool rfid_signal_load(RfidApp* app, const char* name) {
    furi_assert(app);
    furi_assert(name);
    
    if(app->state->signal_count >= MAX_SIGNALS) {
        FURI_LOG_W(TAG, "Signal library full");
        return false;
    }
    
    FuriString* path = furi_string_alloc();
    furi_string_printf(path, "%s/%s%s", RFID_APP_FOLDER, name, RFID_APP_EXTENSION);
    
    Storage* storage = app->storage;
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    
    if(storage_file_open(file, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        RfidSignal* signal = &app->state->signals[app->state->signal_count];
        size_t bytes_read = storage_file_read(file, signal, sizeof(RfidSignal));
        
        if(bytes_read == sizeof(RfidSignal)) {
            app->state->signal_count++;
            success = true;
            FURI_LOG_I(TAG, "Loaded signal: %s", signal->name);
        }
        
        storage_file_close(file);
    }
    
    storage_file_free(file);
    furi_string_free(path);
    
    return success;
}

// Delete signal
bool rfid_signal_delete(RfidApp* app, uint32_t index) {
    furi_assert(app);
    
    if(index >= app->state->signal_count) return false;
    
    RfidSignal* signal = &app->state->signals[index];
    
    FuriString* path = furi_string_alloc();
    furi_string_printf(path, "%s/%s%s", RFID_APP_FOLDER, signal->name, RFID_APP_EXTENSION);
    
    // Delete file
    bool success = storage_common_remove(app->storage, furi_string_get_cstr(path));
    
    if(success) {
        // Remove from array
        for(uint32_t i = index; i < app->state->signal_count - 1; i++) {
            app->state->signals[i] = app->state->signals[i + 1];
        }
        app->state->signal_count--;
        FURI_LOG_I(TAG, "Deleted signal: %s", signal->name);
    }
    
    furi_string_free(path);
    
    return success;
}

// Load all signals from storage
void rfid_signal_load_all(RfidApp* app) {
    furi_assert(app);
    
    File* dir = storage_file_alloc(app->storage);
    
    if(storage_dir_open(dir, RFID_APP_FOLDER)) {
        FileInfo file_info;
        char name[256];
        
        while(storage_dir_read(dir, &file_info, name, sizeof(name))) {
            if(file_info_is_dir(&file_info)) continue;
            
            // Check if it's an RFID file
            FuriString* filename = furi_string_alloc_set(name);
            if(furi_string_end_with(filename, RFID_APP_EXTENSION)) {
                // Remove extension
                size_t ext_pos = furi_string_search_rchar(filename, '.');
                furi_string_left(filename, ext_pos);
                
                rfid_signal_load(app, furi_string_get_cstr(filename));
            }
            furi_string_free(filename);
            
            if(app->state->signal_count >= MAX_SIGNALS) break;
        }
        
        storage_dir_close(dir);
    }
    
    storage_file_free(dir);
    
    FURI_LOG_I(TAG, "Loaded %lu signals", app->state->signal_count);
}

// Backup all signals
void rfid_signal_backup_all(RfidApp* app) {
    furi_assert(app);
    
    for(uint32_t i = 0; i < app->state->signal_count; i++) {
        rfid_signal_save(app, &app->state->signals[i]);
    }
    
    FURI_LOG_I(TAG, "Backed up %lu signals", app->state->signal_count);
}
