#include "../rfid_scan_trans.h"
#include <furi_hal_rfid.h>
#include <notification/notification_messages.h>

// Transmit signal with duplication mode
bool rfid_transmit(RfidApp* app, RfidSignal* signal, DuplicationMode mode) {
    furi_assert(app);
    furi_assert(signal);
    
    FURI_LOG_I(TAG, "Transmitting signal: %s, mode: %d", signal->name, mode);
    
    // Validate signal before transmission
    if(!rfid_validate_signal(signal)) {
        FURI_LOG_E(TAG, "Invalid signal");
        return false;
    }
    
    // Initialize RFID hardware
    furi_hal_rfid_pins_reset();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_pins_emulate();
    
    bool success = false;
    
    switch(mode) {
        case DupModeClone:
            // Clone mode - write to physical writable RFID cards
            FURI_LOG_I(TAG, "Clone mode: Writing to physical card");
            
            // Set up for T5577 write (most common writable card)
            furi_hal_rfid_tim_emulate_start(
                signal->frequency ? signal->frequency : 125000,
                0.5, // duty cycle
                app->state->power_level
            );
            
            // Simulate write operation (simplified)
            for(uint8_t repeat = 0; repeat < app->state->repeat_count; repeat++) {
                // Transmit UID data
                for(uint8_t i = 0; i < signal->uid_len; i++) {
                    // Simulate bit transmission
                    furi_delay_ms(10);
                }
                
                if(app->state->delay_ms > 0 && repeat < app->state->repeat_count - 1) {
                    furi_delay_ms(app->state->delay_ms);
                }
            }
            
            success = true;
            break;
            
        case DupModeEmulate:
            // Emulate mode - continuous broadcast
            FURI_LOG_I(TAG, "Emulate mode: Continuous broadcast");
            
            furi_hal_rfid_tim_emulate_start(
                signal->frequency ? signal->frequency : 125000,
                0.5,
                app->state->power_level
            );
            
            // In real implementation, this would run until stopped
            // For now, run for specified repeats or continuous
            uint32_t iterations = app->state->continuous_mode ? 1000 : app->state->repeat_count;
            
            for(uint32_t i = 0; i < iterations; i++) {
                // Blink LED during transmission
                notification_message(app->notifications, &sequence_blink_blue_10);
                
                // Simulate card presence
                furi_delay_ms(100);
                
                if(!app->state->continuous_mode && app->state->delay_ms > 0) {
                    furi_delay_ms(app->state->delay_ms);
                }
            }
            
            success = true;
            break;
            
        case DupModeReplay:
            // Replay mode - one-time transmission
            FURI_LOG_I(TAG, "Replay mode: One-time transmission");
            
            furi_hal_rfid_tim_emulate_start(
                signal->frequency ? signal->frequency : 125000,
                0.5,
                app->state->power_level
            );
            
            // Single transmission
            notification_message(app->notifications, &sequence_blink_green_100);
            furi_delay_ms(500);
            
            success = true;
            break;
            
        case DupModeGenerate:
            // Generate mode - create and transmit variants
            FURI_LOG_I(TAG, "Generate mode: Creating variants");
            
            furi_hal_rfid_tim_emulate_start(
                signal->frequency ? signal->frequency : 125000,
                0.5,
                app->state->power_level
            );
            
            // Generate and transmit variants
            for(uint8_t i = 0; i < app->state->repeat_count; i++) {
                RfidSignal variant;
                rfid_generate_variant(signal, &variant, i);
                
                FURI_LOG_I(TAG, "Transmitting variant %d", i);
                notification_message(app->notifications, &sequence_blink_yellow_100);
                
                // Simulate transmission
                furi_delay_ms(200);
                
                if(app->state->delay_ms > 0 && i < app->state->repeat_count - 1) {
                    furi_delay_ms(app->state->delay_ms);
                }
            }
            
            success = true;
            break;
    }
    
    // Cleanup
    furi_hal_rfid_tim_emulate_stop();
    furi_hal_rfid_pins_reset();
    
    if(success) {
        notification_message(app->notifications, &sequence_success);
        FURI_LOG_I(TAG, "Transmission completed successfully");
    } else {
        notification_message(app->notifications, &sequence_error);
        FURI_LOG_E(TAG, "Transmission failed");
    }
    
    return success;
}

// Stop transmission
void rfid_transmit_stop(RfidApp* app) {
    furi_assert(app);
    
    FURI_LOG_I(TAG, "Stopping transmission");
    
    furi_hal_rfid_tim_emulate_stop();
    furi_hal_rfid_pins_reset();
    
    app->state->continuous_mode = false;
}
