/**
 * @file logger_client.c
 * @brief Logger client implementation for autoMLOS
 * 
 * Implements client-side communication with the logger service
 * through IPC, maintaining proper module boundaries.
 */

#include "logger_client.h"
#include "../libsyscall/libsyscalls.h"
#include "../../services/logger/logger_service.h"
#include <string.h>

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
    // Parameter validation
    if (level >= LOGGER_SERVICE_LEVEL_COUNT) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
    }
    
    // Create service-compatible log entry
    logger_service_entry_t entry;
    uint64_t timestamp;
    system_error_t time_result = sys_time_get_us(&timestamp);
    if (time_result.error_code != ERR_BASE_OK) {
        return time_result;
    }
    
    entry.timestamp = (uint32_t)timestamp;
    entry.process_id = sys_getpid();
    entry.level = level;
    entry.tag_id = tag_id;
    entry.message_id = message_id;
    entry.data[0] = data1;
    entry.data[1] = data2;
    entry.checksum = 0; // Will be calculated
    
    // Calculate checksum
    const uint8_t* data = (const uint8_t*)&entry;
    uint32_t checksum = 0;
    for (size_t i = 0; i < (sizeof(logger_service_entry_t) - sizeof(entry.checksum)); i++) {
        checksum += data[i];
    }
    entry.checksum = (uint16_t)checksum;
    
    // Create service message
    logger_msg_entry_t msg;
    msg.type = LOGGER_MSG_LOG_ENTRY;
    msg.source_pid = sys_getpid();
    msg.entry = entry;
    
    // Send to logger service
    uint16_t logger_pid = LOGGER_SERVICE_PID; // Service discovery
    system_error_t send_result = sys_ipc_send(logger_pid, &msg, sizeof(msg));
    
    return send_result;
}

/**
 * @brief Get logger service status via IPC
 * 
 * @return system_error_t Service status
 */
system_error_t logger_client_get_status(void) {
    // Create status request
    logger_msg_status_t msg;
    msg.type = LOGGER_MSG_BUFFER_STATUS;
    msg.requester_pid = sys_getpid();
    
    // Send to logger service
    uint16_t logger_pid = LOGGER_SERVICE_PID;
    system_error_t send_result = sys_ipc_send(logger_pid, &msg, sizeof(msg));
    
    return send_result;
}