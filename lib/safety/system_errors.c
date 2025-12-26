/**
 * @file system_errors.c
 * @brief System-wide error handling implementation
 * 
 * Provides unified error management across all autoMLOS modules.
 * Ensures consistent and deterministic error handling.
 */

#include "system_errors.h"
#include <string.h>
#include <stdint.h>
#include "../logging/log_client.h"  

// Global error state (for error tracking and reporting)
static system_error_t g_last_error = {0};

/**
 * @brief Log error to appropriate destination with binary logging
 */
void system_error_log(const system_error_t* error) {
    if (error == NULL) {
        return;
    }
    
    // Convert error to binary log format
    uint8_t log_level = LOG_LEVEL_ERROR;
    if (error->severity >= ERROR_SEVERITY_HIGH) {
        log_level = LOG_LEVEL_EMERGENCY;
    } else if (error->severity == ERROR_SEVERITY_MEDIUM) {
        log_level = LOG_LEVEL_ERROR;
    } else {
        log_level = LOG_LEVEL_INFO;
    }
    
    // Send error log entry
    log_client_send_entry(log_level, LOG_TAG_SYSTEM, 
                         LOG_MSG_SERVICE_ERROR, 
                         error->error_code, 
                         error->module_id << 16 | error->category);
}

/**
 * @brief Create a system error with comprehensive context
 * 
 * @param error_code Specific error code
 * @param category Error category
 * @param severity Error severity
 * @param module_id Source module identifier
 * @return system_error_t Created error structure
 */
system_error_t system_error_create(uint32_t error_code, error_category_t category, 
                                  error_severity_t severity, uint16_t module_id) {
    system_error_t error = {
        .error_code = error_code,
        .category = category,
        .severity = severity,
        .module_id = module_id,
        .timestamp = 0, // Will be set by time function, TODO: Implement system timestamp
        .additional_info = 0
    };
    
    // Store last error for debugging
    g_last_error = error;
    
    return error;
}

/**
 * @brief Check if error is safety-critical
 * 
 * Enhanced function with comprehensive boundary condition handling
 * and safety compliance checks.
 * 
 * @param error Pointer to error structure
 * @return 1 if critical, 0 otherwise
 */
int system_error_is_critical(const system_error_t* error) {
    // Safety: NULL pointer check - always critical
    if (error == NULL) {
        return 1; 
    }
    
    // Safety: Validate error severity range
    if (error->severity >= ERROR_SEVERITY_COUNT) {
        return 1; // Invalid severity is critical
    }
    
    // Safety: Validate error category range
    if (error->category >= ERROR_CATEGORY_COUNT) {
        return 1; // Invalid category is critical
    }
    
    // Safety: Critical errors based on severity
    // HIGH severity and above are considered critical for automotive safety
    if (error->severity >= ERROR_SEVERITY_HIGH) {
        return 1;
    }
    
    // Safety: Medium severity safety category errors are also critical
    if (error->category == ERROR_CATEGORY_SAFETY && 
        error->severity >= ERROR_SEVERITY_MEDIUM) {
        return 1;
    }
    
    // Non-critical errors
    return 0;
}

/**
 * @brief Reset error state
 */
void system_error_reset(void) {
    memset(&g_last_error, 0, sizeof(g_last_error));
}