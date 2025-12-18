/**
 * Unit Tests for Kissing Boundary Optimization
 * 
 * Tests lock-free reads, minimal locking for writes,
 * cache coherency, and performance characteristics.
 */

#include "ai/cllm_kissing_boundaries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing " #name "...\n"); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * Test: Create and destroy boundary
 */
int test_create_destroy() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    assert(boundary->sphere_a == 0);
    assert(boundary->sphere_b == 1);
    assert(boundary->boundary_memory != NULL);
    assert(boundary->boundary_size == KISSING_BOUNDARY_SIZE);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Invalid sphere IDs
 */
int test_invalid_sphere_ids() {
    KissingBoundary* boundary = kissing_boundary_create(-1, 0, KISSING_BOUNDARY_SIZE);
    assert(boundary == NULL);
    
    boundary = kissing_boundary_create(0, -1, KISSING_BOUNDARY_SIZE);
    assert(boundary == NULL);
    
    return 1;
}

/**
 * Test: Default size
 */
int test_default_size() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, 0);
    assert(boundary != NULL);
    assert(boundary->boundary_size == KISSING_BOUNDARY_SIZE);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Lock-free read
 */
int test_lockfree_read() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Write some data first
    char write_data[256];
    memset(write_data, 0xAB, sizeof(write_data));
    
    size_t written = kissing_boundary_write(boundary, 0, 0, write_data, sizeof(write_data));
    assert(written == sizeof(write_data));
    
    // Read data back (lock-free)
    char read_data[256];
    memset(read_data, 0, sizeof(read_data));
    
    size_t read = kissing_boundary_read(boundary, 0, 0, read_data, sizeof(read_data));
    assert(read == sizeof(read_data));
    
    // Verify data matches
    assert(memcmp(write_data, read_data, sizeof(write_data)) == 0);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Write with locking
 */
int test_write_locking() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Write from sphere A
    char data_a[256];
    memset(data_a, 0xAA, sizeof(data_a));
    size_t written = kissing_boundary_write(boundary, 0, 0, data_a, sizeof(data_a));
    assert(written == sizeof(data_a));
    
    // Write from sphere B
    char data_b[256];
    memset(data_b, 0xBB, sizeof(data_b));
    written = kissing_boundary_write(boundary, 1, 256, data_b, sizeof(data_b));
    assert(written == sizeof(data_b));
    
    // Read back both
    char read_a[256], read_b[256];
    kissing_boundary_read(boundary, 0, 0, read_a, sizeof(read_a));
    kissing_boundary_read(boundary, 1, 256, read_b, sizeof(read_b));
    
    assert(memcmp(data_a, read_a, sizeof(data_a)) == 0);
    assert(memcmp(data_b, read_b, sizeof(data_b)) == 0);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Bounds checking
 */
int test_bounds_checking() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    char buffer[256];
    
    // Try to read beyond boundary
    size_t result = kissing_boundary_read(boundary, 0, KISSING_BOUNDARY_SIZE, 
                                         buffer, sizeof(buffer));
    assert(result == 0);  // Should fail
    
    // Try to write beyond boundary
    result = kissing_boundary_write(boundary, 0, KISSING_BOUNDARY_SIZE,
                                    buffer, sizeof(buffer));
    assert(result == 0);  // Should fail
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Invalid sphere access
 */
int test_invalid_sphere_access() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    char buffer[256];
    
    // Try to access from invalid sphere
    size_t result = kissing_boundary_read(boundary, 2, 0, buffer, sizeof(buffer));
    assert(result == 0);  // Should fail
    
    result = kissing_boundary_write(boundary, 2, 0, buffer, sizeof(buffer));
    assert(result == 0);  // Should fail
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Version tracking
 */
int test_version_tracking() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Initial version should be 0
    uint32_t version = kissing_boundary_get_version(boundary);
    assert(version == 0);
    
    // Write should increment version
    char data[256];
    kissing_boundary_write(boundary, 0, 0, data, sizeof(data));
    
    uint32_t new_version = kissing_boundary_get_version(boundary);
    assert(new_version == 1);
    
    // Another write should increment again
    kissing_boundary_write(boundary, 1, 0, data, sizeof(data));
    new_version = kissing_boundary_get_version(boundary);
    assert(new_version == 2);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Cache invalidation
 */
int test_cache_invalidation() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    uint32_t version = kissing_boundary_get_version(boundary);
    
    // Invalidate cache
    kissing_boundary_invalidate_cache(boundary);
    
    uint32_t new_version = kissing_boundary_get_version(boundary);
    assert(new_version == version + 1);
    
    // Check version validity
    assert(kissing_boundary_check_version(boundary, version) == 0);  // Old version invalid
    assert(kissing_boundary_check_version(boundary, new_version) == 1);  // New version valid
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Statistics tracking
 */
int test_statistics() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    char buffer[256];
    
    // Initial statistics should be zero
    uint64_t reads_a, reads_b, writes_a, writes_b;
    kissing_boundary_get_stats(boundary, &reads_a, &reads_b, &writes_a, &writes_b);
    assert(reads_a == 0 && reads_b == 0 && writes_a == 0 && writes_b == 0);
    
    // Perform operations
    kissing_boundary_write(boundary, 0, 0, buffer, sizeof(buffer));
    kissing_boundary_read(boundary, 0, 0, buffer, sizeof(buffer));
    kissing_boundary_write(boundary, 1, 0, buffer, sizeof(buffer));
    kissing_boundary_read(boundary, 1, 0, buffer, sizeof(buffer));
    
    // Check statistics
    kissing_boundary_get_stats(boundary, &reads_a, &reads_b, &writes_a, &writes_b);
    assert(reads_a == 1);
    assert(reads_b == 1);
    assert(writes_a == 1);
    assert(writes_b == 1);
    
    // Reset statistics
    kissing_boundary_reset_stats(boundary);
    kissing_boundary_get_stats(boundary, &reads_a, &reads_b, &writes_a, &writes_b);
    assert(reads_a == 0 && reads_b == 0 && writes_a == 0 && writes_b == 0);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Atomic exchange
 */
int test_atomic_exchange() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Write initial value
    uint64_t initial_value = 0x1234567890ABCDEF;
    kissing_boundary_write(boundary, 0, 0, &initial_value, sizeof(initial_value));
    
    // Atomic exchange
    uint64_t new_value = 0xFEDCBA0987654321;
    uint64_t old_value = 0;
    
    int result = kissing_boundary_atomic_exchange(
        boundary, 0, 0, &new_value, &old_value, sizeof(uint64_t)
    );
    assert(result == 1);
    assert(old_value == initial_value);
    
    // Read back to verify
    uint64_t read_value = 0;
    kissing_boundary_read(boundary, 0, 0, &read_value, sizeof(read_value));
    assert(read_value == new_value);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Atomic exchange invalid size
 */
int test_atomic_exchange_invalid_size() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    char buffer[3];
    char old_buffer[3];
    
    // Try 3-byte exchange (should fail)
    int result = kissing_boundary_atomic_exchange(
        boundary, 0, 0, buffer, old_buffer, 3
    );
    assert(result == 0);  // Should fail
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: System create and destroy
 */
int test_system_create_destroy() {
    KissingBoundarySystem* system = kissing_boundary_system_create(66);
    assert(system != NULL);
    assert(system->max_boundaries == 66);
    assert(system->num_boundaries == 0);
    assert(system->boundaries != NULL);
    
    kissing_boundary_system_destroy(system);
    return 1;
}

/**
 * Test: System add boundary
 */
int test_system_add() {
    KissingBoundarySystem* system = kissing_boundary_system_create(10);
    assert(system != NULL);
    
    // Create and add boundaries
    for (int i = 0; i < 5; i++) {
        KissingBoundary* boundary = kissing_boundary_create(i, i+1, KISSING_BOUNDARY_SIZE);
        assert(boundary != NULL);
        
        int result = kissing_boundary_system_add(system, boundary);
        assert(result == 0);
        
        kissing_boundary_destroy(boundary);
    }
    
    assert(system->num_boundaries == 5);
    
    kissing_boundary_system_destroy(system);
    return 1;
}

/**
 * Test: System find boundary
 */
int test_system_find() {
    KissingBoundarySystem* system = kissing_boundary_system_create(10);
    assert(system != NULL);
    
    // Create and add boundaries
    KissingBoundary* b1 = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    KissingBoundary* b2 = kissing_boundary_create(2, 3, KISSING_BOUNDARY_SIZE);
    
    kissing_boundary_system_add(system, b1);
    kissing_boundary_system_add(system, b2);
    
    // Find boundaries
    KissingBoundary* found = kissing_boundary_system_find(system, 0, 1);
    assert(found != NULL);
    assert(found->sphere_a == 0 && found->sphere_b == 1);
    
    found = kissing_boundary_system_find(system, 1, 0);  // Reverse order
    assert(found != NULL);
    
    found = kissing_boundary_system_find(system, 2, 3);
    assert(found != NULL);
    
    found = kissing_boundary_system_find(system, 4, 5);  // Not found
    assert(found == NULL);
    
    kissing_boundary_destroy(b1);
    kissing_boundary_destroy(b2);
    kissing_boundary_system_destroy(system);
    return 1;
}

/**
 * Test: System validation
 */
int test_system_validation() {
    KissingBoundarySystem* system = kissing_boundary_system_create(10);
    assert(system != NULL);
    
    // Should be valid
    assert(kissing_boundary_system_validate(system) == 1);
    
    // NULL system should be invalid
    assert(kissing_boundary_system_validate(NULL) == 0);
    
    kissing_boundary_system_destroy(system);
    return 1;
}

/**
 * Test: Large data transfer
 */
int test_large_transfer() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Allocate large buffer (32 KB)
    size_t large_size = 32 * 1024;
    char* large_buffer = malloc(large_size);
    assert(large_buffer != NULL);
    
    // Fill with pattern
    for (size_t i = 0; i < large_size; i++) {
        large_buffer[i] = (char)(i % 256);
    }
    
    // Write to boundary
    size_t written = kissing_boundary_write(boundary, 0, 0, large_buffer, large_size);
    assert(written == large_size);
    
    // Read back
    char* read_buffer = malloc(large_size);
    assert(read_buffer != NULL);
    
    size_t read = kissing_boundary_read(boundary, 1, 0, read_buffer, large_size);
    assert(read == large_size);
    
    // Verify
    assert(memcmp(large_buffer, read_buffer, large_size) == 0);
    
    free(large_buffer);
    free(read_buffer);
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Concurrent reads (simulated)
 */
int test_concurrent_reads() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Write initial data
    char data[256];
    memset(data, 0xCD, sizeof(data));
    kissing_boundary_write(boundary, 0, 0, data, sizeof(data));
    
    // Simulate concurrent reads
    char buffer[256];
    for (int i = 0; i < 100; i++) {
        kissing_boundary_read(boundary, 0, 0, buffer, sizeof(buffer));
        kissing_boundary_read(boundary, 1, 0, buffer, sizeof(buffer));
    }
    
    // Check statistics
    uint64_t reads_a, reads_b, writes_a, writes_b;
    kissing_boundary_get_stats(boundary, &reads_a, &reads_b, &writes_a, &writes_b);
    assert(reads_a == 100);
    assert(reads_b == 100);
    assert(writes_a == 1);
    assert(writes_b == 0);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Version conflicts detection
 */
int test_version_conflicts() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    char data[256];
    char buffer[256];
    
    // Write and read multiple times
    for (int i = 0; i < 10; i++) {
        memset(data, i, sizeof(data));
        kissing_boundary_write(boundary, 0, 0, data, sizeof(data));
        kissing_boundary_read(boundary, 1, 0, buffer, sizeof(buffer));
    }
    
    // Version should have incremented
    uint32_t version = kissing_boundary_get_version(boundary);
    assert(version == 10);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

/**
 * Test: Atomic exchange 32-bit
 */
int test_atomic_exchange_32bit() {
    KissingBoundary* boundary = kissing_boundary_create(0, 1, KISSING_BOUNDARY_SIZE);
    assert(boundary != NULL);
    
    // Write initial value
    uint32_t initial_value = 0x12345678;
    kissing_boundary_write(boundary, 0, 0, &initial_value, sizeof(initial_value));
    
    // Atomic exchange
    uint32_t new_value = 0x87654321;
    uint32_t old_value = 0;
    
    int result = kissing_boundary_atomic_exchange(
        boundary, 0, 0, &new_value, &old_value, sizeof(uint32_t)
    );
    assert(result == 1);
    assert(old_value == initial_value);
    
    // Read back to verify
    uint32_t read_value = 0;
    kissing_boundary_read(boundary, 0, 0, &read_value, sizeof(read_value));
    assert(read_value == new_value);
    
    kissing_boundary_destroy(boundary);
    return 1;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("=== Kissing Boundary Optimization Unit Tests ===\n\n");
    
    TEST(create_destroy);
    TEST(invalid_sphere_ids);
    TEST(default_size);
    TEST(lockfree_read);
    TEST(write_locking);
    TEST(bounds_checking);
    TEST(invalid_sphere_access);
    TEST(version_tracking);
    TEST(cache_invalidation);
    TEST(statistics);
    TEST(large_transfer);
    TEST(concurrent_reads);
    TEST(version_conflicts);
    TEST(atomic_exchange);
    TEST(atomic_exchange_32bit);
    TEST(atomic_exchange_invalid_size);
    TEST(system_create_destroy);
    TEST(system_add);
    TEST(system_find);
    TEST(system_validation);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}