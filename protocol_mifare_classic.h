#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Mifare Classic Protocol Implementation
// 13.56 MHz RFID protocol (uses Flipper's NFC hardware)
// Available in 1K, 4K variants with CRYPTO1 authentication

#define MIFARE_CLASSIC_UID_4 4
#define MIFARE_CLASSIC_UID_7 7
#define MIFARE_CLASSIC_1K_BLOCKS 64
#define MIFARE_CLASSIC_4K_BLOCKS 256

typedef enum {
    MifareClassic1K,
    MifareClassic4K,
} MifareClassicType;

typedef struct {
    uint8_t uid[10];          // UID (4 or 7 bytes)
    uint8_t uid_length;       // 4 or 7
    uint8_t sak;              // Select Acknowledge
    uint8_t atqa[2];          // Answer to Request Type A
    MifareClassicType type;   // 1K or 4K
    bool valid;               // Indicates if data is valid
} MifareClassicData;

// Decode Mifare Classic from NFC response
bool mifare_classic_decode(const uint8_t* raw_data, size_t data_length, MifareClassicData* out);

// Encode Mifare Classic data for emulation
size_t mifare_classic_encode(const MifareClassicData* data, uint8_t* raw_data, size_t buffer_size);

// Detect Mifare Classic type from SAK
MifareClassicType mifare_classic_detect_type(uint8_t sak);

// Format Mifare Classic data to string
void mifare_classic_format_string(const MifareClassicData* data, char* buffer, size_t buffer_size);

// Verify Mifare Classic UID checksum (BCC for 4-byte UID)
bool mifare_classic_verify_uid(const MifareClassicData* data);
