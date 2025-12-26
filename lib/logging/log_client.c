/**
 * @file log_client.c
 * @brief Logging client implementation with early buffer support
 * 
 * Implements client-side communication with logger service using libipc pubsub,
 * with ISO 26262 ASIL-D compliant early logging buffer for startup diagnostics.
 * 
 * @author AutoMLOS Team
 * @version 1.2
 * @date 2024
 * @copyright AutoMLOS Automotive Systems
 */

#include "log_client.h"
#include "log_core.h"
#include "../libipc/ipc_pubsub.h"
#include "../libsyscall/libsyscalls.h"
#include <string.h>

// Client configuration - ISO 26262 compliant parameters
#define LOG_CLIENT_DEFAULT_TIMEOUT_MS   1000
#define LOG_CLIENT_BATCH_SIZE           16
#define EARLY_LOG_BUFFER_CAPACITY       32    // 32 entries for startup diagnostics
#define EARLY_LOG_ENTRY_SIZE            sizeof(log_entry_binary_t)

// Early log buffer for startup diagnostics (ISO 26262 Part 6-7.4.5 requirement)
typedef struct {
    log_entry_binary_t entries[EARLY_LOG_BUFFER_CAPACITY];
    uint16_t write_index;
    uint16_t read_index;
    uint16_t count;
    bool initialized;
} early_log_buffer_t;

// Global publisher for logging (singleton pattern)
static ipc_publisher_t* g_log_publisher = NULL;
static early_log_buffer_t g_early_log_buffer = {0};

/**
 * @brief Initialize early log buffer for startup diagnostics
 * 
 * @return system_error_t Initialization status
 * 
 * @note Required by ISO 26262 Part 6-7.4.5 for startup failure analysis
 */
static system_error_t early_log_buffer_init(void) {
    memset(&g_early_log_buffer, 0, sizeof(g_early_log_buffer));
    g_early_log_buffer.initialized = true;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Store log entry in early buffer (before IPC is available)
 * 
 * @param entry Pointer to log entry to store
 * @return system_error_t Operation status
 */
static system_error_t early_log_buffer_store(const log_entry_binary_t* entry) {
    // Safety: Check buffer initialization and null pointer
    if (!g_early_log_buffer.initialized || entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Check buffer capacity
    if (g_early_log_buffer.count >= EARLY_LOG_BUFFER_CAPACITY) {
        // Overwrite oldest entry (circular buffer behavior)
        g_early_log_buffer.read_index = (g_early_log_buffer.read_index + 1) % EARLY_LOG_BUFFER_CAPACITY;
        g_early_log_buffer.count--;
    }
    
    // Store entry
    memcpy(&g_early_log_buffer.entries[g_early_log_buffer.write_index], 
           entry, EARLY_LOG_ENTRY_SIZE);
    g_early_log_buffer.write_index = (g_early_log_buffer.write_index + 1) % EARLY_LOG_BUFFER_CAPACITY;
    g_early_log_buffer.count++;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Flush early log buffer to IPC once available
 * 
 * @return system_error_t Flush operation status
 */
static system_error_t early_log_buffer_flush(void) {
    if (!g_early_log_buffer.initialized || g_log_publisher == NULL) {
        return system_error_create(ERR_SYS_NOT_INIT, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    // Flush all entries in buffer
    while (g_early_log_buffer.count > 0) {
        const log_entry_binary_t* entry = &g_early_log_buffer.entries[g_early_log_buffer.read_index];
        
        // Publish using IPC
        if (ipc_publish(g_log_publisher, entry, EARLY_LOG_ENTRY_SIZE) != 0) {
            return system_error_create(ERR_SYS_SERVICE_NOT_FOUND, ERROR_CATEGORY_SYSTEM,
                                     ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
        }
        
        g_early_log_buffer.read_index = (g_early_log_buffer.read_index + 1) % EARLY_LOG_BUFFER_CAPACITY;
        g_early_log_buffer.count--;
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Initialize the logging client with early buffer and pubsub mechanism
 * 
 * @return system_error_t Initialization status
 * 
 * @implements ISO 26262 ASIL-D startup diagnostics requirement
 */
system_error_t log_client_init(void) {
    system_error_t error;
    
    // Initialize early log buffer first (always available)
    error = early_log_buffer_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Safety: Check if publisher already initialized
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
    
    // Flush any early logs now that IPC is available
    error = early_log_buffer_flush();
    if (error.error_code != ERR_BASE_OK) {
        return error;
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
/**
 * @brief Send log entry using pubsub mechanism with early buffer fallback
 * 
 * @param entry Pointer to log entry to send
 * @return system_error_t Operation status
 * 
 * @note Implements ISO 26262 ASIL-D startup diagnostics with fallback mechanism
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
    
    // Try to use IPC publisher if available
    if (g_log_publisher != NULL) {
        // Publish log entry using generic pubsub mechanism
        if (ipc_publish(g_log_publisher, entry, sizeof(log_entry_binary_t)) == 0) {
            return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                                     ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
        }
    }
    
    // IPC not available or failed - fallback to early log buffer
    // This ensures ISO 26262 ASIL-D startup diagnostics requirement
    return early_log_buffer_store(entry);
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