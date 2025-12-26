# AutoMLOS Libraries Build System
# Handles all library compilation and linking details

# Inherit from parent Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Build directories
BUILD_DIR = ../build
LIB_BUILD_DIR = $(BUILD_DIR)/lib

# Library sources
LOGGING_SOURCES = logging/log_core.c logging/log_client.c logging/log_storage_adapter.c
LIBIPC_SOURCES = libipc/ipc_topics.c libipc/ipc_pubsub.c libipc/ipc_batch.c libipc/ipc_priority.c
SAFETY_SOURCES = safety/system_errors.c
LIBSYSCALL_SOURCES = libsyscall/libsyscall.c
TIME_SOURCES = time/time.c
WATCHDOG_SOURCES = watchdog/watchdog.c
BSP_SOURCES = ../platform/bsp/hw_timer.c ../platform/bsp/hw_watchdog.c ../platform/bsp/hw_rtc.c

# Library objects
LOGGING_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LOGGING_SOURCES:.c=.o)))
LIBIPC_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LIBIPC_SOURCES:.c=.o)))
SAFETY_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(SAFETY_SOURCES:.c=.o)))
LIBSYSCALL_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LIBSYSCALL_SOURCES:.c=.o)))
TIME_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(TIME_SOURCES:.c=.o)))
WATCHDOG_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(WATCHDOG_SOURCES:.c=.o)))
BSP_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(BSP_SOURCES:.c=.o)))

# Library targets
LOGGING_LIB = $(LIB_BUILD_DIR)/libautolog.a
LIBIPC_LIB = $(LIB_BUILD_DIR)/libipc.a
SAFETY_LIB = $(LIB_BUILD_DIR)/libsafety.a
LIBSYSCALL_LIB = $(LIB_BUILD_DIR)/libsyscall.a
TIME_LIB = $(LIB_BUILD_DIR)/libtime.a
WATCHDOG_LIB = $(LIB_BUILD_DIR)/libwatchdog.a
BSP_LIB = $(LIB_BUILD_DIR)/libbsp.a

# Include directories
INCLUDES = -I. -Ilogging -Isafety -Ilibsyscall -Ilibipc -I../platform/bsp

# Default target
all: $(LOGGING_LIB) $(LIBIPC_LIB) $(SAFETY_LIB) $(LIBSYSCALL_LIB) $(TIME_LIB) $(WATCHDOG_LIB) $(BSP_LIB)

# Create build directories
$(LIB_BUILD_DIR):
	mkdir -p $@

# Library compilation rules
$(LOGGING_LIB): $(LOGGING_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LOGGING_OBJECTS)
	ranlib $@

$(LIBIPC_LIB): $(LIBIPC_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LIBIPC_OBJECTS)
	ranlib $@

$(SAFETY_LIB): $(SAFETY_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(SAFETY_OBJECTS)
	ranlib $@

$(LIBSYSCALL_LIB): $(LIBSYSCALL_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(LIBSYSCALL_OBJECTS)
	ranlib $@

$(TIME_LIB): $(TIME_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(TIME_OBJECTS)
	ranlib $@

$(WATCHDOG_LIB): $(WATCHDOG_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(WATCHDOG_OBJECTS)
	ranlib $@

# Object file compilation
$(LIB_BUILD_DIR)/log_core.o: logging/log_core.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/log_client.o: logging/log_client.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/log_storage_adapter.o: logging/log_storage_adapter.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/ipc_topics.o: libipc/ipc_topics.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/ipc_pubsub.o: libipc/ipc_pubsub.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/ipc_batch.o: libipc/ipc_batch.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/ipc_priority.o: libipc/ipc_priority.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/system_errors.o: safety/system_errors.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/libsyscall.o: libsyscall/libsyscall.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/time.o: time/time.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/watchdog.o: watchdog/watchdog.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# BSP library
$(BSP_LIB): $(BSP_OBJECTS) | $(LIB_BUILD_DIR)
	ar rcs $@ $(BSP_OBJECTS)
	ranlib $@

# Object file compilation - fix the path references
$(LIB_BUILD_DIR)/hw_timer.o: ../platform/bsp/hw_timer.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -I../platform/bsp -c $< -o $@

$(LIB_BUILD_DIR)/hw_watchdog.o: ../platform/bsp/hw_watchdog.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -I../platform/bsp -c $< -o $@

$(LIB_BUILD_DIR)/hw_rtc.o: ../platform/bsp/hw_rtc.c | $(LIB_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -I../platform/bsp -c $< -o $@

# Clean library artifacts
clean:
	rm -f $(LIB_BUILD_DIR)/*.o $(LIB_BUILD_DIR)/*.a

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

.PHONY: all clean debug