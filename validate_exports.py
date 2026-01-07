#!/usr/bin/env python3
"""
Validation script for RFID export formats.
This script validates the output of various export formats against expected structures.
"""

import os
import json
import csv
import sys
from pathlib import Path

# Default export directory path
DEFAULT_EXPORT_DIR = "/ext/rfid_exports"

def validate_flipper_format(file_path):
    """Validate Flipper SubGhz format (.sub)"""
    print(f"Validating Flipper format: {file_path}")
    
    required_fields = [
        "Filetype: Flipper SubGhz RAW File",
        "Version:",
        "Frequency:",
        "Protocol:",
        "RAW_Data:"
    ]
    
    try:
        with open(file_path, 'r') as f:
            content = f.read()
            
        for field in required_fields:
            if field not in content:
                print(f"  ❌ Missing required field: {field}")
                return False
        
        print("  ✅ Valid Flipper format")
        return True
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def validate_csv_format(file_path):
    """Validate CSV format"""
    print(f"Validating CSV format: {file_path}")
    
    required_headers = [
        "Timestamp", "Protocol", "Frequency_kHz", 
        "Data_Hex", "Data_Length", "RSSI_dBm"
    ]
    
    try:
        with open(file_path, 'r') as f:
            reader = csv.DictReader(f)
            headers = reader.fieldnames
            
            if not headers:
                print("  ❌ No headers found")
                return False
            
            for header in required_headers:
                if header not in headers:
                    print(f"  ❌ Missing required header: {header}")
                    return False
            
            # Try to read at least one row
            row_count = sum(1 for _ in reader)
            print(f"  ℹ️  Contains {row_count} data rows")
        
        print("  ✅ Valid CSV format")
        return True
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def validate_wiegand_format(file_path):
    """Validate Wiegand format"""
    print(f"Validating Wiegand format: {file_path}")
    
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Check for required sections
        if "# Wiegand Format Export" not in content:
            print("  ❌ Missing format header")
            return False
        
        if "Protocol:" not in content:
            print("  ❌ Missing protocol information")
            return False
        
        if "Hex:" not in content and "Binary:" not in content:
            print("  ❌ Missing data representation")
            return False
        
        print("  ✅ Valid Wiegand format")
        return True
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def validate_proxmark3_format(file_path):
    """Validate Proxmark3 script format"""
    print(f"Validating Proxmark3 format: {file_path}")
    
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Check for script header
        if "# Proxmark3 Command Script" not in content:
            print("  ❌ Missing script header")
            return False
        
        # Check for at least one command
        pm3_commands = ['lf em', 'lf hid', 'lf indala', 'hf mf', 'hf 14a']
        has_command = any(cmd in content for cmd in pm3_commands)
        
        if not has_command:
            print("  ❌ No Proxmark3 commands found")
            return False
        
        print("  ✅ Valid Proxmark3 format")
        return True
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def validate_json_format(file_path):
    """Validate JSON format"""
    print(f"Validating JSON format: {file_path}")
    
    required_fields = [
        "protocol", "frequency_khz", "timestamp", 
        "rssi_dbm", "data", "metadata"
    ]
    
    try:
        with open(file_path, 'r') as f:
            data = json.load(f)
        
        # Check top-level required fields
        for field in required_fields:
            if field not in data:
                print(f"  ❌ Missing required field: {field}")
                return False
        
        # Check data structure
        if "hex" not in data["data"] or "length" not in data["data"]:
            print("  ❌ Invalid data structure")
            return False
        
        # Validate data types
        if not isinstance(data["timestamp"], int):
            print("  ❌ Invalid timestamp type")
            return False
        
        if not isinstance(data["rssi_dbm"], int):
            print("  ❌ Invalid RSSI type")
            return False
        
        print("  ✅ Valid JSON format")
        return True
    except json.JSONDecodeError as e:
        print(f"  ❌ Invalid JSON: {e}")
        return False
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def main():
    """Main validation function"""
    print("=" * 60)
    print("RFID Export Format Validator")
    print("=" * 60)
    print()
    
    # Check for test export directory
    export_dir = Path(DEFAULT_EXPORT_DIR)
    
    if len(sys.argv) > 1:
        export_dir = Path(sys.argv[1])
    
    if not export_dir.exists():
        print(f"Export directory not found: {export_dir}")
        print("Run the export examples first to generate test files.")
        return 1
    
    print(f"Scanning directory: {export_dir}")
    print()
    
    # Track validation results
    results = {
        ".sub": [],
        ".csv": [],
        ".wiegand": [],
        ".pm3": [],
        ".json": []
    }
    
    # Validate each file type
    for ext, validator in [
        (".sub", validate_flipper_format),
        (".csv", validate_csv_format),
        (".wiegand", validate_wiegand_format),
        (".pm3", validate_proxmark3_format),
        (".json", validate_json_format)
    ]:
        files = list(export_dir.glob(f"*{ext}"))
        if files:
            print(f"\n{ext.upper()} Files:")
            print("-" * 60)
            for file_path in files:
                result = validator(file_path)
                results[ext].append(result)
        else:
            print(f"\nNo {ext} files found")
    
    # Summary
    print()
    print("=" * 60)
    print("Validation Summary")
    print("=" * 60)
    
    all_passed = True
    for ext, result_list in results.items():
        if result_list:
            passed = sum(result_list)
            total = len(result_list)
            status = "✅" if passed == total else "❌"
            print(f"{status} {ext}: {passed}/{total} files valid")
            all_passed = all_passed and (passed == total)
        else:
            print(f"⚠️  {ext}: No files tested")
    
    print()
    if all_passed and any(results.values()):
        print("✅ All validation checks passed!")
        return 0
    else:
        print("❌ Some validation checks failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())
