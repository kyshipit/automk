/**
 * @file ipc_topics.h
 * @brief AutoMLOS IPC topic definitions (centralized management)
 * 
 * Centralized topic management following system-wide naming conventions
 * defined in docs/msg_types.md.
 * 
 * This file provides a unified interface for topic validation, construction,
 * and management across the entire AutoMLOS system.
 */

#ifndef IPC_TOPICS_H
#define IPC_TOPICS_H

#include <stdint.h>
#include <stdbool.h>

// Safety: Topic validation result codes
typedef enum {
    IPC_TOPIC_VALID,                    /**< Topic format is valid */
    IPC_TOPIC_INVALID_FORMAT,           /**< Topic format is invalid */
    IPC_TOPIC_INVALID_CATEGORY,         /**< Topic category is invalid */
    IPC_TOPIC_INVALID_SUBCATEGORY,      /**< Topic subcategory is invalid */
    IPC_TOPIC_RESERVED_NAMESPACE,       /**< Topic uses reserved namespace */
    IPC_TOPIC_LENGTH_EXCEEDED           /**< Topic length exceeds limit */
} ipc_topic_validation_t;

/**
 * @brief Topic category enumeration for type safety
 * 
 * Defines all available topic categories in the AutoMLOS system.
 * Each category corresponds to a major system domain.
 */
typedef enum {
    IPC_TOPIC_CATEGORY_VISION = 0,      /**< Vision processing topics */
    IPC_TOPIC_CATEGORY_VEHICLE,         /**< Vehicle control topics */
    IPC_TOPIC_CATEGORY_AI,              /**< AI/ML processing topics */
    IPC_TOPIC_CATEGORY_SYSTEM,          /**< System management topics */
    IPC_TOPIC_CATEGORY_SAFETY,          /**< Safety-critical topics */
    IPC_TOPIC_CATEGORY_COMMUNICATION,   /**< Communication topics */
    IPC_TOPIC_CATEGORY_DIAGNOSTICS,     /**< Diagnostic topics */
    IPC_TOPIC_CATEGORY_POWER,           /**< Power management topics */
    IPC_TOPIC_CATEGORY_COUNT            /**< Total number of categories */
} ipc_topic_category_t;

/**
 * @brief System domain subcategories
 * 
 * Defines subcategories for the SYSTEM topic category.
 */
typedef enum {
    IPC_TOPIC_SYSTEM_MONITOR = 0,       /**< System monitoring topics */
    IPC_TOPIC_SYSTEM_DIAGNOSTIC,        /**< System diagnostic topics */
    IPC_TOPIC_SYSTEM_LOGGING,           /**< System logging topics */
    IPC_TOPIC_SYSTEM_POWER,             /**< System power topics */
    IPC_TOPIC_SYSTEM_COUNT              /**< Total number of system subcategories */
} ipc_topic_system_t;

/**
 * @brief Logging subcategories
 * 
 * Defines logging levels for the LOGGING system subcategory.
 * Aligned with msg_types.md specifications.
 */
typedef enum {
    IPC_TOPIC_LOGGING_EMERGENCY = 0,    /**< Emergency logging level */
    IPC_TOPIC_LOGGING_ERROR,            /**< Error logging level */
    IPC_TOPIC_LOGGING_INFO,             /**< Information logging level */
    IPC_TOPIC_LOGGING_DEBUG,            /**< Debug logging level */
    IPC_TOPIC_LOGGING_COUNT             /**< Total number of logging levels */
} ipc_topic_logging_t;

/**
 * @brief Vehicle control subcategories
 * 
 * Defines subcategories for the VEHICLE topic category.
 */
typedef enum {
    IPC_TOPIC_VEHICLE_CONTROL = 0,      /**< Vehicle control commands */
    IPC_TOPIC_VEHICLE_SENSOR,           /**< Vehicle sensor data */
    IPC_TOPIC_VEHICLE_STATUS,           /**< Vehicle status information */
    IPC_TOPIC_VEHICLE_COMMAND,          /**< Vehicle command topics */
    IPC_TOPIC_VEHICLE_COUNT             /**< Total number of vehicle subcategories */
} ipc_topic_vehicle_t;

/**
 * @brief Vision processing subcategories
 * 
 * Defines subcategories for the VISION topic category.
 */
typedef enum {
    IPC_TOPIC_VISION_CAMERA = 0,        /**< Camera frame data */
    IPC_TOPIC_VISION_OBJECT,            /**< Object detection results */
    IPC_TOPIC_VISION_LANE,              /**< Lane detection results */
    IPC_TOPIC_VISION_DETECTION,         /**< General detection results */
    IPC_TOPIC_VISION_COUNT              /**< Total number of vision subcategories */
} ipc_topic_vision_t;

/**
 * @brief AI processing subcategories
 * 
 * Defines subcategories for the AI topic category.
 */
typedef enum {
    IPC_TOPIC_AI_INFERENCE = 0,         /**< AI inference results */
    IPC_TOPIC_AI_TRAINING,              /**< AI training status */
    IPC_TOPIC_AI_MODEL,                 /**< AI model updates */
    IPC_TOPIC_AI_RESULT,                /**< AI processing results */
    IPC_TOPIC_AI_COUNT                  /**< Total number of AI subcategories */
} ipc_topic_ai_t;

/**
 * @brief Safety subcategories
 * 
 * Defines subcategories for the SAFETY topic category.
 */
typedef enum {
    IPC_TOPIC_SAFETY_STATUS = 0,        /**< Safety status information */
    IPC_TOPIC_SAFETY_EVENT,             /**< Safety event notifications */
    IPC_TOPIC_SAFETY_COMPONENT,         /**< Component safety status */
    IPC_TOPIC_SAFETY_CRITICAL,          /**< Critical safety events */
    IPC_TOPIC_SAFETY_COUNT              /**< Total number of safety subcategories */
} ipc_topic_safety_t;

/**
 * @brief Communication subcategories
 * 
 * Defines subcategories for the COMMUNICATION topic category.
 */
typedef enum {
    IPC_TOPIC_COMMUNICATION_CAN = 0,    /**< CAN bus communication */
    IPC_TOPIC_COMMUNICATION_ETHERNET,   /**< Ethernet communication */
    IPC_TOPIC_COMMUNICATION_WIRELESS,   /**< Wireless communication */
    IPC_TOPIC_COMMUNICATION_PACKET,     /**< Packet-level communication */
    IPC_TOPIC_COMMUNICATION_COUNT       /**< Total number of communication subcategories */
} ipc_topic_communication_t;

/**
 * @brief Validate topic string against naming conventions
 * 
 * Comprehensive topic validation with safety checks including:
 * - Null pointer validation
 * - Length validation (MAX_TOPIC_LENGTH)
 * - Format validation (exactly 2 slashes)
 * - Reserved namespace checking
 * - Empty segment detection
 * 
 * @param topic Topic string to validate
 * @return ipc_topic_validation_t Validation result code
 */
ipc_topic_validation_t ipc_validate_topic(const char* topic);

/**
 * @brief Build IPC topic string from category and type
 * 
 * Type-safe topic construction following system naming conventions.
 * This function provides a unified way to construct valid topics
 * without manual string manipulation.
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
 * Useful for routing decisions and topic-based filtering.
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

/**
 * @brief Get all predefined system topics
 * 
 * Returns a NULL-terminated array of all predefined system topics
 * that should be registered during IPC initialization.
 * 
 * @return const char** Array of predefined topic strings
 */
const char** ipc_get_predefined_topics(void);

/**
 * @brief Check if a topic is predefined
 * 
 * Determines if a topic is part of the predefined system topics.
 * Useful for access control and security validation.
 * 
 * @param topic Topic to check
 * @return bool True if topic is predefined
 */
bool ipc_is_predefined_topic(const char* topic);

/**
 * @brief Get topic category from string
 * 
 * Extracts the category from a topic string.
 * 
 * @param topic Topic string
 * @return ipc_topic_category_t Extracted category
 */
ipc_topic_category_t ipc_get_topic_category(const char* topic);

// Predefined topic constants for common use cases (compile-time constants)
#define IPC_TOPIC_SYSTEM_LOGGING_EMERGENCY "system/logging/emergency"  /**< System emergency logging */
#define IPC_TOPIC_SYSTEM_LOGGING_ERROR     "system/logging/error"      /**< System error logging */
#define IPC_TOPIC_SYSTEM_LOGGING_INFO      "system/logging/info"       /**< System info logging */
#define IPC_TOPIC_SYSTEM_LOGGING_DEBUG     "system/logging/debug"      /**< System debug logging */

#define IPC_TOPIC_SYSTEM_MONITOR_STATUS    "system/monitor/status"     /**< System monitor status */
#define IPC_TOPIC_SYSTEM_DIAGNOSTIC_STATUS "system/diagnostic/status"  /**< System diagnostic status */
#define IPC_TOPIC_SYSTEM_POWER_STATUS      "system/power/status"       /**< System power status */

#define IPC_TOPIC_SAFETY_STATUS_OVERALL    "safety/status/overall"     /**< Safety overall status */
#define IPC_TOPIC_VEHICLE_CONTROL_COMMAND  "vehicle/control/command"   /**< Vehicle control command */
#define IPC_TOPIC_VISION_CAMERA_FRAME      "vision/camera/frame"       /**< Vision camera frame */
#define IPC_TOPIC_AI_INFERENCE_RESULT      "ai/inference/result"       /**< AI inference result */
#define IPC_TOPIC_COMMUNICATION_CAN_MSG    "communication/can/message" /**< CAN communication message */

// Watchdog topics - unified with system naming conventions
#define IPC_TOPIC_SYSTEM_WATCHDOG_STATUS   "system/watchdog/status"    /**< System watchdog status */
#define IPC_TOPIC_SYSTEM_WATCHDOG_EVENT    "system/watchdog/event"     /**< System watchdog events */
#define IPC_TOPIC_SYSTEM_WATCHDOG_HEARTBEAT "system/watchdog/heartbeat" /**< System watchdog heartbeat */

// Safety: Static assertions for topic length limits
_Static_assert(sizeof(IPC_TOPIC_SYSTEM_LOGGING_EMERGENCY) <= 64, 
               "Topic length exceeds safety limit");
_Static_assert(sizeof(IPC_TOPIC_SYSTEM_WATCHDOG_STATUS) <= 64, 
               "Watchdog topic length exceeds safety limit");

#endif // IPC_TOPICS_H