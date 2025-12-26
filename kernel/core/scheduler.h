/**
 * @file scheduler.h
 * @brief AutoMLOS Kernel Scheduler with Hard Real-Time Support
 * 
 * Enhanced scheduler with hard real-time capabilities for automotive systems
 * with ISO 26262 ASIL-D safety compliance and power management.
 */

#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "system_errors.h"
#include "time.h"  // Add kernel time support

// Process priorities with hard real-time support
typedef enum {
    PROCESS_PRIORITY_CRITICAL = 0,     // Hard real-time (safety-critical)
    PROCESS_PRIORITY_REALTIME = 1,     // Soft real-time (control systems)
    PROCESS_PRIORITY_HIGH = 2,         // High priority (user interfaces)
    PROCESS_PRIORITY_NORMAL = 3,       // Normal priority (applications)
    PROCESS_PRIORITY_BACKGROUND = 4,   // Background tasks
    PROCESS_PRIORITY_COUNT
} process_priority_t;

// Process states
typedef enum {
    PROCESS_STATE_CREATED = 0,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_SUSPENDED,
    PROCESS_STATE_TERMINATED
} process_state_t;

// Power management modes for automotive systems
typedef enum {
    POWER_MODE_ACTIVE = 0,      /**< Full power operation for active tasks */
    POWER_MODE_LOW_POWER = 1,   /**< Reduced power with limited performance */
    POWER_MODE_SLEEP = 2,       /**< Sleep state with wake-on-interrupt capability */
    POWER_MODE_DEEP_SLEEP = 3   /**< Deep sleep with minimal power consumption */
} power_mode_t;

// Real-time scheduling parameters
typedef struct {
    uint32_t period_ms;           // Task period (for periodic tasks)
    uint32_t deadline_ms;         // Deadline (relative to period)
    uint32_t worst_case_execution_time_us; // WCET
    uint32_t jitter_tolerance_us; // Maximum allowed jitter
} rt_scheduling_params_t;

// Process control block with real-time support
struct process_control_block {
    uint16_t pid;                   // Process ID
    process_state_t state;          // Current state
    process_priority_t priority;    // Priority level
    rt_scheduling_params_t rt_params; // Real-time parameters
    uint64_t time_used;             // CPU time used (microseconds)
    uint64_t last_scheduled_time;   // Last time process was scheduled
    uint64_t deadline_miss_count;   // Number of deadline misses
    uint32_t context[16];           // Process context (registers, etc.)
    void* stack_pointer;            // Stack pointer
    void* heap_pointer;             // Heap pointer
    uint64_t wakeup_time;           // Wakeup time for blocked processes
};

// Enhanced scheduler state with power management support
struct scheduler_state {
    struct process_control_block process_table[SCHEDULER_MAX_PROCESSES];
    uint16_t process_count;
    uint16_t current_pid;
    uint64_t scheduler_ticks;
    uint64_t last_dispatch_time;
    uint64_t last_tick_time;
    uint64_t last_deadline_check;
    uint64_t idle_time_accumulated;  /**< Cumulative idle time for power management */
    power_mode_t current_power_mode; /**< Current power state for energy optimization */
    bool rt_initialized;
};

// Scheduler limits
#define SCHEDULER_MAX_PROCESSES         32
#define SCHEDULER_IDLE_PID              0
#define SCHEDULER_TIME_SLICE_US         1000    // 1ms time slice
#define SCHEDULER_RT_DEADLINE_MARGIN_US 100     // 100us deadline margin

// Scheduler functions
system_error_t scheduler_init(void);
system_error_t scheduler_start(void);
void scheduler_tick(void);
system_error_t process_create(uint16_t* pid, process_priority_t priority, 
                             const rt_scheduling_params_t* rt_params);
system_error_t process_terminate(uint16_t pid);
system_error_t process_suspend(uint16_t pid);
system_error_t process_resume(uint16_t pid);
uint16_t scheduler_get_current_pid(void);
process_state_t scheduler_get_process_state(uint16_t pid);
system_error_t scheduler_get_process_info(uint16_t pid, struct process_control_block* pcb);

// Real-time scheduling analysis
bool scheduler_rt_feasibility_check(const rt_scheduling_params_t* params);
system_error_t scheduler_set_rt_params(uint16_t pid, const rt_scheduling_params_t* params);
uint64_t scheduler_get_deadline_miss_count(uint16_t pid);

// Deadline monitoring
void scheduler_check_deadlines(void);
bool scheduler_is_deadline_met(uint16_t pid);

// Power management functions
power_mode_t scheduler_get_power_mode(void);
float scheduler_get_system_utilization(void);

// Safety logging function declaration
void safety_log_error(const char* format, ...);

#endif // KERNEL_SCHEDULER_H