/**
 * @file memory.h
 * @brief AutoMLOS Kernel Memory Management
 * 
 * Memory management for automotive embedded systems
 * with ISO 26262 ASIL-D safety compliance.
 */

#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include "../safety/system_errors.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Memory configuration
#define MEMORY_PAGE_SIZE         4096    // Page size in bytes
#define MEMORY_MAX_PAGES         1024    // Maximum number of pages
#define MEMORY_KERNEL_HEAP_SIZE  (1024 * 1024) // 1MB kernel heap

// Memory protection flags
typedef enum {
    MEMORY_PROT_NONE = 0,         // No access
    MEMORY_PROT_READ = 1,         // Read access
    MEMORY_PROT_WRITE = 2,        // Write access
    MEMORY_PROT_EXEC = 4          // Execute access
} memory_protection_t;

// Memory management functions
system_error_t memory_init(void);
void* memory_alloc(size_t size);
void memory_free(void* ptr);
void* memory_realloc(void* ptr, size_t size);

// Shared memory functions
system_error_t memory_alloc_shared(size_t size, void** buffer);

// User space memory validation
void* memory_validate_user_pointer(void* ptr, size_t size);
char* memory_validate_user_string(char* str, size_t max_len);

// Memory statistics
size_t memory_get_free_size(void);
size_t memory_get_used_size(void);
size_t memory_get_total_size(void);

#endif // KERNEL_MEMORY_H