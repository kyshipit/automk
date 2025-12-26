/**
 * @file watchdog.c
 * @brief AutoMLOS Kernel Watchdog Management Implementation
 */

#include "watchdog.h"
#include "../safety/system_errors.h"
#include "../libipc/ipc_topics.h"  // Add unified topic management
#include "../kernel/core/ipc.h"
#include "system_errors.h"
#include <string.h>
#include "../logging/log_client.h"

// Kernel watchdog context
static struct {
    bool initialized;
    uint32_t component_count;
    struct {
        kernel_watchdog_level_t level;
        uint32_t timeout_ms;
        char component_name[32];
        uint64_t last_pet_time;
        kernel_watchdog_state_t state;
    } components[8]; // Max 8 kernel components
} kernel_watchdog_ctx;


// Watchdog module ID
#define MODULE_ID_WATCHDOG 0x7000

/**
 * @brief Create success error code for watchdog module
 */
static system_error_t watchdog_error_success(void) {
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_NONE, MODULE_ID_WATCHDOG);
}

/**
 * @brief Create already initialized error for watchdog module
 */
static system_error_t watchdog_error_already_initialized(void) {
    return system_error_create(ERR_SYS_ALREADY_INIT, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_WARNING, MODULE_ID_WATCHDOG);
}

/**
 * @brief Create not initialized error for watchdog module
 */
static system_error_t watchdog_error_not_initialized(void) {
    return system_error_create(ERR_SYS_NOT_INIT, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_MEDIUM, MODULE_ID_WATCHDOG);
}

/**
 * @brief Create resource exhausted error for watchdog module
 */
static system_error_t watchdog_error_resource_exhausted(void) {
    return system_error_create(ERR_SYS_RESOURCE_UNAVAIL, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_MEDIUM, MODULE_ID_WATCHDOG);
}

/**
 * @brief Create invalid parameter error for watchdog module
 */
static system_error_t watchdog_error_invalid_parameter(void) {
    return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_MEDIUM, MODULE_ID_WATCHDOG);
}

system_error_t kernel_watchdog_init(void)
{
    if (kernel_watchdog_ctx.initialized) {
        return watchdog_error_already_initialized();
    }
    
    // Initialize context
    memset(&kernel_watchdog_ctx, 0, sizeof(kernel_watchdog_ctx));
    kernel_watchdog_ctx.initialized = true;
    
    // Register IPC topic using direct constant
    system_error_t ret = ipc_register_topic(IPC_TOPIC_SYSTEM_WATCHDOG_STATUS, 0);
    if (ret.error_code != ERR_BASE_OK) {
        kernel_watchdog_ctx.initialized = false;
        return ret;
    }
    
    return watchdog_error_success();
}

/**
 * @brief Watchdog logging function for component registration
 */
system_error_t watchdog_log_component_register(kernel_watchdog_level_t level, const char* name) {
    uint32_t name_hash = 0;
    for (const char* p = name; *p; p++) {
        name_hash = (name_hash << 5) - name_hash + *p;
    }
    system_error_t ret = log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_KERNEL, 
                               LOG_MSG_SERVICE_READY, level, name_hash);
    return ret;
}

/**
 * @brief Watchdog logging function for timeout events
 */
system_error_t watchdog_log_timeout_event(kernel_watchdog_level_t level) {
    system_error_t ret = log_client_send_entry(LOG_LEVEL_EMERGENCY, LOG_TAG_KERNEL, 
                               LOG_MSG_SERVICE_ERROR, level, 0);
    return ret;
}

system_error_t kernel_watchdog_register(
    kernel_watchdog_level_t level,
    uint32_t timeout_ms,
    const char* component_name)
{
    if (!kernel_watchdog_ctx.initialized) {
        return watchdog_error_not_initialized();
    }
    
    if (kernel_watchdog_ctx.component_count >= 8) {
        return watchdog_error_resource_exhausted();
    }
    
    if (timeout_ms < 10 || timeout_ms > 10000) {
        return watchdog_error_invalid_parameter();
    }
    
    // Add component to registry
    uint32_t index = kernel_watchdog_ctx.component_count++;
    kernel_watchdog_ctx.components[index].level = level;
    kernel_watchdog_ctx.components[index].timeout_ms = timeout_ms;
    kernel_watchdog_ctx.components[index].state = KERNEL_WATCHDOG_STATE_DISABLED;
    strncpy(kernel_watchdog_ctx.components[index].component_name, 
            component_name, 31);
    
    // Log component registration
    watchdog_log_component_register(level, component_name);
    
    return watchdog_error_success();
}

void kernel_watchdog_timeout_handler(kernel_watchdog_level_t level)
{
    // This function is called by system monitor service via IPC callback
    // when a kernel component watchdog times out
    
    for (uint32_t i = 0; i < kernel_watchdog_ctx.component_count; i++) {
        if (kernel_watchdog_ctx.components[i].level == level) {
            kernel_watchdog_ctx.components[i].state = KERNEL_WATCHDOG_STATE_TRIGGERED;
            
            // Log timeout event
            watchdog_log_timeout_event(level);
            
            // Handle timeout - this would trigger appropriate recovery actions
            // based on the component that timed out
            switch (level) {
                case KERNEL_WATCHDOG_LEVEL_SCHEDULER:
                    // Scheduler recovery logic
                    break;
                case KERNEL_WATCHDOG_LEVEL_MEMORY:
                    // Memory management recovery
                    break;
                case KERNEL_WATCHDOG_LEVEL_IPC:
                    // IPC subsystem recovery
                    break;
                case KERNEL_WATCHDOG_LEVEL_SYSCALL:
                    // System call recovery
                    break;
            }
            break;
        }
    }
}