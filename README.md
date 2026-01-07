# RFID Scan & Trans

A comprehensive RFID scanner and signal management application for Flipper Zero with advanced features for signal storage, multi-format export, and various transmission modes.

## 🎯 Key Features

### 1. Signal Storage System
- **Signal Library**: Database of captured signals with comprehensive metadata
- **Auto-save**: Automatically save captures to SD card
- **Backup System**: Automatic backups on exit to prevent data loss
- **Search & Filter**: Find signals by name, protocol, or UID
- **Favorites**: Mark important signals for quick access and prioritization

### 2. Multi-Format Export
Export your captured RFID signals in various industry-standard formats:

- **Flipper Format (`.sub`)**: Native Flipper Zero SubGHz format for use with other Flipper apps
- **CSV Database**: Spreadsheet-friendly format for analysis and record-keeping
- **Wiegand Format**: Industry-standard Wiegand output with facility code and card number parsing
- **Proxmark3 Commands (`.pm3`)**: Ready-to-use Proxmark3 scripts for cloning and analysis
- **JSON Metadata**: Structured data format for integration with other tools

### 3. Duplication Modes
Multiple transmission modes for different use cases:

- **Clone Mode**: Write signal to physical writable RFID cards (T5577, etc.)
- **Emulate Mode**: Continuous broadcast to simulate card presence
- **Replay Mode**: One-time transmission of captured signal
- **Generate Mode**: Create and transmit variants (incrementing IDs, batch testing)

### 4. Transmission Control
Fine-tune your transmissions with professional controls:

- **Power Levels**: 8 adjustable levels (0-7) for precise range control
- **Repeat Control**: Configurable repeat counts (1, 2, 3, 5, 10, 20, 50)
- **Timing Control**: Adjustable delay between transmissions (0ms - 2000ms)
- **Continuous Mode**: Infinite broadcast for long-term emulation
- **Real-time Monitoring**: Visual feedback with LED notifications during transmission

### 5. Professional Features
Advanced capabilities for security researchers and professionals:

- **Signal Validation**: Check signal integrity before transmission
- **Auto-classification**: Intelligent protocol detection (EM4100, HID Prox, Indala, etc.)
- **Batch Operations**: Export multiple signals at once
- **Encryption Support**: Basic signal obfuscation for sensitive data
- **Metadata Management**: Add notes, timestamps, and signal strength indicators

## 📋 Supported Protocols

- **EM4100**: 125 kHz read-only cards (5-byte UID)
- **HID Prox**: HID proximity cards (6-8 byte)
- **IO Prox**: Indala proximity format
- **Indala**: 125 kHz format (4-byte)
- **T5577**: Writable RFID chip support
- **Auto-detection**: Automatic protocol identification

## 🔄 Usage Workflow

### Basic Workflow
1. **Capture** → Scan RFID card/tag
2. **Store** → Automatically save to library
3. **Analyze** → Review signal details and metadata
4. **Transmit/Duplicate** → Choose duplication mode and transmit

### Detailed Steps

#### Capturing a Signal
1. Open "RFID Scan & Trans" from Flipper Zero apps
2. Select "Capture Signal"
3. Hold RFID card/tag near Flipper Zero
4. Signal is automatically detected and saved

#### Managing Your Library
1. Select "Signal Library" from main menu
2. Favorites (★) appear first for quick access
3. Select any signal to view details
4. Options: Transmit, Export, Edit, Mark Favorite, Delete

#### Exporting Signals
1. From signal details, select "Export"
2. Choose export format:
   - Flipper (.sub) - For use with other Flipper apps
   - CSV - For spreadsheet analysis
   - Wiegand - For access control systems
   - Proxmark3 - For hardware cloning
   - JSON - For programmatic access
3. File saved to `/ext/rfid_signals/` on SD card

#### Transmitting/Duplicating
1. From signal details, select "Transmit/Duplicate"
2. Choose mode:
   - **Clone**: Place writable card near Flipper, select to write
   - **Emulate**: Continuous broadcast (hold near reader)
   - **Replay**: Single transmission attempt
   - **Generate**: Creates variants with incremented IDs
3. LED feedback shows transmission status

#### Adjusting Settings
1. Select "Settings" from main menu
2. Configure:
   - **Power Level**: 0 (lowest) to 7 (highest) - affects range
   - **Repeat Count**: How many times to transmit
   - **Delay**: Time between repeated transmissions
   - **Auto-Save**: Toggle automatic saving on capture

## 📂 File Structure

```
/ext/rfid_signals/           # Main storage directory
├── *.rfid                   # Binary signal files
├── *.sub                    # Flipper format exports
├── *.csv                    # CSV database exports
├── *.wiegand                # Wiegand format exports
├── *.pm3                    # Proxmark3 scripts
└── *.json                   # JSON metadata exports
```

## 🔐 Security & Legal Notice

**⚠️ IMPORTANT**: This tool is designed for:
- Security research and testing
- Legitimate access control system evaluation
- Personal use with your own RFID devices
- Educational purposes

**DO NOT USE** for:
- Unauthorized access to systems
- Cloning cards you don't own
- Any illegal activities

Users are responsible for compliance with local laws and regulations regarding RFID devices and access control systems.

## 🛠️ Technical Details

### Signal Metadata
Each captured signal stores:
- Name (customizable)
- Protocol type
- UID (up to 32 bytes)
- Frequency (typically 125 kHz for LF RFID)
- Signal strength
- Timestamp
- User notes
- Favorite status
- Encryption flag

### Export Format Details

**Flipper Format (.sub)**:
```
Filetype: Flipper SubGHz RAW File
Version: 1
Frequency: 125000
Preset: FuriHalSubGhzPresetOok650Async
Protocol: EM4100
Key: 12 34 56 78 9A
```

**Wiegand Format**:
- Parses 26-bit Wiegand standard
- Extracts facility code (8 bits)
- Extracts card number (16 bits)
- Provides binary and hex representations

**Proxmark3 Commands**:
- Protocol-specific read commands
- Clone/write commands with proper syntax
- Ready to copy-paste into Proxmark3 client

## 🔧 Installation

1. Copy the entire `RFID-SCAN-TRANS` folder to your Flipper Zero:
   - Via qFlipper: Drag folder to `/ext/apps/RFID/`
   - Via SD card: Copy to SD card under `/ext/apps/RFID/`

2. Refresh app list on Flipper Zero:
   - Main Menu → Settings → System → Reboot

3. Launch from: Main Menu → Applications → RFID → RFID Scan & Trans

## 📊 Features Comparison

| Feature | This App | Basic RFID Reader |
|---------|----------|-------------------|
| Multi-protocol support | ✅ | Limited |
| Signal library | ✅ | ❌ |
| Export formats | 5+ | 0 |
| Duplication modes | 4 | 1 |
| Power control | 8 levels | Fixed |
| Batch operations | ✅ | ❌ |
| Favorites | ✅ | ❌ |
| Metadata/notes | ✅ | ❌ |

## 🐛 Troubleshooting

**Signal not detected:**
- Ensure card is close to Flipper (within 1-2 cm)
- Check card is compatible LF RFID (125 kHz)
- Try increasing power level in settings

**Export failed:**
- Check SD card is inserted
- Verify sufficient storage space
- Check file permissions

**Transmission not working:**
- Validate signal before transmitting
- Check power level setting
- Ensure continuous mode for emulation

## 🤝 Contributing

Contributions are welcome! Areas for enhancement:
- Additional protocol support (Mifare, NTAG, etc.)
- Advanced signal analysis tools
- Cloud backup integration
- Custom export templates

## 📝 Version History

**v1.0** (Current)
- Initial release
- 5 export formats
- 4 duplication modes
- Signal library with favorites
- Configurable transmission settings
- Auto-save and backup system

## 📧 Support

For issues, questions, or feature requests:
- Open an issue on GitHub
- Check Flipper Zero community forums
- Review documentation in this README

## 📄 License

This project is for educational and research purposes. Use responsibly and ethically.

---

**Built for Flipper Zero** | **Security Research Tool** | **Use Responsibly**
