/**
 * @file syscall.h
 * @brief AutoMLOS Kernel System Call Definitions
 * 
 * System call numbers and interface definitions for automotive embedded systems.
 */

#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "../safety/system_errors.h"

// System call numbers
typedef enum {
    // IPC system calls
    SYSCALL_IPC_SEND = 0,
    SYSCALL_IPC_RECEIVE,
    SYSCALL_IPC_REGISTER_TOPIC,
    SYSCALL_IPC_UNREGISTER_TOPIC,
    SYSCALL_IPC_PUBLISH,
    SYSCALL_IPC_SUBSCRIBE,
    
    // Time system calls
    SYSCALL_TIME_GET_US,
    
    // Process system calls
    SYSCALL_GETPID,
    SYSCALL_SERVICE_LOOKUP,
    
    // Maximum system call number
    SYSCALL_MAX
} syscall_number_t;

// System call function prototypes
system_error_t syscall_init(void);
system_error_t syscall_handler(uint32_t syscall_num, uint32_t arg1, 
                              uint32_t arg2, uint32_t arg3, uint32_t arg4);

// External dependencies (to be implemented in other modules)
uint16_t scheduler_get_current_pid(void);
void* memory_validate_user_pointer(void* ptr, size_t size);
char* memory_validate_user_string(char* str, size_t max_len);

#endif // KERNEL_SYSCALL_H