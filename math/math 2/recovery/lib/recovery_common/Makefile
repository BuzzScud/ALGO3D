# Makefile for recovery_common library

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../include -I../../include/recovery_common -fPIC
LDFLAGS = -ljansson -lssl -lcrypto -lm
AR = ar
ARFLAGS = rcs

# Source files
SRCS = ../../src/loaders/file_io.c \
       ../../src/loaders/wav_loader.c \
       ../../src/loaders/json_loader.c \
       ../../src/loaders/csv_loader.c \
       ../../src/loaders/corruption_detection.c \
       ../../src/loaders/algorithm_dispatcher.c \
       ../../src/loaders/error_handling.c \
       ../../src/loaders/metrics.c \
       ../../src/loaders/validation.c \
       ../../src/loaders/logging.c \
       ../../src/loaders/visualization.c

# Object files
OBJS = $(SRCS:.c=.o)

# Library name
LIB = librecovery_common.a

# Default target
all: $(LIB)

# Build library
$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	@echo "Built $@"

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJS) $(LIB)

# Install (copy to lib directory)
install: $(LIB)
	mkdir -p ../../../lib
	cp $(LIB) ../../../lib/

.PHONY: all clean install