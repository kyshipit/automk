/**
 * @file syscalls.h
 * @brief System call wrapper definitions for autoMLOS
 * 
 * This file defines the system call interfaces for IPC, time, and process management.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include <stddef.h>
#include "../safety/system_errors.h"

// IPC related system calls with unified error handling
system_error_t sys_ipc_send(uint16_t target_pid, const void* message, size_t size);
system_error_t sys_ipc_receive(void* buffer, size_t size, uint32_t timeout_ms);
system_error_t sys_ipc_register_topic(const char* topic_name, uint32_t flags);
system_error_t sys_ipc_unregister_topic(const char* topic_name);
system_error_t sys_ipc_publish(const char* topic, const void* message, size_t size);
system_error_t sys_ipc_subscribe(const char* topic, uint32_t flags);

// Time related system calls with proper error handling
system_error_t sys_time_get_us(uint64_t* timestamp);

// Process related system calls
system_error_t sys_getpid(uint16_t* pid);
system_error_t sys_service_lookup(const char* service_name, uint16_t* service_pid);

#endif // SYSCALLS_H