/**
 * @file syscall.c
 * @brief AutoMLOS Kernel System Call Handler
 * 
 * System call interface implementation for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 */

#include "syscall.h"
#include "ipc.h"
#include "scheduler.h"
#include "memory.h"
#include "../safety/system_errors.h"
#include <stdint.h>

// System call table
typedef system_error_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, 
                                           uint32_t arg3, uint32_t arg4);

static syscall_handler_t g_syscall_table[SYSCALL_MAX];

/**
 * @brief Initialize system call subsystem
 */
system_error_t syscall_init(void) {
    // Clear system call table
    for (int i = 0; i < SYSCALL_MAX; i++) {
        g_syscall_table[i] = NULL;
    }
    
    // Register IPC system calls
    g_syscall_table[SYSCALL_IPC_SEND] = syscall_ipc_send;
    g_syscall_table[SYSCALL_IPC_RECEIVE] = syscall_ipc_receive;
    g_syscall_table[SYSCALL_IPC_REGISTER_TOPIC] = syscall_ipc_register_topic;
    g_syscall_table[SYSCALL_IPC_UNREGISTER_TOPIC] = syscall_ipc_unregister_topic;
    g_syscall_table[SYSCALL_IPC_PUBLISH] = syscall_ipc_publish;
    g_syscall_table[SYSCALL_IPC_SUBSCRIBE] = syscall_ipc_subscribe;
    
    // Register time system calls
    g_syscall_table[SYSCALL_TIME_GET_US] = syscall_time_get_us;
    
    // Register process system calls
    g_syscall_table[SYSCALL_GETPID] = syscall_getpid;
    g_syscall_table[SYSCALL_SERVICE_LOOKUP] = syscall_service_lookup;
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

/**
 * @brief Main system call handler
 */
system_error_t syscall_handler(uint32_t syscall_num, uint32_t arg1, 
                              uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    // Validate system call number
    if (syscall_num >= SYSCALL_MAX) {
        return system_error_create(ERR_SYS_INVALID_SYSCALL, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Check if system call is implemented
    if (g_syscall_table[syscall_num] == NULL) {
        return system_error_create(ERR_SYS_SYSCALL_NOT_IMPLEMENTED, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Call the system call handler
    return g_syscall_table[syscall_num](arg1, arg2, arg3, arg4);
}

// IPC System Call Implementations

/**
 * @brief IPC send system call
 */
static system_error_t syscall_ipc_send(uint32_t target_pid, uint32_t message_ptr, 
                                      uint32_t size, uint32_t flags) {
    // Validate parameters
    if (target_pid == 0 || message_ptr == 0 || size == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get current process ID
    uint16_t current_pid = scheduler_get_current_pid();
    
    // Validate message pointer (user space to kernel space)
    void* kernel_message = memory_validate_user_pointer((void*)message_ptr, size);
    if (kernel_message == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Call kernel IPC function
    return ipc_publish_message("direct", kernel_message, size, current_pid);
}

/**
 * @brief IPC receive system call
 */
static system_error_t syscall_ipc_receive(uint32_t buffer_ptr, uint32_t size, 
                                         uint32_t timeout_ms, uint32_t flags) {
    // Validate parameters
    if (buffer_ptr == 0 || size == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate buffer pointer
    void* kernel_buffer = memory_validate_user_pointer((void*)buffer_ptr, size);
    if (kernel_buffer == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get current process ID
    uint16_t current_pid = scheduler_get_current_pid();
    
    // Call kernel IPC function (simplified - real implementation would block)
    return ipc_receive_message(current_pid, kernel_buffer, size, timeout_ms);
}

/**
 * @brief IPC register topic system call
 */
static system_error_t syscall_ipc_register_topic(uint32_t topic_name_ptr, 
                                                uint32_t flags, uint32_t arg3, 
                                                uint32_t arg4) {
    // Validate parameters
    if (topic_name_ptr == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate topic name pointer
    char* kernel_topic_name = memory_validate_user_string((char*)topic_name_ptr, 64);
    if (kernel_topic_name == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Call kernel IPC function
    return ipc_register_topic(kernel_topic_name, flags);
}

/**
 * @brief IPC publish system call
 */
static system_error_t syscall_ipc_publish(uint32_t topic_name_ptr, 
                                         uint32_t message_ptr, uint32_t size, 
                                         uint32_t flags) {
    // Validate parameters
    if (topic_name_ptr == 0 || message_ptr == 0 || size == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate topic name pointer
    char* kernel_topic_name = memory_validate_user_string((char*)topic_name_ptr, 64);
    if (kernel_topic_name == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate message pointer
    void* kernel_message = memory_validate_user_pointer((void*)message_ptr, size);
    if (kernel_message == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get current process ID
    uint16_t current_pid = scheduler_get_current_pid();
    
    // Call kernel IPC function
    return ipc_publish_message(kernel_topic_name, kernel_message, size, current_pid);
}

/**
 * @brief IPC subscribe system call
 */
static system_error_t syscall_ipc_subscribe(uint32_t topic_name_ptr, 
                                           uint32_t flags, uint32_t arg3, 
                                           uint32_t arg4) {
    // Validate parameters
    if (topic_name_ptr == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate topic name pointer
    char* kernel_topic_name = memory_validate_user_string((char*)topic_name_ptr, 64);
    if (kernel_topic_name == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get current process ID
    uint16_t current_pid = scheduler_get_current_pid();
    
    // Call kernel IPC function
    return ipc_subscribe_topic(kernel_topic_name, current_pid);
}

// Time System Call Implementation

/**
 * @brief Get system time in microseconds
 */
static system_error_t syscall_time_get_us(uint32_t timestamp_ptr, uint32_t arg2, 
                                         uint32_t arg3, uint32_t arg4) {
    // Validate parameters
    if (timestamp_ptr == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate timestamp pointer
    uint64_t* kernel_timestamp = memory_validate_user_pointer((uint64_t*)timestamp_ptr, 
                                                             sizeof(uint64_t));
    if (kernel_timestamp == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get system time (to be implemented in time subsystem)
    *kernel_timestamp = 0; // Placeholder
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

// Process System Call Implementations

/**
 * @brief Get current process ID
 */
static system_error_t syscall_getpid(uint32_t pid_ptr, uint32_t arg2, 
                                    uint32_t arg3, uint32_t arg4) {
    // Validate parameters
    if (pid_ptr == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate PID pointer
    uint16_t* kernel_pid = memory_validate_user_pointer((uint16_t*)pid_ptr, 
                                                       sizeof(uint16_t));
    if (kernel_pid == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Get current process ID
    *kernel_pid = scheduler_get_current_pid();
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

/**
 * @brief Service lookup system call
 */
static system_error_t syscall_service_lookup(uint32_t service_name_ptr, 
                                            uint32_t service_pid_ptr, 
                                            uint32_t arg3, uint32_t arg4) {
    // Validate parameters
    if (service_name_ptr == 0 || service_pid_ptr == 0) {
        return system_error_create(ERR_SYS_INVALID_PARAM, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate service name pointer
    char* kernel_service_name = memory_validate_user_string((char*)service_name_ptr, 64);
    if (kernel_service_name == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Validate service PID pointer
    uint16_t* kernel_service_pid = memory_validate_user_pointer((uint16_t*)service_pid_ptr, 
                                                               sizeof(uint16_t));
    if (kernel_service_pid == NULL) {
        return system_error_create(ERR_SYS_INVALID_POINTER, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_SYSCALL);
    }
    
    // Service lookup logic (to be implemented in service registry)
    *kernel_service_pid = 0; // Placeholder
    
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_SYSCALL);
}

// Stub implementations for dependencies
uint16_t scheduler_get_current_pid(void) {
    return 1; // To be implemented in scheduler
}

void* memory_validate_user_pointer(void* ptr, size_t size) {
    return ptr; // Simplified validation for now
}

char* memory_validate_user_string(char* str, size_t max_len) {
    return str; // Simplified validation for now
}