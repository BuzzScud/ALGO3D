#include "math/types.h"
/**
 * @file cllm_entropy_allocation.c
 * @brief Implementation of entropy-based thread allocation
 */

#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "ai/cllm_entropy_allocation.h"
#include <string.h>
#include <stdio.h>

#ifndef MATH_INFINITY
#endif

/**
 * @brief Small epsilon for floating point comparisons
 */
#define MATH_EPSILON 1e-10

/**
 * @brief Initialize allocation configuration with defaults
 */
void allocation_config_init_default(AllocationConfig* config) {
    if (!config) {
        return;
    }
    
    config->strategy = ALLOCATION_PROPORTIONAL;
    config->entropy_threshold = 0.01;  // Minimum entropy to allocate threads
    config->enforce_12fold = true;     // Enforce 12-fold symmetry by default
    config->min_threads_per_dimension = 12;  // Minimum 12 threads (one group)
    config->max_threads_per_dimension = 144; // Maximum 144 threads (12 groups)
    config->balance_factor = 0.5;      // 50% balance for BALANCED strategy
}

/**
 * @brief Adjust thread count to enforce 12-fold symmetry
 */
int adjust_to_12fold_symmetry(int threads) {
    if (threads <= 0) {
        return 0;
    }
    
    // Round to nearest multiple of 12
    int groups = (threads + 6) / 12;  // Round to nearest
    
    // Ensure at least 1 group if threads > 0
    if (groups == 0 && threads > 0) {
        groups = 1;
    }
    
    return groups * 12;
}

/**
 * @brief Calculate allocation for specific dimension
 */
int calculate_dimension_threads(
    double entropy,
    double total_entropy,
    int available_threads,
    const AllocationConfig* config
) {
    if (!config || total_entropy < MATH_EPSILON || available_threads <= 0) {
        return 0;
    }
    
    // Check entropy threshold
    if (entropy < config->entropy_threshold) {
        return 0;
    }
    
    int threads = 0;
    
    switch (config->strategy) {
        case ALLOCATION_PROPORTIONAL:
            // Proportional to entropy
            threads = (int)((entropy / total_entropy) * available_threads);
            break;
            
        case ALLOCATION_THRESHOLD:
            // Equal allocation for dimensions above threshold
            threads = available_threads / 12;  // Divide equally among 12 dimensions
            break;
            
        case ALLOCATION_BALANCED:
            // Balance between proportional and equal
            {
                int proportional = (int)((entropy / total_entropy) * available_threads);
                int equal = available_threads / 12;
                threads = (int)(config->balance_factor * proportional + 
                               (1.0 - config->balance_factor) * equal);
            }
            break;
            
        case ALLOCATION_ADAPTIVE:
            // Adaptive based on entropy magnitude
            {
                double normalized = entropy / total_entropy;
                // Use exponential scaling for adaptive allocation
                double scale = math_exp(normalized * 2.0) / math_exp(2.0);
                threads = (int)(scale * available_threads);
            }
            break;
    }
    
    // Apply min/max constraints
    if (threads > 0 && threads < config->min_threads_per_dimension) {
        threads = config->min_threads_per_dimension;
    }
    if (threads > config->max_threads_per_dimension) {
        threads = config->max_threads_per_dimension;
    }
    
    return threads;
}

/**
 * @brief Calculate thread allocation based on entropy distribution
 */
bool calculate_thread_allocation(
    EntropyIntegrationContext* ctx,
    int available_threads,
    const AllocationConfig* config,
    ThreadAllocationPlan* plan
) {
    if (!ctx || !config || !plan || available_threads <= 0) {
        return false;
    }
    
    // Initialize plan
    memset(plan, 0, sizeof(ThreadAllocationPlan));
    plan->total_available_threads = available_threads;
    plan->enforce_12fold = config->enforce_12fold;
    
    // Get entropy distribution
    double entropies[MAX_ALLOCATION_DIMENSIONS];
    calculate_entropy_distribution(ctx, 100, entropies);  // Use sequence length 100
    
    // Calculate total entropy
    double total_entropy = 0.0;
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        total_entropy += entropies[i];
    }
    plan->total_entropy = total_entropy;
    
    if (total_entropy < MATH_EPSILON) {
        // No entropy, no allocation
        return true;
    }
    
    // Calculate allocation for each dimension
    int total_allocated = 0;
    int active_count = 0;
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        DimensionAllocation* alloc = &plan->dimensions[i];
        
        alloc->dimension = i;
        alloc->entropy = entropies[i];
        alloc->entropy_fraction = entropies[i] / total_entropy;
        
        // Calculate threads for this dimension
        alloc->allocated_threads = calculate_dimension_threads(
            entropies[i],
            total_entropy,
            available_threads,
            config
        );
        
        // Adjust for 12-fold symmetry if enabled
        if (config->enforce_12fold) {
            alloc->adjusted_threads = adjust_to_12fold_symmetry(alloc->allocated_threads);
        } else {
            alloc->adjusted_threads = alloc->allocated_threads;
        }
        
        // Mark as active if threads allocated
        alloc->is_active = (alloc->adjusted_threads > 0);
        
        if (alloc->is_active) {
            active_count++;
            total_allocated += alloc->adjusted_threads;
        }
    }
    
    plan->total_allocated_threads = total_allocated;
    plan->total_adjusted_threads = total_allocated;
    plan->active_dimensions = active_count;
    
    // If we allocated more than available, scale down iteratively
    while (total_allocated > available_threads && active_count > 0) {
        // Find dimension with most threads and reduce it
        int max_idx = -1;
        int max_threads = 0;
        
        for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
            if (plan->dimensions[i].is_active && 
                plan->dimensions[i].adjusted_threads > max_threads) {
                max_threads = plan->dimensions[i].adjusted_threads;
                max_idx = i;
            }
        }
        
        if (max_idx < 0) break;
        
        // Reduce by one 12-fold group
        int new_threads = plan->dimensions[max_idx].adjusted_threads - 12;
        
        if (new_threads < config->min_threads_per_dimension) {
            // Deactivate this dimension
            total_allocated -= plan->dimensions[max_idx].adjusted_threads;
            plan->dimensions[max_idx].adjusted_threads = 0;
            plan->dimensions[max_idx].is_active = false;
            active_count--;
        } else {
            // Reduce threads
            total_allocated -= 12;
            plan->dimensions[max_idx].adjusted_threads = new_threads;
        }
    }
    
    plan->total_adjusted_threads = total_allocated;
    plan->active_dimensions = active_count;
    
    return true;
}

/**
 * @brief Validate thread allocation plan
 */
bool validate_allocation_plan(const ThreadAllocationPlan* plan) {
    if (!plan) {
        return false;
    }
    
    // Check total allocated doesn't exceed available
    if (plan->total_adjusted_threads > plan->total_available_threads) {
        return false;
    }
    
    // Check active dimensions count
    int active_count = 0;
    int total_threads = 0;
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        const DimensionAllocation* alloc = &plan->dimensions[i];
        
        if (alloc->is_active) {
            active_count++;
            total_threads += alloc->adjusted_threads;
            
            // Check 12-fold symmetry if enforced
            if (plan->enforce_12fold && alloc->adjusted_threads % 12 != 0) {
                return false;
            }
            
            // Check threads are positive
            if (alloc->adjusted_threads <= 0) {
                return false;
            }
        }
    }
    
    // Verify counts match
    if (active_count != (int)plan->active_dimensions) {
        return false;
    }
    
    if (total_threads != plan->total_adjusted_threads) {
        return false;
    }
    
    return true;
}

/**
 * @brief Apply allocation plan to training system
 */
bool apply_allocation_plan(const ThreadAllocationPlan* plan) {
    if (!plan) {
        return false;
    }
    
    // Validate plan first
    if (!validate_allocation_plan(plan)) {
        return false;
    }
    
    // TODO: Integrate with actual training system
    // For now, just log the allocation
    printf("[Allocation] Applying thread allocation plan:\n");
    printf("  Total threads: %d / %d\n", 
           plan->total_adjusted_threads, 
           plan->total_available_threads);
    printf("  Active dimensions: %u\n", plan->active_dimensions);
    
    return true;
}

/**
 * @brief Get allocation for specific dimension
 */
const DimensionAllocation* get_dimension_allocation(
    const ThreadAllocationPlan* plan,
    uint32_t dimension
) {
    if (!plan || dimension >= MAX_ALLOCATION_DIMENSIONS) {
        return NULL;
    }
    
    return &plan->dimensions[dimension];
}

/**
 * @brief Calculate allocation efficiency
 */
double calculate_allocation_efficiency(const ThreadAllocationPlan* plan) {
    if (!plan || plan->total_available_threads == 0) {
        return 0.0;
    }
    
    return (double)plan->total_adjusted_threads / (double)plan->total_available_threads;
}

/**
 * @brief Calculate allocation balance
 */
double calculate_allocation_balance(const ThreadAllocationPlan* plan) {
    if (!plan || plan->active_dimensions == 0) {
        return 0.0;
    }
    
    // Calculate mean threads per active dimension
    double mean = (double)plan->total_adjusted_threads / (double)plan->active_dimensions;
    
    // Calculate variance
    double variance = 0.0;
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        if (plan->dimensions[i].is_active) {
            double diff = plan->dimensions[i].adjusted_threads - mean;
            variance += diff * diff;
        }
    }
    variance /= plan->active_dimensions;
    
    // Return coefficient of variation (normalized standard deviation)
    double std_dev = math_sqrt(variance);
    return (mean > MATH_EPSILON) ? (std_dev / mean) : 0.0;
}

/**
 * @brief Rebalance allocation plan
 */
bool rebalance_allocation(ThreadAllocationPlan* plan, double target_balance) {
    if (!plan || target_balance < 0.0) {
        return false;
    }
    
    // Current balance
    double current_balance = calculate_allocation_balance(plan);
    
    if (current_balance <= target_balance) {
        // Already balanced enough
        return true;
    }
    
    // Simple rebalancing: move threads from over-allocated to under-allocated
    // while respecting entropy proportions
    
    int mean_threads = plan->total_adjusted_threads / plan->active_dimensions;
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        if (plan->dimensions[i].is_active) {
            int current = plan->dimensions[i].adjusted_threads;
            int target = mean_threads;
            
            // Adjust towards mean while respecting entropy
            int adjusted = (current + target) / 2;
            
            if (plan->enforce_12fold) {
                adjusted = adjust_to_12fold_symmetry(adjusted);
            }
            
            plan->dimensions[i].adjusted_threads = adjusted;
        }
    }
    
    // Recalculate totals
    int total = 0;
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        if (plan->dimensions[i].is_active) {
            total += plan->dimensions[i].adjusted_threads;
        }
    }
    plan->total_adjusted_threads = total;
    
    return true;
}

/**
 * @brief Compare two allocation plans
 */
double compare_allocation_plans(
    const ThreadAllocationPlan* plan1,
    const ThreadAllocationPlan* plan2
) {
    if (!plan1 || !plan2) {
        return MATH_INFINITY;
    }
    
    double diff = 0.0;
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        int threads1 = plan1->dimensions[i].adjusted_threads;
        int threads2 = plan2->dimensions[i].adjusted_threads;
        
        int delta = threads1 - threads2;
        diff += delta * delta;
    }
    
    return math_sqrt(diff);
}

/**
 * @brief Print allocation plan
 */
void print_allocation_plan(const ThreadAllocationPlan* plan) {
    if (!plan) {
        return;
    }
    
    printf("\n=== Thread Allocation Plan ===\n");
    printf("Total Available: %d threads\n", plan->total_available_threads);
    printf("Total Allocated: %d threads\n", plan->total_adjusted_threads);
    printf("Active Dimensions: %u / %d\n", plan->active_dimensions, MAX_ALLOCATION_DIMENSIONS);
    printf("Total Entropy: %.6f\n", plan->total_entropy);
    printf("12-fold Symmetry: %s\n", plan->enforce_12fold ? "Enabled" : "Disabled");
    
    printf("\nPer-Dimension Allocation:\n");
    printf("Dim | Entropy  | Fraction | Allocated | Adjusted | Active\n");
    printf("----+----------+----------+-----------+----------+--------\n");
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        const DimensionAllocation* alloc = &plan->dimensions[i];
        printf("%3d | %8.4f | %8.4f | %9d | %8d | %s\n",
               alloc->dimension,
               alloc->entropy,
               alloc->entropy_fraction,
               alloc->allocated_threads,
               alloc->adjusted_threads,
               alloc->is_active ? "Yes" : "No");
    }
    
    printf("==============================\n\n");
}

/**
 * @brief Print allocation statistics
 */
void print_allocation_statistics(const ThreadAllocationPlan* plan) {
    if (!plan) {
        return;
    }
    
    double efficiency = calculate_allocation_efficiency(plan);
    double balance = calculate_allocation_balance(plan);
    
    printf("\n=== Allocation Statistics ===\n");
    printf("Efficiency: %.2f%% (%d / %d threads)\n",
           efficiency * 100.0,
           plan->total_adjusted_threads,
           plan->total_available_threads);
    printf("Balance: %.4f (lower is better)\n", balance);
    printf("Active Dimensions: %u / %d\n", 
           plan->active_dimensions, 
           MAX_ALLOCATION_DIMENSIONS);
    
    // Find min/max allocations
    int min_threads = INT32_MAX;
    int max_threads = 0;
    
    for (int i = 0; i < MAX_ALLOCATION_DIMENSIONS; i++) {
        if (plan->dimensions[i].is_active) {
            int threads = plan->dimensions[i].adjusted_threads;
            if (threads < min_threads) min_threads = threads;
            if (threads > max_threads) max_threads = threads;
        }
    }
    
    if (plan->active_dimensions > 0) {
        printf("Thread Range: %d - %d\n", min_threads, max_threads);
        printf("Mean Threads: %.1f per active dimension\n",
               (double)plan->total_adjusted_threads / plan->active_dimensions);
    }
    
    printf("============================\n\n");
}