/**
 * @file time.h
 * @brief AutoMLOS Kernel Time Management Interface
 */

#ifndef KERNEL_TIME_H
#define KERNEL_TIME_H

#include "../safety/system_errors.h"
#include "../../platform/bsp/hw_timer.h"
#include "../logging/log_common.h"
// Time source types
typedef enum {
    TIME_SOURCE_PRIMARY = 0,      // High-frequency hardware timer
    TIME_SOURCE_SECONDARY,        // Real-time clock (RTC)
    TIME_SOURCE_SOFTWARE,         // Software-based fallback
    TIME_SOURCE_COUNT
} time_source_t;

// Time source health information structure
typedef struct {
    bool source_initialized[TIME_SOURCE_COUNT];
    bool source_healthy[TIME_SOURCE_COUNT];
    uint32_t fail_count[TIME_SOURCE_COUNT];
    uint64_t cumulative_drift[TIME_SOURCE_COUNT];
    time_source_t active_source;
    bool system_healthy;
} time_source_health_t;

// Time management functions
system_error_t kernel_time_init(void);
uint64_t kernel_time_get_tick_count(void);
uint64_t kernel_time_get_uptime_us(void);
uint32_t kernel_time_get_uptime_ms(void);
void kernel_time_delay_us(uint32_t us);
void kernel_time_delay_ms(uint32_t ms);
uint64_t kernel_time_get_interrupt_start(void);
uint64_t kernel_time_calculate_latency(uint64_t start_time);
system_error_t kernel_time_get_health_info(time_source_health_t* health_info);
// Time logging functions
system_error_t time_log_hw_timer_init(uint32_t frequency);
system_error_t time_log_timer_overflow(void);

#endif // KERNEL_TIME_H