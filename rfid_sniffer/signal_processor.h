#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct SignalProcessor SignalProcessor;

typedef enum {
    SignalFormatRaw,
    SignalFormatASK,
    SignalFormatFSK,
    SignalFormatPSK,
} SignalFormat;

typedef enum {
    ProtocolEM4100,
    ProtocolHIDProx,
    ProtocolIndala,
    ProtocolIOProx,
    ProtocolAWID,
} ProtocolType;

SignalProcessor* signal_processor_alloc(void);
void signal_processor_free(SignalProcessor* processor);

bool signal_processor_start(SignalProcessor* processor);
void signal_processor_stop(SignalProcessor* processor);

bool signal_processor_has_data(SignalProcessor* processor);
bool signal_processor_save(SignalProcessor* processor, const char* file_path);
bool signal_processor_load(SignalProcessor* processor, const char* file_path);

bool signal_processor_clone(SignalProcessor* processor);
bool signal_processor_emulate(SignalProcessor* processor);
bool signal_processor_replay(SignalProcessor* processor);
bool signal_processor_generate(SignalProcessor* processor);

void signal_processor_set_format(SignalProcessor* processor, SignalFormat format);
SignalFormat signal_processor_get_format(SignalProcessor* processor);

void signal_processor_set_protocol(SignalProcessor* processor, ProtocolType protocol);
ProtocolType signal_processor_get_protocol(SignalProcessor* processor);

size_t signal_processor_get_data_size(SignalProcessor* processor);
const uint8_t* signal_processor_get_data(SignalProcessor* processor);

// Thread-safe data copy (recommended for most use cases)
bool signal_processor_copy_data(SignalProcessor* processor, uint8_t* buffer, size_t buffer_size, size_t* copied_size);
