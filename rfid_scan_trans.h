#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>

#define TAG "RfidScanTrans"
#define RFID_APP_FOLDER "/ext/rfid_signals"
#define RFID_APP_EXTENSION ".rfid"
#define MAX_SIGNALS 100
#define MAX_SIGNAL_NAME 32
#define MAX_PROTOCOL_NAME 16
#define MAX_UID_SIZE 32
#define MAX_NOTES_SIZE 128

// Signal protocols
typedef enum {
    RfidProtocolEM4100,
    RfidProtocolHIDProx,
    RfidProtocolIOProx,
    RfidProtocolIndala,
    RfidProtocolT5577,
    RfidProtocolUnknown,
} RfidProtocol;

// Duplication modes
typedef enum {
    DupModeClone,    // Write to physical writable RFID cards
    DupModeEmulate,  // Continuous broadcast (simulates card presence)
    DupModeReplay,   // One-time transmission of captured signal
    DupModeGenerate, // Create variants (incrementing IDs, etc.)
} DuplicationMode;

// Signal metadata structure
typedef struct {
    char name[MAX_SIGNAL_NAME];
    char protocol_name[MAX_PROTOCOL_NAME];
    RfidProtocol protocol;
    uint8_t uid[MAX_UID_SIZE];
    uint8_t uid_len;
    uint32_t frequency;
    uint8_t signal_strength;
    bool is_favorite;
    uint32_t timestamp;
    char notes[MAX_NOTES_SIZE];
    bool encrypted;
} RfidSignal;

// Application state
typedef struct {
    RfidSignal signals[MAX_SIGNALS];
    uint32_t signal_count;
    uint32_t selected_signal_index;
    
    // Transmission settings
    uint8_t power_level;    // 0-7
    uint8_t repeat_count;
    uint16_t delay_ms;
    bool continuous_mode;
    
    // UI state
    char text_input_buffer[MAX_SIGNAL_NAME];
    bool auto_save_enabled;
} RfidAppState;

// Main application structure
typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    TextInput* text_input;
    Popup* popup;
    VariableItemList* variable_item_list;
    Widget* widget;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    
    RfidAppState* state;
} RfidApp;

// Views
typedef enum {
    RfidViewSubmenu,
    RfidViewTextInput,
    RfidViewPopup,
    RfidViewVariableItemList,
    RfidViewWidget,
} RfidView;

// Scenes
typedef enum {
    RfidSceneStart,
    RfidSceneCapture,
    RfidSceneLibrary,
    RfidSceneSignalInfo,
    RfidSceneExport,
    RfidSceneTransmit,
    RfidSceneSettings,
    RfidSceneEditName,
    RfidSceneEditNotes,
    RfidSceneCount,
} RfidScene;

// Function prototypes
RfidApp* rfid_app_alloc();
void rfid_app_free(RfidApp* app);

// Signal management
bool rfid_signal_save(RfidApp* app, RfidSignal* signal);
bool rfid_signal_load(RfidApp* app, const char* name);
bool rfid_signal_delete(RfidApp* app, uint32_t index);
void rfid_signal_load_all(RfidApp* app);
void rfid_signal_backup_all(RfidApp* app);

// Export functions
bool rfid_export_flipper(RfidApp* app, RfidSignal* signal, const char* path);
bool rfid_export_csv(RfidApp* app, const char* path);
bool rfid_export_wiegand(RfidApp* app, RfidSignal* signal, const char* path);
bool rfid_export_proxmark3(RfidApp* app, RfidSignal* signal, const char* path);
bool rfid_export_json(RfidApp* app, RfidSignal* signal, const char* path);

// Transmission functions
bool rfid_transmit(RfidApp* app, RfidSignal* signal, DuplicationMode mode);
void rfid_transmit_stop(RfidApp* app);

// Utility functions
const char* rfid_protocol_to_string(RfidProtocol protocol);
RfidProtocol rfid_detect_protocol(uint8_t* uid, uint8_t uid_len);
bool rfid_validate_signal(RfidSignal* signal);
void rfid_generate_variant(RfidSignal* source, RfidSignal* dest, int32_t offset);
