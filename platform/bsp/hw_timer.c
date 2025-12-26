/**
 * @file hw_timer.c
 * @brief AutoMLOS Hardware Timer Driver Implementation
 * 
 * Hardware timer abstraction for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 */

#include "hw_timer.h"
#include <string.h>

// Platform-specific function prototypes
static system_error_t hw_timer_platform_init(uint32_t frequency_hz, hw_timer_mode_t mode);
static system_error_t hw_timer_platform_deinit(void);
static uint64_t hw_timer_platform_get_counter(void);

// Global timer state
static struct {
    bool initialized;
    uint32_t frequency_hz;
    hw_timer_mode_t mode;
    hw_timer_callback_t callback;
    void* callback_context;
    uint64_t base_counter;
} g_hw_timer_state = {0};

/**
 * @brief Initialize hardware timer
 * 
 * @param frequency_hz Timer frequency in Hz
 * @param mode Timer operating mode
 * @return system_error_t Error code
 */
system_error_t hw_timer_init(uint32_t frequency_hz, hw_timer_mode_t mode) {
    // Safety checks
    if (g_hw_timer_state.initialized) {
        return system_error_create(ERR_SYS_ALREADY_INIT, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_MEDIUM, 
                                 MODULE_ID_HW_TIMER);
    }
    
    if (frequency_hz < HW_TIMER_MIN_FREQUENCY_HZ || 
        frequency_hz > HW_TIMER_MAX_FREQUENCY_HZ) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_HW_TIMER);
    }
    
    // Initialize timer state
    memset(&g_hw_timer_state, 0, sizeof(g_hw_timer_state));
    g_hw_timer_state.frequency_hz = frequency_hz;
    g_hw_timer_state.mode = mode;
    
    // Platform-specific timer initialization
    system_error_t error = hw_timer_platform_init(frequency_hz, mode);
    if (error.error_code != ERR_SYS_OK) {
        return error;
    }
    
    // Perform safety self-test
    error = hw_timer_safety_self_test();
    if (error.error_code != ERR_SYS_OK) {
        hw_timer_platform_deinit();
        return error;
    }
    
    g_hw_timer_state.initialized = true;
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_TIMER);
}

/**
 * @brief Get current timer counter value
 * 
 * @return uint64_t Timer counter value
 */
uint64_t hw_timer_get_counter(void) {
    if (!g_hw_timer_state.initialized) {
        return 0;
    }
    
    return hw_timer_platform_get_counter() + g_hw_timer_state.base_counter;
}

/**
 * @brief Get timer frequency
 * 
 * @return uint64_t Timer frequency in Hz
 */
uint64_t hw_timer_get_frequency(void) {
    return g_hw_timer_state.initialized ? g_hw_timer_state.frequency_hz : 0;
}

/**
 * @brief Set timer interrupt callback
 * 
 * @param callback Callback function
 * @param context Callback context
 * @return system_error_t Error code
 */
system_error_t hw_timer_set_interrupt_callback(hw_timer_callback_t callback, void* context) {
    if (!g_hw_timer_state.initialized) {
        return system_error_create(ERR_SYS_NOT_INIT, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_HW_TIMER);
    }
    
    g_hw_timer_state.callback = callback;
    g_hw_timer_state.callback_context = context;
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_TIMER);
}

/**
 * @brief Hardware timer interrupt handler (called from platform code)
 */
void hw_timer_interrupt_handler(void) {
    if (g_hw_timer_state.initialized && g_hw_timer_state.callback) {
        g_hw_timer_state.callback(g_hw_timer_state.callback_context);
    }
    
    // Update base counter for free-running mode
    if (g_hw_timer_state.mode == TIMER_MODE_FREE_RUNNING) {
        g_hw_timer_state.base_counter += (1ULL << 32); // Assuming 32-bit timer
    }
}

// Platform-specific function stubs (to be implemented per platform)
static system_error_t hw_timer_platform_init(uint32_t frequency_hz, hw_timer_mode_t mode) {
    // Platform-specific implementation
    // This would configure the actual hardware timer
    (void)frequency_hz;  // Mark parameter as used
    (void)mode;          // Mark parameter as used
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_TIMER);
}

static system_error_t hw_timer_platform_deinit(void) {
    // Platform-specific implementation
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_HW_TIMER);
}

static uint64_t hw_timer_platform_get_counter(void) {
    // Platform-specific implementation
    return 0;
}