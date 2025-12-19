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

// Parser configuration
#define MAX_FORMAT_STRING_LENGTH 256
#define MAX_TAG_NAME_LENGTH 64
#define MAX_MESSAGE_TEXT_LENGTH 128

// Format string database entry
typedef struct {
    uint16_t message_id;
    char format_string[MAX_FORMAT_STRING_LENGTH];
    char description[MAX_MESSAGE_TEXT_LENGTH];
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
int log_parser_validate_entry(const uint8_t* entry_data, size_t entry_size);
void log_parser_get_stats(parser_stats_t* stats);
const char* log_parser_get_tag_name(uint8_t tag_id);
const char* log_parser_get_message_text(uint16_t message_id);

#endif // LOG_PARSER_H