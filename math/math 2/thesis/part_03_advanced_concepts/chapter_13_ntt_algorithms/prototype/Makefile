CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lm

TARGETS = ntt_demo

all: $(TARGETS)

ntt_demo: ntt_demo.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: ntt_demo
	@echo "=== Testing NTT Demo ==="
	@echo ""
	./ntt_demo

clean:
	rm -f $(TARGETS)

.PHONY: all test clean