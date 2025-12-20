/**
 * @file binary_log_generator.c
 * @brief Binary log file generator for log_parser testing
 * 
 * This program generates binary log files in the exact format expected
 * by the log_parser tool for comprehensive testing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../lib/logging/log_common.h"
#include "../lib/logging/log_core.h"

/**
 * @brief Generate binary log file with diverse entries
 * 
 * Creates a binary log file containing various log entries with
 * proper checksums and valid data for log_parser testing.
 */
int main(void) {
    const char* output_file = "binary_test.log";
    FILE* file = fopen(output_file, "wb");
    
    if (!file) {
        printf("Error: Cannot create output file %s\n", output_file);
        return 1;
    }
    
    printf("=== Generating Binary Log File for Parser Testing ===\n");
    printf("Output file: %s\n", output_file);
    
    // Initialize logging system
    system_error_t error = log_system_init_safe();
    if (error.error_code != ERR_BASE_OK) {
        printf("Error: Failed to initialize logging system\n");
        fclose(file);
        return 1;
    }
    
    int entries_generated = 0;
    
    // Generate diverse log entries
    for (int i = 0; i < 100; i++) {
        // Create log entry with current timestamp and process ID
        log_entry_binary_t entry;
        
        // Use different combinations for diversity
        uint8_t level = (i % 4);      // Cycle through all levels
        uint8_t tag = (i % 8);        // Cycle through all tags  
        uint16_t message = (i % 10);  // Cycle through all messages
        
        uint32_t data1 = 0x10000000 + i;
        uint32_t data2 = 0x20000000 + (i * 2);
        
        // Create entry with safety validation
        error = log_create_entry(&entry, level, tag, message, data1, data2);
        if (error.error_code != ERR_BASE_OK) {
            printf("Warning: Failed to create entry %d\n", i);
            continue;
        }
        
        // Write binary entry to file
        size_t written = fwrite(&entry, sizeof(entry), 1, file);
        if (written != 1) {
            printf("Error: Failed to write entry %d to file\n", i);
            break;
        }
        
        entries_generated++;
        
        if (i % 20 == 0) {
            printf("Generated entry %d: level=%d, tag=%d, message=%d\n", 
                   i, level, tag, message);
        }
    }
    
    fclose(file);
    
    printf("\n=== Generation Complete ===\n");
    printf("Total entries generated: %d\n", entries_generated);
    printf("File size: %ld bytes\n", entries_generated * sizeof(log_entry_binary_t));
    printf("Binary log file ready for parser testing: %s\n", output_file);
    
    return 0;
}