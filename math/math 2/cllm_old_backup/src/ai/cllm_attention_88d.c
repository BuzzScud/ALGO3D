/**
 * @file cllm_attention_88d.c
 * @brief Parallel Multi-Head Attention using 88D Threading System
 * 
 * This file implements parallel multi-head attention computation that leverages
 * the 88D threading system for efficient distributed computation.
 * 
 * Key Features:
 * - Parallel Q×K^T computation across threads
 * - Parallel softmax computation
 * - Parallel attention×V computation
 * - Head-wise parallelization
 * - Sequence-wise parallelization
 * - Efficient memory access patterns
 */

#include "ai/cllm_attention_88d.h"
#include "ai/cllm_88d_integration.h"
#include "algorithms/hierarchical_threading.h"
#include "algorithms/work_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// ATTENTION COMPUTATION WORK ITEMS
// ============================================================================

/**
 * Work item for parallel Q×K^T computation
 */
typedef struct {
    const float* query;      // [batch × seq_len × hidden]
    const float* key;        // [batch × seq_len × hidden]
    float* scores;           // [batch × num_heads × seq_len × seq_len]
    uint32_t batch_idx;
    uint32_t head_start;
    uint32_t head_end;
    uint32_t seq_length;
    uint32_t head_dim;
    float scale;
} QKComputeWork;

/**
 * Work item for parallel softmax computation
 */
typedef struct {
    float* scores;           // [batch × num_heads × seq_len × seq_len]
    const float* mask;       // [batch × seq_len × seq_len] (optional)
    uint32_t batch_idx;
    uint32_t head_start;
    uint32_t head_end;
    uint32_t seq_length;
} SoftmaxWork;

/**
 * Work item for parallel attention×V computation
 */
typedef struct {
    const float* attention;  // [batch × num_heads × seq_len × seq_len]
    const float* value;      // [batch × seq_len × hidden]
    float* output;           // [batch × seq_len × hidden]
    uint32_t batch_idx;
    uint32_t head_start;
    uint32_t head_end;
    uint32_t seq_length;
    uint32_t head_dim;
} AttentionVWork;

// ============================================================================
// WORKER FUNCTIONS
// ============================================================================

/**
 * Worker function for Q×K^T computation
 */
static void* qk_compute_worker(void* arg) {
    QKComputeWork* work = (QKComputeWork*)arg;
    
    uint32_t batch = work->batch_idx;
    uint32_t seq_len = work->seq_length;
    uint32_t head_dim = work->head_dim;
    
    // Process assigned heads
    for (uint32_t h = work->head_start; h < work->head_end; h++) {
        // For each query position
        for (uint32_t i = 0; i < seq_len; i++) {
            // For each key position
            for (uint32_t j = 0; j < seq_len; j++) {
                float score = 0.0f;
                
                // Compute dot product Q[i] · K[j]
                for (uint32_t d = 0; d < head_dim; d++) {
                    uint32_t q_idx = batch * seq_len * head_dim * work->head_end + 
                                    h * seq_len * head_dim + 
                                    i * head_dim + d;
                    uint32_t k_idx = batch * seq_len * head_dim * work->head_end + 
                                    h * seq_len * head_dim + 
                                    j * head_dim + d;
                    
                    score += work->query[q_idx] * work->key[k_idx];
                }
                
                // Scale by sqrt(head_dim)
                score *= work->scale;
                
                // Store score
                uint32_t score_idx = batch * work->head_end * seq_len * seq_len + 
                                    h * seq_len * seq_len + 
                                    i * seq_len + j;
                work->scores[score_idx] = score;
            }
        }
    }
    
    return NULL;
}

/**
 * Worker function for softmax computation
 */
static void* softmax_worker(void* arg) {
    SoftmaxWork* work = (SoftmaxWork*)arg;
    
    uint32_t batch = work->batch_idx;
    uint32_t seq_len = work->seq_length;
    
    // Process assigned heads
    for (uint32_t h = work->head_start; h < work->head_end; h++) {
        // For each query position
        for (uint32_t i = 0; i < seq_len; i++) {
            // Find max for numerical stability
            float max_score = -INFINITY;
            for (uint32_t j = 0; j < seq_len; j++) {
                uint32_t idx = batch * work->head_end * seq_len * seq_len + 
                              h * seq_len * seq_len + 
                              i * seq_len + j;
                
                // Apply mask if provided
                if (work->mask) {
                    uint32_t mask_idx = batch * seq_len * seq_len + i * seq_len + j;
                    if (work->mask[mask_idx] == 0.0f) {
                        work->scores[idx] = -INFINITY;
                    }
                }
                
                if (work->scores[idx] > max_score) {
                    max_score = work->scores[idx];
                }
            }
            
            // Compute exp and sum
            float sum = 0.0f;
            for (uint32_t j = 0; j < seq_len; j++) {
                uint32_t idx = batch * work->head_end * seq_len * seq_len + 
                              h * seq_len * seq_len + 
                              i * seq_len + j;
                
                if (work->scores[idx] != -INFINITY) {
                    work->scores[idx] = expf(work->scores[idx] - max_score);
                    sum += work->scores[idx];
                } else {
                    work->scores[idx] = 0.0f;
                }
            }
            
            // Normalize
            if (sum > 0.0f) {
                for (uint32_t j = 0; j < seq_len; j++) {
                    uint32_t idx = batch * work->head_end * seq_len * seq_len + 
                                  h * seq_len * seq_len + 
                                  i * seq_len + j;
                    work->scores[idx] /= sum;
                }
            }
        }
    }
    
    return NULL;
}

/**
 * Worker function for attention×V computation
 */
static void* attention_v_worker(void* arg) {
    AttentionVWork* work = (AttentionVWork*)arg;
    
    uint32_t batch = work->batch_idx;
    uint32_t seq_len = work->seq_length;
    uint32_t head_dim = work->head_dim;
    
    // Process assigned heads
    for (uint32_t h = work->head_start; h < work->head_end; h++) {
        // For each query position
        for (uint32_t i = 0; i < seq_len; i++) {
            // For each dimension
            for (uint32_t d = 0; d < head_dim; d++) {
                float sum = 0.0f;
                
                // Weighted sum over values
                for (uint32_t j = 0; j < seq_len; j++) {
                    uint32_t attn_idx = batch * work->head_end * seq_len * seq_len + 
                                       h * seq_len * seq_len + 
                                       i * seq_len + j;
                    uint32_t v_idx = batch * seq_len * head_dim * work->head_end + 
                                    h * seq_len * head_dim + 
                                    j * head_dim + d;
                    
                    sum += work->attention[attn_idx] * work->value[v_idx];
                }
                
                // Store output
                uint32_t out_idx = batch * seq_len * head_dim * work->head_end + 
                                  h * seq_len * head_dim + 
                                  i * head_dim + d;
                work->output[out_idx] = sum;
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * Parallel multi-head attention computation
 */
int cllm_attention_compute_parallel(
    CLLMModel* model,
    const float* query,
    const float* key,
    const float* value,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t num_heads,
    uint32_t head_dim
) {
    if (!model || !query || !key || !value || !output) {
        return -1;
    }
    
    if (!model->threading_88d.pool) {
        fprintf(stderr, "Error: 88D threading not initialized\n");
        return -1;
    }
    
    HierarchicalThreadPool* pool = model->threading_88d.pool;
    uint32_t num_threads = pool->num_threads;
    
    // Allocate temporary buffers for attention scores
    size_t scores_size = batch_size * num_heads * seq_length * seq_length;
    float* scores = calloc(scores_size, sizeof(float));
    if (!scores) {
        return -1;
    }
    
    float scale = 1.0f / sqrtf((float)head_dim);
    
    // Step 1: Compute Q×K^T in parallel
    uint32_t heads_per_thread = (num_heads + num_threads - 1) / num_threads;
    
    for (uint32_t b = 0; b < batch_size; b++) {
        QKComputeWork* qk_work = calloc(num_threads, sizeof(QKComputeWork));
        if (!qk_work) {
            free(scores);
            return -1;
        }
        
        for (uint32_t t = 0; t < num_threads; t++) {
            qk_work[t].query = query;
            qk_work[t].key = key;
            qk_work[t].scores = scores;
            qk_work[t].batch_idx = b;
            qk_work[t].head_start = t * heads_per_thread;
            qk_work[t].head_end = (t + 1) * heads_per_thread;
            if (qk_work[t].head_end > num_heads) {
                qk_work[t].head_end = num_heads;
            }
            qk_work[t].seq_length = seq_length;
            qk_work[t].head_dim = head_dim;
            qk_work[t].scale = scale;
            
            HierarchicalThread* thread = pool->threads[t];
            if (thread && qk_work[t].head_start < qk_work[t].head_end) {
                hierarchical_thread_submit_work(
                    thread,
                    qk_compute_worker,
                    &qk_work[t],
                    WORK_PRIORITY_HIGH
                );
            }
        }
        
        hierarchical_thread_pool_wait_all(pool);
        free(qk_work);
    }
    
    // Step 2: Apply softmax in parallel
    for (uint32_t b = 0; b < batch_size; b++) {
        SoftmaxWork* softmax_work = calloc(num_threads, sizeof(SoftmaxWork));
        if (!softmax_work) {
            free(scores);
            return -1;
        }
        
        for (uint32_t t = 0; t < num_threads; t++) {
            softmax_work[t].scores = scores;
            softmax_work[t].mask = mask;
            softmax_work[t].batch_idx = b;
            softmax_work[t].head_start = t * heads_per_thread;
            softmax_work[t].head_end = (t + 1) * heads_per_thread;
            if (softmax_work[t].head_end > num_heads) {
                softmax_work[t].head_end = num_heads;
            }
            softmax_work[t].seq_length = seq_length;
            
            HierarchicalThread* thread = pool->threads[t];
            if (thread && softmax_work[t].head_start < softmax_work[t].head_end) {
                hierarchical_thread_submit_work(
                    thread,
                    softmax_worker,
                    &softmax_work[t],
                    WORK_PRIORITY_HIGH
                );
            }
        }
        
        hierarchical_thread_pool_wait_all(pool);
        free(softmax_work);
    }
    
    // Step 3: Compute attention×V in parallel
    for (uint32_t b = 0; b < batch_size; b++) {
        AttentionVWork* av_work = calloc(num_threads, sizeof(AttentionVWork));
        if (!av_work) {
            free(scores);
            return -1;
        }
        
        for (uint32_t t = 0; t < num_threads; t++) {
            av_work[t].attention = scores;
            av_work[t].value = value;
            av_work[t].output = output;
            av_work[t].batch_idx = b;
            av_work[t].head_start = t * heads_per_thread;
            av_work[t].head_end = (t + 1) * heads_per_thread;
            if (av_work[t].head_end > num_heads) {
                av_work[t].head_end = num_heads;
            }
            av_work[t].seq_length = seq_length;
            av_work[t].head_dim = head_dim;
            
            HierarchicalThread* thread = pool->threads[t];
            if (thread && av_work[t].head_start < av_work[t].head_end) {
                hierarchical_thread_submit_work(
                    thread,
                    attention_v_worker,
                    &av_work[t],
                    WORK_PRIORITY_HIGH
                );
            }
        }
        
        hierarchical_thread_pool_wait_all(pool);
        free(av_work);
    }
    
    free(scores);
    return 0;
}

/**
 * Parallel self-attention (Q=K=V)
 */
int cllm_attention_self_attention_parallel(
    CLLMModel* model,
    const float* input,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t num_heads,
    uint32_t head_dim
) {
    // Self-attention: Q = K = V = input
    return cllm_attention_compute_parallel(
        model,
        input,  // query
        input,  // key
        input,  // value
        mask,
        output,
        batch_size,
        seq_length,
        num_heads,
        head_dim
    );
}

/**
 * Parallel cross-attention
 */
int cllm_attention_cross_attention_parallel(
    CLLMModel* model,
    const float* query,
    const float* context,
    const float* mask,
    float* output,
    uint32_t batch_size,
    uint32_t query_length,
    uint32_t context_length,
    uint32_t num_heads,
    uint32_t head_dim
) {
    // Cross-attention: Q from decoder, K=V from encoder
    // Note: This is simplified; actual implementation would handle different lengths
    return cllm_attention_compute_parallel(
        model,
        query,
        context,  // key
        context,  // value
        mask,
        output,
        batch_size,
        query_length,
        num_heads,
        head_dim
    );
}

/**
 * Get attention statistics
 */
int cllm_attention_get_stats(
    CLLMModel* model,
    CLLMAttentionStats* stats
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
    stats->attention_ops_computed = 0; // TODO: Track this
    stats->total_attention_time_ms = 0.0; // TODO: Track this
    stats->flops_per_second = 0.0; // TODO: Calculate this
    
    return 0;
}

/**
 * Print attention statistics
 */
void cllm_attention_print_stats(const CLLMAttentionStats* stats) {
    if (!stats) {
        return;
    }
    
    printf("\n=== Attention Statistics ===\n");
    printf("Threads: %u\n", stats->num_threads);
    printf("Attention ops computed: %lu\n", stats->attention_ops_computed);
    printf("Total attention time: %.2f ms\n", stats->total_attention_time_ms);
    printf("FLOPS: %.2f GFLOPS\n", stats->flops_per_second / 1e9);
    printf("============================\n\n");
}