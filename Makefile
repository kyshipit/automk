# AutoMLOS Project Build System
# Top-level Makefile - Only handles final linking and coordination

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Build directories
BUILD_DIR = build
LIB_BUILD_DIR = $(BUILD_DIR)/lib
SERVICE_BUILD_DIR = $(BUILD_DIR)/services

# Default target - Build everything
all: libraries kernel services

# Create build directories
$(BUILD_DIR) $(LIB_BUILD_DIR) $(SERVICE_BUILD_DIR):
	mkdir -p $@

# Library building - delegate to lib/Makefile
libraries: | $(LIB_BUILD_DIR)
	@echo "=== Building AutoMLOS Libraries ==="
	$(MAKE) -C lib all

# Service building - delegate to services/Makefile  
services: libraries | $(SERVICE_BUILD_DIR)
	@echo "=== Building AutoMLOS Services ==="
	$(MAKE) -C services all

# Test building - delegate to tests/Makefile
tests: libraries services
	@echo "=== Building AutoMLOS Tests ==="
	$(MAKE) -C tests all

# Run tests
test: tests
	@echo "=== Running AutoMLOS Test Suite ==="
	$(MAKE) -C tests test

# Run logger service
run-logger: services
	@echo "=== Starting Logger Service ==="
	$(SERVICE_BUILD_DIR)/logger_service

# Kernel building - delegate to kernel/Makefile
kernel: libraries | $(BUILD_DIR)
	@echo "=== Building AutoMLOS Kernel ==="
	$(MAKE) -C kernel all

# Clean all build artifacts
clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C lib clean
	$(MAKE) -C kernel clean
	$(MAKE) -C services clean
	$(MAKE) -C tests clean

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

# Release build
release: CFLAGS += -DNDEBUG
release: all

# Show build configuration
config:
	@echo "AutoMLOS Build Configuration:"
	@echo "  Build Directory: $(BUILD_DIR)"
	@echo "  Compiler: $(CC)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Subdirectories:"
	@echo "    - lib/ (Libraries)"
	@echo "    - services/ (Services)"
	@echo "    - tests/ (Tests)"

.PHONY: all libraries services tests test run-logger clean debug release config