/**
 * @file log_storage_adapter.h
 * @brief Storage adapter for logging system - completely decoupled
 * 
 * Implements publish-subscribe pattern for storage operations.
 * Logging core publishes entries, storage subscribes independently.
 * 
 * Key design principles:
 * - Complete separation from logging core
 * - Optional storage functionality
 * - Failure isolation (storage failures don't affect logging)
 * - Asynchronous operation for performance
 */

#ifndef LOG_STORAGE_ADAPTER_H
#define LOG_STORAGE_ADAPTER_H

#include "log_core.h"

// Storage adapter operating modes
typedef enum {
    STORAGE_MODE_DISABLED = 0,    // Storage completely disabled
    STORAGE_MODE_BUFFER_ONLY,     // Buffer only, no persistent storage
    STORAGE_MODE_PERSISTENT       // Full persistent storage
} storage_mode_t;

// Storage callback function type (for subscription)
typedef void (*storage_entry_callback_t)(const log_entry_binary_t* entry);

// Storage adapter API (completely independent)
void storage_adapter_init(storage_mode_t mode);
void storage_adapter_shutdown(void);
void storage_adapter_subscribe(storage_entry_callback_t callback);
void storage_adapter_unsubscribe(void);
int storage_adapter_is_available(void);

#endif // LOG_STORAGE_ADAPTER_H