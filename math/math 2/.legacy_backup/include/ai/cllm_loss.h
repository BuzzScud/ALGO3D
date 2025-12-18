#ifndef CLLM_LOSS_H
#define CLLM_LOSS_H

#include <stdint.h>
#include "cllm.h"  // Need full CLLMModel definition for accessing prime encodings

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compute Loss (The ONLY Loss Function)
 * 
 * Uses deterministic GCD-based similarity with learned prime encodings
 * and lattice positions. This is the core of the crystalline design.
 * 
 * @param model CLLM model with prime encodings
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Average loss
 */

/**
 * Compute loss with detailed statistics
 * 
 * @param model CLLM model
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @param avg_gcd_sim Output: average GCD similarity
 * @param avg_spatial_sim Output: average spatial similarity
 * @return Average loss
 */
double cllm_compute_loss_detailed(
    CLLMModel* model,
    uint32_t* input_tokens,
    uint32_t* target_tokens,
    int num_tokens,
    double* avg_gcd_sim,
    double* avg_spatial_sim
);

/**
 * Compute loss gradient for backpropagation
 * 
 * @param logits Predicted logits [vocab_size]
 * @param target Target token ID
 * @param vocab_size Vocabulary size
 * @param grad_output Output gradients [vocab_size] (caller must allocate)
 */
void cllm_compute_loss_gradient(double* logits, uint32_t target, 
                                int vocab_size, double* grad_output);

/**
 * Compute perplexity from loss
 * 
 * @param loss Loss value
 * @return Perplexity value
 */
double cllm_compute_perplexity(double loss);

/**
 * Compute accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @return Accuracy (0 to 1)
 */
double cllm_compute_accuracy(double* logits, uint32_t* targets,
                           int batch_size, int vocab_size);

/**
 * Compute top-k accuracy
 * 
 * @param logits Predicted logits [batch_size x vocab_size]
 * @param targets Target token IDs [batch_size]
 * @param batch_size Batch size
 * @param vocab_size Vocabulary size
 * @param k Top-k value
 * @return Top-k accuracy (0 to 1)
 */
double cllm_compute_top_k_accuracy(double* logits, uint32_t* targets,
                                  int batch_size, int vocab_size, int k);

#ifdef __cplusplus
}
#endif

#endif // CLLM_LOSS_H