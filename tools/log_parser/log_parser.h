/**
 * @file log_parser.h
 * @brief Binary log offline parser for autoMLOS
 * 
 * Provides tools for parsing binary log files and converting
 * them to human-readable formats with safety validation.
 */

#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stdint.h>
#include <stdio.h>

// Include log common definitions for binary entry structure
#include "../../lib/logging/log_common.h"

// Parser configuration
#define MAX_FORMAT_STRING_LENGTH 256
#define MAX_TAG_NAME_LENGTH 64
#define MAX_MESSAGE_TEXT_LENGTH 128
#define MAX_CONTEXT_LENGTH 32
#define MAX_KEY_DATA_LENGTH 64

// ANSI color codes for minimal color scheme
#define COLOR_RED     "\033[31m"
#define COLOR_RESET   "\033[0m"

// Macro to mark unused parameters
#define UNUSED(x) (void)(x)

// Use the log_level_t type from log_common.h directly
// No need to redefine it

// Color mode options
typedef enum {
    COLOR_MODE_NEVER = 0,    // No colors
    COLOR_MODE_MINIMAL,      // Only ERROR/EMERGENCY in red
    COLOR_MODE_ALWAYS        // Full colors (not recommended)
} color_mode_t;

// Smart format options
typedef enum {
    FORMAT_SMART = 0,        // Auto-detect message type
    FORMAT_FIXED,            // Fixed field format
    FORMAT_MINIMAL,          // Minimal output
    FORMAT_VERBOSE           // Detailed output
} log_format_t;

// Message types for smart formatting
typedef enum {
    MESSAGE_TYPE_GENERAL = 0,
    MESSAGE_TYPE_CAN,
    MESSAGE_TYPE_UDS,
    MESSAGE_TYPE_KERNEL,
    MESSAGE_TYPE_SECURITY,
    MESSAGE_TYPE_COUNT
} message_type_t;

// Format string database entry
typedef struct {
    uint16_t message_id;
    char format_string[MAX_FORMAT_STRING_LENGTH];
    char description[MAX_MESSAGE_TEXT_LENGTH];
    message_type_t message_type;  // For smart formatting
} format_string_entry_t;

// Tag name database entry  
typedef struct {
    uint8_t tag_id;
    char tag_name[MAX_TAG_NAME_LENGTH];
} tag_name_entry_t;

// Parser output formats
typedef enum {
    OUTPUT_FORMAT_TEXT = 0,    // Human-readable text
    OUTPUT_FORMAT_JSON,        // JSON format
    OUTPUT_FORMAT_CSV,         // CSV format
    OUTPUT_FORMAT_BINARY       // Raw binary (for analysis)
} output_format_t;

// Module filter for selective output
typedef struct {
    uint8_t tag_id;
    log_level_t min_level;  // Use log_level_t from log_common.h
} module_filter_t;

// Parser statistics
typedef struct {
    uint32_t total_entries_parsed;
    uint32_t valid_entries;
    uint32_t corrupted_entries;
    uint32_t unknown_messages;
} parser_stats_t;

// API function declarations
int log_parser_init(const char* format_db_path, const char* tag_db_path);
int log_parser_process_file(const char* input_file, const char* output_file, output_format_t format);
int log_parser_process_file_ex(const char* input_file, const char* output_file, 
                              output_format_t format, color_mode_t color_mode, 
                              log_format_t log_format, const module_filter_t* filters, 
                              uint32_t filter_count);
int log_parser_validate_entry(const uint8_t* entry_data, size_t entry_size);
void log_parser_get_stats(parser_stats_t* stats);
const char* log_parser_get_tag_name(uint8_t tag_id);
const char* log_parser_get_message_text(uint16_t message_id);
const char* log_parser_get_level_name(log_level_t level);
message_type_t log_parser_detect_message_type(uint16_t message_id, const char* message_format);

#endif // LOG_PARSER_H