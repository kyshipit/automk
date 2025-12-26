/**
 * @file ipc_priority.h
 * @brief AutoMLOS IPC Priority Mechanism
 * 
 * Provides priority-based QoS support for IPC communication,
 * enabling differentiated message handling based on urgency.
 */

#ifndef IPC_PRIORITY_H
#define IPC_PRIORITY_H

#include <stdint.h>

// IPC Priority levels (0=highest, 3=lowest)
typedef enum {
    IPC_PRIORITY_EMERGENCY = 0,  // Highest priority: emergency messages
    IPC_PRIORITY_ERROR     = 1,  // High priority: error messages
    IPC_PRIORITY_WARNING   = 2,  // Normal priority: warning messages
    IPC_PRIORITY_INFO      = 3,  // Low priority: informational messages
    IPC_PRIORITY_COUNT     = 4   // Total number of priority levels
} ipc_priority_t;

// System utilization statistics structure
typedef struct {
    uint8_t utilization[IPC_PRIORITY_COUNT];  // Utilization per priority level
    uint16_t total_utilization;               // Total system utilization
    uint8_t average_utilization;              // Average utilization
} ipc_utilization_stats_t;

// Priority configuration structure
typedef struct {
    uint32_t timeout_ms;         // Timeout for each priority level
    uint8_t retry_count;         // Retry attempts for each level
    uint8_t max_queue_depth;     // Maximum queue depth per priority
    uint8_t current_utilization; // Current utilization percentage (0-100)
} ipc_priority_config_t;

/**
 * @brief Get default priority configuration
 * 
 * @param priority Priority level to get configuration for
 * @param config Output parameter for configuration
 * @return 0 on success, -1 on error
 */
int ipc_get_priority_config(ipc_priority_t priority, ipc_priority_config_t* config);

/**
 * @brief Set custom priority configuration
 * 
 * @param priority Priority level to configure
 * @param config Configuration to apply
 * @return 0 on success, -1 on error
 */
int ipc_set_priority_config(ipc_priority_t priority, const ipc_priority_config_t* config);

/**
 * @brief Validate priority level
 * 
 * @param priority Priority level to validate
 * @return 1 if valid, 0 if invalid
 */
int ipc_validate_priority(ipc_priority_t priority);

/**
 * @brief Get priority name as string
 * 
 * @param priority Priority level
 * @return Priority name string
 */
const char* ipc_get_priority_name(ipc_priority_t priority);

/**
 * @brief Adjust priority configuration based on system load
 * 
 * @param priority Priority level to adjust
 * @param current_depth Current queue depth
 * @return int Adjustment result (0: no change, 1: adjusted, -1: error)
 */
int ipc_adjust_priority_dynamic(ipc_priority_t priority, uint16_t current_depth);

/**
 * @brief Get dynamic priority configuration based on current system state
 * 
 * @param priority Priority level to get configuration for
 * @param config Output parameter for configuration
 * @param current_depth Current queue depth for dynamic adjustment
 * @return 0 on success, -1 on error
 */
int ipc_get_dynamic_priority_config(ipc_priority_t priority, 
                                   ipc_priority_config_t* config,
                                   uint16_t current_depth);

/**
 * @brief Reset priority configuration to defaults
 * 
 * @param priority Priority level to reset
 * @return 0 on success, -1 on error
 */
int ipc_reset_priority_config(ipc_priority_t priority);

/**
 * @brief Get system-wide IPC utilization statistics
 * 
 * @param stats Output parameter for statistics
 * @return 0 on success, -1 on error
 */
int ipc_get_system_utilization_stats(ipc_utilization_stats_t* stats);

#endif // IPC_PRIORITY_H