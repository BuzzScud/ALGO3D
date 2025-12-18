#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "math/math.h"

#include "math/types.h"
#include "cllm_hierarchical_abacus.h"
#include "ai/cllm_sphere_stats.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("\n=== Testing: %s ===\n", #name); \
    if (test_##name()) { \
        printf("✓ %s PASSED\n", #name); \
        tests_passed++; \
    } else { \
        printf("✗ %s FAILED\n", #name); \
        tests_failed++; \
    }

// ============================================================================
// MATHEMATICAL CONSTANTS TESTS
// ============================================================================

int test_vector_culmination() {
    if (VECTOR_CULMINATION != 144000) {
        printf("ERROR: VECTOR_CULMINATION = %llu, expected 144000\n", 
               (unsigned long long)VECTOR_CULMINATION);
        return 0;
    }
    return 1;
}

int test_twin_primes() {
    if (TWIN_PRIME_LOWER != 143999) {
        printf("ERROR: TWIN_PRIME_LOWER = %llu, expected 143999\n",
               (unsigned long long)TWIN_PRIME_LOWER);
        return 0;
    }
    
    if (TWIN_PRIME_UPPER != 144001) {
        printf("ERROR: TWIN_PRIME_UPPER = %llu, expected 144001\n",
               (unsigned long long)TWIN_PRIME_UPPER);
        return 0;
    }
    
    return 1;
}

int test_einstein_lambda() {
    double lambda = cllm_get_einstein_lambda();
    double expected = 3.0 / 144000.0;
    double error = fabs(lambda - expected);
    
    if (error > 1e-10) {
        printf("ERROR: Einstein's Lambda = %.15f, expected %.15f\n",
               lambda, expected);
        return 0;
    }
    
    int num, den;
    cllm_get_einstein_lambda_fraction(&num, &den);
    
    if (num != 3 || den != 144000) {
        printf("ERROR: Lambda fraction = %d/%d, expected 3/144000\n",
               num, den);
        return 0;
    }
    
    return 1;
}

int test_symmetry_group_count() {
    if (SYMMETRY_GROUP_COUNT != 12) {
        printf("ERROR: SYMMETRY_GROUP_COUNT = %d, expected 12\n",
               SYMMETRY_GROUP_COUNT);
        return 0;
    }
    return 1;
}

int test_dimensional_frequencies() {
    // Test first few dimensional frequencies
    if (DIMENSIONAL_FREQUENCIES[0] != 3) {
        printf("ERROR: φ₀ = %llu, expected 3\n",
               (unsigned long long)DIMENSIONAL_FREQUENCIES[0]);
        return 0;
    }
    
    if (DIMENSIONAL_FREQUENCIES[1] != 7) {
        printf("ERROR: φ₁ = %llu, expected 7\n",
               (unsigned long long)DIMENSIONAL_FREQUENCIES[1]);
        return 0;
    }
    
    if (DIMENSIONAL_FREQUENCIES[2] != 31) {
        printf("ERROR: φ₂ = %llu, expected 31\n",
               (unsigned long long)DIMENSIONAL_FREQUENCIES[2]);
        return 0;
    }
    
    // Test getter function
    uint64_t phi_0 = cllm_get_dimensional_frequency(0);
    if (phi_0 != 3) {
        printf("ERROR: cllm_get_dimensional_frequency(0) = %llu, expected 3\n",
               (unsigned long long)phi_0);
        return 0;
    }
    
    return 1;
}

int test_boundary_detection() {
    // Test 144000 boundary detection
    if (!cllm_is_near_144000_boundary(144000)) {
        printf("ERROR: 144000 not detected as near boundary\n");
        return 0;
    }
    
    if (!cllm_is_near_144000_boundary(TWIN_PRIME_LOWER)) {
        printf("ERROR: Twin prime lower not detected\n");
        return 0;
    }
    
    if (!cllm_is_near_144000_boundary(TWIN_PRIME_UPPER)) {
        printf("ERROR: Twin prime upper not detected\n");
        return 0;
    }
    
    if (cllm_is_near_144000_boundary(100000)) {
        printf("ERROR: 100000 incorrectly detected as near boundary\n");
        return 0;
    }
    
    return 1;
}

int test_mathematical_validation() {
    if (!cllm_validate_mathematical_constants()) {
        printf("ERROR: Mathematical constant validation failed\n");
        return 0;
    }
    return 1;
}

int test_bigfixed_constants() {
    MathematicalConstantsBigFixed constants;
    cllm_init_mathematical_constants(&constants);
    
    // Just verify initialization doesn't crash
    // Detailed BigFixed testing would require more infrastructure
    
    cllm_free_mathematical_constants(&constants);
    return 1;
}

// ============================================================================
// HIERARCHICAL ABACUS TESTS
// ============================================================================

int test_hierarchical_abacus_creation() {
    HierarchicalAbacus* abacus = hierarchical_abacus_create(
        NULL,  // No parent
        5,     // Symmetry group 5
        0,     // Start at prime 0
        1000   // Cache size
    );
    
    if (!abacus) {
        printf("ERROR: Failed to create hierarchical abacus\n");
        return 0;
    }
    
    hierarchical_abacus_free(abacus);
    return 1;
}

int test_hierarchical_abacus_next_prime() {
    HierarchicalAbacus* abacus = hierarchical_abacus_create(
        NULL, 5, 0, 1000
    );
    
    if (!abacus) {
        printf("ERROR: Failed to create abacus\n");
        return 0;
    }
    
    // Get first few primes in symmetry group 5
    uint64_t prime1 = hierarchical_abacus_next_prime(abacus);
    uint64_t prime2 = hierarchical_abacus_next_prime(abacus);
    uint64_t prime3 = hierarchical_abacus_next_prime(abacus);
    
    // Verify they're all in symmetry group 5 (p mod 12 == 5)
    if (prime1 % 12 != 5) {
        printf("ERROR: prime1 = %llu not in group 5\n", 
               (unsigned long long)prime1);
        hierarchical_abacus_free(abacus);
        return 0;
    }
    
    if (prime2 % 12 != 5) {
        printf("ERROR: prime2 = %llu not in group 5\n",
               (unsigned long long)prime2);
        hierarchical_abacus_free(abacus);
        return 0;
    }
    
    if (prime3 % 12 != 5) {
        printf("ERROR: prime3 = %llu not in group 5\n",
               (unsigned long long)prime3);
        hierarchical_abacus_free(abacus);
        return 0;
    }
    
    // Verify they're increasing
    if (!(prime1 < prime2 && prime2 < prime3)) {
        printf("ERROR: Primes not increasing: %llu, %llu, %llu\n",
               (unsigned long long)prime1,
               (unsigned long long)prime2,
               (unsigned long long)prime3);
        hierarchical_abacus_free(abacus);
        return 0;
    }
    
    hierarchical_abacus_free(abacus);
    return 1;
}

int test_hierarchical_abacus_statistics() {
    HierarchicalAbacus* abacus = hierarchical_abacus_create(
        NULL, 7, 0, 1000
    );
    
    if (!abacus) {
        printf("ERROR: Failed to create abacus\n");
        return 0;
    }
    
    // Generate some primes
    for (int i = 0; i < 10; i++) {
        hierarchical_abacus_next_prime(abacus);
    }
    
    // Print statistics (just verify it doesn't crash)
    hierarchical_abacus_print_stats(abacus);
    
    hierarchical_abacus_free(abacus);
    return 1;
}

// ============================================================================
// SPHERE STATISTICS TESTS
// ============================================================================

int test_sphere_stats_init() {
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 3, 0);
    
    if (stats.symmetry_group != 3) {
        printf("ERROR: Symmetry group = %d, expected 3\n",
               stats.symmetry_group);
        return 0;
    }
    
    if (stats.hierarchy_level != 0) {
        printf("ERROR: Hierarchy level = %d, expected 0\n",
               stats.hierarchy_level);
        return 0;
    }
    
    if (atomic_load(&stats.primes_processed) != 0) {
        printf("ERROR: Initial primes_processed not zero\n");
        return 0;
    }
    
    return 1;
}

int test_sphere_stats_record_prime() {
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 5, 0);
    
    // Record some primes
    cllm_sphere_stats_record_prime(&stats, 5, 1000);
    cllm_sphere_stats_record_prime(&stats, 17, 1000);
    cllm_sphere_stats_record_prime(&stats, 29, 1000);
    
    if (atomic_load(&stats.primes_processed) != 3) {
        printf("ERROR: primes_processed = %lu, expected 3\n",
               (unsigned long)atomic_load(&stats.primes_processed));
        return 0;
    }
    
    // Check symmetry group tracking
    if (atomic_load(&stats.primes_per_group[5]) != 3) {
        printf("ERROR: primes_per_group[5] = %lu, expected 3\n",
               (unsigned long)atomic_load(&stats.primes_per_group[5]));
        return 0;
    }
    
    return 1;
}

int test_sphere_stats_boundary_tracking() {
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 11, 0);
    
    // Record boundary primes
    cllm_sphere_stats_record_prime(&stats, TWIN_PRIME_LOWER, 1000);
    cllm_sphere_stats_record_prime(&stats, TWIN_PRIME_UPPER, 1000);
    
    if (atomic_load(&stats.twin_prime_hits) != 2) {
        printf("ERROR: twin_prime_hits = %lu, expected 2\n",
               (unsigned long)atomic_load(&stats.twin_prime_hits));
        return 0;
    }
    
    if (atomic_load(&stats.near_boundary_primes) != 2) {
        printf("ERROR: near_boundary_primes = %lu, expected 2\n",
               (unsigned long)atomic_load(&stats.near_boundary_primes));
        return 0;
    }
    
    return 1;
}

int test_sphere_stats_cache_tracking() {
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 7, 0);
    
    // Record cache accesses
    cllm_sphere_stats_record_cache_access(&stats, 1);  // Hit
    cllm_sphere_stats_record_cache_access(&stats, 1);  // Hit
    cllm_sphere_stats_record_cache_access(&stats, 0);  // Miss
    
    double hit_rate = cllm_sphere_stats_get_cache_hit_rate(&stats);
    double expected_rate = 2.0 / 3.0;
    
    if (fabs(hit_rate - expected_rate) > 0.01) {
        printf("ERROR: Cache hit rate = %.2f, expected %.2f\n",
               hit_rate, expected_rate);
        return 0;
    }
    
    return 1;
}

int test_sphere_stats_print() {
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 9, 1);
    
    // Add some data
    cllm_sphere_stats_record_prime(&stats, 11, 1000);
    cllm_sphere_stats_record_batch(&stats, 10, 10000);
    cllm_sphere_stats_record_gradients(&stats, 100);
    
    // Print (just verify it doesn't crash)
    cllm_sphere_stats_print(&stats, 0);
    cllm_sphere_stats_print_detailed(&stats, 0);
    
    return 1;
}

int test_sphere_stats_merge() {
    SphereStatistics parent, child;
    cllm_sphere_stats_init(&parent, 0, 0);
    cllm_sphere_stats_init(&child, 0, 1);
    
    // Add data to child
    cllm_sphere_stats_record_prime(&child, 13, 1000);
    cllm_sphere_stats_record_prime(&child, 37, 1000);
    
    // Merge into parent
    cllm_sphere_stats_merge(&parent, &child);
    
    if (atomic_load(&parent.primes_processed) != 2) {
        printf("ERROR: After merge, parent primes_processed = %lu, expected 2\n",
               (unsigned long)atomic_load(&parent.primes_processed));
        return 0;
    }
    
    return 1;
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

int test_abacus_with_stats() {
    HierarchicalAbacus* abacus = hierarchical_abacus_create(
        NULL, 11, 0, 1000
    );
    
    SphereStatistics stats;
    cllm_sphere_stats_init(&stats, 11, 0);
    
    // Generate primes and track statistics
    for (int i = 0; i < 20; i++) {
        uint64_t prime = hierarchical_abacus_next_prime(abacus);
        cllm_sphere_stats_record_prime(&stats, prime, 1000);
    }
    
    if (atomic_load(&stats.primes_processed) != 20) {
        printf("ERROR: primes_processed = %lu, expected 20\n",
               (unsigned long)atomic_load(&stats.primes_processed));
        hierarchical_abacus_free(abacus);
        return 0;
    }
    
    hierarchical_abacus_free(abacus);
    return 1;
}

int test_all_symmetry_groups() {
    // Test that we can create abacuses for all 12 symmetry groups
    for (int group = 0; group < 12; group++) {
        HierarchicalAbacus* abacus = hierarchical_abacus_create(
            NULL, group, 0, 100
        );
        
        if (!abacus) {
            printf("ERROR: Failed to create abacus for group %d\n", group);
            return 0;
        }
        
        // Get a few primes and verify they're in the right group
        for (int i = 0; i < 5; i++) {
            uint64_t prime = hierarchical_abacus_next_prime(abacus);
            if (prime % 12 != (uint64_t)group) {
                printf("ERROR: Group %d produced prime %llu (mod 12 = %llu)\n",
                       group, (unsigned long long)prime,
                       (unsigned long long)(prime % 12));
                hierarchical_abacus_free(abacus);
                return 0;
            }
        }
        
        hierarchical_abacus_free(abacus);
    }
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     PHASE 1, DAY 2 - COMPREHENSIVE TEST SUITE             ║\n");
    printf("║     Mathematical Constants, Hierarchical Abacus, Stats    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Mathematical Constants Tests
    printf("\n--- MATHEMATICAL CONSTANTS TESTS ---\n");
    TEST(vector_culmination);
    TEST(twin_primes);
    TEST(einstein_lambda);
    TEST(symmetry_group_count);
    TEST(dimensional_frequencies);
    TEST(boundary_detection);
    TEST(mathematical_validation);
    TEST(bigfixed_constants);
    
    // Hierarchical Abacus Tests
    printf("\n--- HIERARCHICAL ABACUS TESTS ---\n");
    TEST(hierarchical_abacus_creation);
    TEST(hierarchical_abacus_next_prime);
    TEST(hierarchical_abacus_statistics);
    
    // Sphere Statistics Tests
    printf("\n--- SPHERE STATISTICS TESTS ---\n");
    TEST(sphere_stats_init);
    TEST(sphere_stats_record_prime);
    TEST(sphere_stats_boundary_tracking);
    TEST(sphere_stats_cache_tracking);
    TEST(sphere_stats_print);
    TEST(sphere_stats_merge);
    
    // Integration Tests
    printf("\n--- INTEGRATION TESTS ---\n");
    TEST(abacus_with_stats);
    TEST(all_symmetry_groups);
    
    // Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests:  %3d                                         ║\n", 
           tests_passed + tests_failed);
    printf("║  Passed:       %3d ✓                                       ║\n", 
           tests_passed);
    printf("║  Failed:       %3d ✗                                       ║\n", 
           tests_failed);
    printf("║  Success Rate: %3d%%                                        ║\n",
           (tests_passed * 100) / (tests_passed + tests_failed));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return tests_failed > 0 ? 1 : 0;
}