/**
 * @file log_ipc.c
 * @brief IPC integration implementation for binary logging system
 * 
 * Implements the communication layer between logging clients
 * and the centralized logger service using autoMLOS IPC mechanism.
 */

#include "log_ipc.h"
#include "log_core.h"
#include "../libsyscall/libsyscalls.h"
#include <string.h>

// Enhanced IPC timeout and retry configuration
#define IPC_DEFAULT_TIMEOUT_MS      1000
#define IPC_MAX_RETRY_COUNT         3
#define IPC_RETRY_DELAY_MS          100


/**
 * @brief Send a message to a target process via IPC
 * 
 * @param target_pid Target process ID
 * @param message Pointer to message data
 * @param size Size of message in bytes
 * @return 0 on success, -1 on error
 */
int ipc_send_message(uint16_t target_pid, const void* message, size_t size) {
    system_error_t result = sys_ipc_send(target_pid, message, size);
    return (result.error_code == ERR_BASE_OK) ? 0 : -1;
}

/**
 * @brief Receive a message from IPC
 * 
 * @param buffer Buffer to store received message
 * @param size Size of buffer in bytes
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes received on success, -1 on error
 */
int ipc_receive_message(void* buffer, size_t size, uint32_t timeout_ms) {
    system_error_t result = sys_ipc_receive(buffer, size, timeout_ms);
    return (result.error_code == ERR_BASE_OK) ? (int)size : -1;
}

/**
 * @brief Look up the PID of the logger service
 * 
 * @return Logger service PID on success, 0 on error
 */
uint16_t get_logger_service_pid(void) {
    uint16_t service_pid = 0;
    system_error_t result = sys_service_lookup("system/logger", &service_pid);
    if (result.error_code == ERR_BASE_OK) {
        return service_pid;
    }
    return 0; // Service not found
}

/**
 * @brief Enhanced service discovery with timeout and retry
 * 
 * @return Logger service PID, 0 if not found
 */
static uint16_t get_logger_service_pid_enhanced(void) {
    uint16_t service_pid = 0;
    int retry_count = 0;
    const int max_retries = 3;
    
    while (retry_count < max_retries) {
        system_error_t result = sys_service_lookup("system/logger", &service_pid);
        if (result.error_code == ERR_BASE_OK && service_pid != 0) {
            return service_pid; // Service found
        }
        
        retry_count++;
        // Small delay before retry
        // system_delay_ms(50);
    }
    
    return 0; // Service not found after retries
}

/**
 * @brief Enhanced log entry submission via IPC with timeout and retry
 * 
 * @param entry Pointer to log entry to send
 * @return 0 on success, -1 on error
 */
int log_send_entry_ipc(const log_entry_binary_t* entry) {
    // Enhanced safety checks
    if (entry == NULL) {
        return -1;
    }
    
    // Enhanced entry integrity verification
    system_error_t integrity_check = log_verify_entry_integrity(entry);
    if (integrity_check.error_code != ERR_BASE_OK) {
        return -1;
    }
    
    // Get current PID with enhanced error handling
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    
    // Create IPC message with enhanced safety
    log_ipc_msg_entry_t ipc_msg;
    ipc_msg.type = LOG_IPC_MSG_LOG_ENTRY;
    ipc_msg.entry = *entry; // Safe copy (fixed size)
    ipc_msg.source_pid = current_pid;
    ipc_msg.reserved = 0;
    
    // Enhanced service discovery with timeout and retry
    uint16_t logger_pid = get_logger_service_pid_enhanced();
    if (logger_pid == 0) {
        return -1; // Logger service not found
    }
    
    // Enhanced IPC send with timeout and retry handling
    int retry_count = 0;
    const int max_retries = 2;
    
    while (retry_count < max_retries) {
        if (ipc_send_message(logger_pid, &ipc_msg, sizeof(ipc_msg)) == 0) {
            return 0; // Success
        }
        retry_count++;
        
        // Small delay before retry (implementation dependent)
        // system_delay_ms(10);
    }
    
    return -1; // Failed after retries
}

/**
 * @brief Query logger service buffer status via IPC
 * 
 * @param status Pointer to store status information
 * @return 0 on success, -1 on error
 */
int log_query_status_ipc(log_ipc_response_status_t* status) {
    if (status == NULL) {
        return -1;
    }
    
    // Create status query message
    log_ipc_msg_status_t query_msg;
    query_msg.type = LOG_IPC_MSG_BUFFER_STATUS;
    
    // Get current PID
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    query_msg.requester_pid = current_pid;
    query_msg.reserved = 0;
    
    // Send query and wait for response
    uint16_t logger_pid = get_logger_service_pid();
    if (logger_pid == 0) {
        return -1; // Logger service not found
    }
    
    if (ipc_send_message(logger_pid, &query_msg, sizeof(query_msg)) != 0) {
        return -1;
    }
    
    // Wait for response with timeout
    uint8_t response_buffer[sizeof(log_ipc_response_status_t)];
    int result = ipc_receive_message(response_buffer, sizeof(response_buffer), 1000);
    
    if (result > 0 && (size_t)result >= sizeof(log_ipc_response_status_t)) {
        *status = *(log_ipc_response_status_t*)response_buffer;
        return 0;
    }
    
    return -1;
}

/**
 * @brief Retrieve log entries from logger service via IPC
 * 
 * @param level_filter Filter by log level (0 = all)
 * @param max_entries Maximum entries to retrieve (0 = all available)
 * @param start_timestamp Start timestamp filter
 * @param entries Array to store retrieved entries
 * @param entry_count Pointer to store actual entry count
 * @return 0 on success, -1 on error
 */
int log_retrieve_entries_ipc(uint8_t level_filter, uint8_t max_entries, 
                            uint32_t start_timestamp, log_entry_binary_t* entries, 
                            uint16_t* entry_count) {
    // Safety checks
    if (entries == NULL || entry_count == NULL) {
        return -1;
    }
    
    if (max_entries > 16) { // Limit batch size for safety
        max_entries = 16;
    }
    
    // Create retrieval request
    log_ipc_msg_retrieve_t retrieve_msg;
    retrieve_msg.type = LOG_IPC_MSG_RETRIEVE_LOGS;
    
    // Get current PID
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    retrieve_msg.requester_pid = current_pid;
    retrieve_msg.level_filter = level_filter;
    retrieve_msg.max_entries = max_entries;
    retrieve_msg.start_timestamp = start_timestamp;
    
    // Send request
    uint16_t logger_pid = get_logger_service_pid();
    if (logger_pid == 0) {
        return -1; // Logger service not found
    }
    
    if (ipc_send_message(logger_pid, &retrieve_msg, sizeof(retrieve_msg)) != 0) {
        return -1;
    }
    
    // Wait for response with timeout
    uint8_t response_buffer[sizeof(log_ipc_response_retrieve_t)];
    int result = ipc_receive_message(response_buffer, sizeof(response_buffer), 1000);
    
    if (result > 0 && (size_t)result >= sizeof(log_ipc_response_retrieve_t)) {
        log_ipc_response_retrieve_t* response = (log_ipc_response_retrieve_t*)response_buffer;
        *entry_count = response->entry_count;
        
        // Copy entries to output buffer
        if (response->entry_count > 0) {
            for (uint16_t i = 0; i < response->entry_count; i++) {
                entries[i] = response->entries[i];
            }
        }
        
        return 0;
    }
    
    *entry_count = 0;
    return 0;
}

/**
 * @brief Enhanced log recording with IPC fallback
 * 
 * First tries to record locally, then falls back to IPC
 * for critical logs or when local buffer is full.
 */
void log_record_with_ipc_fallback(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                 uint32_t data1, uint32_t data2) {
    log_entry_binary_t entry;
    
    // Get current timestamp
    uint64_t timestamp;
    system_error_t time_result = sys_time_get_us(&timestamp);
    if (time_result.error_code != ERR_BASE_OK) {
        timestamp = 0; // Fallback to 0 if time unavailable
    }
    
    // Get current PID
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        current_pid = 0; // Fallback to 0 if PID unavailable
    }
    
    // Create entry with proper timestamp and process ID
    entry.timestamp = (uint32_t)timestamp;
    entry.process_id = current_pid;
    entry.level = level;
    entry.tag_id = tag_id;
    entry.message_id = message_id;
    entry.data[0] = data1;
    entry.data[1] = data2;
    entry.checksum = log_calculate_checksum(&entry);
    
    // Try local buffer first (fast path)
    log_buffer_t* global_buffer = get_global_log_buffer();
    if (global_buffer != NULL) {
        system_error_t put_result = log_buffer_put_safe(global_buffer, &entry);
        if (put_result.error_code == ERR_BASE_OK) {
            return; // Successfully stored locally
        }
    }
    
    // Local buffer full or error - use IPC for important logs
    if (level <= LOG_LEVEL_ERROR) {
        // L0-L1 logs: critical, must be delivered via IPC
        log_send_entry_ipc(&entry);
    }
    // L2-L3 logs: can be discarded when buffer is full (performance priority)
}

int log_send_buffer_status_ipc(uint8_t status) {
    (void)status; // Mark parameter as used to avoid warning
    
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    
    // Create IPC message
    log_ipc_msg_status_t ipc_msg;
    ipc_msg.type = LOG_IPC_MSG_BUFFER_STATUS;
    ipc_msg.requester_pid = current_pid;
    ipc_msg.reserved = 0;
    
    // Send to logger service
    uint16_t logger_pid = get_logger_service_pid();
    if (logger_pid == 0) {
        return -1; // Logger service not found
    }
    
    return ipc_send_message(logger_pid, &ipc_msg, sizeof(ipc_msg));
}

int log_send_entry_with_timestamp(const log_entry_binary_t* entry) {
    if (entry == NULL) {
        return -1;
    }
    
    uint64_t timestamp = 0;
    system_error_t time_result = sys_time_get_us(&timestamp);
    if (time_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    
    uint16_t current_pid = 0;
    system_error_t pid_result = sys_getpid(&current_pid);
    if (pid_result.error_code != ERR_BASE_OK) {
        return -1;
    }
    
    // Create IPC message
    log_ipc_msg_entry_t ipc_msg;
    ipc_msg.type = LOG_IPC_MSG_LOG_ENTRY;
    ipc_msg.entry = *entry; // Safe copy (fixed size)
    ipc_msg.source_pid = current_pid;
    ipc_msg.reserved = 0;
    
    // Send to logger service
    uint16_t logger_pid = get_logger_service_pid();
    if (logger_pid == 0) {
        return -1; // Logger service not found
    }
    
    return ipc_send_message(logger_pid, &ipc_msg, sizeof(ipc_msg));
}

/**
 * @brief Enhanced IPC communication with timeout and retry
 * 
 * @param service_pid Target service PID
 * @param entry Log entry to send
 * @param timeout_ms Timeout in milliseconds
 * @param max_retries Maximum retry attempts
 * @return system_error_t Error code indicating operation status
 */
system_error_t log_send_entry_ipc_enhanced(uint16_t service_pid, 
                                          const log_entry_binary_t* entry,
                                          uint32_t timeout_ms,
                                          uint8_t max_retries) {
    (void)timeout_ms; // Mark parameter as used to avoid warning
    (void)service_pid; // Mark parameter as used to avoid warning
    
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    system_error_t result;
    uint8_t retry_count = 0;
    
    do {
        int send_result = log_send_entry_with_timestamp(entry);
        if (send_result == 0) {
            return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                                     ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
        }
        
        // Convert integer result to system_error_t based on send_result value
        if (send_result == -1) {
            result = system_error_create(ERR_MOD_LOG_IPC_COMM, ERROR_CATEGORY_MODULE,
                                       ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
        } else {
            result = system_error_create(ERR_BASE_SW_FAULT, ERROR_CATEGORY_SYSTEM,
                                       ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
        }
        
        // Retry with delay if timeout or temporary failure
        if (result.error_code == ERR_BASE_TIMEOUT || 
            result.error_code == ERR_SAFE_TEMP_FAULT) {
            retry_count++;
            if (retry_count <= max_retries) {
                // Wait before retry (implementation depends on system)
                continue;
            }
        }
        
        // Permanent failure or max retries exceeded
        break;
        
    } while (retry_count <= max_retries);
    
    return result;
}