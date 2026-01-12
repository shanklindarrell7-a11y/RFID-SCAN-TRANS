#include "protocol_em4100.h"
#include <string.h>
#include <stdio.h>

// Decode EM4100 signal from raw data
bool em4100_decode(const uint8_t* raw_data, size_t data_length, EM4100Data* out) {
    if(!raw_data || !out || data_length < 5) {
        return false;
    }
    
    // EM4100 typically uses 5 bytes (40 bits of actual data)
    // Format: [Version 8 bits][ID 32 bits]
    
    out->version = raw_data[0];
    out->unique_id = ((uint32_t)raw_data[1] << 24) |
                     ((uint32_t)raw_data[2] << 16) |
                     ((uint32_t)raw_data[3] << 8) |
                     ((uint32_t)raw_data[4]);
    
    // In a full implementation, would verify Manchester encoding and parity
    out->valid = true;
    
    return true;
}

// Encode EM4100 data to raw bits
size_t em4100_encode(const EM4100Data* data, uint8_t* raw_data, size_t buffer_size) {
    if(!data || !raw_data || buffer_size < 5) {
        return 0;
    }
    
    raw_data[0] = data->version;
    raw_data[1] = (data->unique_id >> 24) & 0xFF;
    raw_data[2] = (data->unique_id >> 16) & 0xFF;
    raw_data[3] = (data->unique_id >> 8) & 0xFF;
    raw_data[4] = data->unique_id & 0xFF;
    
    return 5;
}

// Verify EM4100 parity bits
bool em4100_verify_parity(const uint8_t* raw_data, size_t data_length) {
    if(!raw_data || data_length < 5) {
        return false;
    }
    
    // Simplified parity check
    // In full implementation, would check row and column parity
    uint8_t parity = 0;
    for(size_t i = 0; i < data_length; i++) {
        parity ^= raw_data[i];
    }
    
    return (parity == 0); // Simplified check
}

// Format EM4100 data to string
void em4100_format_string(const EM4100Data* data, char* buffer, size_t buffer_size) {
    if(!data || !buffer || buffer_size < 20) {
        return;
    }
    
    snprintf(
        buffer,
        buffer_size,
        "EM4100: %02X%08lX",
        data->version,
        data->unique_id);
}
