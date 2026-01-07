#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

enum ExportSubmenuIndex {
    ExportIndexFlipper,
    ExportIndexCSV,
    ExportIndexWiegand,
    ExportIndexProxmark3,
    ExportIndexJSON,
};

void rfid_export_callback(void* context, uint32_t index) {
    RfidApp* app = context;
    RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
    
    FuriString* path = furi_string_alloc();
    bool success = false;
    
    switch(index) {
        case ExportIndexFlipper:
            furi_string_printf(path, "%s/%s.sub", RFID_APP_FOLDER, signal->name);
            success = rfid_export_flipper(app, signal, furi_string_get_cstr(path));
            break;
            
        case ExportIndexCSV:
            furi_string_printf(path, "%s/export.csv", RFID_APP_FOLDER);
            success = rfid_export_csv(app, furi_string_get_cstr(path));
            break;
            
        case ExportIndexWiegand:
            furi_string_printf(path, "%s/%s.wiegand", RFID_APP_FOLDER, signal->name);
            success = rfid_export_wiegand(app, signal, furi_string_get_cstr(path));
            break;
            
        case ExportIndexProxmark3:
            furi_string_printf(path, "%s/%s.pm3", RFID_APP_FOLDER, signal->name);
            success = rfid_export_proxmark3(app, signal, furi_string_get_cstr(path));
            break;
            
        case ExportIndexJSON:
            furi_string_printf(path, "%s/%s.json", RFID_APP_FOLDER, signal->name);
            success = rfid_export_json(app, signal, furi_string_get_cstr(path));
            break;
    }
    
    furi_string_free(path);
    
    if(success) {
        notification_message(app->notifications, &sequence_success);
    } else {
        notification_message(app->notifications, &sequence_error);
    }
    
    scene_manager_previous_scene(app->scene_manager);
}

void rfid_scene_export_on_enter(void* context) {
    RfidApp* app = context;
    Submenu* submenu = app->submenu;
    
    submenu_reset(submenu);
    submenu_set_header(submenu, "Export Format");
    
    submenu_add_item(submenu, "Flipper (.sub)", ExportIndexFlipper, rfid_export_callback, app);
    submenu_add_item(submenu, "CSV Database", ExportIndexCSV, rfid_export_callback, app);
    submenu_add_item(submenu, "Wiegand Format", ExportIndexWiegand, rfid_export_callback, app);
    submenu_add_item(submenu, "Proxmark3 Script", ExportIndexProxmark3, rfid_export_callback, app);
    submenu_add_item(submenu, "JSON Metadata", ExportIndexJSON, rfid_export_callback, app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewSubmenu);
}

bool rfid_scene_export_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_export_on_exit(void* context) {
    RfidApp* app = context;
    submenu_reset(app->submenu);
}
