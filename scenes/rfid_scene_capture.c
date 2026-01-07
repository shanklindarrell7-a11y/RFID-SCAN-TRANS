#include "../rfid_scan_trans.h"
#include "rfid_scene.h"
#include <furi_hal_rfid.h>

void rfid_scene_capture_on_enter(void* context) {
    RfidApp* app = context;
    Popup* popup = app->popup;
    
    popup_reset(popup);
    popup_set_header(popup, "Scanning RFID", 64, 10, AlignCenter, AlignTop);
    popup_set_text(popup, "Hold card near device", 64, 30, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewPopup);
    
    // Simulate RFID capture (in real implementation, this would use actual RFID hardware)
    notification_message(app->notifications, &sequence_blink_start_blue);
    
    // Simulate card detection after a delay
    // In real implementation, this would be asynchronous and hardware-driven
}

bool rfid_scene_capture_on_event(void* context, SceneManagerEvent event) {
    RfidApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == 1) { // Card detected
            // Create new signal
            if(app->state->signal_count < MAX_SIGNALS) {
                RfidSignal* signal = &app->state->signals[app->state->signal_count];
                
                // Simulate captured data (in real implementation, read from hardware)
                snprintf(signal->name, MAX_SIGNAL_NAME, "Card_%lu", furi_get_tick() / 1000);
                
                // Example UID (EM4100 format)
                uint8_t example_uid[] = {0x12, 0x34, 0x56, 0x78, 0x9A};
                memcpy(signal->uid, example_uid, sizeof(example_uid));
                signal->uid_len = sizeof(example_uid);
                
                // Auto-detect protocol
                signal->protocol = rfid_detect_protocol(signal->uid, signal->uid_len);
                snprintf(signal->protocol_name, MAX_PROTOCOL_NAME, "%s", 
                        rfid_protocol_to_string(signal->protocol));
                
                // Set metadata
                signal->frequency = 125000; // 125 kHz LF RFID
                signal->signal_strength = 75; // Example strength
                signal->is_favorite = false;
                signal->timestamp = furi_get_tick();
                signal->encrypted = false;
                signal->notes[0] = '\0';
                
                app->state->signal_count++;
                app->state->selected_signal_index = app->state->signal_count - 1;
                
                // Auto-save if enabled
                if(app->state->auto_save_enabled) {
                    rfid_signal_save(app, signal);
                }
                
                notification_message(app->notifications, &sequence_success);
                
                // Go to signal info scene
                scene_manager_next_scene(app->scene_manager, RfidSceneSignalInfo);
                consumed = true;
            }
        }
    }
    
    return consumed;
}

void rfid_scene_capture_on_exit(void* context) {
    RfidApp* app = context;
    popup_reset(app->popup);
    notification_message(app->notifications, &sequence_blink_stop);
}
