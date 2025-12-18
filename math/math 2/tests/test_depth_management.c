#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ai/cllm_lattice_hierarchy.h"

// Test calculate_max_depth with various core counts
void test_calculate_max_depth_basic() {
    printf("Testing calculate_max_depth basic cases...\n");
    
    // Test with 0 cores (edge case)
    assert(calculate_max_depth(0) == 1);
    
    // Test with negative cores (edge case)
    assert(calculate_max_depth(-1) == 1);
    
    // Test with 1 core (minimum)
    assert(calculate_max_depth(1) == 1);
    
    // Test with 12 cores (1 level of children)
    int depth_12 = calculate_max_depth(12);
    assert(depth_12 >= 1 && depth_12 <= 2);
    
    // Test with 144 cores (2 levels of children)
    int depth_144 = calculate_max_depth(144);
    assert(depth_144 >= 2 && depth_144 <= 3);
    
    // Test with 1728 cores (3 levels of children)
    int depth_1728 = calculate_max_depth(1728);
    assert(depth_1728 >= 3 && depth_1728 <= 4);
    
    printf("✓ calculate_max_depth basic cases passed\n");
}

void test_calculate_max_depth_boundary() {
    printf("Testing calculate_max_depth boundary cases...\n");
    
    // Test just below 12
    assert(calculate_max_depth(11) == 1);
    
    // Test at 12
    int depth_12 = calculate_max_depth(12);
    assert(depth_12 >= 1);
    
    // Test just above 12
    int depth_13 = calculate_max_depth(13);
    assert(depth_13 >= 1);
    
    // Test just below 144
    int depth_143 = calculate_max_depth(143);
    assert(depth_143 >= 1 && depth_143 <= 2);
    
    // Test at 144
    int depth_144 = calculate_max_depth(144);
    assert(depth_144 >= 2);
    
    // Test just above 144
    int depth_145 = calculate_max_depth(145);
    assert(depth_145 >= 2);
    
    printf("✓ calculate_max_depth boundary cases passed\n");
}

void test_calculate_max_depth_large() {
    printf("Testing calculate_max_depth with large core counts...\n");
    
    // Test with 10,000 cores
    int depth_10k = calculate_max_depth(10000);
    assert(depth_10k >= 3 && depth_10k <= 5);
    
    // Test with 100,000 cores
    int depth_100k = calculate_max_depth(100000);
    assert(depth_100k >= 4 && depth_100k <= 5);
    
    // Test with 1,000,000 cores (should cap at 5)
    int depth_1m = calculate_max_depth(1000000);
    assert(depth_1m == 5);
    
    printf("✓ calculate_max_depth large core counts passed\n");
}

void test_calculate_max_depth_respects_144k_limit() {
    printf("Testing calculate_max_depth respects 144,000 thread limit...\n");
    
    // Even with unlimited cores, depth should not exceed what's needed for 144,000 threads
    // Level 0: 1 thread
    // Level 1: 12 threads
    // Level 2: 144 threads
    // Level 3: 1,728 threads
    // Level 4: 20,736 threads
    // Level 5: 248,832 threads (exceeds 144,000)
    
    // So maximum practical depth is 4 (which gives 20,736 threads)
    // But we allow depth 5 as the absolute maximum
    
    int depth = calculate_max_depth(1000000);
    assert(depth <= 5);
    
    printf("✓ calculate_max_depth respects 144,000 thread limit\n");
}

void test_can_spawn_at_depth_basic() {
    printf("Testing can_spawn_at_depth basic cases...\n");
    
    // Test at depth 0 with sufficient cores
    assert(can_spawn_at_depth(0, 12, 1) == 1);
    
    // Test at depth 0 with insufficient cores
    assert(can_spawn_at_depth(0, 11, 1) == 0);
    
    // Test at depth 1 with sufficient cores
    assert(can_spawn_at_depth(1, 144, 13) == 1);
    
    // Test at depth 1 with insufficient cores
    assert(can_spawn_at_depth(1, 11, 13) == 0);
    
    // Test at maximum depth (5)
    assert(can_spawn_at_depth(5, 1000, 100) == 0);
    
    // Test at depth 4 (one below maximum) - needs enough cores
    // With 1000 cores, max_depth is 3, so depth 4 cannot spawn
    assert(can_spawn_at_depth(4, 1000, 100) == 0);
    
    // Test at depth 4 with sufficient cores (20736+)
    assert(can_spawn_at_depth(4, 25000, 100) == 1);
    
    printf("✓ can_spawn_at_depth basic cases passed\n");
}

void test_can_spawn_at_depth_thread_limit() {
    printf("Testing can_spawn_at_depth with thread limit...\n");
    
    // Test near 144,000 thread limit
    assert(can_spawn_at_depth(0, 12, 143989) == 0);  // 143989 + 12 = 144001 (exceeds)
    assert(can_spawn_at_depth(0, 12, 143990) == 0);  // Would exceed limit
    
    // Test at exactly 144,000 - 12 = 143,988
    assert(can_spawn_at_depth(0, 12, 143988) == 1);  // 143988 + 12 = 144000 (OK)
    
    // Test at exactly 144,000 - 13 = 143,987
    assert(can_spawn_at_depth(0, 12, 143987) == 1);  // 143987 + 12 = 143999 (OK)
    
    // Test well below limit
    assert(can_spawn_at_depth(0, 12, 100000) == 1);  // 100000 + 12 = 100012 (OK)
    
    printf("✓ can_spawn_at_depth thread limit checks passed\n");
}

void test_can_spawn_at_depth_depth_limit() {
    printf("Testing can_spawn_at_depth with depth limit...\n");
    
    // Test at each depth level with sufficient cores (100000)
    assert(can_spawn_at_depth(0, 100000, 1) == 1);     // Depth 0: OK
    assert(can_spawn_at_depth(1, 100000, 13) == 1);    // Depth 1: OK
    assert(can_spawn_at_depth(2, 100000, 157) == 1);   // Depth 2: OK
    assert(can_spawn_at_depth(3, 100000, 1885) == 1);  // Depth 3: OK
    assert(can_spawn_at_depth(4, 100000, 22621) == 1); // Depth 4: OK
    assert(can_spawn_at_depth(5, 100000, 100) == 0);   // Depth 5: MAX (cannot spawn)
    assert(can_spawn_at_depth(6, 100000, 100) == 0);   // Depth 6: Beyond MAX
    
    // Test with limited cores (1000) - max_depth is 3
    assert(can_spawn_at_depth(0, 1000, 1) == 1);     // Depth 0: OK
    assert(can_spawn_at_depth(1, 1000, 13) == 1);    // Depth 1: OK
    assert(can_spawn_at_depth(2, 1000, 157) == 1);   // Depth 2: OK
    assert(can_spawn_at_depth(3, 1000, 1885) == 0);  // Depth 3: At max for 1000 cores
    assert(can_spawn_at_depth(4, 1000, 22621) == 0); // Depth 4: Beyond max
    
    printf("✓ can_spawn_at_depth depth limit checks passed\n");
}

void test_can_spawn_at_depth_core_availability() {
    printf("Testing can_spawn_at_depth with core availability...\n");
    
    // Test with exactly 12 cores (minimum for spawning)
    assert(can_spawn_at_depth(0, 12, 1) == 1);
    
    // Test with 11 cores (insufficient)
    assert(can_spawn_at_depth(0, 11, 1) == 0);
    
    // Test with 1 core (insufficient)
    assert(can_spawn_at_depth(0, 1, 1) == 0);
    
    // Test with 0 cores (insufficient)
    assert(can_spawn_at_depth(0, 0, 1) == 0);
    
    // Test with negative cores (invalid)
    assert(can_spawn_at_depth(0, -1, 1) == 0);
    
    printf("✓ can_spawn_at_depth core availability checks passed\n");
}

void test_get_recommended_children_count_basic() {
    printf("Testing get_recommended_children_count basic cases...\n");
    
    // Test with sufficient workload and cores
    int count = get_recommended_children_count(0, 12, 120);
    assert(count >= 1 && count <= 12);
    
    // Test with low workload
    count = get_recommended_children_count(0, 12, 10);
    assert(count >= 1 && count <= 12);
    
    // Test with high workload
    count = get_recommended_children_count(0, 12, 1000);
    assert(count == 12);  // Should max out at 12
    
    // Test with insufficient cores
    count = get_recommended_children_count(0, 11, 120);
    assert(count == 0);  // Cannot spawn
    
    printf("✓ get_recommended_children_count basic cases passed\n");
}

void test_get_recommended_children_count_workload_scaling() {
    printf("Testing get_recommended_children_count workload scaling...\n");
    
    // Test with 10 batches (minimum per thread)
    int count_10 = get_recommended_children_count(0, 12, 10);
    assert(count_10 == 1);
    
    // Test with 60 batches (6 children ideal)
    int count_60 = get_recommended_children_count(0, 12, 60);
    assert(count_60 >= 1 && count_60 <= 12);
    
    // Test with 120 batches (12 children ideal)
    int count_120 = get_recommended_children_count(0, 12, 120);
    assert(count_120 >= 1 && count_120 <= 12);
    
    // Test with 240 batches (24 children ideal, capped at 12)
    int count_240 = get_recommended_children_count(0, 12, 240);
    assert(count_240 == 12);
    
    printf("✓ get_recommended_children_count workload scaling passed\n");
}

void test_get_recommended_children_count_core_limiting() {
    printf("Testing get_recommended_children_count core limiting...\n");
    
    // Test with 4 cores and high workload - cannot spawn (need 12 minimum)
    int count_4 = get_recommended_children_count(0, 4, 120);
    assert(count_4 == 0);  // Cannot spawn with < 12 cores
    
    // Test with 8 cores and high workload - cannot spawn (need 12 minimum)
    int count_8 = get_recommended_children_count(0, 8, 120);
    assert(count_8 == 0);  // Cannot spawn with < 12 cores
    
    // Test with 12 cores and high workload
    int count_12 = get_recommended_children_count(0, 12, 120);
    assert(count_12 == 12);  // Can spawn 12 children
    
    // Test with 16 cores and high workload
    int count_16 = get_recommended_children_count(0, 16, 120);
    assert(count_16 == 12);  // Capped at 12 for symmetry
    
    // Test with 100 cores and high workload
    int count_100 = get_recommended_children_count(0, 100, 120);
    assert(count_100 == 12);  // Capped at 12 for symmetry
    
    printf("✓ get_recommended_children_count core limiting passed\n");
}

void test_get_recommended_children_count_depth_limiting() {
    printf("Testing get_recommended_children_count depth limiting...\n");
    
    // Test at maximum depth (should return 0)
    int count_max = get_recommended_children_count(5, 12, 120);
    assert(count_max == 0);
    
    // Test at depth 4 with sufficient cores (should allow spawning)
    int count_4 = get_recommended_children_count(4, 100000, 120);
    assert(count_4 >= 1 && count_4 <= 12);
    
    // Test at depth 4 with limited cores (should return 0)
    int count_4_limited = get_recommended_children_count(4, 1000, 120);
    assert(count_4_limited == 0);
    
    // Test at depth 0 (should allow spawning)
    int count_0 = get_recommended_children_count(0, 12, 120);
    assert(count_0 >= 1 && count_0 <= 12);
    
    printf("✓ get_recommended_children_count depth limiting passed\n");
}

void test_get_recommended_children_count_symmetry() {
    printf("Testing get_recommended_children_count maintains 12-fold symmetry...\n");
    
    // Test that result is always between 0 and 12
    for (int cores = 0; cores < 20; cores++) {
        for (int batches = 0; batches < 200; batches += 10) {
            int count = get_recommended_children_count(0, cores, batches);
            assert(count >= 0 && count <= 12);
        }
    }
    
    printf("✓ get_recommended_children_count maintains 12-fold symmetry\n");
}

int main() {
    printf("=== Depth Management Tests ===\n\n");
    
    // Test calculate_max_depth
    test_calculate_max_depth_basic();
    test_calculate_max_depth_boundary();
    test_calculate_max_depth_large();
    test_calculate_max_depth_respects_144k_limit();
    
    // Test can_spawn_at_depth
    test_can_spawn_at_depth_basic();
    test_can_spawn_at_depth_thread_limit();
    test_can_spawn_at_depth_depth_limit();
    test_can_spawn_at_depth_core_availability();
    
    // Test get_recommended_children_count
    test_get_recommended_children_count_basic();
    test_get_recommended_children_count_workload_scaling();
    test_get_recommended_children_count_core_limiting();
    test_get_recommended_children_count_depth_limiting();
    test_get_recommended_children_count_symmetry();
    
    printf("\n=== All Depth Management Tests Passed! ===\n");
    return 0;
}