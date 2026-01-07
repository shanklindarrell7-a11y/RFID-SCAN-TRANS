#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

enum SignalInfoSubmenuIndex {
    SignalInfoIndexTransmit,
    SignalInfoIndexExport,
    SignalInfoIndexEditName,
    SignalInfoIndexEditNotes,
    SignalInfoIndexToggleFavorite,
    SignalInfoIndexDelete,
};

void rfid_signal_info_submenu_callback(void* context, uint32_t index) {
    RfidApp* app = context;
    
    switch(index) {
        case SignalInfoIndexTransmit:
            scene_manager_next_scene(app->scene_manager, RfidSceneTransmit);
            break;
        case SignalInfoIndexExport:
            scene_manager_next_scene(app->scene_manager, RfidSceneExport);
            break;
        case SignalInfoIndexEditName:
            scene_manager_next_scene(app->scene_manager, RfidSceneEditName);
            break;
        case SignalInfoIndexEditNotes:
            scene_manager_next_scene(app->scene_manager, RfidSceneEditNotes);
            break;
        case SignalInfoIndexToggleFavorite: {
            RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
            signal->is_favorite = !signal->is_favorite;
            if(app->state->auto_save_enabled) {
                rfid_signal_save(app, signal);
            }
            // Refresh scene
            scene_manager_previous_scene(app->scene_manager);
            scene_manager_next_scene(app->scene_manager, RfidSceneSignalInfo);
            break;
        }
        case SignalInfoIndexDelete:
            if(rfid_signal_delete(app, app->state->selected_signal_index)) {
                notification_message(app->notifications, &sequence_success);
                scene_manager_previous_scene(app->scene_manager);
            }
            break;
    }
}

void rfid_scene_signal_info_on_enter(void* context) {
    RfidApp* app = context;
    
    if(app->state->selected_signal_index >= app->state->signal_count) {
        scene_manager_previous_scene(app->scene_manager);
        return;
    }
    
    RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
    
    Widget* widget = app->widget;
    widget_reset(widget);
    
    FuriString* info = furi_string_alloc();
    
    furi_string_printf(info, "Name: %s\n", signal->name);
    furi_string_cat_printf(info, "Protocol: %s\n", signal->protocol_name);
    furi_string_cat_printf(info, "UID: ");
    for(uint8_t i = 0; i < signal->uid_len; i++) {
        furi_string_cat_printf(info, "%02X", signal->uid[i]);
        if(i < signal->uid_len - 1) furi_string_cat_printf(info, ":");
    }
    furi_string_cat_printf(info, "\nFreq: %lu kHz\n", signal->frequency / 1000);
    furi_string_cat_printf(info, "Strength: %u\n", signal->signal_strength);
    furi_string_cat_printf(info, "Favorite: %s\n", signal->is_favorite ? "Yes" : "No");
    if(signal->notes[0] != '\0') {
        furi_string_cat_printf(info, "Notes: %s\n", signal->notes);
    }
    
    widget_add_text_scroll_element(widget, 0, 0, 128, 40, furi_string_get_cstr(info));
    furi_string_free(info);
    
    widget_add_button_element(widget, GuiButtonTypeLeft, "Back", NULL, NULL);
    widget_add_button_element(widget, GuiButtonTypeRight, "Menu", NULL, NULL);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewWidget);
}

bool rfid_scene_signal_info_on_event(void* context, SceneManagerEvent event) {
    RfidApp* app = context;
    bool consumed = false;
    
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            // Show options menu
            Submenu* submenu = app->submenu;
            submenu_reset(submenu);
            submenu_set_header(submenu, "Signal Options");
            
            submenu_add_item(submenu, "Transmit/Duplicate", SignalInfoIndexTransmit, 
                           rfid_signal_info_submenu_callback, app);
            submenu_add_item(submenu, "Export", SignalInfoIndexExport, 
                           rfid_signal_info_submenu_callback, app);
            submenu_add_item(submenu, "Edit Name", SignalInfoIndexEditName, 
                           rfid_signal_info_submenu_callback, app);
            submenu_add_item(submenu, "Edit Notes", SignalInfoIndexEditNotes, 
                           rfid_signal_info_submenu_callback, app);
            
            RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
            submenu_add_item(submenu, 
                           signal->is_favorite ? "Remove Favorite" : "Mark Favorite", 
                           SignalInfoIndexToggleFavorite, 
                           rfid_signal_info_submenu_callback, app);
            submenu_add_item(submenu, "Delete", SignalInfoIndexDelete, 
                           rfid_signal_info_submenu_callback, app);
            
            view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewSubmenu);
            consumed = true;
        }
    }
    
    return consumed;
}

void rfid_scene_signal_info_on_exit(void* context) {
    RfidApp* app = context;
    widget_reset(app->widget);
}
