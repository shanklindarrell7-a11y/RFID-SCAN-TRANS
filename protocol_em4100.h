#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// EM4100 Protocol Implementation
// 125 kHz RFID protocol with Manchester encoding
// 64-bit data: 9 header bits + 40 data bits (8 version + 32 unique ID) + 10 parity + 1 stop

#define EM4100_HEADER_SIZE 9
#define EM4100_DATA_BITS 40
#define EM4100_PARITY_BITS 10
#define EM4100_TOTAL_BITS 64

typedef struct {
    uint8_t version;      // 8-bit version/customer ID
    uint32_t unique_id;   // 32-bit unique ID
    bool valid;           // Indicates if data is valid
} EM4100Data;

// Decode EM4100 signal from raw bits
bool em4100_decode(const uint8_t* raw_data, size_t data_length, EM4100Data* out);

// Encode EM4100 data to raw bits for emulation
size_t em4100_encode(const EM4100Data* data, uint8_t* raw_data, size_t buffer_size);

// Verify EM4100 parity bits
bool em4100_verify_parity(const uint8_t* raw_data, size_t data_length);

// Format EM4100 data to string
void em4100_format_string(const EM4100Data* data, char* buffer, size_t buffer_size);
