# RFID-SCAN-TRANS
Flipper Zero app for RFID sniffing with multi-protocol support and advanced export capabilities.

## Features

### Multi-Protocol Support
- **EM4100** (125 kHz) - Common proximity cards
- **HID Prox** (125 kHz) - Access control cards
- **Indala** (125 kHz) - Proximity cards
- **Mifare Classic** (13.56 MHz) - Contactless smart cards
- **ISO14443A** (13.56 MHz) - NFC/RFID standard

### Advanced Features
- **Auto-save**: Automatically save captures to SD card
- **Signal Analysis**: Real-time RSSI strength visualization
- **Raw Bitstream Logging**: Capture raw signal data
- **Auto-protocol Detection**: Intelligent protocol identification
- **Configurable Bandwidth**: Adjustable center frequency and bandwidth

### Multi-Format Export
Export captured signals in multiple industry-standard formats:

#### 1. **Flipper Format** (.sub)
Native Flipper Zero format for signal replay
- Direct playback on Flipper Zero
- Compatible with SubGhz apps
- Preserves signal metadata

#### 2. **CSV Database** (.csv)
Spreadsheet-friendly format for data analysis
- Import into Excel, Google Sheets
- Batch logging support
- Timestamped entries with full metadata

#### 3. **Wiegand Format** (.wiegand)
Industry-standard access control format
- Facility codes and card numbers
- Binary and hex representations
- 26-bit, 34-bit, and 37-bit support

#### 4. **Proxmark3 Commands** (.pm3)
Ready-to-use Proxmark3 scripts
- Direct execution on Proxmark3 hardware
- Protocol-specific commands
- Read, clone, and simulation scripts

#### 5. **JSON Metadata** (.json)
Structured data for API integration
- Machine-readable format
- Complete signal metadata
- Easy parsing in any language

## Installation

1. Copy the application files to your Flipper Zero SD card
2. Navigate to Applications → GPIO → RFID Sniffer
3. Select protocol and start sniffing

## Usage

### Basic Sniffing
1. Launch the RFID Sniffer app
2. Select the target protocol (or enable auto-detect)
3. Press OK to start sniffing
4. Bring RFID tag/card near the device
5. View captured signals and RSSI strength

### Exporting Signals

#### Export Single Signal to All Formats
```c
export_signal_all_formats(storage, &signal, protocol);
```

#### Export to Specific Format
```c
export_signal_format(storage, &signal, protocol, ExportFormatJSON, "/ext/my_signal.json");
```

#### Batch Export to CSV
```c
export_batch_to_csv(storage, signals, count, protocol, "/ext/batch_log.csv");
```

### Configuration Options
- **Save to SD**: Enable automatic signal saving
- **Auto Protocol Detect**: Automatically identify signal protocol
- **Center Frequency**: Adjust receiver frequency (125 kHz or 13.56 MHz)
- **Bandwidth**: Configure signal bandwidth
- **Raw Bitstream Logging**: Enable detailed bit-level capture

## Export File Locations

Exported files are saved to:
```
/ext/rfid_exports/
├── signal_TIMESTAMP.sub      # Flipper format
├── signal_TIMESTAMP.csv      # CSV format
├── signal_TIMESTAMP.wiegand  # Wiegand format
├── signal_TIMESTAMP.pm3      # Proxmark3 scripts
└── signal_TIMESTAMP.json     # JSON metadata
```

Daily logs:
```
/ext/rfid_exports/log_YYYYMMDD.csv  # Daily CSV log
```

## Documentation

- [Export Formats Guide](EXPORT_FORMATS.md) - Detailed export format documentation
- [Example Code](export_examples.c) - Usage examples and test code

## Building

This is a Flipper Zero application. To build:

```bash
# In your Flipper Zero firmware directory
./fbt fap_rfid_sniffer
```

## Technical Details

### Signal Data Structure
```c
typedef struct {
    uint8_t data[64];      // Raw signal data
    size_t data_length;    // Length of data in bytes
    int8_t rssi;          // Signal strength in dBm
    uint32_t timestamp;    // Capture timestamp
} SignalData;
```

### Export Result Structure
```c
typedef struct {
    bool success;              // Export success status
    char error_message[128];   // Error description if failed
    size_t bytes_written;      // Bytes written to file
} ExportResult;
```

## Use Cases

- **Security Research**: Analyze access control systems
- **Card Duplication**: Clone compatible RFID cards
- **Protocol Analysis**: Study RFID protocol implementations
- **Data Logging**: Long-term signal capture and analysis
- **Integration**: Export to other RFID tools (Proxmark3, etc.)

## Legal Notice

This tool is for educational and security research purposes only. Ensure you have proper authorization before capturing or cloning RFID signals. Unauthorized access to access control systems may be illegal in your jurisdiction.

## Contributing

Contributions are welcome! Please submit pull requests or open issues for bugs and feature requests.

## License

[Add your license information here]

## Credits

Developed for Flipper Zero platform with support for professional RFID analysis and export capabilities.
