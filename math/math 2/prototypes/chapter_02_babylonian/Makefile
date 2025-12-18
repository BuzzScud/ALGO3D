CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lm

TARGETS = base60_converter

all: $(TARGETS)

base60_converter: base60_converter.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: base60_converter
	@echo "=== Testing Base-60 Converter ==="
	@echo ""
	@echo "Test 1: Running demonstration"
	./base60_converter
	@echo ""
	@echo "Test 2: Converting specific values"
	./base60_converter 3661 0
	@echo ""
	./base60_converter 3.141592653589793 4
	@echo ""
	./base60_converter 60 0

clean:
	rm -f $(TARGETS)

.PHONY: all test clean