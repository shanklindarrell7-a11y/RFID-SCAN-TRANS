# RFID-SCAN-TRANS
Flipper Zero app for multi-protocol RFID sniffing with advanced features

## Features

### Multi-Protocol Support
- **EM4100** (125 kHz) - Manchester encoded, 40-bit data
- **HID Prox** (125 kHz) - 26-bit format with parity
- **Indala** (125 kHz) - 64-bit and 224-bit formats
- **Mifare Classic** (13.56 MHz) - Via Flipper's NFC hardware, 1K/4K variants
- **ISO14443A** (13.56 MHz) - Type 2 and Type 4 cards

### Advanced Features
- **Auto Protocol Detection** - Automatically identifies RFID protocol from signal characteristics
- **SD Card Logging** - Save captured signals to SD card with timestamps
- **Raw Bitstream Logging** - Optional detailed binary logging for analysis
- **RSSI Visualization** - Real-time signal strength monitoring
- **Configurable Settings**:
  - Center frequency adjustment
  - Bandwidth configuration
  - Protocol-specific parameters

### Signal Analysis
- RSSI (Received Signal Strength Indicator) tracking
- Signal strength history visualization (100-sample buffer)
- Real-time signal quality monitoring

## File Structure

```
.
├── application.fam              # Flipper App Manifest
├── rfid_sniffer.h              # Main application header
├── rfid_sniffer.c              # Main application implementation
├── protocol_em4100.h/c         # EM4100 protocol implementation
├── protocol_hid_prox.h/c       # HID Prox protocol implementation
├── protocol_indala.h/c         # Indala protocol implementation
├── protocol_mifare_classic.h/c # Mifare Classic protocol implementation
└── protocol_iso14443a.h/c      # ISO14443A protocol implementation
```

## Building

This application is designed for the Flipper Zero and requires the official firmware SDK.

### Requirements
- Flipper Zero device
- Flipper Zero firmware development tools (`fbt` or `ufbt`)

### Build Instructions

1. Clone this repository to your Flipper Zero applications directory:
   ```bash
   cd applications_user
   git clone https://github.com/shanklindarrell7-a11y/RFID-SCAN-TRANS.git
   ```

2. Build the application:
   ```bash
   ./fbt fap_rfid_sniffer
   ```
   or with ufbt:
   ```bash
   ufbt
   ```

3. The built `.fap` file will be in `dist/` directory

## Usage

1. Copy the `.fap` file to your Flipper Zero SD card under `/ext/apps/RFID/`
2. Launch the app from the Applications menu
3. Select a protocol or enable auto-detection
4. Configure advanced options:
   - Enable/disable SD card logging
   - Enable/disable raw bitstream logging
   - Adjust center frequency and bandwidth
5. Start sniffing to capture RFID signals

## Application Structure

### Main Components

**RFIDSnifferState**: Holds application state including:
- Current protocol selection
- Sniffing status
- Advanced feature flags (save_to_sd, auto_protocol_detect, etc.)
- Signal data and RSSI history
- File handling for SD logging

**Protocol Implementations**: Each protocol has dedicated decode/encode functions:
- Signal decoding from raw bits
- Data validation (parity, checksums)
- Encoding for emulation
- String formatting for display

### Key Functions

- `rfid_sniffer_start()` - Initialize and start sniffing
- `rfid_sniffer_stop()` - Stop sniffing and close logs
- `rfid_sniffer_process_signal()` - Process captured signals
- `rfid_sniffer_auto_detect_protocol()` - Auto-detect protocol from signal
- `rfid_sniffer_save_signal()` - Save signal to SD card

## Technical Details

### Protocol Specifications

**125 kHz Protocols (LF RFID)**:
- EM4100: 64 bits total (9 header + 40 data + 10 parity + 1 stop)
- HID Prox: 26 bits (1 even parity + 8 facility + 16 card number + 1 odd parity)
- Indala: 64 or 224 bits depending on format

**13.56 MHz Protocols (HF NFC)**:
- Mifare Classic: Uses Flipper's NFC hardware, CRYPTO1 authentication
- ISO14443A: Standard for contactless cards, 4/7/10 byte UIDs

### Log Format

Logs are saved to `/ext/rfid_logs/rfid_YYYYMMDD_HHMMSS.log` with format:
```
Protocol: <protocol_name>, RSSI: <rssi> dBm, Time: <timestamp>, Length: <bytes>
Data: <hex_bytes>
Raw: <binary_bits> (if raw logging enabled)
```

## Contributing

Contributions are welcome! Please ensure code follows the existing style and includes appropriate documentation.

## License

This project is open source. Please check the repository for license details.

## Acknowledgments

- Flipper Zero development team for the excellent SDK
- Community protocol implementations and documentation 
