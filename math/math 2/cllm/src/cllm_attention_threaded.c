/**
 * @file cllm_attention_threaded.c
 * Note: Function names no longer have _threaded suffix (Phase 3)
 * @brief Thread-centric attention mechanism (88D architecture)
 * 
 * This file implements attention using the 88D thread-centric architecture.
 * All parameters are stored in thread-local CrystallineAbacus storage.
 * 
 * ARCHITECTURE:
 * - Each layer has 12 threads (1 control + 11 workers)
 * - Control thread (dimension 0): Coordinates attention computation
 * - Worker threads (dimensions 1-11): Own Q/K/V projection weights
 * - Tokens are permanently assigned to threads
 * - All computation happens in parallel across threads
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "ai/cllm.h"
#include "ai/cllm_attention.h"
#include "ai/cllm_attention_helpers.h"
#include "ai/cllm_embedding_helpers.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"

// ============================================================================
// PURE CRYSTALLINE MATH FUNCTIONS (NO math.h!)
// ============================================================================

/**
 * Pure crystalline sqrt using Newton's method (NO math.h)
 */
static double crystalline_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    
    // Newton's method: x_{n+1} = (x_n + a/x_n) / 2
    double guess = x;
    for (int i = 0; i < 10; i++) {
        guess = (guess + x / guess) * 0.5;
    }
    return guess;
}

// ============================================================================
// THREAD-CENTRIC ATTENTION FORWARD PASS
// ============================================================================

/**
 * Thread-centric attention forward pass
 * 
 * This is a SIMPLIFIED version for Week 2 Day 8.
 * It demonstrates the thread-centric architecture without full parallelization.
 * 
 * ARCHITECTURE:
 * 1. Each token is processed by its assigned thread
 * 2. Thread extracts embedding from CrystallineAbacus
 * 3. Thread computes Q/K/V using thread-local weights
 * 4. Control thread aggregates and computes attention
 * 5. Results distributed back to worker threads
 * 
 * @param model CLLM model with 88D threading
 * @param layer_idx Layer index
 * @param input Input embeddings [batch_size × seq_len × embedding_dim]
 * @param output Output embeddings [batch_size × seq_len × embedding_dim]
 * @param batch_size Batch size
 * @param seq_len Sequence length
 */
void cllm_attention_forward(
    CLLMModel* model,
    uint32_t layer_idx,
    const double* input,
    double* output,
    uint32_t batch_size,
    uint32_t seq_len
) {
    if (!model || !input || !output || layer_idx >= model->num_layers) {
        fprintf(stderr, "Error: Invalid attention parameters\n");
        return;
    }
    
    // Verify 88D threading is initialized
    if (!model->threads) {
        fprintf(stderr, "Error: 88D thread pool not initialized\n");
        return;
    }
    
    uint32_t embedding_dim = model->embedding_dim;
    uint32_t num_heads = model->num_heads;
    uint32_t head_dim = embedding_dim / num_heads;
    
    // Allocate workspace for Q/K/V
    size_t qkv_size = batch_size * seq_len * embedding_dim * sizeof(double);
    double* Q = (double*)calloc(batch_size * seq_len * embedding_dim, sizeof(double));
    double* K = (double*)calloc(batch_size * seq_len * embedding_dim, sizeof(double));
    double* V = (double*)calloc(batch_size * seq_len * embedding_dim, sizeof(double));
    double* scores = (double*)calloc(seq_len * seq_len, sizeof(double));
    double* attn_output = (double*)calloc(batch_size * seq_len * embedding_dim, sizeof(double));
    
    if (!Q || !K || !V || !scores || !attn_output) {
        fprintf(stderr, "Error: Failed to allocate attention workspace\n");
        free(Q); free(K); free(V); free(scores); free(attn_output);
        return;
    }
    
    // STEP 1: Compute Q/K/V using thread-local weights
    // For now, use a simplified approach: get weights from first worker thread
    // TODO: Properly distribute across all 11 worker threads
    
    HierarchicalThread* worker_thread = hierarchical_thread_get(
        model->threads, layer_idx, 1  // First worker thread
    );
    
    if (!worker_thread) {
        fprintf(stderr, "Error: Failed to get worker thread for layer %u\n", layer_idx);
        free(Q); free(K); free(V); free(scores); free(attn_output);
        return;
    }
    
    // Process each batch and token
    for (uint32_t b = 0; b < batch_size; b++) {
        for (uint32_t t = 0; t < seq_len; t++) {
            const double* token_embedding = &input[b * seq_len * embedding_dim + t * embedding_dim];
            double* token_Q = &Q[b * seq_len * embedding_dim + t * embedding_dim];
            double* token_K = &K[b * seq_len * embedding_dim + t * embedding_dim];
            double* token_V = &V[b * seq_len * embedding_dim + t * embedding_dim];
            
            // Compute Q/K/V using thread-local weights
            cllm_compute_qkv(
                worker_thread,
                token_embedding,
                token_Q,
                token_K,
                token_V,
                embedding_dim,
                num_heads,
                head_dim
            );
        }
    }
    
    // STEP 2: Compute attention scores and apply softmax
    // This is done by the control thread (dimension 0)
    
    double scale = 1.0 / crystalline_sqrt((double)head_dim);
    
    for (uint32_t b = 0; b < batch_size; b++) {
        double* batch_Q = &Q[b * seq_len * embedding_dim];
        double* batch_K = &K[b * seq_len * embedding_dim];
        double* batch_V = &V[b * seq_len * embedding_dim];
        double* batch_output = &attn_output[b * seq_len * embedding_dim];
        
        // Multi-head attention
        for (uint32_t h = 0; h < num_heads; h++) {
            // Compute attention scores
            cllm_compute_attention_scores(
                &batch_Q[h * head_dim],
                &batch_K[h * head_dim],
                scores,
                1,  // Process one head at a time
                seq_len,
                head_dim
            );
            
            // Apply softmax
            cllm_apply_softmax_to_scores(scores, 1, seq_len);
            
            // Compute attention output
            cllm_compute_attention_output(
                scores,
                &batch_V[h * head_dim],
                &batch_output[h * head_dim],
                1,  // Process one head at a time
                seq_len,
                head_dim
            );
        }
    }
    
    // STEP 3: Apply output projection using thread-local weights
    
    for (uint32_t b = 0; b < batch_size; b++) {
        for (uint32_t t = 0; t < seq_len; t++) {
            const double* token_attn = &attn_output[b * seq_len * embedding_dim + t * embedding_dim];
            double* token_output = &output[b * seq_len * embedding_dim + t * embedding_dim];
            
            // Apply output projection using thread-local weights
            cllm_apply_output_projection(
                worker_thread,
                token_attn,
                token_output,
                embedding_dim,
                num_heads,
                head_dim
            );
        }
    }
    
    // Cleanup
    free(Q);
    free(K);
    free(V);
    free(scores);
    free(attn_output);
}

/**
 * Simplified attention forward (wrapper for backward compatibility)
 * 
 * This function provides a simple interface that automatically uses
 * the thread-centric implementation.
 */
void cllm_attention_forward_simple(
    CLLMModel* model,
    uint32_t layer_idx,
    const double* input,
    double* output,
    uint32_t batch_size,
    uint32_t seq_len
) {
    cllm_attention_forward(model, layer_idx, input, output, batch_size, seq_len);
}