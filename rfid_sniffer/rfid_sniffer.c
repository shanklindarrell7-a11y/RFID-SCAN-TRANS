#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include "signal_processor.h"

#define TAG "RFIDSniffer"
#define RFID_SNIFFER_VERSION "1.0.0"

typedef enum {
    RFIDSnifferViewSubmenu,
    RFIDSnifferViewCapture,
    RFIDSnifferViewStorage,
    RFIDSnifferViewDuplicate,
    RFIDSnifferViewSettings,
} RFIDSnifferView;

typedef enum {
    RFIDSnifferEventCapture,
    RFIDSnifferEventStorage,
    RFIDSnifferEventDuplicate,
    RFIDSnifferEventSettings,
    RFIDSnifferEventBack,
} RFIDSnifferEvent;

typedef enum {
    DuplicateModeClone,
    DuplicateModeEmulate,
    DuplicateModeReplay,
    DuplicateModeGenerate,
} DuplicateMode;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    NotificationApp* notifications;
    Storage* storage;
    SignalProcessor* signal_processor;
    bool is_capturing;
    DuplicateMode duplicate_mode;
    char file_path[256];
    FuriMutex* mutex;
} RFIDSnifferApp;

static void rfid_sniffer_submenu_callback(void* context, uint32_t index) {
    RFIDSnifferApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static bool rfid_sniffer_navigation_callback(void* context) {
    RFIDSnifferApp* app = context;
    return view_dispatcher_send_custom_event(app->view_dispatcher, RFIDSnifferEventBack);
}

static void rfid_sniffer_draw_callback(Canvas* canvas, void* context) {
    RFIDSnifferApp* app = context;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "RFID Sniffer");
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 24, "Version: " RFID_SNIFFER_VERSION);
    
    if(app->is_capturing) {
        canvas_draw_str(canvas, 2, 36, "Status: Capturing...");
        canvas_draw_str(canvas, 2, 48, "Press Back to stop");
    } else {
        canvas_draw_str(canvas, 2, 36, "Status: Ready");
        canvas_draw_str(canvas, 2, 48, "Select option from menu");
    }
    
    furi_mutex_release(app->mutex);
}

static bool rfid_sniffer_input_callback(InputEvent* event, void* context) {
    RFIDSnifferApp* app = context;
    bool consumed = false;
    
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        if(app->is_capturing) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            app->is_capturing = false;
            signal_processor_stop(app->signal_processor);
            furi_mutex_release(app->mutex);
            consumed = true;
        }
    }
    
    return consumed;
}

static void rfid_sniffer_start_capture(RFIDSnifferApp* app) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    
    if(!app->is_capturing) {
        FURI_LOG_I(TAG, "Starting RFID signal capture");
        app->is_capturing = true;
        
        if(signal_processor_start(app->signal_processor)) {
            notification_message(app->notifications, &sequence_blink_start_cyan);
            FURI_LOG_I(TAG, "Capture started successfully");
        } else {
            FURI_LOG_E(TAG, "Failed to start capture");
            app->is_capturing = false;
            notification_message(app->notifications, &sequence_error);
        }
    }
    
    furi_mutex_release(app->mutex);
}

static void rfid_sniffer_stop_capture(RFIDSnifferApp* app) {
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    
    if(app->is_capturing) {
        FURI_LOG_I(TAG, "Stopping RFID signal capture");
        signal_processor_stop(app->signal_processor);
        app->is_capturing = false;
        notification_message(app->notifications, &sequence_success);
    }
    
    furi_mutex_release(app->mutex);
}

static void rfid_sniffer_save_signal(RFIDSnifferApp* app) {
    FURI_LOG_I(TAG, "Saving captured signal");
    
    if(signal_processor_has_data(app->signal_processor)) {
        if(signal_processor_save(app->signal_processor, app->file_path)) {
            FURI_LOG_I(TAG, "Signal saved to: %s", app->file_path);
            notification_message(app->notifications, &sequence_success);
        } else {
            FURI_LOG_E(TAG, "Failed to save signal");
            notification_message(app->notifications, &sequence_error);
        }
    } else {
        FURI_LOG_W(TAG, "No signal data to save");
        notification_message(app->notifications, &sequence_error);
    }
}

static void rfid_sniffer_duplicate_signal(RFIDSnifferApp* app) {
    FURI_LOG_I(TAG, "Duplicating signal - Mode: %d", app->duplicate_mode);
    
    if(!signal_processor_has_data(app->signal_processor)) {
        FURI_LOG_W(TAG, "No signal data to duplicate");
        notification_message(app->notifications, &sequence_error);
        return;
    }
    
    bool result = false;
    switch(app->duplicate_mode) {
        case DuplicateModeClone:
            FURI_LOG_I(TAG, "Cloning signal...");
            result = signal_processor_clone(app->signal_processor);
            break;
        case DuplicateModeEmulate:
            FURI_LOG_I(TAG, "Emulating signal...");
            result = signal_processor_emulate(app->signal_processor);
            break;
        case DuplicateModeReplay:
            FURI_LOG_I(TAG, "Replaying signal...");
            result = signal_processor_replay(app->signal_processor);
            break;
        case DuplicateModeGenerate:
            FURI_LOG_I(TAG, "Generating signal...");
            result = signal_processor_generate(app->signal_processor);
            break;
    }
    
    if(result) {
        notification_message(app->notifications, &sequence_success);
    } else {
        notification_message(app->notifications, &sequence_error);
    }
}

static bool rfid_sniffer_custom_event_callback(void* context, uint32_t event) {
    RFIDSnifferApp* app = context;
    bool consumed = false;
    
    switch(event) {
        case RFIDSnifferEventCapture:
            FURI_LOG_D(TAG, "Event: Capture");
            rfid_sniffer_start_capture(app);
            consumed = true;
            break;
        case RFIDSnifferEventStorage:
            FURI_LOG_D(TAG, "Event: Storage");
            rfid_sniffer_save_signal(app);
            consumed = true;
            break;
        case RFIDSnifferEventDuplicate:
            FURI_LOG_D(TAG, "Event: Duplicate");
            rfid_sniffer_duplicate_signal(app);
            consumed = true;
            break;
        case RFIDSnifferEventSettings:
            FURI_LOG_D(TAG, "Event: Settings");
            consumed = true;
            break;
        case RFIDSnifferEventBack:
            rfid_sniffer_stop_capture(app);
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
            break;
    }
    
    return consumed;
}

static RFIDSnifferApp* rfid_sniffer_app_alloc(void) {
    RFIDSnifferApp* app = malloc(sizeof(RFIDSnifferApp));
    
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);
    
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, rfid_sniffer_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, rfid_sniffer_navigation_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Capture Signal", RFIDSnifferEventCapture, rfid_sniffer_submenu_callback, app);
    submenu_add_item(app->submenu, "Save to Storage", RFIDSnifferEventStorage, rfid_sniffer_submenu_callback, app);
    submenu_add_item(app->submenu, "Duplicate Signal", RFIDSnifferEventDuplicate, rfid_sniffer_submenu_callback, app);
    submenu_add_item(app->submenu, "Settings", RFIDSnifferEventSettings, rfid_sniffer_submenu_callback, app);
    view_dispatcher_add_view(app->view_dispatcher, RFIDSnifferViewSubmenu, submenu_get_view(app->submenu));
    
    app->signal_processor = signal_processor_alloc();
    app->is_capturing = false;
    app->duplicate_mode = DuplicateModeClone;
    snprintf(app->file_path, sizeof(app->file_path), "/ext/rfid/captured_signal.rfid");
    
    storage_common_mkdir(app->storage, "/ext/rfid");
    
    return app;
}

static void rfid_sniffer_app_free(RFIDSnifferApp* app) {
    furi_assert(app);
    
    if(app->is_capturing) {
        signal_processor_stop(app->signal_processor);
    }
    
    view_dispatcher_remove_view(app->view_dispatcher, RFIDSnifferViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    
    signal_processor_free(app->signal_processor);
    
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    furi_mutex_free(app->mutex);
    
    free(app);
}

int32_t rfid_sniffer_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I(TAG, "RFID Sniffer starting...");
    
    RFIDSnifferApp* app = rfid_sniffer_app_alloc();
    
    view_dispatcher_switch_to_view(app->view_dispatcher, RFIDSnifferViewSubmenu);
    view_dispatcher_run(app->view_dispatcher);
    
    rfid_sniffer_app_free(app);
    
    FURI_LOG_I(TAG, "RFID Sniffer stopped");
    
    return 0;
}
