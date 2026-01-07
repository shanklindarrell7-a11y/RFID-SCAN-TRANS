#pragma once

#include "export_formats.h"
#include "rfid_sniffer.h"
#include <storage/storage.h>

/**
 * Export a captured signal to all supported formats
 * @param storage Storage API handle
 * @param signal Signal data to export
 * @param protocol RFID protocol type
 */
void export_signal_all_formats(Storage* storage, const SignalData* signal, RFIDProtocol protocol);

/**
 * Export single signal to specific format
 * @param storage Storage API handle
 * @param signal Signal data to export
 * @param protocol RFID protocol type
 * @param format Export format to use
 * @param custom_path Optional custom file path (NULL for auto-generation)
 * @return true if export succeeded
 */
bool export_signal_format(
    Storage* storage,
    const SignalData* signal,
    RFIDProtocol protocol,
    ExportFormat format,
    const char* custom_path);

/**
 * Batch export multiple signals to CSV file
 * @param storage Storage API handle
 * @param signals Array of signal data
 * @param signal_count Number of signals in array
 * @param protocol RFID protocol type
 * @param file_path Output CSV file path
 */
void export_batch_to_csv(
    Storage* storage,
    const SignalData* signals,
    size_t signal_count,
    RFIDProtocol protocol,
    const char* file_path);

/**
 * Create sample signal data for testing
 * @param protocol RFID protocol type
 * @return Sample signal data
 */
SignalData create_sample_signal(RFIDProtocol protocol);

/**
 * Test all export formats with sample data
 * @param storage Storage API handle
 */
void test_export_formats(Storage* storage);
