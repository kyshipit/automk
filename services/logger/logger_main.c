/**
 * @file logger_main.c
 * @brief Logger service main program using libipc pubsub
 * 
 * Implements the logger service main loop using libipc pubsub mechanism
 * for efficient one-to-many communication with multiple clients.
 */

#include "logger_service.h"
#include "../../lib/libipc/ipc_pubsub.h"
#include "../../lib/libsyscall/libsyscalls.h"
#include "../../lib/logging/log_common.h"  // Add log_common.h for log_entry_binary_t
#include <stdio.h>
#include <unistd.h>

// Service configuration
#define LOGGER_SERVICE_LOOP_DELAY_MS    100    // Main loop delay in milliseconds
#define LOGGER_SERVICE_MAX_SUBSCRIBERS  32     // Maximum expected subscribers

// Global subscriber for receiving log entries
static ipc_subscriber_t* g_log_subscriber = NULL;

/**
 * @brief Callback function for received log entries
 * 
 * @param message Received message data
 * @param size Message size
 * @param user_data User data (unused)
 */
static void log_entry_callback(const void* message, size_t size, void* user_data) {
    // Mark unused parameter to avoid compiler warning
    (void)user_data;
    
    // Safety check: null pointer validation
    if (message == NULL || size < sizeof(log_entry_binary_t)) {
        return;
    }
    
    // Process the log entry message
    system_error_t result = logger_service_process_message(message, size);
    if (result.error_code != ERR_BASE_OK) {
        // Log processing error internally
        printf("Logger service: Failed to process log entry, error: %d\n", 
               result.error_code);
    }
}

/**
 * @brief Initialize logger service with pubsub mechanism
 * 
 * @return system_error_t Initialization status
 */
static system_error_t logger_service_init_pubsub(void) {
    // Initialize the core logger service
    system_error_t result = logger_service_init();
    if (result.error_code != ERR_BASE_OK) {
        return result;
    }
    
    // Create subscriber for log entries
    g_log_subscriber = ipc_create_subscriber(IPC_TOPIC_SYSTEM_LOGGING_INFO, 
                                            log_entry_callback, NULL);
    if (g_log_subscriber == NULL) {
        return system_error_create(ERR_SYS_SERVICE_NOT_FOUND, ERROR_CATEGORY_SYSTEM,
                                 ERROR_SEVERITY_HIGH, MODULE_ID_LOGGING);
    }
    
    printf("Logger service: Successfully initialized with pubsub mechanism\n");
    return system_error_create(ERR_BASE_OK, ERROR_CATEGORY_SYSTEM,
                             ERROR_SEVERITY_LOW, MODULE_ID_LOGGING);
}

/**
 * @brief Main service loop using pubsub mechanism
 */
static void logger_service_loop(void) {
    printf("Logger service: Starting main loop\n");
    
    while (1) {
        // Receive messages with timeout (non-blocking)
        int result = ipc_subscriber_receive(g_log_subscriber, LOGGER_SERVICE_LOOP_DELAY_MS);
        
        if (result == 0) {
            // Message processed successfully in callback
            // Additional service maintenance can be done here
        }
        
        // Service maintenance tasks (buffer flushing, status updates, etc.)
        // This runs periodically regardless of message reception
        
        // Check if service should shut down (could be based on signal)
        // if (should_shutdown) break;
    }
}

/**
 * @brief Cleanup logger service resources
 */
static void logger_service_cleanup(void) {
    if (g_log_subscriber != NULL) {
        ipc_destroy_subscriber(g_log_subscriber);
        g_log_subscriber = NULL;
    }
    
    printf("Logger service: Cleanup completed\n");
}

/**
 * @brief Main entry point for logger service
 * 
 * @return int Exit status
 */
int main(void) {
    printf("Logger service: Starting...\n");
    
    // Initialize service with pubsub mechanism
    system_error_t init_result = logger_service_init_pubsub();
    if (init_result.error_code != ERR_BASE_OK) {
        printf("Logger service: Initialization failed with error: %d\n", 
               init_result.error_code);
        return -1;
    }
    
    // Run main service loop
    logger_service_loop();
    
    // Cleanup before exit
    logger_service_cleanup();
    
    printf("Logger service: Exiting\n");
    return 0;
}