/**
 * @file main.c
 * @brief Binary log parser main program
 * 
 * Command-line tool for parsing autoMLOS binary log files.
 */

#include "log_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Print usage information
void print_usage(const char* program_name) {
    printf("AutoMLOS Binary Log Parser\n");
    printf("Usage: %s [OPTIONS] INPUT_FILE OUTPUT_FILE\n\n", program_name);
    printf("Options:\n");
    printf("  -f, --format FORMAT    Output format (text, json, csv, binary)\n");
    printf("  -d, --db PATH          Format database path\n");
    printf("  -t, --tags PATH        Tag database path\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s log.bin log.txt -f text\n", program_name);
    printf("  %s log.bin log.json -f json -d formats.db -t tags.db\n", program_name);
}

// Convert format string to enum
output_format_t parse_format(const char* format_str) {
    if (strcmp(format_str, "text") == 0) return OUTPUT_FORMAT_TEXT;
    if (strcmp(format_str, "json") == 0) return OUTPUT_FORMAT_JSON;
    if (strcmp(format_str, "csv") == 0) return OUTPUT_FORMAT_CSV;
    if (strcmp(format_str, "binary") == 0) return OUTPUT_FORMAT_BINARY;
    return OUTPUT_FORMAT_TEXT; // Default
}

int main(int argc, char* argv[]) {
    // Default values
    const char* input_file = NULL;
    const char* output_file = NULL;
    const char* format_db = "formats.db";
    const char* tag_db = "tags.db";
    output_format_t output_format = OUTPUT_FORMAT_TEXT;
    int verbose = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < argc) {
                output_format = parse_format(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--db") == 0) {
            if (i + 1 < argc) {
                format_db = argv[++i];
            }
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tags") == 0) {
            if (i + 1 < argc) {
                tag_db = argv[++i];
            }
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
        else {
            if (!input_file) {
                input_file = argv[i];
            } else if (!output_file) {
                output_file = argv[i];
            }
        }
    }
    
    // Validate arguments
    if (!input_file || !output_file) {
        fprintf(stderr, "Error: Input and output files must be specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    if (verbose) {
        printf("Initializing parser...\n");
        printf("Format database: %s\n", format_db);
        printf("Tag database: %s\n", tag_db);
    }
    
    // Initialize parser
    if (log_parser_init(format_db, tag_db) != 0) {
        fprintf(stderr, "Error: Failed to initialize parser\n");
        return 1;
    }
    
    if (verbose) {
        printf("Processing file: %s -> %s\n", input_file, output_file);
    }
    
    // Process the log file
    if (log_parser_process_file(input_file, output_file, output_format) != 0) {
        fprintf(stderr, "Error: Failed to process log file\n");
        return 1;
    }
    
    // Print statistics
    parser_stats_t stats;
    log_parser_get_stats(&stats);
    
    printf("Processing completed successfully:\n");
    printf("  Total entries parsed: %u\n", stats.total_entries_parsed);
    printf("  Valid entries: %u\n", stats.valid_entries);
    printf("  Corrupted entries: %u\n", stats.corrupted_entries);
    printf("  Unknown messages: %u\n", stats.unknown_messages);
    
    return 0;
}