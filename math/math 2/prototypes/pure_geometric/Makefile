# Makefile for Pure Geometric Computation
# 
# KEY POINT: We do NOT link to -lm (math library)
# This proves we're not using any math.h functions

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I.
LDFLAGS = 
# NOTE: NO -lm flag! We don't need the math library!

# Source files
CORE_SRCS = core/lookup_tables.c
OPS_SRCS = operations/position_ops.c
TEST_SRCS = tests/test_pure_geometric.c

# Object files
CORE_OBJS = $(CORE_SRCS:.c=.o)
OPS_OBJS = $(OPS_SRCS:.c=.o)
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Targets
all: test_pure_geometric

# Build test executable
test_pure_geometric: $(CORE_OBJS) $(OPS_OBJS) $(TEST_OBJS)
	@echo "Linking WITHOUT -lm (no math library needed)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✓ Built successfully WITHOUT math library!"

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run tests
test: test_pure_geometric
	@echo ""
	@echo "Running pure geometric tests..."
	@echo "This will prove we don't need math.h!"
	@echo ""
	./test_pure_geometric

# Clean
clean:
	rm -f $(CORE_OBJS) $(OPS_OBJS) $(TEST_OBJS) test_pure_geometric
	@echo "Cleaned build files"

# Verify no math.h usage
verify-no-math:
	@echo "Checking for math.h usage..."
	@if grep -r "include.*math\.h" core/ operations/ 2>/dev/null; then \
		echo "✗ ERROR: math.h found!"; \
		exit 1; \
	else \
		echo "✓ No math.h includes found"; \
	fi
	@if grep -r "sqrt\|sin\|cos\|tan\|pow\|exp\|log" core/ operations/ 2>/dev/null; then \
		echo "✗ ERROR: math functions found!"; \
		exit 1; \
	else \
		echo "✓ No math functions found"; \
	fi
	@echo "✓ Pure geometric implementation verified!"

.PHONY: all test clean verify-no-math