/**
 * Unit Tests for Hierarchical Memory System
 * 
 * Tests parent-child memory relationships, shared boundaries,
 * and memory coherency.
 */

#include "ai/cllm_hierarchical_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
 * Test: Create and destroy hierarchical memory
 */
int test_create_destroy() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    assert(mem->parent_sphere_id == 0);
    assert(mem->num_children == 12);
    assert(mem->parent_memory != NULL);
    assert(mem->parent_size == CONTROL_MEMORY_SIZE);
    
    // Check all children allocated
    for (int i = 0; i < 12; i++) {
        assert(mem->child_memories[i] != NULL);
        assert(mem->child_sizes[i] == WORKER_MEMORY_SIZE);
        assert(mem->shared_boundaries[i] != NULL);
        assert(mem->boundary_sizes[i] == SHARED_BOUNDARY_SIZE);
    }
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Create with zero children
 */
int test_zero_children() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 0);
    assert(mem != NULL);
    assert(mem->num_children == 0);
    assert(mem->parent_memory != NULL);
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Invalid num_children
 */
int test_invalid_children() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, -1);
    assert(mem == NULL);
    
    mem = hierarchical_memory_create(0, 13);
    assert(mem == NULL);
    
    return 1;
}

/**
 * Test: Parent memory access (read/write)
 */
int test_parent_access() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Write data to parent memory
    char write_data[256];
    memset(write_data, 0xAB, sizeof(write_data));
    
    size_t written = hierarchical_memory_access_parent(
        mem, 0, write_data, sizeof(write_data), 1
    );
    assert(written == sizeof(write_data));
    
    // Read data back
    char read_data[256];
    memset(read_data, 0, sizeof(read_data));
    
    size_t read = hierarchical_memory_access_parent(
        mem, 0, read_data, sizeof(read_data), 0
    );
    assert(read == sizeof(read_data));
    
    // Verify data matches
    assert(memcmp(write_data, read_data, sizeof(write_data)) == 0);
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Child memory access
 */
int test_child_access() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Test each child
    for (int child = 0; child < 12; child++) {
        // Write data to child memory
        char write_data[256];
        memset(write_data, 0xCD + child, sizeof(write_data));
        
        size_t written = hierarchical_memory_access_child(
            mem, child, 0, write_data, sizeof(write_data), 1
        );
        assert(written == sizeof(write_data));
        
        // Read data back
        char read_data[256];
        memset(read_data, 0, sizeof(read_data));
        
        size_t read = hierarchical_memory_access_child(
            mem, child, 0, read_data, sizeof(read_data), 0
        );
        assert(read == sizeof(read_data));
        
        // Verify data matches
        assert(memcmp(write_data, read_data, sizeof(write_data)) == 0);
    }
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Boundary memory access
 */
int test_boundary_access() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Test each boundary
    for (int child = 0; child < 12; child++) {
        // Write data to boundary
        char write_data[256];
        memset(write_data, 0xEF + child, sizeof(write_data));
        
        size_t written = hierarchical_memory_access_boundary(
            mem, child, 0, write_data, sizeof(write_data), 1
        );
        assert(written == sizeof(write_data));
        
        // Read data back
        char read_data[256];
        memset(read_data, 0, sizeof(read_data));
        
        size_t read = hierarchical_memory_access_boundary(
            mem, child, 0, read_data, sizeof(read_data), 0
        );
        assert(read == sizeof(read_data));
        
        // Verify data matches
        assert(memcmp(write_data, read_data, sizeof(write_data)) == 0);
    }
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Bounds checking
 */
int test_bounds_checking() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    char buffer[256];
    
    // Try to access beyond parent memory
    size_t result = hierarchical_memory_access_parent(
        mem, CONTROL_MEMORY_SIZE, buffer, sizeof(buffer), 0
    );
    assert(result == 0);  // Should fail
    
    // Try to access beyond child memory
    result = hierarchical_memory_access_child(
        mem, 0, WORKER_MEMORY_SIZE, buffer, sizeof(buffer), 0
    );
    assert(result == 0);  // Should fail
    
    // Try to access beyond boundary memory
    result = hierarchical_memory_access_boundary(
        mem, 0, SHARED_BOUNDARY_SIZE, buffer, sizeof(buffer), 0
    );
    assert(result == 0);  // Should fail
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Invalid child index
 */
int test_invalid_child_index() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    char buffer[256];
    
    // Try to access invalid child
    size_t result = hierarchical_memory_access_child(
        mem, -1, 0, buffer, sizeof(buffer), 0
    );
    assert(result == 0);  // Should fail
    
    result = hierarchical_memory_access_child(
        mem, 12, 0, buffer, sizeof(buffer), 0
    );
    assert(result == 0);  // Should fail
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Statistics tracking
 */
int test_statistics() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    char buffer[256];
    
    // Initial statistics should be zero
    assert(atomic_load(&mem->parent_accesses) == 0);
    for (int i = 0; i < 12; i++) {
        assert(atomic_load(&mem->child_accesses[i]) == 0);
        assert(atomic_load(&mem->boundary_accesses[i]) == 0);
    }
    
    // Access parent memory
    hierarchical_memory_access_parent(mem, 0, buffer, sizeof(buffer), 0);
    assert(atomic_load(&mem->parent_accesses) == 1);
    
    // Access child memory
    hierarchical_memory_access_child(mem, 0, 0, buffer, sizeof(buffer), 0);
    assert(atomic_load(&mem->child_accesses[0]) == 1);
    
    // Access boundary memory
    hierarchical_memory_access_boundary(mem, 0, 0, buffer, sizeof(buffer), 0);
    assert(atomic_load(&mem->boundary_accesses[0]) == 1);
    
    // Reset statistics
    hierarchical_memory_reset_stats(mem);
    assert(atomic_load(&mem->parent_accesses) == 0);
    assert(atomic_load(&mem->child_accesses[0]) == 0);
    assert(atomic_load(&mem->boundary_accesses[0]) == 0);
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Validation
 */
int test_validation() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Should be valid
    assert(hierarchical_memory_validate(mem) == 1);
    
    // Test NULL
    assert(hierarchical_memory_validate(NULL) == 0);
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Child sphere ID assignment
 */
int test_child_sphere_ids() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Add child sphere IDs
    for (int i = 0; i < 12; i++) {
        int result = hierarchical_memory_add_child(mem, i, 100 + i);
        assert(result == 1);  // Success (returns 1)
    }
    
    // Verify assignments
    for (int i = 0; i < 12; i++) {
        int child_id = hierarchical_memory_get_child_id(mem, i);
        assert(child_id == 100 + i);
    }
    
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Large data transfer
 */
int test_large_transfer() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    // Allocate large buffer (100 KB)
    size_t large_size = 100 * 1024;
    char* large_buffer = malloc(large_size);
    assert(large_buffer != NULL);
    
    // Fill with pattern
    for (size_t i = 0; i < large_size; i++) {
        large_buffer[i] = (char)(i % 256);
    }
    
    // Write to parent memory
    size_t written = hierarchical_memory_access_parent(
        mem, 0, large_buffer, large_size, 1
    );
    assert(written == large_size);
    
    // Read back
    char* read_buffer = malloc(large_size);
    assert(read_buffer != NULL);
    
    size_t read = hierarchical_memory_access_parent(
        mem, 0, read_buffer, large_size, 0
    );
    assert(read == large_size);
    
    // Verify
    assert(memcmp(large_buffer, read_buffer, large_size) == 0);
    
    free(large_buffer);
    free(read_buffer);
    hierarchical_memory_destroy(mem);
    return 1;
}

/**
 * Test: Concurrent access simulation
 */
int test_concurrent_access() {
    HierarchicalMemory* mem = hierarchical_memory_create(0, 12);
    assert(mem != NULL);
    
    char buffer[256];
    
    // Simulate concurrent accesses from multiple children
    for (int iteration = 0; iteration < 100; iteration++) {
        for (int child = 0; child < 12; child++) {
            // Each child writes to its own memory
            memset(buffer, child, sizeof(buffer));
            hierarchical_memory_access_child(
                mem, child, 0, buffer, sizeof(buffer), 1
            );
            
            // Each child accesses shared boundary
            hierarchical_memory_access_boundary(
                mem, child, 0, buffer, sizeof(buffer), 0
            );
        }
    }
    
    // Verify statistics
    for (int child = 0; child < 12; child++) {
        assert(atomic_load(&mem->child_accesses[child]) == 100);
        assert(atomic_load(&mem->boundary_accesses[child]) == 100);
    }
    
    hierarchical_memory_destroy(mem);
    return 1;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("=== Hierarchical Memory Unit Tests ===\n\n");
    
    TEST(create_destroy);
    TEST(zero_children);
    TEST(invalid_children);
    TEST(parent_access);
    TEST(child_access);
    TEST(boundary_access);
    TEST(bounds_checking);
    TEST(invalid_child_index);
    TEST(statistics);
    TEST(validation);
    TEST(child_sphere_ids);
    TEST(large_transfer);
    TEST(concurrent_access);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
}