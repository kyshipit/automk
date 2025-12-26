/**
 * @file time.c
 * @brief AutoMLOS Kernel Time Management with Multi-Source Redundancy
 * 
 * Automotive-grade time management with ISO 26262 ASIL-D compliance,
 * featuring multiple time sources and automatic fault detection/switching.
 * 
 * @author AutoMLOS Team
 * @version 2.4
 * @date 2024
 * @copyright AutoMLOS Automotive Systems
 */

#include "time.h"
#include <stdint.h>
#include <string.h> 
#include "../logging/log_client.h"
#include "../../platform/bsp/hw_timer.h"
#include "../../platform/bsp/hw_rtc.h"  // Include RTC for backup time source

// Time source configuration
#define PRIMARY_TIME_SOURCE_FREQUENCY_HZ   1000000    // 1MHz primary timer
#define SECONDARY_TIME_SOURCE_FREQUENCY_HZ 32768      // 32.768kHz RTC backup
#define TIME_SOURCE_VALIDATION_INTERVAL_MS 1000       // Validate every 1 second
#define MAX_TIME_DRIFT_US                  1000       // Maximum acceptable drift: 1ms

// Time-specific log tags and messages
#define LOG_TAG_TIME 100  // Custom time module tag ID
#define LOG_MSG_PRIMARY_TIME_SOURCE_READY   200
#define LOG_MSG_SECONDARY_TIME_SOURCE_READY 201
#define LOG_MSG_SOFTWARE_TIME_SOURCE_ACTIVE 202
#define LOG_MSG_TIME_DRIFT_DETECTED         203
#define LOG_MSG_TIME_SOURCE_FAILED          204
#define LOG_MSG_TIME_SOURCE_SWITCHED        205

// Time module ID (using system module ID space)
#define MODULE_ID_TIME 0x6000

// Enhanced kernel time context with redundancy
static struct {
    uint64_t boot_time_ticks[TIME_SOURCE_COUNT];  // Boot time per source
    uint64_t tick_frequency[TIME_SOURCE_COUNT];   // Frequency per source
    bool source_initialized[TIME_SOURCE_COUNT];   // Initialization status
    bool source_healthy[TIME_SOURCE_COUNT];       // Health status
    time_source_t active_source;                  // Currently active source
    uint64_t last_validation_time;                // Last validation timestamp
    uint32_t source_fail_count[TIME_SOURCE_COUNT]; // Failure counters
    uint64_t cumulative_drift[TIME_SOURCE_COUNT];  // Cumulative time drift
} kernel_time_ctx = {0};

/**
 * @brief Create success error code for time module
 * 
 * @return system_error_t Success error structure
 */
static system_error_t time_error_success(void) {
    return system_error_create(ERR_SYS_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_NONE, MODULE_ID_TIME);
}

/**
 * @brief Create already initialized error for time module
 * 
 * @return system_error_t Already initialized error structure
 */
static system_error_t time_error_already_initialized(void) {
    return system_error_create(ERR_SYS_ALREADY_INIT, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_WARNING, MODULE_ID_TIME);
}

/**
 * @brief Create invalid parameter error for time module
 * 
 * @return system_error_t Invalid parameter error structure
 */
static system_error_t time_error_invalid_parameter(void) {
    return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_MEDIUM, MODULE_ID_TIME);
}

/**
 * @brief Initialize primary time source (high-frequency hardware timer)
 * 
 * @return system_error_t Initialization status
 */
static system_error_t init_primary_time_source(void) {
    system_error_t error = hw_timer_init(PRIMARY_TIME_SOURCE_FREQUENCY_HZ, 
                                       TIMER_MODE_FREE_RUNNING);
    if (error.error_code != ERR_BASE_OK) { 
        return error;
    }
    
    kernel_time_ctx.boot_time_ticks[TIME_SOURCE_PRIMARY] = hw_timer_get_counter();
    kernel_time_ctx.tick_frequency[TIME_SOURCE_PRIMARY] = hw_timer_get_frequency();
    kernel_time_ctx.source_initialized[TIME_SOURCE_PRIMARY] = true;
    kernel_time_ctx.source_healthy[TIME_SOURCE_PRIMARY] = true;
    
    log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_TIME,
                        LOG_MSG_PRIMARY_TIME_SOURCE_READY,
                        PRIMARY_TIME_SOURCE_FREQUENCY_HZ, 0);
    
    return time_error_success();
}

/**
 * @brief Initialize secondary time source (real-time clock)
 * 
 * @return system_error_t Initialization status
 */
static system_error_t init_secondary_time_source(void) {
    system_error_t error = hw_rtc_init();
    if (error.error_code != ERR_BASE_OK) {
        return error;
    }
    
    kernel_time_ctx.boot_time_ticks[TIME_SOURCE_SECONDARY] = hw_rtc_get_counter();
    kernel_time_ctx.tick_frequency[TIME_SOURCE_SECONDARY] = SECONDARY_TIME_SOURCE_FREQUENCY_HZ;
    kernel_time_ctx.source_initialized[TIME_SOURCE_SECONDARY] = true;
    kernel_time_ctx.source_healthy[TIME_SOURCE_SECONDARY] = true;
    
    log_client_send_entry(LOG_LEVEL_INFO, LOG_TAG_TIME,
                        LOG_MSG_SECONDARY_TIME_SOURCE_READY,
                        SECONDARY_TIME_SOURCE_FREQUENCY_HZ, 0);
    
    return time_error_success();
}

/**
 * @brief Initialize software fallback time source
 * 
 * @return system_error_t Initialization status
 */
static system_error_t init_software_time_source(void) {
    // Software-based time source using system tick counter
    // This is a fallback when hardware sources fail
    kernel_time_ctx.boot_time_ticks[TIME_SOURCE_SOFTWARE] = 0;
    kernel_time_ctx.tick_frequency[TIME_SOURCE_SOFTWARE] = 1000; // 1kHz software timer
    kernel_time_ctx.source_initialized[TIME_SOURCE_SOFTWARE] = true;
    kernel_time_ctx.source_healthy[TIME_SOURCE_SOFTWARE] = true;
    
    log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_TIME,
                        LOG_MSG_SOFTWARE_TIME_SOURCE_ACTIVE, 0, 0);
    
    return time_error_success();
}

/**
 * @brief Validate time source consistency and detect faults
 * 
 * @return system_error_t Validation status
 * 
 * @implements ISO 26262 Part 6-7.4.10 time source validation
 */
static system_error_t validate_time_sources(void) {
    uint64_t current_time = kernel_time_get_tick_count();
    
    // Check if it's time for validation
    if (current_time - kernel_time_ctx.last_validation_time < 
        TIME_SOURCE_VALIDATION_INTERVAL_MS * 1000) {
        return time_error_success();
    }
    
    kernel_time_ctx.last_validation_time = current_time;
    
    // Compare time sources for consistency
    uint64_t primary_time = 0, secondary_time = 0;
    
    if (kernel_time_ctx.source_healthy[TIME_SOURCE_PRIMARY]) {
        primary_time = hw_timer_get_counter() - kernel_time_ctx.boot_time_ticks[TIME_SOURCE_PRIMARY];
        primary_time = (primary_time * 1000000ULL) / kernel_time_ctx.tick_frequency[TIME_SOURCE_PRIMARY];
    }
    
    if (kernel_time_ctx.source_healthy[TIME_SOURCE_SECONDARY]) {
        secondary_time = hw_rtc_get_counter() - kernel_time_ctx.boot_time_ticks[TIME_SOURCE_SECONDARY];
        secondary_time = (secondary_time * 1000000ULL) / kernel_time_ctx.tick_frequency[TIME_SOURCE_SECONDARY];
    }
    
    // Check for significant drift between sources
    if (kernel_time_ctx.source_healthy[TIME_SOURCE_PRIMARY] && 
        kernel_time_ctx.source_healthy[TIME_SOURCE_SECONDARY]) {
        uint64_t drift = (primary_time > secondary_time) ? 
                        (primary_time - secondary_time) : (secondary_time - primary_time);
        
        kernel_time_ctx.cumulative_drift[TIME_SOURCE_PRIMARY] += drift;
        kernel_time_ctx.cumulative_drift[TIME_SOURCE_SECONDARY] += drift;
        
        if (drift > MAX_TIME_DRIFT_US) {
            // Log time drift warning
            log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_TIME,
                                LOG_MSG_TIME_DRIFT_DETECTED, drift, 
                                kernel_time_ctx.active_source);
            
            // Mark source with higher drift as unhealthy
            if (primary_time > secondary_time) {
                kernel_time_ctx.source_fail_count[TIME_SOURCE_PRIMARY]++;
            } else {
                kernel_time_ctx.source_fail_count[TIME_SOURCE_SECONDARY]++;
            }
        }
    }
    
    // Check if any source has exceeded failure threshold
    for (int i = 0; i < TIME_SOURCE_COUNT; i++) {
        if (kernel_time_ctx.source_fail_count[i] > 3) {
            kernel_time_ctx.source_healthy[i] = false;
            log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_TIME,
                                LOG_MSG_TIME_SOURCE_FAILED, i, 
                                kernel_time_ctx.source_fail_count[i]);
        }
    }
    
    // Switch to backup source if primary fails
    if (!kernel_time_ctx.source_healthy[kernel_time_ctx.active_source]) {
        for (int i = 0; i < TIME_SOURCE_COUNT; i++) {
            if (kernel_time_ctx.source_healthy[i] && kernel_time_ctx.source_initialized[i]) {
                kernel_time_ctx.active_source = i;
                log_client_send_entry(LOG_LEVEL_ERROR, LOG_TAG_TIME,
                                    LOG_MSG_TIME_SOURCE_SWITCHED,
                                    kernel_time_ctx.active_source, i);
                break;
            }
        }
    }
    
    return time_error_success();
}

/**
 * @brief Initialize kernel time management with redundant sources
 * 
 * @return system_error_t Initialization status
 */
system_error_t kernel_time_init(void) {
    if (kernel_time_ctx.source_initialized[TIME_SOURCE_PRIMARY]) {
        return time_error_already_initialized();
    }
    
    system_error_t error;
    
    // Initialize primary time source
    error = init_primary_time_source();
    if (error.error_code == ERR_BASE_OK) {
        kernel_time_ctx.active_source = TIME_SOURCE_PRIMARY;
    }
    
    // Initialize secondary time source (continue even if primary fails)
    init_secondary_time_source();
    
    // Initialize software fallback (always available)
    init_software_time_source();
    
    // Set initial validation time
    kernel_time_ctx.last_validation_time = kernel_time_get_tick_count();
    
    return (kernel_time_ctx.active_source != TIME_SOURCE_SOFTWARE) ? 
           time_error_success() : system_error_create(ERR_SYS_CONFIG_ERROR, 
                                                     ERROR_CATEGORY_SYSTEM,
                                                     ERROR_SEVERITY_WARNING,
                                                     MODULE_ID_TIME);
}

/**
 * @brief Get current tick count from active time source
 * 
 * @return uint64_t Current tick count
 */
uint64_t kernel_time_get_tick_count(void) {
    if (!kernel_time_ctx.source_initialized[kernel_time_ctx.active_source]) {
        return 0;
    }
    
    // Perform periodic validation
    validate_time_sources();
    
    // Get time from active source
    switch (kernel_time_ctx.active_source) {
        case TIME_SOURCE_PRIMARY:
            return hw_timer_get_counter() - kernel_time_ctx.boot_time_ticks[TIME_SOURCE_PRIMARY];
            
        case TIME_SOURCE_SECONDARY:
            return hw_rtc_get_counter() - kernel_time_ctx.boot_time_ticks[TIME_SOURCE_SECONDARY];
            
        case TIME_SOURCE_SOFTWARE: {
            // Software-based tick counter (simplified implementation)
            static uint64_t software_ticks = 0;
            software_ticks++;
            return software_ticks;
        }
            
        default:
            return 0;
    }
}

/**
 * @brief Get time source health information
 * 
 * @param health_info Pointer to health info structure
 * @return system_error_t Operation status
 */
system_error_t kernel_time_get_health_info(time_source_health_t* health_info) {
    if (health_info == NULL) {
        return time_error_invalid_parameter();
    }
    
    // Copy health information
    for (int i = 0; i < TIME_SOURCE_COUNT; i++) {
        health_info->source_initialized[i] = kernel_time_ctx.source_initialized[i];
        health_info->source_healthy[i] = kernel_time_ctx.source_healthy[i];
        health_info->fail_count[i] = kernel_time_ctx.source_fail_count[i];
        health_info->cumulative_drift[i] = kernel_time_ctx.cumulative_drift[i];
    }
    health_info->active_source = kernel_time_ctx.active_source;
    health_info->system_healthy = (kernel_time_ctx.active_source != TIME_SOURCE_SOFTWARE);
    
    return time_error_success();
}

uint64_t kernel_time_get_uptime_us(void)
{
    return kernel_time_get_tick_count();
}

uint32_t kernel_time_get_uptime_ms(void)
{
    return (uint32_t)(kernel_time_get_uptime_us() / 1000);
}

void kernel_time_delay_us(uint32_t us)
{
    uint64_t start = kernel_time_get_tick_count();
    while ((kernel_time_get_tick_count() - start) < us) {
        // Busy wait
    }
}

void kernel_time_delay_ms(uint32_t ms)
{
    kernel_time_delay_us(ms * 1000);
}

uint64_t kernel_time_get_interrupt_start(void)
{
    return kernel_time_get_tick_count();
}

uint64_t kernel_time_calculate_latency(uint64_t start_time)
{
    uint64_t end_time = kernel_time_get_tick_count();
    if (end_time >= start_time) {
        return end_time - start_time;
    }
    // Handle timer overflow
    return (UINT64_MAX - start_time) + end_time + 1;
}