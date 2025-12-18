# Makefile for Prime Math Library Demo Programs

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -L.. -L../math/lib -lcllm -lalgorithms -lcrystallinemath -lpthread -lm
INCLUDES = -I../cllm/include/ai -I../algorithms/include -I../math/include

# All demo source files
DEMO_SOURCES = $(wildcard *.c)
DEMO_EXECUTABLES = $(DEMO_SOURCES:.c=)

# Default target
all: $(DEMO_EXECUTABLES)

# Generic rule for building demo executables
%: %.c
	@echo "Building $@..."
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS) -Wl,-rpath,..

# Run all demos
run: all
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║       Prime Math Library Demo Programs                ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Available demos:"
	@for demo in $(DEMO_EXECUTABLES); do \
		echo "  - $$demo"; \
	done
	@echo ""
	@echo "Run individual demos with: LD_LIBRARY_PATH=.. ./demo_name"

# Run individual demos
run_prime: prime_demo
	@echo "Running Prime Math demo..."
	@LD_LIBRARY_PATH=.. ./prime_demo

run_cllm: cllm_demo
	@echo "Running CLLM demo..."
	@LD_LIBRARY_PATH=.. ./cllm_demo

run_pretrain: pretrain_model
	@echo "Running CLLM pretraining demo..."
	@LD_LIBRARY_PATH=.. ./pretrain_model

# Clean
clean:
	@echo "Cleaning demo executables..."
	rm -f $(DEMO_EXECUTABLES)
	@echo "✓ Demos cleaned"

# Info
info:
	@echo "Demo Programs Information"
	@echo "========================="
	@echo "Total demos: $(words $(DEMO_EXECUTABLES))"
	@echo ""
	@echo "Demo files:"
	@for demo in $(DEMO_EXECUTABLES); do \
		echo "  - $$demo"; \
	done
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build all demos"
	@echo "  run          - Show available demos"
	@echo "  run_prime    - Run prime math demo"
	@echo "  run_cllm     - Run CLLM demo"
	@echo "  run_pretrain - Run pretraining demo"
	@echo "  clean        - Remove demo executables"

.PHONY: all run run_prime run_cllm run_pretrain clean info
