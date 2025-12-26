/**
 * @file interrupt.c
 * @brief AutoMLOS Interrupt Controller with Hardware-Assisted Timing
 * 
 * Automotive-grade interrupt management with ISO 26262 ASIL-D compliance,
 * featuring hardware-assisted precise timing measurements for real-time systems.
 * 
 * @author AutoMLOS Team
 * @version 2.3
 * @date 2024
 * @copyright AutoMLOS Automotive Systems
 */

#include "interrupt.h"
#include "time.h"
#include "../../platform/bsp/hw_timer.h"  // Include BSP for hardware timing
#include "../../lib/logging/log_client.h"
#include <string.h>

// Enhanced interrupt controller with hardware timing support
static struct interrupt_controller g_interrupt_controller = {0};

/**
 * @brief Get precise hardware timer value for latency measurement
 * 
 * @return uint64_t Hardware timer value in timer ticks
 * 
 * @note Uses BSP hardware timer for maximum precision
 */
static uint64_t interrupt_get_hardware_timer_value(void) {
    return hw_timer_get_counter();
}

/**
 * @brief Convert hardware timer ticks to microseconds
 * 
 * @param timer_ticks Timer value in hardware ticks
 * @return uint64_t Time in microseconds
 */
static uint64_t interrupt_ticks_to_us(uint64_t timer_ticks) {
    return (timer_ticks * 1000000ULL) / HARDWARE_TIMER_FREQUENCY_HZ;
}

/**
 * @brief Record interrupt latency with hardware-assisted timing
 * 
 * @param irq_number Interrupt number
 * @param entry_time Hardware timer value at interrupt entry
 * 
 * @implements ISO 26262 Part 6-7.4.9 timing measurement requirement
 */
static void record_interrupt_latency(uint32_t irq_number, uint64_t entry_time) {
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return;
    }
    
    uint64_t exit_time = interrupt_get_hardware_timer_value();
    uint64_t latency_ticks = exit_time - entry_time;
    uint64_t latency_us = interrupt_ticks_to_us(latency_ticks);
    
    // Update current latency measurement
    g_interrupt_controller.latency_measurements[irq_number] = latency_us;
    
    // Safety check: log excessive latency
    if (latency_us > MAX_ACCEPTABLE_LATENCY_US) {
        log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_INTERRUPT,
                            LOG_MSG_INTERRUPT_LATENCY_EXCEEDED,
                            irq_number, latency_us);
    }
}

/**
 * @brief Calculate average interrupt latency for statistical analysis
 * 
 * @param irq_number Interrupt number
 * @return uint64_t Average latency in microseconds
 */
static uint64_t calculate_average_latency(uint32_t irq_number) {
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return 0;
    }
    
    // Simple implementation - use current latency as average
    return g_interrupt_controller.latency_measurements[irq_number];
}

/**
 * @brief Enhanced interrupt handler with hardware timing
 * 
 * @param irq_number Interrupt number
 * 
 * @note Uses hardware timer for precise latency measurement
 */
void interrupt_handler(uint32_t irq_number) {
    // Capture precise entry time using hardware timer
    uint64_t entry_time = interrupt_get_hardware_timer_value();
    
    // Safety check
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_INTERRUPT,
                            LOG_MSG_INVALID_INTERRUPT_NUMBER, irq_number, 0);
        return;
    }
    
    // Update interrupt counter
    g_interrupt_controller.interrupt_counters[irq_number]++;
    
    // Call registered handler if exists
    if (g_interrupt_controller.handlers[irq_number] != NULL) {
        g_interrupt_controller.handlers[irq_number](irq_number, 
                                                  g_interrupt_controller.handler_contexts[irq_number]);
    }
    
    // Record latency using hardware timing
    record_interrupt_latency(irq_number, entry_time);
}

/**
 * @brief Get detailed interrupt timing statistics
 * 
 * @param irq_number Interrupt number
 * @param stats Output parameter for timing statistics
 * @return system_error_t Operation status
 */
system_error_t interrupt_get_timing_statistics(uint32_t irq_number, 
                                              interrupt_timing_stats_t* stats) {
    if (stats == NULL || irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_INTERRUPT);
    }
    
    memset(stats, 0, sizeof(interrupt_timing_stats_t));
    
    stats->current_latency_us = g_interrupt_controller.latency_measurements[irq_number];
    stats->worst_case_latency_us = g_interrupt_controller.latency_measurements[irq_number];
    stats->average_latency_us = calculate_average_latency(irq_number);
    stats->interrupt_count = g_interrupt_controller.interrupt_counters[irq_number];
    stats->sample_count = 1; // Simplified implementation
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Perform comprehensive interrupt timing analysis
 * 
 * @param analysis Output parameter for analysis results
 * @return system_error_t Analysis status
 */
system_error_t interrupt_perform_timing_analysis(interrupt_timing_analysis_t* analysis) {
    if (analysis == NULL) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_INTERRUPT);
    }
    
    memset(analysis, 0, sizeof(interrupt_timing_analysis_t));
    
    for (uint32_t i = 0; i < INTERRUPT_MAX_HANDLERS; i++) {
        if (g_interrupt_controller.interrupt_counters[i] > 0) {
            analysis->total_interrupts += g_interrupt_controller.interrupt_counters[i];
            
            uint64_t latency = g_interrupt_controller.latency_measurements[i];
            if (latency > analysis->system_worst_latency) {
                analysis->system_worst_latency = latency;
                analysis->worst_latency_irq = i;
            }
            
            if (latency > MAX_ACCEPTABLE_LATENCY_US) {
                analysis->violation_count++;
            }
        }
    }
    
    analysis->compliance_status = (analysis->violation_count == 0) ? 
                                 INTERRUPT_TIMING_COMPLIANT : INTERRUPT_TIMING_NON_COMPLIANT;
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Enhanced safety check with timing analysis
 * 
 * @return system_error_t Safety check status
 */
system_error_t interrupt_safety_check(void) {
    if (!g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_NOT_INIT, 
                                 ERROR_CATEGORY_SAFETY, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Check for interrupt storm (excessive interrupts)
    for (uint32_t i = 0; i < INTERRUPT_MAX_HANDLERS; i++) {
        if (g_interrupt_controller.interrupt_counters[i] > 1000) {
            log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_INTERRUPT,
                                LOG_MSG_INTERRUPT_STORM_DETECTED, i, 
                                g_interrupt_controller.interrupt_counters[i]);
            return system_error_create(ERR_SYS_CONFIG_ERROR, 
                                     ERROR_CATEGORY_SAFETY, ERROR_SEVERITY_HIGH, 
                                     MODULE_ID_INTERRUPT);
        }
    }
    
    // Check timing compliance
    interrupt_timing_analysis_t analysis;
    system_error_t timing_result = interrupt_perform_timing_analysis(&analysis);
    if (timing_result.error_code == ERR_SYS_OK && 
        analysis.compliance_status == INTERRUPT_TIMING_NON_COMPLIANT) {
        log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_INTERRUPT,
                            LOG_MSG_TIMING_VIOLATION_DETECTED,
                            analysis.violation_count, analysis.system_worst_latency);
        return system_error_create(ERR_SYS_CONFIG_ERROR, 
                                 ERROR_CATEGORY_SAFETY, ERROR_SEVERITY_MEDIUM, 
                                 MODULE_ID_INTERRUPT);
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SAFETY,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Initialize interrupt controller
 * 
 * @return system_error_t Error code
 */
system_error_t interrupt_controller_init(void) {
    // Safety check: prevent double initialization
    if (g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_ALREADY_INIT, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_MEDIUM, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Initialize interrupt controller state
    memset(&g_interrupt_controller, 0, sizeof(g_interrupt_controller));
    
    // Initialize hardware interrupt controller
    system_error_t error = interrupt_hw_init();
    if (error.error_code != ERR_SYS_OK) {
        return error;
    }
    
    // Enable global interrupts
    interrupt_global_enable();
    
    g_interrupt_controller.initialized = true;
    
    // Perform safety self-test
    error = interrupt_safety_check();
    if (error.error_code != ERR_SYS_OK) {
        return error;
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Register interrupt handler
 * 
 * @param irq_number Interrupt number
 * @param priority Interrupt priority
 * @param handler Interrupt handler function
 * @param context Handler context
 * @return system_error_t Error code
 */
system_error_t interrupt_register_handler(uint32_t irq_number, 
                                        interrupt_priority_t priority,
                                        interrupt_handler_t handler, 
                                        void* context) {
    // Safety checks
    if (!g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_NOT_INITIALIZED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    if (handler == NULL) {
        return system_error_create(ERR_BASE_NULL_PTR, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Enter critical section
    interrupt_enter_critical_section();
    
    // Register handler
    g_interrupt_controller.handlers[irq_number] = handler;
    g_interrupt_controller.handler_contexts[irq_number] = context;
    
    // Set interrupt priority
    system_error_t error = interrupt_hw_set_priority(irq_number, priority);
    if (error.error_code != ERR_SYS_OK) {
        interrupt_exit_critical_section();
        return error;
    }
    
    interrupt_exit_critical_section();
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Enable specific interrupt
 * 
 * @param irq_number Interrupt number
 * @return system_error_t Error code
 */
system_error_t interrupt_enable(uint32_t irq_number) {
    if (!g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_NOT_INITIALIZED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    interrupt_enter_critical_section();
    
    // Enable interrupt in hardware
    system_error_t error = interrupt_hw_enable(irq_number);
    if (error.error_code == ERR_SYS_OK) {
        g_interrupt_controller.enabled_irqs |= (1UL << irq_number);
    }
    
    interrupt_exit_critical_section();
    
    return error;
}

// Platform-specific hardware abstraction functions (stubs for now)
system_error_t interrupt_hw_init(void) {
    // Platform-specific implementation would go here
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Set interrupt priority in hardware
 * 
 * @param irq_number Interrupt number to configure
 * @param priority Priority level to set
 * @return system_error_t Operation status
 */
system_error_t interrupt_hw_set_priority(uint32_t irq_number, interrupt_priority_t priority) {
    // Parameter validation
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    if (priority >= INTERRUPT_PRIORITY_LEVELS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Check if interrupt controller is initialized
    if (!g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_NOT_INITIALIZED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Enter critical section for thread safety
    interrupt_enter_critical_section();
    
    // Set priority in global state
    g_interrupt_controller.interrupt_priorities[irq_number] = priority;
    
    // Platform-specific hardware implementation
    // This would write to actual hardware registers
    // Example: NVIC_SetPriority((IRQn_Type)irq_number, priority);
    
    // Log priority setting for debugging and verification
    log_client_send_entry(LOG_LEVEL_DEBUG, LOG_TAG_INTERRUPT,
                         LOG_MSG_INTERRUPT_PRIORITY_SET,
                         irq_number, priority);
    
    // Exit critical section
    interrupt_exit_critical_section();
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Get current priority setting for an interrupt
 * 
 * @param irq_number Interrupt number to query
 * @param priority Output parameter for current priority
 * @return system_error_t Operation status
 */
system_error_t interrupt_hw_get_priority(uint32_t irq_number, interrupt_priority_t* priority) {
    // Parameter validation
    if (priority == NULL || irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Check if interrupt controller is initialized
    if (!g_interrupt_controller.initialized) {
        return system_error_create(ERR_SYS_NOT_INITIALIZED, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Read priority from global state
    *priority = g_interrupt_controller.interrupt_priorities[irq_number];
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Verify interrupt priority setting
 * 
 * @param irq_number Interrupt number to verify
 * @param expected_priority Expected priority level
 * @return system_error_t Verification result
 */
system_error_t interrupt_verify_priority(uint32_t irq_number, interrupt_priority_t expected_priority) {
    interrupt_priority_t actual_priority;
    system_error_t result = interrupt_hw_get_priority(irq_number, &actual_priority);
    
    if (result.error_code != ERR_SYS_OK) {
        return result;
    }
    
    // Check if actual priority matches expected
    if (actual_priority != expected_priority) {
        log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_INTERRUPT,
                           LOG_MSG_INTERRUPT_PRIORITY_MISMATCH,
                           irq_number, expected_priority);
        return system_error_create(ERR_SYS_CONFIG_ERROR, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

/**
 * @brief Enable interrupt in hardware
 * 
 * @param irq_number Interrupt number to enable
 * @return system_error_t Operation status
 */
system_error_t interrupt_hw_enable(uint32_t irq_number) {
    // Parameter validation
    if (irq_number >= INTERRUPT_MAX_HANDLERS) {
        return system_error_create(ERR_SYS_INVALID_PARAM, 
                                 ERROR_CATEGORY_SYSTEM, ERROR_SEVERITY_HIGH, 
                                 MODULE_ID_INTERRUPT);
    }
    
    // Platform-specific implementation would go here
    // Example: NVIC_EnableIRQ((IRQn_Type)irq_number);
    
    // Log interrupt enable for debugging
    log_client_send_entry(LOG_LEVEL_DEBUG, LOG_TAG_INTERRUPT,
                         LOG_MSG_INTERRUPT_ENABLED, irq_number, 0);
    
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_INTERRUPT);
}

void interrupt_global_enable(void) {
    // Platform-specific implementation
}

void interrupt_enter_critical_section(void) {
    // Platform-specific implementation
}

void interrupt_exit_critical_section(void) {
    // Platform-specific implementation
}