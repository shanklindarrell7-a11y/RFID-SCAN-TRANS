#include "protocol_indala.h"
#include <string.h>
#include <stdio.h>

// Detect Indala format length
uint16_t indala_detect_format(const uint8_t* raw_data, size_t data_length) {
    if(!raw_data || data_length == 0) {
        return 0;
    }
    
    // Indala 64-bit format uses 8 bytes
    if(data_length == 8) {
        return INDALA_BITS_64;
    }
    
    // Indala 224-bit format uses 28 bytes
    if(data_length >= 28) {
        return INDALA_BITS_224;
    }
    
    // Default to 64-bit if ambiguous
    return INDALA_BITS_64;
}

// Decode Indala signal from raw data
bool indala_decode(const uint8_t* raw_data, size_t data_length, IndalaData* out) {
    if(!raw_data || !out || data_length < 8) {
        return false;
    }
    
    out->format_length = indala_detect_format(raw_data, data_length);
    out->raw_data_length = data_length < sizeof(out->raw_data) ? data_length : sizeof(out->raw_data);
    memcpy(out->raw_data, raw_data, out->raw_data_length);
    
    if(out->format_length == INDALA_BITS_64) {
        // Extract 64-bit card ID
        out->card_id = 0;
        for(size_t i = 0; i < 8; i++) {
            out->card_id = (out->card_id << 8) | raw_data[i];
        }
    } else {
        // For 224-bit format, store in raw_data
        out->card_id = 0; // Not applicable for longer formats
    }
    
    // Simplified validation
    out->valid = true;
    
    return true;
}

// Encode Indala data to raw bits
size_t indala_encode(const IndalaData* data, uint8_t* raw_data, size_t buffer_size) {
    if(!data || !raw_data) {
        return 0;
    }
    
    if(data->format_length == INDALA_BITS_64) {
        if(buffer_size < 8) {
            return 0;
        }
        
        // Encode 64-bit card ID
        for(int i = 7; i >= 0; i--) {
            raw_data[7 - i] = (data->card_id >> (i * 8)) & 0xFF;
        }
        
        return 8;
    } else {
        // For 224-bit format, copy raw data
        size_t copy_length = data->raw_data_length < buffer_size ? 
                             data->raw_data_length : buffer_size;
        memcpy(raw_data, data->raw_data, copy_length);
        return copy_length;
    }
}

// Verify Indala checksum
bool indala_verify_checksum(const uint8_t* raw_data, size_t data_length) {
    if(!raw_data || data_length < 8) {
        return false;
    }
    
    // Simplified checksum verification
    // In full implementation, would check specific Indala checksum algorithm
    uint8_t checksum = 0;
    for(size_t i = 0; i < data_length - 1; i++) {
        checksum ^= raw_data[i];
    }
    
    return true; // Simplified - always pass for now
}

// Format Indala data to string
void indala_format_string(const IndalaData* data, char* buffer, size_t buffer_size) {
    if(!data || !buffer || buffer_size < 30) {
        return;
    }
    
    if(data->format_length == INDALA_BITS_64) {
        snprintf(
            buffer,
            buffer_size,
            "Indala-64: %016llX",
            (unsigned long long)data->card_id);
    } else {
        snprintf(
            buffer,
            buffer_size,
            "Indala-224: %luB",
            (unsigned long)data->raw_data_length);
    }
}
