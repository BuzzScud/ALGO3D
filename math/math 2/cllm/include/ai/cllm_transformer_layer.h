/**
 * Complete Transformer Layer API
 * 
 * Provides functions for processing tokens through complete transformer layers
 * with attention, feed-forward networks, and layer normalization.
 */

#ifndef CLLM_TRANSFORMER_LAYER_H
#define CLLM_TRANSFORMER_LAYER_H

#include "ai/cllm.h"
#include "hierarchical_threading.h"
#include <stdint.h>

/**
 * Process a token through a complete transformer layer
 * 
 * @param model The CLLM model
 * @param thread The thread processing this token
 * @param layer_idx The layer index (0-7)
 * @param input Input embedding (embedding_dim)
 * @param output Output embedding (embedding_dim)
 * @return 0 on success, -1 on error
 */
int cllm_transformer_layer_forward(
    CLLMModel* model,
    HierarchicalThread* thread,
    uint8_t layer_idx,
    const double* input,
    double* output
);

/**
 * Process a token through all transformer layers (thread-based)
 * 
 * @param model The CLLM model
 * @param thread The thread processing this token
 * @param input Input embedding (embedding_dim)
 * @param output Output embedding (embedding_dim)
 * @return 0 on success, -1 on error
 */
int cllm_transformer_forward(
    CLLMModel* model,
    HierarchicalThread* thread,
    const double* input,
    double* output
);

/**
 * Compute logits for all vocabulary tokens
 * 
 * @param model The CLLM model
 * @param hidden Final hidden state (embedding_dim)
 * @param logits Output logits (vocab_size)
 * @return 0 on success, -1 on error
 */
int cllm_compute_logits(
    CLLMModel* model,
    const double* hidden,
    double* logits
);

/**
 * Apply softmax to logits (in-place)
 * Uses pure crystalline math (no math.h)
 * 
 * @param logits Logits array (vocab_size)
 * @param vocab_size Size of vocabulary
 */
void cllm_softmax(double* logits, uint32_t vocab_size);

/**
 * Sample from probability distribution
 * 
 * @param probs Probability distribution (vocab_size)
 * @param vocab_size Size of vocabulary
 * @param temperature Sampling temperature (higher = more random)
 * @return Sampled token ID
 */
uint32_t cllm_sample_token(const double* probs, uint32_t vocab_size, double temperature);

#endif // CLLM_TRANSFORMER_LAYER_H