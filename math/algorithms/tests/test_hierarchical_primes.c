/**
 * Hierarchical Prime Generation - Test Suite
 * 
 * Tests the hierarchical prime generation system and 12-fold symmetry.
 */

#include "hierarchical_primes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"

// Test counter
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define TEST(name) \
    printf(COLOR_BLUE "TEST: %s" COLOR_RESET "\n", name); \
    tests_run++;

#define PASS() \
    do { \
        tests_passed++; \
        printf(COLOR_GREEN "✓ PASS" COLOR_RESET "\n\n"); \
    } while(0)

#define FAIL(msg) \
    do { \
        tests_failed++; \
        printf(COLOR_RED "✗ FAIL: %s" COLOR_RESET "\n\n", msg); \
    } while(0)

#define ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            FAIL(msg); \
            return; \
        } \
    } while(0)

/**
 * Test 1: Create hierarchical prime generator
 */
void test_create_generator() {
    TEST("Create Hierarchical Prime Generator");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(100, 200, 1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    ASSERT(gen->partition.range_start == 100, "Incorrect range start");
    ASSERT(gen->partition.range_end == 200, "Incorrect range end");
    ASSERT(gen->partition.symmetry_group == 1, "Incorrect symmetry group");
    ASSERT(gen->partition.filter_by_symmetry == true, "Filtering should be enabled");
    ASSERT(gen->parent == NULL, "Parent should be NULL");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 2: Generate first prime
 */
void test_generate_first_prime() {
    TEST("Generate First Prime");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 100, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    uint64_t prime;
    bool found = hierarchical_prime_next(gen, &prime);
    ASSERT(found, "Failed to find first prime");
    ASSERT(prime == 2, "First prime should be 2");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 3: Generate sequence of primes
 */
void test_generate_sequence() {
    TEST("Generate Sequence of Primes");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 30, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    uint64_t expected[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    size_t expected_count = sizeof(expected) / sizeof(expected[0]);
    
    for (size_t i = 0; i < expected_count; i++) {
        uint64_t prime;
        bool found = hierarchical_prime_next(gen, &prime);
        ASSERT(found, "Failed to find prime");
        ASSERT(prime == expected[i], "Incorrect prime in sequence");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 4: Prime caching
 */
void test_prime_caching() {
    TEST("Prime Caching");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 100, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Generate some primes to populate cache
    uint64_t prime;
    for (int i = 0; i < 5; i++) {
        hierarchical_prime_next(gen, &prime);
    }
    
    // Check that primes are in cache
    ASSERT(hierarchical_prime_cache_lookup(gen, 2), "2 should be in cache");
    ASSERT(hierarchical_prime_cache_lookup(gen, 3), "3 should be in cache");
    ASSERT(hierarchical_prime_cache_lookup(gen, 5), "5 should be in cache");
    ASSERT(hierarchical_prime_cache_lookup(gen, 7), "7 should be in cache");
    ASSERT(hierarchical_prime_cache_lookup(gen, 11), "11 should be in cache");
    
    // Check that non-primes are not in cache
    ASSERT(!hierarchical_prime_cache_lookup(gen, 4), "4 should not be in cache");
    ASSERT(!hierarchical_prime_cache_lookup(gen, 6), "6 should not be in cache");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 5: Primality testing
 */
void test_primality_testing() {
    TEST("Primality Testing");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 100, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Test known primes
    ASSERT(hierarchical_prime_is_prime(gen, 2), "2 is prime");
    ASSERT(hierarchical_prime_is_prime(gen, 3), "3 is prime");
    ASSERT(hierarchical_prime_is_prime(gen, 5), "5 is prime");
    ASSERT(hierarchical_prime_is_prime(gen, 7), "7 is prime");
    ASSERT(hierarchical_prime_is_prime(gen, 11), "11 is prime");
    ASSERT(hierarchical_prime_is_prime(gen, 97), "97 is prime");
    
    // Test known composites
    ASSERT(!hierarchical_prime_is_prime(gen, 1), "1 is not prime");
    ASSERT(!hierarchical_prime_is_prime(gen, 4), "4 is not prime");
    ASSERT(!hierarchical_prime_is_prime(gen, 6), "6 is not prime");
    ASSERT(!hierarchical_prime_is_prime(gen, 9), "9 is not prime");
    ASSERT(!hierarchical_prime_is_prime(gen, 100), "100 is not prime");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 6: Partition boundary checking
 */
void test_partition_boundaries() {
    TEST("Partition Boundary Checking");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 30, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Test primes in partition
    ASSERT(hierarchical_prime_in_partition(gen, 11), "11 is in partition");
    ASSERT(hierarchical_prime_in_partition(gen, 13), "13 is in partition");
    ASSERT(hierarchical_prime_in_partition(gen, 29), "29 is in partition");
    
    // Test primes outside partition
    ASSERT(!hierarchical_prime_in_partition(gen, 2), "2 is not in partition");
    ASSERT(!hierarchical_prime_in_partition(gen, 7), "7 is not in partition");
    ASSERT(!hierarchical_prime_in_partition(gen, 31), "31 is not in partition");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 7: Prime in partition verification
 */
void test_prime_in_partition() {
    TEST("Prime in Partition Verification");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(50, 150, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    uint64_t prime;
    while (hierarchical_prime_next(gen, &prime)) {
        ASSERT(prime >= 50 && prime <= 150, "Prime outside partition range");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 8: Partition transitions
 */
void test_partition_transitions() {
    TEST("Partition Transitions");
    
    // Create two adjacent partitions
    HierarchicalPrimeGen* gen1 = hierarchical_prime_create(2, 50, -1, NULL);
    HierarchicalPrimeGen* gen2 = hierarchical_prime_create(51, 100, -1, NULL);
    
    ASSERT(gen1 != NULL, "Failed to create gen1");
    ASSERT(gen2 != NULL, "Failed to create gen2");
    
    // Generate all primes from gen1
    uint64_t last_prime1 = 0;
    uint64_t prime;
    while (hierarchical_prime_next(gen1, &prime)) {
        last_prime1 = prime;
    }
    
    // Generate first prime from gen2
    uint64_t first_prime2;
    bool found = hierarchical_prime_next(gen2, &first_prime2);
    ASSERT(found, "Failed to find first prime in gen2");
    ASSERT(first_prime2 > last_prime1, "Partition transition failed");
    
    hierarchical_prime_free(gen1);
    hierarchical_prime_free(gen2);
    PASS();
}

/**
 * Test 9: Multiple partition levels
 */
void test_multiple_partition_levels() {
    TEST("Multiple Partition Levels");
    
    // Create root partition
    HierarchicalPrimeGen* root = hierarchical_prime_create(2, 1000, -1, NULL);
    ASSERT(root != NULL, "Failed to create root");
    
    // Create child partitions
    HierarchicalPrimeGen* child1 = hierarchical_prime_create(2, 100, -1, root);
    HierarchicalPrimeGen* child2 = hierarchical_prime_create(101, 200, -1, root);
    
    ASSERT(child1 != NULL, "Failed to create child1");
    ASSERT(child2 != NULL, "Failed to create child2");
    ASSERT(child1->parent == root, "child1 parent incorrect");
    ASSERT(child2->parent == root, "child2 parent incorrect");
    
    hierarchical_prime_free(child1);
    hierarchical_prime_free(child2);
    hierarchical_prime_free(root);
    PASS();
}

/**
 * Test 10: Mod 12 filtering
 */
void test_mod12_filtering() {
    TEST("Mod 12 Filtering");
    
    // Test filtering for symmetry group 1 (primes ≡ 1 mod 12)
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 100, 1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    uint64_t prime;
    while (hierarchical_prime_next(gen, &prime)) {
        ASSERT(prime % 12 == 1, "Prime does not match symmetry group 1");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 11: Symmetry group 1 (primes ≡ 1 mod 12)
 */
void test_symmetry_group_1() {
    TEST("Symmetry Group 1 (primes ≡ 1 mod 12)");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 200, 1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Expected primes: 13, 37, 61, 73, 97, 109, 157, 181, 193
    uint64_t expected[] = {13, 37, 61, 73, 97, 109, 157, 181, 193};
    size_t expected_count = sizeof(expected) / sizeof(expected[0]);
    
    for (size_t i = 0; i < expected_count; i++) {
        uint64_t prime;
        bool found = hierarchical_prime_next(gen, &prime);
        ASSERT(found, "Failed to find prime");
        ASSERT(prime == expected[i], "Incorrect prime in symmetry group 1");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 12: Symmetry group 5 (primes ≡ 5 mod 12)
 */
void test_symmetry_group_5() {
    TEST("Symmetry Group 5 (primes ≡ 5 mod 12)");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 200, 5, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Expected primes: 5, 17, 29, 41, 53, 89, 101, 113, 137, 149, 173, 197
    uint64_t expected[] = {17, 29, 41, 53, 89, 101, 113, 137, 149, 173, 197};
    size_t expected_count = sizeof(expected) / sizeof(expected[0]);
    
    for (size_t i = 0; i < expected_count; i++) {
        uint64_t prime;
        bool found = hierarchical_prime_next(gen, &prime);
        ASSERT(found, "Failed to find prime");
        ASSERT(prime == expected[i], "Incorrect prime in symmetry group 5");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 13: Symmetry group 7 (primes ≡ 7 mod 12)
 */
void test_symmetry_group_7() {
    TEST("Symmetry Group 7 (primes ≡ 7 mod 12)");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 200, 7, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Expected primes: 7, 19, 31, 43, 67, 79, 103, 127, 139, 151, 163, 199
    uint64_t expected[] = {19, 31, 43, 67, 79, 103, 127, 139, 151, 163, 199};
    size_t expected_count = sizeof(expected) / sizeof(expected[0]);
    
    for (size_t i = 0; i < expected_count; i++) {
        uint64_t prime;
        bool found = hierarchical_prime_next(gen, &prime);
        ASSERT(found, "Failed to find prime");
        ASSERT(prime == expected[i], "Incorrect prime in symmetry group 7");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 14: Symmetry group 11 (primes ≡ 11 mod 12)
 */
void test_symmetry_group_11() {
    TEST("Symmetry Group 11 (primes ≡ 11 mod 12)");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10, 200, 11, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Expected primes: 11, 23, 47, 59, 71, 83, 107, 131, 167, 179, 191
    uint64_t expected[] = {11, 23, 47, 59, 71, 83, 107, 131, 167, 179, 191};
    size_t expected_count = sizeof(expected) / sizeof(expected[0]);
    
    for (size_t i = 0; i < expected_count; i++) {
        uint64_t prime;
        bool found = hierarchical_prime_next(gen, &prime);
        ASSERT(found, "Failed to find prime");
        ASSERT(prime == expected[i], "Incorrect prime in symmetry group 11");
    }
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 15: 12-fold symmetry distribution
 */
void test_12fold_symmetry_distribution() {
    TEST("12-Fold Symmetry Distribution");
    
    size_t distribution[12];
    size_t total = hierarchical_prime_symmetry_distribution(4, 200, distribution);
    
    ASSERT(total > 0, "No primes found");
    
    // Primes > 3 should only appear in groups 1, 5, 7, 11
    // Note: 2 is in group 2, 3 is in group 3 (special cases)
    ASSERT(distribution[0] == 0, "No primes in group 0");
    ASSERT(distribution[4] == 0, "No primes in group 4");
    ASSERT(distribution[6] == 0, "No primes in group 6");
    ASSERT(distribution[8] == 0, "No primes in group 8");
    ASSERT(distribution[9] == 0, "No primes in group 9");
    ASSERT(distribution[10] == 0, "No primes in group 10");
    
    // Groups 1, 5, 7, 11 should have primes
    ASSERT(distribution[1] > 0, "Group 1 should have primes");
    ASSERT(distribution[5] > 0, "Group 5 should have primes");
    ASSERT(distribution[7] > 0, "Group 7 should have primes");
    ASSERT(distribution[11] > 0, "Group 11 should have primes");
    
    printf("  Distribution: [");
    for (int i = 0; i < 12; i++) {
        printf("%zu%s", distribution[i], i < 11 ? ", " : "");
    }
    printf("]\n");
    printf("  Total primes: %zu\n", total);
    
    PASS();
}

/**
 * Test 16: Parent-child relationships
 */
void test_parent_child_relationships() {
    TEST("Parent-Child Relationships");
    
    // Create parent
    HierarchicalPrimeGen* parent = hierarchical_prime_create(2, 1000, -1, NULL);
    ASSERT(parent != NULL, "Failed to create parent");
    
    // Prefill parent cache
    size_t parent_primes = hierarchical_prime_prefill_cache(parent, 50);
    ASSERT(parent_primes > 0, "Failed to prefill parent cache");
    
    // Create child with parent reference
    HierarchicalPrimeGen* child = hierarchical_prime_create(100, 200, -1, parent);
    ASSERT(child != NULL, "Failed to create child");
    ASSERT(child->parent == parent, "Parent reference incorrect");
    
    // Child should be able to lookup primes from parent cache
    ASSERT(hierarchical_prime_cache_lookup(child, 2), "Should find 2 in parent cache");
    ASSERT(hierarchical_prime_cache_lookup(child, 3), "Should find 3 in parent cache");
    
    // Check parent lookup statistics
    uint64_t generated, hits, misses, lookups;
    hierarchical_prime_get_stats(child, &generated, &hits, &misses, &lookups);
    ASSERT(lookups > 0, "Should have parent lookups");
    
    hierarchical_prime_free(child);
    hierarchical_prime_free(parent);
    PASS();
}

/**
 * Test 17: Multi-level hierarchy
 */
void test_multi_level_hierarchy() {
    TEST("Multi-Level Hierarchy");
    
    // Create 3-level hierarchy
    HierarchicalPrimeGen* root = hierarchical_prime_create(2, 1000, -1, NULL);
    HierarchicalPrimeGen* level1 = hierarchical_prime_create(2, 500, -1, root);
    HierarchicalPrimeGen* level2 = hierarchical_prime_create(2, 100, -1, level1);
    
    ASSERT(root != NULL, "Failed to create root");
    ASSERT(level1 != NULL, "Failed to create level1");
    ASSERT(level2 != NULL, "Failed to create level2");
    
    ASSERT(level1->parent == root, "level1 parent incorrect");
    ASSERT(level2->parent == level1, "level2 parent incorrect");
    
    // Prefill caches
    hierarchical_prime_prefill_cache(root, 20);
    hierarchical_prime_prefill_cache(level1, 10);
    
    // level2 should be able to lookup from level1 and root
    ASSERT(hierarchical_prime_cache_lookup(level2, 2), "Should find 2 in hierarchy");
    
    hierarchical_prime_free(level2);
    hierarchical_prime_free(level1);
    hierarchical_prime_free(root);
    PASS();
}

/**
 * Test 18: Hierarchical prime filtering
 */
void test_hierarchical_filtering() {
    TEST("Hierarchical Prime Filtering");
    
    // Create parent with no filtering
    HierarchicalPrimeGen* parent = hierarchical_prime_create(2, 1000, -1, NULL);
    ASSERT(parent != NULL, "Failed to create parent");
    
    // Create child with symmetry group filtering
    HierarchicalPrimeGen* child = hierarchical_prime_create(10, 200, 1, parent);
    ASSERT(child != NULL, "Failed to create child");
    
    // All primes from child should match symmetry group 1
    uint64_t prime;
    while (hierarchical_prime_next(child, &prime)) {
        ASSERT(prime % 12 == 1, "Prime does not match symmetry group");
    }
    
    hierarchical_prime_free(child);
    hierarchical_prime_free(parent);
    PASS();
}

/**
 * Test 19: Abacus coordination
 */
void test_abacus_coordination() {
    TEST("Abacus Coordination");
    
    // Create multiple generators for different symmetry groups
    HierarchicalPrimeGen* gen1 = hierarchical_prime_create(10, 200, 1, NULL);
    HierarchicalPrimeGen* gen5 = hierarchical_prime_create(10, 200, 5, NULL);
    HierarchicalPrimeGen* gen7 = hierarchical_prime_create(10, 200, 7, NULL);
    HierarchicalPrimeGen* gen11 = hierarchical_prime_create(10, 200, 11, NULL);
    
    ASSERT(gen1 != NULL, "Failed to create gen1");
    ASSERT(gen5 != NULL, "Failed to create gen5");
    ASSERT(gen7 != NULL, "Failed to create gen7");
    ASSERT(gen11 != NULL, "Failed to create gen11");
    
    // Count primes in each group
    size_t count1 = hierarchical_prime_prefill_cache(gen1, 1000);
    size_t count5 = hierarchical_prime_prefill_cache(gen5, 1000);
    size_t count7 = hierarchical_prime_prefill_cache(gen7, 1000);
    size_t count11 = hierarchical_prime_prefill_cache(gen11, 1000);
    
    printf("  Group 1: %zu primes\n", count1);
    printf("  Group 5: %zu primes\n", count5);
    printf("  Group 7: %zu primes\n", count7);
    printf("  Group 11: %zu primes\n", count11);
    
    // All groups should have primes
    ASSERT(count1 > 0, "Group 1 should have primes");
    ASSERT(count5 > 0, "Group 5 should have primes");
    ASSERT(count7 > 0, "Group 7 should have primes");
    ASSERT(count11 > 0, "Group 11 should have primes");
    
    hierarchical_prime_free(gen1);
    hierarchical_prime_free(gen5);
    hierarchical_prime_free(gen7);
    hierarchical_prime_free(gen11);
    PASS();
}

/**
 * Test 20: Large prime generation
 */
void test_large_prime_generation() {
    TEST("Large Prime Generation");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(10000, 11000, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    size_t count = 0;
    uint64_t prime;
    while (hierarchical_prime_next(gen, &prime)) {
        ASSERT(prime >= 10000 && prime <= 11000, "Prime outside range");
        count++;
    }
    
    ASSERT(count > 0, "Should find primes in range");
    printf("  Found %zu primes in range [10000, 11000]\n", count);
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 21: Cache efficiency
 */
void test_cache_efficiency() {
    TEST("Cache Efficiency");
    
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 1000, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    // Generate primes to populate cache
    hierarchical_prime_prefill_cache(gen, 50);
    
    // Perform lookups to test cache
    for (int i = 0; i < 100; i++) {
        hierarchical_prime_cache_lookup(gen, 2);
        hierarchical_prime_cache_lookup(gen, 3);
        hierarchical_prime_cache_lookup(gen, 5);
    }
    
    double efficiency = hierarchical_prime_cache_efficiency(gen);
    printf("  Cache efficiency: %.2f%%\n", efficiency);
    ASSERT(efficiency > 0.0, "Cache efficiency should be > 0");
    
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 22: Memory usage
 */
void test_memory_usage() {
    TEST("Memory Usage");
    
    // Create generator and fill cache
    HierarchicalPrimeGen* gen = hierarchical_prime_create(2, 10000, -1, NULL);
    ASSERT(gen != NULL, "Failed to create generator");
    
    size_t primes_generated = hierarchical_prime_prefill_cache(gen, 1000);
    ASSERT(primes_generated > 0, "Should generate primes");
    
    printf("  Generated %zu primes\n", primes_generated);
    printf("  Cache size: %zu / %zu\n", gen->cache_size, gen->cache_capacity);
    
    // Verify no memory leaks (valgrind would catch this)
    hierarchical_prime_free(gen);
    PASS();
}

/**
 * Test 23: Verify 12-fold symmetry
 */
void test_verify_12fold_symmetry() {
    TEST("Verify 12-Fold Symmetry");
    
    // Test multiple ranges
    ASSERT(hierarchical_prime_verify_symmetry(4, 100), "Symmetry verification failed for [4, 100]");
    ASSERT(hierarchical_prime_verify_symmetry(4, 500), "Symmetry verification failed for [4, 500]");
    ASSERT(hierarchical_prime_verify_symmetry(4, 1000), "Symmetry verification failed for [4, 1000]");
    
    printf("  ✓ All primes > 3 follow 12-fold symmetry (mod 12 ∈ {1, 5, 7, 11})\n");
    
    PASS();
}

/**
 * Main test runner
 */
int main() {
    printf("\n");
    printf(COLOR_YELLOW "========================================\n");
    printf("  Hierarchical Prime Generation Test Suite\n");
    printf("========================================\n" COLOR_RESET);
    printf("\n");
    
    // Run all tests
    test_create_generator();
    test_generate_first_prime();
    test_generate_sequence();
    test_prime_caching();
    test_primality_testing();
    test_partition_boundaries();
    test_prime_in_partition();
    test_partition_transitions();
    test_multiple_partition_levels();
    test_mod12_filtering();
    test_symmetry_group_1();
    test_symmetry_group_5();
    test_symmetry_group_7();
    test_symmetry_group_11();
    test_12fold_symmetry_distribution();
    test_parent_child_relationships();
    test_multi_level_hierarchy();
    test_hierarchical_filtering();
    test_abacus_coordination();
    test_large_prime_generation();
    test_cache_efficiency();
    test_memory_usage();
    test_verify_12fold_symmetry();
    
    // Print summary
    printf(COLOR_YELLOW "========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n" COLOR_RESET);
    printf("Total Tests: %d\n", tests_run);
    printf(COLOR_GREEN "Passed: %d\n" COLOR_RESET, tests_passed);
    printf(COLOR_RED "Failed: %d\n" COLOR_RESET, tests_failed);
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n✓ ALL TESTS PASSED!\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "\n✗ SOME TESTS FAILED\n" COLOR_RESET);
    }
    printf("\n");
    
    return tests_failed == 0 ? 0 : 1;
}
