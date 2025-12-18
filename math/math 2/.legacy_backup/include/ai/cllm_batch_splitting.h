/**
 * Plimpton 322 Batch Splitting
 * 
 * Implements batch splitting based on Plimpton 322 work distribution ratios.
 * Splits batches between parent and children according to calculated ratios.
 * 
 * Phase 4, Day 14 Implementation
 */

#ifndef CLLM_BATCH_SPLITTING_H
#define CLLM_BATCH_SPLITTING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Include batch and work distribution headers
#include "../cllm_batch.h"
#include "cllm_work_distribution.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// STRUCTURES
// ============================================================================

/**
 * Split Batch Result
 * 
 * Contains the parent and child batches after splitting.
 */
typedef struct {
    CLLMBatch* parent_batch;     // Batch for parent thread
    CLLMBatch* child_batch;      // Batch for child thread
    uint32_t parent_samples;     // Number of samples for parent
    uint32_t child_samples;      // Number of samples for child
    bool is_valid;               // Split was successful
} SplitBatchResult;

/**
 * Multi-Child Split Result
 * 
 * Contains the parent and multiple child batches after splitting.
 */
typedef struct {
    CLLMBatch* parent_batch;     // Batch for parent thread
    CLLMBatch** child_batches;   // Array of batches for children
    uint32_t parent_samples;     // Number of samples for parent
    uint32_t* child_samples;     // Number of samples for each child
    size_t num_children;         // Number of children
    bool is_valid;               // Split was successful
} MultiChildSplitResult;

/**
 * Batch Splitting Statistics
 * 
 * Tracks batch splitting operations and performance.
 */
typedef struct {
    uint64_t total_splits;
    uint64_t successful_splits;
    uint64_t failed_splits;
    uint64_t total_samples_split;
    uint64_t total_parent_samples;
    uint64_t total_child_samples;
    double avg_parent_ratio;
    double avg_child_ratio;
} BatchSplittingStats;

// ============================================================================
// BATCH SPLITTING FUNCTIONS
// ============================================================================

/**
 * Split batch by Plimpton ratios
 * 
 * Splits a batch between parent and child according to work distribution ratios.
 * 
 * @param source_batch Source batch to split
 * @param distribution Work distribution ratios
 * @return Split batch result
 */
SplitBatchResult split_batch_by_ratios(
    const CLLMBatch* source_batch,
    const WorkDistribution* distribution
);

/**
 * Split batch for multiple children
 * 
 * Splits a batch among parent and multiple children.
 * 
 * @param source_batch Source batch to split
 * @param distribution Multi-child work distribution
 * @return Multi-child split result
 */
MultiChildSplitResult split_batch_multi_child(
    const CLLMBatch* source_batch,
    const MultiChildDistribution* distribution
);

/**
 * Split batch by sample count
 * 
 * Splits a batch into two parts with specified sample counts.
 * 
 * @param source_batch Source batch to split
 * @param parent_samples Number of samples for parent
 * @param child_samples Number of samples for child
 * @return Split batch result
 */
SplitBatchResult split_batch_by_count(
    const CLLMBatch* source_batch,
    uint32_t parent_samples,
    uint32_t child_samples
);

// ============================================================================
// RESULT MANAGEMENT
// ============================================================================

/**
 * Free split batch result
 * 
 * Frees all batches in the split result.
 * 
 * @param result Split batch result to free
 */
void split_batch_result_free(SplitBatchResult* result);

/**
 * Free multi-child split result
 * 
 * Frees all batches in the multi-child split result.
 * 
 * @param result Multi-child split result to free
 */
void multi_child_split_result_free(MultiChildSplitResult* result);

/**
 * Validate split batch result
 * 
 * Checks that the split result is valid and consistent.
 * 
 * @param result Split batch result
 * @param source_batch Original source batch
 * @return true if valid, false otherwise
 */
bool validate_split_result(
    const SplitBatchResult* result,
    const CLLMBatch* source_batch
);

/**
 * Validate multi-child split result
 * 
 * Checks that the multi-child split result is valid and consistent.
 * 
 * @param result Multi-child split result
 * @param source_batch Original source batch
 * @return true if valid, false otherwise
 */
bool validate_multi_child_split_result(
    const MultiChildSplitResult* result,
    const CLLMBatch* source_batch
);

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

/**
 * Copy batch samples
 * 
 * Copies a range of samples from source to destination batch.
 * 
 * @param dest Destination batch
 * @param source Source batch
 * @param start_idx Starting sample index
 * @param count Number of samples to copy
 * @return true if successful, false otherwise
 */
bool copy_batch_samples(
    CLLMBatch* dest,
    const CLLMBatch* source,
    uint32_t start_idx,
    uint32_t count
);

/**
 * Calculate split indices
 * 
 * Calculates the split point for dividing a batch.
 * 
 * @param total_samples Total number of samples
 * @param parent_ratio Parent's ratio (0.0 to 1.0)
 * @param parent_samples Output: samples for parent
 * @param child_samples Output: samples for child
 */
void calculate_split_indices(
    uint32_t total_samples,
    double parent_ratio,
    uint32_t* parent_samples,
    uint32_t* child_samples
);

/**
 * Calculate multi-child split indices
 * 
 * Calculates split points for multiple children.
 * 
 * @param total_samples Total number of samples
 * @param distribution Multi-child distribution
 * @param parent_samples Output: samples for parent
 * @param child_samples Output: array of samples for each child
 */
void calculate_multi_child_split_indices(
    uint32_t total_samples,
    const MultiChildDistribution* distribution,
    uint32_t* parent_samples,
    uint32_t* child_samples
);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Initialize batch splitting statistics
 * 
 * @param stats Statistics structure to initialize
 */
void batch_splitting_stats_init(BatchSplittingStats* stats);

/**
 * Update statistics with split result
 * 
 * @param stats Statistics structure
 * @param result Split batch result
 * @param distribution Work distribution used
 */
void batch_splitting_stats_update(
    BatchSplittingStats* stats,
    const SplitBatchResult* result,
    const WorkDistribution* distribution
);

/**
 * Print batch splitting statistics
 * 
 * @param stats Statistics structure
 */
void batch_splitting_stats_print(const BatchSplittingStats* stats);

/**
 * Reset batch splitting statistics
 * 
 * @param stats Statistics structure
 */
void batch_splitting_stats_reset(BatchSplittingStats* stats);

// ============================================================================
// UTILITIES
// ============================================================================

/**
 * Print split batch result
 * 
 * @param result Split batch result
 */
void print_split_batch_result(const SplitBatchResult* result);

/**
 * Print multi-child split result
 * 
 * @param result Multi-child split result
 */
void print_multi_child_split_result(const MultiChildSplitResult* result);

/**
 * Get total samples in split result
 * 
 * @param result Split batch result
 * @return Total number of samples
 */
uint32_t get_split_result_total_samples(const SplitBatchResult* result);

/**
 * Get total samples in multi-child split result
 * 
 * @param result Multi-child split result
 * @return Total number of samples
 */
uint32_t get_multi_child_split_result_total_samples(
    const MultiChildSplitResult* result
);

#ifdef __cplusplus
}
#endif

#endif // CLLM_BATCH_SPLITTING_H