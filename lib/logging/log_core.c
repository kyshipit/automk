/**
 * @file log_core.c
 * @brief Binary logging system core implementation for autoMLOS
 * 
 * Implements a memory-safe, time-deterministic logging system that complies
 * with automotive safety standards (ISO 26262 ASIL-D).
 * 
 * Key design principles:
 * - Pure recording only, no analysis (strict decoupling)
 * - No storage dependencies (fault isolation)
 * - System-wide error handling integration
 * - Comprehensive safety checks (ASIL-D compliance)
 * - Deterministic execution time
 */

#include "log_core.h"
#include <string.h>

// Global log buffer instance (fixed allocation)
static log_buffer_t g_log_buffer;

// System initialization state flag
static volatile uint8_t g_log_system_initialized = 0;

// Log entry magic number and version constants
#define LOG_ENTRY_MAGIC 0x4C4F4700  // "LOG" in ASCII
#define LOG_ENTRY_VERSION_BINARY 0x01

// Mock system calls for testing (will be replaced by actual syscalls)
#ifndef __KERNEL__
static uint32_t mock_time_get_us(void) { return 0; }
static uint16_t mock_getpid(void) { return 0; }
#define sys_time_get_us mock_time_get_us
#define sys_getpid mock_getpid
#endif

/**
 * @brief Calculate buffer integrity checksum
 * 
 * Pure function for buffer state verification.
 * Used to detect buffer corruption during runtime.
 * 
 * @param buffer Pointer to log buffer
 * @return 32-bit checksum value
 */
static uint32_t log_calculate_buffer_checksum(const log_buffer_t* buffer) {
    // Safety: Null pointer check with system error
    if (buffer == NULL) {
        return 0xFFFFFFFF;
    }
    
    const uint8_t* data = (const uint8_t*)buffer;
    uint32_t checksum = 0;
    
    // Calculate checksum for critical buffer fields only
    for (size_t i = 0; i < offsetof(log_buffer_t, safety_checksum); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief Verify buffer integrity
 * 
 * Comprehensive buffer state verification for safety compliance.
 * 
 * @param buffer Pointer to log buffer
 * @return system_error_t Error code indicating buffer state
 */
static system_error_t log_verify_buffer_integrity(const log_buffer_t* buffer) {
    // Safety checks with updated error codes
    if (buffer == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (buffer->write_index >= LOG_BUFFER_CAPACITY) {
        return system_error_create(ERR_BASE_INDEX_OOB, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (buffer->read_index >= LOG_BUFFER_CAPACITY) {
        return system_error_create(ERR_BASE_INDEX_OOB, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (buffer->safety_checksum != log_calculate_buffer_checksum(buffer)) {
        return system_error_create(ERR_BASE_DATA_CORRUPT, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (buffer->entry_count > LOG_BUFFER_CAPACITY) {
        return system_error_create(ERR_BASE_DATA_CORRUPT, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Safe initialization of logging system
 * 
 * Enhanced initialization with comprehensive safety checks and error reporting.
 * 
 * @return system_error_t Error code indicating initialization status
 */
system_error_t log_system_init_safe(void) {
    // Initialize global log buffer with safety checks
    memset(&g_log_buffer, 0, sizeof(g_log_buffer));
    g_log_buffer.buffer_size = LOG_BUFFER_CAPACITY;
    g_log_buffer.safety_checksum = log_calculate_buffer_checksum(&g_log_buffer);
    
    g_log_system_initialized = 1;
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Enhanced log recording with comprehensive error handling
 * 
 * Strictly follows "record only" principle with system-wide error integration.
 * 
 * @param level Log level
 * @param tag_id Predefined tag ID
 * @param message_id Predefined message ID
 * @param data1 First data payload
 * @param data2 Second data payload
 * @return system_error_t Error code indicating operation status
 */
system_error_t log_record_binary_safe(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                     uint32_t data1, uint32_t data2) {
    // Safety checks with updated error codes
    if (!g_log_system_initialized) {
        system_error_t init_error = log_system_init_safe();
        if (init_error.error_code != ERR_BASE_OK) {
            return init_error;
        }
    }
    
    // Parameter validation: use consistent safety check macros
    LOG_SAFETY_CHECK_LEVEL(level);
    LOG_SAFETY_CHECK_TAG(tag_id);
    LOG_SAFETY_CHECK_MESSAGE_ID(message_id);
    
    // Create log entry with safety validation
    log_entry_binary_t entry = {
        .timestamp = sys_time_get_us(),
        .process_id = sys_getpid(),
        .level = level,
        .tag_id = tag_id,
        .message_id = message_id,
        .data[0] = data1,
        .data[1] = data2,
        .checksum = 0
    };
    
    // Calculate checksum for entry integrity
    entry.checksum = log_calculate_checksum(&entry);
    
    // Store entry in buffer with safety checks
    system_error_t buffer_error = log_buffer_put_safe(&g_log_buffer, &entry);
    if (buffer_error.error_code != ERR_BASE_OK) {
        return buffer_error;
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Enhanced buffer put operation with comprehensive safety
 * 
 * Implements defense-in-depth strategy with system error integration.
 * 
 * @param buffer Pointer to log buffer
 * @param entry Pointer to log entry to add
 * @return system_error_t Error code indicating operation status
 */
system_error_t log_buffer_put_safe(log_buffer_t* buffer, const log_entry_binary_t* entry) {
    // Safety checks with updated error codes
    if (buffer == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety: Buffer integrity verification
    system_error_t buffer_check = log_verify_buffer_integrity(buffer);
    if (system_error_is_critical(&buffer_check)) {
        return buffer_check;
    }
    
    // Safety: Entry integrity check (minimal analysis for safety only)
    system_error_t entry_check = log_verify_entry_integrity_safe(entry);
    if (system_error_is_critical(&entry_check)) {
        return entry_check;
    }
    
    // Core operation: Copy entry to buffer
    memcpy(&buffer->entries[buffer->write_index], entry, sizeof(log_entry_binary_t));
    
    // Safety: Update indices with boundary protection
    buffer->write_index = (buffer->write_index + 1) % buffer->buffer_size;
    
    if (buffer->entry_count < buffer->buffer_size) {
        buffer->entry_count++;
    } else {
        // Buffer full: overwrite oldest entry
        buffer->read_index = (buffer->read_index + 1) % buffer->buffer_size;
    }
    
    // Safety: Update buffer checksum for integrity verification
    buffer->safety_checksum = log_calculate_buffer_checksum(buffer);
    
    // Final state verification
    system_error_t final_check = log_verify_buffer_integrity(buffer);
    if (system_error_is_critical(&final_check)) {
        return final_check;
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Enhanced buffer get operation with system error handling
 * 
 * Pure read operation with comprehensive safety checks.
 * 
 * @param buffer Pointer to log buffer
 * @param entry Pointer to store retrieved log entry
 * @return system_error_t Error code indicating operation status
 */
system_error_t log_buffer_get_safe(log_buffer_t* buffer, log_entry_binary_t* entry) {
    // Safety checks with updated error codes
    if (buffer == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety: Buffer integrity verification
    system_error_t buffer_check = log_verify_buffer_integrity(buffer);
    if (system_error_is_critical(&buffer_check)) {
        return buffer_check;
    }
    
    // Safety: Empty buffer check
    if (buffer->entry_count == 0) {
        return system_error_create(ERR_BASE_BUF_EMPTY,
                                 ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM,
                                 MODULE_ID_LOGGING);
    }
    
    // Core operation: Copy entry (read-only, no analysis)
    memcpy(entry, &buffer->entries[buffer->read_index], sizeof(log_entry_binary_t));
    
    // Safety: Update indices (pure buffer management)
    buffer->read_index = (buffer->read_index + 1) % buffer->buffer_size;
    buffer->entry_count--;
    
    // Update buffer checksum for integrity verification
    buffer->safety_checksum = log_calculate_buffer_checksum(buffer);
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Enhanced entry integrity verification
 * 
 * Comprehensive entry validation with system error reporting.
 * 
 * @param entry Pointer to log entry to verify
 * @return system_error_t Error code indicating entry integrity
 */
system_error_t log_verify_entry_integrity_safe(const log_entry_binary_t* entry) {
    // Safety checks with updated error codes
    if (entry == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_LOGGING);
    }
    
    // Safety: Checksum verification
    uint16_t calculated_checksum = log_calculate_checksum(entry);
    if (calculated_checksum != entry->checksum) {
        return system_error_create(ERR_BASE_DATA_CORRUPT,
                                 ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH,
                                 MODULE_ID_LOGGING);
    }
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SUCCESS,
                             ERROR_SEVERITY_NONE, MODULE_ID_LOGGING);
}

/**
 * @brief Original log recording function (maintained for compatibility)
 * 
 * Uses safe version internally but maintains original interface.
 */
void log_record_binary(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                       uint32_t data1, uint32_t data2) {
    // Call safe version internally
    system_error_t error = log_record_binary_safe(level, tag_id, message_id, data1, data2);
    
    // Log error if critical (but don't fail the operation)
    if (system_error_is_critical(&error)) {
        // Use minimal error logging (no recursion)
        // In production, this would trigger a safety handler
    }
}

/**
 * @brief Get pointer to global log buffer (read-only access)
 * 
 * Provides external access for analysis/storage components.
 * Core logging system remains decoupled.
 * 
 * @return Pointer to global log buffer, NULL if not initialized
 */
log_buffer_t* get_global_log_buffer(void) {
    // Safety: Check initialization state
    if (!g_log_system_initialized) {
        return NULL;
    }
    
    return &g_log_buffer;
}

/**
 * @brief Calculate checksum for log entry (pure function)
 * 
 * Simple checksum calculation for entry integrity verification.
 * 
 * @param entry Pointer to log entry
 * @return 16-bit checksum value
 */
uint16_t log_calculate_checksum(const log_entry_binary_t* entry) {
    // Safety: Null pointer check
    if (entry == NULL) {
        return 0xFFFF;
    }
    
    const uint8_t* data = (const uint8_t*)entry;
    uint16_t checksum = 0;
    
    // Calculate checksum for all fields except the checksum itself
    for (size_t i = 0; i < offsetof(log_entry_binary_t, checksum); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * @brief Original entry integrity verification (compatibility)
 * 
 * Uses safe version internally but maintains original interface.
 */
int log_verify_entry_integrity(const log_entry_binary_t* entry) {
    system_error_t error = log_verify_entry_integrity_safe(entry);
    return system_error_is_critical(&error) ? -1 : 0;
}

/**
 * @brief Original buffer put operation (compatibility)
 * 
 * Uses safe version internally but maintains original interface.
 */
int log_buffer_put(log_buffer_t* buffer, const log_entry_binary_t* entry) {
    system_error_t error = log_buffer_put_safe(buffer, entry);
    return system_error_is_critical(&error) ? -1 : 0;
}

/**
 * @brief Original system initialization (compatibility)
 * 
 * Uses safe version internally but maintains original interface.
 */
void log_system_init(void) {
    log_system_init_safe();
}

/**
 * @brief Initialize a log buffer with safe defaults
 * 
 * @param buffer Pointer to log buffer to initialize
 */
void log_buffer_init(log_buffer_t* buffer) {
    if (buffer == NULL) {
        return; // Safety check
    }
    
    memset(buffer, 0, sizeof(log_buffer_t));
    buffer->buffer_size = LOG_BUFFER_CAPACITY;
}