/**
 * @file logger_service.h
 * @brief Logger service interface for autoMLOS
 * 
 * Defines the logger service API with unified error handling and
 * proper module boundaries.
 */

#ifndef LOGGER_SERVICE_H
#define LOGGER_SERVICE_H

#include <stddef.h>  // Add for size_t definition
#include "../../lib/safety/system_errors.h"

// Service constants
#define LOGGER_SERVICE_NAME "system/logger"
#define LOGGER_SERVICE_PID 100

// Service-specific buffer configuration (different from client library)
#define LOGGER_SERVICE_BUFFER_CAPACITY 100
#define LOGGER_SERVICE_LEVEL_COUNT 6

// Service message types for IPC communication
typedef enum {
    LOGGER_MSG_LOG_ENTRY = 1,
    LOGGER_MSG_BUFFER_STATUS = 2,
    LOGGER_MSG_RETRIEVE_LOGS = 3
} logger_msg_type_t;

// Service-specific log entry structure (decoupled from library)
typedef struct __attribute__((packed)) {
    uint32_t timestamp;        // System timestamp
    uint16_t process_id;       // Source process ID
    uint8_t  level;            // Log level
    uint8_t  tag_id;           // Tag ID
    uint16_t message_id;       // Message ID
    uint32_t data[2];          // Data payload
    uint16_t checksum;         // Integrity checksum
} logger_service_entry_t;

_Static_assert(sizeof(logger_service_entry_t) == 20, "Service entry must be 20 bytes");

// IPC message structures for service communication
typedef struct __attribute__((packed)) {
    uint8_t type;                    // LOGGER_MSG_LOG_ENTRY
    uint16_t source_pid;             // Source process ID
    logger_service_entry_t entry;    // Log entry data
} logger_msg_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t type;                   // LOGGER_MSG_BUFFER_STATUS
    uint16_t requester_pid;         // Requesting process ID
} logger_msg_status_t;

typedef struct __attribute__((packed)) {
    uint16_t entry_count;           // Total entries in buffers
    uint16_t buffer_capacity;       // Total buffer capacity
    uint16_t dropped_count;         // Number of dropped entries
    uint32_t storage_usage;         // Storage usage in bytes
} logger_status_response_t;

// Service API with unified error handling
system_error_t logger_service_init(void);
system_error_t logger_service_process_message(const void* message, size_t size);
system_error_t logger_service_get_status(void);
system_error_t logger_service_get_last_status(logger_status_response_t* status);

#endif // LOGGER_SERVICE_H