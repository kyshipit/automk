/**
 * @file logger_service.c
 * @brief Logger service implementation for autoMLOS
 * 
 * Implements a centralized logging service that collects, stores,
 * and manages log entries from all system components.
 * 
 * Key design principles:
 * - Decoupled from logging library internals
 * - Service-specific buffer management
 * - Unified error handling
 * - Deterministic performance
 */

#include "logger_service.h"
#include "../../lib/libsyscall/libsyscalls.h"
#include "../../lib/logging/log_common.h"  // Add missing header for log_entry_binary_t
#include "../../lib/logging/log_core.h"    // Add missing header for log_verify_entry_integrity_safe
#include <string.h>

// Service-specific buffer structure (decoupled from library)
typedef struct {
    logger_service_entry_t entries[LOGGER_SERVICE_BUFFER_CAPACITY];
    uint16_t write_index;
    uint16_t read_index;
    uint16_t entry_count;
    uint16_t buffer_size;
} logger_service_buffer_t;

// Internal buffer for different log levels
static logger_service_buffer_t g_level_buffers[LOGGER_SERVICE_LEVEL_COUNT];

// Storage management structure
static struct {
    uint32_t storage_base_address;   // Base address of log storage
    uint32_t storage_capacity;       // Total storage capacity in bytes
    uint32_t current_write_offset;   // Current write position in storage
    uint64_t last_flush_time;        // Timestamp of last storage flush
    uint8_t storage_available;       // Flag indicating storage availability
    uint8_t service_running;         // Flag indicating service status
} g_logger_service;

// Global variable for storing the last status (internal use only)
static logger_status_response_t g_last_status = {0};

/**
 * @brief Calculate checksum for service log entry
 * 
 * @param entry Pointer to log entry
 * @return 16-bit checksum value
 */
static uint16_t logger_service_calculate_checksum(const logger_service_entry_t* entry) {
    // Safety check: null pointer validation
    if (entry == NULL) {
        return 0xFFFF;
    }
    
    const uint8_t* data = (const uint8_t*)entry;
    uint32_t checksum = 0;
    
    // Calculate checksum for all fields except checksum field itself
    for (size_t i = 0; i < (sizeof(logger_service_entry_t) - sizeof(entry->checksum)); i++) {
        checksum += data[i];
    }
    
    return (uint16_t)checksum;
}

/**
 * @brief Verify service log entry integrity
 * 
 * @param entry Pointer to log entry to verify
 * @return 1 if entry is valid, 0 otherwise
 */
static int logger_service_verify_entry_integrity(const logger_service_entry_t* entry) {
    // Safety check: null pointer validation
    if (entry == NULL) {
        return 0;
    }
    
    // Verify checksum
    uint16_t calculated_checksum = logger_service_calculate_checksum(entry);
    if (entry->checksum != calculated_checksum) {
        return 0;
    }
    
    // Verify field ranges (service-specific validation)
    if (entry->level >= LOGGER_SERVICE_LEVEL_COUNT) {
        return 0;
    }
    
    // Additional service-specific validation can be added here
    
    return 1;
}

/**
 * @brief Initialize service buffer
 * 
 * @param buffer Pointer to service buffer
 */
static void logger_service_buffer_init(logger_service_buffer_t* buffer) {
    // Safety check: null pointer validation
    if (buffer == NULL) {
        return;
    }
    
    memset(buffer, 0, sizeof(logger_service_buffer_t));
    buffer->buffer_size = LOGGER_SERVICE_BUFFER_CAPACITY;
}

/**
 * @brief Add entry to service buffer
 * 
 * @param buffer Pointer to service buffer
 * @param entry Pointer to log entry
 * @return 0 on success, -1 on error
 */
static int logger_service_buffer_put(logger_service_buffer_t* buffer, 
                                   const logger_service_entry_t* entry) {
    // Safety checks: null pointer validation
    if (buffer == NULL || entry == NULL) {
        return -1;
    }
    
    // Verify entry integrity before adding to buffer
    if (!logger_service_verify_entry_integrity(entry)) {
        return -1;
    }
    
    // Buffer full check
    if (buffer->entry_count >= buffer->buffer_size) {
        return -1; // Service-specific full handling
    }
    
    // Copy entry to buffer
    memcpy(&buffer->entries[buffer->write_index], entry, sizeof(logger_service_entry_t));
    
    // Update indices with boundary protection
    buffer->write_index = (buffer->write_index + 1) % buffer->buffer_size;
    buffer->entry_count++;
    
    return 0;
}

/**
 * @brief Initialize the logger service with unified error handling
 * 
 * @return system_error_t Initialization status
 */
system_error_t logger_service_init(void) {
    // Initialize all level buffers
    for (uint8_t i = 0; i < LOGGER_SERVICE_LEVEL_COUNT; i++) {
        logger_service_buffer_init(&g_level_buffers[i]);
    }
    
    // Initialize storage management
    memset(&g_logger_service, 0, sizeof(g_logger_service));
    
    // Register the logger service with the kernel
    system_error_t result = sys_ipc_register_topic(LOGGER_SERVICE_NAME, 0);
    if (result.error_code != ERR_BASE_OK) {
        return system_error_create(ERR_SYS_SERVICE_REG_FAILED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_CRITICAL, 
                                 MODULE_ID_LOGGING);
    }
    
    // Set service running flag
    g_logger_service.service_running = 1;
    
    // Get initial timestamp
    uint64_t timestamp;
    result = sys_time_get_us(&timestamp);
    if (result.error_code == ERR_BASE_OK) {
        g_logger_service.last_flush_time = timestamp;
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Process a log entry service message with unified error handling
 * 
 * @param msg Pointer to service message
 * @param msg_size Size of service message
 * @return system_error_t Error status
 */
__attribute__((unused)) static system_error_t process_log_entry_message(const logger_msg_entry_t* msg, size_t msg_size) {
    // Safety check: message size validation
    if (msg_size != sizeof(logger_msg_entry_t)) {
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety check: null pointer validation
    if (msg == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Validate message type
    if (msg->type != LOGGER_MSG_LOG_ENTRY) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Validate source process ID
    if (msg->source_pid == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Store entry in appropriate level buffer using service function
    uint8_t level = msg->entry.level;
    if (level < LOGGER_SERVICE_LEVEL_COUNT) {
        logger_service_buffer_put(&g_level_buffers[level], &msg->entry);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Process buffer status service message with unified error handling
 * 
 * @param msg Pointer to service message
 * @param msg_size Size of service message
 * @return system_error_t Error status
 */
__attribute__((unused)) static system_error_t process_buffer_status_message(const logger_msg_status_t* msg, size_t msg_size) {
    // Safety check: message size validation
    if (msg_size != sizeof(logger_msg_status_t)) {
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety check: null pointer validation
    if (msg == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Validate message type
    if (msg->type != LOGGER_MSG_BUFFER_STATUS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Validate requester PID
    if (msg->requester_pid == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Prepare response
    logger_status_response_t response = {0};
    
    // Calculate total entry count across all level buffers
    uint16_t total_entries = 0;
    for (uint8_t i = 0; i < LOGGER_SERVICE_LEVEL_COUNT; i++) {
        total_entries += g_level_buffers[i].entry_count;
    }
    
    response.entry_count = total_entries;
    response.buffer_capacity = LOGGER_SERVICE_BUFFER_CAPACITY * LOGGER_SERVICE_LEVEL_COUNT;
    response.dropped_count = 0; // Track dropped entries in production
    
    // Set storage usage if storage is available
    if (g_logger_service.storage_available) {
        response.storage_usage = g_logger_service.current_write_offset;
    } else {
        response.storage_usage = 0;
    }
    
    // Send response back to requester
    system_error_t send_result = sys_ipc_send(msg->requester_pid, &response, sizeof(response));
    if (send_result.error_code != ERR_BASE_OK) {
        return send_result;
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Process a log entry received via pubsub mechanism
 * 
 * @param entry Pointer to log entry
 * @param entry_size Size of log entry
 * @return system_error_t Error status
 */
static system_error_t process_log_entry_pubsub(const log_entry_binary_t* entry, size_t entry_size) {
    // Safety check: message size validation
    if (entry_size != sizeof(log_entry_binary_t)) {
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety check: null pointer validation
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Verify log entry integrity using library function
    system_error_t integrity_result = log_verify_entry_integrity_safe(entry);
    if (integrity_result.error_code != ERR_BASE_OK) {
        return integrity_result;
    }
    
    // Convert to service-specific format for storage
    logger_service_entry_t service_entry;
    service_entry.timestamp = entry->timestamp;
    service_entry.process_id = entry->process_id;  // Use process_id from binary entry
    service_entry.level = entry->level;
    service_entry.tag_id = entry->tag_id;
    service_entry.message_id = entry->message_id;
    service_entry.data[0] = entry->data[0];  // Use data[0] from binary entry
    service_entry.data[1] = entry->data[1];  // Use data[1] from binary entry
    service_entry.checksum = logger_service_calculate_checksum(&service_entry);
    
    // Store entry in appropriate level buffer using service function
    uint8_t level = service_entry.level;
    if (level < LOGGER_SERVICE_LEVEL_COUNT) {
        logger_service_buffer_put(&g_level_buffers[level], &service_entry);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Process incoming pubsub message with unified error handling
 * 
 * @param message Pointer to message data
 * @param size Message size
 * @return system_error_t Error status
 */
system_error_t logger_service_process_message(const void* message, size_t size) {
    // Safety checks
    if (message == NULL || size < sizeof(log_entry_binary_t)) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // For pubsub mechanism, we expect log_entry_binary_t format directly
    return process_log_entry_pubsub((const log_entry_binary_t*)message, size);
}

/**
 * @brief Get logger service status with unified error handling
 * 
 * @return system_error_t Service status information
 */
system_error_t logger_service_get_status(void) {
    // Safety check: verify service is running
    if (!g_logger_service.service_running) {
        return system_error_create(ERR_SYS_NOT_INIT, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
    }
    
    // Calculate total buffer usage across all levels
    uint16_t total_entries = 0;
    uint16_t dropped_entries = 0;
    
    for (uint8_t i = 0; i < LOGGER_SERVICE_LEVEL_COUNT; i++) {
        total_entries += g_level_buffers[i].entry_count;
        // In production, track dropped entries per level
        // dropped_entries += g_level_buffers[i].dropped_count;
    }
    
    // Calculate buffer usage percentage (used for internal logic)
    uint16_t total_capacity = LOGGER_SERVICE_BUFFER_CAPACITY * LOGGER_SERVICE_LEVEL_COUNT;
    uint8_t usage_percentage = (total_entries * 100) / total_capacity;
    
    // Check if buffer usage is high (for internal monitoring)
    if (usage_percentage > 80) {
        // Log high buffer usage internally
        // This is for internal service monitoring, not returned to caller
    }
    
    // Prepare comprehensive status information
    logger_status_response_t status = {
        .entry_count = total_entries,
        .buffer_capacity = total_capacity,
        .dropped_count = dropped_entries,
        .storage_usage = g_logger_service.storage_available ? 
                        g_logger_service.current_write_offset : 0
    };
    
    // Store status in global variable for potential future use
    // This allows other internal functions to access the latest status
    g_last_status = status;
    
    // Return success status
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Get the last status information retrieved by logger_service_get_status
 * 
 * @param status Pointer to status structure to fill
 * @return system_error_t Error status
 */
system_error_t logger_service_get_last_status(logger_status_response_t* status) {
    // Safety check: null pointer validation
    if (status == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety check: verify service is running
    if (!g_logger_service.service_running) {
        return system_error_create(ERR_SYS_NOT_INIT, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_LOGGING);
    }
    
    // Copy the last status information
    *status = g_last_status;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}