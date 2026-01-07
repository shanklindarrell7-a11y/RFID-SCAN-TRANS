#ifndef EXPORT_H
#define EXPORT_H

#include "rfid_scan_trans.h"
#include <storage/storage.h>

typedef enum {
    EXPORT_FORMAT_SUB,      // Flipper SubGHz format
    EXPORT_FORMAT_CSV,      // CSV format
    EXPORT_FORMAT_JSON,     // JSON format
    EXPORT_FORMAT_HEX,      // Hexadecimal dump
    EXPORT_FORMAT_BIN,      // Binary format
} ExportFormat;

/**
 * Export signal to file
 * @param signal Signal to export
 * @param storage Storage instance
 * @param path Output file path
 * @param format Export format
 * @return true if successful
 */
bool export_signal(
    const SignalData* signal,
    Storage* storage,
    const char* path,
    ExportFormat format);

/**
 * Export entire library to file
 * @param library Library to export
 * @param storage Storage instance
 * @param path Output file path
 * @param format Export format
 * @return true if successful
 */
bool export_library(
    const SignalLibrary* library,
    Storage* storage,
    const char* path,
    ExportFormat format);

/**
 * Get format name
 * @param format Export format
 * @return Format name string
 */
const char* export_get_format_name(ExportFormat format);

#endif // EXPORT_H
