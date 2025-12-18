/**
 * Plimpton 322 Work Distribution Implementation
 * 
 * Phase 4, Day 13
 */

#include "ai/cllm_work_distribution.h"
#include "ai/cllm_plimpton_relationships.h"
#include "math/angular_position.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"
#include "math/arithmetic.h"

// Default epsilon for ratio validation
#define DEFAULT_EPSILON 0.0001

// ============================================================================
// WORK DISTRIBUTION CALCULATION
// ============================================================================

WorkDistribution calculate_work_distribution(
    uint64_t parent_id,
    uint64_t child_id
) {
    WorkDistribution dist;
    memset(&dist, 0, sizeof(WorkDistribution));
    
    dist.parent_id = parent_id;
    dist.child_id = child_id;
    
    // Calculate Plimpton 322 ratios
    PlimptonRatios ratios = calculate_plimpton_ratios(parent_id, child_id);
    
    // Normalize ratios so they sum to 1.0
    // b/d and c/d are the two legs divided by hypotenuse
    // We need to normalize them: ratio_i = value_i / (value_1 + value_2)
    double sum = ratios.ratio_b_d + ratios.ratio_c_d;
    
    if (sum > 0.0) {
        // Parent keeps normalized b/d ratio
        dist.parent_keeps = ratios.ratio_b_d / sum;
        
        // Child gets normalized c/d ratio
        dist.child_gets = ratios.ratio_c_d / sum;
    } else {
        // Fallback to equal split
        dist.parent_keeps = 0.5;
        dist.child_gets = 0.5;
    }
    
    // Validate
    dist.is_valid = validate_work_distribution(&dist, DEFAULT_EPSILON);
    
    return dist;
}

bool calculate_work_distribution_validated(
    uint64_t parent_id,
    uint64_t child_id,
    WorkDistribution* distribution
) {
    if (!distribution) return false;
    
    // First validate the parent-child relationship
    if (!validate_parent_child_relation(parent_id, child_id)) {
        memset(distribution, 0, sizeof(WorkDistribution));
        distribution->parent_id = parent_id;
        distribution->child_id = child_id;
        distribution->is_valid = false;
        return false;
    }
    
    // Calculate distribution
    *distribution = calculate_work_distribution(parent_id, child_id);
    
    return distribution->is_valid;
}

MultiChildDistribution calculate_multi_child_distribution(
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children
) {
    MultiChildDistribution dist;
    memset(&dist, 0, sizeof(MultiChildDistribution));
    
    if (!child_ids || num_children == 0) {
        return dist;
    }
    
    dist.parent_id = parent_id;
    dist.num_children = num_children;
    
    // Allocate arrays
    dist.child_ids = malloc(num_children * sizeof(uint64_t));
    dist.child_ratios = malloc(num_children * sizeof(double));
    
    if (!dist.child_ids || !dist.child_ratios) {
        free(dist.child_ids);
        free(dist.child_ratios);
        memset(&dist, 0, sizeof(MultiChildDistribution));
        return dist;
    }
    
    // Copy child IDs
    memcpy(dist.child_ids, child_ids, num_children * sizeof(uint64_t));
    
    // Calculate unnormalized ratio for each child
    double child_ratios_unnormalized[num_children];
    double total_unnormalized = 0.0;
    
    for (size_t i = 0; i < num_children; i++) {
        WorkDistribution child_dist = calculate_work_distribution(
            parent_id,
            child_ids[i]
        );
        
        // Use the child's ratio (already normalized for single child)
        child_ratios_unnormalized[i] = child_dist.child_gets;
        total_unnormalized += child_dist.child_gets;
    }
    
    // Normalize all ratios (including parent) so they sum to 1.0
    // Give parent a base ratio, then distribute rest among children
    double parent_base_ratio = 1.0 / (num_children + 1.0);  // Equal share as baseline
    double children_total_ratio = 1.0 - parent_base_ratio;
    
    // Normalize child ratios to fit in children_total_ratio
    if (total_unnormalized > 0.0) {
        for (size_t i = 0; i < num_children; i++) {
            dist.child_ratios[i] = (child_ratios_unnormalized[i] / total_unnormalized) * children_total_ratio;
        }
    } else {
        // Fallback to equal distribution
        for (size_t i = 0; i < num_children; i++) {
            dist.child_ratios[i] = children_total_ratio / num_children;
        }
    }
    
    dist.parent_keeps = parent_base_ratio;
    
    // Validate
    dist.is_valid = validate_multi_child_distribution(&dist, DEFAULT_EPSILON);
    
    return dist;
}

// ============================================================================
// VALIDATION
// ============================================================================

bool ratios_sum_to_one(
    double parent_ratio,
    double child_ratio,
    double epsilon
) {
    double sum = parent_ratio + child_ratio;
    return math_abs(sum - 1.0) < epsilon;
}

bool validate_work_distribution(
    const WorkDistribution* distribution,
    double epsilon
) {
    if (!distribution) return false;
    
    // Check ratios are in valid range [0, 1]
    if (distribution->parent_keeps < 0.0 || distribution->parent_keeps > 1.0) {
        return false;
    }
    if (distribution->child_gets < 0.0 || distribution->child_gets > 1.0) {
        return false;
    }
    
    // Check ratios sum to approximately 1.0
    return ratios_sum_to_one(
        distribution->parent_keeps,
        distribution->child_gets,
        epsilon
    );
}

bool validate_multi_child_distribution(
    const MultiChildDistribution* distribution,
    double epsilon
) {
    if (!distribution || !distribution->child_ratios) {
        return false;
    }
    
    // Check parent ratio is in valid range
    if (distribution->parent_keeps < 0.0 || distribution->parent_keeps > 1.0) {
        return false;
    }
    
    // Check all child ratios are in valid range
    for (size_t i = 0; i < distribution->num_children; i++) {
        if (distribution->child_ratios[i] < 0.0 || 
            distribution->child_ratios[i] > 1.0) {
            return false;
        }
    }
    
    // Check all ratios sum to approximately 1.0
    double sum = distribution->parent_keeps;
    for (size_t i = 0; i < distribution->num_children; i++) {
        sum += distribution->child_ratios[i];
    }
    
    return math_abs(sum - 1.0) < epsilon;
}

// ============================================================================
// WORK SPLITTING
// ============================================================================

uint64_t calculate_work_amount(
    uint64_t total_work,
    double ratio
) {
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;
    
    return (uint64_t)(total_work * ratio);
}

bool split_work(
    uint64_t total_work,
    const WorkDistribution* distribution,
    uint64_t* parent_work,
    uint64_t* child_work
) {
    if (!distribution || !parent_work || !child_work) {
        return false;
    }
    
    if (!distribution->is_valid) {
        return false;
    }
    
    // Calculate work amounts
    *parent_work = calculate_work_amount(total_work, distribution->parent_keeps);
    *child_work = calculate_work_amount(total_work, distribution->child_gets);
    
    // Adjust for rounding errors
    uint64_t total_assigned = *parent_work + *child_work;
    if (total_assigned < total_work) {
        // Give remainder to parent
        *parent_work += (total_work - total_assigned);
    } else if (total_assigned > total_work) {
        // Take from parent
        uint64_t excess = total_assigned - total_work;
        if (*parent_work >= excess) {
            *parent_work -= excess;
        } else {
            return false;
        }
    }
    
    return true;
}

bool split_work_multi_child(
    uint64_t total_work,
    const MultiChildDistribution* distribution,
    uint64_t* parent_work,
    uint64_t* child_work
) {
    if (!distribution || !parent_work || !child_work) {
        return false;
    }
    
    if (!distribution->is_valid) {
        return false;
    }
    
    // Calculate parent work
    *parent_work = calculate_work_amount(total_work, distribution->parent_keeps);
    
    // Calculate work for each child
    uint64_t total_child_work = 0;
    for (size_t i = 0; i < distribution->num_children; i++) {
        child_work[i] = calculate_work_amount(
            total_work,
            distribution->child_ratios[i]
        );
        total_child_work += child_work[i];
    }
    
    // Adjust for rounding errors
    uint64_t total_assigned = *parent_work + total_child_work;
    if (total_assigned < total_work) {
        // Give remainder to parent
        *parent_work += (total_work - total_assigned);
    } else if (total_assigned > total_work) {
        // Take from parent
        uint64_t excess = total_assigned - total_work;
        if (*parent_work >= excess) {
            *parent_work -= excess;
        } else {
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// STATISTICS
// ============================================================================

void work_distribution_stats_init(WorkDistributionStats* stats) {
    if (!stats) return;
    
    memset(stats, 0, sizeof(WorkDistributionStats));
    stats->min_parent_ratio = 1.0;
    stats->min_child_ratio = 1.0;
}

void work_distribution_stats_update(
    WorkDistributionStats* stats,
    const WorkDistribution* distribution
) {
    if (!stats || !distribution) return;
    
    stats->total_calculations++;
    
    if (distribution->is_valid) {
        stats->valid_distributions++;
        
        // Update averages
        double n = (double)stats->valid_distributions;
        stats->avg_parent_ratio = 
            (stats->avg_parent_ratio * (n - 1.0) + distribution->parent_keeps) / n;
        stats->avg_child_ratio = 
            (stats->avg_child_ratio * (n - 1.0) + distribution->child_gets) / n;
        
        // Update min/max
        if (distribution->parent_keeps < stats->min_parent_ratio) {
            stats->min_parent_ratio = distribution->parent_keeps;
        }
        if (distribution->parent_keeps > stats->max_parent_ratio) {
            stats->max_parent_ratio = distribution->parent_keeps;
        }
        if (distribution->child_gets < stats->min_child_ratio) {
            stats->min_child_ratio = distribution->child_gets;
        }
        if (distribution->child_gets > stats->max_child_ratio) {
            stats->max_child_ratio = distribution->child_gets;
        }
    } else {
        stats->invalid_distributions++;
        
        // Check if it's a ratio sum error
        if (!ratios_sum_to_one(
            distribution->parent_keeps,
            distribution->child_gets,
            DEFAULT_EPSILON
        )) {
            stats->ratio_sum_errors++;
        }
    }
}

void work_distribution_stats_get(
    const WorkDistributionStats* stats,
    uint64_t* total_calculations,
    uint64_t* valid_distributions,
    uint64_t* invalid_distributions,
    double* avg_parent_ratio,
    double* avg_child_ratio
) {
    if (!stats) return;
    
    if (total_calculations) *total_calculations = stats->total_calculations;
    if (valid_distributions) *valid_distributions = stats->valid_distributions;
    if (invalid_distributions) *invalid_distributions = stats->invalid_distributions;
    if (avg_parent_ratio) *avg_parent_ratio = stats->avg_parent_ratio;
    if (avg_child_ratio) *avg_child_ratio = stats->avg_child_ratio;
}

void work_distribution_stats_print(const WorkDistributionStats* stats) {
    if (!stats) return;
    
    printf("Work Distribution Statistics:\n");
    printf("  Total Calculations:      %llu\n", 
           (unsigned long long)stats->total_calculations);
    printf("  Valid Distributions:     %llu\n", 
           (unsigned long long)stats->valid_distributions);
    printf("  Invalid Distributions:   %llu\n", 
           (unsigned long long)stats->invalid_distributions);
    printf("  Ratio Sum Errors:        %llu\n", 
           (unsigned long long)stats->ratio_sum_errors);
    
    if (stats->valid_distributions > 0) {
        printf("  Avg Parent Ratio:        %.6f\n", stats->avg_parent_ratio);
        printf("  Avg Child Ratio:         %.6f\n", stats->avg_child_ratio);
        printf("  Min Parent Ratio:        %.6f\n", stats->min_parent_ratio);
        printf("  Max Parent Ratio:        %.6f\n", stats->max_parent_ratio);
        printf("  Min Child Ratio:         %.6f\n", stats->min_child_ratio);
        printf("  Max Child Ratio:         %.6f\n", stats->max_child_ratio);
    }
}

void work_distribution_stats_reset(WorkDistributionStats* stats) {
    if (!stats) return;
    
    work_distribution_stats_init(stats);
}

// ============================================================================
// UTILITIES
// ============================================================================

MultiChildDistribution* multi_child_distribution_create(
    uint64_t parent_id,
    const uint64_t* child_ids,
    size_t num_children
) {
    MultiChildDistribution* dist = malloc(sizeof(MultiChildDistribution));
    if (!dist) {
        fprintf(stderr, "ERROR: Failed to allocate multi-child distribution\n");
        return NULL;
    }
    
    *dist = calculate_multi_child_distribution(parent_id, child_ids, num_children);
    
    if (!dist->is_valid) {
        multi_child_distribution_destroy(dist);
        return NULL;
    }
    
    return dist;
}

void multi_child_distribution_destroy(MultiChildDistribution* distribution) {
    if (!distribution) return;
    
    free(distribution->child_ids);
    free(distribution->child_ratios);
    free(distribution);
}

void print_work_distribution(const WorkDistribution* distribution) {
    if (!distribution) return;
    
    printf("Work Distribution:\n");
    printf("  Parent ID:       %llu\n", (unsigned long long)distribution->parent_id);
    printf("  Child ID:        %llu\n", (unsigned long long)distribution->child_id);
    printf("  Parent Keeps:    %.6f (%.2f%%)\n", 
           distribution->parent_keeps, distribution->parent_keeps * 100.0);
    printf("  Child Gets:      %.6f (%.2f%%)\n", 
           distribution->child_gets, distribution->child_gets * 100.0);
    printf("  Ratio Sum:       %.6f\n", get_ratio_sum(distribution));
    printf("  Valid:           %s\n", distribution->is_valid ? "Yes" : "No");
}

void print_multi_child_distribution(const MultiChildDistribution* distribution) {
    if (!distribution) return;
    
    printf("Multi-Child Work Distribution:\n");
    printf("  Parent ID:       %llu\n", (unsigned long long)distribution->parent_id);
    printf("  Num Children:    %zu\n", distribution->num_children);
    printf("  Parent Keeps:    %.6f (%.2f%%)\n", 
           distribution->parent_keeps, distribution->parent_keeps * 100.0);
    
    for (size_t i = 0; i < distribution->num_children; i++) {
        printf("  Child %zu (ID %llu): %.6f (%.2f%%)\n",
               i,
               (unsigned long long)distribution->child_ids[i],
               distribution->child_ratios[i],
               distribution->child_ratios[i] * 100.0);
    }
    
    printf("  Ratio Sum:       %.6f\n", get_multi_child_ratio_sum(distribution));
    printf("  Valid:           %s\n", distribution->is_valid ? "Yes" : "No");
}

double get_ratio_sum(const WorkDistribution* distribution) {
    if (!distribution) return 0.0;
    
    return distribution->parent_keeps + distribution->child_gets;
}

double get_multi_child_ratio_sum(const MultiChildDistribution* distribution) {
    if (!distribution) return 0.0;
    
    double sum = distribution->parent_keeps;
    for (size_t i = 0; i < distribution->num_children; i++) {
        sum += distribution->child_ratios[i];
    }
    
    return sum;
}