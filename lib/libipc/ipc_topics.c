/**
 * @file ipc_topics.c
 * @brief IPC topic management implementation with enhanced safety
 */

#include "ipc_topics.h"
#include <string.h>
#include <stdio.h>

// Safety: Maximum topic length for buffer safety
#define MAX_TOPIC_LENGTH 64

// Reserved namespaces (system-critical topics)
static const char* RESERVED_NAMESPACES[] = {
    "system/", "kernel/", "safety/", NULL
};

// Topic base strings (aligned with msg_types.md)
static const char* TOPIC_BASE_STRINGS[] = {
    [IPC_TOPIC_CATEGORY_VISION] = "vision",
    [IPC_TOPIC_CATEGORY_VEHICLE] = "vehicle", 
    [IPC_TOPIC_CATEGORY_AI] = "ai",
    [IPC_TOPIC_CATEGORY_SYSTEM] = "system",
    [IPC_TOPIC_CATEGORY_SAFETY] = "safety",
    [IPC_TOPIC_CATEGORY_COMMUNICATION] = "communication"
};

// System subcategory strings
static const char* TOPIC_SYSTEM_STRINGS[] = {
    [IPC_TOPIC_SYSTEM_MONITOR] = "monitor",
    [IPC_TOPIC_SYSTEM_DIAGNOSTIC] = "diagnostic",
    [IPC_TOPIC_SYSTEM_LOGGING] = "logging",
    [IPC_TOPIC_SYSTEM_POWER] = "power"
};

// Logging subcategory strings  
static const char* TOPIC_LOGGING_STRINGS[] = {
    [IPC_TOPIC_LOGGING_EMERGENCY] = "emergency",
    [IPC_TOPIC_LOGGING_ERROR] = "error",
    [IPC_TOPIC_LOGGING_INFO] = "info",
    [IPC_TOPIC_LOGGING_DEBUG] = "debug"
};

/**
 * @brief Enhanced topic validation with comprehensive safety checks
 */
ipc_topic_validation_t ipc_validate_topic(const char* topic) {
    // Safety: Null pointer check
    if (topic == NULL) {
        return IPC_TOPIC_INVALID_FORMAT;
    }
    
    // Safety: Length check
    size_t topic_len = strlen(topic);
    if (topic_len == 0 || topic_len > MAX_TOPIC_LENGTH) {
        return IPC_TOPIC_LENGTH_EXCEEDED;
    }
    
    // Safety: Format validation (must contain exactly 2 slashes)
    int slash_count = 0;
    for (size_t i = 0; i < topic_len; i++) {
        if (topic[i] == '/') {
            slash_count++;
        }
    }
    
    if (slash_count != 2) {
        return IPC_TOPIC_INVALID_FORMAT;
    }
    
    // Safety: Reserved namespace check
    for (int i = 0; RESERVED_NAMESPACES[i] != NULL; i++) {
        if (strncmp(topic, RESERVED_NAMESPACES[i], 
                   strlen(RESERVED_NAMESPACES[i])) == 0) {
            return IPC_TOPIC_RESERVED_NAMESPACE;
        }
    }
    
    return IPC_TOPIC_VALID;
}

/**
 * @brief Enhanced topic construction with parameter validation
 */
const char* ipc_build_topic(ipc_topic_category_t category, 
                           ipc_topic_system_t system_type,
                           ipc_topic_logging_t logging_type) {
    static char topic_buffer[MAX_TOPIC_LENGTH + 1];
    
    // Safety: Clear buffer
    memset(topic_buffer, 0, sizeof(topic_buffer));
    
    // Safety: Parameter validation
    if (category >= IPC_TOPIC_CATEGORY_COUNT) {
        return NULL;
    }
    
    // Build topic based on validated parameters
    switch (category) {
        case IPC_TOPIC_CATEGORY_SYSTEM:
            if (system_type >= IPC_TOPIC_SYSTEM_COUNT) {
                return NULL;
            }
            
            switch (system_type) {
                case IPC_TOPIC_SYSTEM_LOGGING:
                    if (logging_type >= IPC_TOPIC_LOGGING_COUNT) {
                        return NULL;
                    }
                    
                    snprintf(topic_buffer, sizeof(topic_buffer), 
                            "%s/%s/%s", 
                            TOPIC_BASE_STRINGS[category],
                            TOPIC_SYSTEM_STRINGS[system_type],
                            TOPIC_LOGGING_STRINGS[logging_type]);
                    break;
                    
                default:
                    // Handle other system types
                    return NULL; // Not implemented
            }
            break;
            
        default:
            // Handle other categories
            return NULL; // Not implemented
    }
    
    // Final validation
    if (ipc_validate_topic(topic_buffer) != IPC_TOPIC_VALID) {
        return NULL;
    }
    
    return topic_buffer;
}