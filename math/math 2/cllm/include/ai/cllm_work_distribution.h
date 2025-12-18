/**
 * Plimpton 322 Work Distribution
 * 
 * Implements work distribution based on Plimpton 322 ratios:
 * - Parent keeps: (p²-q²)/(p²+q²) = b/d
 * - Child gets: 2pq/(p²+q²) = c/d
 * 
 * These ratios ensure fair work distribution while maintaining
 * mathematical properties of Pythagorean triples.
 * 
 * Phase 4, Day 13 Implementation
 */

#ifndef CLLM_WORK_DISTRIBUTION_H
#define CLLM_WORK_DISTRIBUTION_H

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
 * Work Distribution Ratios
 * 
 * Represents how work is split between parent and child based on
 * Plimpton 322 ratios.
 */
typedef struct {
    double parent_keeps;     // Ratio of work parent keeps (b/d)
    double child_gets;       // Ratio of work child gets (c/d)
    uint64_t parent_id;      // Parent thread ID (p)
    uint64_t child_id;       // Child thread ID (q)
    bool is_valid;           // Ratios are valid (sum to ~1.0)
} WorkDistribution;

/**
 * Multi-Child Work Distribution
 * 
 * Distributes work among parent and multiple children.
 */
typedef struct {
    double parent_keeps;     // Ratio parent keeps
    double* child_ratios;    // Ratio each child gets
    size_t num_children;     // Number of children
    uint64_t parent_id;      // Parent thread ID
    uint64_t* child_ids;     // Child thread IDs
    bool is_valid;           // Distribution is valid
} MultiChildDistribution;

/**
 * Work Distribution Statistics
 * 
 * Tracks work distribution calculations and performance.
 */
typedef struct {
    uint64_t total_calculations;
    uint64_t valid_distributions;
    uint64_t invalid_distributions;
    uint64_t ratio_sum_errors;
    double avg_parent_ratio;
    double avg_child_ratio;
    double min_parent_ratio;
    double max_parent_ratio;
    double min_child_ratio;
    double max_child_ratio;
} WorkDistributionStats;

// ============================================================================
// WORK DISTRIBUTION CALCULATION
// ============================================================================

/**
 * Calculate work distribution for parent-child pair
 * 
 * Uses Plimpton 322 ratios:
 * - parent_keeps = (p²-q²)/(p²+q²) = b/d
 * - child_gets = 2pq/(p²+q²) = c/d
 * 
 * @param parent_id Parent thread ID (p)
 * @param child_id Child thread ID (q)
 * @return Work distribution structure
 */
WorkDistribution calculate_work_distribution(
    uint64_t parent_id,
    uint64_t child_id
);

/**
 * Calculate work distribution with validation
 * 
 * Validates parent-child relationship before calculating distribution.
 * 
 * @param parent_id Parent thread ID
 * @param child_id Child thread ID
 * @param distribution Output: work distribution
 * @return true if valid, false otherwise
 */
bool calculate_work_distribution_validated(
    uint64_t parent_id,
    uint64_t child_id,
    WorkDistribution* distribution
);

/**
 * Calculate multi-child work distribution
 * 
 * Distributes work among parent and multiple children.
 * Parent keeps average of all child ratios.
 * 
 * @param parent_id Parent thread ID
 * @param child_ids Array of child thread IDs
 * @param num_children Number of children
 * @return Multi-child distribution structure
 */
MultiChildDistribution calculate_multi_child_distribution(
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children
);

// ============================================================================
// VALIDATION
// ============================================================================

/**
 * Validate work distribution
 * 
 * Checks that ratios sum to approximately 1.0 (within epsilon).
 * 
 * @param distribution Work distribution to validate
 * @param epsilon Tolerance for ratio sum (default: 0.0001)
 * @return true if valid, false otherwise
 */
bool validate_work_distribution(
    const WorkDistribution* distribution,
    double epsilon
);

/**
 * Validate multi-child distribution
 * 
 * Checks that all ratios sum to approximately 1.0.
 * 
 * @param distribution Multi-child distribution to validate
 * @param epsilon Tolerance for ratio sum
 * @return true if valid, false otherwise
 */
bool validate_multi_child_distribution(
    const MultiChildDistribution* distribution,
    double epsilon
);

/**
 * Check if ratios sum to 1.0
 * 
 * @param parent_ratio Parent's ratio
 * @param child_ratio Child's ratio
 * @param epsilon Tolerance
 * @return true if sum is within epsilon of 1.0
 */
bool ratios_sum_to_one(
    double parent_ratio,
    double child_ratio,
    double epsilon
);

// ============================================================================
// WORK SPLITTING
// ============================================================================

/**
 * Split work amount between parent and child
 * 
 * @param total_work Total amount of work to split
 * @param distribution Work distribution ratios
 * @param parent_work Output: work for parent
 * @param child_work Output: work for child
 * @return true if split successfully, false otherwise
 */
bool split_work(
    uint64_t total_work,
    const WorkDistribution* distribution,
    uint64_t* parent_work,
    uint64_t* child_work
);

/**
 * Split work among parent and multiple children
 * 
 * @param total_work Total amount of work to split
 * @param distribution Multi-child distribution
 * @param parent_work Output: work for parent
 * @param child_work Output: array of work for each child
 * @return true if split successfully, false otherwise
 */
bool split_work_multi_child(
    uint64_t total_work,
    const MultiChildDistribution* distribution,
    uint64_t* parent_work,
    uint64_t* child_work
);

/**
 * Calculate work amount from ratio
 * 
 * @param total_work Total work
 * @param ratio Work ratio (0.0 to 1.0)
 * @return Work amount
 */
uint64_t calculate_work_amount(
    uint64_t total_work,
    double ratio
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Initialize work distribution statistics
 * 
 * @param stats Statistics structure to initialize
 */
void work_distribution_stats_init(WorkDistributionStats* stats);

/**
 * Update statistics with new distribution
 * 
 * @param stats Statistics structure
 * @param distribution Work distribution
 */
void work_distribution_stats_update(
    WorkDistributionStats* stats,
    const WorkDistribution* distribution
);

/**
 * Get statistics
 * 
 * @param stats Statistics structure
 * @param total_calculations Output: total calculations
 * @param valid_distributions Output: valid distributions
 * @param invalid_distributions Output: invalid distributions
 * @param avg_parent_ratio Output: average parent ratio
 * @param avg_child_ratio Output: average child ratio
 */
void work_distribution_stats_get(
    const WorkDistributionStats* stats,
    uint64_t* total_calculations,
    uint64_t* valid_distributions,
    uint64_t* invalid_distributions,
    double* avg_parent_ratio,
    double* avg_child_ratio
);

/**
 * Print statistics
 * 
 * @param stats Statistics structure
 */
void work_distribution_stats_print(const WorkDistributionStats* stats);

/**
 * Reset statistics
 * 
 * @param stats Statistics structure
 */
void work_distribution_stats_reset(WorkDistributionStats* stats);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Create multi-child distribution
 * 
 * @param parent_id Parent thread ID
 * @param child_ids Array of child thread IDs
 * @param num_children Number of children
 * @return Allocated distribution, or NULL on error
 */
MultiChildDistribution* multi_child_distribution_create(
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children
);

/**
 * Destroy multi-child distribution
 * 
 * @param distribution Distribution to destroy
 */
void multi_child_distribution_destroy(MultiChildDistribution* distribution);

/**
 * Print work distribution
 * 
 * @param distribution Work distribution to print
 */
void print_work_distribution(const WorkDistribution* distribution);

/**
 * Print multi-child distribution
 * 
 * @param distribution Multi-child distribution to print
 */
void print_multi_child_distribution(const MultiChildDistribution* distribution);

/**
 * Get ratio sum
 * 
 * @param distribution Work distribution
 * @return Sum of parent and child ratios
 */
double get_ratio_sum(const WorkDistribution* distribution);

/**
 * Get multi-child ratio sum
 * 
 * @param distribution Multi-child distribution
 * @return Sum of all ratios
 */
double get_multi_child_ratio_sum(const MultiChildDistribution* distribution);

#ifdef __cplusplus
}
#endif

#endif // CLLM_WORK_DISTRIBUTION_H