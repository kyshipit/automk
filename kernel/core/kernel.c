/**
 * @file kernel.c
 * @brief AutoMLOS Kernel Main Entry Point with Smart Dependency Management
 * 
 * Kernel initialization and main loop for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance and AUTOSAR-compliant startup sequence.
 * 
 * @author AutoMLOS Team
 * @version 3.0
 * @date 2024
 * @copyright AutoMLOS Automotive Systems
 */

#include "kernel.h"
#include "ipc.h"
#include "syscall.h"
#include "scheduler.h"
#include "memory.h"
#include "../../lib/safety/system_errors.h"
#include "interrupt.h"
#include "../../platform/bsp/hw_timer.h"
#include "../../platform/bsp/hw_watchdog.h"
#include "../../services/system_monitor/system_monitor.h"
#include "../../lib/logging/log_client.h"
#include "../../lib/time/time.h"
#include "../../lib/watchdog/watchdog.h"

// Startup phase definitions (AUTOSAR compliant)
typedef enum {
    STARTUP_PHASE_PURE_HARDWARE = 0,  // Pure hardware initialization (no logging)
    STARTUP_PHASE_HARDWARE,           // Hardware initialization with logging
    STARTUP_PHASE_KERNEL_CORE,        // Kernel core services
    STARTUP_PHASE_SAFETY_MONITOR,     // Safety monitoring
    STARTUP_PHASE_SYSTEM_SERVICES,    // System services
    STARTUP_PHASE_APPLICATION,        // Application layer
    STARTUP_PHASE_COMPLETE            // System ready
} startup_phase_t;

static startup_phase_t g_current_phase = STARTUP_PHASE_PURE_HARDWARE;

/**
 * @brief Log startup phase transition for diagnostics
 * 
 * @param phase New startup phase
 * @return system_error_t Logging status
 */
static system_error_t kernel_log_phase_transition(startup_phase_t phase) {
    return log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_KERNEL, 
                               LOG_MSG_STARTUP_PHASE, phase, g_current_phase);
}

/**
 * @brief Initialize pure hardware foundation layer (Phase 0)
 * 
 * @return system_error_t Initialization status
 * 
 * @phase STARTUP_PHASE_PURE_HARDWARE
 * @note This phase performs hardware initialization without logging capability
 */
static system_error_t kernel_init_pure_hardware(void) {
    system_error_t error;
    
    // Initialize kernel time management first (hardware timer)
    error = kernel_time_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Initialize interrupt controller
    error = interrupt_controller_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    g_current_phase = STARTUP_PHASE_PURE_HARDWARE;
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_KERNEL);
}

/**
 * @brief Initialize early logging system (Phase 1)
 * 
 * @return system_error_t Initialization status
 * 
 * @phase STARTUP_PHASE_HARDWARE
 * @note This phase initializes the early logging buffer for startup diagnostics
 */
static system_error_t kernel_init_early_logging(void) {
    system_error_t error;
    
    // Initialize early logging client with early buffer support
    error = log_client_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Record hardware phase completion with logging capability
    g_current_phase = STARTUP_PHASE_HARDWARE;
    return kernel_log_phase_transition(g_current_phase);
}

/**
 * @brief Initialize kernel core services (Phase 2)
 * 
 * @return system_error_t Initialization status
 * 
 * @phase STARTUP_PHASE_KERNEL_CORE
 */
static system_error_t kernel_init_core_services(void) {
    system_error_t error;
    
    // Initialize memory subsystem
    error = memory_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Initialize IPC subsystem
    error = ipc_kernel_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    g_current_phase = STARTUP_PHASE_KERNEL_CORE;
    return kernel_log_phase_transition(g_current_phase);
}

/**
 * @brief Initialize safety monitoring layer (Phase 3)
 * 
 * @return system_error_t Initialization status
 * 
 * @phase STARTUP_PHASE_SAFETY_MONITOR
 */
static system_error_t kernel_init_safety_monitor(void) {
    system_error_t error;
    
    // Initialize system monitor service
    error = system_monitor_service_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Start system monitor service
    error = system_monitor_service_start();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    g_current_phase = STARTUP_PHASE_SAFETY_MONITOR;
    return kernel_log_phase_transition(g_current_phase);
}

/**
 * @brief Initialize system services layer (Phase 4)
 * 
 * @return system_error_t Initialization status
 * 
 * @phase STARTUP_PHASE_SYSTEM_SERVICES
 */
static system_error_t kernel_init_system_services(void) {
    system_error_t error;
    
    // Initialize system call interface
    error = syscall_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Initialize scheduler
    error = scheduler_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Initialize kernel watchdog management
    error = kernel_watchdog_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Register kernel components for watchdog monitoring
    kernel_watchdog_register(KERNEL_WATCHDOG_LEVEL_SCHEDULER, 1000, "scheduler");
    kernel_watchdog_register(KERNEL_WATCHDOG_LEVEL_MEMORY, 2000, "memory_manager");
    kernel_watchdog_register(KERNEL_WATCHDOG_LEVEL_IPC, 1500, "ipc_subsystem");
    
    g_current_phase = STARTUP_PHASE_SYSTEM_SERVICES;
    return kernel_log_phase_transition(g_current_phase);
}

/**
 * @brief Kernel initialization with redesigned phased startup
 * 
 * @return system_error_t Initialization status
 * 
 * @implements AUTOSAR startup sequence and ISO 26262 safety requirements
 * @note This implementation fixes the startup sequence to ensure proper dependency management
 */
system_error_t kernel_init(void) {
    system_error_t error;
    
    // Phase 0: Pure hardware initialization (no logging capability)
    error = kernel_init_pure_hardware();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Phase 1: Initialize early logging system
    error = kernel_init_early_logging();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Phase 2: Initialize kernel core services
    error = kernel_init_core_services();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Phase 3: Initialize safety monitoring
    error = kernel_init_safety_monitor();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // Phase 4: Initialize system services
    error = kernel_init_system_services();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    // System startup complete
    g_current_phase = STARTUP_PHASE_COMPLETE;
    kernel_log_phase_transition(g_current_phase);
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_KERNEL);
}

/**
 * @brief Kernel main loop
 */
void kernel_main(void) {
    uint64_t last_watchdog_pet = kernel_time_get_uptime_us();
    
    while (1) {
        uint64_t current_time = kernel_time_get_uptime_us();
        
        // Pet kernel component watchdogs periodically using kernel time
        if (current_time - last_watchdog_pet >= 500000) { // Every 500ms
            kernel_watchdog_pet(KERNEL_WATCHDOG_LEVEL_SCHEDULER);
            kernel_log_watchdog_pet(KERNEL_WATCHDOG_LEVEL_SCHEDULER);
            
            kernel_watchdog_pet(KERNEL_WATCHDOG_LEVEL_MEMORY);
            kernel_log_watchdog_pet(KERNEL_WATCHDOG_LEVEL_MEMORY);
            
            kernel_watchdog_pet(KERNEL_WATCHDOG_LEVEL_IPC);
            kernel_log_watchdog_pet(KERNEL_WATCHDOG_LEVEL_IPC);
            
            last_watchdog_pet = current_time;
        }
        
        // Start scheduler (this function should not return)
        scheduler_start();
    }
}

/**
 * @brief Kernel shutdown function
 */
void kernel_shutdown(void) {
    // Cleanup resources (in a real system, this would handle graceful shutdown)
    // No return value needed for shutdown function
}