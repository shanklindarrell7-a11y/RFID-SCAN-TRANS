# Protocol Extension Guide

This guide demonstrates how to add support for new RFID/NFC protocols to RFID-SCAN-TRANS.

## Overview

The protocol detection and decoding system is designed to be easily extensible. Adding a new protocol requires three simple steps:

1. Implement a detector function
2. Implement a decoder function  
3. Register the protocol in the `protocol_handlers` array

## Step-by-Step Example: Adding AWID Protocol

Let's walk through adding support for the AWID (Applied Wireless Identification) protocol.

### Step 1: Add Protocol Type

First, add the protocol type to the enum in `include/rfid_scan_trans.h`:

```c
typedef enum {
    PROTOCOL_UNKNOWN,
    PROTOCOL_EM4100,
    PROTOCOL_HID,
    PROTOCOL_INDALA,
    PROTOCOL_AWID,      // Add your protocol here
    PROTOCOL_RAW,
} ProtocolType;
```

### Step 2: Implement Detector Function

Add the detector function in `src/signal_processor.c`. The detector identifies if captured samples match your protocol's characteristics:

```c
static bool detect_awid(const uint8_t* samples, size_t sample_count) {
    // AWID uses FSK modulation with specific characteristics
    if(sample_count < 96) return false;  // AWID needs at least 96 bits
    
    // Check for AWID preamble (simplified example)
    // Real implementation would check:
    // - Bit timing (typically 1:2 ratio for FSK)
    // - Preamble pattern (0x01)
    // - Valid site code and card number ranges
    
    if(samples[0] == 0x01 && samples[1] == 0x1F) {
        // Additional validation here
        return true;
    }
    
    return false;
}
```

**Key Points:**
- Return `true` if samples match your protocol
- Return `false` if they don't match
- Check minimum sample count needed
- Look for protocol-specific patterns (preamble, timing, etc.)

### Step 3: Implement Decoder Function

Add the decoder function in `src/signal_processor.c`. The decoder extracts meaningful data from the samples:

```c
static bool decode_awid(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    // Set protocol type and name
    signal->type = PROTOCOL_AWID;
    strncpy(signal->name, "AWID", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Parse AWID data structure
    // AWID format: Preamble + Site Code + Card Number
    
    // Extract site code (bits 8-23, example)
    uint16_t site_code = (samples[1] << 8) | samples[2];
    
    // Extract card number (bits 24-47, example)
    uint32_t card_number = (samples[3] << 16) | (samples[4] << 8) | samples[5];
    
    // Store decoded data
    signal->data[0] = (site_code >> 8) & 0xFF;
    signal->data[1] = site_code & 0xFF;
    signal->data[2] = (card_number >> 16) & 0xFF;
    signal->data[3] = (card_number >> 8) & 0xFF;
    signal->data[4] = card_number & 0xFF;
    signal->data_size = 5;
    
    // Set frequency (AWID typically uses 125 kHz)
    signal->frequency = 125000;
    
    return true;
}
```

**Key Points:**
- Validate input parameters
- Set `signal->type` to your protocol enum value
- Set `signal->name` to a readable string
- Parse and store decoded data in `signal->data`
- Set `signal->data_size` to the number of bytes stored
- Set `signal->frequency` to the protocol's operating frequency

### Step 4: Register Protocol Handler

Add your protocol to the `protocol_handlers` array in `src/signal_processor.c`:

```c
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
        .name = "AWID",           // Add your protocol entry
        .detector = detect_awid,
        .decoder = decode_awid,
    },
    {
        .name = "RAW",
        .detector = NULL,
        .decoder = decode_raw,
    },
};
```

**Important:** Always keep the RAW protocol as the last entry! It serves as a fallback for unknown protocols.

## Testing Your Protocol

After adding your protocol:

1. **Build the application:**
   ```bash
   make clean
   make
   ```

2. **Test with known samples:**
   - Create test samples that match your protocol
   - Verify detection returns `true`
   - Verify decoding extracts correct data

3. **Test edge cases:**
   - Too few samples
   - Invalid data
   - Corrupted signals

4. **Test on hardware:**
   - Deploy to Flipper Zero
   - Capture real signals
   - Verify correct protocol identification

## Common Patterns

### Checksum Validation

Many protocols include checksums. Validate them in the detector:

```c
static bool detect_protocol_with_checksum(const uint8_t* samples, size_t sample_count) {
    if(sample_count < 10) return false;
    
    // Calculate checksum
    uint8_t checksum = 0;
    for(size_t i = 0; i < 8; i++) {
        checksum ^= samples[i];
    }
    
    // Verify checksum matches
    return (checksum == samples[8]);
}
```

### Manchester Encoding

For Manchester-encoded protocols:

```c
static bool decode_manchester(const uint8_t* samples, size_t sample_count, uint8_t* output) {
    // Manchester: transition in middle of each bit
    // 0 = high-to-low, 1 = low-to-high
    
    for(size_t i = 0; i < sample_count / 2; i++) {
        bool first_half = samples[i * 2] > 128;
        bool second_half = samples[i * 2 + 1] > 128;
        
        if(first_half && !second_half) {
            output[i / 8] |= (0 << (7 - (i % 8)));
        } else if(!first_half && second_half) {
            output[i / 8] |= (1 << (7 - (i % 8)));
        } else {
            return false; // Invalid Manchester encoding
        }
    }
    
    return true;
}
```

### Frequency Detection

Some protocols can be identified by frequency:

```c
static bool detect_by_frequency(const uint8_t* samples, size_t sample_count) {
    // Analyze signal frequency characteristics
    uint32_t zero_crossings = 0;
    
    for(size_t i = 1; i < sample_count; i++) {
        if((samples[i-1] < 128 && samples[i] >= 128) ||
           (samples[i-1] >= 128 && samples[i] < 128)) {
            zero_crossings++;
        }
    }
    
    // Calculate approximate frequency
    uint32_t freq = (zero_crossings * 1000000) / sample_count;
    
    // Check if frequency matches expected value (e.g., 125kHz ± 5%)
    return (freq >= 118750 && freq <= 131250);
}
```

## Best Practices

1. **Validate inputs:** Always check for NULL pointers and minimum sample counts
2. **Document formats:** Add comments explaining the protocol's data format
3. **Use meaningful names:** Name variables according to the protocol specification
4. **Handle errors gracefully:** Return `false` on any error condition
5. **Test thoroughly:** Test with various real-world samples
6. **Add logging:** Use `FURI_LOG_*` macros for debugging

## Example: Complete Protocol Implementation

Here's a complete example combining all concepts:

```c
// Protocol specification: XYZ Card
// - Frequency: 125 kHz
// - Encoding: Manchester
// - Format: 8-bit preamble + 16-bit ID + 8-bit checksum

static bool detect_xyz(const uint8_t* samples, size_t sample_count) {
    if(sample_count < 32) return false;  // 32 bits minimum
    
    // Check preamble (0xAA in Manchester)
    if(samples[0] != 0x55 || samples[1] != 0x55) {
        return false;
    }
    
    // Verify checksum
    uint8_t checksum = 0;
    for(size_t i = 2; i < 10; i++) {
        checksum ^= samples[i];
    }
    
    return (checksum == samples[10]);
}

static bool decode_xyz(const uint8_t* samples, size_t sample_count, SignalData* signal) {
    if(!samples || !signal) return false;
    
    signal->type = PROTOCOL_XYZ;
    strncpy(signal->name, "XYZ Card", sizeof(signal->name) - 1);
    signal->name[sizeof(signal->name) - 1] = '\0';
    
    // Decode Manchester to binary
    uint8_t decoded[4] = {0};
    if(!decode_manchester(samples + 2, 16, decoded)) {
        return false;
    }
    
    // Extract 16-bit ID
    uint16_t card_id = (decoded[0] << 8) | decoded[1];
    
    // Store in signal
    signal->data[0] = (card_id >> 8) & 0xFF;
    signal->data[1] = card_id & 0xFF;
    signal->data_size = 2;
    signal->frequency = 125000;
    
    FURI_LOG_I(TAG, "Decoded XYZ Card ID: %04X", card_id);
    
    return true;
}
```

## Further Resources

- **Flipper Zero Documentation**: https://docs.flipperzero.one/
- **SubGHz Protocol Analysis**: Study existing protocol implementations in Flipper firmware
- **RFID Protocol Specifications**: Manufacturer datasheets and standards documents

## Support

If you need help adding a protocol:

1. Check existing protocol implementations for similar patterns
2. Review the protocol's technical specification
3. Open an issue with the `protocol-request` label
4. Include sample captures and documentation links

Happy coding!
