#include "signal_processor.h"
#include <furi.h>
#include <string.h>

// Protocol detector functions
static bool detect_em4100(const uint8_t* samples, size_t sample_count) {
    // EM4100 detection logic
    // Check for 64-bit Manchester encoding with specific preamble
    if(sample_count < 64) return false;
    
    // Simplified detection: look for characteristic pattern
    uint8_t header = samples[0];
    return (header == 0xFF); // Simplified check
}

static bool detect_hid(const uint8_t* samples, size_t sample_count) {
    // HID ProxCard detection logic
    // Check for FSK modulation pattern
    if(sample_count < 44) return false;
    
    // Simplified detection
    return (samples[0] == 0x1D); // Simplified check
}

static bool detect_indala(const uint8_t* samples, size_t sample_count) {
    // Indala detection logic
    // Check for PSK modulation
    if(sample_count < 64) return false;
    
    // Simplified detection
    return (samples[0] == 0x80); // Simplified check
}

// Protocol decoder functions
static bool decode_em4100(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    signal->type = PROTOCOL_EM4100;
    strncpy(signal->name, "EM4100", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Copy raw data (simplified)
    size_t copy_size = sample_count < sizeof(signal->data) ? sample_count : sizeof(signal->data);
    memcpy(signal->data, samples, copy_size);
    signal->data_size = copy_size;
    signal->frequency = 125000; // 125 kHz
    
    return true;
}

static bool decode_hid(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    signal->type = PROTOCOL_HID;
    strncpy(signal->name, "HID ProxCard", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Copy raw data (simplified)
    size_t copy_size = sample_count < sizeof(signal->data) ? sample_count : sizeof(signal->data);
    memcpy(signal->data, samples, copy_size);
    signal->data_size = copy_size;
    signal->frequency = 125000; // 125 kHz
    
    return true;
}

static bool decode_indala(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    signal->type = PROTOCOL_INDALA;
    strncpy(signal->name, "Indala", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Copy raw data (simplified)
    size_t copy_size = sample_count < sizeof(signal->data) ? sample_count : sizeof(signal->data);
    memcpy(signal->data, samples, copy_size);
    signal->data_size = copy_size;
    signal->frequency = 125000; // 125 kHz
    
    return true;
}

static bool decode_raw(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    signal->type = PROTOCOL_RAW;
    strncpy(signal->name, "RAW", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Copy raw data
    size_t copy_size = sample_count < sizeof(signal->data) ? sample_count : sizeof(signal->data);
    memcpy(signal->data, samples, copy_size);
    signal->data_size = copy_size;
    signal->frequency = 125000; // 125 kHz
    
    return true;
}

// Protocol handlers array - easy to extend with new protocols
static const ProtocolHandler protocol_handlers[] = {
    {
        .name = "EM4100",
        .detector = detect_em4100,
        .decoder = decode_em4100,
    },
    {
        .name = "HID ProxCard",
        .detector = detect_hid,
        .decoder = decode_hid,
    },
    {
        .name = "Indala",
        .detector = detect_indala,
        .decoder = decode_indala,
    },
    {
        .name = "RAW",
        .detector = NULL, // RAW is fallback, no detector needed
        .decoder = decode_raw,
    },
};

static const size_t protocol_handlers_count = 
    sizeof(protocol_handlers) / sizeof(protocol_handlers[0]);

bool process_signal(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || sample_count == 0 || !signal) {
        return false;
    }
    
    // Try each protocol detector in order
    for(size_t i = 0; i < protocol_handlers_count; i++) {
        const ProtocolHandler* handler = &protocol_handlers[i];
        
        // If no detector (RAW fallback) or detector returns true
        if(handler->detector == NULL || handler->detector(samples, sample_count)) {
            // Try to decode
            if(handler->decoder && handler->decoder(samples, sample_count, signal)) {
                return true;
            }
        }
    }
    
    // Should not reach here since RAW handler (detector=NULL) should always match
    return false;
}

size_t get_protocol_count(void) {
    return protocol_handlers_count;
}

const char* get_protocol_name(size_t index) {
    if(index >= protocol_handlers_count) return NULL;
    return protocol_handlers[index].name;
}
