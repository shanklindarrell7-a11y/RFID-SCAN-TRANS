#include "protocol_hid_prox.h"
#include <string.h>
#include <stdio.h>

// Calculate even parity for first 13 bits
uint8_t hid_prox_calc_even_parity(uint32_t data) {
    uint8_t parity = 0;
    uint32_t mask = data & 0x1FFE; // Bits 1-12
    
    while(mask) {
        parity ^= (mask & 1);
        mask >>= 1;
    }
    
    return parity;
}

// Calculate odd parity for last 13 bits
uint8_t hid_prox_calc_odd_parity(uint32_t data) {
    uint8_t parity = 1; // Start with 1 for odd parity
    uint32_t mask = (data >> 13) & 0x1FFE; // Shift and mask to get bits 13-24
    
    while(mask) {
        parity ^= (mask & 1);
        mask >>= 1;
    }
    
    return parity;
}

// Decode HID Prox signal from raw data
bool hid_prox_decode(const uint8_t* raw_data, size_t data_length, HIDProxData* out) {
    if(!raw_data || !out || data_length < 3) {
        return false;
    }
    
    // HID Prox 26-bit format typically in 4 bytes
    // Format: [Parity][Facility Code 8 bits][Card Number 16 bits][Parity]
    
    uint32_t raw_value = 0;
    
    // Combine bytes into 32-bit value
    for(size_t i = 0; i < data_length && i < 4; i++) {
        raw_value = (raw_value << 8) | raw_data[i];
    }
    
    // Extract facility code and card number
    // Assuming 26-bit format
    out->format_length = HID_PROX_BITS_26;
    out->facility_code = (raw_value >> 17) & 0xFF;
    out->card_number = (raw_value >> 1) & 0xFFFF;
    
    // Verify parity bits
    uint8_t even_parity = (raw_value >> 25) & 1;
    uint8_t odd_parity = raw_value & 1;
    
    uint8_t calc_even = hid_prox_calc_even_parity(raw_value);
    uint8_t calc_odd = hid_prox_calc_odd_parity(raw_value);
    
    out->valid = (even_parity == calc_even) && (odd_parity == calc_odd);
    
    return true;
}

// Encode HID Prox data to raw bits
size_t hid_prox_encode(const HIDProxData* data, uint8_t* raw_data, size_t buffer_size) {
    if(!data || !raw_data || buffer_size < 4) {
        return 0;
    }
    
    // Build 26-bit format
    uint32_t value = 0;
    
    // Add facility code and card number
    value |= ((uint32_t)data->facility_code << 17);
    value |= ((uint32_t)data->card_number << 1);
    
    // Calculate and add parity bits
    uint8_t even_parity = hid_prox_calc_even_parity(value);
    uint8_t odd_parity = hid_prox_calc_odd_parity(value);
    
    value |= ((uint32_t)even_parity << 25);
    value |= (uint32_t)odd_parity;
    
    // Convert to bytes
    raw_data[0] = (value >> 24) & 0xFF;
    raw_data[1] = (value >> 16) & 0xFF;
    raw_data[2] = (value >> 8) & 0xFF;
    raw_data[3] = value & 0xFF;
    
    return 4;
}

// Verify HID Prox parity bits
bool hid_prox_verify_parity(const uint8_t* raw_data, size_t data_length) {
    if(!raw_data || data_length < 3) {
        return false;
    }
    
    HIDProxData data;
    return hid_prox_decode(raw_data, data_length, &data) && data.valid;
}

// Format HID Prox data to string
void hid_prox_format_string(const HIDProxData* data, char* buffer, size_t buffer_size) {
    if(!data || !buffer || buffer_size < 30) {
        return;
    }
    
    snprintf(
        buffer,
        buffer_size,
        "HID Prox: FC=%03u CN=%05u",
        data->facility_code,
        data->card_number);
}
