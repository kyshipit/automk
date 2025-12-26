# Platform BSP Build System

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

BUILD_DIR = ../build
BSP_BUILD_DIR = $(BUILD_DIR)/bsp

BSP_SOURCES = bsp/hw_timer.c bsp/hw_watchdog.c bsp/hw_rtc.c
BSP_OBJECTS = $(addprefix $(BSP_BUILD_DIR)/, $(notdir $(BSP_SOURCES:.c=.o)))
BSP_LIB = $(BSP_BUILD_DIR)/libbsp.a

INCLUDES = -I. -Ibsp

all: $(BSP_LIB)

$(BSP_BUILD_DIR):
	mkdir -p $@

$(BSP_LIB): $(BSP_OBJECTS) | $(BSP_BUILD_DIR)
	ar rcs $@ $(BSP_OBJECTS)
	ranlib $@

$(BSP_BUILD_DIR)/hw_timer.o: bsp/hw_timer.c | $(BSP_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BSP_BUILD_DIR)/hw_watchdog.o: bsp/hw_watchdog.c | $(BSP_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BSP_BUILD_DIR)/hw_rtc.o: bsp/hw_rtc.c | $(BSP_BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(BSP_BUILD_DIR)/*.o $(BSP_BUILD_DIR)/*.a

.PHONY: all clean