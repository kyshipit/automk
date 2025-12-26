/**
 * @file hw_timer.h
 * @brief AutoMLOS Hardware Timer Driver
 * 
 * Hardware timer abstraction for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 * 
 * Features:
 * - High-resolution timer with microsecond precision
 * - Timer interrupt configuration
 * - Clock source monitoring
 * - Safety-critical timing functions
 */

#ifndef BSP_HW_TIMER_H
#define BSP_HW_TIMER_H

#include "../../lib/safety/system_errors.h"
#include <stdint.h>
#include <stdbool.h>

// Hardware Timer Module ID
#define MODULE_ID_HW_TIMER 0x6000

// Timer configuration
#define HW_TIMER_MAX_FREQUENCY_HZ     1000000     // 1MHz maximum frequency
#define HW_TIMER_MIN_FREQUENCY_HZ     1000        // 1kHz minimum frequency
#define HW_TIMER_DEFAULT_FREQUENCY_HZ 1000        // Default 1kHz

// Timer modes
typedef enum {
    TIMER_MODE_PERIODIC = 0,           // Periodic timer
    TIMER_MODE_ONESHOT,                // One-shot timer
    TIMER_MODE_FREE_RUNNING            // Free-running counter
} hw_timer_mode_t;

// Clock sources
typedef enum {
    CLOCK_SOURCE_MAIN_OSC = 0,         // Main oscillator
    CLOCK_SOURCE_PLL,                  // PLL output
    CLOCK_SOURCE_EXTERNAL,             // External clock
    CLOCK_SOURCE_RTC                   // RTC clock
} hw_clock_source_t;

// Timer callback function prototype
typedef void (*hw_timer_callback_t)(void* context);

// Hardware timer management functions
system_error_t hw_timer_init(uint32_t frequency_hz, hw_timer_mode_t mode);
system_error_t hw_timer_deinit(void);
uint64_t hw_timer_get_counter(void);
uint64_t hw_timer_get_frequency(void);

// Timer interrupt functions
system_error_t hw_timer_set_interrupt_callback(hw_timer_callback_t callback, void* context);
system_error_t hw_timer_enable_interrupt(void);
system_error_t hw_timer_disable_interrupt(void);

// Clock management functions
system_error_t hw_timer_set_clock_source(hw_clock_source_t source);
system_error_t hw_timer_get_clock_stability(uint32_t* stability_score);

// Safety functions
system_error_t hw_timer_safety_self_test(void);
system_error_t hw_timer_fault_injection_test(void);

#endif // BSP_HW_TIMER_H