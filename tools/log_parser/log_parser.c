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

// Color mode configuration
static color_mode_t g_color_mode = COLOR_MODE_MINIMAL;

#include <stdio.h>
#include <ctype.h>

/**
 * @brief Load format string database from file
 * 
 * @param format_db_path Path to format database file
 * @return 0 on success, -1 on error
 */
int load_format_database(const char* format_db_path) {
    FILE* file = fopen(format_db_path, "r");
    if (!file) {
        fprintf(stderr, "Warning: Cannot open format database file: %s\n", format_db_path);
        return -1;
    }
    
    // Count lines in file
    uint32_t line_count = 0;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != '#' && strlen(buffer) > 1) {
            line_count++;
        }
    }
    rewind(file);
    
    if (line_count == 0) {
        fclose(file);
        return -1;
    }
    
    // Allocate memory for database
    g_format_db = malloc(line_count * sizeof(format_string_entry_t));
    if (!g_format_db) {
        fclose(file);
        return -1;
    }
    
    // Parse file
    uint32_t index = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        // Skip comments and empty lines
        if (buffer[0] == '#' || strlen(buffer) <= 1) {
            continue;
        }
        
        // Parse line: message_id|format_string|description|message_type
        uint16_t message_id;
        char format_string[MAX_FORMAT_STRING_LENGTH] = {0};
        char description[MAX_MESSAGE_TEXT_LENGTH] = {0};
        int message_type = 0;
        
        if (sscanf(buffer, "%hu|%255[^|]|%127[^|]|%d", 
                   &message_id, format_string, description, &message_type) >= 3) {
            g_format_db[index].message_id = message_id;
            // Safe string copy with explicit null termination
            strncpy(g_format_db[index].format_string, format_string, MAX_FORMAT_STRING_LENGTH - 1);
            g_format_db[index].format_string[MAX_FORMAT_STRING_LENGTH - 1] = '\0';
            strncpy(g_format_db[index].description, description, MAX_MESSAGE_TEXT_LENGTH - 1);
            g_format_db[index].description[MAX_MESSAGE_TEXT_LENGTH - 1] = '\0';
            g_format_db[index].message_type = (message_type_t)message_type;
            index++;
        }
    }
    
    g_format_db_size = index;
    fclose(file);
    return 0;
}

/**
 * @brief Load tag database from file
 * 
 * @param tag_db_path Path to tag database file
 * @return 0 on success, -1 on error
 */
int load_tag_database(const char* tag_db_path) {
    FILE* file = fopen(tag_db_path, "r");
    if (!file) {
        fprintf(stderr, "Warning: Cannot open tag database file: %s\n", tag_db_path);
        return -1;
    }
    
    // Count lines in file
    uint32_t line_count = 0;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] != '#' && strlen(buffer) > 1) {
            line_count++;
        }
    }
    rewind(file);
    
    if (line_count == 0) {
        fclose(file);
        return -1;
    }
    
    // Allocate memory for database
    g_tag_db = malloc(line_count * sizeof(tag_name_entry_t));
    if (!g_tag_db) {
        fclose(file);
        return -1;
    }
    
    // Parse file
    uint32_t index = 0;
    while (fgets(buffer, sizeof(buffer), file)) {
        // Skip comments and empty lines
        if (buffer[0] == '#' || strlen(buffer) <= 1) {
            continue;
        }
        
        // Parse line: tag_id|tag_name
        uint8_t tag_id;
        char tag_name[MAX_TAG_NAME_LENGTH] = {0};
        
        if (sscanf(buffer, "%hhu|%63[^\n]", &tag_id, tag_name) == 2) {
            g_tag_db[index].tag_id = tag_id;
            // Safe string copy with explicit null termination
            strncpy(g_tag_db[index].tag_name, tag_name, MAX_TAG_NAME_LENGTH - 1);
            g_tag_db[index].tag_name[MAX_TAG_NAME_LENGTH - 1] = '\0';
            index++;
        }
    }
    
    g_tag_db_size = index;
    fclose(file);
    return 0;
}

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
    
    // Try to load databases from files
    int format_loaded = load_format_database(format_db_path);
    int tag_loaded = load_tag_database(tag_db_path);
    
    // Remove hardcoded fallback - if database files cannot be loaded, fail immediately
    if (format_loaded != 0) {
        fprintf(stderr, "Error: Cannot load format database from: %s\n", format_db_path);
        fprintf(stderr, "Please ensure the database file exists in the tool directory\n");
        return -1;
    }
    
    if (tag_loaded != 0) {
        fprintf(stderr, "Error: Cannot load tag database from: %s\n", tag_db_path);
        fprintf(stderr, "Please ensure the database file exists in the tool directory\n");
        return -1;
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
/**
 * @brief Get log level name string
 * 
 * @param level Log level
 * @return Level name string
 */
const char* log_parser_get_level_name(log_level_t level) {
    static const char* level_names[] = {
        "EMERGENCY", "ERROR", "INFO", "DEBUG"
    };
    
    if (level < LOG_LEVEL_COUNT) {
        return level_names[level];
    }
    return "UNKNOWN";
}

/**
 * @brief Detect message type for smart formatting
 * 
 * @param message_id Message ID (unused parameter marked with UNUSED macro)
 * @param message_format Message format string
 * @return Detected message type
 */
message_type_t log_parser_detect_message_type(uint16_t message_id, const char* message_format) {
    UNUSED(message_id);  // Mark as unused to avoid compiler warning
    
    // Detect CAN messages
    if (message_format && (strstr(message_format, "CAN") || strstr(message_format, "ID:") || 
                          strstr(message_format, "DLC:") || strstr(message_format, "frame"))) {
        return MESSAGE_TYPE_CAN;
    }
    
    // Detect UDS messages
    if (message_format && (strstr(message_format, "UDS") || strstr(message_format, "SID:") || 
                          strstr(message_format, "DID:") || strstr(message_format, "diagnostic"))) {
        return MESSAGE_TYPE_UDS;
    }
    
    // Detect kernel messages
    if (message_format && (strstr(message_format, "kernel") || strstr(message_format, "memory") || 
                          strstr(message_format, "scheduler") || strstr(message_format, "interrupt"))) {
        return MESSAGE_TYPE_KERNEL;
    }
    
    // Detect security messages
    if (message_format && (strstr(message_format, "security") || strstr(message_format, "permission") || 
                          strstr(message_format, "access") || strstr(message_format, "denied"))) {
        return MESSAGE_TYPE_SECURITY;
    }
    
    return MESSAGE_TYPE_GENERAL;
}

/**
 * @brief Extract context information from message
 * 
 * @param message_format Message format string
 * @param data Message data (use local copy to avoid alignment issues)
 * @param context Output context buffer
 * @param context_size Context buffer size
 */
void log_parser_extract_context(const char* message_format, const uint32_t* data,
                               char* context, size_t context_size) {
    if (!message_format || !context || context_size == 0) {
        if (context && context_size > 0) {
            context[0] = '\0';
        }
        return;
    }
    
    // Create local copy of data to avoid alignment issues
    uint32_t data_copy[2] = {data[0], data[1]};
    
    // Extract session ID
    if (strstr(message_format, "session")) {
        snprintf(context, context_size, "Session:%u", data_copy[0]);
        return;
    }
    
    // Extract process/thread ID
    if (strstr(message_format, "process") || strstr(message_format, "thread")) {
        snprintf(context, context_size, "Process:%u", data_copy[0]);
        return;
    }
    
    // Extract user/device ID
    if (strstr(message_format, "user") || strstr(message_format, "device")) {
        snprintf(context, context_size, "User:%u", data_copy[0]);
        return;
    }
    
    // Default: no specific context
    context[0] = '\0';
}

/**
 * @brief Extract key data for smart formatting
 * 
 * @param message_format Message format string
 * @param data Message data (use local copy to avoid alignment issues)
 * @param key_data Output key data buffer
 * @param key_data_size Key data buffer size
 */
void log_parser_extract_key_data(const char* message_format, const uint32_t* data, 
                                char* key_data, size_t key_data_size) {
    if (!message_format || !key_data || key_data_size == 0) {
        if (key_data && key_data_size > 0) {
            key_data[0] = '\0';
        }
        return;
    }
    
    // Create local copy of data to avoid alignment issues
    uint32_t data_copy[2] = {data[0], data[1]};
    
    // CAN message: extract ID and DLC
    if (strstr(message_format, "CAN") || strstr(message_format, "ID:")) {
        snprintf(key_data, key_data_size, "ID:0x%X DLC:%u", data_copy[0], data_copy[1]);
        return;
    }
    
    // UDS message: extract SID and result
    if (strstr(message_format, "UDS") || strstr(message_format, "SID:")) {
        snprintf(key_data, key_data_size, "SID:0x%X Result:%s", data_copy[0], 
                data_copy[1] == 0 ? "Success" : "Failure");
        return;
    }
    
    // Error message: extract error code
    if (strstr(message_format, "error") || strstr(message_format, "code")) {
        snprintf(key_data, key_data_size, "ErrorCode:%u", data_copy[0]);
        return;
    }
    
    // Performance message: extract metrics
    if (strstr(message_format, "latency") || strstr(message_format, "time")) {
        snprintf(key_data, key_data_size, "Latency:%ums", data_copy[0]);
        return;
    }
    
    // Default: format both data values
    if (data_copy[0] != 0 || data_copy[1] != 0) {
        snprintf(key_data, key_data_size, "Data:%u,%u", data_copy[0], data_copy[1]);
    } else {
        key_data[0] = '\0';
    }
}

/**
 * @brief Format log entry with smart formatting and minimal colors
 * 
 * @param entry Log entry to format
 * @param output File to write to
 * @param format Output format (unused parameter marked with UNUSED macro)
 * @param color_mode Color mode
 * @param log_format Log format type
 */
void log_parser_format_entry_smart(const log_entry_binary_t* entry, FILE* output, 
                                  output_format_t format, color_mode_t color_mode,
                                  log_format_t log_format) {
    UNUSED(format);  // Mark as unused to avoid compiler warning
    
    if (!entry || !output) return;
    
    const char* tag_name = log_parser_get_tag_name(entry->tag_id);
    const char* message_format = log_parser_get_message_text(entry->message_id);
    const char* level_name = log_parser_get_level_name((log_level_t)entry->level);
    
    // Extract context and key data
    char context[MAX_CONTEXT_LENGTH] = {0};
    char key_data[MAX_KEY_DATA_LENGTH] = {0};
    
    // Use local copy of data to avoid alignment issues
    uint32_t data_copy[2] = {entry->data[0], entry->data[1]};
    log_parser_extract_context(message_format, data_copy, context, sizeof(context));
    log_parser_extract_key_data(message_format, data_copy, key_data, sizeof(key_data));
    
    // Apply color if needed (only for ERROR/EMERGENCY in minimal mode)
    int use_color = 0;
    if (color_mode == COLOR_MODE_MINIMAL && 
        (entry->level == LOG_LEVEL_ERROR || entry->level == LOG_LEVEL_EMERGENCY)) {
        use_color = 1;
        fprintf(output, COLOR_RED);
    }
    
    // Format based on selected format type
    switch (log_format) {
        case FORMAT_SMART:
        case FORMAT_FIXED:
            // Standard format: Timestamp | Level | Module | Context | Message | Key Data
            fprintf(output, "%u | %s | %s", entry->timestamp, level_name, tag_name);
            
            if (context[0] != '\0') {
                fprintf(output, " | %s", context);
            } else {
                fprintf(output, " | -");
            }
            
            if (message_format) {
                char formatted_message[MAX_MESSAGE_TEXT_LENGTH] = {0};
                snprintf(formatted_message, sizeof(formatted_message), 
                        message_format, data_copy[0], data_copy[1]);
                fprintf(output, " | %s", formatted_message);
            } else {
                fprintf(output, " | Unknown message %u", entry->message_id);
            }
            
            if (key_data[0] != '\0') {
                fprintf(output, " | %s", key_data);
            }
            break;
            
        case FORMAT_MINIMAL:
            // Minimal format: only timestamp, level, and message
            if (message_format) {
                char formatted_message[MAX_MESSAGE_TEXT_LENGTH] = {0};
                snprintf(formatted_message, sizeof(formatted_message), 
                        message_format, data_copy[0], data_copy[1]);
                fprintf(output, "%u | %s | %s", entry->timestamp, level_name, formatted_message);
            } else {
                fprintf(output, "%u | %s | Unknown message %u", entry->timestamp, level_name, entry->message_id);
            }
            break;
            
        case FORMAT_VERBOSE:
            // Verbose format: include all available information
            fprintf(output, "Timestamp:%u Level:%s Module:%s Process:%u", 
                   entry->timestamp, level_name, tag_name, entry->process_id);
            
            if (context[0] != '\0') {
                fprintf(output, " Context:%s", context);
            }
            
            if (message_format) {
                char formatted_message[MAX_MESSAGE_TEXT_LENGTH] = {0};
                snprintf(formatted_message, sizeof(formatted_message), 
                        message_format, data_copy[0], data_copy[1]);
                fprintf(output, " Message:%s", formatted_message);
            } else {
                fprintf(output, " Message:Unknown message %u", entry->message_id);
            }
            
            if (key_data[0] != '\0') {
                fprintf(output, " KeyData:%s", key_data);
            }
            break;
    }
    
    // Reset color if used
    if (use_color) {
        fprintf(output, COLOR_RESET);
    }
    
    fprintf(output, "\n");
}

/**
 * @brief Process binary log file with extended options
 * 
 * @param input_file Path to input binary log file
 * @param output_file Path to output file
 * @param format Output format
 * @param color_mode Color mode
 * @param log_format Log format type
 * @param filters Module filters
 * @param filter_count Number of filters
 * @return 0 on success, -1 on error
 */
int log_parser_process_file_ex(const char* input_file, const char* output_file, 
                              output_format_t format, color_mode_t color_mode, 
                              log_format_t log_format, const module_filter_t* filters, 
                              uint32_t filter_count) {
    FILE* input = fopen(input_file, "rb");
    FILE* output = fopen(output_file, "w");
    
    if (!input || !output) {
        if (input) fclose(input);
        if (output) fclose(output);
        return -1;
    }
    
    // Set global color mode
    g_color_mode = color_mode;
    
    // Process each log entry
    log_entry_binary_t entry;
    while (fread(&entry, sizeof(entry), 1, input) == 1) {
        g_parser_stats.total_entries_parsed++;
        
        // Validate entry
        if (log_parser_validate_entry((const uint8_t*)&entry, sizeof(entry)) == 0) {
            g_parser_stats.valid_entries++;
            
            // Apply module filtering if specified
            int should_output = 1;
            if (filters && filter_count > 0) {
                should_output = 0;
                for (uint32_t i = 0; i < filter_count; i++) {
                    if (filters[i].tag_id == entry.tag_id && 
                        entry.level >= filters[i].min_level) {
                        should_output = 1;
                        break;
                    }
                }
            }
            
            if (should_output) {
                // Convert to output format
                if (format == OUTPUT_FORMAT_TEXT) {
                    log_parser_format_entry_smart(&entry, output, format, color_mode, log_format);
                } else {
                    // TODO: Implement other output formats with smart formatting
                    const char* tag_name = log_parser_get_tag_name(entry.tag_id);
                    const char* message_format = log_parser_get_message_text(entry.message_id);
                    
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
            }
        }
    }
    
    fclose(input);
    fclose(output);
    return 0;
}

// 删除以下重复的函数定义：
// /**
//  * @brief Get parser statistics
//  * 
//  * @param stats Pointer to statistics structure
//  */
// void log_parser_get_stats(parser_stats_t* stats) {
//     if (stats) {
//         *stats = g_parser_stats;
//     }
// }