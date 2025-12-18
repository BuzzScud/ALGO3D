# Makefile for OBJECTIVE 29 CLI Tools

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -I../include -I../include/blind_recovery
LDFLAGS = -L.. -lalgorithms -L../.. -lcrystalline -lm

# Output directory
BIN_DIR = bin

# CLI Tools
TOOLS = $(BIN_DIR)/geometric-recovery-test

all: $(BIN_DIR) $(TOOLS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/geometric-recovery-test: geometric_recovery_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TOOLS)
	rm -rf $(BIN_DIR)

test: $(BIN_DIR)/geometric-recovery-test
	@echo "Testing geometric-recovery-test..."
	@$(BIN_DIR)/geometric-recovery-test --solid cube --corruption 0.15 --verbose

.PHONY: all clean test