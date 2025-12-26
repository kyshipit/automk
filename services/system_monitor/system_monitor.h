/**
 * @file system_monitor.h
 * @brief AutoMLOS System Monitor Service with Redundant Monitoring
 * 
 * Enhanced system health monitoring with redundant watchdog management
 * for automotive embedded systems with ISO 26262 ASIL-D safety compliance.
 */

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../lib/safety/system_errors.h"

// Redundant monitoring configuration
#define SYSTEM_MONITOR_REDUNDANT_WATCHDOGS    2    // Number of redundant watchdogs
#define SYSTEM_MONITOR_HEARTBEAT_INTERVAL_MS  100  // Heartbeat interval
#define SYSTEM_MONITOR_WATCHDOG_TIMEOUT_MS    1000 // Watchdog timeout

// Health monitoring levels
typedef enum {
    HEALTH_LEVEL_CRITICAL = 0,   // Critical system functions
    HEALTH_LEVEL_IMPORTANT,      // Important system functions  
    HEALTH_LEVEL_NORMAL,         // Normal system functions
    HEALTH_LEVEL_BACKGROUND      // Background functions
} health_level_t;

// Redundant watchdog state
typedef struct {
    bool armed;
    uint32_t timeout_ms;
    uint64_t last_pet_time;
    uint32_t timeout_count;
    bool independent;  // Independent hardware watchdog
} redundant_watchdog_t;

// System health state with redundancy
struct system_health {
    uint32_t overall_score;           // Overall health score (0-100)
    uint32_t component_scores[4];     // Component-specific scores
    uint64_t last_health_check;       // Last health check time
    uint32_t consecutive_failures;    // Consecutive health check failures
    bool primary_monitor_healthy;     // Primary monitor health status
    bool secondary_monitor_healthy;   // Secondary monitor health status
};

// System monitor service interface
system_error_t system_monitor_service_init(void);
system_error_t system_monitor_service_start(void);
system_error_t system_monitor_service_stop(void);

// Redundant watchdog management
system_error_t system_monitor_watchdog_arm_redundant(void);
system_error_t system_monitor_watchdog_pet_redundant(void);
system_error_t system_monitor_watchdog_check_consistency(void);

// Health monitoring with redundancy
system_error_t system_monitor_update_health_redundant(void);
system_error_t system_monitor_check_redundancy(void);
bool system_monitor_is_redundant_system_healthy(void);

// Time service
system_error_t system_monitor_time_service_init(void);
uint64_t system_monitor_get_system_time_us(void);

// Safety functions
system_error_t system_monitor_enter_safe_state(void);
system_error_t system_monitor_perform_self_test(void);

#endif // SYSTEM_MONITOR_H