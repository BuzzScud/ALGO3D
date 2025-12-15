# Makefile for recovery_common library

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../include -I../../../src
AR = ar
ARFLAGS = rcs

# Source files
SRCS = file_io.c \
       wav_loader.c \
       corruption_detection.c \
       algorithm_dispatcher.c \
       error_handling.c \
       metrics.c \
       validation.c \
       logging.c \
       visualization.c

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