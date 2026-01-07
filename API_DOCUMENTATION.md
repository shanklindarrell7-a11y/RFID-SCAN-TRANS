# API Documentation

## Signal Processor API

The Signal Processor provides a comprehensive API for RFID signal capture, storage, and manipulation.

### Data Types

#### SignalFormat
Enumeration of supported signal formats:
```c
typedef enum {
    SignalFormatRaw,    // Raw unprocessed signal
    SignalFormatASK,    // Amplitude Shift Keying
    SignalFormatFSK,    // Frequency Shift Keying
    SignalFormatPSK,    // Phase Shift Keying
} SignalFormat;
```

#### ProtocolType
Enumeration of supported RFID protocols:
```c
typedef enum {
    ProtocolEM4100,     // EM4100 125 kHz
    ProtocolHIDProx,    // HID Proximity
    ProtocolIndala,     // Indala
    ProtocolIOProx,     // IOProx
    ProtocolAWID,       // AWID
} ProtocolType;
```

### Memory Management

#### signal_processor_alloc()
Allocates and initializes a new SignalProcessor instance.

```c
SignalProcessor* signal_processor_alloc(void);
```

**Returns:** Pointer to allocated SignalProcessor, or NULL on failure

**Example:**
```c
SignalProcessor* processor = signal_processor_alloc();
if (processor == NULL) {
    // Handle allocation failure
}
```

#### signal_processor_free()
Frees a SignalProcessor instance and all associated resources.

```c
void signal_processor_free(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance to free (must not be NULL)

**Example:**
```c
signal_processor_free(processor);
processor = NULL; // Good practice
```

### Signal Capture

#### signal_processor_start()
Starts asynchronous signal capture in a separate thread.

```c
bool signal_processor_start(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** true on success, false if already capturing or on error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_start(processor)) {
    printf("Capture started\n");
} else {
    printf("Failed to start capture\n");
}
```

#### signal_processor_stop()
Stops ongoing signal capture and waits for capture thread to complete.

```c
void signal_processor_stop(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Thread Safety:** Thread-safe

**Example:**
```c
signal_processor_stop(processor);
printf("Capture stopped\n");
```

### Data Access

#### signal_processor_has_data()
Checks if signal data has been captured.

```c
bool signal_processor_has_data(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** true if data is available, false otherwise

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_has_data(processor)) {
    // Process the captured data
}
```

#### signal_processor_get_data_size()
Returns the size of captured signal data in bytes.

```c
size_t signal_processor_get_data_size(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** Size of signal data in bytes

**Thread Safety:** Thread-safe

#### signal_processor_copy_data() ⭐ Recommended
Thread-safe function to copy signal data to a buffer.

```c
bool signal_processor_copy_data(
    SignalProcessor* processor,
    uint8_t* buffer,
    size_t buffer_size,
    size_t* copied_size
);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)
- `buffer`: Destination buffer (must not be NULL)
- `buffer_size`: Size of destination buffer in bytes
- `copied_size`: Output parameter for actual bytes copied (must not be NULL)

**Returns:** true on success, false if no data available

**Thread Safety:** Thread-safe (handles mutex internally)

**Example:**
```c
uint8_t buffer[MAX_SIGNAL_SIZE];
size_t copied;

if (signal_processor_copy_data(processor, buffer, sizeof(buffer), &copied)) {
    printf("Copied %zu bytes\n", copied);
    // Use buffer safely
} else {
    printf("No data available\n");
}
```

#### signal_processor_get_data() ⚠️ Advanced Use Only
Returns a pointer to internal signal data buffer.

```c
const uint8_t* signal_processor_get_data(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** Pointer to internal signal data buffer

**Thread Safety:** ⚠️ NOT thread-safe. Caller must acquire mutex first!

**Warning:** This is a low-level API. Use `signal_processor_copy_data()` instead for most use cases.

**Example (Advanced):**
```c
// Only use if you need zero-copy access and understand the risks
furi_mutex_acquire(processor->mutex, FuriWaitForever);
const uint8_t* data = signal_processor_get_data(processor);
// Use data (but don't store the pointer!)
// ...
furi_mutex_release(processor->mutex);
```

### File Operations

#### signal_processor_save()
Saves captured signal to a file in Flipper RFID format.

```c
bool signal_processor_save(SignalProcessor* processor, const char* file_path);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)
- `file_path`: Path to save file (must not be NULL)

**Returns:** true on success, false on error

**Thread Safety:** Thread-safe

**File Format:**
```
Filetype: Flipper RFID key
Version: 1
Format: <format_id>
Protocol: <protocol_id>
Data size: <size>
Data: <hex bytes>
```

**Example:**
```c
if (signal_processor_save(processor, "/ext/rfid/my_signal.rfid")) {
    printf("Signal saved successfully\n");
} else {
    printf("Failed to save signal\n");
}
```

#### signal_processor_load()
Loads signal data from a file.

```c
bool signal_processor_load(SignalProcessor* processor, const char* file_path);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)
- `file_path`: Path to load file (must not be NULL)

**Returns:** true on success, false on error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_load(processor, "/ext/rfid/my_signal.rfid")) {
    printf("Signal loaded successfully\n");
} else {
    printf("Failed to load signal\n");
}
```

### Signal Duplication

#### signal_processor_clone()
Creates an exact copy of the captured signal for writing to a tag.

```c
bool signal_processor_clone(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance with captured data (must not be NULL)

**Returns:** true on success, false if no data available or error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_clone(processor)) {
    printf("Signal cloned successfully\n");
} else {
    printf("Clone failed\n");
}
```

#### signal_processor_emulate()
Emulates the captured RFID tag.

```c
bool signal_processor_emulate(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance with captured data (must not be NULL)

**Returns:** true on success, false if no data available or error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_emulate(processor)) {
    printf("Emulation started\n");
} else {
    printf("Emulation failed\n");
}
```

#### signal_processor_replay()
Replays the captured signal.

```c
bool signal_processor_replay(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance with captured data (must not be NULL)

**Returns:** true on success, false if no data available or error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_replay(processor)) {
    printf("Replay completed\n");
} else {
    printf("Replay failed\n");
}
```

#### signal_processor_generate()
Generates a new signal based on captured data patterns.

```c
bool signal_processor_generate(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** true on success, false on error

**Thread Safety:** Thread-safe

**Example:**
```c
if (signal_processor_generate(processor)) {
    printf("Signal generated\n");
} else {
    printf("Generation failed\n");
}
```

### Configuration

#### signal_processor_set_format()
Sets the signal format for capture and processing.

```c
void signal_processor_set_format(SignalProcessor* processor, SignalFormat format);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)
- `format`: Signal format to use

**Thread Safety:** Thread-safe

**Example:**
```c
signal_processor_set_format(processor, SignalFormatASK);
```

#### signal_processor_get_format()
Gets the current signal format.

```c
SignalFormat signal_processor_get_format(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** Current signal format

**Thread Safety:** Thread-safe

#### signal_processor_set_protocol()
Sets the RFID protocol for capture and processing.

```c
void signal_processor_set_protocol(SignalProcessor* processor, ProtocolType protocol);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)
- `protocol`: Protocol type to use

**Thread Safety:** Thread-safe

**Example:**
```c
signal_processor_set_protocol(processor, ProtocolEM4100);
```

#### signal_processor_get_protocol()
Gets the current RFID protocol.

```c
ProtocolType signal_processor_get_protocol(SignalProcessor* processor);
```

**Parameters:**
- `processor`: SignalProcessor instance (must not be NULL)

**Returns:** Current protocol type

**Thread Safety:** Thread-safe

## Usage Examples

### Basic Capture and Save
```c
// Allocate processor
SignalProcessor* processor = signal_processor_alloc();

// Start capture
if (signal_processor_start(processor)) {
    // Wait for capture (or use callbacks)
    furi_delay_ms(5000);
    
    // Stop capture
    signal_processor_stop(processor);
    
    // Check if data was captured
    if (signal_processor_has_data(processor)) {
        // Save to file
        signal_processor_save(processor, "/ext/rfid/captured.rfid");
    }
}

// Cleanup
signal_processor_free(processor);
```

### Load and Duplicate
```c
SignalProcessor* processor = signal_processor_alloc();

// Load existing signal
if (signal_processor_load(processor, "/ext/rfid/existing.rfid")) {
    // Clone the signal
    signal_processor_clone(processor);
}

signal_processor_free(processor);
```

### Thread-Safe Data Access
```c
SignalProcessor* processor = signal_processor_alloc();

// After capturing data...
if (signal_processor_has_data(processor)) {
    uint8_t buffer[4096];
    size_t size;
    
    // Copy data safely
    if (signal_processor_copy_data(processor, buffer, sizeof(buffer), &size)) {
        // Process buffer
        for (size_t i = 0; i < size; i++) {
            printf("%02X ", buffer[i]);
        }
    }
}

signal_processor_free(processor);
```

## Error Handling

All functions that can fail return a boolean indicating success/failure. Always check return values:

```c
if (!signal_processor_start(processor)) {
    FURI_LOG_E(TAG, "Failed to start capture");
    // Handle error
    return;
}
```

For functions that allocate memory:
```c
SignalProcessor* processor = signal_processor_alloc();
if (processor == NULL) {
    FURI_LOG_E(TAG, "Failed to allocate processor");
    return;
}
```

## Thread Safety Summary

| Function | Thread Safe | Notes |
|----------|-------------|-------|
| `signal_processor_alloc()` | N/A | Call only from main thread |
| `signal_processor_free()` | No | Call only when no other threads using |
| `signal_processor_start()` | Yes | Uses mutex internally |
| `signal_processor_stop()` | Yes | Uses mutex internally |
| `signal_processor_has_data()` | Yes | Uses mutex internally |
| `signal_processor_get_data_size()` | Yes | Uses mutex internally |
| `signal_processor_copy_data()` | Yes | ⭐ Recommended for data access |
| `signal_processor_get_data()` | ⚠️ No | Caller must hold mutex |
| `signal_processor_save()` | Yes | Uses mutex internally |
| `signal_processor_load()` | Yes | Uses mutex internally |
| `signal_processor_clone()` | Yes | Uses mutex internally |
| `signal_processor_emulate()` | Yes | Uses mutex internally |
| `signal_processor_replay()` | Yes | Uses mutex internally |
| `signal_processor_generate()` | Yes | Uses mutex internally |
| `signal_processor_set_format()` | Yes | Uses mutex internally |
| `signal_processor_get_format()` | Yes | Uses mutex internally |
| `signal_processor_set_protocol()` | Yes | Uses mutex internally |
| `signal_processor_get_protocol()` | Yes | Uses mutex internally |

## Best Practices

1. **Always check return values** from functions that can fail
2. **Use `signal_processor_copy_data()`** instead of `signal_processor_get_data()` for thread safety
3. **Free resources** with `signal_processor_free()` when done
4. **Stop capture** before freeing the processor
5. **Check for data** with `signal_processor_has_data()` before save/duplicate operations
6. **Handle errors gracefully** and provide user feedback

## Constants

```c
#define MAX_SIGNAL_SIZE 4096       // Maximum signal data size in bytes
#define CAPTURE_TIMEOUT_MS 5000    // Capture timeout in milliseconds
#define RFID_FREQUENCY_125KHZ 125000 // RFID frequency in Hz
```

---

**API Version:** 1.0.0  
**Last Updated:** January 2026
