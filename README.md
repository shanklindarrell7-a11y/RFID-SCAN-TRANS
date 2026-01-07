# RFID-SCAN-TRANS
Flipper Zero app for RFID sniffing and signal analysis

## Overview
RFID Sniffer Pro is a comprehensive RFID signal capture and analysis application for the Flipper Zero device. It enables users to capture, store, analyze, and replay RFID signals across multiple frequencies and protocols.

## Features
- Capture RFID signals (EM4100, HID, Indala, and more)
- Store signals in organized database
- Export captured data in multiple formats
- Automatic backup functionality
- Signal categorization and favorites
- Replay captured signals

## File Structure
The application uses a structured directory layout on the Flipper Zero's SD card:

```
/ext/
├── apps/RFID/rfid_sniffer_pro.fap
├── rfid_signals/           # Captured signals (.sub files)
├── rfid_database.csv       # Signal library database
├── rfid_exports/           # Export files
└── rfid_backup_*.csv      # Automatic backups
```

For detailed information about the file structure, see [FILE_STRUCTURE.md](FILE_STRUCTURE.md).

## Database Format
The application stores captured signals in a CSV database. For complete database format specification, see [DATABASE_FORMAT.md](DATABASE_FORMAT.md).

## Examples
Example files demonstrating the proper format for database and signal files can be found in the [examples/](examples/) directory.

## Installation
1. Copy `rfid_sniffer_pro.fap` to `/ext/apps/RFID/` on your Flipper Zero SD card
2. The application will automatically create required directories on first run

## Usage
1. Launch RFID Sniffer Pro from the Applications menu
2. Select "Capture" to start listening for RFID signals
3. Captured signals are automatically saved to the database
4. Use "Browse" to view and manage captured signals
5. Use "Export" to create backup files or share data

## Support
For issues and questions, please use the GitHub issue tracker. 
