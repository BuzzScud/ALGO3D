#ifndef CLLM_INFERENCE_H
#define CLLM_INFERENCE_H

/*
 * cllm_inference.h - Inference engine header
 * Source: cllm_inference.c
 * REVERTED: Uses double* for standard precision (BigFixed removed)
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "math/types.h"

/* Local includes */
#include "cllm.h"

/* Type definitions */
typedef struct { int idx; float prob; } IndexProb;

/* Legacy type compatibility - these are now part of CLLMModel */
typedef struct AttentionLayer AttentionLayer;
typedef struct Embeddings Embeddings;
typedef struct PositionalEncoding PositionalEncoding;

/* FeedForwardLayer structure definition */
typedef struct FeedForwardLayer {
    uint32_t input_dim;
    uint32_t hidden_dim;
    uint32_t output_dim;
    double* w1;      // [input_dim × hidden_dim]
    double* b1;      // [hidden_dim]
    double* w2;      // [hidden_dim × output_dim]
    double* b2;      // [output_dim]
} FeedForwardLayer;

/*
 * CLLM Inference Engine - Runtime inference state
 * Phase 2: NO GLOBAL BUFFERS - All computation in thread-local storage
 * 
 * CRITICAL: This structure contains NO working buffers.
 * All computation happens in thread-local CrystallineAbacus storage.
 * The thread pool (model->threads) is MANDATORY.
 */
typedef struct {
    CLLMModel* model;            // Pointer to the model (MUST have threads)
    float temperature;           // Sampling temperature
    float top_p;                 // Nucleus sampling parameter
    int top_k;                   // Top-k sampling parameter
    int max_tokens;              // Maximum tokens to generate
    float repetition_penalty;    // Repetition penalty factor
    
    // Generation state tracking (NO working buffers)
    int current_position;        // Current generation position
    uint32_t* generated_tokens;  // Generated token sequence
    int num_generated;           // Number of tokens generated
       
} CLLMInference;

/* Function declarations */
CLLMInference* cllm_inference_init(CLLMModel* model);
int cllm_tokenize(CLLMInference* inference, const char* text, uint32_t* tokens, int max_tokens);
void cllm_detokenize(CLLMInference* inference, uint32_t* tokens, int num_tokens, char* output, int max_length);
void cllm_get_embedding(CLLMInference* inference, uint32_t token_id, float* embedding);
void cllm_apply_positional_encoding(CLLMInference* inference, double* embedding, int position);

/* Embedding layer functions */
void cllm_embed_token(CLLMInference* inf, uint32_t token_id, float* output);
void cllm_apply_lattice_transform(float* embedding, double* transform, int dim);
void cllm_get_embedding_transformed(CLLMInference* inf, uint32_t token_id, float* output);
void cllm_embed_tokens_batch(CLLMInference* inf, uint32_t* token_ids, int num_tokens, float* output);
void cllm_project_to_vocab(CLLMInference* inf, float* hidden_state, float* logits);
float cllm_embedding_norm(float* embedding, int dim);
void cllm_normalize_embedding(float* embedding, int dim);

/* Embedding initialization functions */
void cllm_init_embeddings(CLLMModel* model);
void cllm_init_embeddings_with_lattice(CLLMModel* model);  // Legacy wrapper

/* Attention functions */
void cllm_attention_forward(AttentionLayer* layer, double* input, double* output,
                           float* key_cache, float* value_cache, int seq_len);

/**
 * Enhanced attention forward pass with neighbor influence (DEFAULT)
 * 
 * This is the STANDARD attention function that includes neighbor influence
 * from the 12 kissing spheres. Since kissing spheres is THE architecture,
 * this is the default behavior (not optional).
 * 
 * Use this function for all new code. The basic cllm_attention_forward()
 * is kept for backward compatibility only.
 * 
 * @param layer Attention layer
 * @param input Input tensor [seq_len x embed_dim]
 * @param output Output tensor [seq_len x embed_dim]
 * @param model CLLM model (for neighbor access, can be NULL for basic attention)
 * @param token_ids Token IDs [seq_len] (for neighbor lookup, can be NULL)
 * @param key_cache Key cache (optional)
 * @param value_cache Value cache (optional)
 * @param seq_len Sequence length
 * @param neighbor_strength Neighbor influence strength (0.0-1.0, default 0.1)
 */
void cllm_attention_forward_enhanced(
    AttentionLayer* layer,
    double* input,
    double* output,
    CLLMModel* model,
    uint32_t* token_ids,
    float* key_cache,
    float* value_cache,
    int seq_len,
    float neighbor_strength
);

void cllm_attention_forward_hybrid(CLLMModel* model, AttentionLayer* layer, float* input, float* output,
                                   uint32_t* token_ids, float* key_cache, float* value_cache, int seq_len);
void cllm_multi_head_attention(CLLMInference* inf, int layer_idx, float* input, float* output, int seq_len);
void cllm_attention_init(AttentionLayer* layer, uint32_t num_heads, uint32_t head_dim);
void cllm_attention_free(AttentionLayer* layer);

/* Feed-forward functions */
void cllm_feedforward(FeedForwardLayer* layer, double* input, double* output);
void cllm_activation_gelu(float* x, int size);
void cllm_activation_relu(float* x, int size);
void cllm_feedforward_inplace(FeedForwardLayer* layer, float* data);
void cllm_feedforward_batch(FeedForwardLayer* layer, float* input, float* output, int batch_size);
void cllm_feedforward_init(FeedForwardLayer* layer, uint32_t input_dim, uint32_t hidden_dim, uint32_t output_dim);
void cllm_feedforward_free(FeedForwardLayer* layer);

/* Layer normalization functions */
void cllm_layer_norm(CLLMLayerNorm* ln, double* input, double* output);
void cllm_layer_norm_inplace(CLLMLayerNorm* ln, float* data);
void cllm_layer_norm_batch(CLLMLayerNorm* ln, float* input, float* output, int batch_size);
void cllm_layer_norm_init(CLLMLayerNorm* ln, uint32_t dim, float epsilon);
void cllm_layer_norm_free(CLLMLayerNorm* ln);
void cllm_layer_norm_stats(float* input, uint32_t dim, float* mean, float* variance);

/* Lattice embedding functions */
void cllm_compute_spiral_position(uint64_t prime, double* angle, double* radius);
void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, double* coords);
void cllm_generate_lattice_embedding(uint32_t token_id, uint64_t prime, uint32_t embedding_dim, double* output);
void cllm_generate_lattice_embeddings(CLLMModel* model);
void cllm_generate_lattice_transform(double* transform, int dim);
double cllm_lattice_token_distance(uint32_t token1_id, uint64_t prime1, uint32_t token2_id, uint64_t prime2);
void cllm_find_lattice_neighbors(uint32_t token_id, uint64_t prime, uint32_t* all_tokens, uint64_t* all_primes, uint32_t num_tokens, int k, uint32_t* neighbors);

/* Positional encoding functions */
void cllm_generate_spiral_encoding(PositionalEncoding* pos_enc);
void cllm_generate_clock_encoding(PositionalEncoding* pos_enc);
void cllm_generate_prime_encoding(PositionalEncoding* pos_enc);
void cllm_initialize_learned_encoding(PositionalEncoding* pos_enc);
void cllm_apply_positional_encoding_complete(CLLMInference* inf, float* embedding, int position);
void cllm_generate_all_positional_encodings(CLLMModel* model);
void cllm_free_positional_encoding(PositionalEncoding* pos_enc);

/* Symmetry functions */
uint32_t cllm_compute_symmetry_group(uint64_t prime);
void cllm_apply_symmetry_transform(double* embedding, int symmetry_group, int dim);
void cllm_apply_inverse_symmetry_transform(double* embedding, int symmetry_group, int dim);
void cllm_compute_symmetry_invariants(double* embedding, int dim, double* features, int num_features);
void cllm_apply_equivariant_transform(double* embedding, int symmetry_group, double* transform_matrix, int dim);
double cllm_symmetry_compatibility(double* embedding1, int symmetry1, double* embedding2, int symmetry2, int dim);
void cllm_generate_symmetry_attention_mask(int* symmetry_groups, int seq_len, double* mask);

/* Forward pass and generation */
void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens);
void cllm_compute_logits(CLLMInference* inf, float* hidden_state);
int cllm_sample_token(CLLMInference* inf, double* logits);
void cllm_apply_temperature(double* logits, int vocab_size, double temperature);
void cllm_softmax(double* logits, int vocab_size);
uint32_t cllm_sample_top_k(double* probs, int vocab_size, int k);
uint32_t cllm_sample_top_p(double* probs, int vocab_size, double p);

int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length);

/* Configuration */
void cllm_set_temperature(CLLMInference* inference, float temperature);
void cllm_set_top_p(CLLMInference* inference, float top_p);
void cllm_set_top_k(CLLMInference* inference, int top_k);
void cllm_set_max_tokens(CLLMInference* inference, int max_tokens);
void cllm_inference_cleanup(CLLMInference* inference);

// ============================================================================
// THREAD-LOCAL INFERENCE FUNCTIONS (DAY 12)
// ============================================================================

/**
 * Generate a single token using thread-centric computation
 * 
 * This function uses the thread pool and work queue system to generate
 * the next token. All computation happens in thread-local CrystallineAbacus.
 * 
 * @param inference Inference context
 * @param context Array of context token IDs
 * @param context_len Number of tokens in context
 * @return Next token ID, or 0 on error
 */
uint32_t cllm_generate_token(CLLMInference* inference,
                                       const uint32_t* context,
                                       int context_len);

/**
 * Generate text using thread-centric computation
 * 
 * This function generates text by repeatedly calling cllm_generate_token()
 * until max_tokens is reached or an end-of-sequence token is generated.
 * 
 * @param inference Inference context
 * @param prompt Input prompt text
 * @param output Output buffer for generated text
 * @param max_output_length Maximum length of output buffer
 * @return Number of tokens generated, or -1 on error
 */
int cllm_generate(CLLMInference* inference,
                           const char* prompt,
                           char* output,
                           int max_output_length);

#endif /* CLLM_INFERENCE_H */
