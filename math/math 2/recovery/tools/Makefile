# Makefile for recovery tools

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../../algorithms/include -I../../src
LDFLAGS = -L../../algorithms/lib/recovery_common -L../..
LIBS = ../../algorithms/lib/recovery_common/librecovery_common.a ../../libalgorithms.a ../../libcrystalline.a -lm

# Tools
TOOLS = geometric-recovery signal-recovery image-recovery network-recovery crypto-recovery scientific-recovery ml-recovery platonic-demo

# Default target
all: $(TOOLS)

# Build geometric-recovery
geometric-recovery: geometric-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Clean
clean:
	rm -f $(TOOLS)

# Install
install: $(TOOLS)
	mkdir -p ../../bin
	cp $(TOOLS) ../../bin/

.PHONY: all clean install
# Build signal-recovery
signal-recovery: signal-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build image-recovery
image-recovery: image-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build network-recovery
network-recovery: network-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build crypto-recovery
crypto-recovery: crypto-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build scientific-recovery
scientific-recovery: scientific-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build ml-recovery
ml-recovery: ml-recovery.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"

# Build platonic-demo
platonic-demo: platonic-demo.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)
	@echo "Built $@"
