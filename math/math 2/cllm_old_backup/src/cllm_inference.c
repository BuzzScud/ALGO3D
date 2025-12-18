#include "cllm_inference.h"
#include "cllm.h"
#include "math/transcendental.h"
#include "prime_lattice_core.h"  // For theta_n() angular position
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/transcendental.h"

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
    
    // Allocate working memory using double*
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Use standard malloc for double* arrays
    inference->hidden_states = (double*)calloc(embed_dim, sizeof(double));
    inference->logits = (double*)calloc(vocab_size, sizeof(double));
    
    if (!inference->hidden_states || !inference->logits) {
        fprintf(stderr, "Error: Failed to allocate inference buffers\n");
        cllm_inference_cleanup(inference);
        return NULL;
    }
    
    printf("Inference context initialized successfully (double precision)\n");
    return inference;
}

// Cleanup inference context
void cllm_inference_cleanup(CLLMInference* inference) {
    if (!inference) return;
    
    // Use standard free for double* arrays
    if (inference->hidden_states) {
        free(inference->hidden_states);
    }
    if (inference->logits) {
        free(inference->logits);
    }
    if (inference->key_cache) {
        free(inference->key_cache);
    }
    if (inference->value_cache) {
        free(inference->value_cache);
    }
    
    free(inference);
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
    
    double* embedding = &model->embeddings[token_id * embed_dim];
    
    // Lazy initialization: compute embedding on first access
    if (math_is_nan(embedding[0])) {  // FIXED: Use double version for double*
        extern void cllm_compute_embedding_lazy(CLLMModel* model, uint32_t token_id);
        cllm_compute_embedding_lazy(model, token_id);
    }
    
    memcpy(output, embedding, embed_dim * sizeof(float));
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
    
    // Add positional encoding if available
    if (model->positional_encoding) {
        double* pos_enc = &model->positional_encoding[position * embed_dim];
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
    
    uint32_t embed_dim = model->embedding_dim;
    
    // Check critical pointers
    if (!inference->hidden_states) {
        fprintf(stderr, "Error: hidden_states is NULL\n");
        return;
    }
    if (!inference->logits) {
        fprintf(stderr, "Error: logits is NULL\n");
        return;
    }
    if (!model->embeddings) {
        fprintf(stderr, "Error: embeddings is NULL\n");
        return;
    }
    
    // Get embedding for last token
    uint32_t last_token = tokens[num_tokens - 1];
    if (last_token >= model->vocab_size) {
        fprintf(stderr, "Error: token %u out of range (vocab_size=%lu)\n", last_token, (unsigned long)model->vocab_size);
        return;
    }
    
    // Copy double embedding to double hidden_states
    double* double_embedding = &model->embeddings[last_token * embed_dim];
    
    // CRITICAL FIX: Check for NaN embeddings and trigger lazy initialization
    if (math_is_nan(double_embedding[0])) {
        fprintf(stderr, "Warning: Embedding for token %u is NaN, triggering lazy initialization\n", last_token);
        extern void cllm_compute_embedding_lazy(CLLMModel* model, uint32_t token_id);
        cllm_compute_embedding_lazy(model, last_token);
    }
    
    for (uint32_t i = 0; i < embed_dim; i++) {
        inference->hidden_states[i] = double_embedding[i];
    }
    
    // Apply positional encoding
    cllm_apply_positional_encoding(inference, inference->hidden_states, num_tokens - 1);
    
    // TODO: Reimplement transformer layers using new CLLMModel structure
    // The new structure has layers as an array with query_weights, key_weights, value_weights, etc.
    // Need to integrate with cllm_attention.c (which has NTT support)
    #if 0  // LEGACY TRANSFORMER LOOP - Needs reimplementation
    // Pass through transformer layers using double precision throughout
    if (model->attention_layers && model->ff_layers && model->layer_norms) {
        // Allocate attention output buffer
        double* attn_output = (double*)calloc(embed_dim, sizeof(double));
        if (!attn_output) {
            fprintf(stderr, "Error: Failed to allocate attention output buffer\n");
            return;
        }
        
        for (uint32_t layer = 0; layer < model->num_layers; layer++) {
            // Layer norm (in-place) - now uses double
            cllm_layer_norm(&model->layer_norms[layer], inference->hidden_states, inference->hidden_states);
            
            // Attention - uses double
            AttentionLayer* attn_layer = &model->attention_layers[layer];
            cllm_attention_forward(attn_layer, inference->hidden_states, inference->hidden_states, NULL, NULL, 1);
            
            // Feed-forward (in-place) - uses double
            cllm_feedforward(&model->ff_layers[layer], inference->hidden_states, inference->hidden_states);
        }
        
        // Final layer norm
        cllm_layer_norm(&model->layer_norms[model->num_layers - 1], inference->hidden_states, inference->hidden_states);
        
        free(attn_output);
    }
    #endif  // LEGACY TRANSFORMER LOOP
    
    // Project to vocabulary - compute logits
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        double logit_value = 0.0;
        double* token_embed = &model->embeddings[i * embed_dim];
        for (uint32_t j = 0; j < embed_dim; j++) {
            logit_value += inference->hidden_states[j] * (double)token_embed[j];
        }
        inference->logits[i] = logit_value;
    }
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
void cllm_softmax(double* logits, int vocab_size) {
    // Find max for numerical stability
    double max_logit = logits[0];
    for (int i = 1; i < vocab_size; i++) {
        if (logits[i] > max_logit) max_logit = logits[i];
    }
    
    // Compute exp and sum
    double sum = 0.0;
    for (int i = 0; i < vocab_size; i++) {
        logits[i] = math_exp(logits[i] - max_logit);
        sum += logits[i];
    }
    
    // Normalize
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= sum;
    }
}



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



// Generate text - MAIN FUNCTION
int cllm_generate(CLLMInference* inference, const char* prompt, char* output, int max_output_length) {
    if (!inference || !prompt || !output) return -1;
    
    // Silent generation - no terminal spam
    
    // Tokenize prompt
    uint32_t tokens[MAX_SEQUENCE_LENGTH];
    int num_tokens = cllm_tokenize(inference, prompt, tokens, MAX_SEQUENCE_LENGTH);
    
    if (num_tokens <= 0) {
        strcpy(output, "Error: Could not tokenize prompt");
        return -1;
    }
    
    // Generate tokens
    int tokens_generated = 0;
    while (tokens_generated < inference->max_tokens && num_tokens < MAX_SEQUENCE_LENGTH) {
        // Forward pass
        cllm_forward(inference, tokens, num_tokens);
        
        // Apply temperature
        cllm_apply_temperature(inference->logits, inference->model->vocab_size, inference->temperature);
        
        // Softmax
        cllm_softmax(inference->logits, inference->model->vocab_size);
        
        // Sample next token
        uint32_t next_token;
        if (inference->top_k > 0) {
            next_token = cllm_sample_top_k(inference->logits, inference->model->vocab_size, inference->top_k);
        } else {
            next_token = cllm_sample_top_p(inference->logits, inference->model->vocab_size, inference->top_p);
        }
        
        // Add to sequence
        tokens[num_tokens++] = next_token;
        tokens_generated++;
        
        // Silent generation - no terminal spam
    }
    
    // Detokenize
    cllm_detokenize(inference, tokens, num_tokens, output, max_output_length);
    
    // Silent generation - no terminal spam
    return tokens_generated;
}

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
int cllm_sample_token(CLLMInference* inf, double* logits) {
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
