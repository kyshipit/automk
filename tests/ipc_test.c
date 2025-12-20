/**
 * @file ipc_test.c
 * @brief AutoMLOS IPC library test and validation program
 * 
 * Comprehensive test suite for IPC publish-subscribe and batch messaging functionality.
 * Tests all functions with safety checks and error handling.
 */

#include "../lib/libipc/ipc_pubsub.h"
#include "../lib/libipc/ipc_batch.h"
#include "../lib/libipc/ipc_topics.h"
#include "../lib/safety/system_errors.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Test message structure
typedef struct {
    uint32_t sequence;
    uint64_t timestamp;
    char message[32];
} test_message_t;

// Global counters for test results
static uint32_t g_received_messages = 0;
static uint32_t g_test_passed = 0;
static uint32_t g_test_failed = 0;

/**
 * @brief Test message callback function
 * 
 * @param message Received message data
 * @param size Message size
 * @param user_data User data (unused)
 */
static void test_message_callback(const void* message, size_t size, void* user_data) {
    // Mark unused parameter to avoid compiler warning
    (void)user_data;

    // Safety check: null pointer validation
    if (message == NULL || size < sizeof(test_message_t)) {
        printf("TEST FAILED: Null message or invalid size in callback\n");
        g_test_failed++;
        return;
    }
    
    const test_message_t* msg = (const test_message_t*)message;
    printf("TEST: Received message #%u: %s\n", msg->sequence, msg->message);
    g_received_messages++;
}

/**
 * @brief Test IPC topic validation functionality
 * 
 * @return 0 on success, -1 on failure
 */
static int test_topic_validation(void) {
    printf("\n=== Testing IPC Topic Validation ===\n");
    
    // Test valid topics (must follow domain/component/action format with exactly 2 slashes)
    const char* valid_topics[] = {
        "system/logging/info",
        "system/logging/error", 
        "system/monitor/status",  // Fixed: changed from "system/status" to follow 3-level naming
        "application/vehicle/control",
        NULL
    };
    
    for (int i = 0; valid_topics[i] != NULL; i++) {
        if (ipc_validate_topic(valid_topics[i]) != IPC_TOPIC_VALID) {
            printf("TEST FAILED: Valid topic rejected: %s\n", valid_topics[i]);
            return -1;
        }
        printf("TEST PASSED: Topic validated: %s\n", valid_topics[i]);
    }
    
    // Test invalid topics
    const char* invalid_topics[] = {
        "",
        "invalid/topic/with/too/many/levels",
        "no_slashes",
        "/leading/slash",
        "trailing/slash/",
        "system/status",  // Now correctly invalid: only 1 slash (should be 2)
        NULL
    };
    
    for (int i = 0; invalid_topics[i] != NULL; i++) {
        if (ipc_validate_topic(invalid_topics[i]) == IPC_TOPIC_VALID) {
            printf("TEST FAILED: Invalid topic accepted: %s\n", invalid_topics[i]);
            return -1;
        }
        printf("TEST PASSED: Topic rejected: %s\n", invalid_topics[i]);
    }
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Test IPC publisher creation and destruction
 * 
 * @return 0 on success, -1 on failure
 */
static int test_publisher_creation(void) {
    printf("\n=== Testing IPC Publisher Creation ===\n");
    
    // Test valid publisher creation
    ipc_publisher_t* publisher = ipc_create_publisher("system/logging/info");
    if (publisher == NULL) {
        printf("IPC PubSub: Invalid topic format: system/logging/info\n");
        printf("TEST FAILED: Failed to create valid publisher\n");
        return -1;
    }
    printf("TEST PASSED: Publisher created successfully\n");
    
    // Test publisher statistics
    uint32_t message_count;
    if (ipc_publisher_get_stats(publisher, &message_count) != 0) {
        printf("TEST FAILED: Failed to get publisher stats\n");
        ipc_destroy_publisher(publisher);
        return -1;
    }
    printf("TEST PASSED: Publisher stats retrieved: messages=%u\n", message_count);
    
    // Test invalid publisher creation
    ipc_publisher_t* invalid_publisher = ipc_create_publisher(NULL);
    if (invalid_publisher != NULL) {
        printf("TEST FAILED: Created publisher with NULL topic\n");
        ipc_destroy_publisher(publisher);
        ipc_destroy_publisher(invalid_publisher);
        return -1;
    }
    printf("TEST PASSED: Null topic correctly rejected\n");
    
    ipc_destroy_publisher(publisher);
    printf("TEST PASSED: Publisher destroyed successfully\n");
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Test IPC subscriber creation and destruction
 * 
 * @return 0 on success, -1 on failure
 */
static int test_subscriber_creation(void) {
    printf("\n=== Testing IPC Subscriber Creation ===\n");
    
    // Test valid subscriber creation
    ipc_subscriber_t* subscriber = ipc_create_subscriber("system/logging/info", 
                                                        test_message_callback, NULL);
    if (subscriber == NULL) {
        printf("IPC PubSub: Invalid topic format: system/logging/info\n");
        printf("TEST FAILED: Failed to create valid subscriber\n");
        return -1;
    }
    printf("TEST PASSED: Subscriber created successfully\n");
    
    // Test subscriber statistics
    uint32_t message_count;
    if (ipc_subscriber_get_stats(subscriber, &message_count) != 0) {
        printf("TEST FAILED: Failed to get subscriber stats\n");
        ipc_destroy_subscriber(subscriber);
        return -1;
    }
    printf("TEST PASSED: Subscriber stats retrieved: messages=%u\n", message_count);
    
    // Test invalid subscriber creation
    ipc_subscriber_t* invalid_subscriber1 = ipc_create_subscriber(NULL, 
                                                                 test_message_callback, NULL);
    if (invalid_subscriber1 != NULL) {
        printf("TEST FAILED: Created subscriber with NULL topic\n");
        ipc_destroy_subscriber(subscriber);
        ipc_destroy_subscriber(invalid_subscriber1);
        return -1;
    }
    printf("TEST PASSED: Null topic correctly rejected\n");
    
    ipc_subscriber_t* invalid_subscriber2 = ipc_create_subscriber("system/logging/info", 
                                                                 NULL, NULL);
    if (invalid_subscriber2 != NULL) {
        printf("TEST FAILED: Created subscriber with NULL callback\n");
        ipc_destroy_subscriber(subscriber);
        ipc_destroy_subscriber(invalid_subscriber2);
        return -1;
    }
    printf("TEST PASSED: Null callback correctly rejected\n");
    
    ipc_destroy_subscriber(subscriber);
    printf("TEST PASSED: Subscriber destroyed successfully\n");
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Test IPC batch messaging functionality
 * 
 * @return 0 on success, -1 on failure
 */
static int test_batch_messaging(void) {
    printf("\n=== Testing IPC Batch Messaging ===\n");
    
    // Create batch for test messages
    ipc_batch_t* batch = ipc_create_batch(sizeof(test_message_t), 10);
    if (batch == NULL) {
        printf("TEST FAILED: Failed to create batch\n");
        return -1;
    }
    printf("TEST PASSED: Batch created successfully\n");
    
    // Test adding messages to batch
    test_message_t test_msg;
    for (uint32_t i = 0; i < 5; i++) {
        test_msg.sequence = i;
        test_msg.timestamp = i * 1000;
        snprintf(test_msg.message, sizeof(test_msg.message), "Test message %u", i);
        
        if (ipc_batch_add_message(batch, &test_msg) != 0) {
            printf("TEST FAILED: Failed to add message %u to batch\n", i);
            ipc_destroy_batch(batch);
            return -1;
        }
    }
    printf("TEST PASSED: Added 5 messages to batch\n");
    
    // Test batch statistics
    uint32_t message_count, error_count;
    if (ipc_batch_get_stats(batch, &message_count, &error_count) != 0) {
        printf("TEST FAILED: Failed to get batch stats\n");
        ipc_destroy_batch(batch);
        return -1;
    }
    printf("TEST PASSED: Batch stats: messages=%u, errors=%u\n", message_count, error_count);
    
    // Test batch efficiency calculation
    float efficiency = ipc_batch_get_efficiency(batch);
    if (efficiency < 0) {
        printf("TEST FAILED: Failed to calculate batch efficiency\n");
        ipc_destroy_batch(batch);
        return -1;
    }
    printf("TEST PASSED: Batch efficiency: %.2f messages per call\n", efficiency);
    
    // Test batch clearing
    if (ipc_batch_clear(batch) != 0) {
        printf("TEST FAILED: Failed to clear batch\n");
        ipc_destroy_batch(batch);
        return -1;
    }
    printf("TEST PASSED: Batch cleared successfully\n");
    
    // Verify batch is empty
    if (ipc_batch_get_count(batch) != 0) {
        printf("TEST FAILED: Batch not empty after clear\n");
        ipc_destroy_batch(batch);
        return -1;
    }
    printf("TEST PASSED: Batch confirmed empty\n");
    
    ipc_destroy_batch(batch);
    printf("TEST PASSED: Batch destroyed successfully\n");
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Test IPC subscriber count functionality
 * 
 * @return 0 on success, -1 on failure
 */
static int test_subscriber_count(void) {
    printf("\n=== Testing IPC Subscriber Count ===\n");
    
    // Test subscriber count for non-existent topic
    int count = ipc_get_subscriber_count("nonexistent/topic");
    if (count < 0) {
        printf("TEST FAILED: Failed to get subscriber count for non-existent topic\n");
        return -1;
    }
    printf("TEST PASSED: Subscriber count for non-existent topic: %d\n", count);
    
    // Test topic has subscribers
    int has_subscribers = ipc_topic_has_subscribers("system/logging/info");
    if (has_subscribers < 0) {
        printf("TEST FAILED: Failed to check if topic has subscribers\n");
        return -1;
    }
    printf("TEST PASSED: Topic has subscribers: %s\n", has_subscribers ? "yes" : "no");
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Test error handling and safety checks
 * 
 * @return 0 on success, -1 on failure
 */
static int test_error_handling(void) {
    printf("\n=== Testing Error Handling and Safety Checks ===\n");
    
    // Test null pointer handling in publish
    if (ipc_publish(NULL, "test", 4) != -1) {
        printf("TEST FAILED: Null publisher not rejected\n");
        return -1;
    }
    printf("TEST PASSED: Null publisher correctly rejected\n");
    
    // Test null message handling
    ipc_publisher_t* publisher = ipc_create_publisher("system/logging/info");
    if (publisher != NULL) {
        if (ipc_publish(publisher, NULL, 4) != -1) {
            printf("TEST FAILED: Null message not rejected\n");
            ipc_destroy_publisher(publisher);
            return -1;
        }
        printf("TEST PASSED: Null message correctly rejected\n");
        
        // Test zero size message
        if (ipc_publish(publisher, "test", 0) != -1) {
            printf("TEST FAILED: Zero size message not rejected\n");
            ipc_destroy_publisher(publisher);
            return -1;
        }
        printf("TEST PASSED: Zero size message correctly rejected\n");
        
        ipc_destroy_publisher(publisher);
    }
    
    g_test_passed++;
    return 0;
}

/**
 * @brief Main test function
 * 
 * @return int Exit status (0 = success, -1 = failure)
 */
int main(void) {
    printf("=== Starting AutoMLOS IPC Library Test Suite ===\n");
    
    // Initialize test counters
    g_received_messages = 0;
    g_test_passed = 0;
    g_test_failed = 0;
    
    // Run all test suites
    int overall_result = 0;
    
    if (test_topic_validation() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    if (test_publisher_creation() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    if (test_subscriber_creation() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    if (test_batch_messaging() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    if (test_subscriber_count() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    if (test_error_handling() != 0) {
        overall_result = -1;
        g_test_failed++;
    }
    
    // Print test summary
    printf("\n=== Test Suite Summary ===\n");
    printf("Tests passed: %u\n", g_test_passed);
    printf("Tests failed: %u\n", g_test_failed);
    printf("Total messages received: %u\n", g_received_messages);
    
    if (overall_result == 0) {
        printf("=== ALL TESTS PASSED ===\n");
    } else {
        printf("=== SOME TESTS FAILED ===\n");
    }
    
    return overall_result;
}