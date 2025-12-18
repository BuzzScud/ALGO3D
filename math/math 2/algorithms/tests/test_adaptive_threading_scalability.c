/**
 * @file test_adaptive_threading_scalability.c
 * @brief Test Adaptive Threading Scalability (4-128 cores)
 */

#include "adaptive_threading.h"
#include "hierarchical_threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== TEST: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ PASSED: %s\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ FAILED: %s\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// TEST CASES
// ============================================================================

/**
 * Test 1: System detection
 */
bool test_system_detection() {
    printf("Detecting system resources...\n");
    
    uint32_t cores = adaptive_get_available_cores();
    uint64_t memory_mb = adaptive_get_available_memory_mb();
    
    printf("  Available cores: %u\n", cores);
    printf("  Available memory: %lu MB\n", memory_mb);
    
    assert(cores >= 1);
    assert(cores <= 256);  // Reasonable upper bound
    assert(memory_mb >= 512);  // At least 512 MB
    
    // Test recommended threads
    uint32_t cpu_bound = adaptive_get_recommended_threads(0);
    uint32_t mem_bound = adaptive_get_recommended_threads(1);
    uint32_t balanced = adaptive_get_recommended_threads(2);
    
    printf("  Recommended threads (CPU-bound): %u\n", cpu_bound);
    printf("  Recommended threads (Memory-bound): %u\n", mem_bound);
    printf("  Recommended threads (Balanced): %u\n", balanced);
    
    assert(cpu_bound >= 1 && cpu_bound <= 96);
    assert(mem_bound >= 1 && mem_bound <= 96);
    assert(balanced >= 1 && balanced <= 96);
    
    return true;
}

/**
 * Test 2: Create adaptive pool with 1 core
 */
bool test_create_1_core() {
    printf("Creating adaptive pool with 1 core...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 1);
    if (!pool) {
        printf("Failed to create pool\n");
        return false;
    }
    
    assert(pool->use_adaptive_threading == true);
    assert(pool->max_physical_threads == 1);
    assert(pool->num_logical_threads == 96);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Logical threads: %u\n", logical);
    printf("  Physical threads: %u\n", physical);
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    
    assert(logical == 96);
    assert(physical == 1);
    assert(memory_mb == 100);  // 1 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 3: Create adaptive pool with 4 cores
 */
bool test_create_4_cores() {
    printf("Creating adaptive pool with 4 cores...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 4);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 4);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 400);  // 4 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 4: Create adaptive pool with 8 cores
 */
bool test_create_8_cores() {
    printf("Creating adaptive pool with 8 cores...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 8);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 8);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 800);  // 8 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 5: Create adaptive pool with 16 cores
 */
bool test_create_16_cores() {
    printf("Creating adaptive pool with 16 cores...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 16);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 16);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 1600);  // 16 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 6: Create adaptive pool with 32 cores
 */
bool test_create_32_cores() {
    printf("Creating adaptive pool with 32 cores...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 32);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 32);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 3200);  // 32 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 7: Create adaptive pool with 64 cores
 */
bool test_create_64_cores() {
    printf("Creating adaptive pool with 64 cores...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 64);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 64);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 6400);  // 64 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 8: Create adaptive pool with 96 cores (maximum)
 */
bool test_create_96_cores() {
    printf("Creating adaptive pool with 96 cores (maximum)...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 96);
    if (!pool) return false;
    
    assert(pool->max_physical_threads == 96);
    
    uint32_t logical, physical;
    uint64_t work_stolen, memory_mb;
    adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
    
    printf("  Memory usage: ~%lu MB\n", memory_mb);
    assert(memory_mb == 9600);  // 96 × 100 MB
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 9: Create adaptive pool with 128 cores (clamped to 96)
 */
bool test_create_128_cores() {
    printf("Creating adaptive pool with 128 cores (should clamp to 96)...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 128);
    if (!pool) return false;
    
    // Should be clamped to 96 (88D maximum)
    assert(pool->max_physical_threads == 96);
    
    printf("  Clamped to: %u cores\n", pool->max_physical_threads);
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 10: Auto-detect cores
 */
bool test_auto_detect() {
    printf("Creating adaptive pool with auto-detection...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 0);
    if (!pool) return false;
    
    uint32_t detected = pool->max_physical_threads;
    printf("  Auto-detected: %u cores\n", detected);
    
    assert(detected >= 1);
    assert(detected <= 96);
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 11: Configuration with memory limit
 */
bool test_memory_limit() {
    printf("Testing configuration with memory limit...\n");
    
    AdaptiveThreadingConfig config = adaptive_get_default_config();
    config.max_physical_threads = 16;
    config.memory_limit_mb = 800;  // Limit to 800 MB
    config.memory_pool_size_mb = 100;
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_configured(2, &config);
    if (!pool) return false;
    
    // Should be reduced to 8 threads (800 MB / 100 MB per thread)
    printf("  Requested: 16 threads\n");
    printf("  Memory limit: 800 MB\n");
    printf("  Actual: %u threads\n", pool->max_physical_threads);
    
    assert(pool->max_physical_threads == 8);
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 12: Print statistics
 */
bool test_print_statistics() {
    printf("Testing statistics printing...\n");
    
    HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, 8);
    if (!pool) return false;
    
    adaptive_print_statistics(pool);
    
    hierarchical_thread_pool_free(pool);
    return true;
}

/**
 * Test 13: Scalability comparison
 */
bool test_scalability_comparison() {
    printf("Testing scalability across different core counts...\n");
    
    uint32_t core_counts[] = {1, 2, 4, 8, 16, 32, 64, 96};
    int num_configs = sizeof(core_counts) / sizeof(core_counts[0]);
    
    printf("\n");
    printf("┌────────┬─────────────┬──────────────┬────────────┐\n");
    printf("│ Cores  │   Memory    │  Reduction   │ Efficiency │\n");
    printf("├────────┼─────────────┼──────────────┼────────────┤\n");
    
    for (int i = 0; i < num_configs; i++) {
        uint32_t cores = core_counts[i];
        HierarchicalThreadPool* pool = hierarchical_thread_pool_create_88d_adaptive(2, cores);
        
        if (pool) {
            uint32_t logical, physical;
            uint64_t work_stolen, memory_mb;
            adaptive_get_statistics(pool, &logical, &physical, &work_stolen, &memory_mb);
            
            double reduction = 9216.0 / memory_mb;
            double efficiency = (double)logical / physical * 100.0 / 96.0;
            
            printf("│ %6u │ %7lu MB │ %10.1fx │   %6.1f%% │\n",
                   cores, memory_mb, reduction, efficiency * 100.0);
            
            hierarchical_thread_pool_free(pool);
        }
    }
    
    printf("└────────┴─────────────┴──────────────┴────────────┘\n");
    printf("\n");
    
    return true;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║    Adaptive Threading Scalability Test Suite (4-128 cores)    ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    TEST(system_detection);
    TEST(create_1_core);
    TEST(create_4_cores);
    TEST(create_8_cores);
    TEST(create_16_cores);
    TEST(create_32_cores);
    TEST(create_64_cores);
    TEST(create_96_cores);
    TEST(create_128_cores);
    TEST(auto_detect);
    TEST(memory_limit);
    TEST(print_statistics);
    TEST(scalability_comparison);
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                        TEST SUMMARY                            ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                             ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d ✓                                           ║\n", tests_passed);
    printf("║  Failed:       %3d ✗                                           ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                            ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}