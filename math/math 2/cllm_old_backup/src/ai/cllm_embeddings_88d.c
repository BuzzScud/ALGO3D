/**
 * @file cllm_embeddings_88d.c
 * @brief Parallel Embeddings using 88D Threading System
 * 
 * This file implements parallel embedding lookup and computation that
 * leverages the 88D threading system for efficient distributed processing.
 * 
 * Key Features:
 * - Parallel embedding lookup across threads
 * - Batch embedding processing
 * - Position encoding computation
 * - Efficient memory access patterns
 */

#include "ai/cllm_embeddings_88d.h"
#include "ai/cllm_88d_integration.h"
#include "algorithms/hierarchical_threading.h"
#include "algorithms/work_distribution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// EMBEDDING WORK ITEMS
// ============================================================================

/**
 * Work item for parallel embedding lookup
 */
typedef struct {
    const int32_t* token_ids;    // [batch × seq_len]
    const float* embedding_table; // [vocab_size × embed_dim]
    float* output;                // [batch × seq_len × embed_dim]
    uint32_t batch_start;
    uint32_t batch_end;
    uint32_t seq_length;
    uint32_t embed_dim;
    uint32_t vocab_size;
} EmbeddingLookupWork;

/**
 * Work item for parallel position encoding
 */
typedef struct {
    float* embeddings;            // [batch × seq_len × embed_dim]
    uint32_t batch_start;
    uint32_t batch_end;
    uint32_t seq_length;
    uint32_t embed_dim;
    uint32_t max_position;
} PositionEncodingWork;

// ============================================================================
// WORKER FUNCTIONS
// ============================================================================

/**
 * Worker function for embedding lookup
 */
static void* embedding_lookup_worker(void* arg) {
    EmbeddingLookupWork* work = (EmbeddingLookupWork*)arg;
    
    // Process assigned batch range
    for (uint32_t b = work->batch_start; b < work->batch_end; b++) {
        for (uint32_t s = 0; s < work->seq_length; s++) {
            // Get token ID
            uint32_t token_idx = b * work->seq_length + s;
            int32_t token_id = work->token_ids[token_idx];
            
            // Bounds check
            if (token_id < 0 || (uint32_t)token_id >= work->vocab_size) {
                // Invalid token ID, zero out embedding
                uint32_t out_idx = b * work->seq_length * work->embed_dim + 
                                  s * work->embed_dim;
                memset(&work->output[out_idx], 0, work->embed_dim * sizeof(float));
                continue;
            }
            
            // Lookup embedding
            uint32_t embed_idx = token_id * work->embed_dim;
            uint32_t out_idx = b * work->seq_length * work->embed_dim + 
                              s * work->embed_dim;
            
            // Copy embedding
            memcpy(&work->output[out_idx], 
                   &work->embedding_table[embed_idx], 
                   work->embed_dim * sizeof(float));
        }
    }
    
    return NULL;
}

/**
 * Worker function for position encoding
 */
static void* position_encoding_worker(void* arg) {
    PositionEncodingWork* work = (PositionEncodingWork*)arg;
    
    // Process assigned batch range
    for (uint32_t b = work->batch_start; b < work->batch_end; b++) {
        for (uint32_t s = 0; s < work->seq_length; s++) {
            uint32_t pos = s;
            
            // Add sinusoidal position encoding
            for (uint32_t d = 0; d < work->embed_dim; d++) {
                uint32_t idx = b * work->seq_length * work->embed_dim + 
                              s * work->embed_dim + d;
                
                // Compute position encoding
                float angle = (float)pos / powf(10000.0f, (2.0f * (d / 2)) / (float)work->embed_dim);
                
                if (d % 2 == 0) {
                    work->embeddings[idx] += sinf(angle);
                } else {
                    work->embeddings[idx] += cosf(angle);
                }
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// PUBLIC API
// ============================================================================

/**
 * Parallel embedding lookup
 */
int cllm_embeddings_lookup_parallel(
    CLLMModel* model,
    const int32_t* token_ids,
    const float* embedding_table,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t vocab_size,
    uint32_t embed_dim
) {
    if (!model || !token_ids || !embedding_table || !output) {
        return -1;
    }
    
    if (!model->threading_88d.pool) {
        fprintf(stderr, "Error: 88D threading not initialized\n");
        return -1;
    }
    
    HierarchicalThreadPool* pool = model->threading_88d.pool;
    uint32_t num_threads = pool->num_threads;
    
    // Calculate work distribution
    uint32_t batches_per_thread = (batch_size + num_threads - 1) / num_threads;
    
    // Create work items
    EmbeddingLookupWork* work_items = calloc(num_threads, sizeof(EmbeddingLookupWork));
    if (!work_items) {
        return -1;
    }
    
    // Distribute work across threads
    for (uint32_t t = 0; t < num_threads; t++) {
        work_items[t].token_ids = token_ids;
        work_items[t].embedding_table = embedding_table;
        work_items[t].output = output;
        work_items[t].batch_start = t * batches_per_thread;
        work_items[t].batch_end = (t + 1) * batches_per_thread;
        if (work_items[t].batch_end > batch_size) {
            work_items[t].batch_end = batch_size;
        }
        work_items[t].seq_length = seq_length;
        work_items[t].embed_dim = embed_dim;
        work_items[t].vocab_size = vocab_size;
        
        HierarchicalThread* thread = pool->threads[t];
        if (thread && work_items[t].batch_start < work_items[t].batch_end) {
            hierarchical_thread_submit_work(
                thread,
                embedding_lookup_worker,
                &work_items[t],
                WORK_PRIORITY_HIGH
            );
        }
    }
    
    // Wait for completion
    hierarchical_thread_pool_wait_all(pool);
    
    free(work_items);
    return 0;
}

/**
 * Parallel position encoding
 */
int cllm_embeddings_add_position_encoding_parallel(
    CLLMModel* model,
    float* embeddings,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t embed_dim,
    uint32_t max_position
) {
    if (!model || !embeddings) {
        return -1;
    }
    
    if (!model->threading_88d.pool) {
        fprintf(stderr, "Error: 88D threading not initialized\n");
        return -1;
    }
    
    HierarchicalThreadPool* pool = model->threading_88d.pool;
    uint32_t num_threads = pool->num_threads;
    
    // Calculate work distribution
    uint32_t batches_per_thread = (batch_size + num_threads - 1) / num_threads;
    
    // Create work items
    PositionEncodingWork* work_items = calloc(num_threads, sizeof(PositionEncodingWork));
    if (!work_items) {
        return -1;
    }
    
    // Distribute work across threads
    for (uint32_t t = 0; t < num_threads; t++) {
        work_items[t].embeddings = embeddings;
        work_items[t].batch_start = t * batches_per_thread;
        work_items[t].batch_end = (t + 1) * batches_per_thread;
        if (work_items[t].batch_end > batch_size) {
            work_items[t].batch_end = batch_size;
        }
        work_items[t].seq_length = seq_length;
        work_items[t].embed_dim = embed_dim;
        work_items[t].max_position = max_position;
        
        HierarchicalThread* thread = pool->threads[t];
        if (thread && work_items[t].batch_start < work_items[t].batch_end) {
            hierarchical_thread_submit_work(
                thread,
                position_encoding_worker,
                &work_items[t],
                WORK_PRIORITY_HIGH
            );
        }
    }
    
    // Wait for completion
    hierarchical_thread_pool_wait_all(pool);
    
    free(work_items);
    return 0;
}

/**
 * Parallel token embedding with position encoding
 */
int cllm_embeddings_embed_tokens_parallel(
    CLLMModel* model,
    const int32_t* token_ids,
    const float* embedding_table,
    float* output,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t vocab_size,
    uint32_t embed_dim,
    bool add_position_encoding
) {
    // First, lookup embeddings
    int result = cllm_embeddings_lookup_parallel(
        model,
        token_ids,
        embedding_table,
        output,
        batch_size,
        seq_length,
        vocab_size,
        embed_dim
    );
    
    if (result != 0) {
        return result;
    }
    
    // Optionally add position encoding
    if (add_position_encoding) {
        result = cllm_embeddings_add_position_encoding_parallel(
            model,
            output,
            batch_size,
            seq_length,
            embed_dim,
            seq_length // max_position = seq_length
        );
    }
    
    return result;
}

/**
 * Get embedding statistics
 */
int cllm_embeddings_get_stats(
    CLLMModel* model,
    CLLMEmbeddingStats* stats
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
    stats->embeddings_computed = 0; // TODO: Track this
    stats->total_embedding_time_ms = 0.0; // TODO: Track this
    stats->lookups_per_second = 0.0; // TODO: Calculate this
    
    return 0;
}

/**
 * Print embedding statistics
 */
void cllm_embeddings_print_stats(const CLLMEmbeddingStats* stats) {
    if (!stats) {
        return;
    }
    
    printf("\n=== Embedding Statistics ===\n");
    printf("Threads: %u\n", stats->num_threads);
    printf("Embeddings computed: %lu\n", stats->embeddings_computed);
    printf("Total embedding time: %.2f ms\n", stats->total_embedding_time_ms);
    printf("Lookups per second: %.2f M/s\n", stats->lookups_per_second / 1e6);
    printf("============================\n\n");
}