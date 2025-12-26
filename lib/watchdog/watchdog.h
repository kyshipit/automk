/**
 * @file watchdog.h
 * @brief AutoMLOS Kernel Watchdog Management Interface
 * 
 * Implements kernel-side watchdog management that communicates
 * with user-space system monitor service via IPC.
 */

#ifndef KERNEL_WATCHDOG_H
#define KERNEL_WATCHDOG_H

#include "../safety/system_errors.h"
#include "../logging/log_common.h"  // 添加日志支持

#include <stdint.h>
#include <stdbool.h>
#include "system_errors.h"

// Watchdog levels for kernel components
typedef enum {
    KERNEL_WATCHDOG_LEVEL_SCHEDULER = 0,   // Scheduler watchdog
    KERNEL_WATCHDOG_LEVEL_MEMORY,          // Memory management watchdog
    KERNEL_WATCHDOG_LEVEL_IPC,             // IPC subsystem watchdog
    KERNEL_WATCHDOG_LEVEL_SYSCALL          // System call watchdog
} kernel_watchdog_level_t;

// Watchdog states
typedef enum {
    KERNEL_WATCHDOG_STATE_DISABLED = 0,
    KERNEL_WATCHDOG_STATE_ARMED,
    KERNEL_WATCHDOG_STATE_TRIGGERED
} kernel_watchdog_state_t;

/**
 * @brief Initialize kernel watchdog management
 * @return system_error_t Error code
 * 
 * Establishes IPC connection with system monitor service
 */
system_error_t kernel_watchdog_init(void);

/**
 * @brief Register kernel component for watchdog monitoring
 * @param level Component level
 * @param timeout_ms Timeout in milliseconds
 * @param component_name Component identifier
 * @return system_error_t Error code
 */
system_error_t kernel_watchdog_register(
    kernel_watchdog_level_t level,
    uint32_t timeout_ms,
    const char* component_name);

/**
 * @brief Pet (feed) the watchdog for a kernel component
 * @param level Component level
 * @return system_error_t Error code
 * 
 * Sends IPC message to system monitor service
 */
system_error_t kernel_watchdog_pet(kernel_watchdog_level_t level);

/**
 * @brief Get kernel watchdog status
 * @param level Component level
 * @param state Output state
 * @param last_pet_time Output last pet time
 * @return system_error_t Error code
 */
system_error_t kernel_watchdog_get_status(
    kernel_watchdog_level_t level,
    kernel_watchdog_state_t* state,
    uint64_t* last_pet_time);

/**
 * @brief Handle watchdog timeout from system monitor
 * @param level Component level that timed out
 * 
 * Called via IPC callback when system monitor detects timeout
 */
void kernel_watchdog_timeout_handler(kernel_watchdog_level_t level);

// Watchdog logging functions
system_error_t watchdog_log_component_register(kernel_watchdog_level_t level, const char* name);
system_error_t watchdog_log_timeout_event(kernel_watchdog_level_t level);

#endif // KERNEL_WATCHDOG_H