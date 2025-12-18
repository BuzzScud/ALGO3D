/**
 * @file cllm_layernorm_88d.c
 * @brief Parallel Layer Normalization using 88D Threading System
 * 
 * This file implements parallel layer normalization computation that
 * leverages the 88D threading system for efficient distributed processing.
 * 
 * Key Features:
 * - Parallel mean computation across threads
 * - Parallel variance computation across threads
 * - Parallel normalization across threads
 * - Efficient reduction operations
 */

#include "ai/cllm_layernorm_88d.h"
#include "ai/cllm_88d_integration.h"
#include "algorithms/hierarchical_threading.h"
#include "algorithms/work_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// LAYER NORM WORK ITEMS
// ============================================================================

/**
 * Work item for parallel mean computation
 */
typedef struct {
    const float* input;          // [batch × seq_len × hidden_dim]
    float* means;                // [batch × seq_len]
    uint32_t batch_start;
    uint32_t batch_end;
    uint32_t seq_length;
    uint32_t hidden_dim;
} MeanComputeWork;

/**
 * Work item for parallel variance computation
 */
typedef struct {
    const float* input;          // [batch × seq_len × hidden_dim]
    const float* means;          // [batch × seq_len]
    float* variances;            // [batch × seq_len]
    uint32_t batch_start;
    uint32_t batch_end;
    uint32_t seq_length;
    uint32_t hidden_dim;
} VarianceComputeWork;

/**
 * Work item for parallel normalization
 */
typedef struct {
    const float* input;          // [batch × seq_len × hidden_dim]
    const float* means;          // [batch × seq_len]
    const float* variances;      // [batch × seq_len]
    const float* gamma;          // [hidden_dim] (optional)
    const float* beta;           // [hidden_dim] (optional)
    float* output;               // [batch × seq_len × hidden_dim]
    uint32_t batch_start;
    uint32_t batch_end;
    uint32_t seq_length;
    uint32_t hidden_dim;
    float epsilon;
} NormalizeWork;

// ============================================================================
// WORKER FUNCTIONS
// ============================================================================

/**
 * Worker function for mean computation
 */
static void* mean_compute_worker(void* arg) {
    MeanComputeWork* work = (MeanComputeWork*)arg;
    
    // Process assigned batch range
    for (uint32_t b = work->batch_start; b < work->batch_end; b++) {
        for (uint32_t s = 0; s < work->seq_length; s++) {
            float sum = 0.0f;
            
            // Compute sum
            for (uint32_t d = 0; d < work->hidden_dim; d++) {
                uint32_t idx = b * work->seq_length * work->hidden_dim + 
                              s * work->hidden_dim + d;
                sum += work->input[idx];
            }
            
            // Compute mean
            uint32_t mean_idx = b * work->seq_length + s;
            work->means[mean_idx] = sum / (float)work->hidden_dim;
        }
    }
    
    return NULL;
}

/**
 * Worker function for variance computation
 */
static void* variance_compute_worker(void* arg) {
    VarianceComputeWork* work = (VarianceComputeWork*)arg;
    
    // Process assigned batch range
    for (uint32_t b = work->batch_start; b < work->batch_end; b++) {
        for (uint32_t s = 0; s < work->seq_length; s++) {
            uint32_t mean_idx = b * work->seq_length + s;
            float mean = work->means[mean_idx];
            float sum_sq = 0.0f;
            
            // Compute sum of squared differences
            for (uint32_t d = 0; d < work->hidden_dim; d++) {
                uint32_t idx = b * work->seq_length * work->hidden_dim + 
                              s * work->hidden_dim + d;
                float diff = work->input[idx] - mean;
                sum_sq += diff * diff;
            }
            
            // Compute variance
            work->variances[mean_idx] = sum_sq / (float)work->hidden_dim;
        }
    }
    
    return NULL;
}

/**
 * Worker function for normalization
 */
static void* normalize_worker(void* arg) {
    NormalizeWork* work = (NormalizeWork*)arg;
    
    // Process assigned batch range
    for (uint32_t b = work->batch_start; b < work->batch_end; b++) {
        for (uint32_t s = 0; s < work->seq_length; s++) {
            uint32_t stat_idx = b * work->seq_length + s;
            float mean = work->means[stat_idx];
            float variance = work->variances[stat_idx];
            float std_dev = sqrtf(variance + work->epsilon);
            
            // Normalize
            for (uint32_t d = 0; d < work->hidden_dim; d++) {
                uint32_t idx = b * work->seq_length * work->hidden_dim + 
                              s * work->hidden_dim + d;
                
                // Normalize: (x - mean) / std_dev
                float normalized = (work->input[idx] - mean) / std_dev;
                
                // Apply affine transformation if gamma and beta provided
                if (work->gamma && work->beta) {
                    normalized = normalized * work->gamma[d] + work->beta[d];
                }
                
                work->output[idx] = normalized;
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * Parallel layer normalization
 */
int cllm_layernorm_forward_parallel(
    CLLMModel* model,
    const float* input,
    const float* gamma,
    const float* beta,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t hidden_dim,
    float epsilon
) {
    if (!model || !input || !output) {
        return -1;
    }
    
    if (!model->threading_88d.pool) {
        fprintf(stderr, "Error: 88D threading not initialized\n");
        return -1;
    }
    
    HierarchicalThreadPool* pool = model->threading_88d.pool;
    uint32_t num_threads = pool->num_threads;
    
    // Allocate temporary buffers for statistics
    size_t stats_size = batch_size * seq_length;
    float* means = calloc(stats_size, sizeof(float));
    float* variances = calloc(stats_size, sizeof(float));
    
    if (!means || !variances) {
        free(means);
        free(variances);
        return -1;
    }
    
    // Calculate work distribution
    uint32_t batches_per_thread = (batch_size + num_threads - 1) / num_threads;
    
    // Step 1: Compute means in parallel
    MeanComputeWork* mean_work = calloc(num_threads, sizeof(MeanComputeWork));
    if (!mean_work) {
        free(means);
        free(variances);
        return -1;
    }
    
    for (uint32_t t = 0; t < num_threads; t++) {
        mean_work[t].input = input;
        mean_work[t].means = means;
        mean_work[t].batch_start = t * batches_per_thread;
        mean_work[t].batch_end = (t + 1) * batches_per_thread;
        if (mean_work[t].batch_end > batch_size) {
            mean_work[t].batch_end = batch_size;
        }
        mean_work[t].seq_length = seq_length;
        mean_work[t].hidden_dim = hidden_dim;
        
        HierarchicalThread* thread = pool->threads[t];
        if (thread && mean_work[t].batch_start < mean_work[t].batch_end) {
            hierarchical_thread_submit_work(
                thread,
                mean_compute_worker,
                &mean_work[t],
                WORK_PRIORITY_HIGH
            );
        }
    }
    
    hierarchical_thread_pool_wait_all(pool);
    free(mean_work);
    
    // Step 2: Compute variances in parallel
    VarianceComputeWork* var_work = calloc(num_threads, sizeof(VarianceComputeWork));
    if (!var_work) {
        free(means);
        free(variances);
        return -1;
    }
    
    for (uint32_t t = 0; t < num_threads; t++) {
        var_work[t].input = input;
        var_work[t].means = means;
        var_work[t].variances = variances;
        var_work[t].batch_start = t * batches_per_thread;
        var_work[t].batch_end = (t + 1) * batches_per_thread;
        if (var_work[t].batch_end > batch_size) {
            var_work[t].batch_end = batch_size;
        }
        var_work[t].seq_length = seq_length;
        var_work[t].hidden_dim = hidden_dim;
        
        HierarchicalThread* thread = pool->threads[t];
        if (thread && var_work[t].batch_start < var_work[t].batch_end) {
            hierarchical_thread_submit_work(
                thread,
                variance_compute_worker,
                &var_work[t],
                WORK_PRIORITY_HIGH
            );
        }
    }
    
    hierarchical_thread_pool_wait_all(pool);
    free(var_work);
    
    // Step 3: Normalize in parallel
    NormalizeWork* norm_work = calloc(num_threads, sizeof(NormalizeWork));
    if (!norm_work) {
        free(means);
        free(variances);
        return -1;
    }
    
    for (uint32_t t = 0; t < num_threads; t++) {
        norm_work[t].input = input;
        norm_work[t].means = means;
        norm_work[t].variances = variances;
        norm_work[t].gamma = gamma;
        norm_work[t].beta = beta;
        norm_work[t].output = output;
        norm_work[t].batch_start = t * batches_per_thread;
        norm_work[t].batch_end = (t + 1) * batches_per_thread;
        if (norm_work[t].batch_end > batch_size) {
            norm_work[t].batch_end = batch_size;
        }
        norm_work[t].seq_length = seq_length;
        norm_work[t].hidden_dim = hidden_dim;
        norm_work[t].epsilon = epsilon;
        
        HierarchicalThread* thread = pool->threads[t];
        if (thread && norm_work[t].batch_start < norm_work[t].batch_end) {
            hierarchical_thread_submit_work(
                thread,
                normalize_worker,
                &norm_work[t],
                WORK_PRIORITY_HIGH
            );
        }
    }
    
    hierarchical_thread_pool_wait_all(pool);
    free(norm_work);
    
    free(means);
    free(variances);
    return 0;
}

/**
 * Parallel RMS normalization
 */
int cllm_layernorm_rms_forward_parallel(
    CLLMModel* model,
    const float* input,
    const float* gamma,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t hidden_dim,
    float epsilon
) {
    if (!model || !input || !output) {
        return -1;
    }
    
    // RMS norm: x / sqrt(mean(x^2) + epsilon) * gamma
    // This is a simplified version - full implementation would be similar to layer norm
    
    // For now, fall back to regular layer norm
    return cllm_layernorm_forward_parallel(
        model,
        input,
        gamma,
        NULL,  // No beta for RMS norm
        output,
        batch_size,
        seq_length,
        hidden_dim,
        epsilon
    );
}

/**
 * Get layer norm statistics
 */
int cllm_layernorm_get_stats(
    CLLMModel* model,
    CLLMLayerNormStats* stats
) {
    if (!model || !stats) {
        return -1;
    }
    
    if (!model->threading_88d.pool) {
        fprintf(stderr, "Error: 88D threading not initialized\n");
        return -1;
    }
    
    HierarchicalThreadPool* pool = model->threading_88d.pool;
    
    stats->num_threads = pool->num_threads;
    stats->normalizations_computed = 0; // TODO: Track this
    stats->total_layernorm_time_ms = 0.0; // TODO: Track this
    stats->normalizations_per_second = 0.0; // TODO: Calculate this
    
    return 0;
}

/**
 * Print layer norm statistics
 */
void cllm_layernorm_print_stats(const CLLMLayerNormStats* stats) {
    if (!stats) {
        return;
    }
    
    printf("\n=== Layer Norm Statistics ===\n");
    printf("Threads: %u\n", stats->num_threads);
    printf("Normalizations computed: %lu\n", stats->normalizations_computed);
    printf("Total layer norm time: %.2f ms\n", stats->total_layernorm_time_ms);
    printf("Normalizations per second: %.2f M/s\n", stats->normalizations_per_second / 1e6);
    printf("==============================\n\n");
}