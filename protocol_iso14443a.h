#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// ISO14443A Protocol Implementation
// 13.56 MHz RFID protocol standard
// Used by many NFC cards and tags

#define ISO14443A_UID_SINGLE 4
#define ISO14443A_UID_DOUBLE 7
#define ISO14443A_UID_TRIPLE 10

typedef enum {
    ISO14443A_TypeUnknown,
    ISO14443A_Type2,      // NTAG, Mifare Ultralight
    ISO14443A_Type4,      // DESFire, SmartMX
    ISO14443A_TypeMifare, // Mifare Classic (subset)
} ISO14443AType;

typedef struct {
    uint8_t uid[10];          // UID (4, 7, or 10 bytes)
    uint8_t uid_length;       // 4, 7, or 10
    uint8_t sak;              // Select Acknowledge
    uint8_t atqa[2];          // Answer to Request Type A
    uint8_t ats[32];          // Answer to Select (optional)
    uint8_t ats_length;       // Length of ATS
    ISO14443A_Type type;      // Card type
    bool valid;               // Indicates if data is valid
} ISO14443AData;

// Decode ISO14443A from NFC response
bool iso14443a_decode(const uint8_t* raw_data, size_t data_length, ISO14443AData* out);

// Encode ISO14443A data for emulation
size_t iso14443a_encode(const ISO14443AData* data, uint8_t* raw_data, size_t buffer_size);

// Detect ISO14443A type from SAK and ATS
ISO14443A_Type iso14443a_detect_type(uint8_t sak, const uint8_t* ats, uint8_t ats_length);

// Format ISO14443A data to string
void iso14443a_format_string(const ISO14443AData* data, char* buffer, size_t buffer_size);

// Verify ISO14443A UID cascade
bool iso14443a_verify_uid(const ISO14443AData* data);
