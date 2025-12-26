/**
 * @file hw_watchdog.h
 * @brief AutoMLOS Hardware Watchdog Driver
 * 
 * Hardware watchdog abstraction for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 * 
 * Features:
 * - Independent hardware watchdog
 * - Configurable timeout periods
 * - Safety state management
 * - Fault detection and recovery
 */

#ifndef BSP_HW_WATCHDOG_H
#define BSP_HW_WATCHDOG_H

#include "../../lib/safety/system_errors.h"
#include <stdint.h>
#include <stdbool.h>

// Watchdog configuration
#define HW_WATCHDOG_MIN_TIMEOUT_MS    10      // Minimum timeout (10ms)
#define HW_WATCHDOG_MAX_TIMEOUT_MS    10000   // Maximum timeout (10 seconds)
#define HW_WATCHDOG_DEFAULT_TIMEOUT_MS 1000   // Default timeout (1 second)

// Watchdog states
typedef enum {
    WATCHDOG_STATE_DISABLED = 0,       // Watchdog disabled
    WATCHDOG_STATE_ARMED,              // Watchdog armed
    WATCHDOG_STATE_TRIGGERED,          // Watchdog triggered
    WATCHDOG_STATE_FAULT               // Watchdog fault detected
} hw_watchdog_state_t;

// Watchdog callback function prototype
typedef void (*hw_watchdog_callback_t)(void* context);

// Hardware watchdog management functions
system_error_t hw_watchdog_init(uint32_t timeout_ms);
system_error_t hw_watchdog_deinit(void);
system_error_t hw_watchdog_pet(void);
system_error_t hw_watchdog_disable(void);

// State management functions
system_error_t hw_watchdog_get_state(hw_watchdog_state_t* state);
system_error_t hw_watchdog_get_time_remaining(uint32_t* time_remaining_ms);

// Callback functions
system_error_t hw_watchdog_set_timeout_callback(hw_watchdog_callback_t callback, void* context);

// Safety functions
system_error_t hw_watchdog_safety_self_test(void);
system_error_t hw_watchdog_fault_injection_test(void);

#endif // BSP_HW_WATCHDOG_H