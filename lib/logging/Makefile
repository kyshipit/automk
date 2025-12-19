# AutoMLOS Binary Logging System Makefile
# Compiles with automotive safety standards compliance

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Library source files
LIB_SOURCES = log_core.c log_ipc.c log_storage_adapter.c logger_client.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIB_TARGET = libautolog.a

# Parser tool source files  
PARSER_SOURCES = ../tools/log_parser/log_parser.c ../tools/log_parser/main.c
PARSER_OBJECTS = $(PARSER_SOURCES:.c=.o)
PARSER_TARGET = log_parser

# Include directories
INCLUDES = -I. -I../.. -I../../services/logger

# Default target
all: $(LIB_TARGET) $(PARSER_TARGET)

# Library target
$(LIB_TARGET): $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)
	ranlib $@

# Parser tool target
$(PARSER_TARGET): $(PARSER_OBJECTS) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $(PARSER_OBJECTS) -L. -lautolog

# Pattern rule for object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(LIB_OBJECTS) $(LIB_TARGET)
	rm -f $(PARSER_OBJECTS) $(PARSER_TARGET)
	rm -f *.gch

# Install library to system directory
install: $(LIB_TARGET)
	cp $(LIB_TARGET) /usr/local/lib/
	cp log_core.h log_ipc.h log_storage_adapter.h logger_client.h /usr/local/include/

# Uninstall library
uninstall:
	rm -f /usr/local/lib/$(LIB_TARGET)
	rm -f /usr/local/include/log_core.h /usr/local/include/log_ipc.h /usr/local/include/log_storage_adapter.h /usr/local/include/logger_client.h

# Test target
test: all
	@echo "Running basic tests..."
	# TODO: Implement comprehensive tests