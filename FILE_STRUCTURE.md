# RFID Sniffer Pro - File Structure

This document describes the file structure used by the RFID Sniffer Pro application on the Flipper Zero device.

## Directory Structure

All application files are stored on the Flipper Zero's SD card under the `/ext/` directory:

```
/ext/
├── apps/RFID/rfid_sniffer_pro.fap
├── rfid_signals/           # Captured signals (.sub files)
├── rfid_database.csv       # Signal library database
├── rfid_exports/           # Export files
└── rfid_backup_*.csv      # Automatic backups
```

## Directory and File Descriptions

### `/ext/apps/RFID/`
Contains the RFID Sniffer Pro application file.

- **rfid_sniffer_pro.fap** - The compiled Flipper Application Package

### `/ext/rfid_signals/`
Directory for storing captured RFID signals.

- Captured signals are saved as `.sub` files
- Each file represents a single captured RFID transmission
- Files are named with timestamps or user-defined names

### `/ext/rfid_database.csv`
Main database file containing the signal library.

- CSV format for easy import/export
- Contains metadata about captured signals
- See DATABASE_FORMAT.md for detailed structure

### `/ext/rfid_exports/`
Directory for exported data files.

- Export files in various formats (CSV, JSON, etc.)
- Generated reports and analysis outputs
- Shareable data packages

### `/ext/rfid_backup_*.csv`
Automatic backup files of the database.

- Created automatically at regular intervals
- Named with timestamp pattern: `rfid_backup_YYYYMMDD_HHMMSS.csv`
- Provides recovery options in case of data corruption

## File Management

The application automatically creates these directories on first run if they don't exist. Users should not manually modify the directory structure to ensure proper application functionality.

## Storage Recommendations

- Keep at least 100MB free space on the SD card
- Regularly export and archive old signals
- Automatic backups are kept for 30 days by default
