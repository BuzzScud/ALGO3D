/**
 * Plimpton 322 Integration with Training System
 * 
 * Provides integration between Plimpton 322 work distribution and the
 * crystalline CLLM training system.
 */

#ifndef CLLM_PLIMPTON_INTEGRATION_H
#define CLLM_PLIMPTON_INTEGRATION_H

#include "ai/cllm_plimpton_relationships.h"
#include "ai/cllm_work_distribution.h"
#include "ai/cllm_batch_splitting.h"
#include "ai/cllm_cache_aware_distribution.h"
#include "cllm_batch.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Integration statistics
 */
typedef struct {
    size_t total_spawns;              // Total spawn attempts
    size_t successful_spawns;         // Successful spawns
    size_t failed_spawns;             // Failed spawns
    size_t total_batch_splits;        // Total batch splits
    size_t total_samples_distributed; // Total samples distributed
} PlimptonIntegrationStats;

/**
 * Integration context
 */
typedef struct {
    RelationshipRegistry* relationship_registry;  // Parent-child relationships
    PlimptonIntegrationStats stats;               // Integration statistics
} PlimptonIntegrationContext;

/**
 * Initialize Plimpton integration context
 * 
 * @return Integration context, or NULL on failure
 */
PlimptonIntegrationContext* plimpton_integration_create(void);

/**
 * Free Plimpton integration context
 * 
 * @param ctx Integration context
 */
void plimpton_integration_free(PlimptonIntegrationContext* ctx);

/**
 * Validate parent-child spawn using Plimpton constraints
 * 
 * Checks if the parent-child relationship satisfies Plimpton 322 constraints:
 * - gcd(parent_id, child_id) = 1 (coprime)
 * - Not both odd
 * - parent_id > child_id > 0
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_id Child sphere ID
 * @return 1 if valid, 0 if invalid
 */
int plimpton_validate_spawn(PlimptonIntegrationContext* ctx, 
                            uint64_t parent_id, 
                            uint64_t child_id);

/**
 * Generate valid child IDs for a parent using Plimpton constraints
 * 
 * Generates up to max_children valid child IDs that satisfy Plimpton
 * constraints with the parent ID.
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_ids Output array for child IDs
 * @param max_children Maximum number of children (typically 12)
 * @return Number of children generated
 */
size_t plimpton_generate_children(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t* child_ids,
                                  size_t max_children);

/**
 * Calculate work distribution for parent-child pair
 * 
 * Calculates Plimpton 322 work distribution ratios for a parent-child pair.
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_id Child sphere ID
 * @return Work distribution with parent and child ratios
 */
WorkDistribution plimpton_calculate_distribution(PlimptonIntegrationContext* ctx,
                                                 uint64_t parent_id,
                                                 uint64_t child_id);

/**
 * Split batch according to Plimpton work distribution
 * 
 * Splits a training batch between parent and child according to Plimpton
 * 322 work distribution ratios.
 * 
 * @param ctx Integration context
 * @param source_batch Source batch to split
 * @param parent_id Parent sphere ID
 * @param child_id Child sphere ID
 * @return Split result with parent and child batches
 */
SplitBatchResult plimpton_split_batch(PlimptonIntegrationContext* ctx,
                                     CLLMBatch* source_batch,
                                     uint64_t parent_id,
                                     uint64_t child_id);

/**
 * Split batch among parent and multiple children
 * 
 * Splits a training batch among parent and multiple children according to
 * Plimpton 322 work distribution ratios.
 * 
 * @param ctx Integration context
 * @param source_batch Source batch to split
 * @param parent_id Parent sphere ID
 * @param child_ids Array of child sphere IDs
 * @param num_children Number of children
 * @return Multi-child split result
 */
MultiChildSplitResult plimpton_split_batch_multi(PlimptonIntegrationContext* ctx,
                                                 CLLMBatch* source_batch,
                                                 uint64_t parent_id,
                                                 uint64_t* child_ids,
                                                 size_t num_children);

/**
 * Create cache-aware distribution for children
 * 
 * Creates a cache-aware work distribution for children, considering CPU
 * topology, cache sizes, and NUMA nodes.
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_ids Array of child sphere IDs
 * @param num_children Number of children
 * @param work_sizes Array of work sizes for each child
 * @return Cache-aware distribution
 */
CacheAwareDistribution plimpton_create_cache_distribution(PlimptonIntegrationContext* ctx,
                                                         uint64_t parent_id,
                                                         uint64_t* child_ids,
                                                         size_t num_children,
                                                         uint64_t* work_sizes);

/**
 * Record successful spawn
 * 
 * Records a successful spawn in the integration statistics.
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_ids Array of child sphere IDs
 * @param num_children Number of children
 */
void plimpton_record_spawn_success(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t* child_ids,
                                  size_t num_children);

/**
 * Record failed spawn
 * 
 * Records a failed spawn in the integration statistics.
 * 
 * @param ctx Integration context
 * @param parent_id Parent sphere ID
 * @param child_id Child sphere ID
 */
void plimpton_record_spawn_failure(PlimptonIntegrationContext* ctx,
                                  uint64_t parent_id,
                                  uint64_t child_id);

/**
 * Get integration statistics
 * 
 * @param ctx Integration context
 * @return Integration statistics
 */
PlimptonIntegrationStats plimpton_get_stats(PlimptonIntegrationContext* ctx);

/**
 * Print integration statistics
 * 
 * @param ctx Integration context
 */
void plimpton_print_stats(PlimptonIntegrationContext* ctx);

/**
 * Validate integration context
 * 
 * @param ctx Integration context
 * @return 1 if valid, 0 if invalid
 */
int plimpton_validate_context(PlimptonIntegrationContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // CLLM_PLIMPTON_INTEGRATION_H
