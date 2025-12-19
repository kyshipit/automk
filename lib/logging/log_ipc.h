/**
 * @file log_ipc.h
 * @brief IPC message definitions for binary logging system
 * 
 * Defines the IPC message types and structures used for communication
 * between logging clients and the logger service.
 */

#ifndef LOG_IPC_H
#define LOG_IPC_H

#include "log_common.h"  // Use common logging definitions

// IPC message types for logging system
typedef enum {
    LOG_IPC_MSG_LOG_ENTRY = 1000,    // Log entry submission
    LOG_IPC_MSG_BUFFER_STATUS,       // Buffer status query
    LOG_IPC_MSG_RETRIEVE_LOGS,       // Log retrieval request
    LOG_IPC_MSG_CLEAR_BUFFER,        // Buffer clear request
    LOG_IPC_MSG_SET_LEVEL,           // Log level configuration
} log_ipc_msg_type_t;

/**
 * @brief IPC message for log entry submission
 * 
 * Used by client processes to submit log entries to the logger service.
 */
typedef struct __attribute__((packed)) {
    log_ipc_msg_type_t type;         // Message type: LOG_IPC_MSG_LOG_ENTRY
    log_entry_binary_t entry;        // Binary log entry (20 bytes)
    uint16_t source_pid;             // Source process ID for verification
    uint16_t reserved;               // Alignment padding
} log_ipc_msg_entry_t;

_Static_assert(sizeof(log_ipc_msg_entry_t) == 28, "Log entry IPC message must be 28 bytes");

/**
 * @brief IPC message for buffer status query
 * 
 * Used to query the current state of the logger service buffer.
 */
typedef struct __attribute__((packed)) {
    log_ipc_msg_type_t type;         // Message type: LOG_IPC_MSG_BUFFER_STATUS
    uint16_t requester_pid;          // PID of requesting process
    uint16_t reserved;               // Alignment padding
} log_ipc_msg_status_t;

/**
 * @brief IPC response for buffer status
 */
typedef struct __attribute__((packed)) {
    uint16_t entry_count;            // Current number of entries in buffer
    uint16_t buffer_capacity;        // Maximum buffer capacity
    uint32_t dropped_count;          // Number of dropped entries (L2/L3)
    uint32_t storage_usage;          // Storage medium usage in bytes
} log_ipc_response_status_t;

/**
 * @brief IPC message for log retrieval
 * 
 * Used to retrieve log entries from the logger service.
 */
typedef struct __attribute__((packed)) {
    log_ipc_msg_type_t type;         // Message type: LOG_IPC_MSG_RETRIEVE_LOGS
    uint16_t requester_pid;          // PID of requesting process
    uint8_t  level_filter;           // Filter by log level (0 = all)
    uint8_t  max_entries;            // Maximum entries to retrieve (0 = all)
    uint32_t start_timestamp;        // Start timestamp filter (0 = from beginning)
} log_ipc_msg_retrieve_t;

/**
 * @brief IPC response for log retrieval
 */
typedef struct __attribute__((packed)) {
    uint16_t entry_count;            // Number of entries in this response
    uint16_t total_entries;          // Total entries matching filter
    log_entry_binary_t entries[16];  // Batch of log entries (max 16 per message)
} log_ipc_response_retrieve_t;

_Static_assert(sizeof(log_ipc_response_retrieve_t) == (4 + 16 * 20), 
               "Retrieve response size must be correct");

// IPC topic definitions for publish-subscribe pattern
#define LOG_TOPIC_SYSTEM_MESSAGES    "log/system/messages"
#define LOG_TOPIC_BUFFER_STATUS      "log/system/status"  
#define LOG_TOPIC_STORAGE_ALERTS     "log/system/alerts"
#define LOG_TOPIC_EMERGENCY_LOGS     "log/emergency"
#define LOG_TOPIC_ERROR_LOGS         "log/error"
#define LOG_TOPIC_INFO_LOGS          "log/info"
#define LOG_TOPIC_DEBUG_LOGS         "log/debug"

// Function declarations for IPC integration
// Ensure these function declarations are available for client use
int log_send_entry_ipc(const log_entry_binary_t* entry);
int log_query_status_ipc(log_ipc_response_status_t* status);
int log_retrieve_entries_ipc(uint8_t level_filter, uint8_t max_entries, 
                            uint32_t start_timestamp, log_entry_binary_t* entries, 
                            uint16_t* entry_count);

#endif // LOG_IPC_H