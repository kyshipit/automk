/**
 * @file hw_watchdog.c
 * @brief AutoMLOS Hardware Watchdog Driver Implementation
 * 
 * Hardware watchdog abstraction for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 */

#include "hw_watchdog.h"
#include "hw_timer.h"
#include <string.h>

// Platform-specific function declarations (move to top)
system_error_t hw_watchdog_platform_init(uint32_t timeout_ms);
system_error_t hw_watchdog_platform_deinit(void);
system_error_t hw_watchdog_platform_pet(void);

// Global watchdog state
static struct {
    bool initialized;
    uint32_t timeout_ms;
    hw_watchdog_state_t state;
    hw_watchdog_callback_t timeout_callback;
    void* timeout_callback_context;
    uint64_t last_pet_time;
} g_hw_watchdog_state = {0};

/**
 * @brief Initialize hardware watchdog
 * 
 * @param timeout_ms Timeout period in milliseconds
 * @return system_error_t Error code
 */
system_error_t hw_watchdog_init(uint32_t timeout_ms) {
    // Safety checks
    if (g_hw_watchdog_state.initialized) {
        return system_error_create(ERR_SYS_ALREADY_INIT, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_HW_WATCHDOG);
    }
    
    if (timeout_ms < HW_WATCHDOG_MIN_TIMEOUT_MS || 
        timeout_ms > HW_WATCHDOG_MAX_TIMEOUT_MS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_HW_WATCHDOG);
    }
    
    // Initialize watchdog state
    memset(&g_hw_watchdog_state, 0, sizeof(g_hw_watchdog_state));
    g_hw_watchdog_state.timeout_ms = timeout_ms;
    g_hw_watchdog_state.state = WATCHDOG_STATE_DISABLED;
    
    // Platform-specific watchdog initialization
    system_error_t error = hw_watchdog_platform_init(timeout_ms);
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Perform safety self-test
    error = hw_watchdog_safety_self_test();
    if (error.error_code != ERR_BASE_OK) {
        hw_watchdog_platform_deinit();
        return error;
    }
    
    g_hw_watchdog_state.initialized = true;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_WATCHDOG);
}

/**
 * @brief Pet the watchdog (reset timeout)
 * 
 * @return system_error_t Error code
 */
system_error_t hw_watchdog_pet(void) {
    if (!g_hw_watchdog_state.initialized) {
        return system_error_create(ERR_SYS_NOT_INITIALIZED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_HW_WATCHDOG);
    }
    
    if (g_hw_watchdog_state.state != WATCHDOG_STATE_ARMED) {
        return system_error_create(ERR_BASE_INVALID_SIZE, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_MEDIUM, 
                                 MODULE_ID_HW_WATCHDOG);
    }
    
    // Platform-specific pet operation
    system_error_t error = hw_watchdog_platform_pet();
    if (error.error_code == ERR_BASE_OK) {
        g_hw_watchdog_state.last_pet_time = hw_timer_get_counter(); // Use timer counter
    }
    
    return error;
}

/**
 * @brief Hardware watchdog timeout handler (called from platform code)
 */
void hw_watchdog_timeout_handler(void) {
    if (g_hw_watchdog_state.initialized) {
        g_hw_watchdog_state.state = WATCHDOG_STATE_TRIGGERED;
        
        // Call timeout callback if registered
        if (g_hw_watchdog_state.timeout_callback) {
            g_hw_watchdog_state.timeout_callback(g_hw_watchdog_state.timeout_callback_context);
        }
    }
}

// Platform-specific function stubs
system_error_t hw_watchdog_platform_init(uint32_t timeout_ms) {
    (void)timeout_ms;
    // Platform-specific implementation
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_WATCHDOG);
}

system_error_t hw_watchdog_platform_deinit(void) {
    // Platform-specific implementation
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_WATCHDOG);
}

system_error_t hw_watchdog_platform_pet(void) {
    // Platform-specific implementation
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_WATCHDOG);
}