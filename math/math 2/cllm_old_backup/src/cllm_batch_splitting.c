/**
 * Plimpton 322 Batch Splitting Implementation
 * 
 * Phase 4, Day 14
 */

#include "ai/cllm_batch_splitting.h"
#include "cllm_batch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static CLLMBatch* create_empty_batch(uint32_t batch_size, uint32_t seq_len) {
    CLLMBatch* batch = cllm_batch_create(batch_size, seq_len);
    if (!batch) {
        fprintf(stderr, "ERROR: Failed to create batch\n");
        return NULL;
    }
    return batch;
}

// ============================================================================
// BATCH OPERATIONS
// ============================================================================

void calculate_split_indices(
    uint32_t total_samples,
    double parent_ratio,
    uint32_t* parent_samples,
    uint32_t* child_samples
) {
    if (!parent_samples || !child_samples) return;
    
    // Clamp ratio to [0, 1]
    if (parent_ratio < 0.0) parent_ratio = 0.0;
    if (parent_ratio > 1.0) parent_ratio = 1.0;
    
    // Calculate split
    *parent_samples = (uint32_t)(total_samples * parent_ratio);
    *child_samples = total_samples - *parent_samples;
    
    // Ensure at least one sample for each if possible
    if (*parent_samples == 0 && total_samples > 0) {
        *parent_samples = 1;
        *child_samples = total_samples - 1;
    }
    if (*child_samples == 0 && total_samples > 1) {
        *child_samples = 1;
        *parent_samples = total_samples - 1;
    }
}

void calculate_multi_child_split_indices(
    uint32_t total_samples,
    const MultiChildDistribution* distribution,
    uint32_t* parent_samples,
    uint32_t* child_samples
) {
    if (!distribution || !parent_samples || !child_samples) return;
    
    // Calculate parent samples
    *parent_samples = (uint32_t)(total_samples * distribution->parent_keeps);
    
    // Calculate samples for each child
    uint32_t total_child_samples = 0;
    for (size_t i = 0; i < distribution->num_children; i++) {
        child_samples[i] = (uint32_t)(total_samples * distribution->child_ratios[i]);
        total_child_samples += child_samples[i];
    }
    
    // Adjust for rounding errors
    uint32_t total_assigned = *parent_samples + total_child_samples;
    if (total_assigned < total_samples) {
        // Give remainder to parent
        *parent_samples += (total_samples - total_assigned);
    } else if (total_assigned > total_samples) {
        // Take from parent
        uint32_t excess = total_assigned - total_samples;
        if (*parent_samples >= excess) {
            *parent_samples -= excess;
        }
    }
}

bool copy_batch_samples(
    CLLMBatch* dest,
    const CLLMBatch* source,
    uint32_t start_idx,
    uint32_t count
) {
    if (!dest || !source) {
        fprintf(stderr, "ERROR: NULL batch pointer\n");
        return false;
    }
    
    if (start_idx + count > source->batch_size) {
        fprintf(stderr, "ERROR: Invalid sample range\n");
        return false;
    }
    
    if (count > dest->batch_size) {
        fprintf(stderr, "ERROR: Destination batch too small\n");
        return false;
    }
    
    // Copy input_ids
    if (source->input_ids && dest->input_ids) {
        memcpy(
            dest->input_ids,
            source->input_ids + (start_idx * source->seq_len),
            count * source->seq_len * sizeof(uint32_t)
        );
    }
    
    // Copy target_ids
    if (source->target_ids && dest->target_ids) {
        memcpy(
            dest->target_ids,
            source->target_ids + (start_idx * source->seq_len),
            count * source->seq_len * sizeof(uint32_t)
        );
    }
    
    // Copy attention_mask
    if (source->attention_mask && dest->attention_mask) {
        memcpy(
            dest->attention_mask,
            source->attention_mask + (start_idx * source->seq_len),
            count * source->seq_len * sizeof(float)
        );
    }
    
    // Update batch size
    dest->batch_size = count;
    dest->seq_len = source->seq_len;
    
    return true;
}

// ============================================================================
// BATCH SPLITTING FUNCTIONS
// ============================================================================

SplitBatchResult split_batch_by_count(
    const CLLMBatch* source_batch,
    uint32_t parent_samples,
    uint32_t child_samples
) {
    SplitBatchResult result;
    memset(&result, 0, sizeof(SplitBatchResult));
    
    if (!source_batch) {
        fprintf(stderr, "ERROR: NULL source batch\n");
        return result;
    }
    
    if (parent_samples + child_samples != source_batch->batch_size) {
        fprintf(stderr, "ERROR: Sample counts don't match source batch size\n");
        return result;
    }
    
    // Create parent batch
    result.parent_batch = create_empty_batch(parent_samples, source_batch->seq_len);
    if (!result.parent_batch) {
        return result;
    }
    
    // Create child batch
    result.child_batch = create_empty_batch(child_samples, source_batch->seq_len);
    if (!result.child_batch) {
        cllm_batch_free(result.parent_batch);
        memset(&result, 0, sizeof(SplitBatchResult));
        return result;
    }
    
    // Copy samples to parent batch
    if (!copy_batch_samples(result.parent_batch, source_batch, 0, parent_samples)) {
        cllm_batch_free(result.parent_batch);
        cllm_batch_free(result.child_batch);
        memset(&result, 0, sizeof(SplitBatchResult));
        return result;
    }
    
    // Copy samples to child batch
    if (!copy_batch_samples(result.child_batch, source_batch, parent_samples, child_samples)) {
        cllm_batch_free(result.parent_batch);
        cllm_batch_free(result.child_batch);
        memset(&result, 0, sizeof(SplitBatchResult));
        return result;
    }
    
    result.parent_samples = parent_samples;
    result.child_samples = child_samples;
    result.is_valid = true;
    
    return result;
}

SplitBatchResult split_batch_by_ratios(
    const CLLMBatch* source_batch,
    const WorkDistribution* distribution
) {
    SplitBatchResult result;
    memset(&result, 0, sizeof(SplitBatchResult));
    
    if (!source_batch || !distribution) {
        fprintf(stderr, "ERROR: NULL parameter\n");
        return result;
    }
    
    if (!distribution->is_valid) {
        fprintf(stderr, "ERROR: Invalid work distribution\n");
        return result;
    }
    
    // Calculate split indices
    uint32_t parent_samples, child_samples;
    calculate_split_indices(
        source_batch->batch_size,
        distribution->parent_keeps,
        &parent_samples,
        &child_samples
    );
    
    // Split by count
    return split_batch_by_count(source_batch, parent_samples, child_samples);
}

MultiChildSplitResult split_batch_multi_child(
    const CLLMBatch* source_batch,
    const MultiChildDistribution* distribution
) {
    MultiChildSplitResult result;
    memset(&result, 0, sizeof(MultiChildSplitResult));
    
    if (!source_batch || !distribution) {
        fprintf(stderr, "ERROR: NULL parameter\n");
        return result;
    }
    
    if (!distribution->is_valid) {
        fprintf(stderr, "ERROR: Invalid work distribution\n");
        return result;
    }
    
    result.num_children = distribution->num_children;
    
    // Allocate arrays
    result.child_batches = calloc(result.num_children, sizeof(CLLMBatch*));
    result.child_samples = calloc(result.num_children, sizeof(uint32_t));
    
    if (!result.child_batches || !result.child_samples) {
        fprintf(stderr, "ERROR: Failed to allocate arrays\n");
        free(result.child_batches);
        free(result.child_samples);
        memset(&result, 0, sizeof(MultiChildSplitResult));
        return result;
    }
    
    // Calculate split indices
    calculate_multi_child_split_indices(
        source_batch->batch_size,
        distribution,
        &result.parent_samples,
        result.child_samples
    );
    
    // Create parent batch
    result.parent_batch = create_empty_batch(result.parent_samples, source_batch->seq_len);
    if (!result.parent_batch) {
        free(result.child_batches);
        free(result.child_samples);
        memset(&result, 0, sizeof(MultiChildSplitResult));
        return result;
    }
    
    // Copy samples to parent batch
    if (!copy_batch_samples(result.parent_batch, source_batch, 0, result.parent_samples)) {
        cllm_batch_free(result.parent_batch);
        free(result.child_batches);
        free(result.child_samples);
        memset(&result, 0, sizeof(MultiChildSplitResult));
        return result;
    }
    
    // Create and fill child batches
    uint32_t current_idx = result.parent_samples;
    for (size_t i = 0; i < result.num_children; i++) {
        result.child_batches[i] = create_empty_batch(
            result.child_samples[i],
            source_batch->seq_len
        );
        
        if (!result.child_batches[i]) {
            // Cleanup on failure
            cllm_batch_free(result.parent_batch);
            for (size_t j = 0; j < i; j++) {
                cllm_batch_free(result.child_batches[j]);
            }
            free(result.child_batches);
            free(result.child_samples);
            memset(&result, 0, sizeof(MultiChildSplitResult));
            return result;
        }
        
        if (!copy_batch_samples(
            result.child_batches[i],
            source_batch,
            current_idx,
            result.child_samples[i]
        )) {
            // Cleanup on failure
            cllm_batch_free(result.parent_batch);
            for (size_t j = 0; j <= i; j++) {
                cllm_batch_free(result.child_batches[j]);
            }
            free(result.child_batches);
            free(result.child_samples);
            memset(&result, 0, sizeof(MultiChildSplitResult));
            return result;
        }
        
        current_idx += result.child_samples[i];
    }
    
    result.is_valid = true;
    
    return result;
}

// ============================================================================
// RESULT MANAGEMENT
// ============================================================================

void split_batch_result_free(SplitBatchResult* result) {
    if (!result) return;
    
    if (result->parent_batch) {
        cllm_batch_free(result->parent_batch);
    }
    if (result->child_batch) {
        cllm_batch_free(result->child_batch);
    }
    
    memset(result, 0, sizeof(SplitBatchResult));
}

void multi_child_split_result_free(MultiChildSplitResult* result) {
    if (!result) return;
    
    if (result->parent_batch) {
        cllm_batch_free(result->parent_batch);
    }
    
    if (result->child_batches) {
        for (size_t i = 0; i < result->num_children; i++) {
            if (result->child_batches[i]) {
                cllm_batch_free(result->child_batches[i]);
            }
        }
        free(result->child_batches);
    }
    
    free(result->child_samples);
    
    memset(result, 0, sizeof(MultiChildSplitResult));
}

bool validate_split_result(
    const SplitBatchResult* result,
    const CLLMBatch* source_batch
) {
    if (!result || !source_batch) return false;
    
    if (!result->is_valid) return false;
    
    // Check total samples match
    if (result->parent_samples + result->child_samples != source_batch->batch_size) {
        return false;
    }
    
    // Check batches exist
    if (!result->parent_batch || !result->child_batch) {
        return false;
    }
    
    // Check batch sizes match
    if (result->parent_batch->batch_size != result->parent_samples) {
        return false;
    }
    if (result->child_batch->batch_size != result->child_samples) {
        return false;
    }
    
    return true;
}

bool validate_multi_child_split_result(
    const MultiChildSplitResult* result,
    const CLLMBatch* source_batch
) {
    if (!result || !source_batch) return false;
    
    if (!result->is_valid) return false;
    
    // Check total samples match
    uint32_t total = result->parent_samples;
    for (size_t i = 0; i < result->num_children; i++) {
        total += result->child_samples[i];
    }
    
    if (total != source_batch->batch_size) {
        return false;
    }
    
    // Check parent batch exists
    if (!result->parent_batch) {
        return false;
    }
    
    // Check all child batches exist
    if (!result->child_batches) {
        return false;
    }
    
    for (size_t i = 0; i < result->num_children; i++) {
        if (!result->child_batches[i]) {
            return false;
        }
        if (result->child_batches[i]->batch_size != result->child_samples[i]) {
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// STATISTICS
// ============================================================================

void batch_splitting_stats_init(BatchSplittingStats* stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(BatchSplittingStats));
}

void batch_splitting_stats_update(
    BatchSplittingStats* stats,
    const SplitBatchResult* result,
    const WorkDistribution* distribution
) {
    if (!stats || !result) return;
    
    stats->total_splits++;
    
    if (result->is_valid) {
        stats->successful_splits++;
        stats->total_samples_split += result->parent_samples + result->child_samples;
        stats->total_parent_samples += result->parent_samples;
        stats->total_child_samples += result->child_samples;
        
        if (distribution) {
            double n = (double)stats->successful_splits;
            stats->avg_parent_ratio = 
                (stats->avg_parent_ratio * (n - 1.0) + distribution->parent_keeps) / n;
            stats->avg_child_ratio = 
                (stats->avg_child_ratio * (n - 1.0) + distribution->child_gets) / n;
        }
    } else {
        stats->failed_splits++;
    }
}

void batch_splitting_stats_print(const BatchSplittingStats* stats) {
    if (!stats) return;
    
    printf("Batch Splitting Statistics:\n");
    printf("  Total Splits:         %llu\n", (unsigned long long)stats->total_splits);
    printf("  Successful Splits:    %llu\n", (unsigned long long)stats->successful_splits);
    printf("  Failed Splits:        %llu\n", (unsigned long long)stats->failed_splits);
    printf("  Total Samples Split:  %llu\n", (unsigned long long)stats->total_samples_split);
    printf("  Total Parent Samples: %llu\n", (unsigned long long)stats->total_parent_samples);
    printf("  Total Child Samples:  %llu\n", (unsigned long long)stats->total_child_samples);
    
    if (stats->successful_splits > 0) {
        printf("  Avg Parent Ratio:     %.6f\n", stats->avg_parent_ratio);
        printf("  Avg Child Ratio:      %.6f\n", stats->avg_child_ratio);
    }
}

void batch_splitting_stats_reset(BatchSplittingStats* stats) {
    if (!stats) return;
    batch_splitting_stats_init(stats);
}

// ============================================================================
// UTILITIES
// ============================================================================

void print_split_batch_result(const SplitBatchResult* result) {
    if (!result) return;
    
    printf("Split Batch Result:\n");
    printf("  Valid:           %s\n", result->is_valid ? "Yes" : "No");
    printf("  Parent Samples:  %u\n", result->parent_samples);
    printf("  Child Samples:   %u\n", result->child_samples);
    printf("  Total Samples:   %u\n", result->parent_samples + result->child_samples);
}

void print_multi_child_split_result(const MultiChildSplitResult* result) {
    if (!result) return;
    
    printf("Multi-Child Split Batch Result:\n");
    printf("  Valid:           %s\n", result->is_valid ? "Yes" : "No");
    printf("  Parent Samples:  %u\n", result->parent_samples);
    printf("  Num Children:    %zu\n", result->num_children);
    
    for (size_t i = 0; i < result->num_children; i++) {
        printf("  Child %zu Samples: %u\n", i, result->child_samples[i]);
    }
    
    printf("  Total Samples:   %u\n", get_multi_child_split_result_total_samples(result));
}

uint32_t get_split_result_total_samples(const SplitBatchResult* result) {
    if (!result) return 0;
    return result->parent_samples + result->child_samples;
}

uint32_t get_multi_child_split_result_total_samples(
    const MultiChildSplitResult* result
) {
    if (!result) return 0;
    
    uint32_t total = result->parent_samples;
    for (size_t i = 0; i < result->num_children; i++) {
        total += result->child_samples[i];
    }
    
    return total;
}
