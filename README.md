# RFID-SCAN-TRANS

A comprehensive RFID sniffer application for Flipper Zero that enables real-time RFID signal capture, storage in multiple formats, and advanced signal duplication capabilities.

## Features

### Core Functionality
- **Real-time RFID Signal Capture**: Capture 125 kHz RFID signals in real-time
- **Multiple Signal Formats**: Support for ASK, FSK, PSK, and Raw signal formats
- **Protocol Support**: Compatible with EM4100, HID Prox, Indala, IOProx, and AWID protocols
- **Storage System**: Save captured signals to SD card in Flipper RFID format
- **Signal Duplication**: Four duplication modes for signal manipulation

### Signal Duplication Modes
1. **Clone Mode**: Create an exact copy of the captured signal
2. **Emulate Mode**: Emulate the captured RFID tag
3. **Replay Mode**: Replay the captured signal
4. **Generate Mode**: Generate new signals based on captured data

### Additional Features
- Robust error handling and validation
- Thread-safe signal processing
- Automatic directory creation for storage
- Visual and haptic feedback via notifications
- User-friendly menu interface

## Installation

### Prerequisites
- Flipper Zero device
- UFBT (Universal Flipper Build Tool) or FBT (Flipper Build Tool)
- SD card formatted for Flipper Zero

### Method 1: Using UFBT (Recommended)
```bash
# Navigate to the rfid_sniffer directory
cd rfid_sniffer

# Build the application
ufbt build

# Launch on connected Flipper Zero
ufbt launch
```

### Method 2: Using FBT
```bash
# Copy rfid_sniffer directory to Flipper firmware apps directory
cp -r rfid_sniffer /path/to/flipperzero-firmware/applications/external/

# Build using FBT
cd /path/to/flipperzero-firmware
./fbt fap_rfid_sniffer

# Copy the built .fap file to your Flipper's SD card
cp build/latest/f7-firmware-D/.extapps/rfid_sniffer.fap /media/sdcard/apps/RFID/
```

### Method 3: Manual Installation
1. Download the pre-built `rfid_sniffer.fap` file from releases
2. Connect your Flipper Zero via qFlipper or mount SD card
3. Copy the `.fap` file to `/ext/apps/RFID/` on your SD card
4. Safely eject and restart Flipper Zero

## Usage

### Starting the Application
1. Navigate to **Apps** → **RFID** on your Flipper Zero
2. Select **RFID Sniffer** from the list
3. The main menu will appear with available options

### Capturing RFID Signals
1. Select **Capture Signal** from the main menu
2. Hold your Flipper Zero near the RFID tag/reader
3. Wait for the capture indicator (LED will blink cyan)
4. Press **Back** to stop capturing
5. The signal will be automatically processed and stored in memory

### Saving Captured Signals
1. After capturing a signal, select **Save to Storage**
2. The signal will be saved to `/ext/rfid/captured_signal.rfid`
3. Success notification will confirm the save operation
4. Files can be accessed via qFlipper or SD card reader

### Duplicating Signals
1. Ensure you have captured or loaded a signal
2. Select **Duplicate Signal** from the main menu
3. Choose your duplication mode:
   - **Clone**: Creates exact duplicate
   - **Emulate**: Emulates the tag
   - **Replay**: Replays the signal
   - **Generate**: Generates variations
4. Wait for the operation to complete
5. Success/failure will be indicated via notification

### Settings
Access additional configuration options:
- Change signal format (ASK/FSK/PSK/Raw)
- Select protocol type
- Configure capture timeout
- Adjust storage paths

## Controls

### Main Menu Navigation
- **Up/Down**: Navigate menu items
- **OK**: Select menu item
- **Back**: Return to previous screen or stop operation

### During Capture
- **Back**: Stop capturing and return to menu
- Device will automatically stop after timeout (5 seconds)

### Notifications
- **Cyan Blink**: Capture in progress
- **Green**: Operation successful
- **Red**: Operation failed or error occurred

## File Format

Captured signals are saved in Flipper RFID format:

```
Filetype: Flipper RFID key
Version: 1
Format: <signal_format>
Protocol: <protocol_type>
Data size: <bytes>
Data: <hex_data>
```

Example:
```
Filetype: Flipper RFID key
Version: 1
Format: 1
Protocol: 0
Data size: 64
Data: FF 00 12 34 56 78 9A BC DE F0 11 22 33 44 55 66 ...
```

## Build Configuration

### application.fam
The `application.fam` file defines the app metadata:
- **appid**: rfid_sniffer
- **apptype**: EXTERNAL (FAP plugin)
- **entry_point**: rfid_sniffer_app
- **stack_size**: 2 KB
- **category**: RFID
- **version**: 1.0.0

### Dependencies
- GUI framework
- Storage API
- Notification API
- FURI threading and synchronization

## Architecture

### Components

#### rfid_sniffer.c
Main application logic including:
- User interface and menu system
- Event handling
- Application lifecycle management
- Integration with signal processor

#### signal_processor.c/h
Signal processing module featuring:
- Signal capture thread management
- Protocol encoding/decoding
- File I/O operations
- Duplication mode implementations
- Thread-safe data access

### Thread Safety
- Mutex-protected shared data structures
- Separate capture thread for non-blocking operation
- Proper resource cleanup and synchronization

## Troubleshooting

### Build Issues
**Problem**: Compilation errors
- Ensure UFBT or FBT is properly installed
- Verify SDK version compatibility
- Check that all source files are present

**Problem**: Linking errors
- Verify all required libraries are available
- Check application.fam for correct dependencies

### Runtime Issues
**Problem**: App doesn't appear in menu
- Verify .fap file is in correct directory (`/ext/apps/RFID/`)
- Restart Flipper Zero
- Check file permissions

**Problem**: Capture not working
- Ensure RFID tag is close enough (< 5cm)
- Try different protocols in settings
- Verify SD card is properly mounted

**Problem**: Save operation fails
- Check available SD card space
- Verify `/ext/rfid/` directory exists
- Try different file path in settings

## Development

### Building from Source
```bash
git clone https://github.com/shanklindarrell7-a11y/RFID-SCAN-TRANS.git
cd RFID-SCAN-TRANS/rfid_sniffer
ufbt build
```

### Project Structure
```
rfid_sniffer/
├── application.fam          # App manifest
├── Makefile                 # Build configuration
├── rfid_sniffer.c          # Main application
├── signal_processor.c      # Signal processing
└── signal_processor.h      # Signal processor header
```

### Contributing
Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on hardware
5. Submit a pull request

## Security Considerations

⚠️ **Important**: This tool is intended for:
- Educational purposes
- Security research
- Testing your own RFID systems
- Authorized penetration testing

**Do not use this tool to:**
- Access unauthorized RFID systems
- Clone cards you don't own
- Violate any laws or regulations

Users are responsible for complying with local laws and regulations regarding RFID technology and security testing.

## License

This project is provided as-is for educational and research purposes. Users are responsible for ensuring their use complies with applicable laws and regulations.

## Support

- **Issues**: Report bugs via GitHub Issues
- **Discussions**: Use GitHub Discussions for questions
- **Wiki**: Additional documentation on the Wiki

## Acknowledgments

- Flipper Zero team for the excellent hardware and SDK
- RFID security research community
- Contributors to this project

## Version History

### v1.0.0 (Current)
- Initial release
- Real-time signal capture
- Multiple format support
- Four duplication modes
- Storage system
- Comprehensive error handling

## Roadmap

Future enhancements may include:
- Additional protocol support
- Advanced signal analysis
- Custom signal generation
- Batch processing
- Enhanced UI with visualization
- Export to multiple formats
