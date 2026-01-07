# Directory Structure Template

This document shows the directory structure that should be created on the Flipper Zero SD card for RFID Sniffer Pro.

## Creating the Structure Manually

If you need to manually create the directory structure, use these commands on the Flipper Zero CLI or when the SD card is mounted:

```bash
# Create main application directory
mkdir -p /ext/apps/RFID

# Create signal storage directory
mkdir -p /ext/rfid_signals

# Create export directory
mkdir -p /ext/rfid_exports
```

## Initial Setup

On first launch, the RFID Sniffer Pro application will:

1. Check for required directories
2. Create any missing directories automatically
3. Initialize `rfid_database.csv` if it doesn't exist
4. Create an initial backup file

## Directory Purposes

### `/ext/apps/RFID/`
- **Purpose**: Application binary storage
- **Files**: `rfid_sniffer_pro.fap`
- **Managed by**: User (manual installation)

### `/ext/rfid_signals/`
- **Purpose**: Captured signal files
- **Files**: `*.sub` (Flipper SubGhz format)
- **Managed by**: Application (automatic)
- **Naming**: `capture_YYYYMMDD_HHMMSS.sub` or user-defined names

### `/ext/rfid_database.csv`
- **Purpose**: Main signal database
- **Format**: CSV with defined schema
- **Managed by**: Application (automatic updates)
- **Location**: Root of /ext/ directory

### `/ext/rfid_exports/`
- **Purpose**: User-initiated exports
- **Files**: CSV, JSON, or other export formats
- **Managed by**: User and Application
- **Contents**: Reports, filtered exports, data packages

### `/ext/rfid_backup_*.csv`
- **Purpose**: Automatic database backups
- **Format**: Same as main database
- **Managed by**: Application (automatic)
- **Naming**: `rfid_backup_YYYYMMDD_HHMMSS.csv`
- **Retention**: Configurable (default 30 days)

## Permissions

All directories and files should be readable and writable by the application. The Flipper Zero file system typically provides appropriate permissions automatically.

## Storage Considerations

- Average signal file size: 1-5 KB
- Database size grows with signal count
- Backups can accumulate; monitor storage space
- Recommended: Keep at least 100 MB free on SD card

## Cleanup

The application provides built-in cleanup options:
- Delete old backups
- Archive old signals
- Export and remove processed data
- Compact database

## Migration

If upgrading from a previous version, the application will:
- Migrate existing data to new format
- Create backup before migration
- Preserve all captured signals
- Update file paths if structure changed
