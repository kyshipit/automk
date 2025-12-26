/**
 * @file interrupt.h
 * @brief AutoMLOS Interrupt Controller
 * 
 * Interrupt management for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 * 
 * Features:
 * - Hardware interrupt controller abstraction
 * - Nested interrupt support
 * - Interrupt latency monitoring
 * - Safety-critical interrupt prioritization
 */

#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include "../safety/system_errors.h"
#include <stdint.h>
#include <stdbool.h>

// Hardware timing configuration
#define HARDWARE_TIMER_FREQUENCY_HZ   1000000  // 1MHz timer for microsecond resolution
#define LATENCY_MEASUREMENT_SAMPLES   16       // Number of samples for averaging
#define MAX_ACCEPTABLE_LATENCY_US     50       // ISO 26262 real-time requirement

// Interrupt configuration
#define INTERRUPT_MAX_HANDLERS       32      // Maximum interrupt handlers
#define INTERRUPT_PRIORITY_LEVELS    8       // Hardware priority levels
#define INTERRUPT_LATENCY_LIMIT_US   10      // Maximum interrupt latency (µs)

// Interrupt types (aligned with automotive requirements)
typedef enum {
    INTERRUPT_TYPE_TIMER = 0,           // System timer interrupt
    INTERRUPT_TYPE_IPC,                 // IPC message interrupt
    INTERRUPT_TYPE_SAFETY,              // Safety monitoring interrupt
    INTERRUPT_TYPE_WATCHDOG,            // Watchdog timeout interrupt
    INTERRUPT_TYPE_COMMUNICATION,       // CAN/Ethernet communication
    INTERRUPT_TYPE_SENSOR,              // Sensor data ready
    INTERRUPT_TYPE_ACTUATOR,            // Actuator control
    INTERRUPT_TYPE_DIAGNOSTIC,          // Diagnostic system
    INTERRUPT_TYPE_MAX
} interrupt_type_t;

// Interrupt priority levels (higher number = higher priority)
typedef enum {
    INTERRUPT_PRIORITY_LOWEST = 0,      // Background interrupts
    INTERRUPT_PRIORITY_LOW = 1,         // Non-critical interrupts
    INTERRUPT_PRIORITY_NORMAL = 2,      // Normal priority interrupts
    INTERRUPT_PRIORITY_HIGH = 3,        // High priority interrupts
    INTERRUPT_PRIORITY_SAFETY = 4,      // Safety-critical interrupts
    INTERRUPT_PRIORITY_EMERGENCY = 5,   // Emergency interrupts
    INTERRUPT_PRIORITY_CRITICAL = 6,    // Critical system interrupts
    INTERRUPT_PRIORITY_HIGHEST = 7      // Highest priority (NMI-like)
} interrupt_priority_t;

// Interrupt handler function prototype
typedef void (*interrupt_handler_t)(uint32_t irq_number, void* context);

// Interrupt controller state structure
struct interrupt_controller {
    bool initialized;                    // Controller initialized flag
    uint32_t enabled_irqs;              // Bitmask of enabled interrupts
    uint32_t pending_irqs;              // Bitmask of pending interrupts
    interrupt_handler_t handlers[INTERRUPT_MAX_HANDLERS]; // Interrupt handlers
    void* handler_contexts[INTERRUPT_MAX_HANDLERS];      // Handler contexts
    uint32_t interrupt_counters[INTERRUPT_MAX_HANDLERS]; // Interrupt counters
    uint64_t latency_measurements[INTERRUPT_MAX_HANDLERS]; // Latency measurements
    uint64_t worst_case_latency[INTERRUPT_MAX_HANDLERS];  // Worst-case latency
    
    // Hardware timing support
    uint64_t last_interrupt_entry_time[INTERRUPT_MAX_HANDLERS]; // Last entry time
    uint32_t latency_sample_count[INTERRUPT_MAX_HANDLERS];      // Sample count
    uint64_t latency_samples[INTERRUPT_MAX_HANDLERS][LATENCY_MEASUREMENT_SAMPLES]; // Samples
    
    // Priority management
    interrupt_priority_t interrupt_priorities[INTERRUPT_MAX_HANDLERS]; // Priority settings
};

// Interrupt timing statistics structure
typedef struct {
    uint64_t current_latency_us;     // Current interrupt latency in microseconds
    uint64_t worst_case_latency_us;  // Worst-case latency observed
    uint64_t average_latency_us;     // Average latency over time
    uint32_t interrupt_count;        // Total interrupt count
    uint32_t sample_count;           // Number of latency samples
} interrupt_timing_stats_t;

// Interrupt timing analysis structure
typedef struct {
    uint32_t total_interrupts;       // Total interrupts across all IRQs
    uint64_t system_worst_latency;   // Worst latency in the system
    uint32_t worst_latency_irq;      // IRQ with worst latency
    uint32_t violation_count;        // Number of timing violations
    uint8_t compliance_status;       // Timing compliance status
} interrupt_timing_analysis_t;

// Timing compliance status definitions
#define INTERRUPT_TIMING_COMPLIANT       0   // All interrupts within timing limits
#define INTERRUPT_TIMING_NON_COMPLIANT   1   // Some interrupts exceed timing limits

// Interrupt management functions
system_error_t interrupt_controller_init(void);
system_error_t interrupt_register_handler(uint32_t irq_number, interrupt_priority_t priority, 
                                         interrupt_handler_t handler, void* context);
system_error_t interrupt_enable(uint32_t irq_number);
system_error_t interrupt_disable(uint32_t irq_number);
system_error_t interrupt_set_priority(uint32_t irq_number, interrupt_priority_t priority);

// Interrupt timing analysis functions
system_error_t interrupt_get_timing_statistics(uint32_t irq_number, 
                                              interrupt_timing_stats_t* stats);
system_error_t interrupt_perform_timing_analysis(interrupt_timing_analysis_t* analysis);

// Interrupt control functions
void interrupt_enter_critical_section(void);
void interrupt_exit_critical_section(void);
bool interrupt_is_in_interrupt_context(void);

// Interrupt monitoring functions
system_error_t interrupt_get_latency(uint32_t irq_number, uint64_t* latency_us);
system_error_t interrupt_get_count(uint32_t irq_number, uint32_t* count);
system_error_t interrupt_clear_counters(void);

// Safety functions
system_error_t interrupt_safety_check(void);
system_error_t interrupt_fault_injection_test(uint32_t irq_number);

// Hardware abstraction functions
system_error_t interrupt_hw_init(void);
system_error_t interrupt_hw_set_priority(uint32_t irq_number, interrupt_priority_t priority);
system_error_t interrupt_hw_get_priority(uint32_t irq_number, interrupt_priority_t* priority);
system_error_t interrupt_hw_enable(uint32_t irq_number);
void interrupt_global_enable(void);

#endif // KERNEL_INTERRUPT_H