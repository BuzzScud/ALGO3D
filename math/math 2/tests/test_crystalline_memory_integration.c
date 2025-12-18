/**
 * Test Crystalline Memory Integration with Training System
 * 
 * Tests that crystalline memory is properly integrated with sphere training contexts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ai/cllm_crystalline_memory.h"

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
// TEST SUITE 1: Memory Block Creation
// ============================================================================

void test_memory_block_creation() {
    TEST("Memory block creation for sphere context");
    
    size_t gradient_size = 1024 * 1024;  // 1MB
    int sphere_id = 0;
    int hierarchy_level = 0;
    
    CrystallineMemoryBlock* block = crystalline_memory_create(
        gradient_size,
        sphere_id,
        hierarchy_level
    );
    
    ASSERT(block != NULL, "Memory block should be created");
    ASSERT(block->owner_sphere_id == sphere_id, "Owner sphere ID should match");
    ASSERT(block->hierarchy_level == hierarchy_level, "Hierarchy level should match");
    ASSERT(block->total_size >= gradient_size, "Total size should be at least requested size (may be rounded up)");
    
    crystalline_memory_destroy(block);
    PASS();
}

void test_memory_block_segments() {
    TEST("Memory block has 12 segments");
    
    size_t gradient_size = 1024 * 1024;
    CrystallineMemoryBlock* block = crystalline_memory_create(gradient_size, 0, 0);
    
    ASSERT(block != NULL, "Memory block should be created");
    
    // Check all 12 segments exist
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        CrystallineSegment* seg = crystalline_memory_get_segment(block, i);
        ASSERT(seg != NULL, "Segment should exist");
        ASSERT(seg->symmetry_group == (uint32_t)i, "Symmetry group should match");
        ASSERT(seg->data != NULL, "Segment data should be allocated");
        ASSERT(seg->size > 0, "Segment size should be positive");
    }
    
    crystalline_memory_destroy(block);
    PASS();
}

void test_memory_block_validation() {
    TEST("Memory block validation");
    
    size_t gradient_size = 1024 * 1024;
    CrystallineMemoryBlock* block = crystalline_memory_create(gradient_size, 0, 0);
    
    ASSERT(block != NULL, "Memory block should be created");
    
    int valid = crystalline_memory_validate(block);
    ASSERT(valid == 1, "Memory block should be valid");
    
    crystalline_memory_destroy(block);
    PASS();
}

// ============================================================================
// TEST SUITE 2: Parent-Child Memory Linking
// ============================================================================

void test_parent_child_linking() {
    TEST("Parent-child memory linking");
    
    size_t gradient_size = 1024 * 1024;
    
    // Create parent and child blocks
    CrystallineMemoryBlock* parent = crystalline_memory_create(gradient_size, 0, 0);
    CrystallineMemoryBlock* child = crystalline_memory_create(gradient_size, 1, 1);
    
    ASSERT(parent != NULL, "Parent block should be created");
    ASSERT(child != NULL, "Child block should be created");
    
    // Link parent and child
    uint32_t child_symmetry_group = 0;
    size_t shared_size = gradient_size / NUM_SYMMETRY_GROUPS;
    
    int result = crystalline_memory_link_parent_child(
        parent,
        child,
        child_symmetry_group,
        shared_size
    );
    
    ASSERT(result == 1, "Linking should succeed");
    
    // Verify shared memory exists
    SharedMemoryRegion* parent_shared = crystalline_memory_get_child_shared(parent, child_symmetry_group);
    SharedMemoryRegion* child_shared = crystalline_memory_get_parent_shared(child);
    
    ASSERT(parent_shared != NULL, "Parent should have child shared memory");
    ASSERT(child_shared != NULL, "Child should have parent shared memory");
    ASSERT(parent_shared == child_shared, "Shared memory should be the same");
    
    crystalline_memory_destroy(child);
    crystalline_memory_destroy(parent);
    PASS();
}

void test_multiple_children_linking() {
    TEST("Multiple children linking (12-fold)");
    
    size_t gradient_size = 1024 * 1024;
    
    // Create parent
    CrystallineMemoryBlock* parent = crystalline_memory_create(gradient_size, 0, 0);
    ASSERT(parent != NULL, "Parent block should be created");
    
    // Create 12 children and link them
    CrystallineMemoryBlock* children[NUM_SYMMETRY_GROUPS];
    size_t shared_size = gradient_size / NUM_SYMMETRY_GROUPS;
    
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        children[i] = crystalline_memory_create(gradient_size, i + 1, 1);
        ASSERT(children[i] != NULL, "Child block should be created");
        
        int result = crystalline_memory_link_parent_child(
            parent,
            children[i],
            i,
            shared_size
        );
        ASSERT(result == 1, "Linking should succeed");
    }
    
    // Verify all links
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        SharedMemoryRegion* shared = crystalline_memory_get_child_shared(parent, i);
        ASSERT(shared != NULL, "Parent should have shared memory with each child");
    }
    
    // Cleanup
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        crystalline_memory_destroy(children[i]);
    }
    crystalline_memory_destroy(parent);
    PASS();
}

// ============================================================================
// TEST SUITE 3: Segment Access
// ============================================================================

void test_segment_read_write() {
    TEST("Segment read/write operations");
    
    size_t gradient_size = 1024 * 1024;
    CrystallineMemoryBlock* block = crystalline_memory_create(gradient_size, 0, 0);
    
    ASSERT(block != NULL, "Memory block should be created");
    
    // Get segment 0
    uint32_t symmetry_group = 0;
    CrystallineSegment* segment = crystalline_memory_get_segment(block, symmetry_group);
    ASSERT(segment != NULL, "Segment should exist");
    
    double test_data[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    
    // Write to segment
    size_t write_result = crystalline_segment_write(
        segment,
        0,  // offset
        test_data,
        sizeof(test_data)
    );
    
    ASSERT(write_result == sizeof(test_data), "Write should succeed");
    
    // Read back from segment
    double read_data[10] = {0};
    size_t read_result = crystalline_segment_read(
        segment,
        0,  // offset
        read_data,
        sizeof(read_data)
    );
    
    ASSERT(read_result == sizeof(read_data), "Read should succeed");
    
    // Verify data matches
    for (int i = 0; i < 10; i++) {
        ASSERT(read_data[i] == test_data[i], "Read data should match written data");
    }
    
    crystalline_memory_destroy(block);
    PASS();
}

void test_segment_access_statistics() {
    TEST("Segment access statistics tracking");
    
    size_t gradient_size = 1024 * 1024;
    CrystallineMemoryBlock* block = crystalline_memory_create(gradient_size, 0, 0);
    
    ASSERT(block != NULL, "Memory block should be created");
    
    // Get segment
    CrystallineSegment* segment = crystalline_memory_get_segment(block, 0);
    ASSERT(segment != NULL, "Segment should exist");
    
    // Get initial segment stats
    uint64_t initial_reads = atomic_load(&segment->read_count);
    uint64_t initial_writes = atomic_load(&segment->write_count);
    
    // Perform some operations
    double test_data[10] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    crystalline_segment_write(segment, 0, test_data, sizeof(test_data));
    crystalline_segment_read(segment, 0, test_data, sizeof(test_data));
    
    // Get updated segment stats
    uint64_t final_reads = atomic_load(&segment->read_count);
    uint64_t final_writes = atomic_load(&segment->write_count);
    
    ASSERT(final_writes > initial_writes, "Write count should increase");
    ASSERT(final_reads > initial_reads, "Read count should increase");
    
    crystalline_memory_destroy(block);
    PASS();
}

// ============================================================================
// TEST SUITE 4: Hierarchy Integration
// ============================================================================

void test_hierarchy_level_tracking() {
    TEST("Hierarchy level tracking in memory blocks");
    
    size_t gradient_size = 1024 * 1024;
    
    // Create blocks at different hierarchy levels
    CrystallineMemoryBlock* level0 = crystalline_memory_create(gradient_size, 0, 0);
    CrystallineMemoryBlock* level1 = crystalline_memory_create(gradient_size, 1, 1);
    CrystallineMemoryBlock* level2 = crystalline_memory_create(gradient_size, 2, 2);
    
    ASSERT(level0 != NULL, "Level 0 block should be created");
    ASSERT(level1 != NULL, "Level 1 block should be created");
    ASSERT(level2 != NULL, "Level 2 block should be created");
    
    ASSERT(level0->hierarchy_level == 0, "Level 0 should be correct");
    ASSERT(level1->hierarchy_level == 1, "Level 1 should be correct");
    ASSERT(level2->hierarchy_level == 2, "Level 2 should be correct");
    
    crystalline_memory_destroy(level2);
    crystalline_memory_destroy(level1);
    crystalline_memory_destroy(level0);
    PASS();
}

void test_sphere_id_tracking() {
    TEST("Sphere ID tracking in memory blocks");
    
    size_t gradient_size = 1024 * 1024;
    
    // Create blocks with different sphere IDs
    CrystallineMemoryBlock* sphere0 = crystalline_memory_create(gradient_size, 0, 0);
    CrystallineMemoryBlock* sphere1 = crystalline_memory_create(gradient_size, 1, 0);
    CrystallineMemoryBlock* sphere12 = crystalline_memory_create(gradient_size, 12, 0);
    
    ASSERT(sphere0 != NULL, "Sphere 0 block should be created");
    ASSERT(sphere1 != NULL, "Sphere 1 block should be created");
    ASSERT(sphere12 != NULL, "Sphere 12 block should be created");
    
    ASSERT(sphere0->owner_sphere_id == 0, "Sphere 0 ID should be correct");
    ASSERT(sphere1->owner_sphere_id == 1, "Sphere 1 ID should be correct");
    ASSERT(sphere12->owner_sphere_id == 12, "Sphere 12 ID should be correct");
    
    crystalline_memory_destroy(sphere12);
    crystalline_memory_destroy(sphere1);
    crystalline_memory_destroy(sphere0);
    PASS();
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("=== Crystalline Memory Integration Tests ===\n\n");
    
    // Suite 1: Memory Block Creation
    printf("Suite 1: Memory Block Creation\n");
    test_memory_block_creation();
    test_memory_block_segments();
    test_memory_block_validation();
    printf("\n");
    
    // Suite 2: Parent-Child Memory Linking
    printf("Suite 2: Parent-Child Memory Linking\n");
    test_parent_child_linking();
    test_multiple_children_linking();
    printf("\n");
    
    // Suite 3: Segment Access
    printf("Suite 3: Segment Access\n");
    test_segment_read_write();
    test_segment_access_statistics();
    printf("\n");
    
    // Suite 4: Hierarchy Integration
    printf("Suite 4: Hierarchy Integration\n");
    test_hierarchy_level_tracking();
    test_sphere_id_tracking();
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