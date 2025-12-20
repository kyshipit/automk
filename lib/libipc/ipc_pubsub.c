/**
 * @file ipc_pubsub.c
 * @brief AutoMLOS IPC publish-subscribe mechanism implementation
 * 
 * Generic publish-subscribe functionality for all services to use,
 * leveraging kernel routing advantages for one-to-many communication.
 */

#include "ipc_pubsub.h"
#include "../libsyscall/libsyscalls.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Publisher structure
struct ipc_publisher {
    char topic[IPC_TOPIC_MAX_LENGTH];
    uint32_t message_count;
    uint32_t error_count;
};

// Subscriber structure
struct ipc_subscriber {
    char topic[IPC_TOPIC_MAX_LENGTH];
    ipc_message_callback_t callback;
    void* user_data;
    uint32_t message_count;
    uint32_t error_count;
};

/**
 * @brief Create a publisher for a specific topic
 * 
 * @param topic Topic to publish to
 * @return Publisher handle, NULL on error
 */
ipc_publisher_t* ipc_create_publisher(const char* topic) {
    // Safety check: null pointer validation
    if (topic == NULL) {
        fprintf(stderr, "IPC PubSub: Null topic pointer\n");
        return NULL;
    }
    
    // Validate topic format
    if (ipc_validate_topic(topic) != IPC_TOPIC_VALID) {
        fprintf(stderr, "IPC PubSub: Invalid topic format: %s\n", topic);
        return NULL;
    }
    
    // Safety check: topic length validation
    if (strlen(topic) >= IPC_TOPIC_MAX_LENGTH) {
        fprintf(stderr, "IPC PubSub: Topic too long: %s\n", topic);
        return NULL;
    }
    
    // Allocate publisher structure
    ipc_publisher_t* publisher = malloc(sizeof(ipc_publisher_t));
    if (publisher == NULL) {
        fprintf(stderr, "IPC PubSub: Failed to allocate publisher\n");
        return NULL;
    }
    
    // Initialize publisher structure
    strncpy(publisher->topic, topic, IPC_TOPIC_MAX_LENGTH - 1);
    publisher->topic[IPC_TOPIC_MAX_LENGTH - 1] = '\0';
    publisher->message_count = 0;
    publisher->error_count = 0;
    
    // Register with kernel for publishing
    system_error_t result = sys_ipc_register_topic(topic, 0);
    if (result.error_code != ERR_BASE_OK) {
        fprintf(stderr, "IPC PubSub: Failed to register topic: %s, error: %d\n", 
                topic, result.error_code);
        free(publisher);
        return NULL;
    }
    
    printf("IPC PubSub: Created publisher for topic: %s\n", topic);
    return publisher;
}

/**
 * @brief Publish a message to the topic
 * 
 * @param publisher Publisher handle
 * @param message Message data to publish
 * @param size Size of message in bytes
 * @return 0 on success, -1 on error
 */
int ipc_publish(ipc_publisher_t* publisher, const void* message, size_t size) {
    // Safety check: null pointer validation
    if (publisher == NULL) {
        fprintf(stderr, "IPC PubSub: Null publisher pointer\n");
        return -1;
    }
    
    if (message == NULL) {
        fprintf(stderr, "IPC PubSub: Null message pointer for topic: %s\n", publisher->topic);
        publisher->error_count++;
        return -1;
    }
    
    // Safety check: message size validation
    if (size == 0) {
        fprintf(stderr, "IPC PubSub: Zero message size for topic: %s\n", publisher->topic);
        publisher->error_count++;
        return -1;
    }
    
    if (size > IPC_MAX_MESSAGE_SIZE) {
        fprintf(stderr, "IPC PubSub: Message too large: %zu bytes for topic: %s\n", 
                size, publisher->topic);
        publisher->error_count++;
        return -1;
    }
    
    // Use kernel broadcast mechanism for one-to-many publishing
    system_error_t result = sys_ipc_publish(publisher->topic, message, size);
    if (result.error_code != ERR_BASE_OK) {
        fprintf(stderr, "IPC PubSub: Failed to publish to topic: %s, error: %d\n", 
                publisher->topic, result.error_code);
        publisher->error_count++;
        return -1;
    }
    
    publisher->message_count++;
    return 0;
}

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
                       size_t size, uint32_t timeout_ms) {
    // For now, use the same implementation as ipc_publish
    // In a real system, this would implement timeout logic
    (void)timeout_ms; // Mark parameter as used to avoid warning
    return ipc_publish(publisher, message, size);
}

/**
 * @brief Destroy a publisher and release resources
 * 
 * @param publisher Publisher handle to destroy
 */
void ipc_destroy_publisher(ipc_publisher_t* publisher) {
    if (publisher != NULL) {
        printf("IPC PubSub: Destroying publisher for topic: %s (messages: %u, errors: %u)\n",
               publisher->topic, publisher->message_count, publisher->error_count);
        free(publisher);
    }
}

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
                                       void* user_data) {
    // Safety check: null pointer validation
    if (topic == NULL) {
        fprintf(stderr, "IPC PubSub: Null topic pointer\n");
        return NULL;
    }
    
    if (callback == NULL) {
        fprintf(stderr, "IPC PubSub: Null callback pointer\n");
        return NULL;
    }
    
    // Validate topic format
    if (ipc_validate_topic(topic) != IPC_TOPIC_VALID) {
        fprintf(stderr, "IPC PubSub: Invalid topic format: %s\n", topic);
        return NULL;
    }
    
    // Safety check: topic length validation
    if (strlen(topic) >= IPC_TOPIC_MAX_LENGTH) {
        fprintf(stderr, "IPC PubSub: Topic too long: %s\n", topic);
        return NULL;
    }
    
    // Allocate subscriber structure
    ipc_subscriber_t* subscriber = malloc(sizeof(ipc_subscriber_t));
    if (subscriber == NULL) {
        fprintf(stderr, "IPC PubSub: Failed to allocate subscriber\n");
        return NULL;
    }
    
    // Initialize subscriber structure
    strncpy(subscriber->topic, topic, IPC_TOPIC_MAX_LENGTH - 1);
    subscriber->topic[IPC_TOPIC_MAX_LENGTH - 1] = '\0';
    subscriber->callback = callback;
    subscriber->user_data = user_data;
    subscriber->message_count = 0;
    subscriber->error_count = 0;
    
    // Register with kernel for subscription
    system_error_t result = sys_ipc_subscribe(topic, 0);
    if (result.error_code != ERR_BASE_OK) {
        fprintf(stderr, "IPC PubSub: Failed to subscribe to topic: %s, error: %d\n", 
                topic, result.error_code);
        free(subscriber);
        return NULL;
    }
    
    printf("IPC PubSub: Created subscriber for topic: %s\n", topic);
    return subscriber;
}

/**
 * @brief Start receiving messages for a subscriber
 * 
 * @param subscriber Subscriber handle
 * @param timeout_ms Timeout in milliseconds (0 = non-blocking)
 * @return 0 on success, -1 on error or timeout
 */
int ipc_subscriber_receive(ipc_subscriber_t* subscriber, uint32_t timeout_ms) {
    // Safety check: null pointer validation
    if (subscriber == NULL) {
        fprintf(stderr, "IPC PubSub: Null subscriber pointer\n");
        return -1;
    }
    
    // Use kernel receive mechanism with timeout
    char buffer[IPC_MAX_MESSAGE_SIZE];
    size_t received_size = 0;
    
    system_error_t result = sys_ipc_receive(buffer, sizeof(buffer), timeout_ms);
    if (result.error_code == ERR_BASE_OK && received_size > 0) {
        // Call user callback with received message
        subscriber->callback(buffer, received_size, subscriber->user_data);
        subscriber->message_count++;
        return 0;
    } else if (result.error_code != ERR_BASE_OK) {
        subscriber->error_count++;
        fprintf(stderr, "IPC PubSub: Receive error for topic: %s, error: %d\n",
                subscriber->topic, result.error_code);
    }
    
    return -1;
}

/**
 * @brief Destroy a subscriber and release resources
 * 
 * @param subscriber Subscriber handle to destroy
 */
void ipc_destroy_subscriber(ipc_subscriber_t* subscriber) {
    if (subscriber != NULL) {
        // Unsubscribe from topic
        sys_ipc_unregister_topic(subscriber->topic);
        
        printf("IPC PubSub: Destroying subscriber for topic: %s (messages: %u, errors: %u)\n",
               subscriber->topic, subscriber->message_count, subscriber->error_count);
        free(subscriber);
    }
}

/**
 * @brief Get number of active subscribers for a topic
 * 
 * @param topic Topic to check
 * @return Number of active subscribers, -1 on error
 */
int ipc_get_subscriber_count(const char* topic) {
    // Safety check: null pointer validation
    if (topic == NULL) {
        fprintf(stderr, "IPC PubSub: Null topic pointer\n");
        return -1;
    }
    
    // Note: sys_ipc_get_subscriber_count is not available in libsyscalls.h
    // For now, return a default value (0 subscribers)
    // In a real implementation, this would query the kernel for subscriber count
    (void)topic; // Mark parameter as used to avoid warning
    return 0;
}

/**
 * @brief Check if a topic has active subscribers
 * 
 * @param topic Topic to check
 * @return 1 if has subscribers, 0 if not, -1 on error
 */
int ipc_topic_has_subscribers(const char* topic) {
    int count = ipc_get_subscriber_count(topic);
    if (count > 0) {
        return 1;
    } else if (count == 0) {
        return 0;
    } else {
        return -1;
    }
}

/**
 * @brief Get publisher statistics
 * 
 * @param publisher Publisher handle
 * @param message_count Output parameter for message count
 * @return 0 on success, -1 on error
 */
int ipc_publisher_get_stats(ipc_publisher_t* publisher, uint32_t* message_count) {
    // Safety check: null pointer validation
    if (publisher == NULL || message_count == NULL) {
        fprintf(stderr, "IPC PubSub: Null pointer in publisher stats\n");
        return -1;
    }
    
    *message_count = publisher->message_count;
    return 0;
}

/**
 * @brief Get subscriber statistics
 * 
 * @param subscriber Subscriber handle
 * @param message_count Output parameter for message count
 * @return 0 on success, -1 on error
 */
int ipc_subscriber_get_stats(ipc_subscriber_t* subscriber, uint32_t* message_count) {
    // Safety check: null pointer validation
    if (subscriber == NULL || message_count == NULL) {
        fprintf(stderr, "IPC PubSub: Null pointer in subscriber stats\n");
        return -1;
    }
    
    *message_count = subscriber->message_count;
    return 0;
}