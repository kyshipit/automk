/**
 * @file log_core.h
 * @brief Binary logging system core definitions for autoMLOS
 * 
 * This module implements a memory-safe, time-deterministic logging system
 * that complies with automotive safety standards (ASIL-D).
 * 
 * Key safety features:
 * - No dynamic memory allocation
 * - Fixed-size buffers with boundary checks
 * - No formatting functions at runtime
 * - Worst-case execution time predictability
 * - Unified system error handling
 */

#ifndef LOG_CORE_H
#define LOG_CORE_H

#include <stdint.h>
#include <stddef.h>
#include "../safety/system_errors.h"
#include "log_common.h"  // Use common logging definitions

// Module identifier for error reporting (aligned with system errors)
#define LOG_MODULE_ID MODULE_ID_LOGGING

// Constants for safety validation (based on log_common.h definitions)
#define MAX_LOG_TAG_ID (LOG_TAG_COUNT - 1)
#define MAX_LOG_MESSAGE_ID (LOG_MSG_COUNT - 1)

// Enhanced API with explicit error reporting (using unified system errors)
system_error_t log_system_init_safe(void);
system_error_t log_record_binary_safe(uint8_t level, uint8_t tag_id, uint16_t message_id, 
                                     uint32_t data1, uint32_t data2);
system_error_t log_buffer_put_safe(log_buffer_t* buffer, const log_entry_binary_t* entry);
system_error_t log_buffer_get_safe(log_buffer_t* buffer, log_entry_binary_t* entry);
system_error_t log_verify_entry_integrity_safe(const log_entry_binary_t* entry);

// Log entry creation function
system_error_t log_create_entry(log_entry_binary_t* entry, uint8_t level, uint8_t tag_id, 
                               uint16_t message_id, uint32_t data1, uint32_t data2);

// Original API declarations (maintained for backward compatibility)
void log_system_init(void);
void log_record_binary(uint8_t level, uint8_t tag_id, uint16_t message_id, uint32_t data1, uint32_t data2);
log_buffer_t* get_global_log_buffer(void);

// Buffer management functions
void log_buffer_init(log_buffer_t* buffer);

#endif // LOG_CORE_H