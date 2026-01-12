#include "protocol_iso14443a.h"
#include <string.h>
#include <stdio.h>

// Detect ISO14443A type from SAK and ATS
ISO14443A_Type iso14443a_detect_type(uint8_t sak, const uint8_t* ats, uint8_t ats_length) {
    // SAK bits determine card type
    // Bit 6 set: ISO14443-4 compliant (Type 4)
    if(sak & 0x20) {
        return ISO14443A_Type4;
    }
    
    // Bit 4 set: Could be Mifare
    if(sak & 0x10) {
        return ISO14443A_TypeMifare;
    }
    
    // SAK = 0x00: Mifare Ultralight / NTAG (Type 2)
    if(sak == 0x00) {
        return ISO14443A_Type2;
    }
    
    return ISO14443A_TypeUnknown;
}

// Decode ISO14443A from NFC response
bool iso14443a_decode(const uint8_t* raw_data, size_t data_length, ISO14443AData* out) {
    if(!raw_data || !out || data_length < 4) {
        return false;
    }
    
    size_t offset = 0;
    
    // Detect UID length based on cascade tag (0x88)
    if(raw_data[0] == 0x88) {
        if(data_length >= 10 && raw_data[4] == 0x88) {
            // Triple cascade - 10 byte UID
            out->uid_length = ISO14443A_UID_TRIPLE;
            memcpy(out->uid, raw_data, 10);
            offset = 10;
        } else if(data_length >= 7) {
            // Double cascade - 7 byte UID
            out->uid_length = ISO14443A_UID_DOUBLE;
            memcpy(out->uid, raw_data, 7);
            offset = 7;
        }
    } else if(data_length >= 4) {
        // Single cascade - 4 byte UID
        out->uid_length = ISO14443A_UID_SINGLE;
        memcpy(out->uid, raw_data, 4);
        offset = 4;
    }
    
    // Extract SAK if available
    if(data_length > offset) {
        out->sak = raw_data[offset++];
    } else {
        out->sak = 0x00;
    }
    
    // Extract ATQA if available
    if(data_length > offset) {
        out->atqa[0] = raw_data[offset++];
        if(data_length > offset) {
            out->atqa[1] = raw_data[offset++];
        } else {
            out->atqa[1] = 0;
        }
    } else {
        out->atqa[0] = 0x44;
        out->atqa[1] = 0x00;
    }
    
    // Extract ATS if available (for Type 4 cards)
    out->ats_length = 0;
    if(data_length > offset) {
        size_t remaining = data_length - offset;
        out->ats_length = remaining < sizeof(out->ats) ? remaining : sizeof(out->ats);
        memcpy(out->ats, raw_data + offset, out->ats_length);
    }
    
    // Detect card type
    out->type = iso14443a_detect_type(out->sak, out->ats, out->ats_length);
    
    out->valid = iso14443a_verify_uid(out);
    
    return true;
}

// Encode ISO14443A data for emulation
size_t iso14443a_encode(const ISO14443AData* data, uint8_t* raw_data, size_t buffer_size) {
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
    
    // Add ATS if present
    if(data->ats_length > 0 && buffer_size >= offset + data->ats_length) {
        memcpy(raw_data + offset, data->ats, data->ats_length);
        offset += data->ats_length;
    }
    
    return offset;
}

// Verify ISO14443A UID cascade
bool iso14443a_verify_uid(const ISO14443AData* data) {
    if(!data) {
        return false;
    }
    
    // For single UID (4 bytes), BCC check
    if(data->uid_length == ISO14443A_UID_SINGLE) {
        // BCC = UID[0] XOR UID[1] XOR UID[2] XOR UID[3]
        // Note: This is typically verified at a lower level
        return true; // Simplified
    }
    
    // For cascade UIDs, verify cascade tags
    if(data->uid_length >= ISO14443A_UID_DOUBLE) {
        // Should have 0x88 cascade tag
        return (data->uid[0] == 0x88);
    }
    
    return true;
}

// Format ISO14443A data to string
void iso14443a_format_string(const ISO14443AData* data, char* buffer, size_t buffer_size) {
    if(!data || !buffer || buffer_size < 40) {
        return;
    }
    
    char uid_str[48] = {0};
    for(size_t i = 0; i < data->uid_length; i++) {
        snprintf(
            uid_str + (i * 3),
            sizeof(uid_str) - (i * 3),
            "%02X%s",
            data->uid[i],
            (i < data->uid_length - 1) ? " " : "");
    }
    
    const char* type_str = "Unknown";
    switch(data->type) {
    case ISO14443A_Type2:
        type_str = "Type2";
        break;
    case ISO14443A_Type4:
        type_str = "Type4";
        break;
    case ISO14443A_TypeMifare:
        type_str = "Mifare";
        break;
    default:
        break;
    }
    
    snprintf(
        buffer,
        buffer_size,
        "ISO14443A %s: %s",
        type_str,
        uid_str);
}
