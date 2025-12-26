/**
 * @file ipc_priority.c
 * @brief AutoMLOS IPC Priority Mechanism with Dynamic QoS Adjustment
 * 
 * Implements priority-based QoS support for IPC communication with dynamic
 * adjustment capabilities for automotive real-time systems.
 * 
 * @author AutoMLOS Team
 * @version 2.0
 * @date 2024
 * @copyright AutoMLOS Automotive Systems
 */

#include "ipc_priority.h"
#include <string.h>

// Default priority configurations (ISO 26262 compliant parameters)
static ipc_priority_config_t g_priority_configs[IPC_PRIORITY_COUNT] = {
    { .timeout_ms = 10,   .retry_count = 3, .max_queue_depth = 10, .current_utilization = 0 },  // Emergency
    { .timeout_ms = 50,   .retry_count = 2, .max_queue_depth = 20, .current_utilization = 0 },  // Error
    { .timeout_ms = 100,  .retry_count = 1, .max_queue_depth = 50, .current_utilization = 0 },  // Warning
    { .timeout_ms = 500,  .retry_count = 0, .max_queue_depth = 100, .current_utilization = 0 }  // Info
};

// Dynamic adjustment thresholds
#define UTILIZATION_HIGH_THRESHOLD     80    // 80% utilization triggers adjustment
#define UTILIZATION_CRITICAL_THRESHOLD 95    // 95% utilization triggers emergency measures

// Priority names
static const char* g_priority_names[IPC_PRIORITY_COUNT] = {
    "EMERGENCY",
    "ERROR", 
    "WARNING",
    "INFO"
};

/**
 * @brief Calculate current utilization percentage for a priority level
 * 
 * @param priority Priority level to calculate utilization for
 * @param current_depth Current queue depth
 * @return uint8_t Utilization percentage (0-100)
 */
static uint8_t calculate_utilization(ipc_priority_t priority, uint16_t current_depth) {
    if (!ipc_validate_priority(priority)) {
        return 0;
    }
    
    uint16_t max_depth = g_priority_configs[priority].max_queue_depth;
    if (max_depth == 0) {
        return 100;  // Avoid division by zero
    }
    
    return (current_depth * 100) / max_depth;
}

/**
 * @brief Adjust priority configuration based on system load
 * 
 * @param priority Priority level to adjust
 * @param current_depth Current queue depth
 * @return int Adjustment result (0: no change, 1: adjusted, -1: error)
 * 
 * @implements adaptive QoS for automotive real-time systems
 */
int ipc_adjust_priority_dynamic(ipc_priority_t priority, uint16_t current_depth) {
    if (!ipc_validate_priority(priority)) {
        return -1;
    }
    
    uint8_t utilization = calculate_utilization(priority, current_depth);
    g_priority_configs[priority].current_utilization = utilization;
    
    // Dynamic adjustment logic
    if (utilization >= UTILIZATION_CRITICAL_THRESHOLD) {
        // Emergency measures for critical utilization
        if (priority > IPC_PRIORITY_EMERGENCY) {
            // Reduce timeout for lower priorities to free resources
            g_priority_configs[priority].timeout_ms = 
                g_priority_configs[priority].timeout_ms / 2;
            if (g_priority_configs[priority].timeout_ms < 5) {
                g_priority_configs[priority].timeout_ms = 5;  // Minimum timeout
            }
            return 1;
        }
    } else if (utilization >= UTILIZATION_HIGH_THRESHOLD) {
        // Moderate adjustment for high utilization
        if (priority < IPC_PRIORITY_INFO) {
            // Slightly increase timeout to reduce retry frequency
            g_priority_configs[priority].timeout_ms += 10;
        }
        return 1;
    }
    
    return 0;  // No adjustment needed
}

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
                                   uint16_t current_depth) {
    // Safety check: null pointer validation
    if (config == NULL) {
        return -1;
    }
    
    // Safety check: priority validation
    if (!ipc_validate_priority(priority)) {
        return -1;
    }
    
    // Apply dynamic adjustment
    ipc_adjust_priority_dynamic(priority, current_depth);
    
    // Copy current configuration
    memcpy(config, &g_priority_configs[priority], sizeof(ipc_priority_config_t));
    return 0;
}

/**
 * @brief Reset priority configuration to defaults
 * 
 * @param priority Priority level to reset
 * @return 0 on success, -1 on error
 */
int ipc_reset_priority_config(ipc_priority_t priority) {
    if (!ipc_validate_priority(priority)) {
        return -1;
    }
    
    // Restore default configuration based on priority level
    switch (priority) {
        case IPC_PRIORITY_EMERGENCY:
            g_priority_configs[priority].timeout_ms = 10;
            g_priority_configs[priority].retry_count = 3;
            g_priority_configs[priority].max_queue_depth = 10;
            break;
        case IPC_PRIORITY_ERROR:
            g_priority_configs[priority].timeout_ms = 50;
            g_priority_configs[priority].retry_count = 2;
            g_priority_configs[priority].max_queue_depth = 20;
            break;
        case IPC_PRIORITY_WARNING:
            g_priority_configs[priority].timeout_ms = 100;
            g_priority_configs[priority].retry_count = 1;
            g_priority_configs[priority].max_queue_depth = 50;
            break;
        case IPC_PRIORITY_INFO:
            g_priority_configs[priority].timeout_ms = 500;
            g_priority_configs[priority].retry_count = 0;
            g_priority_configs[priority].max_queue_depth = 100;
            break;
        default:
            return -1;
    }
    
    g_priority_configs[priority].current_utilization = 0;
    return 0;
}

/**
 * @brief Get system-wide IPC utilization statistics
 * 
 * @param stats Output parameter for statistics
 * @return 0 on success, -1 on error
 */
int ipc_get_system_utilization_stats(ipc_utilization_stats_t* stats) {
    if (stats == NULL) {
        return -1;
    }
    
    memset(stats, 0, sizeof(ipc_utilization_stats_t));
    
    for (ipc_priority_t i = IPC_PRIORITY_EMERGENCY; i < IPC_PRIORITY_COUNT; i++) {
        stats->utilization[i] = g_priority_configs[i].current_utilization;
        stats->total_utilization += g_priority_configs[i].current_utilization;
    }
    
    stats->average_utilization = stats->total_utilization / IPC_PRIORITY_COUNT;
    
    return 0;
}

/**
 * @brief Get default priority configuration
 * 
 * @param priority Priority level to get configuration for
 * @param config Output parameter for configuration
 * @return 0 on success, -1 on error
 */
int ipc_get_priority_config(ipc_priority_t priority, ipc_priority_config_t* config) {
    // Safety check: null pointer validation
    if (config == NULL) {
        return -1;
    }
    
    // Safety check: priority validation
    if (!ipc_validate_priority(priority)) {
        return -1;
    }
    
    // Copy configuration
    memcpy(config, &g_priority_configs[priority], sizeof(ipc_priority_config_t));
    return 0;
}

/**
 * @brief Set custom priority configuration
 * 
 * @param priority Priority level to configure
 * @param config Configuration to apply
 * @return 0 on success, -1 on error
 */
int ipc_set_priority_config(ipc_priority_t priority, const ipc_priority_config_t* config) {
    // Safety check: null pointer validation
    if (config == NULL) {
        return -1;
    }
    
    // Safety check: priority validation
    if (!ipc_validate_priority(priority)) {
        return -1;
    }
    
    // Safety check: configuration validation
    if (config->timeout_ms == 0 || config->max_queue_depth == 0) {
        return -1;
    }
    
    // Apply configuration
    memcpy(&g_priority_configs[priority], config, sizeof(ipc_priority_config_t));
    return 0;
}

/**
 * @brief Validate priority level
 * 
 * @param priority Priority level to validate
 * @return 1 if valid, 0 if invalid
 */
int ipc_validate_priority(ipc_priority_t priority) {
    return (priority >= IPC_PRIORITY_EMERGENCY && priority < IPC_PRIORITY_COUNT);
}

/**
 * @brief Get priority name as string
 * 
 * @param priority Priority level
 * @return Priority name string
 */
const char* ipc_get_priority_name(ipc_priority_t priority) {
    if (ipc_validate_priority(priority)) {
        return g_priority_names[priority];
    }
    return "INVALID";
}