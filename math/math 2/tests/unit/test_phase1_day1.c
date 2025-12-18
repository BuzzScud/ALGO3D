/**
 * Comprehensive Test for Phase 1, Day 1 Components
 */

#include "ai/cllm_thread_allocation.h"
#include "ai/cllm_shared_memory.h"
#include "lattice_sphere_positions.h"
#include "bigint_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            tests_passed++; \
        } else { \
            tests_failed++; \
            fprintf(stderr, "FAIL: %s\n", message); \
        } \
    } while(0)

void test_shared_memory_readonly() {
    printf("\nTest: Shared Memory READ_ONLY\n");
    
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_ONLY);
    TEST_ASSERT(region != NULL, "Should create region");
    TEST_ASSERT(region->access_mode == SHARED_READ_ONLY, "Should be READ_ONLY");
    
    const void* data = shared_memory_read(region);
    TEST_ASSERT(data != NULL, "Should read data");
    shared_memory_release_read(region);
    
    void* write_data = shared_memory_write(region);
    TEST_ASSERT(write_data == NULL, "Should not allow write to READ_ONLY");
    
    TEST_ASSERT(shared_memory_validate(region), "Should be valid");
    
    shared_memory_free(region);
}

void test_shared_memory_locked() {
    printf("\nTest: Shared Memory LOCKED_WRITE\n");
    
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_LOCKED_WRITE);
    TEST_ASSERT(region != NULL, "Should create region");
    
    void* write_data = shared_memory_write(region);
    TEST_ASSERT(write_data != NULL, "Should allow write");
    memset(write_data, 0xCD, 100);
    shared_memory_release_write(region);
    
    const void* read_data = shared_memory_read(region);
    TEST_ASSERT(read_data != NULL, "Should read data");
    TEST_ASSERT(((unsigned char*)read_data)[0] == 0xCD, "Should read written data");
    shared_memory_release_read(region);
    
    shared_memory_free(region);
}

void test_sphere_position_basic() {
    printf("\nTest: Sphere Position Basic\n");
    
    for (int sg = 0; sg < 12; sg++) {
        SpherePosition* pos = sphere_position_create(sg, NULL, NULL, 0);
        TEST_ASSERT(pos != NULL, "Should create position");
        TEST_ASSERT(pos->symmetry_group == sg, "Should have correct symmetry group");
        TEST_ASSERT(validate_sphere_position(pos), "Should be valid");
        sphere_position_free(pos);
    }
}

void test_sphere_position_144000_boundary() {
    printf("\nTest: Sphere Position 144000 Boundary\n");
    
    BigInt center, radius;
    big_init(&center);
    big_init(&radius);
    
    big_from_int(&center, 144000);
    big_from_int(&radius, 10);
    
    SpherePosition* pos = sphere_position_create(0, &center, &radius, 0);
    TEST_ASSERT(pos != NULL, "Should create position");
    TEST_ASSERT(pos->partition->contains_twin_boundary, "Should detect 144000 boundary");
    
    double omega = get_einstein_lambda_correction(pos);
    double expected = 3.0 / 144000.0;
    TEST_ASSERT(omega > 0.0, "Should have omega correction");
    TEST_ASSERT(fabs(omega - expected) < 1e-10, "Should have correct omega value");
    
    printf("  Einstein's Lambda correction: %.10f\n", omega);
    
    big_free(&center);
    big_free(&radius);
    sphere_position_free(pos);
}

void test_integration_threads_and_spheres() {
    printf("\nTest: Integration - Threads and Spheres\n");
    
    ThreadAllocationStrategy* strategy = create_thread_allocation(4);
    TEST_ASSERT(strategy != NULL, "Should create strategy");
    
    SpherePosition* positions[12];
    for (int i = 0; i < 12; i++) {
        positions[i] = sphere_position_create(i, NULL, NULL, 1);
        TEST_ASSERT(positions[i] != NULL, "Should create position");
    }
    
    for (int t = 0; t < 4; t++) {
        int* groups;
        int count;
        int result = get_symmetry_groups_for_thread(strategy, t, &groups, &count);
        TEST_ASSERT(result == 0, "Should get groups");
        
        printf("  Thread %d handles %d groups\n", t, count);
        for (int i = 0; i < count; i++) {
            int sg = groups[i];
            TEST_ASSERT(positions[sg] != NULL, "Should have position for group");
        }
    }
    
    for (int i = 0; i < 12; i++) {
        sphere_position_free(positions[i]);
    }
    free_thread_allocation(strategy);
}

int main() {
    printf("=== PHASE 1 DAY 1 COMPREHENSIVE TESTS ===\n");
    
    test_shared_memory_readonly();
    test_shared_memory_locked();
    test_sphere_position_basic();
    test_sphere_position_144000_boundary();
    test_integration_threads_and_spheres();
    
    printf("\n=== TEST RESULTS ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n✓ ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n✗ SOME TESTS FAILED\n");
        return 1;
    }
}
