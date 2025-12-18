/**
 * Unit Tests for Plimpton 322 Parent-Child Relationships
 * 
 * Tests validation, creation, and registry management of parent-child
 * relationships based on Plimpton 322 constraints.
 */

#include "ai/cllm_plimpton_relationships.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "math/math.h"

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    printf("Testing " #name "...\n"); \
    if (test_##name()) { \
        printf("  ✓ PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("  ✗ FAILED\n"); \
        tests_failed++; \
    }

// ============================================================================
// VALIDATION TESTS
// ============================================================================

int test_coprime_validation() {
    // Test coprime pairs
    assert(is_coprime(3, 2) == true);   // gcd(3,2) = 1
    assert(is_coprime(5, 3) == true);   // gcd(5,3) = 1
    assert(is_coprime(7, 4) == true);   // gcd(7,4) = 1
    
    // Test non-coprime pairs
    assert(is_coprime(4, 2) == false);  // gcd(4,2) = 2
    assert(is_coprime(6, 3) == false);  // gcd(6,3) = 3
    assert(is_coprime(10, 5) == false); // gcd(10,5) = 5
    
    // Test edge cases
    assert(is_coprime(0, 5) == false);  // Zero case
    assert(is_coprime(5, 0) == false);  // Zero case
    assert(is_coprime(1, 1) == true);   // gcd(1,1) = 1
    
    return 1;
}

int test_both_odd_check() {
    // Both odd
    assert(both_odd(3, 5) == true);
    assert(both_odd(7, 9) == true);
    
    // One even
    assert(both_odd(2, 3) == false);
    assert(both_odd(3, 4) == false);
    
    // Both even
    assert(both_odd(2, 4) == false);
    assert(both_odd(6, 8) == false);
    
    return 1;
}

int test_ordering_validation() {
    // Valid ordering (p > q > 0)
    assert(valid_ordering(5, 3) == true);
    assert(valid_ordering(10, 1) == true);
    assert(valid_ordering(100, 50) == true);
    
    // Invalid ordering
    assert(valid_ordering(3, 5) == false);  // p < q
    assert(valid_ordering(5, 5) == false);  // p = q
    assert(valid_ordering(5, 0) == false);  // q = 0
    assert(valid_ordering(0, 5) == false);  // p = 0
    
    return 1;
}

int test_valid_relationships() {
    // Known valid Plimpton 322 pairs
    assert(validate_parent_child_relation(2, 1) == true);   // (2,1): coprime, not both odd, 2>1
    assert(validate_parent_child_relation(3, 2) == true);   // (3,2): coprime, not both odd, 3>2
    assert(validate_parent_child_relation(4, 1) == true);   // (4,1): coprime, not both odd, 4>1
    assert(validate_parent_child_relation(4, 3) == true);   // (4,3): coprime, not both odd, 4>3
    assert(validate_parent_child_relation(5, 2) == true);   // (5,2): coprime, not both odd, 5>2
    assert(validate_parent_child_relation(5, 4) == true);   // (5,4): coprime, not both odd, 5>4
    
    return 1;
}

int test_invalid_relationships() {
    // Not coprime
    assert(validate_parent_child_relation(4, 2) == false);  // gcd(4,2) = 2
    assert(validate_parent_child_relation(6, 3) == false);  // gcd(6,3) = 3
    
    // Both odd
    assert(validate_parent_child_relation(5, 3) == false);  // Both odd
    assert(validate_parent_child_relation(7, 5) == false);  // Both odd
    
    // Invalid ordering
    assert(validate_parent_child_relation(2, 3) == false);  // p < q
    assert(validate_parent_child_relation(3, 3) == false);  // p = q
    assert(validate_parent_child_relation(3, 0) == false);  // q = 0
    
    return 1;
}

int test_detailed_validation() {
    ParentChildRelation relation;
    
    // Valid relationship
    bool valid = validate_relation_detailed(3, 2, &relation);
    assert(valid == true);
    assert(relation.parent_id == 3);
    assert(relation.child_id == 2);
    assert(relation.is_coprime == true);
    assert(relation.is_valid == true);
    assert(relation.gcd_value == 1);
    
    // Invalid relationship (both odd)
    valid = validate_relation_detailed(5, 3, &relation);
    assert(valid == false);
    assert(relation.is_valid == false);
    
    // Invalid relationship (not coprime)
    valid = validate_relation_detailed(4, 2, &relation);
    assert(valid == false);
    assert(relation.is_valid == false);
    assert(relation.gcd_value == 2);
    
    return 1;
}

// ============================================================================
// RELATIONSHIP CREATION TESTS
// ============================================================================

int test_create_relation() {
    // Valid relationship
    ParentChildRelation rel = create_parent_child_relation(3, 2);
    assert(rel.is_valid == true);
    assert(rel.parent_id == 3);
    assert(rel.child_id == 2);
    
    // Invalid relationship
    rel = create_parent_child_relation(5, 3);
    assert(rel.is_valid == false);
    
    return 1;
}

int test_find_valid_child() {
    // Find first valid child for parent 5
    uint64_t child = find_valid_child_id(5, 1, 10);
    assert(child == 2);  // (5,2) is valid
    
    // Find next valid child
    child = find_valid_child_id(5, 3, 10);
    assert(child == 4);  // (5,4) is valid
    
    // No valid child found
    child = find_valid_child_id(2, 2, 10);
    assert(child == 0);  // No valid child >= 2 for parent 2
    
    // Edge cases
    child = find_valid_child_id(0, 1, 10);
    assert(child == 0);  // Invalid parent
    
    child = find_valid_child_id(5, 0, 10);
    assert(child == 2);  // Start from 1 automatically
    
    return 1;
}

int test_generate_children() {
    uint64_t child_ids[12];
    
    // Generate children for parent 10
    size_t count = generate_valid_children(10, child_ids, 12);
    assert(count > 0);
    
    // Verify all generated children are valid
    for (size_t i = 0; i < count; i++) {
        assert(validate_parent_child_relation(10, child_ids[i]) == true);
    }
    
    // Generate children for parent 5
    count = generate_valid_children(5, child_ids, 12);
    assert(count >= 2);  // At least (5,2) and (5,4)
    
    // Edge cases
    count = generate_valid_children(0, child_ids, 12);
    assert(count == 0);  // Invalid parent
    
    count = generate_valid_children(5, NULL, 12);
    assert(count == 0);  // NULL output
    
    count = generate_valid_children(5, child_ids, 0);
    assert(count == 0);  // Zero max
    
    return 1;
}

// ============================================================================
// REGISTRY TESTS
// ============================================================================

int test_registry_create_destroy() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    assert(registry->capacity == 10);
    assert(registry->count == 0);
    
    relationship_registry_destroy(registry);
    
    // Test with zero capacity (should use default)
    registry = relationship_registry_create(0);
    assert(registry != NULL);
    assert(registry->capacity > 0);
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_add() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add valid relationship
    bool added = relationship_registry_add(registry, 3, 2);
    assert(added == true);
    assert(registry->count == 1);
    
    // Add another valid relationship
    added = relationship_registry_add(registry, 5, 2);
    assert(added == true);
    assert(registry->count == 2);
    
    // Try to add invalid relationship
    added = relationship_registry_add(registry, 5, 3);
    assert(added == false);
    assert(registry->count == 2);  // Count unchanged
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_exists() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add relationships
    relationship_registry_add(registry, 3, 2);
    relationship_registry_add(registry, 5, 2);
    
    // Check existence
    assert(relationship_registry_exists(registry, 3, 2) == true);
    assert(relationship_registry_exists(registry, 5, 2) == true);
    assert(relationship_registry_exists(registry, 7, 2) == false);
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_get_children() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add multiple children for parent 10
    relationship_registry_add(registry, 10, 1);
    relationship_registry_add(registry, 10, 3);
    relationship_registry_add(registry, 10, 7);
    relationship_registry_add(registry, 10, 9);
    
    // Get children
    uint64_t child_ids[10];
    size_t count = relationship_registry_get_children(registry, 10, child_ids, 10);
    assert(count == 4);
    
    // Verify children
    bool found_1 = false, found_3 = false, found_7 = false, found_9 = false;
    for (size_t i = 0; i < count; i++) {
        if (child_ids[i] == 1) found_1 = true;
        if (child_ids[i] == 3) found_3 = true;
        if (child_ids[i] == 7) found_7 = true;
        if (child_ids[i] == 9) found_9 = true;
    }
    assert(found_1 && found_3 && found_7 && found_9);
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_get_parent() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add relationships
    relationship_registry_add(registry, 10, 3);
    relationship_registry_add(registry, 10, 7);
    relationship_registry_add(registry, 5, 2);
    
    // Get parents
    assert(relationship_registry_get_parent(registry, 3) == 10);
    assert(relationship_registry_get_parent(registry, 7) == 10);
    assert(relationship_registry_get_parent(registry, 2) == 5);
    assert(relationship_registry_get_parent(registry, 99) == 0);  // Not found
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_expansion() {
    RelationshipRegistry* registry = relationship_registry_create(2);
    assert(registry != NULL);
    assert(registry->capacity == 2);
    
    // Add more than initial capacity
    relationship_registry_add(registry, 3, 2);
    relationship_registry_add(registry, 4, 1);
    relationship_registry_add(registry, 4, 3);  // Should trigger expansion
    
    assert(registry->count == 3);
    assert(registry->capacity > 2);  // Expanded
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_statistics() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add valid and invalid relationships
    relationship_registry_add(registry, 3, 2);   // Valid
    relationship_registry_add(registry, 5, 3);   // Invalid (both odd)
    relationship_registry_add(registry, 4, 2);   // Invalid (not coprime)
    relationship_registry_add(registry, 2, 3);   // Invalid (ordering)
    
    uint64_t total, valid, invalid, coprime_fail, both_odd_fail, ordering_fail;
    relationship_registry_get_stats(
        registry,
        &total,
        &valid,
        &invalid,
        &coprime_fail,
        &both_odd_fail,
        &ordering_fail
    );
    
    assert(total == 4);
    assert(valid == 1);
    assert(invalid == 3);
    assert(both_odd_fail >= 1);
    assert(coprime_fail >= 1);
    assert(ordering_fail >= 1);
    
    relationship_registry_destroy(registry);
    
    return 1;
}

int test_registry_validation() {
    RelationshipRegistry* registry = relationship_registry_create(10);
    assert(registry != NULL);
    
    // Add valid relationships
    relationship_registry_add(registry, 3, 2);
    relationship_registry_add(registry, 5, 2);
    
    // Validate registry
    assert(relationship_registry_validate(registry) == true);
    
    relationship_registry_destroy(registry);
    
    // Test NULL registry
    assert(relationship_registry_validate(NULL) == false);
    
    return 1;
}

// ============================================================================
// KNOWN PLIMPTON 322 PAIRS TESTS
// ============================================================================

int test_known_plimpton_pairs() {
    // Test first 6 known Plimpton 322 generator pairs
    // These are from the actual Plimpton 322 tablet
    
    // Row 1: (p=12, q=5) -> (119, 120, 169)
    assert(validate_parent_child_relation(12, 5) == true);
    
    // Row 2: (p=64, q=27) -> (3367, 3456, 4825)
    assert(validate_parent_child_relation(64, 27) == true);
    
    // Row 3: (p=75, q=32) -> (4601, 4800, 6649)
    assert(validate_parent_child_relation(75, 32) == true);
    
    // Row 4: (p=125, q=54) -> (12709, 13500, 18541)
    assert(validate_parent_child_relation(125, 54) == true);
    
    // Row 5: (p=9, q=4) -> (65, 72, 97)
    assert(validate_parent_child_relation(9, 4) == true);
    
    // Row 6: (p=20, q=9) -> (319, 360, 481)
    assert(validate_parent_child_relation(20, 9) == true);
    
    return 1;
}

int test_twelve_fold_symmetry() {
    // Test that we can generate 12 valid children for appropriate parents
    uint64_t child_ids[12];
    
    // Try with parent 100
    size_t count = generate_valid_children(100, child_ids, 12);
    assert(count == 12);  // Should be able to generate 12 children
    
    // Verify all are valid
    for (size_t i = 0; i < count; i++) {
        assert(validate_parent_child_relation(100, child_ids[i]) == true);
    }
    
    // Verify all are unique
    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            assert(child_ids[i] != child_ids[j]);
        }
    }
    
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    printf("=== Plimpton 322 Parent-Child Relationships Unit Tests ===\n\n");
    
    // Validation tests
    TEST(coprime_validation);
    TEST(both_odd_check);
    TEST(ordering_validation);
    TEST(valid_relationships);
    TEST(invalid_relationships);
    TEST(detailed_validation);
    
    // Relationship creation tests
    TEST(create_relation);
    TEST(find_valid_child);
    TEST(generate_children);
    
    // Registry tests
    TEST(registry_create_destroy);
    TEST(registry_add);
    TEST(registry_exists);
    TEST(registry_get_children);
    TEST(registry_get_parent);
    TEST(registry_expansion);
    TEST(registry_statistics);
    TEST(registry_validation);
    
    // Plimpton 322 specific tests
    TEST(known_plimpton_pairs);
    TEST(twelve_fold_symmetry);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    return tests_failed == 0 ? 0 : 1;
}