#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

enum TransmitSubmenuIndex {
    TransmitIndexClone,
    TransmitIndexEmulate,
    TransmitIndexReplay,
    TransmitIndexGenerate,
};

void rfid_transmit_callback(void* context, uint32_t index) {
    RfidApp* app = context;
    RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
    
    DuplicationMode mode;
    const char* mode_name;
    
    switch(index) {
        case TransmitIndexClone:
            mode = DupModeClone;
            mode_name = "Clone Mode";
            break;
        case TransmitIndexEmulate:
            mode = DupModeEmulate;
            mode_name = "Emulate Mode";
            break;
        case TransmitIndexReplay:
            mode = DupModeReplay;
            mode_name = "Replay Mode";
            break;
        case TransmitIndexGenerate:
            mode = DupModeGenerate;
            mode_name = "Generate Mode";
            break;
        default:
            return;
    }
    
    // Show transmission popup
    Popup* popup = app->popup;
    popup_reset(popup);
    popup_set_header(popup, mode_name, 64, 10, AlignCenter, AlignTop);
    popup_set_text(popup, "Transmitting...", 64, 30, AlignCenter, AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewPopup);
    
    // Transmit signal
    bool success = rfid_transmit(app, signal, mode);
    
    if(success) {
        popup_set_text(popup, "Success!", 64, 30, AlignCenter, AlignTop);
    } else {
        popup_set_text(popup, "Failed!", 64, 30, AlignCenter, AlignTop);
    }
    
    furi_delay_ms(1500);
    scene_manager_previous_scene(app->scene_manager);
}

void rfid_scene_transmit_on_enter(void* context) {
    RfidApp* app = context;
    Submenu* submenu = app->submenu;
    
    submenu_reset(submenu);
    submenu_set_header(submenu, "Duplication Mode");
    
    submenu_add_item(
        submenu,
        "Clone (Write to Card)",
        TransmitIndexClone,
        rfid_transmit_callback,
        app
    );
    
    submenu_add_item(
        submenu,
        "Emulate (Continuous)",
        TransmitIndexEmulate,
        rfid_transmit_callback,
        app
    );
    
    submenu_add_item(
        submenu,
        "Replay (One-time)",
        TransmitIndexReplay,
        rfid_transmit_callback,
        app
    );
    
    submenu_add_item(
        submenu,
        "Generate (Variants)",
        TransmitIndexGenerate,
        rfid_transmit_callback,
        app
    );
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewSubmenu);
}

bool rfid_scene_transmit_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_transmit_on_exit(void* context) {
    RfidApp* app = context;
    submenu_reset(app->submenu);
}
