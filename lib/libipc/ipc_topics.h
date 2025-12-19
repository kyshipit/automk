/**
 * @file ipc_topics.h
 * @brief AutoMLOS IPC topic definitions (centralized management)
 * 
 * Centralized topic management following system-wide naming conventions
 * defined in docs/msg_types.md.
 */

#ifndef IPC_TOPICS_H
#define IPC_TOPICS_H

#include <stdint.h>
#include <stdbool.h>

// Safety: Topic validation result codes
typedef enum {
    IPC_TOPIC_VALID,
    IPC_TOPIC_INVALID_FORMAT,
    IPC_TOPIC_INVALID_CATEGORY,
    IPC_TOPIC_INVALID_SUBCATEGORY,
    IPC_TOPIC_RESERVED_NAMESPACE,
    IPC_TOPIC_LENGTH_EXCEEDED
} ipc_topic_validation_t;

// Topic category enumeration for type safety
typedef enum {
    IPC_TOPIC_CATEGORY_VISION = 0,
    IPC_TOPIC_CATEGORY_VEHICLE,
    IPC_TOPIC_CATEGORY_AI,
    IPC_TOPIC_CATEGORY_SYSTEM,
    IPC_TOPIC_CATEGORY_SAFETY,
    IPC_TOPIC_CATEGORY_COMMUNICATION,
    IPC_TOPIC_CATEGORY_COUNT
} ipc_topic_category_t;

// System domain subcategories
typedef enum {
    IPC_TOPIC_SYSTEM_MONITOR = 0,
    IPC_TOPIC_SYSTEM_DIAGNOSTIC,
    IPC_TOPIC_SYSTEM_LOGGING,
    IPC_TOPIC_SYSTEM_POWER,
    IPC_TOPIC_SYSTEM_COUNT
} ipc_topic_system_t;

// Logging subcategories (aligned with msg_types.md)
typedef enum {
    IPC_TOPIC_LOGGING_EMERGENCY = 0,
    IPC_TOPIC_LOGGING_ERROR,
    IPC_TOPIC_LOGGING_INFO,
    IPC_TOPIC_LOGGING_DEBUG,
    IPC_TOPIC_LOGGING_COUNT
} ipc_topic_logging_t;

/**
 * @brief Validate topic string against naming conventions
 * 
 * Comprehensive topic validation with safety checks.
 * 
 * @param topic Topic string to validate
 * @return ipc_topic_validation_t Validation result code
 */
ipc_topic_validation_t ipc_validate_topic(const char* topic);

/**
 * @brief Build IPC topic string from category and type
 * 
 * Type-safe topic construction following system naming conventions.
 * 
 * @param category Topic category
 * @param system_type System subcategory (for SYSTEM category)
 * @param logging_type Logging subcategory (for LOGGING subcategory)
 * @return const char* Topic string (NULL if invalid parameters)
 */
const char* ipc_build_topic(ipc_topic_category_t category, 
                           ipc_topic_system_t system_type,
                           ipc_topic_logging_t logging_type);

/**
 * @brief Parse topic string into category and type components
 * 
 * Reverse operation of ipc_build_topic for topic analysis.
 * 
 * @param topic Topic string to parse
 * @param category Output category (if successful)
 * @param system_type Output system type (if applicable)
 * @param logging_type Output logging type (if applicable)
 * @return bool True if parsing successful
 */
bool ipc_parse_topic(const char* topic, ipc_topic_category_t* category,
                    ipc_topic_system_t* system_type, 
                    ipc_topic_logging_t* logging_type);

// Predefined topic constants for common use cases (compile-time constants)
#define IPC_TOPIC_SYSTEM_LOGGING_EMERGENCY "system/logging/emergency"
#define IPC_TOPIC_SYSTEM_LOGGING_ERROR     "system/logging/error"
#define IPC_TOPIC_SYSTEM_LOGGING_INFO      "system/logging/info"
#define IPC_TOPIC_SYSTEM_LOGGING_DEBUG     "system/logging/debug"

// Safety: Static assertions for topic length limits
_Static_assert(sizeof(IPC_TOPIC_SYSTEM_LOGGING_EMERGENCY) <= 64, 
               "Topic length exceeds safety limit");

#endif // IPC_TOPICS_H