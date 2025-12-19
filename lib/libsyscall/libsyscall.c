/**
 * @file syscalls.c
 * @brief System call wrapper implementations for autoMLOS
 * 
 * This file implements the system call wrappers for IPC, time, and process management.
 */

#include "libsyscalls.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// IPC related system calls with unified error handling (updated error codes)
system_error_t sys_ipc_send(uint16_t target_pid, const void* message, size_t size) {
    // Safety checks with proper error codes (updated to new naming)
    if (message == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY, 
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    if (size == 0 || size > 4096) { // Reasonable message size limit
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    if (target_pid == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return success with proper error handling
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

system_error_t sys_ipc_receive(void* buffer, size_t size, uint32_t timeout_ms) {
    (void)timeout_ms; // Mark parameter as used to avoid warning
    
    // Safety checks (updated error codes)
    if (buffer == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    if (size == 0 || size > 4096) {
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return no message (timeout)
    return system_error_create(ERR_BASE_TIMEOUT, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_WARNING, MODULE_ID_SYSCALL);
}

system_error_t sys_ipc_register_topic(const char* topic_name, uint32_t flags) {
    (void)flags; // Mark parameter as used to avoid warning
    
    // Safety checks (updated error codes)
    if (topic_name == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    if (strlen(topic_name) == 0 || strlen(topic_name) > 128) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return success
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

system_error_t sys_ipc_unregister_topic(const char* topic_name) {
    // Safety checks (updated error codes)
    if (topic_name == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return success
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

system_error_t sys_ipc_publish(const char* topic, const void* message, size_t size) {
    // Safety checks (updated error codes)
    if (topic == NULL || message == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    if (size == 0 || size > 4096) {
        return system_error_create(ERR_BASE_INVALID_SIZE, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return success
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

system_error_t sys_ipc_subscribe(const char* topic, uint32_t flags) {
    (void)flags; // Mark parameter as used to avoid warning
    
    // Safety checks (updated error codes)
    if (topic == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would invoke a kernel system call
    // For simulation, return success
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

// Time related system calls (updated error codes)
system_error_t sys_time_get_us(uint64_t* timestamp) {
    // Safety checks
    if (timestamp == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would read from a hardware timer
    // For simulation, use a simple counter
    static uint64_t current_time = 0;
    *timestamp = current_time += 1000; // Increment by 1ms each call
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

// Process related system calls (updated error codes)
system_error_t sys_getpid(uint16_t* pid) {
    // Safety checks
    if (pid == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would return the current process ID
    // For simulation, return a fixed value
    *pid = 123;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

system_error_t sys_service_lookup(const char* service_name, uint16_t* service_pid) {
    // Safety checks (updated error codes)
    if (service_name == NULL || service_pid == NULL) {
        return system_error_create(ERR_SAFE_CRITICAL_NULL, ERROR_CATEGORY_SAFETY,
                                 ERROR_SEVERITY_CRITICAL, MODULE_ID_SYSCALL);
    }
    
    // In a real implementation, this would look up the service in a registry
    // For simulation, check for known services
    if (strcmp(service_name, "system/logger") == 0) {
        *service_pid = 100; // Logger service PID
        return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
    }
    
    // Service not found
    return system_error_create(ERR_SYS_SERVICE_NOT_FOUND, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_WARNING, MODULE_ID_SYSCALL);
}