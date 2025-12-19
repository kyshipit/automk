/**
 * @file log_storage_adapter.c
 * @brief Storage adapter implementation - independent from logging core
 * 
 * Provides optional storage functionality through publish-subscribe pattern.
 * Completely decoupled from logging system for fault isolation.
 */

#include "log_storage_adapter.h"
#include <string.h>

// Storage adapter internal state
static struct {
    storage_mode_t mode;
    storage_entry_callback_t callback;
    uint8_t initialized;
    uint8_t subscription_active;
} g_storage_adapter = {0};

/**
 * @brief Initialize storage adapter (independent operation)
 * 
 * @param mode Storage operating mode
 */
void storage_adapter_init(storage_mode_t mode) {
    // Safety: Prevent double initialization
    if (g_storage_adapter.initialized) {
        return;
    }
    
    g_storage_adapter.mode = mode;
    g_storage_adapter.initialized = 1;
    g_storage_adapter.subscription_active = 0;
    
    // Storage initialization logic here (independent from logging)
}

/**
 * @brief Shutdown storage adapter
 */
void storage_adapter_shutdown(void) {
    if (!g_storage_adapter.initialized) {
        return;
    }
    
    g_storage_adapter.initialized = 0;
    g_storage_adapter.subscription_active = 0;
    g_storage_adapter.callback = NULL;
}

/**
 * @brief Subscribe to log entries (optional functionality)
 * 
 * @param callback Storage callback function
 */
void storage_adapter_subscribe(storage_entry_callback_t callback) {
    if (!g_storage_adapter.initialized) {
        return;
    }
    
    g_storage_adapter.callback = callback;
    g_storage_adapter.subscription_active = 1;
}

/**
 * @brief Unsubscribe from log entries
 */
void storage_adapter_unsubscribe(void) {
    g_storage_adapter.subscription_active = 0;
    g_storage_adapter.callback = NULL;
}

/**
 * @brief Check if storage adapter is available
 * 
 * @return 1 if available, 0 otherwise
 */
int storage_adapter_is_available(void) {
    return g_storage_adapter.initialized && g_storage_adapter.subscription_active;
}