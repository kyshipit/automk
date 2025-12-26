/**
 * @file ipc_topics.c
 * @brief IPC topic management implementation with enhanced safety
 * 
 * Unified topic management implementation providing comprehensive
 * topic validation, construction, and management functionality.
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
    [IPC_TOPIC_CATEGORY_COMMUNICATION] = "communication",
    [IPC_TOPIC_CATEGORY_DIAGNOSTICS] = "diagnostics",
    [IPC_TOPIC_CATEGORY_POWER] = "power"
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

// Vehicle subcategory strings
static const char* TOPIC_VEHICLE_STRINGS[] = {
    [IPC_TOPIC_VEHICLE_CONTROL] = "control",
    [IPC_TOPIC_VEHICLE_SENSOR] = "sensor",
    [IPC_TOPIC_VEHICLE_STATUS] = "status",
    [IPC_TOPIC_VEHICLE_COMMAND] = "command"
};

// Vision subcategory strings
static const char* TOPIC_VISION_STRINGS[] = {
    [IPC_TOPIC_VISION_CAMERA] = "camera",
    [IPC_TOPIC_VISION_OBJECT] = "object",
    [IPC_TOPIC_VISION_LANE] = "lane",
    [IPC_TOPIC_VISION_DETECTION] = "detection"
};

// AI subcategory strings
static const char* TOPIC_AI_STRINGS[] = {
    [IPC_TOPIC_AI_INFERENCE] = "inference",
    [IPC_TOPIC_AI_TRAINING] = "training",
    [IPC_TOPIC_AI_MODEL] = "model",
    [IPC_TOPIC_AI_RESULT] = "result"
};

// Safety subcategory strings
static const char* TOPIC_SAFETY_STRINGS[] = {
    [IPC_TOPIC_SAFETY_STATUS] = "status",
    [IPC_TOPIC_SAFETY_EVENT] = "event",
    [IPC_TOPIC_SAFETY_COMPONENT] = "component",
    [IPC_TOPIC_SAFETY_CRITICAL] = "critical"
};

// Communication subcategory strings
static const char* TOPIC_COMMUNICATION_STRINGS[] = {
    [IPC_TOPIC_COMMUNICATION_CAN] = "can",
    [IPC_TOPIC_COMMUNICATION_ETHERNET] = "ethernet",
    [IPC_TOPIC_COMMUNICATION_WIRELESS] = "wireless",
    [IPC_TOPIC_COMMUNICATION_PACKET] = "packet"
};

// Predefined system topics (NULL-terminated array)
static const char* PREDEFINED_TOPICS[] = {
    // System logging topics
    IPC_TOPIC_SYSTEM_LOGGING_EMERGENCY,
    IPC_TOPIC_SYSTEM_LOGGING_ERROR,
    IPC_TOPIC_SYSTEM_LOGGING_INFO,
    IPC_TOPIC_SYSTEM_LOGGING_DEBUG,
    
    // System management topics
    IPC_TOPIC_SYSTEM_MONITOR_STATUS,
    IPC_TOPIC_SYSTEM_DIAGNOSTIC_STATUS,
    IPC_TOPIC_SYSTEM_POWER_STATUS,
    
    // Watchdog topics
    IPC_TOPIC_SYSTEM_WATCHDOG_STATUS,
    IPC_TOPIC_SYSTEM_WATCHDOG_EVENT,
    IPC_TOPIC_SYSTEM_WATCHDOG_HEARTBEAT,
    
    // Safety topics
    IPC_TOPIC_SAFETY_STATUS_OVERALL,
    
    // Vehicle topics
    IPC_TOPIC_VEHICLE_CONTROL_COMMAND,
    
    // Vision topics
    IPC_TOPIC_VISION_CAMERA_FRAME,
    
    // AI topics
    IPC_TOPIC_AI_INFERENCE_RESULT,
    
    // Communication topics
    IPC_TOPIC_COMMUNICATION_CAN_MSG,
    
    NULL  // Array terminator
};

/**
 * @brief Enhanced topic validation with comprehensive safety checks
 * 
 * Validates topic strings against system naming conventions with
 * comprehensive safety checks including format, length, and namespace validation.
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
    
    // Safety: Check for leading or trailing slashes
    if (topic[0] == '/' || topic[topic_len - 1] == '/') {
        return IPC_TOPIC_INVALID_FORMAT;
    }
    
    // Safety: Check for empty segments (consecutive slashes)
    for (size_t i = 0; i < topic_len - 1; i++) {
        if (topic[i] == '/' && topic[i + 1] == '/') {
            return IPC_TOPIC_INVALID_FORMAT;
        }
    }
    
    // Safety: Reserved namespace check - only for non-system applications
    // System topics (starting with "system/") are always allowed
    if (strncmp(topic, "system/", 7) != 0) {
        // Check other reserved namespaces for non-system topics
        // Use the global RESERVED_NAMESPACES array (skip "system/" since it's already handled)
        for (int i = 0; RESERVED_NAMESPACES[i] != NULL; i++) {
            // Skip "system/" since we already allow it above
            if (strcmp(RESERVED_NAMESPACES[i], "system/") == 0) {
                continue;
            }
            
            if (strncmp(topic, RESERVED_NAMESPACES[i], 
                       strlen(RESERVED_NAMESPACES[i])) == 0) {
                return IPC_TOPIC_RESERVED_NAMESPACE;
            }
        }
    }
    
    return IPC_TOPIC_VALID;
}

/**
 * @brief Enhanced topic construction with parameter validation
 * 
 * Constructs valid topic strings from category and type parameters
 * with comprehensive parameter validation and safety checks.
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
                    
                case IPC_TOPIC_SYSTEM_MONITOR:
                    snprintf(topic_buffer, sizeof(topic_buffer), 
                            "%s/%s/status", 
                            TOPIC_BASE_STRINGS[category],
                            TOPIC_SYSTEM_STRINGS[system_type]);
                    break;
                    
                case IPC_TOPIC_SYSTEM_DIAGNOSTIC:
                    snprintf(topic_buffer, sizeof(topic_buffer), 
                            "%s/%s/status", 
                            TOPIC_BASE_STRINGS[category],
                            TOPIC_SYSTEM_STRINGS[system_type]);
                    break;
                    
                case IPC_TOPIC_SYSTEM_POWER:
                    snprintf(topic_buffer, sizeof(topic_buffer), 
                            "%s/%s/status", 
                            TOPIC_BASE_STRINGS[category],
                            TOPIC_SYSTEM_STRINGS[system_type]);
                    break;
                    
                default:
                    return NULL;
            }
            break;
            
        case IPC_TOPIC_CATEGORY_SAFETY:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/%s/overall", 
                    TOPIC_BASE_STRINGS[category],
                    TOPIC_SAFETY_STRINGS[IPC_TOPIC_SAFETY_STATUS]);
            break;
            
        case IPC_TOPIC_CATEGORY_VEHICLE:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/%s/command", 
                    TOPIC_BASE_STRINGS[category],
                    TOPIC_VEHICLE_STRINGS[IPC_TOPIC_VEHICLE_CONTROL]);
            break;
            
        case IPC_TOPIC_CATEGORY_VISION:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/%s/frame", 
                    TOPIC_BASE_STRINGS[category],
                    TOPIC_VISION_STRINGS[IPC_TOPIC_VISION_CAMERA]);
            break;
            
        case IPC_TOPIC_CATEGORY_AI:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/%s/result", 
                    TOPIC_BASE_STRINGS[category],
                    TOPIC_AI_STRINGS[IPC_TOPIC_AI_INFERENCE]);
            break;
            
        case IPC_TOPIC_CATEGORY_COMMUNICATION:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/%s/message", 
                    TOPIC_BASE_STRINGS[category],
                    TOPIC_COMMUNICATION_STRINGS[IPC_TOPIC_COMMUNICATION_CAN]);
            break;
            
        case IPC_TOPIC_CATEGORY_DIAGNOSTICS:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/status/overall", TOPIC_BASE_STRINGS[category]);
            break;
            
        case IPC_TOPIC_CATEGORY_POWER:
            snprintf(topic_buffer, sizeof(topic_buffer), 
                    "%s/status/overall", TOPIC_BASE_STRINGS[category]);
            break;
            
        default:
            return NULL;
    }
    
    // Final validation
    if (ipc_validate_topic(topic_buffer) != IPC_TOPIC_VALID) {
        return NULL;
    }
    
    return topic_buffer;
}

/**
 * @brief Get all predefined system topics
 * 
 * Returns a NULL-terminated array containing all predefined system topics
 * that should be registered during IPC initialization.
 */
const char** ipc_get_predefined_topics(void) {
    return (const char**)PREDEFINED_TOPICS;
}

/**
 * @brief Check if a topic is predefined
 * 
 * Determines if a topic is part of the predefined system topics
 * by comparing against the predefined topics array.
 */
bool ipc_is_predefined_topic(const char* topic) {
    if (topic == NULL) {
        return false;
    }
    
    for (int i = 0; PREDEFINED_TOPICS[i] != NULL; i++) {
        if (strcmp(topic, PREDEFINED_TOPICS[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Get topic category from string
 * 
 * Extracts the category from a topic string by parsing the first segment.
 */
ipc_topic_category_t ipc_get_topic_category(const char* topic) {
    if (topic == NULL) {
        return IPC_TOPIC_CATEGORY_COUNT;
    }
    
    // Extract first segment (before first slash)
    char category_str[32] = {0};
    const char* slash_pos = strchr(topic, '/');
    if (slash_pos == NULL) {
        return IPC_TOPIC_CATEGORY_COUNT;
    }
    
    size_t category_len = slash_pos - topic;
    if (category_len >= sizeof(category_str)) {
        return IPC_TOPIC_CATEGORY_COUNT;
    }
    
    strncpy(category_str, topic, category_len);
    category_str[category_len] = '\0';
    
    // Compare against known categories
    for (int i = 0; i < IPC_TOPIC_CATEGORY_COUNT; i++) {
        if (TOPIC_BASE_STRINGS[i] != NULL && 
            strcmp(category_str, TOPIC_BASE_STRINGS[i]) == 0) {
            return (ipc_topic_category_t)i;
        }
    }
    
    return IPC_TOPIC_CATEGORY_COUNT;
}

// Placeholder for future implementation
bool ipc_parse_topic(const char* topic, ipc_topic_category_t* category,
                    ipc_topic_system_t* system_type, 
                    ipc_topic_logging_t* logging_type) {
    // TODO: Implement topic parsing functionality
    (void)topic;
    (void)category;
    (void)system_type;
    (void)logging_type;
    return false;
}