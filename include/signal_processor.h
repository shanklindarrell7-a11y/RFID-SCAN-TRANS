#ifndef SIGNAL_PROCESSOR_H
#define SIGNAL_PROCESSOR_H

#include "rfid_scan_trans.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Protocol detector function type
typedef bool (*ProtocolDetector)(const uint8_t* samples, size_t sample_count);

// Protocol decoder function type
typedef bool (*ProtocolDecoder)(const uint8_t* samples, size_t sample_count, SignalData* signal);

// Protocol handler structure for easy extension
typedef struct {
    const char* name;
    ProtocolDetector detector;
    ProtocolDecoder decoder;
} ProtocolHandler;

/**
 * Process captured signal samples and identify protocol
 * @param samples Captured signal samples
 * @param sample_count Number of samples
 * @param signal Output signal data structure
 * @return true if signal was successfully processed
 */
bool process_signal(const uint8_t* samples, size_t sample_count, SignalData* signal);

/**
 * Get number of supported protocols
 * @return Number of protocols
 */
size_t get_protocol_count(void);

/**
 * Get protocol name by index
 * @param index Protocol index
 * @return Protocol name or NULL if invalid index
 */
const char* get_protocol_name(size_t index);

#endif // SIGNAL_PROCESSOR_H
