/**
 * @file log_client.c
 * @brief Logging client implementation using libipc pubsub
 * 
 * Implements client-side communication with logger service using libipc pubsub,
 * eliminating custom IPC implementation and using generic IPC functionality.
 */

#include "log_client.h"
#include "log_core.h"
#include "../libipc/ipc_pubsub.h"
#include "../libsyscall/libsyscalls.h"
#include <string.h>

// Client configuration
#define LOG_CLIENT_DEFAULT_TIMEOUT_MS   1000
#define LOG_CLIENT_BATCH_SIZE           16     // Batch size for efficient publishing

// Global publisher for logging (singleton pattern)
static ipc_publisher_t* g_log_publisher = NULL;

/**
 * @brief Initialize the logging client with pubsub mechanism
 * 
 * @return system_error_t Initialization status
 */
system_error_t log_client_init(void) {
    // Safety: Check if already initialized
    if (g_log_publisher != NULL) {
        return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
    }
    
    // Create publisher for logging topic
    g_log_publisher = ipc_create_publisher(IPC_TOPIC_SYSTEM_LOGGING_INFO);
    if (g_log_publisher == NULL) {
        return system_error_create(ERR_SYS_SERVICE_NOT_FOUND, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Send log entry using pubsub mechanism
 * 
 * @param entry Pointer to log entry to send
 * @return system_error_t Operation status
 */
static system_error_t send_log_entry_pubsub(const log_entry_binary_t* entry) {
    // Safety check: null pointer validation
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Verify entry integrity
    system_error_t integrity_check = log_verify_entry_integrity(entry);
    if (integrity_check.error_code != ERR_BASE_OK) {
        return integrity_check;
    }
    
    // Safety: Check publisher initialization
    if (g_log_publisher == NULL) {
        return system_error_create(ERR_SYS_NOT_INIT, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    // Publish log entry using generic pubsub mechanism
    if (ipc_publish(g_log_publisher, entry, sizeof(log_entry_binary_t)) != 0) {
        return system_error_create(ERR_SYS_SERVICE_NOT_FOUND, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Send log entry through client interface
 * 
 * @param level Log level
 * @param tag_id Tag identifier
 * @param message_id Message identifier
 * @param data1 First data field
 * @param data2 Second data field
 * @return system_error_t Operation status
 */
system_error_t log_client_send_entry(uint8_t level, uint8_t tag_id, uint16_t message_id,
                                   uint32_t data1, uint32_t data2) {
    // Create log entry
    log_entry_binary_t entry;
    system_error_t create_result = log_create_entry(&entry, level, tag_id, message_id,
                                                  data1, data2);
    if (create_result.error_code != ERR_BASE_OK) {
        return create_result;
    }
    
    // Send using pubsub mechanism
    return send_log_entry_pubsub(&entry);
}

/**
 * @brief Send log entry with timeout support
 * 
 * @param level Log level
 * @param tag_id Tag identifier
 * @param message_id Message identifier
 * @param data1 First data field
 * @param data2 Second data field
 * @param timeout_ms Timeout in milliseconds
 * @return system_error_t Operation status
 */
system_error_t log_client_send_entry_timeout(uint8_t level, uint8_t tag_id, uint16_t message_id,
                                           uint32_t data1, uint32_t data2, uint32_t timeout_ms) {
    // For pubsub mechanism, timeout is handled at the IPC level
    // We can implement retry logic if needed
    (void)timeout_ms; // Mark parameter as used to avoid warning
    return log_client_send_entry(level, tag_id, message_id, data1, data2);
}

/**
 * @brief Get logger service status
 * 
 * @return system_error_t Status query result
 */
system_error_t log_client_get_status(void) {
    // TODO: Implement status query using pubsub or separate topic
    // This could use a separate status topic for query-response pattern
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Get logger service status with timeout
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return system_error_t Status query result
 */
system_error_t log_client_get_status_timeout(uint32_t timeout_ms) {
    // TODO: Implement status query with timeout
    (void)timeout_ms; // Mark parameter as used to avoid warning
    return log_client_get_status();
}

/**
 * @brief Cleanup logging client resources
 */
void log_client_cleanup(void) {
    if (g_log_publisher != NULL) {
        ipc_destroy_publisher(g_log_publisher);
        g_log_publisher = NULL;
    }
}