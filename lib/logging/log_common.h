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
int log_verify_entry_integrity(const log_entry_binary_t* entry);
int log_buffer_put(log_buffer_t* buffer, const log_entry_binary_t* entry);
int log_buffer_get(log_buffer_t* buffer, log_entry_binary_t* entry);

// Safety verification macros
#define LOG_VERIFY_LEVEL(level) ((level) < LOG_LEVEL_COUNT)
#define LOG_VERIFY_TAG(tag) ((tag) < LOG_TAG_COUNT)
#define LOG_VERIFY_MESSAGE(msg) ((msg) < LOG_MSG_COUNT)

#endif // LOG_COMMON_H