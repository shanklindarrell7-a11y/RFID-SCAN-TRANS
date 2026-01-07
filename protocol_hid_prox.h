#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// HID Prox Protocol Implementation
// 125 kHz RFID protocol (HID H10301 format is most common)
// 26-bit format: 1 even parity + 8-bit facility code + 16-bit card number + 1 odd parity

#define HID_PROX_BITS_26 26
#define HID_PROX_BITS_37 37

typedef struct {
    uint8_t facility_code;  // 8-bit facility code
    uint16_t card_number;   // 16-bit card number
    uint8_t format_length;  // 26 or 37 bits
    bool valid;             // Indicates if data is valid
} HIDProxData;

// Decode HID Prox signal from raw bits
bool hid_prox_decode(const uint8_t* raw_data, size_t data_length, HIDProxData* out);

// Encode HID Prox data to raw bits for emulation
size_t hid_prox_encode(const HIDProxData* data, uint8_t* raw_data, size_t buffer_size);

// Verify HID Prox parity bits
bool hid_prox_verify_parity(const uint8_t* raw_data, size_t data_length);

// Format HID Prox data to string
void hid_prox_format_string(const HIDProxData* data, char* buffer, size_t buffer_size);

// Calculate even parity for first 13 bits
uint8_t hid_prox_calc_even_parity(uint32_t data);

// Calculate odd parity for last 13 bits
uint8_t hid_prox_calc_odd_parity(uint32_t data);
