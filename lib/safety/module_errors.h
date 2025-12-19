/**
 * @file module_errors.h
 * @brief Module-specific error code registration and management
 * 
 * Provides centralized management of module-specific error codes
 * with compile-time validation and runtime registration.
 */

#ifndef MODULE_ERRORS_H
#define MODULE_ERRORS_H

#include "system_errors.h"

// Module error registration structure
typedef struct {
    uint16_t module_id;
    const char* module_name;
    uint32_t base_error_code;
    uint32_t max_error_code;
    const char* error_descriptions[256];  // Error code descriptions
} module_error_info_t;

// Module error registration function
int system_register_module_errors(const module_error_info_t* info);

// Module error lookup function
const char* system_get_module_error_description(uint32_t error_code);

// Logging module error descriptions
static const char* log_error_descriptions[] = {
    [0] = "No error",
    [1] = "Log buffer full",
    [2] = "Log entry corrupted",
    [3] = "Buffer state corrupted",
    [4] = "Log system not initialized",
    [5] = "IPC communication failure",
    [6] = "Storage not available",
    // ... other error descriptions
};

// Logging module error registration info
static const module_error_info_t log_module_errors = {
    .module_id = MODULE_ID_LOGGING,
    .module_name = "logging",
    .base_error_code = MODULE_ERROR_BASE(MODULE_ID_LOGGING),
    .max_error_code = MODULE_ERROR_BASE(MODULE_ID_LOGGING) + 255,
    .error_descriptions = log_error_descriptions
};

#endif // MODULE_ERRORS_H