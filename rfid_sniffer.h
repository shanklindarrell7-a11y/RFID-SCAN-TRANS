#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

// Protocol definitions
typedef enum {
    RFIDProtocolEM4100,      // 125 kHz
    RFIDProtocolHIDProx,     // 125 kHz
    RFIDProtocolIndala,      // 125 kHz
    RFIDProtocolMifareClassic, // 13.56 MHz - via Flipper's NFC hardware
    RFIDProtocolISO14443A,   // 13.56 MHz
    RFIDProtocolCount,
} RFIDProtocol;

// Protocol information structure
typedef struct {
    RFIDProtocol protocol;
    const char* name;
    uint32_t frequency; // in kHz
    bool is_nfc_based;  // true for 13.56 MHz protocols
} ProtocolInfo;

// Signal data structure
typedef struct {
    uint8_t data[64];
    size_t data_length;
    int8_t rssi;        // Signal strength in dBm
    uint32_t timestamp;
} SignalData;

// Application state structure with advanced features
typedef struct {
    // Basic configuration
    RFIDProtocol current_protocol;
    bool is_sniffing;
    
    // Advanced features from requirements
    bool save_to_sd;
    bool auto_protocol_detect;
    uint32_t center_frequency;
    uint32_t bandwidth;
    bool raw_bitstream_logging;
    
    // Signal analysis
    SignalData last_signal;
    int8_t rssi_history[100]; // For visualization
    size_t rssi_history_index;
    
    // Statistics
    uint32_t signals_captured;
    uint32_t signals_saved;
    
    // File handling
    File* log_file;
    char log_file_path[128];
} RFIDSnifferState;

// Main application structure
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    VariableItemList* variable_item_list;
    Widget* widget;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Storage* storage;
    
    RFIDSnifferState* state;
} RFIDSnifferApp;

// Protocol information table
static const ProtocolInfo protocol_info_table[RFIDProtocolCount] = {
    {RFIDProtocolEM4100, "EM4100", 125, false},
    {RFIDProtocolHIDProx, "HID Prox", 125, false},
    {RFIDProtocolIndala, "Indala", 125, false},
    {RFIDProtocolMifareClassic, "Mifare Classic", 13560, true},
    {RFIDProtocolISO14443A, "ISO14443A", 13560, true},
};

// Function prototypes
RFIDSnifferApp* rfid_sniffer_app_alloc(void);
void rfid_sniffer_app_free(RFIDSnifferApp* app);
int32_t rfid_sniffer_app(void* p);

// Protocol functions
const char* rfid_protocol_get_name(RFIDProtocol protocol);
uint32_t rfid_protocol_get_frequency(RFIDProtocol protocol);
bool rfid_protocol_is_nfc_based(RFIDProtocol protocol);

// Sniffing functions
void rfid_sniffer_start(RFIDSnifferApp* app);
void rfid_sniffer_stop(RFIDSnifferApp* app);
bool rfid_sniffer_process_signal(RFIDSnifferApp* app);

// Signal analysis functions
void rfid_sniffer_update_rssi(RFIDSnifferApp* app, int8_t rssi);
int8_t rfid_sniffer_get_current_rssi(RFIDSnifferApp* app);

// File operations
bool rfid_sniffer_open_log_file(RFIDSnifferApp* app);
void rfid_sniffer_close_log_file(RFIDSnifferApp* app);
bool rfid_sniffer_save_signal(RFIDSnifferApp* app, const SignalData* signal);

// Auto-detection
RFIDProtocol rfid_sniffer_auto_detect_protocol(const SignalData* signal);
