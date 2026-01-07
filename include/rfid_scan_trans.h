#ifndef RFID_SCAN_TRANS_H
#define RFID_SCAN_TRANS_H

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <storage/storage.h>

#define TAG "RFID_Scan_Trans"

// Performance constants
#define MAX_LIBRARY_SIGNALS 1000
#define CAPTURE_BUFFER_SIZE 2048
#define MAX_EXPORT_FORMATS 5
#define TRANSMIT_RANGE_METERS 1

// Library file path
#define LIBRARY_PATH "/ext/subghz/rfid_scan_trans_library.txt"

typedef enum {
    PROTOCOL_UNKNOWN,
    PROTOCOL_EM4100,
    PROTOCOL_HID,
    PROTOCOL_INDALA,
    PROTOCOL_RAW,
} ProtocolType;

typedef struct {
    ProtocolType type;
    char name[32];
    uint8_t data[256];
    size_t data_size;
    uint32_t frequency;
} SignalData;

typedef struct {
    SignalData* signals;
    size_t count;
    size_t capacity;
} SignalLibrary;

typedef struct RfidScanTransApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Storage* storage;
    SignalLibrary library;
} RfidScanTransApp;

#endif // RFID_SCAN_TRANS_H
