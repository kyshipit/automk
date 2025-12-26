# AutoMLOS Kernel Build System
# Handles kernel compilation and linking details

# Inherit from parent Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Build directories
BUILD_DIR = ../build
LIB_BUILD_DIR = $(BUILD_DIR)/lib
KERNEL_BUILD_DIR = $(BUILD_DIR)/kernel

# Kernel sources
KERNEL_SOURCES = core/kernel.c core/interrupt.c core/ipc.c core/memory.c \
                 core/scheduler.c core/syscall.c

# Kernel objects
KERNEL_OBJECTS = $(addprefix $(KERNEL_BUILD_DIR)/, $(notdir $(KERNEL_SOURCES:.c=.o)))

# Kernel executable
KERNEL_EXE = $(KERNEL_BUILD_DIR)/automlos_kernel

# Include directories
INCLUDES = -I. -Icore -I../lib/safety -I../lib/logging -I../lib/libipc \
           -I../lib/libsyscall -I../services/system_monitor -I../platform/bsp

# Default target
all: $(KERNEL_EXE)

# Create build directories
$(KERNEL_BUILD_DIR):
	mkdir -p $@

# Kernel executable compilation
$(KERNEL_EXE): $(KERNEL_OBJECTS) | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -L$(LIB_BUILD_DIR) -o $@ $(KERNEL_OBJECTS) \
	-Wl,--whole-archive -lsyscall -Wl,--no-whole-archive \
	-lautolog -lipc -lsafety -lbsp

# Object file compilation
$(KERNEL_BUILD_DIR)/kernel.o: core/kernel.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KERNEL_BUILD_DIR)/interrupt.o: core/interrupt.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KERNEL_BUILD_DIR)/ipc.o: core/ipc.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KERNEL_BUILD_DIR)/memory.o: core/memory.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KERNEL_BUILD_DIR)/scheduler.o: core/scheduler.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(KERNEL_BUILD_DIR)/syscall.o: core/syscall.c | $(KERNEL_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean kernel artifacts
clean:
	rm -rf $(KERNEL_BUILD_DIR)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

.PHONY: all clean debug