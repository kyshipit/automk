/**
 * @file kernel.h
 * @brief AutoMLOS Kernel Main Header
 * 
 * Kernel entry points and main functions for automotive embedded systems.
 */

#ifndef KERNEL_KERNEL_H
#define KERNEL_KERNEL_H

#include "../../lib/safety/system_errors.h"
#include "../../lib/logging/log_common.h"    
#include "../../lib/watchdog/watchdog.h"

// Kernel module IDs
#define MODULE_ID_KERNEL    0x10

// Kernel management functions
system_error_t kernel_init(void);
void kernel_main(void);
void kernel_shutdown(void);

// Kernel logging functions
system_error_t kernel_log_watchdog_pet(uint8_t component_id);

#endif // KERNEL_KERNEL_H