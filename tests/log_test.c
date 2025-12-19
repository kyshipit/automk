/**
 * @file log_test.c
 * @brief Independent test framework for binary logging system
 * 
 * This test framework validates the logging system without requiring
 * hardware abstraction layer (timestamp and PID).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../lib/logging/log_core.h"
#include "../../lib/safety/system_errors.h"  // 添加系统错误处理

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Test assertion macros
#define TEST_ASSERT(condition) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("PASS: %s\n", #condition); \
        } else { \
            printf("FAIL: %s\n", #condition); \
        } \
    } while(0)

/**
 * @brief Test buffer initialization
 */
void test_buffer_init(void) {
    system_error_t error = log_system_init_safe();  // 使用安全版本
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Verify global buffer is properly initialized
    log_buffer_t* buffer = get_global_log_buffer();
    TEST_ASSERT(buffer != NULL);
    TEST_ASSERT(buffer->buffer_size == LOG_BUFFER_CAPACITY);
    TEST_ASSERT(buffer->entry_count == 0);
    TEST_ASSERT(buffer->write_index == 0);
    TEST_ASSERT(buffer->read_index == 0);
}

/**
 * @brief Test single log entry creation and storage
 */
void test_log_entry_creation(void) {
    system_error_t error = log_system_init_safe();
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Record a test log entry using safe version
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0x12345678, 0xABCDEF00);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Verify entry was stored using safe version
    log_entry_binary_t entry;
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    TEST_ASSERT(entry.level == LOG_LEVEL_ERROR);
    TEST_ASSERT(entry.tag_id == LOG_TAG_SYSTEM);
    TEST_ASSERT(entry.message_id == LOG_MSG_SYSTEM_START);
    TEST_ASSERT(entry.data[0] == 0x12345678);
    TEST_ASSERT(entry.data[1] == 0xABCDEF00);
}

/**
 * @brief Test buffer overflow behavior
 */
void test_buffer_overflow(void) {
    system_error_t error = log_system_init_safe();
    TEST_ASSERT(!system_error_is_critical(&error));
    
    log_buffer_t* buffer = get_global_log_buffer();
    
    // Fill buffer beyond capacity using safe version
    for (size_t i = 0; i < LOG_BUFFER_CAPACITY + 10; i++) {
        error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SYSTEM, 
                                      LOG_MSG_HEARTBEAT, i, i+1);
        TEST_ASSERT(!system_error_is_critical(&error));
    }
    
    // Verify buffer maintains correct size
    TEST_ASSERT(buffer->entry_count <= LOG_BUFFER_CAPACITY);
    
    // Verify we can still retrieve entries using safe version
    log_entry_binary_t entry;
    error = log_buffer_get_safe(buffer, &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
}

/**
 * @brief Test checksum calculation and verification
 */
void test_checksum_verification(void) {
    system_error_t error = log_system_init_safe();
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Record a test entry using safe version
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_DRIVER_CAMERA, 
                                  LOG_MSG_DRIVER_INIT, 0xA5A5A5A5, 0x5A5A5A5A);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Retrieve the entry using safe version
    log_entry_binary_t entry;
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Verify checksum
    uint16_t calculated_checksum = log_calculate_checksum(&entry);
    TEST_ASSERT(calculated_checksum == entry.checksum);
    
    // Corrupt data and verify checksum failure
    entry.data[0] ^= 0xFFFFFFFF;  // Flip all bits
    uint16_t corrupted_checksum = log_calculate_checksum(&entry);
    TEST_ASSERT(corrupted_checksum != entry.checksum);
}

/**
 * @brief Test different log levels
 */
void test_log_levels(void) {
    system_error_t error = log_system_init_safe();
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Test all log levels using safe version
    error = log_record_binary_safe(LOG_LEVEL_EMERGENCY, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_START, 1, 2);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_DIAG_MESSAGE, 3, 4);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_APPLICATION, 
                                  LOG_MSG_SERVICE_READY, 5, 6);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_KERNEL, 
                                  LOG_MSG_SYSTEM_START, 7, 8);
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Verify entries are stored with correct levels
    log_entry_binary_t entry;
    
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    TEST_ASSERT(entry.level == LOG_LEVEL_EMERGENCY);
    
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    TEST_ASSERT(entry.level == LOG_LEVEL_ERROR);
    
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    TEST_ASSERT(entry.level == LOG_LEVEL_INFO);
    
    error = log_buffer_get_safe(get_global_log_buffer(), &entry);
    TEST_ASSERT(!system_error_is_critical(&error));
    TEST_ASSERT(entry.level == LOG_LEVEL_DEBUG);
}

/**
 * @brief Test safety error handling
 */
void test_safety_error_handling(void) {
    // Test NULL pointer safety - should return critical error
    system_error_t error = log_buffer_get_safe(NULL, NULL);
    TEST_ASSERT(system_error_is_critical(&error));
    
    // Test uninitialized system - should return critical error
    log_buffer_t buffer;
    log_entry_binary_t entry;
    error = log_buffer_get_safe(&buffer, &entry);
    TEST_ASSERT(system_error_is_critical(&error));
    
    printf("Safety error handling test passed\n");
}

/**
 * @brief Test error boundary conditions
 */
void test_error_boundary_conditions(void) {
    system_error_t error = log_system_init_safe();
    TEST_ASSERT(!system_error_is_critical(&error));
    
    // Test invalid log level - should return critical error
    error = log_record_binary_safe(LOG_LEVEL_COUNT, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0, 0);
    TEST_ASSERT(system_error_is_critical(&error));
    
    // Test invalid tag ID - should return critical error
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_COUNT, 
                                  LOG_MSG_SYSTEM_START, 0, 0);
    TEST_ASSERT(system_error_is_critical(&error));
    
    printf("Error boundary conditions test passed\n");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("=== AutoMLOS Logging System Test Suite ===\n");
    printf("Testing core functionality with safety features\n\n");
    
    test_buffer_init();
    test_log_entry_creation();
    test_buffer_overflow();
    test_checksum_verification();
    test_log_levels();
    test_safety_error_handling();      // 新增
    test_error_boundary_conditions();  // 新增
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100);
    
    return (tests_passed == tests_run) ? 0 : 1;
}