/**
 * @file cllm_adaptive_hierarchy.c
 * @brief Implementation of entropy-aware adaptive hierarchy depth management
 */

#include "ai/cllm_adaptive_hierarchy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Migrated to NEW math library - no prime_float_math needed

// Stub implementations for missing functions
// TODO: Move these to a proper location or link with cllm_threads.c

int get_num_cpu_cores(void) {
    return 8; // Default stub value
}

int calculate_max_depth(int available_cores) {
    if (available_cores <= 1) return 1;
    if (available_cores <= 13) return 2;
    if (available_cores <= 157) return 3;
    return 4;
}

int can_spawn_at_depth(int current_depth, int available_cores, int current_thread_count) {
    (void)current_depth;
    (void)available_cores;
    (void)current_thread_count;
    return 0; // Stub: don't allow spawning
}

int get_recommended_children_count(int current_depth, int available_cores, int pending_batches) {
    (void)current_depth;
    (void)available_cores;
    (void)pending_batches;
    return 0; // Stub: no children
}

// Helper function to clamp values
static inline double clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Helper function to clamp integers
static inline int clamp_int(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void adaptive_hierarchy_config_init_default(AdaptiveHierarchyConfig* config) {
    if (!config) return;
    
    config->entropy_weight = 0.4;           // 40% weight on entropy
    config->workload_weight = 0.4;          // 40% weight on workload
    config->resource_weight = 0.2;          // 20% weight on resources
    config->adjustment_threshold = 0.6;     // Adjust if score > 0.6
    config->adjustment_interval = 100;      // Minimum 100 steps between adjustments
    config->enable_aggressive_scaling = false;
    config->min_depth = 1;
    config->max_depth = ADAPTIVE_MAX_DEPTH;
}

bool adaptive_hierarchy_init(
    AdaptiveHierarchyContext* ctx,
    EntropyIntegrationContext* entropy_ctx,
    const AdaptiveHierarchyConfig* config
) {
    if (!ctx || !entropy_ctx) {
        return false;
    }
    
    // Initialize configuration
    if (config) {
        ctx->config = *config;
    } else {
        adaptive_hierarchy_config_init_default(&ctx->config);
    }
    
    // Normalize weights to sum to 1.0
    double weight_sum = ctx->config.entropy_weight + 
                       ctx->config.workload_weight + 
                       ctx->config.resource_weight;
    if (weight_sum > 0.0) {
        ctx->config.entropy_weight /= weight_sum;
        ctx->config.workload_weight /= weight_sum;
        ctx->config.resource_weight /= weight_sum;
    }
    
    // Store entropy context reference
    ctx->entropy_ctx = entropy_ctx;
    
    // Initialize state
    ctx->current_depth = 1;
    ctx->recommended_depth = 1;
    ctx->last_adjustment_step = 0;
    ctx->adjustment_count = 0;
    ctx->last_entropy_score = 0.0;
    ctx->last_workload_score = 0.0;
    ctx->last_resource_score = 0.0;
    
    // Initialize last decision
    memset(&ctx->last_decision, 0, sizeof(DepthAdjustmentDecision));
    ctx->last_decision.current_depth = 1;
    ctx->last_decision.recommended_depth = 1;
    
    return true;
}

void adaptive_hierarchy_destroy(AdaptiveHierarchyContext* ctx) {
    if (!ctx) return;
    
    // Clear context (entropy_ctx is not owned by us)
    memset(ctx, 0, sizeof(AdaptiveHierarchyContext));
}

double calculate_entropy_score(const AdaptiveHierarchyContext* ctx) {
    if (!ctx || !ctx->entropy_ctx) {
        return 0.0;
    }
    
    // Get normalized total entropy [0,1]
    double normalized_entropy = ctx->entropy_ctx->stats.normalized_entropy;
    
    // Calculate entropy variance across dimensions
    double entropy_variance = 0.0;
    uint32_t active_dims = 0;
    
    for (uint32_t i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        const DimensionEntropyStats* dim_stats = &ctx->entropy_ctx->stats.dimensions[i];
        if (dim_stats->sample_count > 0) {
            entropy_variance += dim_stats->entropy_variance;
            active_dims++;
        }
    }
    
    if (active_dims > 0) {
        entropy_variance /= active_dims;
    }
    
    // High entropy or high variance suggests need for deeper hierarchy
    // Score is weighted combination of normalized entropy and variance
    double entropy_component = normalized_entropy * 0.7;
    double variance_component = clamp(entropy_variance, 0.0, 1.0) * 0.3;
    
    double score = entropy_component + variance_component;
    return clamp(score, 0.0, 1.0);
}

double calculate_workload_score(int pending_batches, int current_thread_count) {
    if (pending_batches <= 0 || current_thread_count <= 0) {
        return 0.0;
    }
    
    // Calculate batches per thread
    double batches_per_thread = (double)pending_batches / (double)current_thread_count;
    
    // Ideal is around 10 batches per thread
    // Score increases as we deviate from ideal
    const double ideal_batches = 10.0;
    const double max_batches = 50.0;
    
    double score;
    if (batches_per_thread < ideal_batches) {
        // Low workload - score decreases
        score = batches_per_thread / ideal_batches;
    } else {
        // High workload - score increases
        score = clamp((batches_per_thread - ideal_batches) / (max_batches - ideal_batches), 0.0, 1.0);
        score = 0.5 + (score * 0.5); // Map to [0.5, 1.0]
    }
    
    return clamp(score, 0.0, 1.0);
}

double calculate_resource_score(int available_cores, int current_thread_count) {
    if (available_cores <= 0) {
        return 0.0;
    }
    
    // Calculate thread utilization
    double utilization = (double)current_thread_count / (double)available_cores;
    
    // Calculate remaining capacity
    const int max_threads = 144000;
    double capacity = 1.0 - ((double)current_thread_count / (double)max_threads);
    
    // Score is high when we have low utilization and high capacity
    // This indicates we can spawn more threads
    double utilization_score = clamp(1.0 - utilization, 0.0, 1.0);
    double capacity_score = clamp(capacity, 0.0, 1.0);
    
    double score = (utilization_score * 0.6) + (capacity_score * 0.4);
    return clamp(score, 0.0, 1.0);
}

int calculate_entropy_aware_depth(
    AdaptiveHierarchyContext* ctx,
    int available_cores,
    int current_thread_count,
    int pending_batches
) {
    if (!ctx) {
        return 1;
    }
    
    // Calculate individual scores
    double entropy_score = calculate_entropy_score(ctx);
    double workload_score = calculate_workload_score(pending_batches, current_thread_count);
    double resource_score = calculate_resource_score(available_cores, current_thread_count);
    
    // Store scores for later use
    ctx->last_entropy_score = entropy_score;
    ctx->last_workload_score = workload_score;
    ctx->last_resource_score = resource_score;
    
    // Calculate weighted combined score
    double combined_score = 
        (entropy_score * ctx->config.entropy_weight) +
        (workload_score * ctx->config.workload_weight) +
        (resource_score * ctx->config.resource_weight);
    
    // Map combined score to depth recommendation
    int recommended_depth;
    
    if (combined_score < ENTROPY_THRESHOLD_LOW) {
        // Low score - shallow hierarchy
        recommended_depth = 1;
    } else if (combined_score < ENTROPY_THRESHOLD_MEDIUM) {
        // Medium score - moderate hierarchy
        recommended_depth = 2;
    } else if (combined_score < ENTROPY_THRESHOLD_HIGH) {
        // High score - deeper hierarchy
        recommended_depth = 3;
    } else {
        // Very high score - maximum depth
        recommended_depth = ctx->config.enable_aggressive_scaling ? 5 : 4;
    }
    
    // Apply constraints from existing system
    int max_depth_by_cores = calculate_max_depth(available_cores);
    recommended_depth = clamp_int(recommended_depth, 1, max_depth_by_cores);
    
    // Apply configuration constraints
    recommended_depth = clamp_int(recommended_depth, 
                                  ctx->config.min_depth, 
                                  ctx->config.max_depth);
    
    return recommended_depth;
}

bool evaluate_depth_adjustment(
    AdaptiveHierarchyContext* ctx,
    int available_cores,
    int current_thread_count,
    int pending_batches,
    DepthAdjustmentDecision* decision
) {
    if (!ctx || !decision) {
        return false;
    }
    
    // Calculate recommended depth
    int recommended_depth = calculate_entropy_aware_depth(
        ctx, available_cores, current_thread_count, pending_batches
    );
    
    // Fill in decision structure
    decision->current_depth = ctx->current_depth;
    decision->recommended_depth = recommended_depth;
    decision->depth_change = recommended_depth - ctx->current_depth;
    decision->entropy_score = ctx->last_entropy_score;
    decision->workload_score = ctx->last_workload_score;
    decision->resource_score = ctx->last_resource_score;
    
    // Determine adjustment reason
    if (decision->depth_change == 0) {
        decision->reason = DEPTH_ADJUST_NONE;
        decision->should_adjust = false;
        return false;
    }
    
    // Determine primary reason for adjustment
    if (ctx->last_entropy_score > 0.7) {
        decision->reason = DEPTH_ADJUST_ENTROPY_HIGH;
    } else if (ctx->last_entropy_score < 0.3) {
        decision->reason = DEPTH_ADJUST_ENTROPY_LOW;
    } else if (ctx->last_workload_score > 0.7) {
        decision->reason = DEPTH_ADJUST_WORKLOAD_HIGH;
    } else if (ctx->last_workload_score < 0.3) {
        decision->reason = DEPTH_ADJUST_WORKLOAD_LOW;
    } else if (ctx->last_resource_score < 0.3) {
        decision->reason = DEPTH_ADJUST_RESOURCE_LIMIT;
    } else {
        decision->reason = DEPTH_ADJUST_BALANCED;
    }
    
    // Calculate combined adjustment score
    double adjustment_score = 
        (ctx->last_entropy_score * ctx->config.entropy_weight) +
        (ctx->last_workload_score * ctx->config.workload_weight) +
        (ctx->last_resource_score * ctx->config.resource_weight);
    
    // Check if adjustment should be applied
    decision->should_adjust = (adjustment_score >= ctx->config.adjustment_threshold);
    
    return decision->should_adjust;
}

bool should_apply_depth_adjustment(
    const AdaptiveHierarchyContext* ctx,
    uint64_t current_step,
    double adjustment_score
) {
    if (!ctx) {
        return false;
    }
    
    // Check if enough time has passed since last adjustment
    uint64_t steps_since_adjustment = current_step - ctx->last_adjustment_step;
    if (steps_since_adjustment < ctx->config.adjustment_interval) {
        return false;
    }
    
    // Check if adjustment score meets threshold
    if (adjustment_score < ctx->config.adjustment_threshold) {
        return false;
    }
    
    return true;
}

void apply_depth_adjustment(
    AdaptiveHierarchyContext* ctx,
    const DepthAdjustmentDecision* decision,
    uint64_t current_step
) {
    if (!ctx || !decision) {
        return;
    }
    
    // Update context
    ctx->current_depth = decision->recommended_depth;
    ctx->recommended_depth = decision->recommended_depth;
    ctx->last_adjustment_step = current_step;
    ctx->adjustment_count++;
    ctx->last_decision = *decision;
}

int get_entropy_aware_children_count(
    const AdaptiveHierarchyContext* ctx,
    int current_depth,
    int available_cores,
    int pending_batches
) {
    if (!ctx) {
        // Fall back to existing function
        return get_recommended_children_count(current_depth, available_cores, pending_batches);
    }
    
    // Get base recommendation from existing system
    int base_count = get_recommended_children_count(current_depth, available_cores, pending_batches);
    
    if (base_count == 0) {
        return 0; // Can't spawn at this depth
    }
    
    // Adjust based on entropy
    double entropy_score = ctx->last_entropy_score;
    
    // High entropy suggests more children needed
    if (entropy_score > ENTROPY_THRESHOLD_HIGH) {
        // Increase towards 12
        base_count = (base_count + 12) / 2;
    } else if (entropy_score < ENTROPY_THRESHOLD_LOW) {
        // Decrease towards minimum
        base_count = (base_count + 1) / 2;
    }
    
    // Ensure valid range [1, 12]
    return clamp_int(base_count, 1, 12);
}

bool can_spawn_with_entropy_awareness(
    const AdaptiveHierarchyContext* ctx,
    int current_depth,
    int available_cores,
    int current_thread_count
) {
    // First check with existing system
    if (!can_spawn_at_depth(current_depth, available_cores, current_thread_count)) {
        return false;
    }
    
    if (!ctx) {
        return true; // No entropy context, use existing decision
    }
    
    // Check if current depth is within recommended range
    if (current_depth >= ctx->recommended_depth) {
        // At or above recommended depth
        // Only allow spawning if entropy is very high
        if (ctx->last_entropy_score < ENTROPY_THRESHOLD_HIGH) {
            return false;
        }
    }
    
    return true;
}

void get_depth_adjustment_stats(
    const AdaptiveHierarchyContext* ctx,
    int* current_depth,
    int* recommended_depth,
    uint64_t* adjustment_count
) {
    if (!ctx) {
        if (current_depth) *current_depth = 0;
        if (recommended_depth) *recommended_depth = 0;
        if (adjustment_count) *adjustment_count = 0;
        return;
    }
    
    if (current_depth) *current_depth = ctx->current_depth;
    if (recommended_depth) *recommended_depth = ctx->recommended_depth;
    if (adjustment_count) *adjustment_count = ctx->adjustment_count;
}

void print_depth_adjustment_decision(const DepthAdjustmentDecision* decision) {
    if (!decision) {
        return;
    }
    
    printf("\n=== Depth Adjustment Decision ===\n");
    printf("Current Depth: %d\n", decision->current_depth);
    printf("Recommended Depth: %d\n", decision->recommended_depth);
    printf("Depth Change: %+d\n", decision->depth_change);
    printf("Should Adjust: %s\n", decision->should_adjust ? "YES" : "NO");
    
    printf("\nScores:\n");
    printf("  Entropy:  %.3f\n", decision->entropy_score);
    printf("  Workload: %.3f\n", decision->workload_score);
    printf("  Resource: %.3f\n", decision->resource_score);
    
    printf("\nReason: ");
    switch (decision->reason) {
        case DEPTH_ADJUST_NONE:
            printf("No adjustment needed\n");
            break;
        case DEPTH_ADJUST_ENTROPY_HIGH:
            printf("High entropy detected\n");
            break;
        case DEPTH_ADJUST_ENTROPY_LOW:
            printf("Low entropy detected\n");
            break;
        case DEPTH_ADJUST_WORKLOAD_HIGH:
            printf("High workload detected\n");
            break;
        case DEPTH_ADJUST_WORKLOAD_LOW:
            printf("Low workload detected\n");
            break;
        case DEPTH_ADJUST_RESOURCE_LIMIT:
            printf("Resource constraints\n");
            break;
        case DEPTH_ADJUST_BALANCED:
            printf("Balanced adjustment\n");
            break;
    }
    printf("================================\n\n");
}