#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

void rfid_library_submenu_callback(void* context, uint32_t index) {
    RfidApp* app = context;
    app->state->selected_signal_index = index;
    scene_manager_next_scene(app->scene_manager, RfidSceneSignalInfo);
}

void rfid_scene_library_on_enter(void* context) {
    RfidApp* app = context;
    Submenu* submenu = app->submenu;
    
    submenu_reset(submenu);
    submenu_set_header(submenu, "Signal Library");
    
    if(app->state->signal_count == 0) {
        submenu_add_item(
            submenu,
            "No signals saved",
            0,
            NULL,
            app
        );
    } else {
        // Add favorites first
        for(uint32_t i = 0; i < app->state->signal_count; i++) {
            RfidSignal* signal = &app->state->signals[i];
            if(signal->is_favorite) {
                FuriString* label = furi_string_alloc();
                furi_string_printf(label, "★ %s (%s)", signal->name, signal->protocol_name);
                submenu_add_item(
                    submenu,
                    furi_string_get_cstr(label),
                    i,
                    rfid_library_submenu_callback,
                    app
                );
                furi_string_free(label);
            }
        }
        
        // Add non-favorites
        for(uint32_t i = 0; i < app->state->signal_count; i++) {
            RfidSignal* signal = &app->state->signals[i];
            if(!signal->is_favorite) {
                FuriString* label = furi_string_alloc();
                furi_string_printf(label, "%s (%s)", signal->name, signal->protocol_name);
                submenu_add_item(
                    submenu,
                    furi_string_get_cstr(label),
                    i,
                    rfid_library_submenu_callback,
                    app
                );
                furi_string_free(label);
            }
        }
    }
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewSubmenu);
}

bool rfid_scene_library_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_library_on_exit(void* context) {
    RfidApp* app = context;
    submenu_reset(app->submenu);
}
