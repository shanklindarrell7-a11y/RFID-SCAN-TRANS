#pragma once

#include <gui/scene_manager.h>

// Generate scene id and handlers
#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_enter,
void (*const rfid_scene_on_enter_handlers[])(void*) = {
#include "rfid_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_event,
bool (*const rfid_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
#include "rfid_scene_config.h"
};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, id) prefix##_scene_##name##_on_exit,
void (*const rfid_scene_on_exit_handlers[])(void*) = {
#include "rfid_scene_config.h"
};
#undef ADD_SCENE

// Scene manager configuration
const SceneManagerHandlers rfid_scene_handlers = {
    .on_enter_handlers = rfid_scene_on_enter_handlers,
    .on_event_handlers = rfid_scene_on_event_handlers,
    .on_exit_handlers = rfid_scene_on_exit_handlers,
    .scene_num = RfidSceneCount,
};

// Scene function declarations
void rfid_scene_start_on_enter(void* context);
bool rfid_scene_start_on_event(void* context, SceneManagerEvent event);
void rfid_scene_start_on_exit(void* context);

void rfid_scene_capture_on_enter(void* context);
bool rfid_scene_capture_on_event(void* context, SceneManagerEvent event);
void rfid_scene_capture_on_exit(void* context);

void rfid_scene_library_on_enter(void* context);
bool rfid_scene_library_on_event(void* context, SceneManagerEvent event);
void rfid_scene_library_on_exit(void* context);

void rfid_scene_signal_info_on_enter(void* context);
bool rfid_scene_signal_info_on_event(void* context, SceneManagerEvent event);
void rfid_scene_signal_info_on_exit(void* context);

void rfid_scene_export_on_enter(void* context);
bool rfid_scene_export_on_event(void* context, SceneManagerEvent event);
void rfid_scene_export_on_exit(void* context);

void rfid_scene_transmit_on_enter(void* context);
bool rfid_scene_transmit_on_event(void* context, SceneManagerEvent event);
void rfid_scene_transmit_on_exit(void* context);

void rfid_scene_settings_on_enter(void* context);
bool rfid_scene_settings_on_event(void* context, SceneManagerEvent event);
void rfid_scene_settings_on_exit(void* context);

void rfid_scene_edit_name_on_enter(void* context);
bool rfid_scene_edit_name_on_event(void* context, SceneManagerEvent event);
void rfid_scene_edit_name_on_exit(void* context);

void rfid_scene_edit_notes_on_enter(void* context);
bool rfid_scene_edit_notes_on_event(void* context, SceneManagerEvent event);
void rfid_scene_edit_notes_on_exit(void* context);
