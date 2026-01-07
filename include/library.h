#ifndef LIBRARY_H
#define LIBRARY_H

#include "rfid_scan_trans.h"
#include <storage/storage.h>

/**
 * Initialize signal library
 * @param library Library structure to initialize
 * @return true if successful
 */
bool library_init(SignalLibrary* library);

/**
 * Free library resources
 * @param library Library to free
 */
void library_free(SignalLibrary* library);

/**
 * Add signal to library
 * @param library Target library
 * @param signal Signal to add
 * @return true if successful
 */
bool library_add_signal(SignalLibrary* library, const SignalData* signal);

/**
 * Load library from storage
 * @param library Target library
 * @param storage Storage instance
 * @return true if successful
 */
bool library_load(SignalLibrary* library, Storage* storage);

/**
 * Save library to storage
 * @param library Library to save
 * @param storage Storage instance
 * @return true if successful
 */
bool library_save(const SignalLibrary* library, Storage* storage);

/**
 * Get signal by index
 * @param library Library
 * @param index Signal index
 * @return Signal data or NULL
 */
const SignalData* library_get_signal(const SignalLibrary* library, size_t index);

/**
 * Get library count
 * @param library Library
 * @return Number of signals in library
 */
size_t library_get_count(const SignalLibrary* library);

#endif // LIBRARY_H
