# AutoMLOS Services Build System
# Handles all service compilation and linking details

# Inherit from parent Makefile
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Build directories
BUILD_DIR = ../build
LIB_BUILD_DIR = $(BUILD_DIR)/lib
SERVICE_BUILD_DIR = $(BUILD_DIR)/services

# Service sources and targets
LOGGER_SERVICE_SOURCES = logger/logger_service.c logger/logger_main.c
LOGGER_SERVICE_OBJECTS = $(addprefix $(LIB_BUILD_DIR)/, $(notdir $(LOGGER_SERVICE_SOURCES:.c=.o)))
LOGGER_SERVICE_LIB = $(LIB_BUILD_DIR)/liblogger_service.a
LOGGER_SERVICE_EXE = $(SERVICE_BUILD_DIR)/logger_service

# Include directories
INCLUDES = -I. -Ilogger -I../lib/logging -I../lib/safety -I../lib/libsyscall -I../lib/libipc

# Default target
all: $(LOGGER_SERVICE_EXE)

# Create build directories
$(SERVICE_BUILD_DIR):
	mkdir -p $@

# Service executable compilation
$(LOGGER_SERVICE_EXE): logger/logger_main.c $(LOGGER_SERVICE_LIB) | $(SERVICE_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -L$(LIB_BUILD_DIR) -o $@ logger/logger_main.c \
	-Wl,--whole-archive -lsyscall -Wl,--no-whole-archive \
	-llogger_service -lautolog -lipc -lsafety -lbsp

# Logger service library
$(LOGGER_SERVICE_LIB): $(LOGGER_SERVICE_OBJECTS)
	ar rcs $@ $(LOGGER_SERVICE_OBJECTS)
	ranlib $@

# Object file compilation
$(LIB_BUILD_DIR)/logger_service.o: logger/logger_service.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_BUILD_DIR)/logger_main.o: logger/logger_main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean service artifacts
clean:
	rm -f $(SERVICE_BUILD_DIR)/* $(LIB_BUILD_DIR)/logger_*.o $(LIB_BUILD_DIR)/liblogger_service.a

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

.PHONY: all clean debug