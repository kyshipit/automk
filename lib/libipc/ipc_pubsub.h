/**
 * @file ipc_pubsub.h
 * @brief AutoMLOS IPC publish-subscribe mechanism
 * 
 * Generic publish-subscribe functionality for all services to use,
 * leveraging kernel routing advantages for one-to-many communication.
 */

#ifndef IPC_PUBSUB_H
#define IPC_PUBSUB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "ipc_topics.h"

// Maximum message size for IPC communication
#define IPC_MAX_MESSAGE_SIZE 4096

// Maximum topic name length
#define IPC_TOPIC_MAX_LENGTH 64

// Publisher handle type
typedef struct ipc_publisher ipc_publisher_t;

// Subscriber handle type
typedef struct ipc_subscriber ipc_subscriber_t;

// Message callback function type
typedef void (*ipc_message_callback_t)(const void* message, size_t size, void* user_data);

/**
 * @brief Create a publisher for a specific topic
 * 
 * @param topic Topic to publish to
 * @return Publisher handle, NULL on error
 */
ipc_publisher_t* ipc_create_publisher(const char* topic);

/**
 * @brief Publish a message to the topic
 * 
 * @param publisher Publisher handle
 * @param message Message data to publish
 * @param size Size of message in bytes
 * @return 0 on success, -1 on error
 */
int ipc_publish(ipc_publisher_t* publisher, const void* message, size_t size);

/**
 * @brief Publish a message with timeout support
 * 
 * @param publisher Publisher handle
 * @param message Message data to publish
 * @param size Size of message in bytes
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, -1 on error or timeout
 */
int ipc_publish_timeout(ipc_publisher_t* publisher, const void* message, 
                       size_t size, uint32_t timeout_ms);

/**
 * @brief Destroy a publisher and release resources
 * 
 * @param publisher Publisher handle to destroy
 */
void ipc_destroy_publisher(ipc_publisher_t* publisher);

/**
 * @brief Create a subscriber for a specific topic
 * 
 * @param topic Topic to subscribe to
 * @param callback Function to call when message received
 * @param user_data User data passed to callback
 * @return Subscriber handle, NULL on error
 */
ipc_subscriber_t* ipc_create_subscriber(const char* topic, 
                                       ipc_message_callback_t callback, 
                                       void* user_data);

/**
 * @brief Start receiving messages for a subscriber
 * 
 * @param subscriber Subscriber handle
 * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
 * @return 0 on success, -1 on error or timeout
 */
int ipc_subscriber_receive(ipc_subscriber_t* subscriber, uint32_t timeout_ms);

/**
 * @brief Destroy a subscriber and release resources
 * 
 * @param subscriber Subscriber handle to destroy
 */
void ipc_destroy_subscriber(ipc_subscriber_t* subscriber);

/**
 * @brief Get number of active subscribers for a topic
 * 
 * @param topic Topic to check
 * @return Number of active subscribers, -1 on error
 */
int ipc_get_subscriber_count(const char* topic);

/**
 * @brief Check if a topic has active subscribers
 * 
 * @param topic Topic to check
 * @return 1 if has subscribers, 0 if not, -1 on error
 */
int ipc_topic_has_subscribers(const char* topic);

/**
 * @brief Get publisher statistics
 * 
 * @param publisher Publisher handle
 * @param message_count Output parameter for message count
 * @return 0 on success, -1 on error
 */
int ipc_publisher_get_stats(ipc_publisher_t* publisher, uint32_t* message_count);

/**
 * @brief Get subscriber statistics
 * 
 * @param subscriber Subscriber handle
 * @param message_count Output parameter for message count
 * @return 0 on success, -1 on error
 */
int ipc_subscriber_get_stats(ipc_subscriber_t* subscriber, uint32_t* message_count);

/**
 * @brief IPC statistics structure
 */
typedef struct {
    uint64_t total_messages;        // Total messages processed
    uint64_t total_errors;          // Total errors encountered
    uint32_t active_publishers;     // Number of active publishers
    uint32_t active_subscribers;    // Number of active subscribers
    uint64_t timestamp;             // Last update timestamp
} ipc_stats_t;

/**
 * @brief IPC configuration structure
 */
typedef struct {
    size_t max_message_size;        // Maximum message size
    size_t queue_size;              // Message queue size
    uint32_t timeout_ms;            // Default timeout in milliseconds
} ipc_config_t;

// Additional function prototypes
int ipc_init(void);
void ipc_cleanup(void);
int ipc_get_stats(ipc_stats_t* stats);
int ipc_set_config(const ipc_config_t* config);
int ipc_get_config(ipc_config_t* config);

#endif // IPC_PUBSUB_H