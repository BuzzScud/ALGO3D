/**
 * Test Clock-Based Memory Mapping
 * 
 * Tests the Babylonian clock memory mapping system for thread-to-memory mapping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clock_lattice.h"

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
// TEST SUITE 1: Ring Structure
// ============================================================================

void test_ring_positions() {
    TEST("Ring positions match Babylonian structure");
    
    ASSERT(get_ring_positions(0) == 12, "Ring 0 should have 12 positions (hours)");
    ASSERT(get_ring_positions(1) == 60, "Ring 1 should have 60 positions (minutes)");
    ASSERT(get_ring_positions(2) == 60, "Ring 2 should have 60 positions (seconds)");
    ASSERT(get_ring_positions(3) == 100, "Ring 3 should have 100 positions (milliseconds)");
    ASSERT(get_ring_positions(4) == 1000, "Ring 4+ should have 1000 positions");
    ASSERT(get_ring_positions(10) == 1000, "Ring 10 should have 1000 positions");
    
    PASS();
}

void test_cumulative_positions() {
    TEST("Cumulative positions calculation");
    
    ASSERT(get_cumulative_positions(0) == 12, "Ring 0: 12 total");
    ASSERT(get_cumulative_positions(1) == 72, "Ring 1: 12 + 60 = 72 total");
    ASSERT(get_cumulative_positions(2) == 132, "Ring 2: 72 + 60 = 132 total");
    ASSERT(get_cumulative_positions(3) == 232, "Ring 3: 132 + 100 = 232 total");
    ASSERT(get_cumulative_positions(4) == 1232, "Ring 4: 232 + 1000 = 1232 total");
    ASSERT(get_cumulative_positions(5) == 2232, "Ring 5: 1232 + 1000 = 2232 total");
    
    PASS();
}

void test_ring_structure_consistency() {
    TEST("Ring structure consistency");
    
    // Verify cumulative positions match sum of ring positions
    uint32_t sum = 0;
    for (uint32_t ring = 0; ring <= 5; ring++) {
        sum += get_ring_positions(ring);
        uint32_t cumulative = get_cumulative_positions(ring);
        ASSERT(cumulative == sum, "Cumulative should match sum of ring positions");
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 2: Clock Position Calculation
// ============================================================================

void test_clock_position_ring0() {
    TEST("Clock position calculation for ring 0 (hours)");
    
    // Ring 0 has 12 positions
    for (int thread_id = 0; thread_id < 24; thread_id++) {
        uint32_t pos = calculate_thread_clock_position(thread_id, 0);
        ASSERT(pos < 12, "Position should be in range [0, 11]");
        ASSERT(pos == (uint32_t)(thread_id % 12), "Position should wrap around modulo 12");
    }
    
    PASS();
}

void test_clock_position_ring1() {
    TEST("Clock position calculation for ring 1 (minutes)");
    
    // Ring 1 has 60 positions
    for (int thread_id = 0; thread_id < 120; thread_id++) {
        uint32_t pos = calculate_thread_clock_position(thread_id, 1);
        ASSERT(pos < 60, "Position should be in range [0, 59]");
        ASSERT(pos == (uint32_t)(thread_id % 60), "Position should wrap around modulo 60");
    }
    
    PASS();
}

void test_clock_position_ring3() {
    TEST("Clock position calculation for ring 3 (milliseconds)");
    
    // Ring 3 has 100 positions
    for (int thread_id = 0; thread_id < 200; thread_id++) {
        uint32_t pos = calculate_thread_clock_position(thread_id, 3);
        ASSERT(pos < 100, "Position should be in range [0, 99]");
        ASSERT(pos == (uint32_t)(thread_id % 100), "Position should wrap around modulo 100");
    }
    
    PASS();
}

void test_clock_position_distribution() {
    TEST("Clock position distribution is uniform");
    
    // Test that positions are evenly distributed
    int counts[12] = {0};
    for (int thread_id = 0; thread_id < 120; thread_id++) {
        uint32_t pos = calculate_thread_clock_position(thread_id, 0);
        counts[pos]++;
    }
    
    // Each position should appear exactly 10 times
    for (int i = 0; i < 12; i++) {
        ASSERT(counts[i] == 10, "Each position should appear equally");
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 3: Memory Offset Calculation
// ============================================================================

void test_memory_offset_ring0() {
    TEST("Memory offset calculation for ring 0");
    
    size_t total_memory = 1024 * 1024;  // 1 MB
    
    // Ring 0 has 12 positions total
    size_t memory_per_position = total_memory / 12;
    
    for (uint32_t pos = 0; pos < 12; pos++) {
        size_t offset = calculate_memory_offset(0, pos, total_memory);
        ASSERT(offset == pos * memory_per_position, "Offset should be position * memory_per_position");
    }
    
    PASS();
}

void test_memory_offset_ring1() {
    TEST("Memory offset calculation for ring 1");
    
    size_t total_memory = 1024 * 1024;  // 1 MB
    
    // Ring 1: positions 12-71 (60 positions)
    size_t memory_per_position = total_memory / 72;  // Total positions up to ring 1
    
    for (uint32_t pos = 0; pos < 60; pos++) {
        size_t offset = calculate_memory_offset(1, pos, total_memory);
        size_t expected = (12 + pos) * memory_per_position;
        ASSERT(offset == expected, "Offset should account for previous ring positions");
    }
    
    PASS();
}

void test_memory_offset_no_overlap() {
    TEST("Memory offsets do not overlap");
    
    size_t total_memory = 1024 * 1024;
    
    // Test ring 0
    for (uint32_t pos = 0; pos < 11; pos++) {
        size_t offset1 = calculate_memory_offset(0, pos, total_memory);
        size_t offset2 = calculate_memory_offset(0, pos + 1, total_memory);
        ASSERT(offset2 > offset1, "Offsets should be strictly increasing");
    }
    
    PASS();
}

void test_memory_offset_bounds() {
    TEST("Memory offsets stay within bounds");
    
    size_t total_memory = 1024 * 1024;
    
    // Test all rings
    for (uint32_t ring = 0; ring <= 5; ring++) {
        uint32_t positions = get_ring_positions(ring);
        for (uint32_t pos = 0; pos < positions; pos++) {
            size_t offset = calculate_memory_offset(ring, pos, total_memory);
            ASSERT(offset < total_memory, "Offset should be within total memory");
        }
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 4: Complete Memory Mapping
// ============================================================================

void test_map_thread_to_memory_basic() {
    TEST("Basic thread to memory mapping");
    
    size_t total_memory = 1024 * 1024;
    
    ClockMemoryPosition mem_pos = map_thread_to_memory(0, 0, total_memory);
    
    ASSERT(mem_pos.ring == 0, "Ring should be 0 for hierarchy level 0");
    ASSERT(mem_pos.position == 0, "Position should be 0 for thread 0");
    ASSERT(mem_pos.memory_offset == 0, "Offset should be 0 for first position");
    ASSERT(mem_pos.segment_size > 0, "Segment size should be positive");
    
    PASS();
}

void test_map_thread_to_memory_hierarchy() {
    TEST("Thread mapping respects hierarchy levels");
    
    size_t total_memory = 1024 * 1024;
    
    // Test different hierarchy levels
    for (int level = 0; level <= 3; level++) {
        ClockMemoryPosition mem_pos = map_thread_to_memory(0, level, total_memory);
        ASSERT(mem_pos.ring == (uint32_t)level, "Ring should match hierarchy level");
    }
    
    PASS();
}

void test_map_thread_to_memory_unique() {
    TEST("Different threads get different memory positions");
    
    size_t total_memory = 1024 * 1024;
    int hierarchy_level = 0;
    
    ClockMemoryPosition pos1 = map_thread_to_memory(0, hierarchy_level, total_memory);
    ClockMemoryPosition pos2 = map_thread_to_memory(1, hierarchy_level, total_memory);
    
    ASSERT(pos1.position != pos2.position || pos1.ring != pos2.ring,
           "Different threads should get different positions");
    ASSERT(pos1.memory_offset != pos2.memory_offset,
           "Different threads should get different memory offsets");
    
    PASS();
}

void test_map_thread_to_memory_segment_size() {
    TEST("Segment sizes are consistent");
    
    size_t total_memory = 1024 * 1024;
    
    // All threads at same hierarchy level should have same segment size
    ClockMemoryPosition pos1 = map_thread_to_memory(0, 0, total_memory);
    ClockMemoryPosition pos2 = map_thread_to_memory(5, 0, total_memory);
    
    ASSERT(pos1.segment_size == pos2.segment_size,
           "Threads at same level should have same segment size");
    
    PASS();
}

void test_map_thread_to_memory_12_threads() {
    TEST("Mapping 12 threads (12-fold symmetry)");
    
    size_t total_memory = 1024 * 1024;
    int hierarchy_level = 0;
    
    // Map 12 threads (one per symmetry group)
    ClockMemoryPosition positions[12];
    for (int i = 0; i < 12; i++) {
        positions[i] = map_thread_to_memory(i, hierarchy_level, total_memory);
        ASSERT(positions[i].ring == 0, "All should be in ring 0");
        ASSERT(positions[i].position == (uint32_t)i, "Position should match thread ID");
    }
    
    // Verify all positions are unique
    for (int i = 0; i < 12; i++) {
        for (int j = i + 1; j < 12; j++) {
            ASSERT(positions[i].memory_offset != positions[j].memory_offset,
                   "All memory offsets should be unique");
        }
    }
    
    PASS();
}

// ============================================================================
// TEST SUITE 5: Edge Cases
// ============================================================================

void test_large_thread_ids() {
    TEST("Large thread IDs wrap correctly");
    
    size_t total_memory = 1024 * 1024;
    
    // Test large thread IDs
    ClockMemoryPosition pos1 = map_thread_to_memory(1000, 0, total_memory);
    ClockMemoryPosition pos2 = map_thread_to_memory(1000 + 12, 0, total_memory);
    
    ASSERT(pos1.position == pos2.position, "Large thread IDs should wrap around");
    ASSERT(pos1.memory_offset == pos2.memory_offset, "Memory offsets should match for wrapped IDs");
    
    PASS();
}

void test_large_memory_sizes() {
    TEST("Large memory sizes handled correctly");
    
    size_t large_memory = 1024ULL * 1024ULL * 1024ULL;  // 1 GB
    
    ClockMemoryPosition mem_pos = map_thread_to_memory(0, 0, large_memory);
    
    ASSERT(mem_pos.segment_size > 0, "Segment size should be positive");
    ASSERT(mem_pos.memory_offset < large_memory, "Offset should be within bounds");
    
    PASS();
}

void test_different_hierarchy_levels() {
    TEST("Different hierarchy levels use different rings");
    
    size_t total_memory = 1024 * 1024;
    
    ClockMemoryPosition pos0 = map_thread_to_memory(0, 0, total_memory);
    ClockMemoryPosition pos1 = map_thread_to_memory(0, 1, total_memory);
    ClockMemoryPosition pos2 = map_thread_to_memory(0, 2, total_memory);
    
    ASSERT(pos0.ring == 0, "Level 0 should use ring 0");
    ASSERT(pos1.ring == 1, "Level 1 should use ring 1");
    ASSERT(pos2.ring == 2, "Level 2 should use ring 2");
    
    PASS();
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("=== Clock Memory Mapping Tests ===\n\n");
    
    // Suite 1: Ring Structure
    printf("Suite 1: Ring Structure\n");
    test_ring_positions();
    test_cumulative_positions();
    test_ring_structure_consistency();
    printf("\n");
    
    // Suite 2: Clock Position Calculation
    printf("Suite 2: Clock Position Calculation\n");
    test_clock_position_ring0();
    test_clock_position_ring1();
    test_clock_position_ring3();
    test_clock_position_distribution();
    printf("\n");
    
    // Suite 3: Memory Offset Calculation
    printf("Suite 3: Memory Offset Calculation\n");
    test_memory_offset_ring0();
    test_memory_offset_ring1();
    test_memory_offset_no_overlap();
    test_memory_offset_bounds();
    printf("\n");
    
    // Suite 4: Complete Memory Mapping
    printf("Suite 4: Complete Memory Mapping\n");
    test_map_thread_to_memory_basic();
    test_map_thread_to_memory_hierarchy();
    test_map_thread_to_memory_unique();
    test_map_thread_to_memory_segment_size();
    test_map_thread_to_memory_12_threads();
    printf("\n");
    
    // Suite 5: Edge Cases
    printf("Suite 5: Edge Cases\n");
    test_large_thread_ids();
    test_large_memory_sizes();
    test_different_hierarchy_levels();
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