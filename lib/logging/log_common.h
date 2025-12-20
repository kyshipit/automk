/**
 * @file log_common.h
 * @brief Common logging definitions for autoMLOS
 * 
 * Contains shared definitions used by both logging clients and logger service.
 * Prevents duplication and ensures consistency across the logging system.
 */

#ifndef LOG_COMMON_H
#define LOG_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include "../safety/system_errors.h"  // Correct relative path

// Log level definitions (aligned with automotive safety requirements)
typedef enum {
    LOG_LEVEL_EMERGENCY = 0,    // L0: Safety-critical messages
    LOG_LEVEL_ERROR,           // L1: System errors
    LOG_LEVEL_INFO,            // L2: Operational information  
    LOG_LEVEL_DEBUG,           // L3: Development debugging
    LOG_LEVEL_COUNT
} log_level_t;

// Predefined tag IDs (compile-time determined)
typedef enum {
    LOG_TAG_SYSTEM = 0,        // System-level messages
    LOG_TAG_KERNEL,            // Kernel operations
    LOG_TAG_DRIVER_CAMERA,     // Camera driver
    LOG_TAG_DRIVER_CAN,        // CAN bus driver
    LOG_TAG_SERVICE_AI,        // AI inference service
    LOG_TAG_SERVICE_DIAG,      // Diagnostic service
    LOG_TAG_SERVICE_LOGGER,    // Logger service itself
    LOG_TAG_APPLICATION,       // Application messages
    LOG_TAG_COUNT              // Maximum 256 tags supported
} log_tag_id_t;

// Predefined message IDs (compile-time determined)
typedef enum {
    LOG_MSG_SYSTEM_START = 0,      // System startup
    LOG_MSG_SYSTEM_SHUTDOWN,       // System shutdown
    LOG_MSG_SERVICE_READY,         // Service initialization complete
    LOG_MSG_SERVICE_ERROR,         // Service error
    LOG_MSG_DRIVER_INIT,           // Driver initialization
    LOG_MSG_DRIVER_TIMEOUT,        // Driver timeout
    LOG_MSG_AI_INFERENCE_START,    // AI inference started
    LOG_MSG_AI_INFERENCE_RESULT,   // AI inference completed
    LOG_MSG_DIAG_MESSAGE,          // Diagnostic message
    LOG_MSG_HEARTBEAT,             // Heartbeat monitoring
    LOG_MSG_COUNT                  // Maximum 65536 messages supported
} log_message_id_t;

/**
 * @brief Binary log entry structure (packed for efficient storage)
 * 
 * Total size: 20 bytes per entry
 * Designed for memory safety and time determinism
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;        // 4 bytes: System timestamp
    uint16_t process_id;       // 2 bytes: Source process ID
    uint8_t  level;            // 1 byte:  Log level (L0-L3)
    uint8_t  tag_id;           // 1 byte:  Predefined tag ID
    uint16_t message_id;       // 2 bytes: Predefined message ID
    uint32_t data[2];          // 8 bytes: Optional data payload
    uint16_t checksum;         // 2 bytes: Integrity verification
} log_entry_binary_t;

// Static assertion to ensure fixed size
_Static_assert(sizeof(log_entry_binary_t) == 20, "Log entry must be exactly 20 bytes");

/**
 * @brief Fixed-size log buffer structure
 * 
 * Uses circular buffer design with static allocation
 * Prevents buffer overflow through fixed capacity
 */
typedef struct {
    log_entry_binary_t entries[64];  // Fixed capacity: 64 entries (1280 bytes)
    uint16_t write_index;            // Current write position
    uint16_t read_index;             // Current read position  
    uint16_t entry_count;            // Number of entries in buffer
    uint16_t buffer_size;            // Total buffer capacity
    uint16_t safety_checksum;        // Buffer integrity verification
} log_buffer_t;

// Buffer capacity constant for safety checks
#define LOG_BUFFER_CAPACITY (sizeof(((log_buffer_t*)0)->entries) / sizeof(log_entry_binary_t))

// Static assertion for buffer capacity
_Static_assert(LOG_BUFFER_CAPACITY == 64, "Buffer capacity must be 64 entries");

// Common helper function declarations
uint16_t log_calculate_checksum(const log_entry_binary_t* entry);
system_error_t log_verify_entry_integrity(const log_entry_binary_t* entry);
system_error_t log_buffer_put(log_buffer_t* buffer, const log_entry_binary_t* entry);
system_error_t log_buffer_get(log_buffer_t* buffer, log_entry_binary_t* entry);

// Safety verification macros
#define LOG_VERIFY_LEVEL(level) ((level) < LOG_LEVEL_COUNT)
#define LOG_VERIFY_TAG(tag) ((tag) < LOG_TAG_COUNT)
#define LOG_VERIFY_MESSAGE(msg) ((msg) < LOG_MSG_COUNT)

// Unified safety check macros with consistent error handling
#define LOG_SAFETY_CHECK_LEVEL(level) \
    do { \
        if ((level) >= LOG_LEVEL_COUNT) { \
            return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM, \
                                     ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING); \
        } \
    } while(0)

#define LOG_SAFETY_CHECK_TAG(tag_id) \
    do { \
        if ((tag_id) >= LOG_TAG_COUNT) { \
            return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM, \
                                     ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING); \
        } \
    } while(0)

#define LOG_SAFETY_CHECK_MESSAGE_ID(msg_id) \
    do { \
        if ((msg_id) >= LOG_MSG_COUNT) { \
            return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM, \
                                     ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING); \
        } \
    } while(0)

// Unified configuration constants for logging system
#define LOG_CONFIG_BUFFER_CAPACITY          64      // Maximum entries in buffer
#define LOG_CONFIG_IPC_TIMEOUT_MS          1000     // IPC communication timeout
#define LOG_CONFIG_IPC_MAX_RETRIES           2      // Maximum IPC retry attempts
#define LOG_CONFIG_CHECKSUM_VERIFY           1      // Enable checksum verification
#define LOG_CONFIG_BOUNDARY_CHECK            1      // Enable boundary checking
#define LOG_CONFIG_SAFETY_LEVEL              2      // Safety level (0-3)

// Configuration structure for runtime adjustments
/**
 * @brief Logging system configuration structure
 * 
 * Centralized configuration management for logging system
 */
typedef struct {
    uint16_t buffer_capacity;          // Buffer capacity in entries
    uint16_t entry_size;               // Size of each log entry
    uint32_t ipc_timeout_ms;           // IPC communication timeout
    uint8_t  max_retry_count;          // Maximum retry attempts
    uint16_t checksum_algorithm;       // Checksum algorithm identifier
    uint8_t  safety_level;             // Safety compliance level
} log_config_t;

// Default configuration values (use correct member names)
#define LOG_CONFIG_DEFAULT { \
    .buffer_capacity = LOG_CONFIG_BUFFER_CAPACITY, \
    .entry_size = sizeof(log_entry_binary_t), \
    .ipc_timeout_ms = LOG_CONFIG_IPC_TIMEOUT_MS, \
    .max_retry_count = LOG_CONFIG_IPC_MAX_RETRIES, \
    .checksum_algorithm = 1, \
    .safety_level = LOG_CONFIG_SAFETY_LEVEL \
}

// Configuration management functions
system_error_t log_config_init(log_config_t* config);
system_error_t log_config_validate(const log_config_t* config);
system_error_t log_config_set(const log_config_t* new_config);
system_error_t log_config_get(log_config_t* current_config);

// Service communication message types (shared between client and service)
// Use explicit uint32_t type for enum values to ensure fixed size
typedef enum {
    LOG_SERVICE_MSG_ENTRY = 1,        // Log entry submission
    LOG_SERVICE_MSG_STATUS_QUERY,     // Buffer status query
    LOG_SERVICE_MSG_RETRIEVE_LOGS,    // Log retrieval request
    LOG_SERVICE_MSG_CLEAR_BUFFER,     // Buffer clear request
    LOG_SERVICE_MSG_SET_LEVEL,        // Log level configuration
} log_service_msg_type_t;

// Ensure enum size is 4 bytes by using explicit type
typedef uint32_t log_service_msg_type_fixed_t;

/**
 * @brief Service message for log entry submission
 * 
 * Used by client processes to submit log entries to the logger service.
 */
typedef struct __attribute__((packed)) {
    log_service_msg_type_fixed_t type; // Message type: LOG_SERVICE_MSG_ENTRY (4 bytes)
    log_entry_binary_t entry;         // Binary log entry (20 bytes)
    uint16_t source_pid;              // Source process ID for verification (2 bytes)
    uint8_t reserved;                 // Alignment padding (1 byte)
    uint8_t padding;                  // Additional padding for alignment (1 byte)
} log_service_msg_entry_t;

// Static assertion to ensure fixed size (4 + 20 + 2 + 1 + 1 = 28 bytes)
_Static_assert(sizeof(log_service_msg_entry_t) == 28, "Log entry service message must be 28 bytes");

/**
 * @brief Service message for buffer status query
 * 
 * Used to query the current state of the logger service buffer.
 */
typedef struct __attribute__((packed)) {
    log_service_msg_type_fixed_t type; // Message type: LOG_SERVICE_MSG_STATUS_QUERY (4 bytes)
    uint16_t requester_pid;           // PID of requesting process (2 bytes)
    uint16_t reserved;                // Alignment padding (2 bytes)
} log_service_msg_status_t;

/**
 * @brief Service response for buffer status
 */
typedef struct __attribute__((packed)) {
    uint16_t entry_count;             // Current number of entries in buffer
    uint16_t buffer_capacity;         // Maximum buffer capacity
    uint32_t dropped_count;           // Number of dropped entries (L2/L3)
    uint32_t storage_usage;           // Storage medium usage in bytes
} log_service_response_status_t;

/**
 * @brief Service message for log retrieval
 * 
 * Used to retrieve log entries from the logger service.
 */
typedef struct __attribute__((packed)) {
    log_service_msg_type_fixed_t type; // Message type: LOG_SERVICE_MSG_RETRIEVE_LOGS (4 bytes)
    uint16_t requester_pid;           // PID of requesting process (2 bytes)
    uint8_t  level_filter;            // Filter by log level (0 = all) (1 byte)
    uint8_t  max_entries;             // Maximum entries to retrieve (0 = all) (1 byte)
    uint32_t start_timestamp;         // Start timestamp filter (0 = from beginning) (4 bytes)
} log_service_msg_retrieve_t;

/**
 * @brief Service response for log retrieval
 */
typedef struct __attribute__((packed)) {
    uint16_t entry_count;             // Number of entries in this response (2 bytes)
    uint16_t total_entries;           // Total entries matching filter (2 bytes)
    log_entry_binary_t entries[16];   // Batch of log entries (16 * 20 = 320 bytes)
} log_service_response_retrieve_t;

// Static assertion for retrieve response size (2 + 2 + 320 = 324 bytes)
_Static_assert(sizeof(log_service_response_retrieve_t) == 324, 
               "Retrieve response size must be 324 bytes");

// Service name constant
#define LOGGER_SERVICE_NAME           "system/logger"

// Service communication configuration
#define LOG_SERVICE_DEFAULT_TIMEOUT_MS   1000     // Default communication timeout
#define LOG_SERVICE_MAX_RETRY_COUNT       3       // Maximum retry attempts
#define LOG_SERVICE_RETRY_DELAY_MS       50       // Delay between retries

#endif // LOG_COMMON_H