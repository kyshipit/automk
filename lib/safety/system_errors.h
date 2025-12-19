/**
 * @file system_errors.h
 * @brief System-wide error definitions for autoMLOS
 * 
 * Provides unified error handling framework across all modules.
 * Ensures consistent error reporting and safety compliance.
 * 
 * Key features:
 * - Unified error codes across all modules
 * - Safety-critical error prioritization
 * - Deterministic error handling
 * - ASIL-D compliance
 */

#ifndef SYSTEM_ERRORS_H
#define SYSTEM_ERRORS_H

#include <stdint.h>

// System-wide error categories
typedef enum {
    ERROR_CATEGORY_SUCCESS = 0,      // Operation completed successfully
    ERROR_CATEGORY_SAFETY,           // Safety-critical errors
    ERROR_CATEGORY_SYSTEM,           // System-level errors
    ERROR_CATEGORY_MODULE,           // Module-specific errors
    ERROR_CATEGORY_RESOURCE,         // Resource management errors
    ERROR_CATEGORY_COMMUNICATION,    // Communication errors
    ERROR_CATEGORY_COUNT
} error_category_t;

// Unified error code type definitions with consistent naming
typedef enum {
    ERR_BASE_OK = 0,                     // Base success code
    ERR_BASE_NULL_PTR = 0x1000,          // Base null pointer error
    ERR_BASE_BUF_OVERFLOW,               // Base buffer overflow
    ERR_BASE_BUF_EMPTY,                  // Base buffer empty
    ERR_BASE_INDEX_OOB,                  // Base index out of bounds
    ERR_BASE_DATA_CORRUPT,               // Base data corruption
    ERR_BASE_STATE_CORRUPT,              // Base state corruption
    ERR_BASE_TIMEOUT,                    // Base timeout error
    ERR_BASE_HW_FAULT,                   // Base hardware fault
    ERR_BASE_SW_FAULT,                   // Base software fault
    ERR_BASE_INTEGRITY_FAIL,             // Base integrity check failed
    ERR_BASE_INVALID_SIZE,               // Base invalid size parameter
    ERR_BASE_COUNT
} err_base_t;

typedef enum {
    ERR_SYS_OK = 0,                      // System success code
    ERR_SYS_NOT_INIT = 0x2000,           // System not initialized
    ERR_SYS_ALREADY_INIT,                // System already initialized
    ERR_SYS_INVALID_PARAM,               // System invalid parameter
    ERR_SYS_RESOURCE_UNAVAIL,            // System resource unavailable
    ERR_SYS_PERMISSION_DENIED,           // System permission denied
    ERR_SYS_CONFIG_ERROR,                // System configuration error
    ERR_SYS_SERVICE_NOT_FOUND,           // System service not found
    ERR_SYS_SERVICE_REG_FAILED,          // System service registration failed
    ERR_SYS_COUNT
} err_sys_t;

typedef enum {
    ERR_SAFE_OK = 0,                     // Safety success code
    ERR_SAFE_CRITICAL_NULL = 0x3000,     // Safety-critical null pointer
    ERR_SAFE_CRITICAL_BUF_OVER,          // Safety-critical buffer overflow
    ERR_SAFE_MEM_PROTECT_FAULT,          // Safety memory protection fault
    ERR_SAFE_STACK_OVERFLOW,             // Safety stack overflow
    ERR_SAFE_WATCHDOG_TIMEOUT,           // Safety watchdog timeout
    ERR_SAFE_POWER_FAULT,                // Safety power fault
    ERR_SAFE_TEMP_FAULT,                 // Safety temperature fault
    ERR_SAFE_SENSOR_FAULT,               // Safety sensor fault
    ERR_SAFE_ACTUATOR_FAULT,             // Safety actuator fault
    ERR_SAFE_COUNT
} err_safe_t;

// Error severity levels (for prioritization)
typedef enum {
    ERROR_SEVERITY_NONE = 0,         // No error
    ERROR_SEVERITY_LOW,              // Non-critical error
    ERROR_SEVERITY_WARNING,          // Warning level
    ERROR_SEVERITY_MEDIUM,           // Functional error
    ERROR_SEVERITY_HIGH,             // System error
    ERROR_SEVERITY_CRITICAL,         // Safety-critical error
    ERROR_SEVERITY_COUNT
} error_severity_t;

// Module identifier definitions (each module gets 16-bit ID space)
typedef enum {
    MODULE_ID_SYSCALL = 0x0000,      // System call module (0x0000-0x0FFF)
    MODULE_ID_LOGGING = 0x1000,      // Logging module (0x1000-0x1FFF)
    MODULE_ID_AI = 0x2000,           // AI module (0x2000-0x2FFF)
    MODULE_ID_DIAGNOSTICS = 0x3000,  // Diagnostics module (0x3000-0x3FFF)
    MODULE_ID_IPC = 0x4000,          // IPC module (0x4000-0x4FFF)
    MODULE_ID_STORAGE = 0x5000,      // Storage module (0x5000-0x5FFF)
    MODULE_ID_COUNT
} module_id_t;

// Module-specific error code generation macros (updated naming)
#define MODULE_ERR_BASE(module_id) ((module_id) << 16)
#define MODULE_ERR_CODE(module_id, err_num) (MODULE_ERR_BASE(module_id) | (err_num))

// Logging module error codes (using unified error system with new naming)
#define ERR_MOD_LOG_OK                  MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x00)
#define ERR_MOD_LOG_BUF_FULL            MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x01)
#define ERR_MOD_LOG_ENTRY_CORRUPT       MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x02)
#define ERR_MOD_LOG_BUF_CORRUPT         MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x03)
#define ERR_MOD_LOG_NOT_INIT            MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x04)
#define ERR_MOD_LOG_IPC_COMM            MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x05)
#define ERR_MOD_LOG_STORAGE_UNAVAIL     MODULE_ERR_CODE(MODULE_ID_LOGGING, 0x06)

// AI module error codes (example for future modules with new naming)
#define ERR_MOD_AI_OK                   MODULE_ERR_CODE(MODULE_ID_AI, 0x00)
#define ERR_MOD_AI_MODEL_NOT_LOADED     MODULE_ERR_CODE(MODULE_ID_AI, 0x01)
#define ERR_MOD_AI_INFERENCE_FAILED     MODULE_ERR_CODE(MODULE_ID_AI, 0x02)

// System error structure (comprehensive error context)
typedef struct {
    uint32_t error_code;          // Specific error code (err_base_t, err_sys_t, err_safe_t, or module-specific)
    error_category_t category;    // Error category for classification
    error_severity_t severity;    // Error severity level for prioritization
    uint16_t module_id;           // Source module identifier for tracing
    uint64_t timestamp;           // Error occurrence timestamp (microseconds)
    uint32_t additional_info;     // Additional context-specific information
    uint16_t line_number;         // Source code line number (for debugging)
    const char* file_name;        // Source file name (for debugging)
    uint32_t checksum;            // Integrity checksum for error structure
} system_error_t;

// Error handling function prototypes
system_error_t system_error_create(uint32_t error_code, error_category_t category, 
                                  error_severity_t severity, uint16_t module_id);
int system_error_is_critical(const system_error_t* error);
void system_error_log(const system_error_t* error);
void system_error_reset(void);

// Enhanced safety-critical error handling macros with module support
#define SAFETY_CHECK_NULL(ptr, module_id) \
    do { \
        if ((ptr) == NULL) { \
            system_error_t err = system_error_create(ERR_SAFE_CRITICAL_NULL, \
                                                   ERROR_CATEGORY_SAFETY, \
                                                   ERROR_SEVERITY_CRITICAL, \
                                                   module_id); \
            system_error_log(&err); \
            return err; \
        } \
    } while(0)

#define SAFETY_CHECK_BOUNDS(index, max, module_id) \
    do { \
        if ((index) >= (max)) { \
            system_error_t err = system_error_create(ERR_BASE_INDEX_OOB, \
                                                   ERROR_CATEGORY_SAFETY, \
                                                   ERROR_SEVERITY_CRITICAL, \
                                                   module_id); \
            system_error_log(&err); \
            return err; \
        } \
    } while(0)

#define SAFETY_CHECK_BUFFER(buffer, size, module_id) \
    do { \
        if ((buffer) == NULL || (size) == 0) { \
            system_error_t err = system_error_create(ERR_SAFE_CRITICAL_BUF_OVER, \
                                                   ERROR_CATEGORY_SAFETY, \
                                                   ERROR_SEVERITY_CRITICAL, \
                                                   module_id); \
            system_error_log(&err); \
            return err; \
        } \
    } while(0)

// Module-specific error creation macro (updated naming)
#define MODULE_ERR_CREATE(module_err_code, severity, module_id) \
    system_error_create(module_err_code, ERROR_CATEGORY_MODULE, severity, module_id)

#endif // SYSTEM_ERRORS_H