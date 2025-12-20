/**
 * @file main.c
 * @brief Binary log parser main program
 * 
 * Command-line tool for parsing autoMLOS binary log files.
 */

#define _GNU_SOURCE  // Enable GNU extensions for readlink
#include "log_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

// Define PATH_MAX if not defined by system
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Get the directory containing the executable
 * @param buffer Output buffer for directory path
 * @param size Buffer size
 * @return 0 on success, -1 on error
 */
int get_tool_directory(char* buffer, size_t size) {
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) {
        return -1;
    }
    path[len] = '\0';
    
    // Get directory path - dirname may modify the path buffer
    char* dir = dirname(path);
    // Explicitly use path variable to avoid compiler warning
    if (dir == NULL || dir != path) {
        return -1;
    }
    
    if (strlen(dir) >= size) {
        return -1;
    }
    
    strncpy(buffer, dir, size - 1);
    buffer[size - 1] = '\0';
    return 0;
}

// Print usage information
void print_usage(const char* program_name) {
    printf("AutoMLOS Binary Log Parser\n");
    printf("Usage: %s [OPTIONS] INPUT_FILE OUTPUT_FILE\n\n", program_name);
    printf("Options:\n");
    printf("  -f, --format FORMAT    Output format (text, json, csv, binary)\n");
    printf("  -c, --color MODE       Color mode (never, minimal, always)\n");
    printf("  -s, --style STYLE      Log style (smart, fixed, minimal, verbose)\n");
    printf("  -m, --module TAG:LEVEL Filter by module and level (e.g., CAN:INFO)\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s log.bin log.txt -f text -c minimal -s smart\n", program_name);
    printf("  %s log.bin log.txt -f text -c never -m CAN:INFO -m UDS:ERROR\n", program_name);
    printf("  %s log.bin log.json -f json\n", program_name);
}

// Convert format string to enum
output_format_t parse_format(const char* format_str) {
    if (strcmp(format_str, "text") == 0) return OUTPUT_FORMAT_TEXT;
    if (strcmp(format_str, "json") == 0) return OUTPUT_FORMAT_JSON;
    if (strcmp(format_str, "csv") == 0) return OUTPUT_FORMAT_CSV;
    if (strcmp(format_str, "binary") == 0) return OUTPUT_FORMAT_BINARY;
    return OUTPUT_FORMAT_TEXT; // Default
}

// Convert color mode string to enum
color_mode_t parse_color_mode(const char* mode_str) {
    if (strcmp(mode_str, "never") == 0) return COLOR_MODE_NEVER;
    if (strcmp(mode_str, "minimal") == 0) return COLOR_MODE_MINIMAL;
    if (strcmp(mode_str, "always") == 0) return COLOR_MODE_ALWAYS;
    return COLOR_MODE_MINIMAL; // Default
}

// Convert log format string to enum
log_format_t parse_log_format(const char* format_str) {
    if (strcmp(format_str, "smart") == 0) return FORMAT_SMART;
    if (strcmp(format_str, "fixed") == 0) return FORMAT_FIXED;
    if (strcmp(format_str, "minimal") == 0) return FORMAT_MINIMAL;
    if (strcmp(format_str, "verbose") == 0) return FORMAT_VERBOSE;
    return FORMAT_SMART; // Default
}

// Parse module filter string (e.g., "CAN:INFO")
int parse_module_filter(const char* filter_str, module_filter_t* filter) {
    char tag_str[32] = {0};
    char level_str[32] = {0};
    
    if (sscanf(filter_str, "%31[^:]:%31s", tag_str, level_str) != 2) {
        return -1;
    }
    
    // Map tag name to tag ID
    if (strcmp(tag_str, "SYSTEM") == 0) filter->tag_id = LOG_TAG_SYSTEM;
    else if (strcmp(tag_str, "KERNEL") == 0) filter->tag_id = LOG_TAG_KERNEL;
    else if (strcmp(tag_str, "DRIVER_CAN") == 0) filter->tag_id = LOG_TAG_DRIVER_CAN;
    else if (strcmp(tag_str, "SERVICE_AI") == 0) filter->tag_id = LOG_TAG_SERVICE_AI;
    else if (strcmp(tag_str, "SERVICE_DIAG") == 0) filter->tag_id = LOG_TAG_SERVICE_DIAG;
    else if (strcmp(tag_str, "APPLICATION") == 0) filter->tag_id = LOG_TAG_APPLICATION;
    else return -1;
    
    // Map level name to level (WARN level not supported in log_common.h)
    if (strcmp(level_str, "DEBUG") == 0) filter->min_level = LOG_LEVEL_DEBUG;
    else if (strcmp(level_str, "INFO") == 0) filter->min_level = LOG_LEVEL_INFO;
    else if (strcmp(level_str, "ERROR") == 0) filter->min_level = LOG_LEVEL_ERROR;
    else if (strcmp(level_str, "EMERGENCY") == 0) filter->min_level = LOG_LEVEL_EMERGENCY;
    else return -1;
    
    return 0;
}

int main(int argc, char* argv[]) {
    // Default values
    const char* input_file = NULL;
    const char* output_file = NULL;
    char format_db_path[PATH_MAX];
    char tag_db_path[PATH_MAX];
    output_format_t output_format = OUTPUT_FORMAT_TEXT;
    color_mode_t color_mode = COLOR_MODE_MINIMAL;
    log_format_t log_format = FORMAT_SMART;
    int verbose = 0;
    
    // Module filters
    module_filter_t filters[16] = {0};
    uint32_t filter_count = 0;
    
    // Get tool directory and build database paths
    char tool_dir[PATH_MAX];
    if (get_tool_directory(tool_dir, sizeof(tool_dir)) != 0) {
        fprintf(stderr, "Error: Cannot determine tool directory\n");
        return 1;
    }
    
    // Build database file paths with safe string operations
    // Use strncpy and strncat to avoid format truncation warnings
    strncpy(format_db_path, tool_dir, sizeof(format_db_path) - 1);
    format_db_path[sizeof(format_db_path) - 1] = '\0';
    strncat(format_db_path, "/formats.db", sizeof(format_db_path) - strlen(format_db_path) - 1);
    
    strncpy(tag_db_path, tool_dir, sizeof(tag_db_path) - 1);
    tag_db_path[sizeof(tag_db_path) - 1] = '\0';
    strncat(tag_db_path, "/tags.db", sizeof(tag_db_path) - strlen(tag_db_path) - 1);
    
    // Use the variables in debug output to avoid "unused variable" warnings
    if (verbose) {
        printf("Tool directory: %s\n", tool_dir);
        printf("Format database: %s\n", format_db_path);
        printf("Tag database: %s\n", tag_db_path);
    }
    
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
        // Remove -d and -t options - database paths are auto-detected
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color") == 0) {
            if (i + 1 < argc) {
                color_mode = parse_color_mode(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--style") == 0) {
            if (i + 1 < argc) {
                log_format = parse_log_format(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--module") == 0) {
            if (i + 1 < argc && filter_count < 16) {
                if (parse_module_filter(argv[++i], &filters[filter_count]) == 0) {
                    filter_count++;
                } else {
                    fprintf(stderr, "Warning: Invalid module filter format: %s\n", argv[i]);
                }
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
        printf("Tool directory: %s\n", tool_dir);
        printf("Format database: %s\n", format_db_path);
        printf("Tag database: %s\n", tag_db_path);
        printf("Color mode: %s\n", 
               color_mode == COLOR_MODE_NEVER ? "never" : 
               color_mode == COLOR_MODE_MINIMAL ? "minimal" : "always");
        printf("Log format: %s\n", 
               log_format == FORMAT_SMART ? "smart" : 
               log_format == FORMAT_FIXED ? "fixed" : 
               log_format == FORMAT_MINIMAL ? "minimal" : "verbose");
        if (filter_count > 0) {
            printf("Module filters: %u active\n", filter_count);
        }
    }
    
    // Initialize parser with auto-detected database paths
    if (log_parser_init(format_db_path, tag_db_path) != 0) {
        fprintf(stderr, "Error: Failed to initialize parser with database files\n");
        fprintf(stderr, "Please ensure the tool and database files are in the same directory:\n");
        fprintf(stderr, "  Tool: %s/log_parser\n", tool_dir);
        fprintf(stderr, "  Format DB: %s\n", format_db_path);
        fprintf(stderr, "  Tag DB: %s\n", tag_db_path);
        return 1;
    }
    
    if (verbose) {
        printf("Processing file: %s -> %s\n", input_file, output_file);
    }
    
    // Process the log file with extended options
    int result;
    if (filter_count > 0 || color_mode != COLOR_MODE_MINIMAL || log_format != FORMAT_SMART) {
        result = log_parser_process_file_ex(input_file, output_file, output_format, 
                                          color_mode, log_format, filters, filter_count);
    } else {
        result = log_parser_process_file(input_file, output_file, output_format);
    }
    
    if (result != 0) {
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