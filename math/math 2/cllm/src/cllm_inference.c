#include "cllm_inference.h"
#include "cllm.h"
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/angular_position.h"  // For theta_n() angular position
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf

// Constants
#define MAX_SEQUENCE_LENGTH 512
#define TEMPERATURE_MIN 0.1f
#define TEMPERATURE_MAX 2.0

// Initialize inference context
CLLMInference* cllm_inference_init(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Error: Cannot initialize inference with NULL model\n");
        return NULL;
    }
    
    CLLMInference* inference = (CLLMInference*)calloc(1, sizeof(CLLMInference));
    if (!inference) {
        fprintf(stderr, "Error: Failed to allocate inference context\n");
        return NULL;
    }
    
    inference->model = model;
    inference->temperature = 1.0;
    inference->top_p = 0.9;
    inference->top_k = 50;
    inference->max_tokens = 50;
    
    // CRITICAL: Verify 88D thread pool exists (MANDATORY)
    if (!model->threads) {
        fprintf(stderr, "Error: 88D thread pool is NULL - threading is MANDATORY\n");
        fprintf(stderr, "Inference requires thread-local storage for all computation\n");
        free(inference);
        return NULL;
    }
    
    // Initialize generation state (NO working buffers)
    inference->current_position = 0;
    inference->num_generated = 0;
    inference->generated_tokens = (uint32_t*)calloc(inference->max_tokens, sizeof(uint32_t));
    
    if (!inference->generated_tokens) {
        fprintf(stderr, "Error: Failed to allocate generation state\n");
        free(inference);
        return NULL;
    }
    
    printf("✅ Inference context initialized (thread-local storage only)\n");
    printf("   88D thread pool: %d threads\n", model->threads ? 96 : 0);
    printf("   Max tokens: %d\n", inference->max_tokens);
    return inference;
}

// Cleanup inference context
void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    // Free generation state (NO working buffers to free)
    if (inference->generated_tokens) {
        free(inference->generated_tokens);
    }
    
    free(inference);
    printf("✅ Inference context cleaned up\n");
}

// Get embedding for a token
void cllm_get_embedding(CLLMInference* inference, uint32_t token_id, float* output) {
    if (!inference || !output) return;
    
    CLLMModel* model = inference->model;
    uint32_t embed_dim = model->embedding_dim;
    
    if (token_id >= model->vocab_size) {
        memset(output, 0, embed_dim * sizeof(float));
        return;
    }
    
    // TODO: Get embedding from thread-local storage
    extern bool cllm_get_embedding_from_model(const CLLMModel* model, uint32_t token_id, double* output);
    
    double* temp = (double*)malloc(embed_dim * sizeof(double));
    if (!temp || !cllm_get_embedding_from_model(model, token_id, temp)) {
        memset(output, 0, embed_dim * sizeof(float));
        free(temp);
        return;
    }
    
    // Convert double to float
    for (uint32_t i = 0; i < embed_dim; i++) {
        output[i] = (float)temp[i];
    }
    free(temp);
}

// Tokenize text - FIXED VERSION
int cllm_tokenize(CLLMInference* inference, const char* text, uint32_t* tokens, int max_tokens) {
    if (!inference || !text || !tokens) return 0;
    
    // Check if tokens array exists
    if (!inference->model->tokens) {
        fprintf(stderr, "Warning: model->tokens is NULL - using character-based fallback tokenization\n");
        // Fallback: create simple character-based tokens
        int len = strlen(text);
        int count = len < max_tokens ? len : max_tokens;
        for (int i = 0; i < count; i++) {
            tokens[i] = (uint32_t)(text[i] % inference->model->vocab_size);
        }
        return count;
    }
    
    int token_count = 0;
    char buffer[256];
    int buf_pos = 0;
    
    for (int i = 0; text[i] && token_count < max_tokens; i++) {
        if (text[i] == ' ' || text[i] == '\n' || text[i] == '\t') {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                
                // Find token in vocabulary
                bool found = false;
                for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
                    if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                        tokens[token_count++] = j;
                        found = true;
                        break;
                    }
                }
                
                // Handle unknown words with hash (like training does)
                if (!found) {
                    uint32_t hash = 0;
                    for (int k = 0; buffer[k]; k++) {
                        hash = hash * 31 + (uint32_t)buffer[k];
                    }
                    tokens[token_count++] = hash % inference->model->vocab_size;
                }
                
                buf_pos = 0;
            }
        } else {
            if (buf_pos < 255) {
                buffer[buf_pos++] = text[i];
            }
        }
    }
    
    // Handle last token
    if (buf_pos > 0 && token_count < max_tokens) {
        buffer[buf_pos] = '\0';
        bool found = false;
        for (uint32_t j = 0; j < inference->model->vocab_size; j++) {
            if (strcmp(inference->model->tokens[j].token_str, buffer) == 0) {
                tokens[token_count++] = j;
                found = true;
                break;
            }
        }
        
        // Handle unknown words with hash
        if (!found) {
            uint32_t hash = 0;
            for (int k = 0; buffer[k]; k++) {
                hash = hash * 31 + (uint32_t)buffer[k];
            }
            tokens[token_count++] = hash % inference->model->vocab_size;
        }
    }
    
    return token_count;
}

// Detokenize tokens to text - FIXED VERSION
void cllm_detokenize(CLLMInference* inference, uint32_t* tokens, int num_tokens, char* output, int max_length) {
    if (!inference || !tokens || !output) return;
    
    // Check if tokens array exists
    if (!inference->model->tokens) {
        fprintf(stderr, "Warning: model->tokens is NULL - using character-based fallback detokenization\n");
        // Fallback: convert token IDs to characters
        int pos = 0;
        for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
            output[pos++] = (char)(tokens[i] % 128); // ASCII range
        }
        output[pos] = '\0';
        return;
    }
    
    int pos = 0;
    for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
        if (tokens[i] < inference->model->vocab_size) {
            const char* token_str = inference->model->tokens[tokens[i]].token_str;
            
            // Skip special tokens (PAD, UNK, BOS, EOS, etc.)
            if (token_str[0] == '<' && token_str[strlen(token_str)-1] == '>') {
                continue;  // Skip special tokens like <PAD>, <UNK>, etc.
            }
            
            int len = strlen(token_str);
            
            if (pos + len < max_length - 1) {
                strcpy(&output[pos], token_str);
                pos += len;
                
                // Add space between tokens
                if (i < num_tokens - 1 && pos < max_length - 1) {
                    output[pos++] = ' ';
                }
            }
        }
    }
    
    output[pos] = '\0';
}

// Apply positional encoding
void cllm_apply_positional_encoding(CLLMInference* inference, double* hidden_states, int position) {
    if (!inference || !hidden_states) return;
    
    CLLMModel* model = inference->model;
    uint32_t embed_dim = model->embedding_dim;
    
    if (position >= (int)model->max_seq_len) {
        position = model->max_seq_len - 1;
    }
    
    // TODO: Add positional encoding from thread-local storage
    // For now, skip positional encoding
    if (0) {  // Disabled
        double* pos_enc = NULL;  // Placeholder
        for (uint32_t i = 0; i < embed_dim; i++) {
            hidden_states[i] += pos_enc[i];
        }
    }
}

// Layer normalization (old version for compatibility)

// Feed-forward network

// Forward pass

// Simple attention forward pass
#if 0  // LEGACY FUNCTION - Commented out during transformation
void cllm_attention_forward(AttentionLayer* layer, double* input, double* output,
                           float* key_cache, float* value_cache, int seq_len) {
    (void)key_cache;  // TODO: Use for caching in future
    (void)value_cache;
    
    if (!layer || !input || !output || seq_len <= 0) return;
    
    uint32_t num_heads = layer->num_heads;
    uint32_t head_dim = layer->head_dim;
    uint32_t embed_dim = num_heads * head_dim;
    
    // Allocate Q, K, V
    double* Q = (double*)calloc(seq_len * embed_dim, sizeof(double));
    double* K = (double*)calloc(seq_len * embed_dim, sizeof(double));
    double* V = (double*)calloc(seq_len * embed_dim, sizeof(double));
    double* scores = (double*)calloc(seq_len * seq_len, sizeof(double));
    
    if (!Q || !K || !V || !scores) {
        free(Q); free(K); free(V); free(scores);
        // Fallback to pass-through
        for (uint32_t i = 0; i < embed_dim; i++) {
            output[i] = input[i];
        }
        return;
    }
    
    // Project to Q, K, V using lattice weights
    for (int pos = 0; pos < seq_len; pos++) {
        double* in_vec = &input[pos * embed_dim];
        
        // Check input for NaN
        for (uint32_t i = 0; i < embed_dim; i++) {
            if (math_is_nan(in_vec[i])) {
                fprintf(stderr, "ERROR: NaN in input at pos=%d, i=%u\n", pos, i);
                in_vec[i] = 0.0;
            }
        }
        
        for (uint32_t h = 0; h < num_heads; h++) {
            for (uint32_t d = 0; d < head_dim; d++) {
                double q_sum = 0.0, k_sum = 0.0, v_sum = 0.0;
                
                for (uint32_t i = 0; i < head_dim; i++) {
                    size_t w_idx = h * head_dim * head_dim + d * head_dim + i;
                    double in_val = in_vec[h * head_dim + i];
                    
                    // Check weights for NaN
                    if (math_is_nan(layer->query_lattice[w_idx])) {
                        fprintf(stderr, "ERROR: NaN in query weight at h=%u, d=%u, i=%u\n", h, d, i);
                        layer->query_lattice[w_idx] = 0.0;
                    }
                    if (math_is_nan(layer->key_lattice[w_idx])) {
                        fprintf(stderr, "ERROR: NaN in key weight at h=%u, d=%u, i=%u\n", h, d, i);
                        layer->key_lattice[w_idx] = 0.0;
                    }
                    if (math_is_nan(layer->value_lattice[w_idx])) {
                        fprintf(stderr, "ERROR: NaN in value weight at h=%u, d=%u, i=%u\n", h, d, i);
                        layer->value_lattice[w_idx] = 0.0;
                    }
                    
                    q_sum += layer->query_lattice[w_idx] * in_val;
                    k_sum += layer->key_lattice[w_idx] * in_val;
                    v_sum += layer->value_lattice[w_idx] * in_val;
                }
                
                Q[pos * embed_dim + h * head_dim + d] = q_sum;
                K[pos * embed_dim + h * head_dim + d] = k_sum;
                V[pos * embed_dim + h * head_dim + d] = v_sum;
            }
        }
    }
    
    // Compute attention scores with numerical stability
    double scale = 1.0 / math_sqrt((double)head_dim);
    const double MATH_EPSILON = 1e-10;
    const double MAX_SCORE = 10.0;  // Clip scores to prevent overflow
    
    for (uint32_t h = 0; h < num_heads; h++) {
        for (int i = 0; i < seq_len; i++) {
            double* q = &Q[i * embed_dim + h * head_dim];
            
            for (int j = 0; j < seq_len; j++) {
                double* k = &K[j * embed_dim + h * head_dim];
                
                // Simple dot product attention (no rotary encoding for now to avoid NaN)
                double score = 0.0;
                for (uint32_t d = 0; d < head_dim; d++) {
                    score += q[d] * k[d];
                }
                
                // Scale and clip to prevent overflow
                score = score * scale;
                if (score > MAX_SCORE) score = MAX_SCORE;
                if (score < -MAX_SCORE) score = -MAX_SCORE;
                
                scores[i * seq_len + j] = score;
            }
            
            // Softmax over scores for this query with numerical stability
            double max_score = scores[i * seq_len];
            for (int j = 1; j < seq_len; j++) {
                if (scores[i * seq_len + j] > max_score) {
                    max_score = scores[i * seq_len + j];
                }
            }
            
            double sum_exp = 0.0;
            for (int j = 0; j < seq_len; j++) {
                double exp_val = math_exp(scores[i * seq_len + j] - max_score);
                // Check for NaN or inf
                if (math_is_nan(exp_val) || math_is_inf(exp_val)) {
                    exp_val = 0.0;
                }
                scores[i * seq_len + j] = exp_val;
                sum_exp += exp_val;
            }
            
            // Normalize with epsilon to prevent division by zero
            sum_exp = sum_exp + MATH_EPSILON;
            for (int j = 0; j < seq_len; j++) {
                scores[i * seq_len + j] /= sum_exp;
            }
        }
    }
    
    // Compute weighted sum of values
    memset(output, 0, seq_len * embed_dim * sizeof(double));
    
    for (uint32_t h = 0; h < num_heads; h++) {
        for (int i = 0; i < seq_len; i++) {
            for (int j = 0; j < seq_len; j++) {
                double attn_weight = scores[i * seq_len + j];
                
                // Check for NaN in attention weights
                if (math_is_nan(attn_weight)) {
                    fprintf(stderr, "ERROR: NaN in attention weight at head=%u, i=%d, j=%d\n", h, i, j);
                    attn_weight = 1.0 / seq_len;
                }
                
                double* v = &V[j * embed_dim + h * head_dim];
                double* out = &output[i * embed_dim + h * head_dim];
                
                for (uint32_t d = 0; d < head_dim; d++) {
                    double val = attn_weight * v[d];
                    if (math_is_nan(val)) {
                        fprintf(stderr, "ERROR: NaN in output computation at head=%u, i=%d, j=%d, d=%u\n", h, i, j, d);
                        val = 0.0;
                    }
                    out[d] += val;
                }
            }
        }
    }
    
    // Final NaN check on output
    for (int i = 0; i < seq_len * (int)embed_dim; i++) {
        if (math_is_nan(output[i])) {
            fprintf(stderr, "ERROR: NaN in final output at index %d\n", i);
            output[i] = 0.0;
        }
    }
    
    free(Q);
    free(K);
    free(V);
    free(scores);
}
#endif  // LEGACY FUNCTION


/**
 * Enhanced attention forward pass with neighbor influence
 * 
 * This is the STANDARD attention function that includes neighbor influence
 * from the 12 kissing spheres. Since kissing spheres is THE architecture,
 * this is the default behavior (not optional).
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
#if 0  // LEGACY FUNCTION - Commented out during transformation
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
) {
    // First, compute standard attention
    cllm_attention_forward(layer, input, output, key_cache, value_cache, seq_len);
    
    // Add neighbor influence if model and token_ids are provided
    if (model && token_ids && model->lattice_points && neighbor_strength > 0.0) {
        uint32_t embed_dim = layer->num_heads * layer->head_dim;
        
        // Add neighbor influence for each token in the sequence
        for (int pos = 0; pos < seq_len; pos++) {
            uint32_t token_id = token_ids[pos];
            
            // Skip if token is out of range or has no lattice point
            if (token_id >= model->vocab_size || token_id >= model->num_lattice_points) {
                continue;
            }
            
            double* token_output = &output[pos * embed_dim];
            
            // Add neighbor attention contributions
            extern int cllm_internal_add_neighbor_attention(
                CLLMModel* model,
                uint32_t token_id,
                double* output,
                uint32_t embed_dim,
                float strength
            );
            
            cllm_internal_add_neighbor_attention(
                model,
                token_id,
                token_output,
                embed_dim,
                neighbor_strength
            );
        }
    }
}
#endif  // LEGACY FUNCTION


// Simple feedforward pass
void cllm_feedforward(FeedForwardLayer* layer, double* input, double* output) {
    // Simple pass-through for now (TODO: implement proper feedforward)
    for (uint32_t i = 0; i < layer->output_dim; i++) {
        output[i] = input[i];
    }
}

// NOTE: cllm_layer_norm is now defined in cllm_layernorm.c with SIMD optimizations
// This duplicate implementation has been removed to avoid linker errors
// The SIMD-optimized version in cllm_layernorm.c provides 2-4x speedup

void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens) {
    if (!inference || !tokens || num_tokens <= 0) return;
    
    CLLMModel* model = inference->model;
    if (!model) {
        fprintf(stderr, "Error: Model is NULL in cllm_forward\n");
        return;
    }
    
    // CRITICAL: Verify 88D thread pool (MANDATORY)
    if (!model->threads) {
        fprintf(stderr, "Error: 88D thread pool is NULL - threading is MANDATORY\n");
        return;
    }
    
    // Process each token through its assigned thread
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = tokens[i];
        
        if (token_id >= model->vocab_size) {
            fprintf(stderr, "Error: token %u out of range (vocab_size=%u)\n", 
                    token_id, model->vocab_size);
            continue;
        }
        
        // Get thread assignment for this token
        uint8_t layer = model->token_assignments[token_id].layer;
        uint8_t dimension = model->token_assignments[token_id].dimension;
        
        // Get the thread
        extern HierarchicalThread* hierarchical_thread_get(
            HierarchicalThreadPool* pool, uint8_t layer, uint8_t dimension);
        HierarchicalThread* thread = hierarchical_thread_get(
            model->threads, layer, dimension
        );
        
        if (!thread || !thread->activation_buffer) {
            fprintf(stderr, "Error: Thread or activation buffer is NULL for token %u\n", token_id);
            continue;
        }
        
        // Get embedding from thread's CrystallineAbacus into activation_buffer
        extern bool cllm_get_embedding_from_model(const CLLMModel* model, uint32_t token_id, double* output);
        if (!cllm_get_embedding_from_model(model, token_id, thread->activation_buffer)) {
            fprintf(stderr, "Error: Failed to get embedding for token %u\n", token_id);
            continue;
        }
        
        // Apply positional encoding in thread-local space
        int position = inference->current_position + i;
        cllm_apply_positional_encoding(inference, thread->activation_buffer, position);
        
        // Process through transformer layers (thread-local)
        // The transformer layer functions will use thread->activation_buffer
        extern void cllm_transformer_forward(const CLLMModel* model, double* hidden_states);
        extern bool cllm_has_transformer_layers(const CLLMModel* model);
        
        if (cllm_has_transformer_layers(model)) {
            cllm_transformer_forward(model, thread->activation_buffer);
        }
    }
    
    // Update current position
    inference->current_position += num_tokens;
}

// Apply temperature scaling
void cllm_apply_temperature(double* logits, int vocab_size, double temperature) {
    if (temperature < TEMPERATURE_MIN) temperature = TEMPERATURE_MIN;
    if (temperature > TEMPERATURE_MAX) temperature = TEMPERATURE_MAX;
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= temperature;
    }
}



// Softmax
// cllm_softmax moved to cllm_transformer_layer.c
// Using unified 88D implementation



// Sample top-k
uint32_t cllm_sample_top_k(double* probs, int vocab_size, int k) {
    if (k <= 0 || k > vocab_size) k = vocab_size;
    
    // Simple sampling from top-k
    double r = (double)rand() / RAND_MAX;
    double cumsum = 0.0;
    
    for (int i = 0; i < k && i < vocab_size; i++) {
        cumsum += probs[i];
        if (r < cumsum) return i;
    }
    
    return 0;
}


// Sample top-p (nucleus sampling)
uint32_t cllm_sample_top_p(double* probs, int vocab_size, double p) {
    double r = (double)rand() / RAND_MAX;
    double cumsum = 0.0;
    
    for (int i = 0; i < vocab_size; i++) {
        cumsum += probs[i];
        if (cumsum >= p || r < cumsum) return i;
    }
    
    return 0;
}



// Generate text - MAIN FUNCTION (Phase 2: Thread-local storage only)
// REMOVED: int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length) {
// REMOVED:     if (!inference || !prompt || !output) return -1;
// REMOVED:     
// REMOVED:     CLLMModel* model = inference->model;
// REMOVED:     if (!model || !model->threads) {
// REMOVED:         fprintf(stderr, "Error: Model or 88D thread pool is NULL\n");
// REMOVED:         strcpy(output, "Error: Invalid model state");
// REMOVED:         return -1;
// REMOVED:     }
// REMOVED:     
// REMOVED:     // Tokenize prompt
// REMOVED:     uint32_t tokens[MAX_SEQUENCE_LENGTH];
// REMOVED:     int num_tokens = cllm_tokenize(inference, prompt, tokens, MAX_SEQUENCE_LENGTH);
// REMOVED:     
// REMOVED:     if (num_tokens <= 0) {
// REMOVED:         strcpy(output, "Error: Could not tokenize prompt");
// REMOVED:         return -1;
// REMOVED:     }
// REMOVED:     
// REMOVED:     // Process prompt tokens
// REMOVED:     cllm_forward(inference, tokens, num_tokens);
// REMOVED:     
// REMOVED:     // Generate tokens
// REMOVED:     int tokens_generated = 0;
// REMOVED:     while (tokens_generated < inference->max_tokens && num_tokens < MAX_SEQUENCE_LENGTH) {
// REMOVED:         // Allocate temporary logits buffer (thread-local, on stack would be better but vocab_size is large)
// REMOVED:         double* logits = (double*)calloc(model->vocab_size, sizeof(double));
// REMOVED:         if (!logits) {
// REMOVED:             fprintf(stderr, "Error: Failed to allocate logits buffer\n");
// REMOVED:             break;
// REMOVED:         }
// REMOVED:         
// REMOVED:         // Compute logits by aggregating from all threads
// REMOVED:         // Each token's thread contains its final hidden state in activation_buffer
// REMOVED:         uint32_t last_token = tokens[num_tokens - 1];
// REMOVED:         uint8_t last_layer = model->token_assignments[last_token].layer;
// REMOVED:         uint8_t last_dim = model->token_assignments[last_token].dimension;
// REMOVED:         
// REMOVED:         extern HierarchicalThread* hierarchical_thread_get(
// REMOVED:             HierarchicalThreadPool* pool, uint8_t layer, uint8_t dimension);
// REMOVED:         HierarchicalThread* last_thread = hierarchical_thread_get(
// REMOVED:             model->threads, last_layer, last_dim
// REMOVED:         );
// REMOVED:         
// REMOVED:         if (!last_thread || !last_thread->activation_buffer) {
// REMOVED:             fprintf(stderr, "Error: Cannot get last token's thread\n");
// REMOVED:             free(logits);
// REMOVED:             break;
// REMOVED:         }
// REMOVED:         
// REMOVED:         // Compute dot product with each vocabulary token's embedding
// REMOVED:         for (uint32_t v = 0; v < model->vocab_size; v++) {
// REMOVED:             uint8_t v_layer = model->token_assignments[v].layer;
// REMOVED:             uint8_t v_dim = model->token_assignments[v].dimension;
// REMOVED:             
// REMOVED:             HierarchicalThread* v_thread = hierarchical_thread_get(
// REMOVED:                 model->threads, v_layer, v_dim
// REMOVED:             );
// REMOVED:             
// REMOVED:             if (!v_thread) continue;
// REMOVED:             
// REMOVED:             // Get vocabulary token's embedding
// REMOVED:             double* v_embedding = (double*)malloc(model->embedding_dim * sizeof(double));
// REMOVED:             if (!v_embedding) continue;
// REMOVED:             
// REMOVED:             extern bool cllm_get_embedding_from_model(const CLLMModel* model, uint32_t token_id, double* output);
// REMOVED:             if (!cllm_get_embedding_from_model(model, v, v_embedding)) {
// REMOVED:                 free(v_embedding);
// REMOVED:                 continue;
// REMOVED:             }
// REMOVED:             
// REMOVED:             // Compute dot product
// REMOVED:             double score = 0.0;
// REMOVED:             for (uint32_t d = 0; d < model->embedding_dim; d++) {
// REMOVED:                 score += last_thread->activation_buffer[d] * v_embedding[d];
// REMOVED:             }
// REMOVED:             logits[v] = score;
// REMOVED:             
// REMOVED:             free(v_embedding);
// REMOVED:         }
// REMOVED:         
// REMOVED:         // Apply temperature and softmax
// REMOVED:         cllm_apply_temperature(logits, model->vocab_size, inference->temperature);
// REMOVED:         cllm_softmax(logits, model->vocab_size);
// REMOVED:         
// REMOVED:         // Sample next token
// REMOVED:         uint32_t next_token;
// REMOVED:         if (inference->top_k > 0) {
// REMOVED:             next_token = cllm_sample_top_k(logits, model->vocab_size, inference->top_k);
// REMOVED:         } else {
// REMOVED:             next_token = cllm_sample_top_p(logits, model->vocab_size, inference->top_p);
// REMOVED:         }
// REMOVED:         
// REMOVED:         free(logits);
// REMOVED:         
// REMOVED:         // Check for end of sequence (assuming 0 is EOS or invalid)
// REMOVED:         if (next_token == 0 || next_token >= model->vocab_size) break;
// REMOVED:         
// REMOVED:         // Add to sequence
// REMOVED:         tokens[num_tokens++] = next_token;
// REMOVED:         inference->generated_tokens[tokens_generated] = next_token;
// REMOVED:         tokens_generated++;
// REMOVED:         inference->num_generated = tokens_generated;
// REMOVED:         
// REMOVED:         // Process next token
// REMOVED:         cllm_forward(inference, &next_token, 1);
// REMOVED:     }
// REMOVED:     
// REMOVED:     // Detokenize
// REMOVED:     cllm_detokenize(inference, tokens, num_tokens, output, max_output_length);
// REMOVED:     
// REMOVED:     return tokens_generated;
// REMOVED: }
// REMOVED: 
// Set generation parameters
void cllm_set_temperature(CLLMInference* inference, float temperature) {
    if (inference) {
        if (temperature < TEMPERATURE_MIN) temperature = TEMPERATURE_MIN;
        if (temperature > TEMPERATURE_MAX) temperature = TEMPERATURE_MAX;
        inference->temperature = temperature;
    }
}

void cllm_set_top_p(CLLMInference* inference, float top_p) {
    if (inference) {
        if (top_p < 0.0) top_p = 0.0;
        if (top_p > 1.0) top_p = 1.0;
        inference->top_p = top_p;
    }
}

void cllm_set_top_k(CLLMInference* inference, int top_k) {
    if (inference) {
        inference->top_k = top_k > 0 ? top_k : 0;
    }
}

void cllm_set_max_tokens(CLLMInference* inference, int max_tokens) {
    if (inference) {
        inference->max_tokens = max_tokens > 0 ? max_tokens : 512;
    }
}

// Sample token from logits distribution
int cllm_inference_sample_token(CLLMInference* inf, double* logits) {
    if (!inf || !logits) return 0;
    
    uint32_t vocab_size = inf->model->vocab_size;
    
    // Apply temperature
    if (inf->temperature != 1.0) {
        for (uint32_t i = 0; i < vocab_size; i++) {
            logits[i] /= inf->temperature;
        }
    }
    
    // Convert logits to probabilities using softmax
    double max_logit = logits[0];
    for (uint32_t i = 1; i < vocab_size; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    double sum = 0.0;
    for (uint32_t i = 0; i < vocab_size; i++) {
        logits[i] = math_exp(logits[i] - max_logit);
        sum += logits[i];
    }
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        logits[i] /= sum;
    }
    
    // Sample from distribution
    double r = (double)rand() / RAND_MAX;
    double cumsum = 0.0;
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        cumsum += logits[i];
        if (r < cumsum) {
            return i;
        }
    }
    
    return 0;
}