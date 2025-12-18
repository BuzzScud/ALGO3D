#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ai/cllm_crystalline_memory.h"

// Test memory block creation
void test_memory_block_creation() {
    printf("Testing crystalline memory block creation...\n");
    
    // Create block with 12 KB (1 KB per segment)
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    assert(block->owner_sphere_id == 1);
    assert(block->hierarchy_level == 0);
    assert(block->total_size > 0);
    assert(block->segment_size > 0);
    
    // Verify all segments are allocated
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        assert(block->segments[i].data != NULL);
        assert(block->segments[i].symmetry_group == (uint32_t)i);
        assert(block->segments[i].size == block->segment_size);
    }
    
    crystalline_memory_destroy(block);
    
    printf("✓ Memory block creation passed\n");
}

// Test memory block with zero size (should fail)
void test_memory_block_zero_size() {
    printf("Testing memory block with zero size...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(0, 1, 0);
    assert(block == NULL);
    
    printf("✓ Zero size handling passed\n");
}

// Test cache alignment
void test_cache_alignment() {
    printf("Testing cache alignment...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(1024, 1, 0);
    assert(block != NULL);
    
    // Check that segment size is cache-aligned
    assert(block->segment_size % CRYSTALLINE_CACHE_LINE == 0);
    
    // Check that segment data pointers are cache-aligned
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        uintptr_t addr = (uintptr_t)block->segments[i].data;
        assert(addr % CRYSTALLINE_CACHE_LINE == 0);
    }
    
    crystalline_memory_destroy(block);
    
    printf("✓ Cache alignment passed\n");
}

// Test segment access
void test_segment_access() {
    printf("Testing segment access...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Get segment 0
    CrystallineSegment* seg = crystalline_memory_get_segment(block, 0);
    assert(seg != NULL);
    assert(seg->symmetry_group == 0);
    
    // Get segment 11
    seg = crystalline_memory_get_segment(block, 11);
    assert(seg != NULL);
    assert(seg->symmetry_group == 11);
    
    // Try invalid segment (should return NULL)
    seg = crystalline_memory_get_segment(block, 12);
    assert(seg == NULL);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Segment access passed\n");
}

// Test segment read/write
void test_segment_read_write() {
    printf("Testing segment read/write...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    CrystallineSegment* seg = crystalline_memory_get_segment(block, 0);
    assert(seg != NULL);
    
    // Write data
    const char* test_data = "Hello, Crystalline Memory!";
    size_t written = crystalline_segment_write(seg, 0, test_data, strlen(test_data) + 1);
    assert(written == strlen(test_data) + 1);
    
    // Read data back
    char buffer[100];
    size_t read = crystalline_segment_read(seg, 0, buffer, sizeof(buffer));
    assert(read > 0);
    assert(strcmp(buffer, test_data) == 0);
    
    // Check statistics
    assert(atomic_load(&seg->read_count) == 1);
    assert(atomic_load(&seg->write_count) == 1);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Segment read/write passed\n");
}

// Test segment bounds checking
void test_segment_bounds() {
    printf("Testing segment bounds checking...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    CrystallineSegment* seg = crystalline_memory_get_segment(block, 0);
    assert(seg != NULL);
    
    // Try to write beyond segment size
    char large_buffer[10000];
    memset(large_buffer, 'A', sizeof(large_buffer));
    
    size_t written = crystalline_segment_write(seg, 0, large_buffer, sizeof(large_buffer));
    // Should be clamped to segment size
    assert(written <= seg->size);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Segment bounds checking passed\n");
}

// Test kissing boundary creation
void test_kissing_boundary_creation() {
    printf("Testing kissing boundary creation...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Create boundary between segments 0 and 1
    KissingBoundary* boundary = crystalline_boundary_create(block, 0, 1, 1024);
    assert(boundary != NULL);
    assert(boundary->segment_a == 0);
    assert(boundary->segment_b == 1);
    assert(boundary->boundary_size == 1024);
    assert(boundary->boundary_memory != NULL);
    
    crystalline_boundary_destroy(boundary);
    crystalline_memory_destroy(block);
    
    printf("✓ Kissing boundary creation passed\n");
}

// Test kissing boundary invalid cases
void test_kissing_boundary_invalid() {
    printf("Testing kissing boundary invalid cases...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Try to create boundary with same segment
    KissingBoundary* boundary = crystalline_boundary_create(block, 0, 0, 1024);
    assert(boundary == NULL);
    
    // Try to create boundary with invalid segment
    boundary = crystalline_boundary_create(block, 0, 12, 1024);
    assert(boundary == NULL);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Kissing boundary invalid cases passed\n");
}

// Test kissing boundary read/write
void test_kissing_boundary_read_write() {
    printf("Testing kissing boundary read/write...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    KissingBoundary* boundary = crystalline_boundary_create(block, 0, 1, 1024);
    assert(boundary != NULL);
    
    // Write from segment 0
    void* write_ptr = crystalline_boundary_write(boundary, 0);
    assert(write_ptr != NULL);
    strcpy((char*)write_ptr, "Boundary data");
    crystalline_boundary_release(boundary);
    
    // Read from segment 1
    const void* read_ptr = crystalline_boundary_read(boundary, 1);
    assert(read_ptr != NULL);
    assert(strcmp((const char*)read_ptr, "Boundary data") == 0);
    
    // Check access statistics
    assert(atomic_load(&boundary->accesses_a) == 1);
    assert(atomic_load(&boundary->accesses_b) == 1);
    
    crystalline_boundary_destroy(boundary);
    crystalline_memory_destroy(block);
    
    printf("✓ Kissing boundary read/write passed\n");
}

// Test parent-child linking
void test_parent_child_linking() {
    printf("Testing parent-child memory linking...\n");
    
    CrystallineMemoryBlock* parent = crystalline_memory_create(12 * 1024, 1, 0);
    CrystallineMemoryBlock* child = crystalline_memory_create(12 * 1024, 2, 1);
    assert(parent != NULL);
    assert(child != NULL);
    
    // Link parent and child
    int result = crystalline_memory_link_parent_child(parent, child, 0, 1024);
    assert(result == 1);
    
    // Verify link
    SharedMemoryRegion* parent_shared = crystalline_memory_get_child_shared(parent, 0);
    assert(parent_shared != NULL);
    
    SharedMemoryRegion* child_shared = crystalline_memory_get_parent_shared(child);
    assert(child_shared != NULL);
    assert(child_shared == parent_shared);
    
    crystalline_memory_destroy(child);
    crystalline_memory_destroy(parent);
    
    printf("✓ Parent-child linking passed\n");
}

// Test multiple children linking
void test_multiple_children() {
    printf("Testing multiple children linking...\n");
    
    CrystallineMemoryBlock* parent = crystalline_memory_create(12 * 1024, 1, 0);
    assert(parent != NULL);
    
    // Create and link 12 children
    CrystallineMemoryBlock* children[NUM_SYMMETRY_GROUPS];
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        children[i] = crystalline_memory_create(12 * 1024, 10 + i, 1);
        assert(children[i] != NULL);
        
        int result = crystalline_memory_link_parent_child(parent, children[i], i, 1024);
        assert(result == 1);
    }
    
    // Verify all links
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        SharedMemoryRegion* shared = crystalline_memory_get_child_shared(parent, i);
        assert(shared != NULL);
    }
    
    // Cleanup
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        crystalline_memory_destroy(children[i]);
    }
    crystalline_memory_destroy(parent);
    
    printf("✓ Multiple children linking passed\n");
}

// Test statistics tracking
void test_statistics() {
    printf("Testing statistics tracking...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Initial statistics should be zero
    uint64_t reads, writes, hits, misses;
    crystalline_memory_get_stats(block, &reads, &writes, &hits, &misses);
    assert(reads == 0);
    assert(writes == 0);
    
    // Perform some operations
    CrystallineSegment* seg = crystalline_memory_get_segment(block, 0);
    char buffer[100];
    crystalline_segment_write(seg, 0, "test", 5);
    crystalline_segment_read(seg, 0, buffer, 5);
    
    // Check segment statistics
    assert(atomic_load(&seg->read_count) == 1);
    assert(atomic_load(&seg->write_count) == 1);
    
    // Reset statistics
    crystalline_memory_reset_stats(block);
    assert(atomic_load(&seg->read_count) == 0);
    assert(atomic_load(&seg->write_count) == 0);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Statistics tracking passed\n");
}

// Test validation
void test_validation() {
    printf("Testing memory block validation...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Valid block should pass validation
    assert(crystalline_memory_validate(block) == 1);
    
    // NULL block should fail
    assert(crystalline_memory_validate(NULL) == 0);
    
    crystalline_memory_destroy(block);
    
    printf("✓ Validation passed\n");
}

// Test 12-fold symmetry
void test_12_fold_symmetry() {
    printf("Testing 12-fold symmetry...\n");
    
    CrystallineMemoryBlock* block = crystalline_memory_create(12 * 1024, 1, 0);
    assert(block != NULL);
    
    // Verify exactly 12 segments
    int segment_count = 0;
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (block->segments[i].data != NULL) {
            segment_count++;
        }
    }
    assert(segment_count == NUM_SYMMETRY_GROUPS);
    
    // Verify each segment has unique symmetry group
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        assert(block->segments[i].symmetry_group == (uint32_t)i);
    }
    
    crystalline_memory_destroy(block);
    
    printf("✓ 12-fold symmetry passed\n");
}

// Test large memory allocation
void test_large_allocation() {
    printf("Testing large memory allocation...\n");
    
    // Allocate 120 MB (10 MB per segment)
    CrystallineMemoryBlock* block = crystalline_memory_create(120 * 1024 * 1024, 1, 0);
    assert(block != NULL);
    assert(block->total_size >= 120 * 1024 * 1024);
    
    // Verify all segments are allocated
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        assert(block->segments[i].data != NULL);
    }
    
    crystalline_memory_destroy(block);
    
    printf("✓ Large allocation passed\n");
}

int main() {
    printf("=== Crystalline Memory Tests ===\n\n");
    
    printf("Test 1...\n"); fflush(stdout);
    test_memory_block_creation();
    printf("Test 2...\n"); fflush(stdout);
    test_memory_block_zero_size();
    printf("Test 3...\n"); fflush(stdout);
    test_cache_alignment();
    printf("Test 4...\n"); fflush(stdout);
    test_segment_access();
    printf("Test 5...\n"); fflush(stdout);
    test_segment_read_write();
    printf("Test 6...\n"); fflush(stdout);
    test_segment_bounds();
    printf("Test 7...\n"); fflush(stdout);
    test_kissing_boundary_creation();
    printf("Test 8...\n"); fflush(stdout);
    test_kissing_boundary_invalid();
    printf("Test 9...\n"); fflush(stdout);
    test_kissing_boundary_read_write();
    printf("Test 10...\n"); fflush(stdout);
    test_parent_child_linking();
    printf("Test 11...\n"); fflush(stdout);
    test_multiple_children();
    printf("Test 12...\n"); fflush(stdout);
    test_statistics();
    printf("Test 13...\n"); fflush(stdout);
    test_validation();
    printf("Test 14...\n"); fflush(stdout);
    test_12_fold_symmetry();
    printf("Test 15...\n"); fflush(stdout);
    test_large_allocation();
    
    printf("\n=== All Crystalline Memory Tests Passed! ===\n");
    return 0;
}