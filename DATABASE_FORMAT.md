# RFID Database Format Specification

This document describes the CSV database format used by RFID Sniffer Pro.

## rfid_database.csv Format

The database is stored as a CSV file with the following columns:

### Column Structure

| Column | Type | Description | Required |
|--------|------|-------------|----------|
| id | Integer | Unique identifier for the signal | Yes |
| timestamp | ISO8601 | Date and time when signal was captured | Yes |
| signal_type | String | Type of RFID signal (EM4100, HID, etc.) | Yes |
| frequency | Float | Frequency in kHz | Yes |
| protocol | String | RFID protocol name | Yes |
| raw_data | Hex String | Raw hexadecimal data of the signal | Yes |
| decoded_data | String | Decoded/interpreted data | No |
| tag_id | String | Tag identifier if available | No |
| file_path | String | Path to the .sub file | Yes |
| notes | String | User notes about the signal | No |
| category | String | User-defined category | No |
| favorite | Boolean | Marked as favorite (true/false) | No |

### Example Entry

```csv
id,timestamp,signal_type,frequency,protocol,raw_data,decoded_data,tag_id,file_path,notes,category,favorite
1,2026-01-07T09:00:00Z,EM4100,125.0,EM-Marin,0F0368FA8B,,0368FA8B,/ext/rfid_signals/capture_20260107_090000.sub,Office door tag,Access Control,true
2,2026-01-07T10:30:00Z,HID,125.0,HID Prox,2006EC9A5E,12345,EC9A5E,/ext/rfid_signals/capture_20260107_103000.sub,Parking garage,Access Control,false
```

### Field Details

#### signal_type
Supported signal types:
- EM4100 (EM-Marin)
- HID Prox
- Indala
- AWID
- ioProx
- KERI
- Pyramid
- Viking
- Other (custom/unknown)

#### frequency
Common RFID frequencies:
- 125.0 kHz (Low Frequency)
- 134.2 kHz (Animal ID)
- 13.56 MHz (High Frequency - NFC)

#### protocol
Specific protocol implementation within the signal type.

#### raw_data
Hexadecimal representation of the captured signal data.
- No spaces or separators
- Uppercase hexadecimal characters
- Variable length depending on protocol

#### decoded_data
Human-readable interpretation of the raw data when available.
- May include facility code and card number for access control
- Binary representation for debugging
- Protocol-specific formatting

#### tag_id
Extracted unique identifier for the RFID tag.
- Format varies by protocol
- Used for quick lookup and comparison

#### file_path
Absolute path to the corresponding .sub file on the SD card.
- Must start with `/ext/rfid_signals/`
- File must exist for signal playback

#### category
User-defined categories for organization:
- Access Control
- Asset Tracking
- Transportation
- Identification
- Research
- Other

## Data Integrity

- The CSV file must have a header row
- All fields should be properly escaped (quotes for strings with commas)
- Missing optional fields can be empty but commas must be present
- IDs must be unique and sequential
- Timestamps should be in UTC ISO8601 format

## Import/Export

The database supports:
- Full CSV export
- Filtered exports by category, date range, or signal type
- Import from CSV with validation
- Merge operations with duplicate detection

## Backup Format

Backup files (`rfid_backup_*.csv`) use the exact same format as the main database file.
