/**
 * @file cllm_entropy_work_distribution.c
 * @brief Implementation of entropy-based work distribution
 */

#include "ai/cllm_entropy_work_distribution.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "math/transcendental.h"

/**
 * @brief Small epsilon for floating point comparisons
 */
#define MATH_EPSILON 1e-10

/**
 * @brief Initialize work distribution configuration
 */
void work_distribution_config_init(WorkDistributionConfig* config) {
    if (!config) {
        return;
    }
    
    config->strategy = WORK_DIST_COMBINED;
    config->entropy_weight = 0.7;      // 70% entropy, 30% Plimpton
    config->plimpton_weight = 0.3;
    config->enforce_12fold = true;
    config->min_work_per_thread = 1;
    config->max_work_per_thread = MAX_BATCH_SIZE;
}

/**
 * @brief Create work distribution plan
 */
WorkDistributionPlan* work_distribution_plan_create(
    size_t total_work,
    size_t max_assignments
) {
    if (total_work == 0 || max_assignments == 0) {
        return NULL;
    }
    
    WorkDistributionPlan* plan = (WorkDistributionPlan*)calloc(1, sizeof(WorkDistributionPlan));
    if (!plan) {
        return NULL;
    }
    
    plan->assignments = (WorkAssignment*)calloc(max_assignments, sizeof(WorkAssignment));
    if (!plan->assignments) {
        free(plan);
        return NULL;
    }
    
    plan->num_assignments = 0;
    plan->total_work_size = total_work;
    plan->distributed_work = 0;
    plan->total_entropy = 0.0;
    plan->use_plimpton_ratios = false;
    plan->enforce_12fold = true;
    
    return plan;
}

/**
 * @brief Destroy work distribution plan
 */
void work_distribution_plan_destroy(WorkDistributionPlan* plan) {
    if (!plan) {
        return;
    }
    
    if (plan->assignments) {
        free(plan->assignments);
    }
    
    free(plan);
}

/**
 * @brief Calculate work for specific dimension
 */
size_t calculate_dimension_work(
    uint32_t dimension,
    double entropy,
    double total_entropy,
    size_t total_work
) {
    (void)dimension;  // Unused for now
    
    if (total_entropy < MATH_EPSILON || total_work == 0) {
        return 0;
    }
    
    // Work proportional to entropy
    double fraction = entropy / total_entropy;
    size_t work = (size_t)(fraction * total_work);
    
    return work;
}

/**
 * @brief Calculate combined work distribution
 */
WorkDistribution calculate_combined_work_distribution(
    size_t parent_work,
    uint64_t parent_id,
    uint64_t child_id,
    double parent_entropy,
    double child_entropy
) {
    WorkDistribution dist;
    memset(&dist, 0, sizeof(WorkDistribution));
    
    dist.parent_id = parent_id;
    dist.child_id = child_id;
    
    if (parent_work == 0) {
        dist.is_valid = false;
        return dist;
    }
    
    // Calculate Plimpton ratios
    WorkDistribution plimpton = calculate_work_distribution(parent_id, child_id);
    
    // Calculate entropy ratios
    double total_entropy = parent_entropy + child_entropy;
    double entropy_parent_ratio = 0.5;  // Default to equal
    double entropy_child_ratio = 0.5;
    
    if (total_entropy > MATH_EPSILON) {
        entropy_parent_ratio = parent_entropy / total_entropy;
        entropy_child_ratio = child_entropy / total_entropy;
    }
    
    // Combine ratios (weighted average)
    // 70% entropy, 30% Plimpton (configurable)
    double entropy_weight = 0.7;
    double plimpton_weight = 0.3;
    
    dist.parent_keeps = entropy_weight * entropy_parent_ratio + 
                        plimpton_weight * plimpton.parent_keeps;
    dist.child_gets = entropy_weight * entropy_child_ratio + 
                      plimpton_weight * plimpton.child_gets;
    
    // Normalize to ensure sum = 1.0
    double sum = dist.parent_keeps + dist.child_gets;
    if (sum > MATH_EPSILON) {
        dist.parent_keeps /= sum;
        dist.child_gets /= sum;
    }
    
    dist.is_valid = (math_abs(dist.parent_keeps + dist.child_gets - 1.0) < MATH_EPSILON);
    
    return dist;
}

/**
 * @brief Calculate entropy-based work distribution
 */
bool calculate_entropy_work_distribution(
    EntropyIntegrationContext* entropy_ctx,
    const ThreadAllocationPlan* allocation_plan,
    size_t total_work,
    const WorkDistributionConfig* config,
    WorkDistributionPlan* plan
) {
    if (!entropy_ctx || !allocation_plan || !config || !plan) {
        return false;
    }
    
    if (total_work == 0) {
        return false;
    }
    
    // Get entropy distribution
    double entropies[MAX_ALLOCATION_DIMENSIONS];
    calculate_entropy_distribution(entropy_ctx, 100, entropies);
    
    // Calculate total entropy
    double total_entropy = 0.0;
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        total_entropy += entropies[i];
    }
    
    if (total_entropy < MATH_EPSILON) {
        return false;
    }
    
    plan->total_entropy = total_entropy;
    plan->use_plimpton_ratios = (config->strategy == WORK_DIST_PLIMPTON_ONLY ||
                                  config->strategy == WORK_DIST_COMBINED);
    plan->enforce_12fold = config->enforce_12fold;
    
    // Distribute work across active dimensions
    size_t current_index = 0;
    size_t assignment_count = 0;
    
    for (int d = 0; d < MAX_ALLOCATION_DIMENSIONS; d++) {
        const DimensionAllocation* alloc = get_dimension_allocation(allocation_plan, d);
        
        if (!alloc || !alloc->is_active) {
            continue;
        }
        
        // Calculate work for this dimension based on strategy
        size_t dimension_work = 0;
        
        switch (config->strategy) {
            case WORK_DIST_ENTROPY_ONLY:
                dimension_work = calculate_dimension_work(d, entropies[d], total_entropy, total_work);
                break;
                
            case WORK_DIST_PLIMPTON_ONLY:
                // Equal distribution (Plimpton ratios apply at parent-child level)
                dimension_work = total_work / allocation_plan->active_dimensions;
                break;
                
            case WORK_DIST_COMBINED:
                // Weighted combination
                {
                    size_t entropy_work = calculate_dimension_work(d, entropies[d], total_entropy, total_work);
                    size_t equal_work = total_work / allocation_plan->active_dimensions;
                    dimension_work = (size_t)(config->entropy_weight * entropy_work +
                                             config->plimpton_weight * equal_work);
                }
                break;
                
            case WORK_DIST_ADAPTIVE:
                // Adaptive: use entropy with exponential scaling
                {
                    double normalized = entropies[d] / total_entropy;
                    double scale = math_exp(normalized * 2.0) / math_exp(2.0);
                    dimension_work = (size_t)(scale * total_work);
                }
                break;
        }
        
        // Apply min/max constraints
        if (dimension_work < config->min_work_per_thread) {
            dimension_work = config->min_work_per_thread;
        }
        if (dimension_work > config->max_work_per_thread) {
            dimension_work = config->max_work_per_thread;
        }
        
        // Ensure we don't exceed total work
        if (current_index + dimension_work > total_work) {
            dimension_work = total_work - current_index;
        }
        
        if (dimension_work == 0) {
            continue;
        }
        
        // Create work assignment
        WorkAssignment* assignment = &plan->assignments[assignment_count];
        
        assignment->thread_id = d;  // Use dimension as thread ID for now
        assignment->dimension = d;
        assignment->start_index = current_index;
        assignment->end_index = current_index + dimension_work;
        assignment->work_size = dimension_work;
        assignment->entropy_weight = entropies[d] / total_entropy;
        assignment->plimpton_ratio = 0.0;  // Set later if needed
        assignment->combined_weight = assignment->entropy_weight;
        
        current_index += dimension_work;
        assignment_count++;
        
        if (current_index >= total_work) {
            break;
        }
    }
    
    plan->num_assignments = assignment_count;
    plan->distributed_work = current_index;
    
    return true;
}

/**
 * @brief Assign work to threads based on plan
 */
bool assign_work_to_threads(
    WorkDistributionPlan* plan,
    const uint64_t* thread_ids,
    size_t num_threads
) {
    if (!plan || !thread_ids || num_threads == 0) {
        return false;
    }
    
    // Update thread IDs in assignments
    for (size_t i = 0; i < plan->num_assignments && i < num_threads; i++) {
        plan->assignments[i].thread_id = thread_ids[i];
    }
    
    return true;
}

/**
 * @brief Validate work distribution plan
 */
bool validate_work_distribution_plan(const WorkDistributionPlan* plan) {
    if (!plan) {
        return false;
    }
    
    // Check distributed work doesn't exceed total
    if (plan->distributed_work > plan->total_work_size) {
        return false;
    }
    
    // Check all assignments are valid
    size_t total_assigned = 0;
    
    for (size_t i = 0; i < plan->num_assignments; i++) {
        const WorkAssignment* assignment = &plan->assignments[i];
        
        // Check indices are valid
        if (assignment->start_index >= assignment->end_index) {
            return false;
        }
        
        // Check work size matches indices
        if (assignment->work_size != (assignment->end_index - assignment->start_index)) {
            return false;
        }
        
        total_assigned += assignment->work_size;
    }
    
    // Check total matches
    if (total_assigned != plan->distributed_work) {
        return false;
    }
    
    return true;
}

/**
 * @brief Get work assignment for thread
 */
const WorkAssignment* get_thread_work_assignment(
    const WorkDistributionPlan* plan,
    uint64_t thread_id
) {
    if (!plan) {
        return NULL;
    }
    
    for (size_t i = 0; i < plan->num_assignments; i++) {
        if (plan->assignments[i].thread_id == thread_id) {
            return &plan->assignments[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Calculate work distribution efficiency
 */
double calculate_work_distribution_efficiency(const WorkDistributionPlan* plan) {
    if (!plan || plan->total_work_size == 0) {
        return 0.0;
    }
    
    return (double)plan->distributed_work / (double)plan->total_work_size;
}

/**
 * @brief Calculate work distribution balance
 */
double calculate_work_distribution_balance(const WorkDistributionPlan* plan) {
    if (!plan || plan->num_assignments == 0) {
        return 0.0;
    }
    
    // Calculate mean work size
    double mean = (double)plan->distributed_work / (double)plan->num_assignments;
    
    // Calculate variance
    double variance = 0.0;
    for (size_t i = 0; i < plan->num_assignments; i++) {
        double diff = (double)plan->assignments[i].work_size - mean;
        variance += diff * diff;
    }
    variance /= plan->num_assignments;
    
    // Return coefficient of variation
    double std_dev = math_sqrt(variance);
    return (mean > MATH_EPSILON) ? (std_dev / mean) : 0.0;
}

/**
 * @brief Print work distribution plan
 */
void print_work_distribution_plan(const WorkDistributionPlan* plan) {
    if (!plan) {
        return;
    }
    
    printf("\n=== Work Distribution Plan ===\n");
    printf("Total Work: %zu\n", plan->total_work_size);
    printf("Distributed: %zu (%.1f%%)\n", 
           plan->distributed_work,
           100.0 * plan->distributed_work / plan->total_work_size);
    printf("Assignments: %zu\n", plan->num_assignments);
    printf("Total Entropy: %.6f\n", plan->total_entropy);
    printf("Plimpton Ratios: %s\n", plan->use_plimpton_ratios ? "Yes" : "No");
    printf("12-fold Symmetry: %s\n", plan->enforce_12fold ? "Yes" : "No");
    
    printf("\nWork Assignments:\n");
    printf("Thread | Dim | Start    | End      | Size     | Entropy  | Weight\n");
    printf("-------+-----+----------+----------+----------+----------+--------\n");
    
    for (size_t i = 0; i < plan->num_assignments; i++) {
        const WorkAssignment* a = &plan->assignments[i];
        printf("%6lu | %3u | %8zu | %8zu | %8zu | %8.4f | %6.4f\n",
               a->thread_id,
               a->dimension,
               a->start_index,
               a->end_index,
               a->work_size,
               a->entropy_weight,
               a->combined_weight);
    }
    
    printf("================================\n\n");
}

/**
 * @brief Print work distribution statistics
 */
void print_work_distribution_statistics(const WorkDistributionPlan* plan) {
    if (!plan) {
        return;
    }
    
    double efficiency = calculate_work_distribution_efficiency(plan);
    double balance = calculate_work_distribution_balance(plan);
    
    printf("\n=== Work Distribution Statistics ===\n");
    printf("Efficiency: %.2f%% (%zu / %zu)\n",
           efficiency * 100.0,
           plan->distributed_work,
           plan->total_work_size);
    printf("Balance: %.4f (lower is better)\n", balance);
    printf("Assignments: %zu\n", plan->num_assignments);
    
    // Find min/max work sizes
    size_t min_work = SIZE_MAX;
    size_t max_work = 0;
    
    for (size_t i = 0; i < plan->num_assignments; i++) {
        size_t work = plan->assignments[i].work_size;
        if (work < min_work) min_work = work;
        if (work > max_work) max_work = work;
    }
    
    if (plan->num_assignments > 0) {
        printf("Work Range: %zu - %zu\n", min_work, max_work);
        printf("Mean Work: %.1f per assignment\n",
               (double)plan->distributed_work / plan->num_assignments);
    }
    
    printf("====================================\n\n");
}
