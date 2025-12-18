/**
 * Test Cache Optimization System
 * 
 * Tests θ-based cache mapping and CPU affinity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "ai/cllm_cache_optimization.h"
#include "prime_types.h"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("TEST %d: %s\n", tests_run, name); \
    } while(0)

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ✗ FAILED: %s\n", message); \
            return; \
        } \
    } while(0)

#define PASS() \
    do { \
        tests_passed++; \
        printf("  ✓ PASSED\n"); \
    } while(0)

// ============================================================================
// TEST SUITE 1: Cache Line Mapping
// ============================================================================

void test_theta_to_cache_line_basic() {
    TEST("Basic theta to cache line mapping");
    
    // Test θ = 0
    uint32_t cache_line = map_theta_to_cache_line(0.0);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    ASSERT(cache_line == 0, "θ=0 should map to cache line 0");
    
    // Test θ = π
    cache_line = map_theta_to_cache_line(MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    ASSERT(cache_line == NUM_CACHE_LINES / 2, "θ=π should map to middle cache line");
    
    // Test θ = 2π
    cache_line = map_theta_to_cache_line(2.0 * MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    
    PASS();
}

void test_theta_to_cache_line_bounds() {
    TEST("Theta to cache line bounds checking");
    
    // Test various theta values
    for (int i = 0; i < 100; i++) {
        double theta = (i / 100.0) * 2.0 * MATH_PI;
        uint32_t cache_line = map_theta_to_cache_line(theta);
        ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should always be within bounds");
    }
    
    PASS();
}

void test_theta_to_cache_line_negative() {
    TEST("Theta to cache line with negative values");
    
    // Test negative theta (should be normalized)
    uint32_t cache_line = map_theta_to_cache_line(-MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    
    cache_line = map_theta_to_cache_line(-2.0 * MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    
    PASS();
}

void test_theta_to_cache_line_large() {
    TEST("Theta to cache line with large values");
    
    // Test large theta (should be normalized)
    uint32_t cache_line = map_theta_to_cache_line(10.0 * MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    
    cache_line = map_theta_to_cache_line(100.0 * MATH_PI);
    ASSERT(cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    
    PASS();
}

// ============================================================================
// TEST SUITE 2: NUMA Node Mapping
// ============================================================================

void test_theta_to_numa_node() {
    TEST("Theta to NUMA node mapping");
    
    int num_numa_nodes = 4;
    
    // Test θ = 0
    uint32_t numa_node = map_theta_to_numa_node(0.0, num_numa_nodes);
    ASSERT(numa_node < (uint32_t)num_numa_nodes, "NUMA node should be within bounds");
    ASSERT(numa_node == 0, "θ=0 should map to NUMA node 0");
    
    // Test θ = π
    numa_node = map_theta_to_numa_node(MATH_PI, num_numa_nodes);
    ASSERT(numa_node < (uint32_t)num_numa_nodes, "NUMA node should be within bounds");
    
    // Test θ = 2π
    numa_node = map_theta_to_numa_node(2.0 * MATH_PI, num_numa_nodes);
    ASSERT(numa_node < (uint32_t)num_numa_nodes, "NUMA node should be within bounds");
    
    PASS();
}

void test_numa_node_single() {
    TEST("NUMA node mapping with single node");
    
    // With 1 NUMA node, should always return 0
    uint32_t numa_node = map_theta_to_numa_node(0.0, 1);
    ASSERT(numa_node == 0, "Single NUMA node should always return 0");
    
    numa_node = map_theta_to_numa_node(MATH_PI, 1);
    ASSERT(numa_node == 0, "Single NUMA node should always return 0");
    
    PASS();
}

void test_numa_node_distribution() {
    TEST("NUMA node distribution is uniform");
    
    int num_numa_nodes = 4;
    int counts[4] = {0};
    
    // Test 400 theta values
    for (int i = 0; i < 400; i++) {
        double theta = (i / 400.0) * 2.0 * MATH_PI;
        uint32_t numa_node = map_theta_to_numa_node(theta, num_numa_nodes);
        counts[numa_node]++;
    }
    
    // Each NUMA node should get approximately 100 threads
    for (int i = 0; i < num_numa_nodes; i++) {
        ASSERT(counts[i] >= 90 && counts[i] <= 110, "NUMA nodes should be evenly distributed");
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 3: Cache Proximity
// ============================================================================

void test_cache_proximity_same() {
    TEST("Cache proximity for same theta");
    
    double proximity = calculate_cache_proximity(0.0, 0.0);
    ASSERT(proximity == 0.0, "Same theta should have zero proximity");
    
    proximity = calculate_cache_proximity(MATH_PI, MATH_PI);
    ASSERT(proximity == 0.0, "Same theta should have zero proximity");
    
    PASS();
}

void test_cache_proximity_opposite() {
    TEST("Cache proximity for opposite theta");
    
    double proximity = calculate_cache_proximity(0.0, MATH_PI);
    ASSERT(proximity == 1.0, "Opposite theta should have proximity 1.0");
    
    proximity = calculate_cache_proximity(MATH_PI / 2.0, 3.0 * MATH_PI / 2.0);
    ASSERT(proximity == 1.0, "Opposite theta should have proximity 1.0");
    
    PASS();
}

void test_cache_proximity_bounds() {
    TEST("Cache proximity bounds");
    
    // Test various theta pairs
    for (int i = 0; i < 100; i++) {
        double theta1 = (i / 100.0) * 2.0 * MATH_PI;
        for (int j = 0; j < 100; j++) {
            double theta2 = (j / 100.0) * 2.0 * MATH_PI;
            double proximity = calculate_cache_proximity(theta1, theta2);
            ASSERT(proximity >= 0.0 && proximity <= 1.0, "Proximity should be in [0, 1]");
        }
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 4: CPU Affinity
// ============================================================================

void test_cpu_affinity_mask_creation() {
    TEST("CPU affinity mask creation");
    
    CPUAffinityMask mask = create_cpu_affinity_mask(0, 0);
    
    ASSERT(mask.num_cpus > 0, "Should have at least one CPU");
    ASSERT(mask.preferred_cpu >= 0, "Preferred CPU should be non-negative");
    
    PASS();
}

void test_cpu_affinity_different_cache_lines() {
    TEST("Different cache lines get different CPU preferences");
    
    CPUAffinityMask mask1 = create_cpu_affinity_mask(0, 0);
    CPUAffinityMask mask2 = create_cpu_affinity_mask(100, 0);
    
    // Different cache lines may get different preferred CPUs
    // (depending on system configuration)
    ASSERT(mask1.preferred_cpu >= 0, "Preferred CPU should be valid");
    ASSERT(mask2.preferred_cpu >= 0, "Preferred CPU should be valid");
    
    PASS();
}

void test_get_num_cpu_cores() {
    TEST("Get number of CPU cores");
    
    int num_cores = get_num_cpu_cores();
    ASSERT(num_cores > 0, "Should have at least one CPU core");
    ASSERT(num_cores <= 1024, "CPU count should be reasonable");
    
    printf("    Detected %d CPU cores\n", num_cores);
    
    PASS();
}

void test_get_num_numa_nodes() {
    TEST("Get number of NUMA nodes");
    
    int num_nodes = get_num_numa_nodes();
    ASSERT(num_nodes > 0, "Should have at least one NUMA node");
    ASSERT(num_nodes <= MAX_NUMA_NODES, "NUMA nodes should be within limit");
    
    printf("    Detected %d NUMA nodes\n", num_nodes);
    
    PASS();
}

// ============================================================================
// TEST SUITE 5: Cache Placement
// ============================================================================

void test_cache_placement_basic() {
    TEST("Basic cache placement calculation");
    
    CachePlacement placement = calculate_cache_placement(0.0, 0);
    
    ASSERT(placement.cache_line < NUM_CACHE_LINES, "Cache line should be within bounds");
    ASSERT(placement.numa_node < MAX_NUMA_NODES, "NUMA node should be within bounds");
    ASSERT(placement.cpu_core >= 0, "CPU core should be non-negative");
    ASSERT(placement.theta == 0.0, "Theta should match input");
    ASSERT(placement.cache_hits == 0, "Initial cache hits should be 0");
    ASSERT(placement.cache_misses == 0, "Initial cache misses should be 0");
    
    PASS();
}

void test_cache_placement_validation() {
    TEST("Cache placement validation");
    
    CachePlacement placement = calculate_cache_placement(MATH_PI, 5);
    
    int valid = validate_cache_placement(&placement);
    ASSERT(valid == 1, "Valid placement should pass validation");
    
    PASS();
}

void test_cache_placement_invalid() {
    TEST("Invalid cache placement detection");
    
    CachePlacement placement;
    placement.cache_line = NUM_CACHE_LINES + 100;  // Invalid
    placement.numa_node = 0;
    placement.cpu_core = 0;
    placement.theta = 0.0;
    placement.cache_hits = 0;
    placement.cache_misses = 0;
    
    int valid = validate_cache_placement(&placement);
    ASSERT(valid == 0, "Invalid placement should fail validation");
    
    PASS();
}

// ============================================================================
// TEST SUITE 6: Cache Statistics
// ============================================================================

void test_cache_stats_update() {
    TEST("Cache statistics update");
    
    CachePlacement placement = calculate_cache_placement(0.0, 0);
    
    // Update with hits
    update_cache_stats(&placement, 1);
    update_cache_stats(&placement, 1);
    update_cache_stats(&placement, 1);
    
    ASSERT(placement.cache_hits == 3, "Cache hits should be 3");
    ASSERT(placement.cache_misses == 0, "Cache misses should be 0");
    
    // Update with misses
    update_cache_stats(&placement, 0);
    update_cache_stats(&placement, 0);
    
    ASSERT(placement.cache_hits == 3, "Cache hits should still be 3");
    ASSERT(placement.cache_misses == 2, "Cache misses should be 2");
    
    PASS();
}

void test_cache_hit_rate() {
    TEST("Cache hit rate calculation");
    
    CachePlacement placement = calculate_cache_placement(0.0, 0);
    
    // Initially 0
    double hit_rate = get_cache_hit_rate(&placement);
    ASSERT(hit_rate == 0.0, "Initial hit rate should be 0");
    
    // Add some hits and misses
    for (int i = 0; i < 80; i++) {
        update_cache_stats(&placement, 1);  // Hit
    }
    for (int i = 0; i < 20; i++) {
        update_cache_stats(&placement, 0);  // Miss
    }
    
    hit_rate = get_cache_hit_rate(&placement);
    ASSERT(hit_rate == 0.8, "Hit rate should be 80%");
    
    PASS();
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("=== Cache Optimization Tests ===\n\n");
    
    // Suite 1: Cache Line Mapping
    printf("Suite 1: Cache Line Mapping\n");
    test_theta_to_cache_line_basic();
    test_theta_to_cache_line_bounds();
    test_theta_to_cache_line_negative();
    test_theta_to_cache_line_large();
    printf("\n");
    
    // Suite 2: NUMA Node Mapping
    printf("Suite 2: NUMA Node Mapping\n");
    test_theta_to_numa_node();
    test_numa_node_single();
    test_numa_node_distribution();
    printf("\n");
    
    // Suite 3: Cache Proximity
    printf("Suite 3: Cache Proximity\n");
    test_cache_proximity_same();
    test_cache_proximity_opposite();
    test_cache_proximity_bounds();
    printf("\n");
    
    // Suite 4: CPU Affinity
    printf("Suite 4: CPU Affinity\n");
    test_cpu_affinity_mask_creation();
    test_cpu_affinity_different_cache_lines();
    test_get_num_cpu_cores();
    test_get_num_numa_nodes();
    printf("\n");
    
    // Suite 5: Cache Placement
    printf("Suite 5: Cache Placement\n");
    test_cache_placement_basic();
    test_cache_placement_validation();
    test_cache_placement_invalid();
    printf("\n");
    
    // Suite 6: Cache Statistics
    printf("Suite 6: Cache Statistics\n");
    test_cache_stats_update();
    test_cache_hit_rate();
    printf("\n");
    
    // Summary
    printf("=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("\n✓ All tests passed!\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed!\n");
        return 1;
    }
}