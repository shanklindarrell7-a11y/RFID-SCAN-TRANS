#include "rfid_scan_trans.h"
#include "signal_processor.h"
#include "library.h"
#include "export.h"
#include <furi.h>
#include <gui/gui.h>
#include <storage/storage.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} RfidScanTransEvent;

static void rfid_scan_trans_draw_callback(Canvas* canvas, void* context) {
    RfidScanTransApp* app = context;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "RFID Scan Trans");
    
    canvas_set_font(canvas, FontSecondary);
    
    FuriString* str = furi_string_alloc();
    
    furi_string_printf(str, "Library: %zu/%zu signals", 
        library_get_count(&app->library), 
        MAX_LIBRARY_SIGNALS);
    canvas_draw_str(canvas, 2, 25, furi_string_get_cstr(str));
    
    furi_string_printf(str, "Buffer: %d samples", CAPTURE_BUFFER_SIZE);
    canvas_draw_str(canvas, 2, 35, furi_string_get_cstr(str));
    
    furi_string_printf(str, "Protocols: %zu", get_protocol_count());
    canvas_draw_str(canvas, 2, 45, furi_string_get_cstr(str));
    
    furi_string_printf(str, "Formats: %d", MAX_EXPORT_FORMATS);
    canvas_draw_str(canvas, 2, 55, furi_string_get_cstr(str));
    
    canvas_draw_str(canvas, 2, 63, "Press Back to exit");
    
    furi_string_free(str);
}

static void rfid_scan_trans_input_callback(InputEvent* input_event, void* context) {
    furi_assert(context);
    FuriMessageQueue* event_queue = context;
    
    RfidScanTransEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static RfidScanTransApp* rfid_scan_trans_app_alloc(void) {
    RfidScanTransApp* app = malloc(sizeof(RfidScanTransApp));
    
    // Initialize library
    if(!library_init(&app->library)) {
        FURI_LOG_E(TAG, "Failed to initialize library");
        free(app);
        return NULL;
    }
    
    // Open storage
    app->storage = furi_record_open(RECORD_STORAGE);
    
    // Load existing library
    library_load(&app->library, app->storage);
    
    return app;
}

static void rfid_scan_trans_app_free(RfidScanTransApp* app) {
    furi_assert(app);
    
    // Save library before exit
    library_save(&app->library, app->storage);
    
    // Free library
    library_free(&app->library);
    
    // Close storage
    furi_record_close(RECORD_STORAGE);
    
    free(app);
}

int32_t rfid_scan_trans_app(void* p) {
    UNUSED(p);
    
    FURI_LOG_I(TAG, "Starting RFID Scan Trans");
    
    RfidScanTransApp* app = rfid_scan_trans_app_alloc();
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate app");
        return -1;
    }
    
    // Create event queue
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(RfidScanTransEvent));
    
    // Set up view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, rfid_scan_trans_draw_callback, app);
    view_port_input_callback_set(view_port, rfid_scan_trans_input_callback, event_queue);
    
    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    RfidScanTransEvent event;
    bool running = true;
    
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort || event.input.type == InputTypeLong) {
                    if(event.input.key == InputKeyBack) {
                        running = false;
                    }
                }
            }
        }
        view_port_update(view_port);
    }
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(event_queue);
    
    rfid_scan_trans_app_free(app);
    
    FURI_LOG_I(TAG, "RFID Scan Trans stopped");
    
    return 0;
}
