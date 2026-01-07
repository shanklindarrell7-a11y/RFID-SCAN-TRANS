# RFID-SCAN-TRANS

Flipper Zero app for RFID signal scanning, analysis, and transmission.

## Features

- **Multi-Protocol Support**: Detect and decode EM4100, HID ProxCard, Indala, and RAW signals
- **Signal Library**: Store up to 1000+ captured signals
- **Multiple Export Formats**: Export signals in SubGHz, CSV, JSON, Hex, and Binary formats
- **Real-time Processing**: Process signals in real-time for most protocols
- **Extensible Architecture**: Easy to add new protocols

## 🛠️ Development

### Building from Source:

```bash
make
make install
```

### Dependencies:

- Flipper Zero SDK
- SubGHz library
- GUI components
- File system access

### Extending:

To add new protocols:

1. Add detector function in `signal_processor.c`:
   ```c
   static bool detect_my_protocol(const uint8_t* samples, size_t sample_count) {
       // Your detection logic here
       return true; // or false
   }
   ```

2. Add decoder function:
   ```c
   static bool decode_my_protocol(const uint8_t* samples, size_t sample_count, SignalData* signal) {
       // Your decoding logic here
       signal->type = PROTOCOL_MY_PROTOCOL;
       strncpy(signal->name, "My Protocol", sizeof(signal->name) - 1);
       // ... populate signal data
       return true;
   }
   ```

3. Register in `protocol_handlers` array:
   ```c
   static const ProtocolHandler protocol_handlers[] = {
       // ... existing handlers
       {
           .name = "My Protocol",
           .detector = detect_my_protocol,
           .decoder = decode_my_protocol,
       },
   };
   ```

## 📊 Performance

- **Library Capacity**: 1000+ signals
- **Capture Buffer**: 2048 samples
- **Export Formats**: 5+ formats (SubGHz, CSV, JSON, Hex, Binary)
- **Transmit Range**: Up to 1 meter (depends on power)
- **Processing Speed**: Real-time for most protocols

## Project Structure

```
RFID-SCAN-TRANS/
├── application.fam          # Flipper application manifest
├── Makefile                 # Build system
├── include/                 # Header files
│   ├── rfid_scan_trans.h   # Main application header
│   ├── signal_processor.h  # Signal processing header
│   ├── library.h           # Library management header
│   └── export.h            # Export functionality header
└── src/                     # Source files
    ├── rfid_scan_trans.c   # Main application
    ├── signal_processor.c  # Protocol detection and decoding
    ├── library.c           # Signal library management
    └── export.c            # Export to various formats
```

## Usage

1. Launch the app on your Flipper Zero
2. The main screen shows:
   - Current library size
   - Capture buffer size
   - Supported protocols
   - Available export formats
3. Press Back button to exit

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Add tests for new features
4. Commit your changes (`git commit -m 'Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Submit pull request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## License

This project is open source and available for Flipper Zero development.

## Disclaimer

This tool is for educational and authorized testing purposes only. Always ensure you have permission before scanning or transmitting RFID signals. 
