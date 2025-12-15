# Makefile for Test Data Generators

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lm

# Output directory
BIN_DIR = ../bin

# Targets
TARGETS = $(BIN_DIR)/generate_platonic_solids $(BIN_DIR)/generate_corrupted_data

all: $(BIN_DIR) $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/generate_platonic_solids: generate_platonic_solids.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR)/generate_corrupted_data: generate_corrupted_data.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean