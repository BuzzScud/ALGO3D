# CLLM Library Makefile
# Crystalline Lattice Language Model - 88D Architecture

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c11 -D_GNU_SOURCE
# Enable pthread barriers on macOS
ifeq ($(shell uname),Darwin)
CFLAGS += -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200112L
endif
CFLAGS += -I./include -I./include/ai
CFLAGS += -I./include -I../algorithms/include -I../math/include
LDFLAGS = -shared
ARFLAGS = rcs

# Detect CPU capabilities for SIMD optimization
SIMD_FLAGS := $(shell grep -q avx2 /proc/cpuinfo 2>/dev/null && echo "-mavx2 -mfma" || echo "")
CFLAGS += $(SIMD_FLAGS)

# Source files (excluding test files)
SOURCES = $(wildcard src/*.c) \
          $(wildcard src/infrastructure/*.c) \
          $(wildcard src/platonic/*.c) \
          $(wildcard src/crawler/handlers/*.c) \
          $(wildcard src/document_processing/*.c)

# Crawler sources (excluding test files)
CRAWLER_SOURCES = $(filter-out %test_links.c %test_prime.c, $(wildcard src/crawler/*.c))
SOURCES += $(CRAWLER_SOURCES)

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Library files
STATIC_LIB = libcllm.a
SHARED_LIB = libcllm.so

# Default target
.PHONY: all
all: $(STATIC_LIB) $(SHARED_LIB)

# Compile source files
%.o: %.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Build static library
$(STATIC_LIB): $(OBJECTS)
	@echo "Creating static CLLM library: $@"
	@$(AR) $(ARFLAGS) $@ $^
	@echo "✓ CLLM static library created"

# Build shared library
$(SHARED_LIB): $(OBJECTS)
	@echo "Creating shared CLLM library: $@"
	@$(CC) $(LDFLAGS) -o $@ $^ -L.. -L../algorithms -L../math/lib \
		-lalgorithms -lcrystallinemath -lm -lpthread -lcurl -lsqlite3
	@echo "✓ CLLM shared library created"

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning CLLM build artifacts..."
	@rm -f $(OBJECTS)
	@rm -f $(STATIC_LIB) $(SHARED_LIB)
	@echo "✓ CLLM cleaned"

# Install libraries
.PHONY: install
install: all
	@echo "Installing CLLM libraries..."
	@mkdir -p /usr/local/lib
	@mkdir -p /usr/local/include/cllm
	@cp $(STATIC_LIB) /usr/local/lib/
	@cp $(SHARED_LIB) /usr/local/lib/
	@cp -r include/* /usr/local/include/cllm/
	@ldconfig
	@echo "✓ CLLM installation complete"

# Help
.PHONY: help
help:
	@echo "CLLM Library Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build static and shared libraries (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Install libraries to /usr/local"
	@echo "  help    - Show this help message"