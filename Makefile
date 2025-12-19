# AutoMLOS Project Build System
# Modern build system with build directory support and safety compliance

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Build directories
BUILD_DIR = build
LIB_BUILD_DIR = $(BUILD_DIR)/lib
TEST_BUILD_DIR = $(BUILD_DIR)/tests
BIN_BUILD_DIR = $(BUILD_DIR)/bin

# Library sources with full paths
LOGGING_SOURCES = lib/logging/log_core.c lib/logging/log_ipc.c lib/logging/log_storage_adapter.c
SAFETY_SOURCES = lib/safety/system_errors.c
LIBSYSCALL_SOURCES = lib/libsyscall/libsyscall.c
LOGGER_SERVICE_SOURCES = services/logger/logger_service.c

# Library objects with build directory
LOGGING_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LOGGING_SOURCES:.c=.o)))
SAFETY_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(SAFETY_SOURCES:.c=.o)))
LIBSYSCALL_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LIBSYSCALL_SOURCES:.c=.o)))
LOGGER_SERVICE_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LOGGER_SERVICE_SOURCES:.c=.o)))

# Library targets
LOGGING_LIB = $(LIB_BUILD_DIR)/libautolog.a
SAFETY_LIB = $(LIB_BUILD_DIR)/libsafety.a
LIBSYSCALL_LIB = $(LIB_BUILD_DIR)/libsyscall.a
LOGGER_SERVICE_LIB = $(LIB_BUILD_DIR)/liblogger_service.a

# Test sources and targets
LOG_TEST_SOURCE = tests/log_test.c
INTEGRATION_TEST_SOURCE = tests/module_integration_test.c
LOG_TEST_TARGET = $(TEST_BUILD_DIR)/log_test
INTEGRATION_TEST_TARGET = $(TEST_BUILD_DIR)/module_integration_test

# Include directories
INCLUDES = -I. -Ilib/logging -Ilib/safety -Ilib/libsyscall -Iservices/logger

# Default target
all: libraries tests

# Create build directories
$(BUILD_DIR) $(LIB_BUILD_DIR) $(TEST_BUILD_DIR) $(BIN_BUILD_DIR):
	mkdir -p $@

# Library compilation rules
$(LOGGING_LIB): $(LOGGING_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LOGGING_OBJECTS)
	ranlib $@

$(SAFETY_LIB): $(SAFETY_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(SAFETY_OBJECTS)
	ranlib $@

$(LIBSYSCALL_LIB): $(LIBSYSCALL_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LIBSYSCALL_OBJECTS)
	ranlib $@

$(LOGGER_SERVICE_LIB): $(LOGGER_SERVICE_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LOGGER_SERVICE_OBJECTS)
	ranlib $@

# Object file compilation with full source paths
$(LIB_BUILD_DIR)/log_core.o: lib/logging/log_core.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/log_ipc.o: lib/logging/log_ipc.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/log_storage_adapter.o: lib/logging/log_storage_adapter.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/system_errors.o: lib/safety/system_errors.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/libsyscall.o: lib/libsyscall/libsyscall.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/logger_service.o: services/logger/logger_service.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test compilation rules with correct dependencies
$(LOG_TEST_TARGET): $(LOG_TEST_SOURCE) $(LOGGING_LIB) $(SAFETY_LIB) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -L$(LIB_BUILD_DIR) -o $@ $(LOG_TEST_SOURCE) -lautolog -lsafety

$(INTEGRATION_TEST_TARGET): $(INTEGRATION_TEST_SOURCE) $(LOGGING_LIB) $(SAFETY_LIB) $(LIBSYSCALL_LIB) $(LOGGER_SERVICE_LIB) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -L$(LIB_BUILD_DIR) -o $@ $(INTEGRATION_TEST_SOURCE) -lautolog -lsafety -lsyscall -llogger_service

# Convenience targets
libraries: $(LOGGING_LIB) $(SAFETY_LIB) $(LIBSYSCALL_LIB) $(LOGGER_SERVICE_LIB)

tests: $(LOG_TEST_TARGET) $(INTEGRATION_TEST_TARGET)

# Run tests
test: tests
	@echo "=== Running AutoMLOS Test Suite ==="
	@echo "Running log tests..."
	$(LOG_TEST_TARGET)
	@echo "Running integration tests..."
	$(INTEGRATION_TEST_TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

# Release build
release: CFLAGS += -DNDEBUG
release: all

# Show build configuration
config:
	@echo "Build Configuration:"
	@echo "  Build Directory: $(BUILD_DIR)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Libraries:"
	@echo "    - Logging: $(LOGGING_LIB)"
	@echo "    - Safety: $(SAFETY_LIB)"
	@echo "    - Syscall: $(LIBSYSCALL_LIB)"
	@echo "    - Logger Service: $(LOGGER_SERVICE_LIB)"
	@echo "  Tests:"
	@echo "    - Log Test: $(LOG_TEST_TARGET)"
	@echo "    - Integration Test: $(INTEGRATION_TEST_TARGET)"

.PHONY: all libraries tests test clean debug release config