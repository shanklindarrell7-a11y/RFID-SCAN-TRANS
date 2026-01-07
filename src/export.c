#include "export.h"
#include <furi.h>
#include <string.h>

static bool export_signal_sub(const SignalData* signal, File* file) {
    // Flipper SubGHz format
    FuriString* str = furi_string_alloc();
    
    furi_string_printf(str, "Filetype: Flipper SubGHz RAW File\n");
    furi_string_cat_printf(str, "Version: 1\n");
    furi_string_cat_printf(str, "Frequency: %lu\n", signal->frequency);
    furi_string_cat_printf(str, "Preset: FuriHalSubGhzPresetOok650Async\n");
    furi_string_cat_printf(str, "Protocol: %s\n", signal->name);
    furi_string_cat_printf(str, "RAW_Data: ");
    
    storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    
    // Write hex data
    for(size_t i = 0; i < signal->data_size; i++) {
        furi_string_printf(str, "%02X ", signal->data[i]);
        storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    }
    
    furi_string_free(str);
    return true;
}

static bool export_signal_csv(const SignalData* signal, File* file) {
    FuriString* str = furi_string_alloc();
    
    furi_string_printf(
        str,
        "%s,%lu,%zu,",
        signal->name,
        signal->frequency,
        signal->data_size);
    
    storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    
    for(size_t i = 0; i < signal->data_size; i++) {
        furi_string_printf(str, "%02X", signal->data[i]);
        storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    }
    
    storage_file_write(file, "\n", 1);
    
    furi_string_free(str);
    return true;
}

static bool export_signal_json(const SignalData* signal, File* file) {
    FuriString* str = furi_string_alloc();
    
    furi_string_printf(str, "{\n");
    furi_string_cat_printf(str, "  \"protocol\": \"%s\",\n", signal->name);
    furi_string_cat_printf(str, "  \"frequency\": %lu,\n", signal->frequency);
    furi_string_cat_printf(str, "  \"data_size\": %zu,\n", signal->data_size);
    furi_string_cat_printf(str, "  \"data\": \"");
    
    storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    
    for(size_t i = 0; i < signal->data_size; i++) {
        furi_string_printf(str, "%02X", signal->data[i]);
        storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    }
    
    furi_string_printf(str, "\"\n}\n");
    storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    
    furi_string_free(str);
    return true;
}

static bool export_signal_hex(const SignalData* signal, File* file) {
    FuriString* str = furi_string_alloc();
    
    for(size_t i = 0; i < signal->data_size; i++) {
        if(i > 0 && i % 16 == 0) {
            storage_file_write(file, "\n", 1);
        }
        furi_string_printf(str, "%02X ", signal->data[i]);
        storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    }
    
    storage_file_write(file, "\n", 1);
    
    furi_string_free(str);
    return true;
}

static bool export_signal_bin(const SignalData* signal, File* file) {
    // Write binary data directly
    uint16_t bytes_written = storage_file_write(file, signal->data, signal->data_size);
    return (bytes_written == signal->data_size);
}

bool export_signal(
    const SignalData* signal,
    Storage* storage,
    const char* path,
    ExportFormat format) {
    if(!signal || !storage || !path) return false;
    
    File* file = storage_file_alloc(storage);
    
    if(!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open export file: %s", path);
        storage_file_free(file);
        return false;
    }
    
    bool result = false;
    
    switch(format) {
    case EXPORT_FORMAT_SUB:
        result = export_signal_sub(signal, file);
        break;
    case EXPORT_FORMAT_CSV:
        result = export_signal_csv(signal, file);
        break;
    case EXPORT_FORMAT_JSON:
        result = export_signal_json(signal, file);
        break;
    case EXPORT_FORMAT_HEX:
        result = export_signal_hex(signal, file);
        break;
    case EXPORT_FORMAT_BIN:
        result = export_signal_bin(signal, file);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown export format: %d", format);
        break;
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

bool export_library(
    const SignalLibrary* library,
    Storage* storage,
    const char* path,
    ExportFormat format) {
    if(!library || !storage || !path) return false;
    
    File* file = storage_file_alloc(storage);
    
    if(!storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open export file: %s", path);
        storage_file_free(file);
        return false;
    }
    
    // For JSON, write array start
    if(format == EXPORT_FORMAT_JSON) {
        storage_file_write(file, "[\n", 2);
    }
    
    // For CSV, write header
    if(format == EXPORT_FORMAT_CSV) {
        const char* header = "protocol,frequency,data_size,data\n";
        storage_file_write(file, header, strlen(header));
    }
    
    bool result = true;
    for(size_t i = 0; i < library->count; i++) {
        const SignalData* signal = &library->signals[i];
        
        switch(format) {
        case EXPORT_FORMAT_SUB:
            if(!export_signal_sub(signal, file)) result = false;
            storage_file_write(file, "\n", 1);
            break;
        case EXPORT_FORMAT_CSV:
            if(!export_signal_csv(signal, file)) result = false;
            break;
        case EXPORT_FORMAT_JSON:
            if(!export_signal_json(signal, file)) result = false;
            if(i < library->count - 1) {
                storage_file_write(file, ",\n", 2);
            }
            break;
        case EXPORT_FORMAT_HEX:
            if(!export_signal_hex(signal, file)) result = false;
            storage_file_write(file, "\n", 1);
            break;
        case EXPORT_FORMAT_BIN:
            if(!export_signal_bin(signal, file)) result = false;
            break;
        default:
            result = false;
            break;
        }
        
        if(!result) break;
    }
    
    // For JSON, write array end
    if(format == EXPORT_FORMAT_JSON) {
        storage_file_write(file, "]\n", 2);
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    return result;
}

const char* export_get_format_name(ExportFormat format) {
    switch(format) {
    case EXPORT_FORMAT_SUB:
        return "SubGHz";
    case EXPORT_FORMAT_CSV:
        return "CSV";
    case EXPORT_FORMAT_JSON:
        return "JSON";
    case EXPORT_FORMAT_HEX:
        return "Hexadecimal";
    case EXPORT_FORMAT_BIN:
        return "Binary";
    default:
        return "Unknown";
    }
}
