/**
 * @file cllm_embedding_helpers.c
 * @brief Helper Functions for Thread-Based Embedding Access
 * 
 * These helper functions provide a clean API for accessing embeddings
 * stored in thread CrystallineAbacus instead of flat arrays.
 * 
 * Key functions:
 * - cllm_get_embedding_from_thread() - Get embedding from thread
 * - cllm_set_embedding_to_thread() - Set embedding in thread
 * - cllm_get_embedding() - Get embedding by token ID
 * - cllm_set_embedding() - Set embedding by token ID
 */

#include "ai/cllm.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// THREAD-BASED EMBEDDING ACCESS
// ============================================================================

/**
 * Get embedding from thread's CrystallineAbacus
 * 
 * @param thread Thread containing the embedding
 * @param output Output buffer [embedding_dim]
 * @param embedding_dim Embedding dimension
 * @return 0 on success, -1 on error
 */
int cllm_get_embedding_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t embedding_dim
) {
    if (!thread || !output) {
        return -1;
    }
    
    // TODO: Implement proper CrystallineAbacus to double array conversion
    // For now, use thread's activation_buffer if available
    if (thread->activation_buffer && thread->activation_buffer_size >= embedding_dim) {
        memcpy(output, thread->activation_buffer, embedding_dim * sizeof(double));
        return 0;
    }
    
    // Fallback: Zero out (thread not yet initialized)
    memset(output, 0, embedding_dim * sizeof(double));
    return 0;
}

/**
 * Set embedding in thread's CrystallineAbacus
 * 
 * @param thread Thread to store the embedding
 * @param input Input embedding [embedding_dim]
 * @param embedding_dim Embedding dimension
 * @return 0 on success, -1 on error
 */
int cllm_set_embedding_to_thread(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim
) {
    if (!thread || !input) {
        return -1;
    }
    
    // Allocate activation buffer if needed
    if (!thread->activation_buffer) {
        thread->activation_buffer = calloc(embedding_dim, sizeof(double));
        thread->activation_buffer_size = embedding_dim;
        if (!thread->activation_buffer) {
            return -1;
        }
    }
    
    // TODO: Implement proper double array to CrystallineAbacus conversion
    // For now, store in activation_buffer
    memcpy(thread->activation_buffer, input, embedding_dim * sizeof(double));
    
    return 0;
}

/**
 * Get embedding by token ID (convenience function)
 * 
 * @param model Model containing the token assignments
 * @param token_id Token ID
 * @param output Output buffer [embedding_dim]
 * @return 0 on success, -1 on error
 */
int cllm_get_embedding_from_model(
    const CLLMModel* model,
    uint32_t token_id,
    double* output
) {
    if (!model || !output || token_id >= model->vocab_size) {
        return -1;
    }
    
    if (!model->token_assignments) {
        return -1;
    }
    
    // Get thread for this token
    HierarchicalThread* thread = model->token_assignments[token_id].thread;
    if (!thread) {
        return -1;
    }
    
    return cllm_get_embedding_from_thread(thread, output, model->embedding_dim);
}

/**
 * Set embedding by token ID (convenience function)
 * 
 * @param model Model containing the token assignments
 * @param token_id Token ID
 * @param input Input embedding [embedding_dim]
 * @return 0 on success, -1 on error
 */
int cllm_set_embedding(
    CLLMModel* model,
    uint32_t token_id,
    const double* input
) {
    if (!model || !input || token_id >= model->vocab_size) {
        return -1;
    }
    
    if (!model->token_assignments) {
        return -1;
    }
    
    // Get thread for this token
    HierarchicalThread* thread = model->token_assignments[token_id].thread;
    if (!thread) {
        return -1;
    }
    
    return cllm_set_embedding_to_thread(thread, input, model->embedding_dim);
}

/**
 * Get pointer to embedding (for read-only access)
 * 
 * WARNING: This returns a pointer to thread-local storage.
 * Do not modify the returned pointer.
 * Do not store the pointer - it may become invalid.
 * 
 * @param model Model containing the token assignments
 * @param token_id Token ID
 * @return Pointer to embedding (read-only), or NULL on error
 */
const double* cllm_get_embedding_ptr(
    const CLLMModel* model,
    uint32_t token_id
) {
    if (!model || token_id >= model->vocab_size) {
        return NULL;
    }
    
    if (!model->token_assignments) {
        return NULL;
    }
    
    // Get thread for this token
    HierarchicalThread* thread = model->token_assignments[token_id].thread;
    if (!thread || !thread->activation_buffer) {
        return NULL;
    }
    
    return thread->activation_buffer;
}

/**
 * Get mutable pointer to embedding (for modification)
 * 
 * WARNING: This returns a pointer to thread-local storage.
 * Modifications affect the thread's embedding directly.
 * Do not store the pointer - it may become invalid.
 * 
 * @param model Model containing the token assignments
 * @param token_id Token ID
 * @return Pointer to embedding (mutable), or NULL on error
 */
double* cllm_get_embedding_ptr_mut(
    CLLMModel* model,
    uint32_t token_id
) {
    if (!model || token_id >= model->vocab_size) {
        return NULL;
    }
    
    if (!model->token_assignments) {
        return NULL;
    }
    
    // Get thread for this token
    HierarchicalThread* thread = model->token_assignments[token_id].thread;
    if (!thread) {
        return NULL;
    }
    
    // Allocate activation buffer if needed
    if (!thread->activation_buffer) {
        thread->activation_buffer = calloc(model->embedding_dim, sizeof(double));
        thread->activation_buffer_size = model->embedding_dim;
        if (!thread->activation_buffer) {
            return NULL;
        }
    }
    
    return thread->activation_buffer;
}

/**
 * Copy embedding from one token to another
 * 
 * @param model Model containing the token assignments
 * @param src_token_id Source token ID
 * @param dst_token_id Destination token ID
 * @return 0 on success, -1 on error
 */
int cllm_copy_embedding(
    CLLMModel* model,
    uint32_t src_token_id,
    uint32_t dst_token_id
) {
    if (!model || src_token_id >= model->vocab_size || dst_token_id >= model->vocab_size) {
        return -1;
    }
    
    // Get source embedding
    double* temp = calloc(model->embedding_dim, sizeof(double));
    if (!temp) {
        return -1;
    }
    
    if (cllm_get_embedding_from_model(model, src_token_id, temp) != 0) {
        free(temp);
        return -1;
    }
    
    // Set destination embedding
    int result = cllm_set_embedding(model, dst_token_id, temp);
    free(temp);
    
    return result;
}