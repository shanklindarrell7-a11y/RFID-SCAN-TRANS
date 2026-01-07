#include "library.h"
#include <furi.h>
#include <string.h>

bool library_init(SignalLibrary* library) {
    if(!library) return false;
    
    library->capacity = MAX_LIBRARY_SIGNALS;
    library->count = 0;
    library->signals = malloc(sizeof(SignalData) * library->capacity);
    
    if(!library->signals) {
        return false;
    }
    
    memset(library->signals, 0, sizeof(SignalData) * library->capacity);
    return true;
}

void library_free(SignalLibrary* library) {
    if(!library) return;
    
    if(library->signals) {
        free(library->signals);
        library->signals = NULL;
    }
    
    library->count = 0;
    library->capacity = 0;
}

bool library_add_signal(SignalLibrary* library, const SignalData* signal) {
    if(!library || !signal) return false;
    
    if(library->count >= library->capacity) {
        FURI_LOG_W(TAG, "Library full, cannot add signal");
        return false;
    }
    
    memcpy(&library->signals[library->count], signal, sizeof(SignalData));
    library->count++;
    
    return true;
}

bool library_load(SignalLibrary* library, Storage* storage) {
    if(!library || !storage) return false;
    
    File* file = storage_file_alloc(storage);
    
    if(!storage_file_open(file, LIBRARY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_I(TAG, "Library file does not exist, starting with empty library");
        storage_file_free(file);
        return true; // Not an error, just empty library
    }
    
    // Read signals from file
    library->count = 0;
    while(library->count < library->capacity) {
        SignalData signal;
        uint16_t bytes_read = storage_file_read(file, &signal, sizeof(SignalData));
        
        if(bytes_read != sizeof(SignalData)) {
            break; // End of file or read error
        }
        
        memcpy(&library->signals[library->count], &signal, sizeof(SignalData));
        library->count++;
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    FURI_LOG_I(TAG, "Loaded %zu signals from library", library->count);
    return true;
}

bool library_save(const SignalLibrary* library, Storage* storage) {
    if(!library || !storage) return false;
    
    File* file = storage_file_alloc(storage);
    
    if(!storage_file_open(file, LIBRARY_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Failed to open library file for writing");
        storage_file_free(file);
        return false;
    }
    
    // Write all signals
    for(size_t i = 0; i < library->count; i++) {
        uint16_t bytes_written = 
            storage_file_write(file, &library->signals[i], sizeof(SignalData));
        
        if(bytes_written != sizeof(SignalData)) {
            FURI_LOG_E(TAG, "Failed to write signal %zu", i);
            storage_file_close(file);
            storage_file_free(file);
            return false;
        }
    }
    
    storage_file_close(file);
    storage_file_free(file);
    
    FURI_LOG_I(TAG, "Saved %zu signals to library", library->count);
    return true;
}

const SignalData* library_get_signal(const SignalLibrary* library, size_t index) {
    if(!library || index >= library->count) return NULL;
    return &library->signals[index];
}

size_t library_get_count(const SignalLibrary* library) {
    if(!library) return 0;
    return library->count;
}
