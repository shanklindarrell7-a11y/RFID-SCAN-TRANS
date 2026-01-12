#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Indala Protocol Implementation
// 125 kHz RFID protocol
// Variable length formats: 64-bit (Indala-224) and 224-bit most common

#define INDALA_BITS_64 64
#define INDALA_BITS_224 224

typedef struct {
    uint64_t card_id;       // Card ID (for 64-bit format)
    uint8_t raw_data[32];   // Raw data for longer formats
    size_t raw_data_length; // Length of raw data
    uint16_t format_length; // 64 or 224 bits
    bool valid;             // Indicates if data is valid
} IndalaData;

// Decode Indala signal from raw bits
bool indala_decode(const uint8_t* raw_data, size_t data_length, IndalaData* out);

// Encode Indala data to raw bits for emulation
size_t indala_encode(const IndalaData* data, uint8_t* raw_data, size_t buffer_size);

// Verify Indala checksum
bool indala_verify_checksum(const uint8_t* raw_data, size_t data_length);

// Format Indala data to string
void indala_format_string(const IndalaData* data, char* buffer, size_t buffer_size);

// Detect Indala format length
uint16_t indala_detect_format(const uint8_t* raw_data, size_t data_length);
