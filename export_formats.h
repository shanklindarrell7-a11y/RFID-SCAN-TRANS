#pragma once

#include <furi.h>
#include <storage/storage.h>

// Forward declarations
typedef struct SignalData SignalData;
typedef enum RFIDProtocol RFIDProtocol;

// Export format types
typedef enum {
    ExportFormatFlipper,    // .sub files for Flipper Zero apps
    ExportFormatCSV,        // CSV database format
    ExportFormatWiegand,    // Industry-standard Wiegand output
    ExportFormatProxmark3,  // Proxmark3 command scripts
    ExportFormatJSON,       // JSON metadata
    ExportFormatCount,
} ExportFormat;

// Export result structure
typedef struct {
    bool success;
    char error_message[128];
    size_t bytes_written;
} ExportResult;

/**
 * Export signal data to Flipper format (.sub file)
 * @param storage Storage API handle
 * @param file_path Output file path
 * @param protocol RFID protocol type
 * @param signal Signal data to export
 * @return Export result with success status
 */
ExportResult export_to_flipper_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal);

/**
 * Export signal data to CSV format
 * @param storage Storage API handle
 * @param file_path Output file path
 * @param protocol RFID protocol type
 * @param signal Signal data to export
 * @param append If true, append to existing file
 * @return Export result with success status
 */
ExportResult export_to_csv_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal,
    bool append);

/**
 * Export signal data to Wiegand format
 * @param storage Storage API handle
 * @param file_path Output file path
 * @param protocol RFID protocol type
 * @param signal Signal data to export
 * @return Export result with success status
 */
ExportResult export_to_wiegand_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal);

/**
 * Export signal data as Proxmark3 commands
 * @param storage Storage API handle
 * @param file_path Output file path
 * @param protocol RFID protocol type
 * @param signal Signal data to export
 * @return Export result with success status
 */
ExportResult export_to_proxmark3_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal);

/**
 * Export signal data to JSON format
 * @param storage Storage API handle
 * @param file_path Output file path
 * @param protocol RFID protocol type
 * @param signal Signal data to export
 * @return Export result with success status
 */
ExportResult export_to_json_format(
    Storage* storage,
    const char* file_path,
    RFIDProtocol protocol,
    const SignalData* signal);

/**
 * Get file extension for export format
 * @param format Export format type
 * @return File extension string (e.g., ".sub", ".csv")
 */
const char* export_format_get_extension(ExportFormat format);

/**
 * Get format name for display
 * @param format Export format type
 * @return Human-readable format name
 */
const char* export_format_get_name(ExportFormat format);
