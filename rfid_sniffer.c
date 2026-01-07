#include "rfid_sniffer.h"
#include <furi_hal.h>

#define TAG "RFIDSniffer"

// Protocol name getters
const char* rfid_protocol_get_name(RFIDProtocol protocol) {
    if(protocol < RFIDProtocolCount) {
        return protocol_info_table[protocol].name;
    }
    return "Unknown";
}

uint32_t rfid_protocol_get_frequency(RFIDProtocol protocol) {
    if(protocol < RFIDProtocolCount) {
        return protocol_info_table[protocol].frequency;
    }
    return 0;
}

bool rfid_protocol_is_nfc_based(RFIDProtocol protocol) {
    if(protocol < RFIDProtocolCount) {
        return protocol_info_table[protocol].is_nfc_based;
    }
    return false;
}

// State allocation
static RFIDSnifferState* rfid_sniffer_state_alloc(void) {
    RFIDSnifferState* state = malloc(sizeof(RFIDSnifferState));
    
    // Initialize basic state
    state->current_protocol = RFIDProtocolEM4100;
    state->is_sniffing = false;
    
    // Initialize advanced features
    state->save_to_sd = false;
    state->auto_protocol_detect = true;
    state->center_frequency = 125000; // 125 kHz default
    state->bandwidth = 10000; // 10 kHz bandwidth
    state->raw_bitstream_logging = false;
    
    // Initialize signal data
    memset(&state->last_signal, 0, sizeof(SignalData));
    memset(state->rssi_history, 0, sizeof(state->rssi_history));
    state->rssi_history_index = 0;
    
    // Initialize statistics
    state->signals_captured = 0;
    state->signals_saved = 0;
    
    // Initialize file handling
    state->log_file = NULL;
    memset(state->log_file_path, 0, sizeof(state->log_file_path));
    
    return state;
}

static void rfid_sniffer_state_free(RFIDSnifferState* state) {
    free(state);
}

// Application allocation
RFIDSnifferApp* rfid_sniffer_app_alloc(void) {
    RFIDSnifferApp* app = malloc(sizeof(RFIDSnifferApp));
    
    // Allocate state
    app->state = rfid_sniffer_state_alloc();
    
    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->storage = furi_record_open(RECORD_STORAGE);
    
    // Initialize views
    app->submenu = submenu_alloc();
    app->variable_item_list = variable_item_list_alloc();
    app->widget = widget_alloc();
    
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    return app;
}

void rfid_sniffer_app_free(RFIDSnifferApp* app) {
    if(!app) return;
    
    // Stop sniffing if active
    if(app->state->is_sniffing) {
        rfid_sniffer_stop(app);
    }
    
    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    submenu_free(app->submenu);
    variable_item_list_free(app->variable_item_list);
    widget_free(app->widget);
    
    // Free view dispatcher
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    
    // Free state
    rfid_sniffer_state_free(app->state);
    
    free(app);
}

// RSSI tracking for signal strength visualization
void rfid_sniffer_update_rssi(RFIDSnifferApp* app, int8_t rssi) {
    RFIDSnifferState* state = app->state;
    
    state->rssi_history[state->rssi_history_index] = rssi;
    state->rssi_history_index = (state->rssi_history_index + 1) % 100;
    
    state->last_signal.rssi = rssi;
}

int8_t rfid_sniffer_get_current_rssi(RFIDSnifferApp* app) {
    return app->state->last_signal.rssi;
}

// File operations for SD card logging
bool rfid_sniffer_open_log_file(RFIDSnifferApp* app) {
    RFIDSnifferState* state = app->state;
    
    if(!state->save_to_sd) {
        return false;
    }
    
    if(state->log_file != NULL) {
        return true; // Already open
    }
    
    // Create directory if it doesn't exist
    storage_common_mkdir(app->storage, "/ext/rfid_logs");
    
    // Generate filename with timestamp
    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    snprintf(
        state->log_file_path,
        sizeof(state->log_file_path),
        "/ext/rfid_logs/rfid_%04d%02d%02d_%02d%02d%02d.log",
        datetime.year,
        datetime.month,
        datetime.day,
        datetime.hour,
        datetime.minute,
        datetime.second);
    
    // Open file
    state->log_file = storage_file_alloc(app->storage);
    if(!storage_file_open(
           state->log_file, state->log_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_free(state->log_file);
        state->log_file = NULL;
        return false;
    }
    
    // Write header
    const char* header = "RFID Sniffer Log\n";
    storage_file_write(state->log_file, header, strlen(header));
    
    return true;
}

void rfid_sniffer_close_log_file(RFIDSnifferApp* app) {
    RFIDSnifferState* state = app->state;
    
    if(state->log_file != NULL) {
        storage_file_close(state->log_file);
        storage_file_free(state->log_file);
        state->log_file = NULL;
    }
}

bool rfid_sniffer_save_signal(RFIDSnifferApp* app, const SignalData* signal) {
    RFIDSnifferState* state = app->state;
    
    if(!state->save_to_sd) {
        return false;
    }
    
    if(!rfid_sniffer_open_log_file(app)) {
        return false;
    }
    
    // Format signal data
    char buffer[256];
    int len = snprintf(
        buffer,
        sizeof(buffer),
        "Protocol: %s, RSSI: %d dBm, Time: %lu, Length: %zu\nData: ",
        rfid_protocol_get_name(state->current_protocol),
        signal->rssi,
        signal->timestamp,
        signal->data_length);
    
    // Write signal metadata
    storage_file_write(state->log_file, buffer, len);
    
    // Write hex data
    for(size_t i = 0; i < signal->data_length; i++) {
        len = snprintf(buffer, sizeof(buffer), "%02X ", signal->data[i]);
        storage_file_write(state->log_file, buffer, len);
    }
    
    // Write raw bitstream if enabled
    if(state->raw_bitstream_logging) {
        storage_file_write(state->log_file, "\nRaw: ", 6);
        for(size_t i = 0; i < signal->data_length; i++) {
            for(int bit = 7; bit >= 0; bit--) {
                char bit_char = (signal->data[i] & (1 << bit)) ? '1' : '0';
                storage_file_write(state->log_file, &bit_char, 1);
            }
        }
    }
    
    storage_file_write(state->log_file, "\n\n", 2);
    
    state->signals_saved++;
    
    return true;
}

// Auto-detection of protocol based on signal characteristics
RFIDProtocol rfid_sniffer_auto_detect_protocol(const SignalData* signal) {
    if(!signal || signal->data_length == 0) {
        return RFIDProtocolEM4100; // Default
    }
    
    // EM4100: Typically 5 bytes (40 bits)
    if(signal->data_length == 5) {
        return RFIDProtocolEM4100;
    }
    
    // HID Prox: Typically 3 bytes (26 bits with parity)
    if(signal->data_length == 3 || signal->data_length == 4) {
        return RFIDProtocolHIDProx;
    }
    
    // Indala: Variable length, typically longer
    if(signal->data_length >= 8) {
        return RFIDProtocolIndala;
    }
    
    // For 13.56 MHz protocols, check for specific patterns
    // ISO14443A and Mifare have different ATS responses
    if(signal->data_length >= 4) {
        // Simple heuristic: check for Mifare Classic SAK
        if(signal->data[0] == 0x08 || signal->data[0] == 0x18) {
            return RFIDProtocolMifareClassic;
        }
        return RFIDProtocolISO14443A;
    }
    
    return RFIDProtocolEM4100; // Default fallback
}

// Sniffing control functions
void rfid_sniffer_start(RFIDSnifferApp* app) {
    RFIDSnifferState* state = app->state;
    
    if(state->is_sniffing) {
        return; // Already sniffing
    }
    
    FURI_LOG_I(TAG, "Starting RFID sniffer");
    FURI_LOG_I(
        TAG,
        "Protocol: %s, Freq: %lu kHz",
        rfid_protocol_get_name(state->current_protocol),
        rfid_protocol_get_frequency(state->current_protocol));
    
    // Update center frequency based on protocol
    uint32_t freq = rfid_protocol_get_frequency(state->current_protocol);
    state->center_frequency = freq;
    
    // Open log file if SD saving is enabled
    if(state->save_to_sd) {
        rfid_sniffer_open_log_file(app);
    }
    
    state->is_sniffing = true;
    
    // Notify user
    notification_message(app->notifications, &sequence_success);
}

void rfid_sniffer_stop(RFIDSnifferApp* app) {
    RFIDSnifferState* state = app->state;
    
    if(!state->is_sniffing) {
        return; // Not sniffing
    }
    
    FURI_LOG_I(TAG, "Stopping RFID sniffer");
    FURI_LOG_I(
        TAG, "Captured: %lu, Saved: %lu", state->signals_captured, state->signals_saved);
    
    state->is_sniffing = false;
    
    // Close log file
    rfid_sniffer_close_log_file(app);
    
    // Notify user
    notification_message(app->notifications, &sequence_single_vibro);
}

// Signal processing
bool rfid_sniffer_process_signal(RFIDSnifferApp* app) {
    RFIDSnifferState* state = app->state;
    
    if(!state->is_sniffing) {
        return false;
    }
    
    // In a real implementation, this would read from RFID hardware
    // For this template, we simulate signal detection
    
    // Simulated signal data (in real implementation, read from hardware)
    SignalData signal;
    signal.data_length = 5; // Example: EM4100 tag
    signal.timestamp = furi_get_tick();
    signal.rssi = -50; // Example RSSI value
    
    // Example data (in real implementation, actual card data)
    for(size_t i = 0; i < signal.data_length; i++) {
        signal.data[i] = 0xAA; // Placeholder
    }
    
    // Auto-detect protocol if enabled
    if(state->auto_protocol_detect) {
        RFIDProtocol detected = rfid_sniffer_auto_detect_protocol(&signal);
        if(detected != state->current_protocol) {
            FURI_LOG_I(
                TAG,
                "Auto-detected protocol: %s",
                rfid_protocol_get_name(detected));
            state->current_protocol = detected;
        }
    }
    
    // Update RSSI history
    rfid_sniffer_update_rssi(app, signal.rssi);
    
    // Store signal
    memcpy(&state->last_signal, &signal, sizeof(SignalData));
    state->signals_captured++;
    
    // Save to SD card if enabled
    if(state->save_to_sd) {
        rfid_sniffer_save_signal(app, &signal);
    }
    
    return true;
}

// Main application entry point
int32_t rfid_sniffer_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I(TAG, "RFID Sniffer starting");
    
    RFIDSnifferApp* app = rfid_sniffer_app_alloc();
    
    FURI_LOG_I(TAG, "App initialized");
    FURI_LOG_I(TAG, "Supported protocols:");
    for(size_t i = 0; i < RFIDProtocolCount; i++) {
        FURI_LOG_I(
            TAG,
            "  - %s (%lu kHz)",
            protocol_info_table[i].name,
            protocol_info_table[i].frequency);
    }
    
    // Main loop placeholder
    // In a real implementation, this would handle UI events and signal processing
    FURI_LOG_I(TAG, "RFID Sniffer ready");
    
    // Example: Start sniffing
    rfid_sniffer_start(app);
    
    // Simulate some signal captures
    for(int i = 0; i < 5; i++) {
        furi_delay_ms(1000);
        rfid_sniffer_process_signal(app);
    }
    
    // Stop sniffing
    rfid_sniffer_stop(app);
    
    FURI_LOG_I(TAG, "RFID Sniffer stopping");
    
    rfid_sniffer_app_free(app);
    
    return 0;
}
