/**
 * @file ipc_batch.h
 * @brief AutoMLOS IPC batch messaging mechanism
 * 
 * Generic batch messaging functionality for efficient bulk data transfer,
 * reducing IPC overhead by combining multiple messages into single transfers.
 */

#ifndef IPC_BATCH_H
#define IPC_BATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Batch message structure
typedef struct {
    void* messages;           // Pointer to message array
    size_t message_size;      // Size of each individual message
    uint32_t message_count;   // Number of messages in batch
    uint32_t max_messages;    // Maximum capacity of batch
    uint32_t error_count;     // Error counter for statistics
} ipc_batch_t;

/**
 * @brief Create a batch message container
 * 
 * @param message_size Size of each individual message in bytes
 * @param max_messages Maximum number of messages batch can hold
 * @return Batch handle, NULL on error
 */
ipc_batch_t* ipc_create_batch(size_t message_size, uint32_t max_messages);

/**
 * @brief Add a message to the batch
 * 
 * @param batch Batch handle
 * @param message Pointer to message data
 * @return 0 on success, -1 if batch is full
 */
int ipc_batch_add_message(ipc_batch_t* batch, const void* message);

/**
 * @brief Send batch using publish mechanism
 * 
 * @param publisher Publisher handle (from ipc_pubsub.h)
 * @param batch Batch to send
 * @return 0 on success, -1 on error
 */
int ipc_send_batch(void* publisher, const ipc_batch_t* batch);

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
                     void* user_data, uint32_t timeout_ms);

/**
 * @brief Destroy batch and release resources
 * 
 * @param batch Batch handle to destroy
 */
void ipc_destroy_batch(ipc_batch_t* batch);

/**
 * @brief Get current message count in batch
 * 
 * @param batch Batch handle
 * @return Number of messages, -1 on error
 */
int ipc_batch_get_count(const ipc_batch_t* batch);

/**
 * @brief Check if batch is full
 * 
 * @param batch Batch handle
 * @return 1 if full, 0 if not full, -1 on error
 */
int ipc_batch_is_full(const ipc_batch_t* batch);

/**
 * @brief Clear all messages from batch
 * 
 * @param batch Batch handle
 * @return 0 on success, -1 on error
 */
int ipc_batch_clear(ipc_batch_t* batch);

/**
 * @brief Get batch statistics
 * 
 * @param batch Batch handle
 * @param message_count Output parameter for message count
 * @param error_count Output parameter for error count
 * @return 0 on success, -1 on error
 */
int ipc_batch_get_stats(const ipc_batch_t* batch, uint32_t* message_count, 
                       uint32_t* error_count);

/**
 * @brief Calculate batch efficiency (messages per IPC call)
 * 
 * @param batch Batch handle
 * @return Efficiency ratio (messages per call), -1 on error
 */
float ipc_batch_get_efficiency(const ipc_batch_t* batch);

#endif // IPC_BATCH_H