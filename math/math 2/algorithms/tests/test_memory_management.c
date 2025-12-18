/**
 * Test Suite for Memory Management
 * 
 * Tests memory allocation, pooling, and cache optimization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/memory_management.h"

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_RESET "\033[0m"

// Test macros
#define TEST_START(name) \
    do { \
        printf("\n" COLOR_YELLOW "TEST: %s" COLOR_RESET "\n", name); \
        tests_run++; \
    } while(0)

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf(COLOR_RED "  ✗ FAILED: %s" COLOR_RESET "\n", message); \
            tests_failed++; \
            return 0; \
        } \
    } while(0)

#define TEST_PASS() \
    do { \
        printf(COLOR_GREEN "  ✓ PASSED" COLOR_RESET "\n"); \
        tests_passed++; \
        return 1; \
    } while(0)

// ============================================================================
// TEST: Basic Creation and Destruction
// ============================================================================

int test_create_destroy(void) {
    TEST_START("Create and Destroy Memory Manager");
    
    MemoryManager* manager = memory_create(1, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    TEST_ASSERT(manager->enable_pooling == 1, "Pooling not enabled");
    TEST_ASSERT(manager->enable_alignment == 1, "Alignment not enabled");
    
    memory_free(manager);
    
    TEST_PASS();
}

int test_create_without_pooling(void) {
    TEST_START("Create Memory Manager Without Pooling");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    TEST_ASSERT(manager->enable_pooling == 0, "Pooling should be disabled");
    TEST_ASSERT(manager->enable_alignment == 1, "Alignment not enabled");
    
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Pool Management
// ============================================================================

int test_create_pool(void) {
    TEST_START("Create Memory Pool");
    
    MemoryManager* manager = memory_create(1, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    int result = memory_create_pool(manager, MEMORY_POOL_SMALL, 4096, 64);
    TEST_ASSERT(result == 0, "Failed to create pool");
    TEST_ASSERT(manager->num_pools == 1, "Pool count incorrect");
    
    memory_free(manager);
    
    TEST_PASS();
}

int test_multiple_pools(void) {
    TEST_START("Create Multiple Pools");
    
    MemoryManager* manager = memory_create(1, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    int result1 = memory_create_pool(manager, MEMORY_POOL_SMALL, 4096, 64);
    int result2 = memory_create_pool(manager, MEMORY_POOL_MEDIUM, 8192, 256);
    int result3 = memory_create_pool(manager, MEMORY_POOL_LARGE, 16384, 1024);
    
    TEST_ASSERT(result1 == 0, "Failed to create small pool");
    TEST_ASSERT(result2 == 0, "Failed to create medium pool");
    TEST_ASSERT(result3 == 0, "Failed to create large pool");
    TEST_ASSERT(manager->num_pools == 3, "Pool count incorrect");
    
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Basic Allocation
// ============================================================================

int test_basic_allocation(void) {
    TEST_START("Basic Memory Allocation");
    
    MemoryManager* manager = memory_create(0, 0);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr = memory_alloc(manager, 1024);
    TEST_ASSERT(ptr != NULL, "Failed to allocate memory");
    
    // Write to memory to verify it's usable
    memset(ptr, 0xAA, 1024);
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

int test_multiple_allocations(void) {
    TEST_START("Multiple Allocations");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = memory_alloc(manager, 128);
        TEST_ASSERT(ptrs[i] != NULL, "Failed to allocate memory");
    }
    
    for (int i = 0; i < 10; i++) {
        memory_dealloc(manager, ptrs[i]);
    }
    
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Aligned Allocation
// ============================================================================

int test_aligned_allocation(void) {
    TEST_START("Aligned Memory Allocation");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr = memory_alloc_aligned(manager, 1024, 64);
    TEST_ASSERT(ptr != NULL, "Failed to allocate aligned memory");
    TEST_ASSERT(((uintptr_t)ptr % 64) == 0, "Memory not properly aligned");
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

int test_cache_aligned_allocation(void) {
    TEST_START("Cache-Aligned Memory Allocation");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr = memory_alloc_cache_aligned(manager, 1024);
    TEST_ASSERT(ptr != NULL, "Failed to allocate cache-aligned memory");
    TEST_ASSERT(memory_is_cache_aligned(ptr), "Memory not cache-aligned");
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Pool Allocation
// ============================================================================

int test_pool_allocation(void) {
    TEST_START("Pool-Based Allocation");
    
    MemoryManager* manager = memory_create(1, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    // Create a pool
    int result = memory_create_pool(manager, MEMORY_POOL_SMALL, 4096, 64);
    TEST_ASSERT(result == 0, "Failed to create pool");
    
    // Allocate from pool
    void* ptr = memory_alloc(manager, 64);
    TEST_ASSERT(ptr != NULL, "Failed to allocate from pool");
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Sphere-Specific Allocation
// ============================================================================

int test_sphere_array_allocation(void) {
    TEST_START("Sphere Array Allocation");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr = memory_alloc_sphere_array(manager, 100, 128);
    TEST_ASSERT(ptr != NULL, "Failed to allocate sphere array");
    TEST_ASSERT(memory_is_cache_aligned(ptr), "Sphere array not cache-aligned");
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

int test_neighbor_array_allocation(void) {
    TEST_START("Neighbor Array Allocation");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr = memory_alloc_neighbor_array(manager, 100, 12);
    TEST_ASSERT(ptr != NULL, "Failed to allocate neighbor array");
    TEST_ASSERT(memory_is_cache_aligned(ptr), "Neighbor array not cache-aligned");
    
    memory_dealloc(manager, ptr);
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Statistics
// ============================================================================

int test_statistics_tracking(void) {
    TEST_START("Statistics Tracking");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    // Allocate some memory
    void* ptr1 = memory_alloc(manager, 1024);
    void* ptr2 = memory_alloc(manager, 2048);
    
    MemoryStatistics stats;
    int result = memory_get_statistics(manager, &stats);
    TEST_ASSERT(result == 0, "Failed to get statistics");
    TEST_ASSERT(stats.num_allocations == 2, "Allocation count incorrect");
    TEST_ASSERT(stats.total_allocated >= 3072, "Total allocated incorrect");
    
    memory_dealloc(manager, ptr1);
    memory_dealloc(manager, ptr2);
    memory_free(manager);
    
    TEST_PASS();
}

int test_cache_hit_rate(void) {
    TEST_START("Cache Hit Rate Tracking");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    // Allocate cache-aligned memory
    void* ptr1 = memory_alloc_cache_aligned(manager, 1024);
    void* ptr2 = memory_alloc_cache_aligned(manager, 2048);
    
    MemoryStatistics stats;
    memory_get_statistics(manager, &stats);
    TEST_ASSERT(stats.cache_hits == 2, "Cache hits not tracked");
    TEST_ASSERT(stats.cache_hit_rate == 1.0, "Cache hit rate incorrect");
    
    memory_dealloc(manager, ptr1);
    memory_dealloc(manager, ptr2);
    memory_free(manager);
    
    TEST_PASS();
}

int test_peak_usage(void) {
    TEST_START("Peak Usage Tracking");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptr1 = memory_alloc(manager, 1024);
    void* ptr2 = memory_alloc(manager, 2048);
    void* ptr3 = memory_alloc(manager, 4096);
    
    MemoryStatistics stats;
    memory_get_statistics(manager, &stats);
    TEST_ASSERT(stats.peak_usage >= 7168, "Peak usage not tracked correctly");
    
    memory_dealloc(manager, ptr1);
    memory_dealloc(manager, ptr2);
    memory_dealloc(manager, ptr3);
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Utility Functions
// ============================================================================

int test_cache_line_size(void) {
    TEST_START("Get Cache Line Size");
    
    size_t cache_line_size = memory_get_cache_line_size();
    TEST_ASSERT(cache_line_size == 64, "Cache line size incorrect");
    
    TEST_PASS();
}

int test_is_cache_aligned(void) {
    TEST_START("Check Cache Alignment");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* aligned = memory_alloc_cache_aligned(manager, 1024);
    TEST_ASSERT(memory_is_cache_aligned(aligned), "Aligned pointer not detected");
    
    void* unaligned = memory_alloc(manager, 1024);
    // Note: This might be aligned by chance, so we just check the function works
    memory_is_cache_aligned(unaligned);
    
    memory_dealloc(manager, aligned);
    memory_dealloc(manager, unaligned);
    memory_free(manager);
    
    TEST_PASS();
}

int test_validate(void) {
    TEST_START("Validate Memory Manager");
    
    MemoryManager* manager = memory_create(1, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    int result = memory_validate(manager);
    TEST_ASSERT(result == 1, "Validation failed for valid manager");
    
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// TEST: Stress Tests
// ============================================================================

int test_many_allocations(void) {
    TEST_START("Many Allocations Stress Test");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* ptrs[1000];
    for (int i = 0; i < 1000; i++) {
        ptrs[i] = memory_alloc(manager, 64);
        TEST_ASSERT(ptrs[i] != NULL, "Failed to allocate memory");
    }
    
    for (int i = 0; i < 1000; i++) {
        memory_dealloc(manager, ptrs[i]);
    }
    
    memory_free(manager);
    
    TEST_PASS();
}

int test_mixed_sizes(void) {
    TEST_START("Mixed Size Allocations");
    
    MemoryManager* manager = memory_create(0, 1);
    TEST_ASSERT(manager != NULL, "Failed to create memory manager");
    
    void* small = memory_alloc(manager, 16);
    void* medium = memory_alloc(manager, 1024);
    void* large = memory_alloc(manager, 65536);
    
    TEST_ASSERT(small != NULL, "Failed to allocate small");
    TEST_ASSERT(medium != NULL, "Failed to allocate medium");
    TEST_ASSERT(large != NULL, "Failed to allocate large");
    
    memory_dealloc(manager, small);
    memory_dealloc(manager, medium);
    memory_dealloc(manager, large);
    memory_free(manager);
    
    TEST_PASS();
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("  Memory Management Test Suite\n");
    printf("========================================\n");
    
    // Basic tests
    test_create_destroy();
    test_create_without_pooling();
    
    // Pool management tests
    test_create_pool();
    test_multiple_pools();
    
    // Basic allocation tests
    test_basic_allocation();
    test_multiple_allocations();
    
    // Aligned allocation tests
    test_aligned_allocation();
    test_cache_aligned_allocation();
    
    // Pool allocation tests
    test_pool_allocation();
    
    // Sphere-specific tests
    test_sphere_array_allocation();
    test_neighbor_array_allocation();
    
    // Statistics tests
    test_statistics_tracking();
    test_cache_hit_rate();
    test_peak_usage();
    
    // Utility tests
    test_cache_line_size();
    test_is_cache_aligned();
    test_validate();
    
    // Stress tests
    test_many_allocations();
    test_mixed_sizes();
    
    // Print summary
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("  Total Tests:  %d\n", tests_run);
    printf("  " COLOR_GREEN "Passed:       %d" COLOR_RESET "\n", tests_passed);
    printf("  " COLOR_RED "Failed:       %d" COLOR_RESET "\n", tests_failed);
    printf("  Success Rate: %.1f%%\n", (tests_passed * 100.0) / tests_run);
    printf("========================================\n\n");
    
    return (tests_failed == 0) ? 0 : 1;
}
