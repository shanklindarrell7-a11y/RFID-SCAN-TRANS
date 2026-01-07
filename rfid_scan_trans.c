#include "rfid_scan_trans.h"
#include "scenes/rfid_scene.h"
#include <furi_hal.h>

// Application allocation
RfidApp* rfid_app_alloc() {
    RfidApp* app = malloc(sizeof(RfidApp));
    
    // Allocate state
    app->state = malloc(sizeof(RfidAppState));
    memset(app->state, 0, sizeof(RfidAppState));
    app->state->power_level = 4; // Default medium power
    app->state->repeat_count = 1;
    app->state->delay_ms = 100;
    app->state->auto_save_enabled = true;
    
    // Open records
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    
    // Create view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Create scene manager
    app->scene_manager = scene_manager_alloc(&rfid_scene_handlers, app);
    
    // Create views
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RfidViewSubmenu, submenu_get_view(app->submenu));
    
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RfidViewTextInput, text_input_get_view(app->text_input));
    
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RfidViewPopup, popup_get_view(app->popup));
    
    app->variable_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RfidViewVariableItemList, variable_item_list_get_view(app->variable_item_list));
    
    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, RfidViewWidget, widget_get_view(app->widget));
    
    // Ensure storage directory exists
    storage_common_mkdir(app->storage, RFID_APP_FOLDER);
    
    // Load all saved signals
    rfid_signal_load_all(app);
    
    return app;
}

// Application cleanup
void rfid_app_free(RfidApp* app) {
    furi_assert(app);
    
    // Backup signals on exit if auto-save is enabled
    if(app->state->auto_save_enabled) {
        rfid_signal_backup_all(app);
    }
    
    // Remove views
    view_dispatcher_remove_view(app->view_dispatcher, RfidViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, RfidViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, RfidViewPopup);
    view_dispatcher_remove_view(app->view_dispatcher, RfidViewVariableItemList);
    view_dispatcher_remove_view(app->view_dispatcher, RfidViewWidget);
    
    // Free views
    submenu_free(app->submenu);
    text_input_free(app->text_input);
    popup_free(app->popup);
    variable_item_list_free(app->variable_item_list);
    widget_free(app->widget);
    
    // Free scene manager and view dispatcher
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);
    
    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_NOTIFICATION);
    
    // Free state and app
    free(app->state);
    free(app);
}

// Main entry point
int32_t rfid_scan_trans_app(void* p) {
    UNUSED(p);
    
    RfidApp* app = rfid_app_alloc();
    
    // Start with the main menu scene
    scene_manager_next_scene(app->scene_manager, RfidSceneStart);
    
    // Run view dispatcher
    view_dispatcher_run(app->view_dispatcher);
    
    // Cleanup
    rfid_app_free(app);
    
    return 0;
}
