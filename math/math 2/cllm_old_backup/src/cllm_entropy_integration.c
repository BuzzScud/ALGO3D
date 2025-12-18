/**
 * @file cllm_entropy_integration.c
 * @brief Implementation of entropy integration with CLLM training system
 */

#include "ai/cllm_entropy_integration.h"
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"

/**
 * @brief Small epsilon for floating point comparisons
 */
#define MATH_EPSILON 1e-10

/**
 * @brief Initialize dimension statistics
 */
static void init_dimension_stats(DimensionEntropyStats* stats, uint32_t dimension) {
    stats->dimension = dimension;
    stats->current_entropy = 0.0;
    stats->min_entropy = INFINITY;
    stats->max_entropy = -INFINITY;
    stats->avg_entropy = 0.0;
    stats->sample_count = 0;
    stats->entropy_variance = 0.0;
}

/**
 * @brief Update dimension statistics with new entropy value
 */
static void update_dimension_stats(DimensionEntropyStats* stats, double entropy) {
    // Update min/max
    if (entropy < stats->min_entropy) {
        stats->min_entropy = entropy;
    }
    if (entropy > stats->max_entropy) {
        stats->max_entropy = entropy;
    }
    
    // Update running average using Welford's online algorithm
    stats->sample_count++;
    double delta = entropy - stats->avg_entropy;
    stats->avg_entropy += delta / (double)stats->sample_count;
    double delta2 = entropy - stats->avg_entropy;
    stats->entropy_variance += delta * delta2;
    
    stats->current_entropy = entropy;
}

/**
 * @brief Initialize entropy integration context
 */
bool entropy_integration_init(EntropyIntegrationContext* ctx, CLLMModel* model) {
    if (!ctx || !model) {
        return false;
    }
    
    memset(ctx, 0, sizeof(EntropyIntegrationContext));
    
    // Initialize base entropy context
    if (!entropy_context_init(&ctx->entropy_ctx)) {
        return false;
    }
    
    // Initialize dimension statistics
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        init_dimension_stats(&ctx->stats.dimensions[i], i);
    }
    
    ctx->model = model;
    ctx->monitoring_enabled = false;
    ctx->monitoring_interval = 100;  // Default: monitor every 100 steps
    ctx->last_monitoring_step = 0;
    
    ctx->stats.total_entropy = 0.0;
    ctx->stats.normalized_entropy = 0.0;
    ctx->stats.update_count = 0;
    ctx->stats.entropy_trend = 0.0;
    ctx->stats.last_total_entropy = 0.0;
    
    return true;
}

/**
 * @brief Destroy entropy integration context
 */
void entropy_integration_destroy(EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    entropy_context_destroy(&ctx->entropy_ctx);
    memset(ctx, 0, sizeof(EntropyIntegrationContext));
}

/**
 * @brief Calculate entropy for specific dimension
 */
double calculate_dimension_entropy(
    EntropyIntegrationContext* ctx,
    uint32_t dimension,
    uint64_t sequence_length
) {
    if (!ctx || dimension >= MAX_TRACKED_DIMENSIONS) {
        return 0.0;
    }
    
    if (sequence_length == 0) {
        return 0.0;
    }
    
    // Calculate entropy for this dimension
    // Use sequence_length as position n, dimension as d
    double entropy = calculate_lattice_entropy_cached(
        &ctx->entropy_ctx,
        sequence_length,
        dimension + 1  // Convert 0-based to 1-based dimension
    );
    
    return entropy;
}

/**
 * @brief Calculate entropy for model at current state
 */
double calculate_model_entropy(EntropyIntegrationContext* ctx, uint64_t sequence_length) {
    if (!ctx || !ctx->model) {
        return 0.0;
    }
    
    double total_entropy = 0.0;
    
    // Calculate entropy for each dimension
    for (uint32_t d = 0; d < MAX_TRACKED_DIMENSIONS; d++) {
        double dim_entropy = calculate_dimension_entropy(ctx, d, sequence_length);
        total_entropy += dim_entropy;
    }
    
    return total_entropy;
}

/**
 * @brief Update entropy statistics
 */
void update_entropy_statistics(EntropyIntegrationContext* ctx, uint64_t sequence_length) {
    if (!ctx) {
        return;
    }
    
    // Store previous total entropy for trend calculation
    ctx->stats.last_total_entropy = ctx->stats.total_entropy;
    
    // Calculate current entropy for each dimension
    double total_entropy = 0.0;
    for (uint32_t d = 0; d < MAX_TRACKED_DIMENSIONS; d++) {
        double dim_entropy = calculate_dimension_entropy(ctx, d, sequence_length);
        update_dimension_stats(&ctx->stats.dimensions[d], dim_entropy);
        total_entropy += dim_entropy;
    }
    
    // Update model-wide statistics
    ctx->stats.total_entropy = total_entropy;
    ctx->stats.update_count++;
    
    // Calculate normalized entropy (average across dimensions)
    ctx->stats.normalized_entropy = total_entropy / (double)MAX_TRACKED_DIMENSIONS;
    
    // Calculate entropy trend (rate of change)
    if (ctx->stats.update_count > 1) {
        ctx->stats.entropy_trend = ctx->stats.total_entropy - ctx->stats.last_total_entropy;
    }
}

/**
 * @brief Get entropy statistics for dimension
 */
const DimensionEntropyStats* get_dimension_stats(
    const EntropyIntegrationContext* ctx,
    uint32_t dimension
) {
    if (!ctx || dimension >= MAX_TRACKED_DIMENSIONS) {
        return NULL;
    }
    
    return &ctx->stats.dimensions[dimension];
}

/**
 * @brief Get model-wide entropy statistics
 */
const ModelEntropyStats* get_model_entropy_stats(const EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return NULL;
    }
    
    return &ctx->stats;
}

/**
 * @brief Enable entropy monitoring
 */
void enable_entropy_monitoring(EntropyIntegrationContext* ctx, uint64_t interval) {
    if (!ctx) {
        return;
    }
    
    ctx->monitoring_enabled = true;
    ctx->monitoring_interval = interval;
    ctx->last_monitoring_step = 0;
}

/**
 * @brief Disable entropy monitoring
 */
void disable_entropy_monitoring(EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    ctx->monitoring_enabled = false;
}

/**
 * @brief Check if monitoring should occur at current step
 */
bool should_monitor_entropy(EntropyIntegrationContext* ctx, uint64_t current_step) {
    if (!ctx || !ctx->monitoring_enabled) {
        return false;
    }
    
    // Check if enough steps have passed since last monitoring
    if (current_step - ctx->last_monitoring_step >= ctx->monitoring_interval) {
        return true;
    }
    
    return false;
}

/**
 * @brief Perform entropy monitoring at current step
 */
void monitor_entropy_step(
    EntropyIntegrationContext* ctx,
    uint64_t current_step,
    uint64_t sequence_length
) {
    if (!ctx) {
        return;
    }
    
    // Update statistics
    update_entropy_statistics(ctx, sequence_length);
    
    // Update last monitoring step
    ctx->last_monitoring_step = current_step;
    
    // Log entropy information
    printf("[Entropy Monitor] Step %lu: Total=%.6f, Normalized=%.6f, Trend=%.6f\n",
           current_step,
           ctx->stats.total_entropy,
           ctx->stats.normalized_entropy,
           ctx->stats.entropy_trend);
}

/**
 * @brief Calculate entropy trend
 */
double calculate_entropy_trend(const EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return 0.0;
    }
    
    return ctx->stats.entropy_trend;
}

/**
 * @brief Get entropy for lattice point
 * TODO: Reimplement using new token_positions (ClockPosition) structure
 */
double get_lattice_point_entropy(
    EntropyIntegrationContext* ctx,
    uint64_t point_index
) {
    if (!ctx || !ctx->model) {
        return 0.0;
    }
    
    if (point_index >= ctx->model->vocab_size) {  // Use vocab_size instead of num_lattice_points
        return 0.0;
    }
    
    // TODO: Reimplement using token_positions
    // For now, return a simple entropy calculation based on position
    uint32_t dimension = (point_index % 12) + 1;
    
    // Calculate entropy for this point
    double entropy = calculate_lattice_entropy_cached(
        &ctx->entropy_ctx,
        point_index,
        dimension
    );
    
    return entropy;
}

/**
 * @brief Calculate entropy distribution across dimensions
 */
void calculate_entropy_distribution(
    EntropyIntegrationContext* ctx,
    uint64_t sequence_length,
    double* entropies
) {
    if (!ctx || !entropies) {
        return;
    }
    
    // Calculate entropy for each dimension
    for (uint32_t d = 0; d < MAX_TRACKED_DIMENSIONS; d++) {
        entropies[d] = calculate_dimension_entropy(ctx, d, sequence_length);
    }
}

/**
 * @brief Reset entropy statistics
 */
void reset_entropy_statistics(EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    // Reset dimension statistics
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        init_dimension_stats(&ctx->stats.dimensions[i], i);
    }
    
    // Reset model-wide statistics
    ctx->stats.total_entropy = 0.0;
    ctx->stats.normalized_entropy = 0.0;
    ctx->stats.update_count = 0;
    ctx->stats.entropy_trend = 0.0;
    ctx->stats.last_total_entropy = 0.0;
    
    // Clear entropy cache
    entropy_cache_clear(&ctx->entropy_ctx);
}

/**
 * @brief Print entropy statistics
 */
void print_entropy_statistics(const EntropyIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    printf("\n=== Entropy Statistics ===\n");
    printf("Total Entropy: %.6f\n", ctx->stats.total_entropy);
    printf("Normalized Entropy: %.6f\n", ctx->stats.normalized_entropy);
    printf("Entropy Trend: %.6f\n", ctx->stats.entropy_trend);
    printf("Update Count: %lu\n", ctx->stats.update_count);
    
    printf("\nPer-Dimension Statistics:\n");
    printf("Dim | Current  | Min      | Max      | Avg      | Samples\n");
    printf("----+----------+----------+----------+----------+---------\n");
    
    for (int i = 0; i < MAX_TRACKED_DIMENSIONS; i++) {
        const DimensionEntropyStats* stats = &ctx->stats.dimensions[i];
        printf("%3d | %8.4f | %8.4f | %8.4f | %8.4f | %7lu\n",
               stats->dimension,
               stats->current_entropy,
               stats->min_entropy == INFINITY ? 0.0 : stats->min_entropy,
               stats->max_entropy == -INFINITY ? 0.0 : stats->max_entropy,
               stats->avg_entropy,
               stats->sample_count);
    }
    
    // Print cache statistics
    double hit_rate;
    uint64_t total_calcs;
    entropy_get_cache_stats(&ctx->entropy_ctx, &hit_rate, &total_calcs);
    
    printf("\nCache Statistics:\n");
    printf("Hit Rate: %.2f%%\n", hit_rate * 100.0);
    printf("Total Calculations: %lu\n", total_calcs);
    printf("========================\n\n");
}
