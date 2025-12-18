# Makefile for Duality and 88D Architecture Prototype

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I../../math/include -I.
LDFLAGS = -L../../math/lib -lcrystallinemath -lm

# Source files
SOURCES = duality_88d.c
TEST_SOURCES = test_duality_88d.c
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Targets
TARGET = libduality_88d.a
TEST_TARGET = test_duality_88d

.PHONY: all clean test

all: $(TARGET) $(TEST_TARGET)

# Build static library
$(TARGET): $(OBJECTS)
	ar rcs $@ $^
	@echo "Built $@"

# Build test executable
$(TEST_TARGET): $(TEST_OBJECTS) $(TARGET)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJECTS) $(TARGET) $(LDFLAGS)
	@echo "Built $@"

# Compile source files
%.o: %.c duality_88d.h
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests
test: $(TEST_TARGET)
	@echo ""
	@echo "Running tests..."
	@echo ""
	LD_LIBRARY_PATH=../../math/lib:$$LD_LIBRARY_PATH ./$(TEST_TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(TARGET) $(TEST_TARGET)
	@echo "Cleaned build artifacts"

# Help
help:
	@echo "Duality and 88D Architecture Prototype"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build library and test executable (default)"
	@echo "  test    - Build and run tests"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make        # Build everything"
	@echo "  make test   # Run tests"
	@echo "  make clean  # Clean up"