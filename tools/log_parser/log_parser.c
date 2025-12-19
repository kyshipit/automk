/**
 * @file log_parser.c
 * @brief Binary log offline parser implementation
 * 
 * Implements safety-validated parsing of binary log files
 * with comprehensive error detection and reporting.
 */

#include "log_parser.h"
#include <string.h>
#include <stdlib.h>

// Global databases
static format_string_entry_t* g_format_db = NULL;
static tag_name_entry_t* g_tag_db = NULL;
static uint32_t g_format_db_size = 0;
static uint32_t g_tag_db_size = 0;
static parser_stats_t g_parser_stats = {0};

/**
 * @brief Initialize parser with format and tag databases
 * 
 * @param format_db_path Path to format string database
 * @param tag_db_path Path to tag name database
 * @return 0 on success, -1 on error
 */
int log_parser_init(const char* format_db_path, const char* tag_db_path) {
    // Reset statistics
    memset(&g_parser_stats, 0, sizeof(g_parser_stats));
    
    // TODO: Load format database from file
    // For now, initialize with hardcoded values for demonstration
    g_format_db_size = 10;
    g_format_db = malloc(g_format_db_size * sizeof(format_string_entry_t));
    
    if (g_format_db) {
        // System messages
        g_format_db[0] = (format_string_entry_t){LOG_MSG_SYSTEM_START, "System started at timestamp %u", "System startup message"};
        g_format_db[1] = (format_string_entry_t){LOG_MSG_SYSTEM_SHUTDOWN, "System shutdown at timestamp %u", "System shutdown message"};
        g_format_db[2] = (format_string_entry_t){LOG_MSG_SERVICE_READY, "Service %u ready", "Service initialization complete message"};
        
        // Error messages  
        g_format_db[3] = (format_string_entry_t){LOG_MSG_SERVICE_ERROR, "Service %u error: code %u", "Service error message"};
        g_format_db[4] = (format_string_entry_t){LOG_MSG_DRIVER_TIMEOUT, "Driver %u timeout after %u ms", "Driver timeout message"};
        
        // AI service messages
        g_format_db[5] = (format_string_entry_t){LOG_MSG_AI_INFERENCE_START, "AI inference started for model %u", "AI inference start message"};
        g_format_db[6] = (format_string_entry_t){LOG_MSG_AI_INFERENCE_RESULT, "AI inference result: confidence %u%%, latency %u ms", "AI inference result message"};
    }
    
    // TODO: Load tag database from file
    g_tag_db_size = 8;
    g_tag_db = malloc(g_tag_db_size * sizeof(tag_name_entry_t));
    
    if (g_tag_db) {
        g_tag_db[0] = (tag_name_entry_t){LOG_TAG_SYSTEM, "SYSTEM"};
        g_tag_db[1] = (tag_name_entry_t){LOG_TAG_KERNEL, "KERNEL"};
        g_tag_db[2] = (tag_name_entry_t){LOG_TAG_DRIVER_CAMERA, "DRIVER_CAMERA"};
        g_tag_db[3] = (tag_name_entry_t){LOG_TAG_DRIVER_CAN, "DRIVER_CAN"};
        g_tag_db[4] = (tag_name_entry_t){LOG_TAG_SERVICE_AI, "SERVICE_AI"};
        g_tag_db[5] = (tag_name_entry_t){LOG_TAG_SERVICE_DIAG, "SERVICE_DIAG"};
        g_tag_db[6] = (tag_name_entry_t){LOG_TAG_SERVICE_LOGGER, "SERVICE_LOGGER"};
        g_tag_db[7] = (tag_name_entry_t){LOG_TAG_APPLICATION, "APPLICATION"};
    }
    
    return 0;
}

/**
 * @brief Validate binary log entry integrity
 * 
 * @param entry_data Pointer to entry data
 * @param entry_size Entry size in bytes
 * @return 0 if valid, -1 if corrupted
 */
int log_parser_validate_entry(const uint8_t* entry_data, size_t entry_size) {
    if (entry_data == NULL || entry_size != sizeof(log_entry_binary_t)) {
        return -1;
    }
    
    const log_entry_binary_t* entry = (const log_entry_binary_t*)entry_data;
    
    // Verify checksum
    uint16_t calculated_checksum = 0;
    const uint8_t* data = (const uint8_t*)entry;
    size_t data_size = sizeof(*entry) - sizeof(entry->checksum);
    
    for (size_t i = 0; i < data_size; i++) {
        calculated_checksum += data[i];
    }
    
    if (calculated_checksum != entry->checksum) {
        g_parser_stats.corrupted_entries++;
        return -1;
    }
    
    // Verify field ranges
    if (entry->level >= LOG_LEVEL_COUNT) {
        return -1;
    }
    
    if (entry->tag_id >= LOG_TAG_COUNT) {
        g_parser_stats.unknown_messages++;
    }
    
    return 0;
}

/**
 * @brief Get tag name from tag ID
 * 
 * @param tag_id Tag identifier
 * @return Tag name string, "UNKNOWN" if not found
 */
const char* log_parser_get_tag_name(uint8_t tag_id) {
    for (uint32_t i = 0; i < g_tag_db_size; i++) {
        if (g_tag_db[i].tag_id == tag_id) {
            return g_tag_db[i].tag_name;
        }
    }
    return "UNKNOWN";
}

/**
 * @brief Get message text from message ID
 * 
 * @param message_id Message identifier
 * @return Message format string, NULL if not found
 */
const char* log_parser_get_message_text(uint16_t message_id) {
    for (uint32_t i = 0; i < g_format_db_size; i++) {
        if (g_format_db[i].message_id == message_id) {
            return g_format_db[i].format_string;
        }
    }
    return NULL;
}

/**
 * @brief Process binary log file and convert to output format
 * 
 * @param input_file Path to input binary log file
 * @param output_file Path to output file
 * @param format Output format
 * @return 0 on success, -1 on error
 */
int log_parser_process_file(const char* input_file, const char* output_file, output_format_t format) {
    FILE* input = fopen(input_file, "rb");
    FILE* output = fopen(output_file, "w");
    
    if (!input || !output) {
        if (input) fclose(input);
        if (output) fclose(output);
        return -1;
    }
    
    // Process each log entry
    log_entry_binary_t entry;
    while (fread(&entry, sizeof(entry), 1, input) == 1) {
        g_parser_stats.total_entries_parsed++;
        
        // Validate entry
        if (log_parser_validate_entry((const uint8_t*)&entry, sizeof(entry)) == 0) {
            g_parser_stats.valid_entries++;
            
            // Convert to output format
            const char* tag_name = log_parser_get_tag_name(entry.tag_id);
            const char* message_format = log_parser_get_message_text(entry.message_id);
            
            if (format == OUTPUT_FORMAT_TEXT) {
                if (message_format) {
                    fprintf(output, "[%u] %s: ", entry.timestamp, tag_name);
                    fprintf(output, message_format, entry.data[0], entry.data[1]);
                    fprintf(output, "\n");
                } else {
                    fprintf(output, "[%u] %s: Unknown message %u (data: %u, %u)\n",
                           entry.timestamp, tag_name, entry.message_id, 
                           entry.data[0], entry.data[1]);
                }
            }
            // TODO: Implement other output formats
        }
    }
    
    fclose(input);
    fclose(output);
    return 0;
}

/**
 * @brief Get parser statistics
 * 
 * @param stats Pointer to statistics structure
 */
void log_parser_get_stats(parser_stats_t* stats) {
    if (stats) {
        *stats = g_parser_stats;
    }
}