/**
 * @file hw_rtc.h
 * @brief AutoMLOS Hardware RTC Abstraction Header
 * 
 * Hardware RTC (Real-Time Clock) abstraction layer header for automotive embedded systems.
 */

#ifndef BSP_HW_RTC_H
#define BSP_HW_RTC_H

#include <stdint.h>
#include <stdbool.h>
#include "../../lib/safety/system_errors.h"

// BSP module ID
#define MODULE_ID_BSP    0x50

// RTC function prototypes
system_error_t hw_rtc_init(void);
uint64_t hw_rtc_get_counter(void);
system_error_t hw_rtc_set_time(uint64_t time_us);
bool hw_rtc_is_available(void);

#endif // BSP_HW_RTC_H