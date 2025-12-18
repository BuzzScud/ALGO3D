/**
 * Plimpton 322 Integration with Training System
 * 
 * Integrates Plimpton 322 work distribution with the crystalline CLLM training system.
 * Provides functions to:
 * - Validate parent-child relationships using Plimpton constraints
 * - Calculate work distribution ratios for batch splitting
 * - Split batches according to Plimpton ratios
 * - Track integration statistics
 */

#include "ai/cllm_plimpton_integration.h"
#include "ai/cllm_plimpton_relationships.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_batch_splitting.h"
#include "ai/cllm_cache_aware_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize Plimpton integration context
 */
PlimptonIntegrationContext* plimpton_integration_create(void) {
    PlimptonIntegrationContext* ctx = (PlimptonIntegrationContext*)calloc(1, sizeof(PlimptonIntegrationContext));
    if (!ctx) {
        return NULL;
    }
    
    // Initialize relationship registry
    ctx->relationship_registry = relationship_registry_create(100);
    if (!ctx->relationship_registry) {
        free(ctx);
        return NULL;
    }
    
    // Initialize statistics
    ctx->stats.total_spawns = 0;
    ctx->stats.successful_spawns = 0;
    ctx->stats.failed_spawns = 0;
    ctx->stats.total_batch_splits = 0;
    ctx->stats.total_samples_distributed = 0;
    
    return ctx;
}

/**
 * Free Plimpton integration context
 */
void plimpton_integration_free(PlimptonIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->relationship_registry) {
        relationship_registry_destroy(ctx->relationship_registry);
    }
    
    free(ctx);
}

/**
 * Validate parent-child spawn using Plimpton constraints
 */
int plimpton_validate_spawn(PlimptonIntegrationContext* ctx, 
                            uint64_t parent_id, 
                            uint64_t child_id) {
    if (!ctx) {
        return 0;
    }
    
    // Validate Plimpton relationship
    if (!validate_parent_child_relation(parent_id, child_id)) {
        return 0;
    }
    
    // Register relationship
    if (!relationship_registry_add(ctx->relationship_registry, parent_id, child_id)) {
        return 0;
    }
    
    return 1;
}

/**
 * Generate valid child IDs for a parent using Plimpton constraints
 */
size_t plimpton_generate_children(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t* child_ids,
                                  size_t max_children) {
    if (!ctx || !child_ids || max_children == 0) {
        return 0;
    }
    
    // Generate valid children (up to 12 for 12-fold symmetry)
    size_t count = generate_valid_children(parent_id, child_ids, max_children);
    
    // Register all relationships
    for (size_t i = 0; i < count; i++) {
        relationship_registry_add(ctx->relationship_registry, parent_id, child_ids[i]);
    }
    
    return count;
}

/**
 * Calculate work distribution for parent-child pair
 */
WorkDistribution plimpton_calculate_distribution(PlimptonIntegrationContext* ctx,
                                                 uint64_t parent_id,
                                                 uint64_t child_id) {
    (void)ctx;  // May be used for caching in future
    
    return calculate_work_distribution(parent_id, child_id);
}

/**
 * Split batch according to Plimpton work distribution
 */
SplitBatchResult plimpton_split_batch(PlimptonIntegrationContext* ctx,
                                     CLLMBatch* source_batch,
                                     uint64_t parent_id,
                                     uint64_t child_id) {
    if (!ctx || !source_batch) {
        SplitBatchResult result = {0};
        result.is_valid = false;
        return result;
    }
    
    // Calculate work distribution
    WorkDistribution dist = calculate_work_distribution(parent_id, child_id);
    
    // Split batch
    SplitBatchResult result = split_batch_by_ratios(source_batch, &dist);
    
    // Update statistics
    if (result.is_valid) {
        ctx->stats.total_batch_splits++;
        ctx->stats.total_samples_distributed += source_batch->batch_size;
    }
    
    return result;
}

/**
 * Split batch among parent and multiple children
 */
MultiChildSplitResult plimpton_split_batch_multi(PlimptonIntegrationContext* ctx,
                                                 CLLMBatch* source_batch,
                                                 uint64_t parent_id,
                                                 uint64_t* child_ids,
                                                 size_t num_children) {
    if (!ctx || !source_batch || !child_ids || num_children == 0) {
        MultiChildSplitResult result = {0};
        result.is_valid = false;
        return result;
    }
    
    // Calculate multi-child distribution
    MultiChildDistribution dist = calculate_multi_child_distribution(
        parent_id, child_ids, num_children
    );
    
    // Split batch
    MultiChildSplitResult result = split_batch_multi_child(source_batch, &dist);
    
    // Update statistics
    if (result.is_valid) {
        ctx->stats.total_batch_splits++;
        ctx->stats.total_samples_distributed += source_batch->batch_size;
    }
    
    // Free distribution internal arrays (not the struct itself - it's stack-allocated)
    free(dist.child_ids);
    free(dist.child_ratios);
    
    return result;
}

/**
 * Create cache-aware distribution for children
 */
CacheAwareDistribution plimpton_create_cache_distribution(PlimptonIntegrationContext* ctx,
                                                         uint64_t parent_id,
                                                         uint64_t* child_ids,
                                                         size_t num_children,
                                                         uint64_t* work_sizes) {
    (void)ctx;
    (void)parent_id;
    (void)child_ids;
    
    // Detect CPU topology
    CpuTopology topology = detect_cpu_topology();
    
    // Create cache-aware distribution
    CacheAwareDistribution dist = create_cache_aware_distribution(
        num_children, work_sizes, &topology
    );
    
    // Optimize for cache locality
    optimize_for_cache_locality(&dist, &topology);
    
    // Optimize for NUMA
    optimize_for_numa(&dist, &topology);
    
    return dist;
}

/**
 * Record successful spawn
 */
void plimpton_record_spawn_success(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t* child_ids,
                                  size_t num_children) {
    if (!ctx) {
        return;
    }
    
    ctx->stats.total_spawns++;
    ctx->stats.successful_spawns++;
    
    // Register all relationships
    for (size_t i = 0; i < num_children; i++) {
        relationship_registry_add(ctx->relationship_registry, parent_id, child_ids[i]);
    }
}

/**
 * Record failed spawn
 */
void plimpton_record_spawn_failure(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t child_id) {
    if (!ctx) {
        return;
    }
    
    (void)parent_id;
    (void)child_id;
    
    ctx->stats.total_spawns++;
    ctx->stats.failed_spawns++;
}

/**
 * Get integration statistics
 */
PlimptonIntegrationStats plimpton_get_stats(PlimptonIntegrationContext* ctx) {
    if (!ctx) {
        PlimptonIntegrationStats stats = {0};
        return stats;
    }
    
    return ctx->stats;
}

/**
 * Print integration statistics
 */
void plimpton_print_stats(PlimptonIntegrationContext* ctx) {
    if (!ctx) {
        return;
    }
    
    printf("\n=== Plimpton Integration Statistics ===\n");
    printf("Total Spawns:           %zu\n", ctx->stats.total_spawns);
    printf("Successful Spawns:      %zu\n", ctx->stats.successful_spawns);
    printf("Failed Spawns:          %zu\n", ctx->stats.failed_spawns);
    printf("Total Batch Splits:     %zu\n", ctx->stats.total_batch_splits);
    printf("Total Samples:          %zu\n", ctx->stats.total_samples_distributed);
    
    if (ctx->stats.total_spawns > 0) {
        double success_rate = (double)ctx->stats.successful_spawns / ctx->stats.total_spawns * 100.0;
        printf("Success Rate:           %.2f%%\n", success_rate);
    }
    
    printf("=======================================\n\n");
}

/**
 * Validate integration context
 */
int plimpton_validate_context(PlimptonIntegrationContext* ctx) {
    if (!ctx) {
        return 0;
    }
    
    if (!ctx->relationship_registry) {
        return 0;
    }
    
    return 1;
}