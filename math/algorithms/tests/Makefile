# Algorithms Test Suite Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -I../include -I../../include
LDFLAGS = -L.. -L../.. -lalgorithms -lcrystalline -lm -Wl,-rpath,'$$ORIGIN/..:$$ORIGIN/../..'

# Test executables
TESTS = test_sphere_threading \
        test_visualization \
        test_memory_management \
        test_sphere_packing \
        test_threading \
        test_shared_memory \
        test_lock_free_queue \
        test_hierarchical_primes \
        test_hierarchical_structures \
        test_batch_processing

.PHONY: all clean run-all

all: $(TESTS)

test_sphere_threading: test_sphere_threading.c
	@echo "Building test_sphere_threading..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_sphere_threading built"

test_visualization: test_visualization.c
	@echo "Building test_visualization..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_visualization built"

test_sphere_packing: test_sphere_packing.c
	@echo "Building test_sphere_packing..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_sphere_packing built"

test_threading: test_threading.c
	@echo "Building test_threading..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_threading built"

test_shared_memory: test_shared_memory.c
	@echo "Building test_shared_memory..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_shared_memory built"

test_lock_free_queue: test_lock_free_queue.c
	@echo "Building test_lock_free_queue..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_lock_free_queue built"

test_hierarchical_primes: test_hierarchical_primes.c
	@echo "Building test_hierarchical_primes..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_hierarchical_primes built"

test_hierarchical_structures: test_hierarchical_structures.c
	@echo "Building test_hierarchical_structures..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_hierarchical_structures built"

test_batch_processing: test_batch_processing.c
	@echo "Building test_batch_processing..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_batch_processing built"

run-all: $(TESTS)
	@echo ""
	@echo "════════════════════════════════════════════════════════════"
	@echo "Running Algorithms Layer Tests"
	@echo "════════════════════════════════════════════════════════════"
	@for test in $(TESTS); do \
		echo ""; \
		echo "Running $$test..."; \
		LD_LIBRARY_PATH=..:../..:$$LD_LIBRARY_PATH ./$$test || exit 1; \
	done
	@echo ""
	@echo "✓ All algorithms tests completed"

clean:
	@echo "Cleaning algorithms test executables..."
	@rm -f $(TESTS)
	@echo "✓ Algorithms tests cleaned"

test_memory_management: test_memory_management.c
	@echo "Building test_memory_management..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ test_memory_management built"
