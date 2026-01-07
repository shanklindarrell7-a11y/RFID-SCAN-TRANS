# RFID Sniffer Setup Guide

This guide provides detailed instructions for setting up and using the RFID Sniffer application on your Flipper Zero device.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Installation Methods](#installation-methods)
3. [First-Time Setup](#first-time-setup)
4. [Quick Start Guide](#quick-start-guide)
5. [Advanced Configuration](#advanced-configuration)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware
- Flipper Zero device (firmware 0.80.0 or later recommended)
- MicroSD card (formatted FAT32, minimum 1GB recommended)
- USB-C cable for connection (if using qFlipper)

### Software
Choose one of the following build tools:

#### Option A: UFBT (Recommended for standalone development)
```bash
# Install UFBT
pip3 install ufbt

# Update UFBT
ufbt update
```

#### Option B: FBT (For firmware integration)
```bash
# Clone Flipper firmware
git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
cd flipperzero-firmware

# Install dependencies (Ubuntu/Debian)
sudo apt-get install git python3 python3-pip
./fbt

# Install dependencies (macOS)
brew install python3
./fbt
```

#### Option C: Pre-built Binary (Easiest)
- Download from releases page
- Requires qFlipper or SD card reader only

## Installation Methods

### Method 1: UFBT Build and Install (Recommended)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/shanklindarrell7-a11y/RFID-SCAN-TRANS.git
   cd RFID-SCAN-TRANS/rfid_sniffer
   ```

2. **Build the application:**
   ```bash
   ufbt build
   ```

3. **Connect Flipper Zero via USB**

4. **Install to device:**
   ```bash
   ufbt launch
   ```
   This will build, upload, and launch the app automatically.

### Method 2: FBT Integration Build

1. **Copy to firmware directory:**
   ```bash
   cp -r rfid_sniffer /path/to/flipperzero-firmware/applications/external/
   ```

2. **Build with firmware:**
   ```bash
   cd /path/to/flipperzero-firmware
   ./fbt fap_rfid_sniffer
   ```

3. **Locate the built file:**
   ```bash
   find build -name "rfid_sniffer.fap"
   ```

4. **Copy to SD card:**
   - Via qFlipper: Use File Browser
   - Via SD card reader: Copy to `/ext/apps/RFID/`

### Method 3: Manual Installation (Pre-built)

1. **Download rfid_sniffer.fap** from releases page

2. **Install using qFlipper:**
   - Connect Flipper Zero
   - Open qFlipper
   - Navigate to File Manager
   - Go to `/ext/apps/RFID/`
   - Upload `rfid_sniffer.fap`

3. **Or install via SD card:**
   - Remove SD card from Flipper
   - Insert into computer
   - Copy `rfid_sniffer.fap` to `/apps/RFID/`
   - Safely eject and reinsert into Flipper

## First-Time Setup

### 1. Verify Installation

After installation:
1. Restart your Flipper Zero
2. Navigate to: **Main Menu** → **Apps** → **RFID**
3. Verify **RFID Sniffer** appears in the list

### 2. Check SD Card Structure

Ensure your SD card has the correct directory structure:
```
/ext/
├── apps/
│   └── RFID/
│       └── rfid_sniffer.fap
└── rfid/
    └── (captured signals will be saved here)
```

The app will automatically create the `/ext/rfid/` directory if it doesn't exist.

### 3. Initial Test

Perform a quick test:
1. Open RFID Sniffer
2. Select **Generate Signal** from Duplicate menu
3. If successful, you'll see a green notification
4. This confirms the app is working correctly

## Quick Start Guide

### Basic Capture Workflow

1. **Start the application:**
   - Apps → RFID → RFID Sniffer

2. **Capture a signal:**
   - Select "Capture Signal"
   - Hold Flipper near RFID tag (< 5cm)
   - Wait for cyan LED blink
   - Press Back when done

3. **Save the signal:**
   - Select "Save to Storage"
   - Check for green success notification
   - File saved to `/ext/rfid/captured_signal.rfid`

4. **Duplicate/Use signal:**
   - Select "Duplicate Signal"
   - Choose mode (Clone/Emulate/Replay/Generate)
   - Wait for completion

### Understanding Notifications

| Color | Meaning |
|-------|---------|
| Cyan Blinking | Capture in progress |
| Green | Operation successful |
| Red | Error or operation failed |

## Advanced Configuration

### Customizing File Paths

Edit the default save path in `rfid_sniffer.c`:

```c
snprintf(app->file_path, sizeof(app->file_path), "/ext/rfid/my_custom_name.rfid");
```

Rebuild after changes.

### Changing Capture Timeout

Modify the timeout in `signal_processor.c`:

```c
#define CAPTURE_TIMEOUT_MS 10000  // Change to 10 seconds
```

### Protocol Selection

Currently supported protocols:
- EM4100 (default)
- HID Prox
- Indala
- IOProx
- AWID

To change the default protocol, modify in signal_processor_alloc():
```c
processor->protocol = ProtocolHIDProx;  // Change default
```

### Signal Format Options

Available formats:
- SignalFormatASK (default)
- SignalFormatFSK
- SignalFormatPSK
- SignalFormatRaw

Change default in signal_processor_alloc():
```c
processor->format = SignalFormatFSK;
```

## Troubleshooting

### App Doesn't Appear in Menu

**Problem:** RFID Sniffer not visible in Apps → RFID

**Solutions:**
1. Verify .fap file location: `/ext/apps/RFID/rfid_sniffer.fap`
2. Check SD card is properly inserted
3. Restart Flipper Zero
4. Rebuild app with correct SDK version
5. Check firmware compatibility (requires 0.80.0+)

### Capture Fails Immediately

**Problem:** Capture stops without finding signal

**Solutions:**
1. Ensure RFID tag is very close (< 5cm)
2. Try different tag types
3. Check tag frequency (125kHz supported)
4. Verify battery level (low battery affects RFID)
5. Test with known-working RFID tag

### Save Operation Fails

**Problem:** Cannot save captured signal

**Solutions:**
1. Check SD card free space (need at least 1MB)
2. Verify SD card is not write-protected
3. Ensure `/ext/rfid/` directory exists
4. Check SD card filesystem (should be FAT32)
5. Try reformatting SD card

### Build Errors

**Problem:** Compilation fails

**Solutions:**
1. Update UFBT: `ufbt update`
2. Clean build: `ufbt clean && ufbt build`
3. Check SDK version compatibility
4. Verify all source files are present
5. Review error messages for missing dependencies

### Runtime Crashes

**Problem:** App crashes or freezes

**Solutions:**
1. Check firmware version compatibility
2. Review logs via qFlipper
3. Rebuild with debug symbols
4. Report issue with crash logs
5. Try fresh firmware installation

## Performance Tips

### Optimal Capture Distance
- Keep tag 2-5cm from Flipper
- Center tag over RFID antenna area
- Avoid metal objects nearby
- Hold steady during capture

### Battery Conservation
- Stop capture when not actively scanning
- Close app when done
- Use airplane mode if available
- Reduce screen timeout

### Storage Management
- Regularly backup captured signals
- Delete old/unnecessary captures
- Keep 20% SD card free space
- Use descriptive filenames

## Getting Help

### Log Collection

Enable debug logging:
1. Connect via qFlipper
2. Open Log Viewer
3. Reproduce issue
4. Save logs
5. Include in bug report

### Community Support

- GitHub Issues: Bug reports and feature requests
- GitHub Discussions: Questions and general help
- Flipper Discord: Community chat and support
- Wiki: Additional documentation

### Reporting Bugs

Include the following in bug reports:
1. Firmware version
2. App version
3. Steps to reproduce
4. Expected vs actual behavior
5. Log files
6. Screenshots if applicable

## Best Practices

### Security
- Only capture your own RFID tags
- Secure captured signal files
- Follow local laws and regulations
- Obtain proper authorization

### Data Management
- Organize captures by purpose/location
- Use meaningful file names
- Regular backups to computer
- Document capture context

### Maintenance
- Keep firmware updated
- Update app regularly
- Check for compatibility issues
- Clean Flipper RFID contacts

## Additional Resources

- [Flipper Zero Documentation](https://docs.flipper.net/)
- [UFBT Documentation](https://github.com/flipperdevices/flipperzero-ufbt)
- [FBT Documentation](https://github.com/flipperdevices/flipperzero-firmware/blob/dev/documentation/fbt.md)
- [RFID Basics](https://docs.flipper.net/rfid)

## Appendix: File Format Reference

### Captured Signal Format (.rfid)

```
Filetype: Flipper RFID key
Version: 1
Format: <0=Raw, 1=ASK, 2=FSK, 3=PSK>
Protocol: <0=EM4100, 1=HIDProx, 2=Indala, 3=IOProx, 4=AWID>
Data size: <bytes>
Data: <hex bytes space-separated>
```

### Example File

```
Filetype: Flipper RFID key
Version: 1
Format: 1
Protocol: 0
Data size: 16
Data: FF 00 12 34 56 78 9A BC DE F0 11 22 33 44 55 66
```

---

**Last Updated:** January 2026
**Version:** 1.0.0
