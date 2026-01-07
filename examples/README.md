# RFID Sniffer Pro Examples

This directory contains example files demonstrating the structure and format used by RFID Sniffer Pro.

## Files

### rfid_database.csv
Example database file showing the CSV format for storing captured RFID signals. This file includes:
- Sample signal entries
- All required and optional columns
- Various signal types (EM4100, HID)
- Different categories

### capture_example.sub
Example Flipper Zero SubGhz file format for a captured RFID signal. This demonstrates:
- File header format
- Signal metadata
- Raw signal data representation

## Usage

These example files can be used to:
1. Understand the expected file formats
2. Test import functionality
3. Create templates for manual data entry
4. Validate parser implementations

## Notes

- All paths in `rfid_database.csv` reference the `/ext/` directory structure on the Flipper Zero
- The `.sub` file format is standard Flipper Zero SubGhz format
- Timestamps are in UTC ISO8601 format
- Raw data is in hexadecimal format
