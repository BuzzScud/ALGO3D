/**
 * Plimpton 322 Parent-Child Relationships
 * 
 * Implements parent-child relationships based on Plimpton 322 constraints:
 * - gcd(p,q) = 1 (coprime)
 * - Not both odd
 * - p > q > 0
 * 
 * These constraints ensure valid Pythagorean triples and proper work distribution.
 * 
 * Phase 4, Day 13 Implementation
 */

#ifndef CLLM_PLIMPTON_RELATIONSHIPS_H
#define CLLM_PLIMPTON_RELATIONSHIPS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Parent-Child Relationship
 * 
 * Represents a relationship between a parent thread and child thread
 * using Plimpton 322 constraints.
 */
typedef struct {
    uint64_t parent_id;      // p (must be > q)
    uint64_t child_id;       // q (must be < p)
    bool is_coprime;         // gcd(p,q) = 1
    bool is_valid;           // All constraints satisfied
    uint64_t gcd_value;      // Cached GCD value
} ParentChildRelation;

/**
 * Relationship Validator
 * 
 * Validates parent-child relationships against Plimpton constraints.
 */
typedef struct {
    uint64_t total_validations;
    uint64_t valid_relationships;
    uint64_t invalid_relationships;
    uint64_t coprime_failures;
    uint64_t both_odd_failures;
    uint64_t ordering_failures;
} RelationshipValidator;

/**
 * Relationship Registry
 * 
 * Tracks all parent-child relationships in the system.
 */
typedef struct {
    ParentChildRelation* relationships;
    size_t count;
    size_t capacity;
    RelationshipValidator validator;
} RelationshipRegistry;

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

/**
 * Validate parent-child relationship
 * 
 * Checks all Plimpton 322 constraints:
 * 1. gcd(p,q) = 1 (coprime)
 * 2. Not both odd
 * 3. p > q > 0
 * 
 * @param parent_id Parent thread ID (p)
 * @param child_id Child thread ID (q)
 * @return true if all constraints satisfied, false otherwise
 */
bool validate_parent_child_relation(
    uint64_t parent_id,
    uint64_t child_id
);

/**
 * Check if two numbers are coprime
 * 
 * @param a First number
 * @param b Second number
 * @return true if gcd(a,b) = 1, false otherwise
 */
bool is_coprime(uint64_t a, uint64_t b);

/**
 * Check if both numbers are odd
 * 
 * @param a First number
 * @param b Second number
 * @return true if both are odd, false otherwise
 */
bool both_odd(uint64_t a, uint64_t b);

/**
 * Check ordering constraint
 * 
 * @param parent_id Parent ID
 * @param child_id Child ID
 * @return true if parent_id > child_id > 0, false otherwise
 */
bool valid_ordering(uint64_t parent_id, uint64_t child_id);

/**
 * Get detailed validation result
 * 
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @param relation Output: detailed relationship information
 * @return true if valid, false otherwise
 */
bool validate_relation_detailed(
    uint64_t parent_id,
    uint64_t child_id,
    ParentChildRelation* relation
);

// ============================================================================
// RELATIONSHIP CREATION
// ============================================================================

/**
 * Create parent-child relationship
 * 
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @return Relationship structure (check is_valid field)
 */
ParentChildRelation create_parent_child_relation(
    uint64_t parent_id,
    uint64_t child_id
);

/**
 * Find valid child ID for parent
 * 
 * Finds a valid child ID that satisfies all Plimpton constraints
 * for the given parent ID.
 * 
 * @param parent_id Parent thread ID
 * @param start_search Starting point for search
 * @param max_attempts Maximum search attempts
 * @return Valid child ID, or 0 if none found
 */
uint64_t find_valid_child_id(
    uint64_t parent_id,
    uint64_t start_search,
    uint64_t max_attempts
);

/**
 * Generate valid child IDs for parent
 * 
 * Generates up to max_children valid child IDs for the given parent.
 * 
 * @param parent_id Parent thread ID
 * @param child_ids Output: array of valid child IDs
 * @param max_children Maximum number of children to generate
 * @return Number of valid children generated
 */
size_t generate_valid_children(
    uint64_t parent_id,
    uint64_t* child_ids,
    size_t max_children
);

// ============================================================================
// REGISTRY MANAGEMENT
// ============================================================================

/**
 * Create relationship registry
 * 
 * @param initial_capacity Initial capacity for relationships
 * @return Allocated registry, or NULL on error
 */
RelationshipRegistry* relationship_registry_create(size_t initial_capacity);

/**
 * Destroy relationship registry
 * 
 * @param registry Registry to destroy
 */
void relationship_registry_destroy(RelationshipRegistry* registry);

/**
 * Register parent-child relationship
 * 
 * @param registry Relationship registry
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @return true if registered successfully, false otherwise
 */
bool relationship_registry_add(
    RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t child_id
);

/**
 * Check if relationship exists
 * 
 * @param registry Relationship registry
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @return true if relationship exists, false otherwise
 */
bool relationship_registry_exists(
    const RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t child_id
);

/**
 * Get all children for parent
 * 
 * @param registry Relationship registry
 * @param parent_id Parent thread ID
 * @param child_ids Output: array of child IDs
 * @param max_children Maximum number of children to return
 * @return Number of children found
 */
size_t relationship_registry_get_children(
    const RelationshipRegistry* registry,
    uint64_t parent_id,
    uint64_t* child_ids,
    size_t max_children
);

/**
 * Get parent for child
 * 
 * @param registry Relationship registry
 * @param child_id Child thread ID
 * @return Parent ID, or 0 if not found
 */
uint64_t relationship_registry_get_parent(
    const RelationshipRegistry* registry,
    uint64_t child_id
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Get validator statistics
 * 
 * @param registry Relationship registry
 * @param total_validations Output: total validations performed
 * @param valid_relationships Output: number of valid relationships
 * @param invalid_relationships Output: number of invalid relationships
 * @param coprime_failures Output: failures due to non-coprime
 * @param both_odd_failures Output: failures due to both odd
 * @param ordering_failures Output: failures due to ordering
 */
void relationship_registry_get_stats(
    const RelationshipRegistry* registry,
    uint64_t* total_validations,
    uint64_t* valid_relationships,
    uint64_t* invalid_relationships,
    uint64_t* coprime_failures,
    uint64_t* both_odd_failures,
    uint64_t* ordering_failures
);

/**
 * Print validator statistics
 * 
 * @param registry Relationship registry
 */
void relationship_registry_print_stats(const RelationshipRegistry* registry);

/**
 * Reset validator statistics
 * 
 * @param registry Relationship registry
 */
void relationship_registry_reset_stats(RelationshipRegistry* registry);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Validate relationship registry
 * 
 * @param registry Relationship registry
 * @return true if valid, false otherwise
 */
bool relationship_registry_validate(const RelationshipRegistry* registry);

/**
 * Print relationship
 * 
 * @param relation Relationship to print
 */
void print_parent_child_relation(const ParentChildRelation* relation);

/**
 * Print all relationships in registry
 * 
 * @param registry Relationship registry
 */
void relationship_registry_print_all(const RelationshipRegistry* registry);

#ifdef __cplusplus
}
#endif

#endif // CLLM_PLIMPTON_RELATIONSHIPS_H
