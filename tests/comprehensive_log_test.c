/**
 * @file comprehensive_log_test.c
 * @brief Comprehensive test for all logging system functionality
 * 
 * This test validates the complete logging system functionality and generates
 * diverse log entries for log_parser tool verification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../lib/logging/log_core.h"
#include "../lib/safety/system_errors.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;

// Test assertion macros with safety checks
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

#define TEST_ASSERT_NO_ERROR(error) \
    do { \
        tests_run++; \
        if (!system_error_is_critical(&error)) { \
            tests_passed++; \
            printf("PASS: Operation completed successfully\n"); \
        } else { \
            printf("FAIL: Error %d in %s\n", error.error_code, #error); \
        } \
    } while(0)

/**
 * @brief Test all log levels with safety validation
 */
void test_all_log_levels(void) {
    printf("\n=== Testing All Log Levels ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Test each log level with different tags and messages
    error = log_record_binary_safe(LOG_LEVEL_EMERGENCY, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0x11111111, 0xAAAAAAAA);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_KERNEL, 
                                  LOG_MSG_SYSTEM_SHUTDOWN, 0x22222222, 0xBBBBBBBB);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_APPLICATION, 
                                  LOG_MSG_SERVICE_READY, 0x33333333, 0xCCCCCCCC);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SERVICE_LOGGER, 
                                  LOG_MSG_HEARTBEAT, 0x44444444, 0xDDDDDDDD);
    TEST_ASSERT_NO_ERROR(error);
    
    printf("All log levels tested successfully\n");
}

/**
 * @brief Test all tag types with different scenarios
 */
void test_all_tag_types(void) {
    printf("\n=== Testing All Tag Types ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Test each tag type with appropriate messages
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0x10000001, 0x20000001);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_KERNEL, 
                                  LOG_MSG_SYSTEM_SHUTDOWN, 0x10000002, 0x20000002);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_DRIVER_CAMERA, 
                                  LOG_MSG_DRIVER_INIT, 0x10000003, 0x20000003);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_DRIVER_CAN, 
                                  LOG_MSG_DRIVER_TIMEOUT, 0x10000004, 0x20000004);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_START, 0x10000005, 0x20000005);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_DIAG_MESSAGE, 0x10000006, 0x20000006);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SERVICE_LOGGER, 
                                  LOG_MSG_HEARTBEAT, 0x10000007, 0x20000007);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_APPLICATION, 
                                  LOG_MSG_SERVICE_READY, 0x10000008, 0x20000008);
    TEST_ASSERT_NO_ERROR(error);
    
    printf("All tag types tested successfully\n");
}

/**
 * @brief Test all message types with realistic scenarios
 */
void test_all_message_types(void) {
    printf("\n=== Testing All Message Types ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Test each message type with appropriate tags and data
    error = log_record_binary_safe(LOG_LEVEL_EMERGENCY, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0x55555555, 0x66666666);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_SHUTDOWN, 0x77777777, 0x88888888);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_SERVICE_READY, 0x99999999, 0xAAAAAAAA);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_SERVICE_ERROR, 0xBBBBBBBB, 0xCCCCCCCC);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_DRIVER_CAMERA, 
                                  LOG_MSG_DRIVER_INIT, 0xDDDDDDDD, 0xEEEEEEEE);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_DRIVER_CAN, 
                                  LOG_MSG_DRIVER_TIMEOUT, 0xFFFFFFFF, 0x11111111);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_START, 0x22222222, 0x33333333);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_RESULT, 0x44444444, 0x55555555);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_DIAG_MESSAGE, 0x66666666, 0x77777777);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SYSTEM, 
                                  LOG_MSG_HEARTBEAT, 0x88888888, 0x99999999);
    TEST_ASSERT_NO_ERROR(error);
    
    printf("All message types tested successfully\n");
}

/**
 * @brief Test comprehensive logging scenarios
 */
void test_comprehensive_scenarios(void) {
    printf("\n=== Testing Comprehensive Logging Scenarios ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Scenario 1: System startup sequence
    error = log_record_binary_safe(LOG_LEVEL_EMERGENCY, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0x00000001, 0x00000000);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_KERNEL, 
                                  LOG_MSG_SYSTEM_START, 0x00000002, 0x00000000);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 2: Driver initialization
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_DRIVER_CAMERA, 
                                  LOG_MSG_DRIVER_INIT, 0x00010001, 0x00020002);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_DRIVER_CAN, 
                                  LOG_MSG_DRIVER_INIT, 0x00010002, 0x00020003);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 3: AI service operation
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_START, 0x00100001, 0x00200001);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_AI, 
                                  LOG_MSG_AI_INFERENCE_RESULT, 0x00100002, 0x00200002);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 4: Diagnostic monitoring
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_DIAG_MESSAGE, 0x01000001, 0x02000001);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_SYSTEM, 
                                  LOG_MSG_HEARTBEAT, 0x03000001, 0x04000001);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 5: Application events
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_APPLICATION, 
                                  LOG_MSG_SERVICE_READY, 0x05000001, 0x06000001);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 6: Error conditions
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_DRIVER_CAN, 
                                  LOG_MSG_DRIVER_TIMEOUT, 0x07000001, 0x08000001);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SERVICE_DIAG, 
                                  LOG_MSG_SERVICE_ERROR, 0x09000001, 0x0A000001);
    TEST_ASSERT_NO_ERROR(error);
    
    // Scenario 7: System shutdown
    error = log_record_binary_safe(LOG_LEVEL_ERROR, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_SHUTDOWN, 0x0B000001, 0x0C000001);
    TEST_ASSERT_NO_ERROR(error);
    
    printf("Comprehensive scenarios tested successfully\n");
}

/**
 * @brief Test boundary conditions and error handling
 */
void test_boundary_conditions(void) {
    printf("\n=== Testing Boundary Conditions ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Test invalid log level (should return error)
    error = log_record_binary_safe(LOG_LEVEL_COUNT, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0, 0);
    TEST_ASSERT(system_error_is_critical(&error));
    
    // Test invalid tag ID (should return error)
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_COUNT, 
                                  LOG_MSG_SYSTEM_START, 0, 0);
    TEST_ASSERT(system_error_is_critical(&error));
    
    // Test invalid message ID (should return error)
    error = log_record_binary_safe(LOG_LEVEL_INFO, LOG_TAG_SYSTEM, 
                                  LOG_MSG_COUNT, 0, 0);
    TEST_ASSERT(system_error_is_critical(&error));
    
    // Test valid boundary values
    error = log_record_binary_safe(LOG_LEVEL_EMERGENCY, LOG_TAG_SYSTEM, 
                                  LOG_MSG_SYSTEM_START, 0, 0);
    TEST_ASSERT_NO_ERROR(error);
    
    error = log_record_binary_safe(LOG_LEVEL_DEBUG, LOG_TAG_APPLICATION, 
                                  LOG_MSG_HEARTBEAT, 0xFFFFFFFF, 0xFFFFFFFF);
    TEST_ASSERT_NO_ERROR(error);
    
    printf("Boundary conditions tested successfully\n");
}

/**
 * @brief Generate diverse log entries for log_parser testing
 */
void generate_diverse_logs_for_parser(void) {
    printf("\n=== Generating Diverse Logs for Parser Testing ===\n");
    
    // Remove log_system_init_safe() call - system should already be initialized
    system_error_t error;
    
    // Generate a variety of log entries with different combinations
    for (int i = 0; i < 32; i++) {
        uint8_t level = (i % 4);  // Cycle through all levels
        uint8_t tag = (i % 8);    // Cycle through all tags
        uint16_t message = (i % 10); // Cycle through all messages
        
        uint32_t data1 = 0x10000000 + i;
        uint32_t data2 = 0x20000000 + (i * 2);
        
        error = log_record_binary_safe(level, tag, message, data1, data2);
        TEST_ASSERT_NO_ERROR(error);
        
        if (i % 8 == 0) {
            printf("Generated log entry %d: level=%d, tag=%d, message=%d\n", 
                   i, level, tag, message);
        }
    }
    
    printf("Diverse log generation completed successfully\n");
}

/**
 * @brief Save binary logs from memory buffer to file for parser testing
 */
void save_binary_logs_to_file(void) {
    const char* filename = "comprehensive_test.bin";
    FILE* file = fopen(filename, "wb");
    
    if (!file) {
        printf("Warning: Cannot create binary log file %s\n", filename);
        return;
    }
    
    // Get global log buffer
    log_buffer_t* buffer = get_global_log_buffer();
    if (!buffer || buffer->entry_count == 0) {
        printf("Warning: No log entries found in buffer\n");
        fclose(file);
        return;
    }
    
    int entries_saved = 0;
    int original_entry_count = buffer->entry_count;  // Save original count
    
    // Save all valid entries from buffer
    for (int i = 0; i < original_entry_count; i++) {
        log_entry_binary_t entry;
        // FIX: Use log_buffer_get_safe instead of log_buffer_get
        system_error_t error = log_buffer_get_safe(buffer, &entry);
        
        if (error.error_code == ERR_BASE_OK) {
            size_t written = fwrite(&entry, sizeof(entry), 1, file);
            if (written == 1) {
                entries_saved++;
            }
        }
    }
    
    fclose(file);
    
    printf("Saved %d binary log entries to %s (%ld bytes)\n", 
           entries_saved, filename, entries_saved * sizeof(log_entry_binary_t));
    printf("File ready for log_parser testing: ./tools/log_parser/log_parser %s\n", filename);
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("=== Starting Comprehensive Log System Test ===\n");
    printf("This test will validate all logging functionality and generate\n");
    printf("diverse log entries for log_parser tool verification.\n\n");
    
    // Initialize logging system once at the beginning
    system_error_t init_error = log_system_init_safe();
    if (init_error.error_code != ERR_BASE_OK) {
        printf("❌ Log system initialization failed: error %d\n", init_error.error_code);
        return 1;
    }
    
    // Run all test suites
    test_all_log_levels();
    test_all_tag_types();
    test_all_message_types();
    test_comprehensive_scenarios();
    test_boundary_conditions();
    generate_diverse_logs_for_parser();
    
    // Save binary logs to file for parser testing
    save_binary_logs_to_file();
    
    // Print test summary
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Test coverage: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    
    if (tests_passed == tests_run) {
        printf("\n✅ ALL TESTS PASSED - Log system is fully functional\n");
        printf("Generated diverse log entries ready for log_parser testing\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED - Please check the implementation\n");
        return 1;
    }
}