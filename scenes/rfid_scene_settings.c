#include "../rfid_scan_trans.h"
#include "rfid_scene.h"

enum SettingsIndex {
    SettingsIndexPowerLevel,
    SettingsIndexRepeatCount,
    SettingsIndexDelay,
    SettingsIndexAutoSave,
};

static const char* power_level_names[] = {
    "0 (Lowest)",
    "1",
    "2",
    "3",
    "4 (Medium)",
    "5",
    "6",
    "7 (Highest)",
};

static void power_level_change(VariableItem* item) {
    RfidApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, power_level_names[index]);
    app->state->power_level = index;
}

static void repeat_count_change(VariableItem* item) {
    RfidApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    
    uint8_t values[] = {1, 2, 3, 5, 10, 20, 50};
    app->state->repeat_count = values[index];
    
    FuriString* text = furi_string_alloc();
    furi_string_printf(text, "%u", app->state->repeat_count);
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));
    furi_string_free(text);
}

static void delay_change(VariableItem* item) {
    RfidApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    
    uint16_t values[] = {0, 50, 100, 200, 500, 1000, 2000};
    app->state->delay_ms = values[index];
    
    FuriString* text = furi_string_alloc();
    if(app->state->delay_ms == 0) {
        furi_string_printf(text, "None");
    } else {
        furi_string_printf(text, "%u ms", app->state->delay_ms);
    }
    variable_item_set_current_value_text(item, furi_string_get_cstr(text));
    furi_string_free(text);
}

static void auto_save_change(VariableItem* item) {
    RfidApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->state->auto_save_enabled = index == 1;
    variable_item_set_current_value_text(item, app->state->auto_save_enabled ? "ON" : "OFF");
}

void rfid_scene_settings_on_enter(void* context) {
    RfidApp* app = context;
    VariableItemList* var_item_list = app->variable_item_list;
    VariableItem* item;
    
    variable_item_list_reset(var_item_list);
    variable_item_list_set_header(var_item_list, "Transmission Settings");
    
    // Power level
    item = variable_item_list_add(
        var_item_list,
        "Power Level",
        8,
        power_level_change,
        app
    );
    variable_item_set_current_value_index(item, app->state->power_level);
    variable_item_set_current_value_text(item, power_level_names[app->state->power_level]);
    
    // Repeat count
    item = variable_item_list_add(
        var_item_list,
        "Repeat Count",
        7,
        repeat_count_change,
        app
    );
    uint8_t repeat_values[] = {1, 2, 3, 5, 10, 20, 50};
    uint8_t repeat_index = 0;
    for(uint8_t i = 0; i < 7; i++) {
        if(repeat_values[i] == app->state->repeat_count) {
            repeat_index = i;
            break;
        }
    }
    variable_item_set_current_value_index(item, repeat_index);
    FuriString* repeat_text = furi_string_alloc();
    furi_string_printf(repeat_text, "%u", app->state->repeat_count);
    variable_item_set_current_value_text(item, furi_string_get_cstr(repeat_text));
    furi_string_free(repeat_text);
    
    // Delay
    item = variable_item_list_add(
        var_item_list,
        "Delay",
        7,
        delay_change,
        app
    );
    uint16_t delay_values[] = {0, 50, 100, 200, 500, 1000, 2000};
    uint8_t delay_index = 0;
    for(uint8_t i = 0; i < 7; i++) {
        if(delay_values[i] == app->state->delay_ms) {
            delay_index = i;
            break;
        }
    }
    variable_item_set_current_value_index(item, delay_index);
    FuriString* delay_text = furi_string_alloc();
    if(app->state->delay_ms == 0) {
        furi_string_printf(delay_text, "None");
    } else {
        furi_string_printf(delay_text, "%u ms", app->state->delay_ms);
    }
    variable_item_set_current_value_text(item, furi_string_get_cstr(delay_text));
    furi_string_free(delay_text);
    
    // Auto-save
    item = variable_item_list_add(
        var_item_list,
        "Auto-Save",
        2,
        auto_save_change,
        app
    );
    variable_item_set_current_value_index(item, app->state->auto_save_enabled ? 1 : 0);
    variable_item_set_current_value_text(item, app->state->auto_save_enabled ? "ON" : "OFF");
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidViewVariableItemList);
}

bool rfid_scene_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_scene_settings_on_exit(void* context) {
    RfidApp* app = context;
    variable_item_list_reset(app->variable_item_list);
}
