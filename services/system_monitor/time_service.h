/**
 * @file time_service.h
 * @brief AutoMLOS Time Service Interface
 * 
 * System time service for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 * 
 * Features:
 * - High-resolution system time
 * - Time synchronization
 * - Timer services
 * - Clock monitoring
 */

#ifndef SERVICES_TIME_SERVICE_H
#define SERVICES_TIME_SERVICE_H

#include "../../lib/safety/system_errors.h"
#include <stdint.h>
#include <stdbool.h>

// Time service configuration
#define TIME_SERVICE_SYNC_INTERVAL_MS     1000    // Time synchronization interval

// Time service interface functions
system_error_t time_service_init(void);
system_error_t time_service_start(void);
uint64_t time_service_get_system_time_us(void);
uint64_t time_service_get_system_time_ms(void);

// Timer service functions
system_error_t time_service_create_timer(uint32_t* timer_id, uint64_t delay_us, 
                                        uint64_t period_us, void (*callback)(void*), void* context);
system_error_t time_service_start_timer(uint32_t timer_id);
system_error_t time_service_stop_timer(uint32_t timer_id);

// Synchronization functions
system_error_t time_service_sync_with_master(uint64_t master_time_us);
system_error_t time_service_get_sync_status(bool* synchronized);

#endif // SERVICES_TIME_SERVICE_H