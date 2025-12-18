CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS =

TARGETS = clock_hash

all: $(TARGETS)

clock_hash: clock_hash.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: clock_hash
	@echo "=== Testing Clock Hash ==="
	@echo ""
	@echo "Test 1: Running demonstration"
	./clock_hash
	@echo ""
	@echo "Test 2: Hashing specific strings"
	./clock_hash "Hello, World!"
	./clock_hash "test"
	./clock_hash "The quick brown fox"

clean:
	rm -f $(TARGETS)

.PHONY: all test clean