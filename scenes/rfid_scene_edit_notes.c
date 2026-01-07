#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

void rfid_edit_notes_callback(void* context) {
    RfidApp* app = context;
    RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
    
    // Update signal notes
    strncpy(signal->notes, app->state->text_input_buffer, MAX_NOTES_SIZE - 1);
    signal->notes[MAX_NOTES_SIZE - 1] = '\0';
    
    // Save if auto-save is enabled
    if(app->state->auto_save_enabled) {
        rfid_signal_save(app, signal);
    }
    
    notification_message(app->notifications, &sequence_success);
    scene_manager_previous_scene(app->scene_manager);
}

void rfid_scene_edit_notes_on_enter(void* context) {
    RfidApp* app = context;
    RfidSignal* signal = &app->state->signals[app->state->selected_signal_index];
    
    // Copy current notes to buffer
    strncpy(app->state->text_input_buffer, signal->notes, MAX_NOTES_SIZE - 1);
    app->state->text_input_buffer[MAX_NOTES_SIZE - 1] = '\0';
    
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, "Enter Notes");
    text_input_set_result_callback(
        app->text_input,
        rfid_edit_notes_callback,
        app,
        app->state->text_input_buffer,
        MAX_NOTES_SIZE - 1,
        true
    );
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewTextInput);
}

bool rfid_scene_edit_notes_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_edit_notes_on_exit(void* context) {
    RfidApp* app = context;
    text_input_reset(app->text_input);
}
