# System Monitor Service Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -I../../lib -I../../platform/bsp
LDFLAGS = -L../../build/lib -lipc -lsafety

SRCS = system_monitor.c
OBJS = $(SRCS:.c=.o)
TARGET = system_monitor

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) ../../build/services/