/**
 * @file test_hierarchical_memory.c
 * @brief Comprehensive tests for Hierarchical Memory System
 */

#include "../include/hierarchical_memory.h"
// #include "../../math/include/math/threading_integration.h"  // Not yet implemented
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
// TEST: Basic Creation and Destruction
// ============================================================================

bool test_create_destroy() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024,  // 1 MB
        12,           // 12 segments (12-fold symmetry)
        13,           // 13 dimensions
        1,            // owner_id
        0             // hierarchy_level
    );
    
    if (!block) {
        printf("ERROR: Failed to create memory block\n");
        return false;
    }
    
    // Verify basic properties
    assert(block->num_segments == 12);
    assert(block->num_dimensions == 13);
    assert(block->owner_id == 1);
    assert(block->hierarchy_level == 0);
    assert(block->total_size == 1024 * 1024);
    
    // Verify segments allocated
    for (uint32_t i = 0; i < block->num_segments; i++) {
        assert(block->segments[i].data != NULL);
        assert(block->segments[i].position != NULL);
        assert(block->segments[i].segment_id == i);
    }
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Segment Access
// ============================================================================

bool test_segment_access() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Get segment by ID
    HierarchicalSegment* seg = hierarchical_memory_get_segment(block, 0);
    assert(seg != NULL);
    assert(seg->segment_id == 0);
    
    // Get segment by symmetry group
    seg = hierarchical_memory_get_segment_by_symmetry(block, 5);
    assert(seg != NULL);
    assert(seg->symmetry_group == 5);
    
    // Test read/write
    char test_data[] = "Hello, Hierarchical Memory!";
    size_t written = hierarchical_segment_write(seg, 0, test_data, sizeof(test_data));
    assert(written == sizeof(test_data));
    
    char read_buffer[100] = {0};
    size_t read = hierarchical_segment_read(seg, 0, read_buffer, sizeof(test_data));
    assert(read == sizeof(test_data));
    assert(strcmp(read_buffer, test_data) == 0);
    
    // Verify statistics updated
    assert(atomic_load(&seg->read_count) == 1);
    assert(atomic_load(&seg->write_count) == 1);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Kissing Boundaries
// ============================================================================

bool test_kissing_boundaries() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Create boundary between segments 0 and 1
    KissingBoundary* boundary = hierarchical_boundary_create(block, 0, 1, 1024);
    assert(boundary != NULL);
    assert(boundary->segment_a == 0);
    assert(boundary->segment_b == 1);
    assert(boundary->boundary_size == 1024);
    assert(block->num_boundaries == 1);
    
    // Test write to boundary
    void* write_ptr = hierarchical_boundary_write(boundary, 0);
    assert(write_ptr != NULL);
    
    char test_data[] = "Boundary data";
    memcpy(write_ptr, test_data, sizeof(test_data));
    
    hierarchical_boundary_release(boundary);
    
    // Test read from boundary
    const void* read_ptr = hierarchical_boundary_read(boundary, 1);
    assert(read_ptr != NULL);
    assert(strcmp((const char*)read_ptr, test_data) == 0);
    
    // Verify access counters
    assert(atomic_load(&boundary->accesses_a) == 1);
    assert(atomic_load(&boundary->accesses_b) == 1);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Dynamic Segment Scaling
// ============================================================================

bool test_scale_segments() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    printf("Initial segments: %u\n", block->num_segments);
    
    // Scale segments by 2x
    bool scaled = hierarchical_memory_scale_segments(block, 2);
    assert(scaled == true);
    assert(block->num_segments == 24);
    
    printf("After scaling: %u segments\n", block->num_segments);
    
    // Verify all segments valid
    for (uint32_t i = 0; i < block->num_segments; i++) {
        assert(block->segments[i].data != NULL);
        assert(block->segments[i].position != NULL);
        assert(block->segments[i].segment_id == i);
    }
    
    // Verify scaling history
    assert(block->num_segment_scales == 1);
    assert(block->segment_scale_history[0] == 24);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Dynamic Dimension Scaling
// ============================================================================

bool test_scale_dimensions() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    printf("Initial dimensions: %u\n", block->num_dimensions);
    
    // Scale dimensions by adding 7
    bool scaled = hierarchical_memory_scale_dimensions(block, 7);
    assert(scaled == true);
    assert(block->num_dimensions == 20);
    
    printf("After scaling: %u dimensions\n", block->num_dimensions);
    
    // Verify all segments have new dimensions
    for (uint32_t i = 0; i < block->num_segments; i++) {
        assert(block->segments[i].num_dimensions == 20);
        assert(block->segments[i].position != NULL);
    }
    
    // Verify coprime matrix extended
    assert(block->coprime_matrix != NULL);
    
    // Verify scaling history
    assert(block->num_dimension_scales == 1);
    assert(block->dimension_scale_history[0] == 20);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Geometric Organization
// ============================================================================

bool test_geometric_organization() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Verify all segments have positions
    for (uint32_t i = 0; i < block->num_segments; i++) {
        HierarchicalSegment* seg = &block->segments[i];
        assert(seg->position != NULL);
        
        // Verify position is non-zero (geometric organization applied)
        bool has_nonzero = false;
        for (uint32_t d = 0; d < seg->num_dimensions; d++) {
            if (seg->position[d] != 0.0) {
                has_nonzero = true;
                break;
            }
        }
        assert(has_nonzero);
        
        printf("Segment %u position: [%.3f, %.3f, %.3f, ...]\n",
               i, seg->position[0], seg->position[1], seg->position[2]);
    }
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Coprime Relationships
// ============================================================================

bool test_coprime_relationships() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Verify coprime matrix computed
    assert(block->coprime_matrix != NULL);
    
    // Check diagonal (same dimension should be 1)
    for (uint32_t i = 0; i < block->num_dimensions; i++) {
        assert(block->coprime_matrix[i][i] == 1);
    }
    
    // Test communication check
    bool can_comm = hierarchical_memory_can_communicate(block, 0, 1);
    printf("Segments 0 and 1 can communicate: %s\n", can_comm ? "yes" : "no");
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Statistics
// ============================================================================

bool test_statistics() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Perform some operations
    HierarchicalSegment* seg = hierarchical_memory_get_segment(block, 0);
    char data[100];
    
    for (int i = 0; i < 10; i++) {
        hierarchical_segment_write(seg, 0, data, sizeof(data));
        hierarchical_segment_read(seg, 0, data, sizeof(data));
    }
    
    // Check statistics
    uint64_t reads, writes, hits, misses;
    hierarchical_memory_get_stats(block, &reads, &writes, &hits, &misses);
    
    printf("Statistics: reads=%lu, writes=%lu\n", reads, writes);
    
    // Print full statistics
    hierarchical_memory_print_stats(block);
    
    // Reset statistics
    hierarchical_memory_reset_stats(block);
    
    hierarchical_memory_get_stats(block, &reads, &writes, &hits, &misses);
    assert(reads == 0);
    assert(writes == 0);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Validation
// ============================================================================

bool test_validation() {
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024 * 1024, 12, 13, 1, 0
    );
    
    if (!block) return false;
    
    // Validate block
    int valid = hierarchical_memory_validate(block);
    assert(valid == 1);
    
    printf("Block validation: %s\n", valid ? "VALID" : "INVALID");
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// TEST: Abacus Integration
// ============================================================================

// bool test_abacus_integration() {
//     printf("Testing abacus integration...\n");
//     
//     // Test segment to abacus conversion
//     CrystallineAbacus* abacus = segment_to_abacus(5, 3, 12);
//     assert(abacus != NULL);
//     
//     // Test abacus to segment conversion
//     uint64_t segment_id;
//     uint32_t symmetry_group;
//     MathError err = abacus_to_segment(abacus, &segment_id, &symmetry_group);
//     assert(err == MATH_SUCCESS);
//     assert(segment_id == 5);
//     assert(symmetry_group == 3);
//     
//     printf("Segment 5, Group 3 -> Abacus -> Segment %lu, Group %u\n",
//            segment_id, symmetry_group);
//     
//     abacus_free(abacus);
//     return true;
// }

// ============================================================================
// TEST: Thread Geometry
// ============================================================================

// bool test_thread_geometry() {
//     printf("Testing thread geometry...\n");
//     
//     // Test thread ID to position
//     ClockPosition pos;
//     MathError err = thread_id_to_position(42, 12, &pos);
//     assert(err == MATH_SUCCESS);
//     
//     printf("Thread 42 position: [%u, %u, %u, %u]\n",
//            pos.ring0, pos.ring1, pos.ring2, pos.ring3);
//     
//     // Test position to thread ID
//     uint64_t thread_id;
//     err = position_to_thread_id(&pos, 12, &thread_id);
//     assert(err == MATH_SUCCESS);
//     assert(thread_id == 42);
//     
//     // Test N-dimensional position
//     double nd_pos[13];
//     err = thread_compute_nd_position(42, 13, nd_pos);
//     assert(err == MATH_SUCCESS);
//     
//     printf("Thread 42 N-D position: [%.3f, %.3f, %.3f, ...]\n",
//            nd_pos[0], nd_pos[1], nd_pos[2]);
//     
//     return true;
// }

// ============================================================================
// TEST: Thread Neighbors
// ============================================================================

// bool test_thread_neighbors() {
//     printf("Testing thread neighbors...\n");
//     
//     uint64_t neighbors[12];
//     int count = compute_thread_neighbors(5, 100, 12, neighbors);
//     
//     printf("Thread 5 has %d neighbors: ", count);
//     for (int i = 0; i < count && i < 12; i++) {
//         printf("%lu ", neighbors[i]);
//     }
//     printf("\n");
//     
//     assert(count > 0);
//     assert(count <= 12);
//     
//     return true;
// }

// ============================================================================
// TEST: Multiple Blocks
// ============================================================================

bool test_multiple_blocks() {
    printf("Creating multiple memory blocks...\n");
    
    HierarchicalMemoryBlock* blocks[5];
    
    // Create 5 blocks
    for (int i = 0; i < 5; i++) {
        blocks[i] = hierarchical_memory_create(
            512 * 1024,  // 512 KB each
            12,
            13,
            i,
            i
        );
        assert(blocks[i] != NULL);
        printf("Created block %d: %s\n", i, blocks[i]->block_id);
    }
    
    // Verify all blocks independent
    for (int i = 0; i < 5; i++) {
        assert(blocks[i]->owner_id == i);
        assert(blocks[i]->hierarchy_level == i);
    }
    
    // Destroy all blocks
    for (int i = 0; i < 5; i++) {
        hierarchical_memory_destroy(blocks[i]);
    }
    
    return true;
}

// ============================================================================
// TEST: Stress Test
// ============================================================================

bool test_stress() {
    printf("Running stress test...\n");
    
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        10 * 1024 * 1024,  // 10 MB
        12,
        13,
        1,
        0
    );
    
    if (!block) return false;
    
    // Perform many operations
    for (int i = 0; i < 1000; i++) {
        uint32_t seg_id = i % block->num_segments;
        HierarchicalSegment* seg = hierarchical_memory_get_segment(block, seg_id);
        
        char data[256];
        snprintf(data, sizeof(data), "Test data %d", i);
        
        hierarchical_segment_write(seg, 0, data, strlen(data) + 1);
        
        char read_buf[256];
        hierarchical_segment_read(seg, 0, read_buf, sizeof(read_buf));
        
        assert(strcmp(data, read_buf) == 0);
    }
    
    printf("Completed 1000 read/write operations\n");
    
    hierarchical_memory_print_stats(block);
    
    hierarchical_memory_destroy(block);
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   HIERARCHICAL MEMORY SYSTEM - COMPREHENSIVE TESTS         ║\n");
    printf("║   Inspired by Platonic Solid Generator                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    // Run all tests
    TEST(create_destroy);
    TEST(segment_access);
    TEST(kissing_boundaries);
    TEST(scale_segments);
    TEST(scale_dimensions);
    TEST(geometric_organization);
    TEST(coprime_relationships);
    TEST(statistics);
    TEST(validation);
    // TEST(abacus_integration);  // Threading integration not yet implemented
    // TEST(thread_geometry);      // Threading integration not yet implemented
    // TEST(thread_neighbors);     // Threading integration not yet implemented
    TEST(multiple_blocks);
    TEST(stress);
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                         ║\n", tests_passed + tests_failed);
    printf("║  Passed:       %3d  ✓                                      ║\n", tests_passed);
    printf("║  Failed:       %3d  ✗                                      ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return (tests_failed == 0) ? 0 : 1;
}