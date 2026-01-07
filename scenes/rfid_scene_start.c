#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

enum SubmenuIndex {
    SubmenuIndexCapture,
    SubmenuIndexLibrary,
    SubmenuIndexSettings,
};

void rfid_submenu_callback(void* context, uint32_t index) {
    RfidApp* app = context;
    
    switch(index) {
        case SubmenuIndexCapture:
            scene_manager_next_scene(app->scene_manager, RfidSceneCapture);
            break;
        case SubmenuIndexLibrary:
            scene_manager_next_scene(app->scene_manager, RfidSceneLibrary);
            break;
        case SubmenuIndexSettings:
            scene_manager_next_scene(app->scene_manager, RfidSceneSettings);
            break;
    }
}

void rfid_scene_start_on_enter(void* context) {
    RfidApp* app = context;
    Submenu* submenu = app->submenu;
    
    submenu_reset(submenu);
    submenu_set_header(submenu, "RFID Scan & Trans");
    
    submenu_add_item(
        submenu,
        "Capture Signal",
        SubmenuIndexCapture,
        rfid_submenu_callback,
        app
    );
    
    submenu_add_item(
        submenu,
        "Signal Library",
        SubmenuIndexLibrary,
        rfid_submenu_callback,
        app
    );
    
    submenu_add_item(
        submenu,
        "Settings",
        SubmenuIndexSettings,
        rfid_submenu_callback,
        app
    );
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewSubmenu);
}

bool rfid_scene_start_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_start_on_exit(void* context) {
    RfidApp* app = context;
    submenu_reset(app->submenu);
}
