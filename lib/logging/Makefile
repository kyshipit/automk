# AutoMLOS Binary Logging System Makefile
# Compiles with automotive safety standards compliance

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wformat-security -fstack-protector-strong \
         -D_FORTIFY_SOURCE=2 -O2 -std=c99 -pedantic

# Library source files (updated to remove IPC naming)
LIB_SOURCES = log_core.c log_client.c log_storage_adapter.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIB_TARGET = libautolog.a

# Parser tool source files (corrected paths)
PARSER_SOURCES = ../../tools/log_parser/log_parser.c ../../tools/log_parser/main.c
PARSER_OBJECTS = $(PARSER_SOURCES:.c=.o)
PARSER_TARGET = log_parser

# Include directories (updated to include all necessary paths)
INCLUDES = -I. -I../.. -I../../services/logger -I../libipc -I../../lib/logging

# Default target
all: $(LIB_TARGET) $(PARSER_TARGET)

# Library target
$(LIB_TARGET): $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)
	ranlib $@

# Parser tool target
$(PARSER_TARGET): $(PARSER_OBJECTS) $(LIB_TARGET)
	$(CC) $(CFLAGS) -o $@ $(PARSER_OBJECTS) -L. -lautolog

# Pattern rule for object files in current directory
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Special rules for parser tool source files (with correct relative paths)
../../tools/log_parser/log_parser.o: ../../tools/log_parser/log_parser.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

../../tools/log_parser/main.o: ../../tools/log_parser/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(LIB_OBJECTS) $(LIB_TARGET)
	rm -f $(PARSER_OBJECTS) $(PARSER_TARGET)
	rm -f *.gch

# Install library to system directory
install: $(LIB_TARGET)
	cp $(LIB_TARGET) /usr/local/lib/
	cp log_core.h log_client.h log_storage_adapter.h /usr/local/include/

# Uninstall library
uninstall:
	rm -f /usr/local/lib/$(LIB_TARGET)
	rm -f /usr/local/include/log_core.h /usr/local/include/log_client.h /usr/local/include/log_storage_adapter.h

# Test target
test: all
	@echo "Running basic tests..."
	# TODO: Implement comprehensive tests