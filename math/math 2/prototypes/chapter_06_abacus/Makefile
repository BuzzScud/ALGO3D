CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lm

TARGETS = simple_abacus

all: $(TARGETS)

simple_abacus: simple_abacus.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: simple_abacus
	@echo "=== Testing Simple Abacus ==="
	@echo ""
	@echo "Test 1: Running demonstration"
	./simple_abacus
	@echo ""
	@echo "Test 2: Converting specific values"
	./simple_abacus 3.141592653589793 6
	@echo ""
	./simple_abacus 60 0
	@echo ""
	./simple_abacus 3600 0

clean:
	rm -f $(TARGETS)

.PHONY: all test clean