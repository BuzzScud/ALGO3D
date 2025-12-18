/**
 * @file cllm_embeddings_88d.h
 * @brief Parallel Embeddings using 88D Threading System
 * 
 * This header defines the interface for parallel embedding lookup and
 * computation that leverages the 88D threading system.
 */

#ifndef CLLM_EMBEDDINGS_88D_H
#define CLLM_EMBEDDINGS_88D_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// EMBEDDING STATISTICS
// ============================================================================

/**
 * Embedding statistics structure
 */
typedef struct {
    uint32_t num_threads;              // Number of threads used
    uint64_t embeddings_computed;      // Total embeddings computed
    double total_embedding_time_ms;    // Total computation time
    double lookups_per_second;         // Embedding lookups per second
} CLLMEmbeddingStats;

// ============================================================================
// EMBEDDING OPERATIONS
// ============================================================================

/**
 * Parallel embedding lookup
 * 
 * Looks up embeddings for token IDs in parallel across the 88D threading
 * system. Work is distributed by batch items.
 * 
 * @param model CLLM model
 * @param token_ids Token IDs [batch × seq_len]
 * @param embedding_table Embedding table [vocab_size × embed_dim]
 * @param output Output embeddings [batch × seq_len × embed_dim]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param vocab_size Vocabulary size
 * @param embed_dim Embedding dimension
 * @return 0 on success, -1 on error
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
);

/**
 * Parallel position encoding
 * 
 * Adds sinusoidal position encodings to embeddings in parallel.
 * Uses the standard transformer position encoding formula.
 * 
 * @param model CLLM model
 * @param embeddings Embeddings to add position encoding to [batch × seq_len × embed_dim]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param embed_dim Embedding dimension
 * @param max_position Maximum position value
 * @return 0 on success, -1 on error
 */
int cllm_embeddings_add_position_encoding_parallel(
    CLLMModel* model,
    float* embeddings,
    uint32_t batch_size,
    uint32_t seq_length,
    uint32_t embed_dim,
    uint32_t max_position
);

/**
 * Parallel token embedding with position encoding
 * 
 * Convenience function that combines embedding lookup and position encoding.
 * 
 * @param model CLLM model
 * @param token_ids Token IDs [batch × seq_len]
 * @param embedding_table Embedding table [vocab_size × embed_dim]
 * @param output Output embeddings [batch × seq_len × embed_dim]
 * @param batch_size Batch size
 * @param seq_length Sequence length
 * @param vocab_size Vocabulary size
 * @param embed_dim Embedding dimension
 * @param add_position_encoding Whether to add position encoding
 * @return 0 on success, -1 on error
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
);

// ============================================================================
// EMBEDDING STATISTICS
// ============================================================================

/**
 * Get embedding statistics
 * 
 * @param model CLLM model
 * @param stats Output: embedding statistics
 * @return 0 on success, -1 on error
 */
int cllm_embeddings_get_stats(
    CLLMModel* model,
    CLLMEmbeddingStats* stats
);

/**
 * Print embedding statistics
 * 
 * @param stats Embedding statistics to print
 */
void cllm_embeddings_print_stats(const CLLMEmbeddingStats* stats);

#ifdef __cplusplus
}
#endif

#endif // CLLM_EMBEDDINGS_88D_H