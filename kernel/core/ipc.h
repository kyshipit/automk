/**
 * @file ipc.h
 * @brief AutoMLOS Kernel IPC with Zero-Copy Design
 * 
 * Enhanced IPC with zero-copy message passing for automotive embedded systems
 * with ISO 26262 ASIL-D compliance.
 */

#ifndef KERNEL_IPC_H
#define KERNEL_IPC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../safety/system_errors.h"

// IPC configuration constants
#define IPC_MAX_TOPICS                256     // Maximum topics supported
#define IPC_MAX_SUBSCRIBERS_PER_TOPIC 32      // Maximum subscribers per topic
#define IPC_QUEUE_SIZE                1024    // Message queue size
#define IPC_MAX_MESSAGE_SIZE          4096    // Maximum message size
#define IPC_SHARED_BUFFER_SIZE        (64 * 1024) // 64KB shared buffer

// Message priority levels (aligned with automotive requirements)
typedef enum {
    IPC_PRIORITY_EMERGENCY = 0,   // 0-31: Safety-critical messages
    IPC_PRIORITY_HIGH = 32,        // 32-63: Control commands
    IPC_PRIORITY_NORMAL = 64,      // 64-191: Operational messages
    IPC_PRIORITY_LOW = 192         // 192-255: Debug/statistics
} ipc_priority_t;

// Zero-copy message descriptor
struct ipc_message_desc {
    uint32_t buffer_id;           // Shared buffer identifier
    uint32_t offset;              // Offset in shared buffer
    uint32_t size;                // Message size
    uint16_t source_pid;          // Source process ID
    uint8_t priority;             // Message priority
    uint8_t reference_count;      // Reference count for zero-copy
    uint64_t timestamp;          // Timestamp (microseconds)
    uint32_t sequence_number;    // Sequence number for ordering
};

// Topic routing structure with zero-copy support
struct ipc_topic_route {
    char name[64];                              // Topic name
    uint16_t subscriber_pids[IPC_MAX_SUBSCRIBERS_PER_TOPIC]; // Subscriber PIDs
    uint8_t subscriber_count;                   // Active subscriber count
    uint8_t priority;                           // Topic priority
    uint32_t message_count;                     // Statistics: messages routed
    uint32_t zero_copy_count;                   // Zero-copy messages delivered
};

// Shared buffer management
struct ipc_shared_buffer {
    uint8_t* base_address;      // Base address of shared buffer
    uint32_t size;               // Total buffer size
    uint32_t used;               // Used bytes
    uint32_t watermark;          // High watermark for monitoring
    uint32_t allocation_count;   // Number of allocations
};

// Kernel IPC management functions
system_error_t ipc_kernel_init(void);
system_error_t ipc_register_topic(const char* topic_name, uint32_t flags);
system_error_t ipc_unregister_topic(const char* topic_name);
system_error_t ipc_subscribe_topic(const char* topic_name, uint16_t subscriber_pid);
system_error_t ipc_unsubscribe_topic(const char* topic_name, uint16_t subscriber_pid);

// Zero-copy message publishing
system_error_t ipc_publish_message_zero_copy(const char* topic, 
                                           const void* message, 
                                           size_t size, 
                                           uint16_t source_pid,
                                           uint32_t timeout_ms);

// Traditional copy-based publishing (for small messages)
system_error_t ipc_publish_message_copy(const char* topic, 
                                      const void* message, 
                                      size_t size, 
                                      uint16_t source_pid);

// Message reception with zero-copy support
system_error_t ipc_receive_message_zero_copy(uint16_t target_pid, 
                                           struct ipc_message_desc* desc,
                                           uint32_t timeout_ms);

system_error_t ipc_receive_message_copy(uint16_t target_pid, 
                                      void* buffer, 
                                      size_t buffer_size,
                                      uint32_t timeout_ms);

// Shared buffer management
system_error_t ipc_alloc_shared_buffer(uint32_t size, void** buffer, uint32_t* buffer_id);
system_error_t ipc_free_shared_buffer(uint32_t buffer_id);
system_error_t ipc_get_shared_buffer(uint32_t buffer_id, void** buffer, uint32_t* size);

// Reference counting for zero-copy
system_error_t ipc_increment_ref_count(uint32_t buffer_id);
system_error_t ipc_decrement_ref_count(uint32_t buffer_id);

void ipc_dispatch_messages(void);  // Called by scheduler

// Statistics and monitoring
uint32_t ipc_get_topic_count(void);
uint32_t ipc_get_queue_usage(void);
uint32_t ipc_get_shared_buffer_usage(void);
system_error_t ipc_get_topic_stats(const char* topic, struct ipc_topic_route* stats);

#endif // KERNEL_IPC_H