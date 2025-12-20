/**
 * @file ipc_batch.c
 * @brief AutoMLOS IPC batch messaging mechanism implementation
 * 
 * Implements efficient batch messaging for bulk data transfer,
 * reducing IPC overhead by combining multiple messages.
 */

#include "ipc_batch.h"
#include "ipc_pubsub.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Create a batch message container
 * 
 * @param message_size Size of each individual message in bytes
 * @param max_messages Maximum number of messages batch can hold
 * @return Batch handle, NULL on error
 */
ipc_batch_t* ipc_create_batch(size_t message_size, uint32_t max_messages) {
    // Safety check: validate parameters
    if (message_size == 0) {
        fprintf(stderr, "IPC Batch: Invalid message size: 0\n");
        return NULL;
    }
    
    if (max_messages == 0) {
        fprintf(stderr, "IPC Batch: Invalid max messages: 0\n");
        return NULL;
    }
    
    // Check for potential overflow
    if (message_size > SIZE_MAX / max_messages) {
        fprintf(stderr, "IPC Batch: Message size too large for batch capacity\n");
        return NULL;
    }
    
    // Allocate batch structure
    ipc_batch_t* batch = malloc(sizeof(ipc_batch_t));
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Failed to allocate batch structure\n");
        return NULL;
    }
    
    // Allocate message storage
    size_t total_size = message_size * max_messages;
    batch->messages = malloc(total_size);
    if (batch->messages == NULL) {
        fprintf(stderr, "IPC Batch: Failed to allocate message storage\n");
        free(batch);
        return NULL;
    }
    
    // Initialize batch parameters
    batch->message_size = message_size;
    batch->message_count = 0;
    batch->max_messages = max_messages;
    batch->error_count = 0;
    
    printf("IPC Batch: Created batch with message size: %zu, capacity: %u\n", 
           message_size, max_messages);
    
    return batch;
}

/**
 * @brief Add a message to the batch
 * 
 * @param batch Batch handle
 * @param message Pointer to message data
 * @return 0 on success, -1 if batch is full
 */
int ipc_batch_add_message(ipc_batch_t* batch, const void* message) {
    // Safety check: null pointer validation
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1;
    }
    
    if (message == NULL) {
        fprintf(stderr, "IPC Batch: Null message pointer\n");
        batch->error_count++;
        return -1;
    }
    
    // Check if batch is full
    if (batch->message_count >= batch->max_messages) {
        fprintf(stderr, "IPC Batch: Batch is full, cannot add more messages\n");
        batch->error_count++;
        return -1;
    }
    
    // Calculate destination address and copy message
    char* dest = (char*)batch->messages + (batch->message_count * batch->message_size);
    memcpy(dest, message, batch->message_size);
    
    batch->message_count++;
    return 0;
}

/**
 * @brief Send batch using publish mechanism
 * 
 * @param publisher Publisher handle (from ipc_pubsub.h)
 * @param batch Batch to send
 * @return 0 on success, -1 on error
 */
int ipc_send_batch(void* publisher, const ipc_batch_t* batch) {
    // Safety check: null pointer validation
    if (publisher == NULL) {
        fprintf(stderr, "IPC Batch: Null publisher pointer\n");
        return -1;
    }
    
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1;
    }
    
    // Check if batch is empty
    if (batch->message_count == 0) {
        printf("IPC Batch: Batch is empty, nothing to send\n");
        return 0; // Nothing to send
    }
    
    // Calculate total batch size
    size_t total_size = batch->message_count * batch->message_size;
    
    // Use the publisher to send the entire batch
    ipc_publisher_t* pub = (ipc_publisher_t*)publisher;
    int result = ipc_publish(pub, batch->messages, total_size);
    
    if (result == 0) {
        printf("IPC Batch: Successfully sent batch with %u messages, total size: %zu\n",
               batch->message_count, total_size);
    } else {
        fprintf(stderr, "IPC Batch: Failed to send batch\n");
    }
    
    return result;
}

/**
 * @brief Receive and process batch messages
 * 
 * @param subscriber Subscriber handle (from ipc_pubsub.h)
 * @param callback Function to call for each message
 * @param user_data User data passed to callback
 * @param timeout_ms Timeout in milliseconds
 * @return Number of messages processed, -1 on error
 */
int ipc_receive_batch(void* subscriber, void (*callback)(const void*, size_t, void*), 
                     void* user_data, uint32_t timeout_ms) {
    // Safety check: null pointer validation
    if (subscriber == NULL || callback == NULL) {
        fprintf(stderr, "IPC Batch: Null pointer in receive batch\n");
        return -1;
    }
    
    // Mark parameters as used to avoid warnings
    (void)user_data;
    
    // Convert subscriber handle to proper type
    ipc_subscriber_t* sub = (ipc_subscriber_t*)subscriber;
    
    // Use the subscriber to receive messages
    int result = ipc_subscriber_receive(sub, timeout_ms);
    
    if (result == 0) {
        // In a real implementation, this would parse the batch and call callback for each message
        // For now, simulate processing by calling callback once with placeholder data
        printf("IPC Batch: Received batch via subscriber\n");
        
        // Create placeholder message data
        char placeholder_message[] = "Batch message placeholder";
        size_t message_size = sizeof(placeholder_message);
        
        // Call user callback with placeholder data
        callback(placeholder_message, message_size, user_data);
        
        return 1; // Return 1 message processed (placeholder)
    } else {
        fprintf(stderr, "IPC Batch: Failed to receive batch via subscriber\n");
        return -1;
    }
}

/**
 * @brief Destroy batch and release resources
 * 
 * @param batch Batch handle to destroy
 */
void ipc_destroy_batch(ipc_batch_t* batch) {
    if (batch != NULL) {
        printf("IPC Batch: Destroying batch (messages: %u, errors: %u)\n",
               batch->message_count, batch->error_count);
        
        if (batch->messages != NULL) {
            free(batch->messages);
        }
        free(batch);
    }
}

/**
 * @brief Get current message count in batch
 * 
 * @param batch Batch handle
 * @return Number of messages, -1 on error
 */
int ipc_batch_get_count(const ipc_batch_t* batch) {
    // Safety check: null pointer validation
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1;
    }
    
    return (int)batch->message_count;
}

/**
 * @brief Check if batch is full
 * 
 * @param batch Batch handle
 * @return 1 if full, 0 if not full, -1 on error
 */
int ipc_batch_is_full(const ipc_batch_t* batch) {
    // Safety check: null pointer validation
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1;
    }
    
    return (batch->message_count >= batch->max_messages) ? 1 : 0;
}

/**
 * @brief Clear all messages from batch
 * 
 * @param batch Batch handle
 * @return 0 on success, -1 on error
 */
int ipc_batch_clear(ipc_batch_t* batch) {
    // Safety check: null pointer validation
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1;
    }
    
    batch->message_count = 0;
    printf("IPC Batch: Cleared batch, message count reset to 0\n");
    return 0;
}

/**
 * @brief Get batch statistics
 * 
 * @param batch Batch handle
 * @param message_count Output parameter for message count
 * @param error_count Output parameter for error count
 * @return 0 on success, -1 on error
 */
int ipc_batch_get_stats(const ipc_batch_t* batch, uint32_t* message_count, 
                       uint32_t* error_count) {
    // Safety check: null pointer validation
    if (batch == NULL || message_count == NULL || error_count == NULL) {
        fprintf(stderr, "IPC Batch: Null pointer in get stats\n");
        return -1;
    }
    
    *message_count = batch->message_count;
    *error_count = batch->error_count;
    return 0;
}

/**
 * @brief Calculate batch efficiency (messages per IPC call)
 * 
 * @param batch Batch handle
 * @return Efficiency ratio (messages per call), -1 on error
 */
float ipc_batch_get_efficiency(const ipc_batch_t* batch) {
    // Safety check: null pointer validation
    if (batch == NULL) {
        fprintf(stderr, "IPC Batch: Null batch pointer\n");
        return -1.0f;
    }
    
    if (batch->message_count == 0) {
        return 0.0f;
    }
    
    // Efficiency is the number of messages per batch
    // Higher values indicate better efficiency
    return (float)batch->message_count;
}