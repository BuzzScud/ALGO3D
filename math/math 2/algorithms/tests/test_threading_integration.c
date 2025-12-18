/**
 * @file test_threading_integration.c
 * @brief Comprehensive tests for Threading Integration
 */

#include "../include/threading_integration.h"
#include "../include/hierarchical_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
// TEST: Thread ID to Position Mapping
// ============================================================================

bool test_thread_to_position() {
    ClockPosition pos;
    
    // Test thread 0 (should be ring 0, position 0)
    MathError err = thread_id_to_position(0, 12, &pos);
    assert(err == MATH_SUCCESS);
    assert(pos.ring == 0);
    assert(pos.position == 0);
    printf("Thread 0: ring=%u, pos=%u, angle=%.2f, radius=%.2f\n",
           pos.ring, pos.position, pos.angle, pos.radius);
    
    // Test thread 11 (should be ring 0, position 11)
    err = thread_id_to_position(11, 12, &pos);
    assert(err == MATH_SUCCESS);
    assert(pos.ring == 0);
    assert(pos.position == 11);
    
    // Test thread 12 (should be ring 1, position 0)
    err = thread_id_to_position(12, 12, &pos);
    assert(err == MATH_SUCCESS);
    assert(pos.ring == 1);
    assert(pos.position == 0);
    printf("Thread 12: ring=%u, pos=%u, angle=%.2f, radius=%.2f\n",
           pos.ring, pos.position, pos.angle, pos.radius);
    
    // Test thread 71 (should be ring 1, position 59)
    err = thread_id_to_position(71, 12, &pos);
    assert(err == MATH_SUCCESS);
    assert(pos.ring == 1);
    assert(pos.position == 59);
    
    // Test thread 72 (should be ring 2, position 0)
    err = thread_id_to_position(72, 12, &pos);
    assert(err == MATH_SUCCESS);
    assert(pos.ring == 2);
    assert(pos.position == 0);
    printf("Thread 72: ring=%u, pos=%u, angle=%.2f, radius=%.2f\n",
           pos.ring, pos.position, pos.angle, pos.radius);
    
    return true;
}

// ============================================================================
// TEST: Position to Thread ID Mapping
// ============================================================================

bool test_position_to_thread() {
    ClockPosition pos;
    uint64_t thread_id;
    
    // Test round-trip for thread 0
    MathError err = thread_id_to_position(0, 12, &pos);
    assert(err == MATH_SUCCESS);
    
    err = position_to_thread_id(&pos, 12, &thread_id);
    assert(err == MATH_SUCCESS);
    assert(thread_id == 0);
    printf("Round-trip thread 0: OK\n");
    
    // Test round-trip for thread 42
    err = thread_id_to_position(42, 12, &pos);
    assert(err == MATH_SUCCESS);
    
    err = position_to_thread_id(&pos, 12, &thread_id);
    assert(err == MATH_SUCCESS);
    assert(thread_id == 42);
    printf("Round-trip thread 42: OK\n");
    
    // Test round-trip for thread 100
    err = thread_id_to_position(100, 12, &pos);
    assert(err == MATH_SUCCESS);
    
    err = position_to_thread_id(&pos, 12, &thread_id);
    assert(err == MATH_SUCCESS);
    assert(thread_id == 100);
    printf("Round-trip thread 100: OK\n");
    
    return true;
}

// ============================================================================
// TEST: N-Dimensional Position Computation
// ============================================================================

bool test_nd_position() {
    double position[13];
    
    // Test 2D position
    MathError err = thread_compute_nd_position(0, 2, position);
    assert(err == MATH_SUCCESS);
    printf("Thread 0 (2D): [%.3f, %.3f]\n", position[0], position[1]);
    
    // Test 3D position
    err = thread_compute_nd_position(0, 3, position);
    assert(err == MATH_SUCCESS);
    printf("Thread 0 (3D): [%.3f, %.3f, %.3f]\n", 
           position[0], position[1], position[2]);
    
    // Test 13D position
    err = thread_compute_nd_position(42, 13, position);
    assert(err == MATH_SUCCESS);
    printf("Thread 42 (13D): [%.3f, %.3f, %.3f, ...]\n",
           position[0], position[1], position[2]);
    
    // Verify positions are within reasonable bounds
    for (int i = 0; i < 13; i++) {
        assert(fabs(position[i]) < 10.0);  // Reasonable bound
    }
    
    return true;
}

// ============================================================================
// TEST: Segment to Abacus Conversion
// ============================================================================

bool test_segment_abacus() {
    // Test segment to abacus
    CrystallineAbacus* abacus = segment_to_abacus(42, 12);
    assert(abacus != NULL);
    printf("Created abacus for segment 42\n");
    
    // Test abacus to segment
    uint64_t segment_id;
    MathError err = abacus_to_segment(abacus, &segment_id);
    assert(err == MATH_SUCCESS);
    assert(segment_id == 42);
    printf("Round-trip segment 42: OK\n");
    
    abacus_free(abacus);
    
    return true;
}

// ============================================================================
// TEST: Thread Neighbors
// ============================================================================

bool test_thread_neighbors() {
    uint64_t neighbors[12];
    uint32_t num_neighbors;
    
    // Test neighbors for thread 0
    MathError err = compute_thread_neighbors(0, 12, neighbors, &num_neighbors);
    assert(err == MATH_SUCCESS);
    assert(num_neighbors > 0 && num_neighbors <= 12);
    
    printf("Thread 0 has %u neighbors: ", num_neighbors);
    for (uint32_t i = 0; i < num_neighbors; i++) {
        printf("%lu ", (unsigned long)neighbors[i]);
    }
    printf("\n");
    
    // Test neighbors for thread 42
    err = compute_thread_neighbors(42, 12, neighbors, &num_neighbors);
    assert(err == MATH_SUCCESS);
    assert(num_neighbors > 0 && num_neighbors <= 12);
    
    printf("Thread 42 has %u neighbors: ", num_neighbors);
    for (uint32_t i = 0; i < num_neighbors; i++) {
        printf("%lu ", (unsigned long)neighbors[i]);
    }
    printf("\n");
    
    return true;
}

// ============================================================================
// TEST: Coprime Relationships
// ============================================================================

bool test_coprime() {
    // Test known coprime pairs (remember we add 1 to avoid zero)
    assert(threads_are_coprime(0, 1) == true);   // gcd(1, 2) = 1, coprime
    assert(threads_are_coprime(1, 2) == true);   // gcd(2, 3) = 1, coprime
    assert(threads_are_coprime(2, 3) == true);   // gcd(3, 4) = 1, coprime
    
    // Test non-coprime pairs
    assert(threads_are_coprime(1, 3) == false);  // gcd(2, 4) = 2, not coprime
    assert(threads_are_coprime(2, 5) == false);  // gcd(3, 6) = 3, not coprime
    
    printf("Coprime checks passed\n");
    
    // Test coprime matrix
    bool coprime_matrix[16];
    MathError err = compute_thread_coprime_matrix(4, coprime_matrix);
    assert(err == MATH_SUCCESS);
    
    printf("Coprime matrix (4x4):\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%d ", coprime_matrix[i * 4 + j] ? 1 : 0);
        }
        printf("\n");
    }
    
    return true;
}

// ============================================================================
// TEST: Geometric Organization
// ============================================================================

bool test_geometric_organization() {
    const uint32_t num_threads = 24;
    ClockPosition positions[24];
    
    MathError err = organize_threads_geometrically(num_threads, 12, positions);
    assert(err == MATH_SUCCESS);
    
    printf("Organized %u threads geometrically:\n", num_threads);
    for (uint32_t i = 0; i < 12; i++) {  // Print first 12
        printf("  Thread %u: ring=%u, pos=%u, angle=%.2f°\n",
               i, positions[i].ring, positions[i].position,
               (360.0 * positions[i].angle) / (2.0 * M_PI));
    }
    
    // Validate organization
    bool valid = validate_thread_organization(num_threads, positions, 12);
    assert(valid);
    printf("Organization validated: OK\n");
    
    return true;
}

// ============================================================================
// TEST: Work Distribution
// ============================================================================

bool test_work_distribution() {
    const uint32_t num_threads = 12;
    uint64_t work_per_thread[12];
    
    // Test equal distribution
    MathError err = compute_work_distribution(num_threads, 1000, work_per_thread);
    assert(err == MATH_SUCCESS);
    
    uint64_t total = 0;
    printf("Work distribution for 1000 units across %u threads:\n", num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        printf("  Thread %u: %lu units\n", i, (unsigned long)work_per_thread[i]);
        total += work_per_thread[i];
    }
    assert(total == 1000);
    printf("Total work: %lu (verified)\n", (unsigned long)total);
    
    return true;
}

// ============================================================================
// TEST: Load Balancing
// ============================================================================

bool test_load_balancing() {
    const uint32_t num_threads = 12;
    double thread_loads[12] = {
        1.0, 1.5, 0.8, 1.2, 0.9, 1.1,
        1.3, 0.7, 1.4, 0.6, 1.6, 0.5
    };
    uint64_t redistribution[12];
    
    MathError err = balance_load_geometrically(num_threads, thread_loads, redistribution);
    assert(err == MATH_SUCCESS);
    
    printf("Load balancing for %u threads:\n", num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        printf("  Thread %u: load=%.2f, redistribution=%lu\n",
               i, thread_loads[i], (unsigned long)redistribution[i]);
    }
    
    return true;
}

// ============================================================================
// TEST: Print Thread Geometry
// ============================================================================

bool test_print_geometry() {
    printf("\n");
    print_thread_geometry(0, 12);
    printf("\n");
    print_thread_geometry(42, 12);
    printf("\n");
    print_thread_geometry(100, 12);
    
    return true;
}

// ============================================================================
// TEST: Integration with Hierarchical Memory
// ============================================================================

bool test_memory_integration() {
    // Create hierarchical memory block
    HierarchicalMemoryBlock* block = hierarchical_memory_create(
        1024*1024,  // total_size (1 MB)
        12,         // initial_segments (12-fold symmetry)
        13,         // initial_dimensions
        1,          // owner_id
        0           // hierarchy_level
    );
    assert(block != NULL);
    
    // Organize threads geometrically
    ClockPosition positions[12];
    MathError err = organize_threads_geometrically(12, 12, positions);
    assert(err == MATH_SUCCESS);
    
    // Map threads to memory segments
    printf("Mapping threads to memory segments:\n");
    for (uint32_t i = 0; i < 12; i++) {
        printf("  Thread %u -> Segment %u (ring=%u, pos=%u)\n",
               i, i, positions[i].ring, positions[i].position);
    }
    
    // Test segment access with thread mapping
    printf("Testing segment access...\n");
    for (uint32_t i = 0; i < 12; i++) {
        void* segment = hierarchical_memory_get_segment(block, i);
        if (segment == NULL) {
            printf("Error: Segment %u is NULL\n", i);
            hierarchical_memory_destroy(block);
            return false;
        }
        printf("  Segment %u: %p\n", i, segment);
    }
    printf("All segments accessible: OK\n");
    
    hierarchical_memory_destroy(block);
    
    return true;
}

// ============================================================================
// TEST: Stress Test
// ============================================================================

bool test_stress() {
    const uint32_t num_threads = 1000;
    ClockPosition* positions = malloc(num_threads * sizeof(ClockPosition));
    assert(positions != NULL);
    
    printf("Stress testing with %u threads...\n", num_threads);
    
    // Organize all threads
    MathError err = organize_threads_geometrically(num_threads, 12, positions);
    assert(err == MATH_SUCCESS);
    
    // Verify all positions
    for (uint32_t i = 0; i < num_threads; i++) {
        uint64_t thread_id;
        err = position_to_thread_id(&positions[i], 12, &thread_id);
        assert(err == MATH_SUCCESS);
        assert(thread_id == i);
    }
    
    printf("All %u threads verified: OK\n", num_threads);
    
    free(positions);
    
    return true;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║   THREADING INTEGRATION - COMPREHENSIVE TESTS              ║\n");
    printf("║   Math Library + Hierarchical Memory Integration          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");

    TEST(thread_to_position);
    TEST(position_to_thread);
    TEST(nd_position);
    TEST(segment_abacus);
    TEST(thread_neighbors);
    TEST(coprime);
    TEST(geometric_organization);
    TEST(work_distribution);
    TEST(load_balancing);
    TEST(print_geometry);
    TEST(memory_integration);
    TEST(stress);

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:   %2d                                         ║\n", tests_passed + tests_failed);
    printf("║  Passed:        %2d  ✓                                      ║\n", tests_passed);
    printf("║  Failed:        %2d  ✗                                      ║\n", tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n", 
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}