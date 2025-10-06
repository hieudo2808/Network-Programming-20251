# Makefile for User Management System
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

# Source files
SOURCES = main.c linklist.c fileprocess.c validation.c management_handler.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = user_management

# Default target
all: $(TARGET)

# Link all object files to create the executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Show project structure
show:
	@echo "Project files:"
	@echo "=============="
	@echo "Source files: $(SOURCES)"
	@echo "Header files: linklist.h fileprocess.h validation.h management_handler.h"
	@echo "Target: $(TARGET)"

.PHONY: all clean rebuild show