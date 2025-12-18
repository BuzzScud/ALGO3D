/**
 * Plimpton 322 Parent-Child Relationships Implementation
 * 
 * Phase 4, Day 13
 * 
 * REDESIGNED: Independent implementation, NO OLD library dependencies
 */

#include "ai/cllm_plimpton_relationships.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// INDEPENDENT MATHEMATICAL FUNCTIONS
// ============================================================================

/**
 * @brief Compute GCD using Euclidean algorithm
 * 
 * Independent implementation - NO dependencies on OLD library
 */
static uint64_t compute_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

bool is_coprime(uint64_t a, uint64_t b) {
    if (a == 0 || b == 0) return false;
    return compute_gcd(a, b) == 1;
}

bool both_odd(uint64_t a, uint64_t b) {
    return (a % 2 == 1) && (b % 2 == 1);
}

bool valid_ordering(uint64_t parent_id, uint64_t child_id) {
    return (parent_id > child_id) && (child_id > 0);
}

bool validate_parent_child_relation(
    uint64_t parent_id,
    uint64_t child_id
) {
    // Check ordering constraint: p > q > 0
    if (!valid_ordering(parent_id, child_id)) {
        return false;
    }
    
    // Check coprime constraint: gcd(p,q) = 1
    if (!is_coprime(parent_id, child_id)) {
        return false;
    }
    
    // Check not both odd
    if (both_odd(parent_id, child_id)) {
        return false;
    }
    
    return true;
}

bool validate_relation_detailed(
    uint64_t parent_id,
    uint64_t child_id,
    ParentChildRelation* relation
) {
    if (!relation) return false;
    
    relation->parent_id = parent_id;
    relation->child_id = child_id;
    relation->gcd_value = compute_gcd(parent_id, child_id);
    relation->is_coprime = (relation->gcd_value == 1);
    
    // Check all constraints
    bool ordering_ok = valid_ordering(parent_id, child_id);
    bool coprime_ok = relation->is_coprime;
    bool not_both_odd = !both_odd(parent_id, child_id);
    
    relation->is_valid = ordering_ok && coprime_ok && not_both_odd;
    
    return relation->is_valid;
}

// ============================================================================
// RELATIONSHIP CREATION
// ============================================================================

ParentChildRelation create_parent_child_relation(
    uint64_t parent_id,
    uint64_t child_id
) {
    ParentChildRelation relation;
    validate_relation_detailed(parent_id, child_id, &relation);
    return relation;
}

uint64_t find_valid_child_id(
    uint64_t parent_id,
    uint64_t start_search,
    uint64_t max_attempts
) {
    if (parent_id == 0) return 0;
    if (start_search == 0) start_search = 1;
    if (start_search >= parent_id) return 0;
    
    for (uint64_t i = 0; i < max_attempts; i++) {
        uint64_t candidate = start_search + i;
        if (candidate >= parent_id) break;
        
        if (validate_parent_child_relation(parent_id, candidate)) {
            return candidate;
        }
    }
    
    return 0;
}

size_t generate_valid_children(
    uint64_t parent_id,
    uint64_t* child_ids,
    size_t max_children
) {
    if (!child_ids || max_children == 0 || parent_id == 0) {
        return 0;
    }
    
    size_t count = 0;
    uint64_t search_start = 1;
    
    while (count < max_children && search_start < parent_id) {
        uint64_t child_id = find_valid_child_id(
            parent_id,
            search_start,
            parent_id - search_start
        );
        
        if (child_id == 0) break;
        
        child_ids[count++] = child_id;
        search_start = child_id + 1;
    }
    
    return count;
}

// ============================================================================
// REGISTRY MANAGEMENT
// ============================================================================

RelationshipRegistry* relationship_registry_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 128;
    }
    
    RelationshipRegistry* registry = calloc(1, sizeof(RelationshipRegistry));
    if (!registry) {
        fprintf(stderr, "ERROR: Failed to allocate relationship registry\n");
        return NULL;
    }
    
    registry->relationships = calloc(initial_capacity, sizeof(ParentChildRelation));
    if (!registry->relationships) {
        fprintf(stderr, "ERROR: Failed to allocate relationships array\n");
        free(registry);
        return NULL;
    }
    
    registry->capacity = initial_capacity;
    registry->count = 0;
    
    // Initialize validator
    memset(&registry->validator, 0, sizeof(RelationshipValidator));
    
    return registry;
}

void relationship_registry_destroy(RelationshipRegistry* registry) {
    if (!registry) return;
    
    free(registry->relationships);
    free(registry);
}

bool relationship_registry_add(
    RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t child_id
) {
    if (!registry) return false;
    
    // Validate relationship
    ParentChildRelation relation = create_parent_child_relation(parent_id, child_id);
    
    // Update validator statistics
    registry->validator.total_validations++;
    
    if (!relation.is_valid) {
        registry->validator.invalid_relationships++;
        
        // Track failure reasons
        if (!relation.is_coprime) {
            registry->validator.coprime_failures++;
        }
        if (both_odd(parent_id, child_id)) {
            registry->validator.both_odd_failures++;
        }
        if (!valid_ordering(parent_id, child_id)) {
            registry->validator.ordering_failures++;
        }
        
        return false;
    }
    
    registry->validator.valid_relationships++;
    
    // Check if we need to expand
    if (registry->count >= registry->capacity) {
        size_t new_capacity = registry->capacity * 2;
        ParentChildRelation* new_relationships = realloc(
            registry->relationships,
            new_capacity * sizeof(ParentChildRelation)
        );
        
        if (!new_relationships) {
            fprintf(stderr, "ERROR: Failed to expand relationships array\n");
            return false;
        }
        
        registry->relationships = new_relationships;
        registry->capacity = new_capacity;
    }
    
    // Add relationship
    registry->relationships[registry->count++] = relation;
    
    return true;
}

bool relationship_registry_exists(
    const RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t child_id
) {
    if (!registry) return false;
    
    for (size_t i = 0; i < registry->count; i++) {
        if (registry->relationships[i].parent_id == parent_id &&
            registry->relationships[i].child_id == child_id) {
            return true;
        }
    }
    
    return false;
}

size_t relationship_registry_get_children(
    const RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t* child_ids,
    size_t max_children
) {
    if (!registry || !child_ids || max_children == 0) {
        return 0;
    }
    
    size_t count = 0;
    
    for (size_t i = 0; i < registry->count && count < max_children; i++) {
        if (registry->relationships[i].parent_id == parent_id) {
            child_ids[count++] = registry->relationships[i].child_id;
        }
    }
    
    return count;
}

uint64_t relationship_registry_get_parent(
    const RelationshipRegistry* registry,
    uint64_t child_id
) {
    if (!registry) return 0;
    
    for (size_t i = 0; i < registry->count; i++) {
        if (registry->relationships[i].child_id == child_id) {
            return registry->relationships[i].parent_id;
        }
    }
    
    return 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

void relationship_registry_get_stats(
    const RelationshipRegistry* registry,
    uint64_t* total_validations,
    uint64_t* valid_relationships,
    uint64_t* invalid_relationships,
    uint64_t* coprime_failures,
    uint64_t* both_odd_failures,
    uint64_t* ordering_failures
) {
    if (!registry) return;
    
    if (total_validations) {
        *total_validations = registry->validator.total_validations;
    }
    if (valid_relationships) {
        *valid_relationships = registry->validator.valid_relationships;
    }
    if (invalid_relationships) {
        *invalid_relationships = registry->validator.invalid_relationships;
    }
    if (coprime_failures) {
        *coprime_failures = registry->validator.coprime_failures;
    }
    if (both_odd_failures) {
        *both_odd_failures = registry->validator.both_odd_failures;
    }
    if (ordering_failures) {
        *ordering_failures = registry->validator.ordering_failures;
    }
}

void relationship_registry_print_stats(const RelationshipRegistry* registry) {
    if (!registry) return;
    
    printf("Relationship Registry Statistics:\n");
    printf("  Total Validations:      %llu\n", 
           (unsigned long long)registry->validator.total_validations);
    printf("  Valid Relationships:    %llu\n", 
           (unsigned long long)registry->validator.valid_relationships);
    printf("  Invalid Relationships:  %llu\n", 
           (unsigned long long)registry->validator.invalid_relationships);
    printf("  Coprime Failures:       %llu\n", 
           (unsigned long long)registry->validator.coprime_failures);
    printf("  Both Odd Failures:      %llu\n", 
           (unsigned long long)registry->validator.both_odd_failures);
    printf("  Ordering Failures:      %llu\n", 
           (unsigned long long)registry->validator.ordering_failures);
    printf("  Registered Count:       %zu\n", registry->count);
    printf("  Registry Capacity:      %zu\n", registry->capacity);
}

void relationship_registry_reset_stats(RelationshipRegistry* registry) {
    if (!registry) return;
    
    memset(&registry->validator, 0, sizeof(RelationshipValidator));
}

// ============================================================================
// UTILITIES
// ============================================================================

bool relationship_registry_validate(const RelationshipRegistry* registry) {
    if (!registry) {
        fprintf(stderr, "ERROR: NULL registry\n");
        return false;
    }
    
    if (!registry->relationships) {
        fprintf(stderr, "ERROR: NULL relationships array\n");
        return false;
    }
    
    if (registry->count > registry->capacity) {
        fprintf(stderr, "ERROR: Count exceeds capacity\n");
        return false;
    }
    
    // Validate all relationships
    for (size_t i = 0; i < registry->count; i++) {
        const ParentChildRelation* rel = &registry->relationships[i];
        
        if (!rel->is_valid) {
            fprintf(stderr, "ERROR: Invalid relationship at index %zu\n", i);
            return false;
        }
        
        if (!validate_parent_child_relation(rel->parent_id, rel->child_id)) {
            fprintf(stderr, "ERROR: Relationship validation failed at index %zu\n", i);
            return false;
        }
    }
    
    return true;
}

void print_parent_child_relation(const ParentChildRelation* relation) {
    if (!relation) return;
    
    printf("Parent-Child Relationship:\n");
    printf("  Parent ID:  %llu\n", (unsigned long long)relation->parent_id);
    printf("  Child ID:   %llu\n", (unsigned long long)relation->child_id);
    printf("  GCD:        %llu\n", (unsigned long long)relation->gcd_value);
    printf("  Coprime:    %s\n", relation->is_coprime ? "Yes" : "No");
    printf("  Valid:      %s\n", relation->is_valid ? "Yes" : "No");
}

void relationship_registry_print_all(const RelationshipRegistry* registry) {
    if (!registry) return;
    
    printf("Relationship Registry (%zu relationships):\n", registry->count);
    
    for (size_t i = 0; i < registry->count; i++) {
        printf("  [%zu] Parent: %llu, Child: %llu, GCD: %llu, Valid: %s\n",
               i,
               (unsigned long long)registry->relationships[i].parent_id,
               (unsigned long long)registry->relationships[i].child_id,
               (unsigned long long)registry->relationships[i].gcd_value,
               registry->relationships[i].is_valid ? "Yes" : "No");
    }
}