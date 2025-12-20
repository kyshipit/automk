# AutoMLOS Test Suite Build System
# Handles all test compilation and linking details

# Inherit from parent Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic -g

# Build directories
BUILD_DIR = ../build
TEST_BUILD_DIR = $(BUILD_DIR)/tests
LIB_BUILD_DIR = $(BUILD_DIR)/lib

# Test sources and targets
LOG_TEST_SOURCE = log_test.c
IPC_TEST_SOURCE = ipc_test.c
INTEGRATION_TEST_SOURCE = module_integration_test.c
COMPREHENSIVE_LOG_TEST_SOURCE = comprehensive_log_test.c
BINARY_LOG_GENERATOR_SOURCE = binary_log_generator.c

LOG_TEST_TARGET = $(TEST_BUILD_DIR)/log_test
IPC_TEST_TARGET = $(TEST_BUILD_DIR)/ipc_test
INTEGRATION_TEST_TARGET = $(TEST_BUILD_DIR)/module_integration_test
COMPREHENSIVE_LOG_TEST_TARGET = $(TEST_BUILD_DIR)/comprehensive_log_test
BINARY_LOG_GENERATOR_TARGET = $(TEST_BUILD_DIR)/binary_log_generator

# Include directories
INCLUDES = -I. -I../lib/logging -I../lib/safety -I../lib/libsyscall -I../lib/libipc -I../services/logger

# Library paths and optimized link options
LIB_PATHS = -L$(LIB_BUILD_DIR)

# Handle circular dependencies with proper link ordering
# Use explicit library ordering with dependency resolution
LIB_LINK_OPTIONS = -Wl,--start-group -lautolog -lsafety -Wl,--end-group

# Default target
all: $(LOG_TEST_TARGET) $(IPC_TEST_TARGET) $(INTEGRATION_TEST_TARGET) $(COMPREHENSIVE_LOG_TEST_TARGET) $(BINARY_LOG_GENERATOR_TARGET)

# Create build directories
$(TEST_BUILD_DIR):
	mkdir -p $@

# Test compilation rules with optimized linking
$(LOG_TEST_TARGET): $(LOG_TEST_SOURCE) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIB_PATHS) -o $@ $(LOG_TEST_SOURCE) $(LIB_LINK_OPTIONS)

$(IPC_TEST_TARGET): $(IPC_TEST_SOURCE) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIB_PATHS) -o $@ $(IPC_TEST_SOURCE) \
	-Wl,--whole-archive -lsyscall -Wl,--no-whole-archive -lipc -lsafety

$(INTEGRATION_TEST_TARGET): $(INTEGRATION_TEST_SOURCE) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIB_PATHS) -o $@ $(INTEGRATION_TEST_SOURCE) \
	-Wl,--whole-archive -lsyscall -Wl,--no-whole-archive $(LIB_LINK_OPTIONS) -llogger_service

$(COMPREHENSIVE_LOG_TEST_TARGET): $(COMPREHENSIVE_LOG_TEST_SOURCE) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIB_PATHS) -o $@ $(COMPREHENSIVE_LOG_TEST_SOURCE) $(LIB_LINK_OPTIONS)

$(BINARY_LOG_GENERATOR_TARGET): $(BINARY_LOG_GENERATOR_SOURCE) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIB_PATHS) -o $@ $(BINARY_LOG_GENERATOR_SOURCE) $(LIB_LINK_OPTIONS)

# Run specific tests
test-log: $(LOG_TEST_TARGET)
	@echo "=== Running Log Tests ==="
	$(LOG_TEST_TARGET)

test-ipc: $(IPC_TEST_TARGET)
	@echo "=== Running IPC Tests ==="
	$(IPC_TEST_TARGET)

test-integration: $(INTEGRATION_TEST_TARGET)
	@echo "=== Running Integration Tests ==="
	$(INTEGRATION_TEST_TARGET)

test-comprehensive-log: $(COMPREHENSIVE_LOG_TEST_TARGET)
	@echo "=== Running Comprehensive Log Tests ==="
	$(COMPREHENSIVE_LOG_TEST_TARGET)

# Run all tests
test: all
	@echo "=== Running AutoMLOS Test Suite ==="
	@echo "Running IPC tests..."
	$(IPC_TEST_TARGET)
	@echo "Running log tests..."
	$(LOG_TEST_TARGET)
	@echo "Running integration tests..."
	$(INTEGRATION_TEST_TARGET)
	@echo "Running comprehensive log tests..."
	$(COMPREHENSIVE_LOG_TEST_TARGET)

# Clean test artifacts
clean:
	rm -f $(TEST_BUILD_DIR)/*

# Debug build
debug: CFLAGS += -DDEBUG
debug: all

# Generate binary log file for parser testing
generate-binary-logs: $(BINARY_LOG_GENERATOR_TARGET)
	@echo "=== Generating Binary Log Files for Parser Testing ==="
	$(BINARY_LOG_GENERATOR_TARGET)

# Show build configuration
config:
	@echo "Test Build Configuration:"
	@echo "  Build Directory: $(TEST_BUILD_DIR)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Tests:"
	@echo "    - IPC Test: $(IPC_TEST_TARGET)"
	@echo "    - Log Test: $(LOG_TEST_TARGET)"
	@echo "    - Integration Test: $(INTEGRATION_TEST_TARGET)"
	@echo "    - Comprehensive Log Test: $(COMPREHENSIVE_LOG_TEST_TARGET)"
	@echo "    - Binary Log Generator: $(BINARY_LOG_GENERATOR_TARGET)"
	@echo "  Library Link Options: $(LIB_LINK_OPTIONS)"

.PHONY: all test test-log test-ipc test-integration test-comprehensive-log generate-binary-logs clean debug config