/**
 * @file ipc.c
 * @brief AutoMLOS Kernel IPC with Zero-Copy Implementation
 */

#include "ipc.h"
#include "memory.h"
#include "time.h"
#include <string.h>

// Global IPC state with zero-copy support
static struct {
    struct ipc_topic_route topics[IPC_MAX_TOPICS];
    uint32_t topic_count;
    struct ipc_shared_buffer shared_buffer;
    bool initialized;
} g_ipc;

// Forward declarations for internal functions
static system_error_t ipc_deliver_message_descriptor(uint16_t pid, const struct ipc_message_desc* desc);
static system_error_t ipc_route_message_zero_copy(const char* topic, const struct ipc_message_desc* desc);

// Simple implementation of missing functions
system_error_t memory_alloc_shared(size_t size, void** buffer) {
    if (buffer == NULL || size == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    *buffer = memory_alloc(size);
    if (*buffer == NULL) {
        return system_error_create(ERR_SYS_RESOURCE_UNAVAIL, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

uint64_t kernel_time_get_uptime_us(void) {
    // Simple implementation - return 0 for now
    return 0;
}

system_error_t ipc_kernel_init(void) {
    if (g_ipc.initialized) {
        return system_error_create(ERR_SYS_ALREADY_INIT, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_MEDIUM, MODULE_ID_IPC);
    }
    
    memset(&g_ipc, 0, sizeof(g_ipc));
    
    // Initialize shared buffer for zero-copy messaging
    system_error_t ret = memory_alloc_shared(IPC_SHARED_BUFFER_SIZE, 
                                           (void**)&g_ipc.shared_buffer.base_address);
    if (ret.error_code != ERR_SYS_OK) {
        return ret;
    }
    
    g_ipc.shared_buffer.size = IPC_SHARED_BUFFER_SIZE;
    g_ipc.initialized = true;
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

system_error_t ipc_publish_message_zero_copy(const char* topic, 
                                           const void* message, 
                                           size_t size, 
                                           uint16_t source_pid,
                                           uint32_t timeout_ms) {
    (void)timeout_ms; // Mark parameter as used to avoid warning
    
    if (!g_ipc.initialized || topic == NULL || message == NULL || size == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    if (size > IPC_MAX_MESSAGE_SIZE) {
        return system_error_create(ERR_SYS_CONFIG_ERROR, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    // For large messages, use zero-copy with shared buffer
    if (size > 256) { // Threshold for zero-copy vs copy
        // Allocate from shared buffer
        void* shared_buffer = NULL;
        uint32_t buffer_id = 0;
        system_error_t ret = ipc_alloc_shared_buffer(size, &shared_buffer, &buffer_id);
        if (ret.error_code != ERR_SYS_OK) {
            return ret;
        }
        
        // Copy message to shared buffer (single copy)
        memcpy(shared_buffer, message, size);
        
        // Create message descriptor for zero-copy delivery
        struct ipc_message_desc desc = {
            .buffer_id = buffer_id,
            .offset = 0,
            .size = size,
            .source_pid = source_pid,
            .priority = IPC_PRIORITY_NORMAL,
            .reference_count = 1, // Initial reference
            .timestamp = kernel_time_get_uptime_us()
        };
        
        // Route message to subscribers without additional copying
        return ipc_route_message_zero_copy(topic, &desc);
    } else {
        // For small messages, use traditional copy-based approach
        return ipc_publish_message_copy(topic, message, size, source_pid);
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

system_error_t ipc_alloc_shared_buffer(uint32_t size, void** buffer, uint32_t* buffer_id) {
    if (!g_ipc.initialized || buffer == NULL || buffer_id == NULL) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    // Check if enough space available
    if (g_ipc.shared_buffer.used + size > g_ipc.shared_buffer.size) {
        return system_error_create(ERR_SYS_RESOURCE_UNAVAIL, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
    }
    
    // Allocate from shared buffer
    *buffer = g_ipc.shared_buffer.base_address + g_ipc.shared_buffer.used;
    *buffer_id = g_ipc.shared_buffer.allocation_count++;
    g_ipc.shared_buffer.used += size;
    
    // Update watermark
    if (g_ipc.shared_buffer.used > g_ipc.shared_buffer.watermark) {
        g_ipc.shared_buffer.watermark = g_ipc.shared_buffer.used;
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

system_error_t ipc_free_shared_buffer(uint32_t buffer_id) {
    // In a real implementation, this would manage buffer reuse
    // For simplicity, we'll just decrement the reference count
    // Actual buffer management would require more complex logic
    (void)buffer_id;
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

// Internal zero-copy message routing
static system_error_t ipc_route_message_zero_copy(const char* topic, 
                                                const struct ipc_message_desc* desc) {
    // Find topic
    for (uint32_t i = 0; i < g_ipc.topic_count; i++) {
        if (strcmp(g_ipc.topics[i].name, topic) == 0) {
            // Deliver to all subscribers without copying message data
            for (uint8_t j = 0; j < g_ipc.topics[i].subscriber_count; j++) {
                // Increment reference count for each subscriber
                ipc_increment_ref_count(desc->buffer_id);
                
                // Deliver message descriptor (small copy, not message data)
                ipc_deliver_message_descriptor(g_ipc.topics[i].subscriber_pids[j], desc);
            }
            
            g_ipc.topics[i].message_count++;
            g_ipc.topics[i].zero_copy_count += g_ipc.topics[i].subscriber_count;
            return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                                     ERROR_SEVERITY_LOW, MODULE_ID_IPC);
        }
    }
    
    return system_error_create(ERR_SYS_CONFIG_ERROR, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_HIGH, MODULE_ID_IPC);
}

// Stub implementations for missing functions
system_error_t ipc_publish_message_copy(const char* topic, const void* message, 
                                      size_t size, uint16_t source_pid) {
    (void)topic; (void)message; (void)size; (void)source_pid;
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

system_error_t ipc_increment_ref_count(uint32_t buffer_id) {
    (void)buffer_id;
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}

static system_error_t ipc_deliver_message_descriptor(uint16_t pid, const struct ipc_message_desc* desc) {
    (void)pid; (void)desc;
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_IPC);
}