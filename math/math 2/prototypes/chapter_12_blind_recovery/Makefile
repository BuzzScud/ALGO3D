CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lm

TARGETS = blind_recovery_demo

all: $(TARGETS)

blind_recovery_demo: blind_recovery_demo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: blind_recovery_demo
	@echo "=== Testing Blind Recovery ==="
	@echo ""
	@echo "Test 1: Running demonstration"
	./blind_recovery_demo
	@echo ""
	@echo "Test 2: Specific values"
	./blind_recovery_demo 144000
	@echo ""
	./blind_recovery_demo 3.141592653589793

clean:
	rm -f $(TARGETS)

.PHONY: all test clean