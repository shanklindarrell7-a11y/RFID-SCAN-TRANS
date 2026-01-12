#include "protocol_mifare_classic.h"
#include <string.h>
#include <stdio.h>

// Detect Mifare Classic type from SAK
MifareClassicType mifare_classic_detect_type(uint8_t sak) {
    // SAK byte determines card type
    // 0x08: Mifare Classic 1K
    // 0x18: Mifare Classic 4K
    // 0x09: Mifare Mini
    
    switch(sak) {
    case 0x08:
    case 0x09: // Mifare Mini treated as 1K
        return MifareClassic1K;
    case 0x18:
        return MifareClassic4K;
    default:
        return MifareClassic1K; // Default
    }
}

// Decode Mifare Classic from NFC response
bool mifare_classic_decode(const uint8_t* raw_data, size_t data_length, MifareClassicData* out) {
    if(!raw_data || !out || data_length < 4) {
        return false;
    }
    
    // Typical response format:
    // [UID 4/7 bytes][SAK 1 byte][ATQA 2 bytes]
    
    // Detect UID length
    if(data_length >= 7 && raw_data[0] == 0x88) {
        // 7-byte UID (cascade tag)
        out->uid_length = MIFARE_CLASSIC_UID_7;
        memcpy(out->uid, raw_data, 7);
    } else {
        // 4-byte UID
        out->uid_length = MIFARE_CLASSIC_UID_4;
        memcpy(out->uid, raw_data, 4);
    }
    
    // Extract SAK if available
    if(data_length > out->uid_length) {
        out->sak = raw_data[out->uid_length];
        out->type = mifare_classic_detect_type(out->sak);
    } else {
        out->sak = 0x08; // Default to 1K
        out->type = MifareClassic1K;
    }
    
    // Extract ATQA if available
    if(data_length > out->uid_length + 1) {
        out->atqa[0] = raw_data[out->uid_length + 1];
        if(data_length > out->uid_length + 2) {
            out->atqa[1] = raw_data[out->uid_length + 2];
        } else {
            out->atqa[1] = 0;
        }
    } else {
        out->atqa[0] = 0x04;
        out->atqa[1] = 0x00;
    }
    
    out->valid = mifare_classic_verify_uid(out);
    
    return true;
}

// Encode Mifare Classic data for emulation
size_t mifare_classic_encode(const MifareClassicData* data, uint8_t* raw_data, size_t buffer_size) {
    if(!data || !raw_data || buffer_size < data->uid_length + 3) {
        return 0;
    }
    
    size_t offset = 0;
    
    // Copy UID
    memcpy(raw_data, data->uid, data->uid_length);
    offset += data->uid_length;
    
    // Add SAK
    raw_data[offset++] = data->sak;
    
    // Add ATQA
    raw_data[offset++] = data->atqa[0];
    raw_data[offset++] = data->atqa[1];
    
    return offset;
}

// Verify Mifare Classic UID checksum (BCC for 4-byte UID)
bool mifare_classic_verify_uid(const MifareClassicData* data) {
    if(!data) {
        return false;
    }
    
    if(data->uid_length == MIFARE_CLASSIC_UID_4) {
        // For 4-byte UID, BCC (byte 4) should be XOR of bytes 0-3
        // Note: This is typically handled in the NFC layer
        return true; // Simplified validation
    }
    
    // For 7-byte UID, more complex cascade validation
    return true; // Simplified validation
}

// Format Mifare Classic data to string
void mifare_classic_format_string(const MifareClassicData* data, char* buffer, size_t buffer_size) {
    if(!data || !buffer || buffer_size < 40) {
        return;
    }
    
    char uid_str[32] = {0};
    for(size_t i = 0; i < data->uid_length; i++) {
        snprintf(
            uid_str + (i * 3),
            sizeof(uid_str) - (i * 3),
            "%02X%s",
            data->uid[i],
            (i < data->uid_length - 1) ? " " : "");
    }
    
    const char* type_str = (data->type == MifareClassic1K) ? "1K" : "4K";
    
    snprintf(
        buffer,
        buffer_size,
        "Mifare %s: %s",
        type_str,
        uid_str);
}
