/**
 * @file logger_client.c
 * @brief Logger client implementation for autoMLOS
 * 
 * Implements client-side communication with the logger service
 * through IPC, maintaining proper module boundaries.
 */

#include "logger_client.h"
#include "../libsyscall/libsyscalls.h"
#include "log_common.h"  // Use common logging definitions instead of service header
#include "log_ipc.h"     // Use IPC module for communication
#include <string.h>

// Remove hardcoded service PID and use dynamic discovery
#define LOGGER_SERVICE_NAME "system/logger"

/**
 * @brief Initialize logger client
 * 
 * @return system_error_t Initialization status
 */
system_error_t logger_client_init(void) {
    // Client initialization is minimal - service discovery happens at runtime
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Send log entry to logger service via IPC
 * 
 * @param level Log level
 * @param tag_id Tag ID
 * @param message_id Message ID
 * @param data1 First data payload
 * @param data2 Second data payload
 * @return system_error_t Operation status
 */
system_error_t logger_client_send_entry(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                       uint32_t data1, uint32_t data2) {
    // Parameter validation using common definitions
    if (!LOG_VERIFY_LEVEL(level)) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
    }
    
    // Use common log entry structure instead of service-specific one
    log_entry_binary_t entry;
    uint64_t timestamp;
    system_error_t time_result = sys_time_get_us(&timestamp);
    if (time_result.error_code != ERR_BASE_OK) {
        return time_result;
    }
    
    // Get current PID - CORRECTED: sys_getpid requires pointer parameter
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return pid_result;
    }
    
    // Create log entry using common structure
    entry.timestamp = (uint32_t)timestamp;
    entry.process_id = current_pid;  // Use correct PID value
    entry.level = level;
    entry.tag_id = tag_id;
    entry.message_id = message_id;
    entry.data[0] = data1;
    entry.data[1] = data2;
    entry.checksum = 0; // Will be calculated
    
    // Calculate checksum using common function instead of manual calculation
    entry.checksum = log_calculate_checksum(&entry);
    
    // Use IPC module for communication instead of direct service communication
    int result = log_send_entry_ipc(&entry);
    if (result != 0) {
        return system_error_create(ERR_SYS_IPC_FAILED, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Get logger service status via IPC
 * 
 * @return system_error_t Service status
 */
system_error_t logger_client_get_status(void) {
    // Use IPC module for status query instead of direct service communication
    log_ipc_response_status_t status;
    int result = log_query_status_ipc(&status);
    
    if (result != 0) {
        return system_error_create(ERR_SYS_IPC_FAILED, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}