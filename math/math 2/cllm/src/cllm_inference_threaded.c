/**
 * Thread-Local Inference Implementation (Day 12)
 * 
 * This file implements inference using the 88D thread pool and work queue system.
 * All computation happens in thread-local CrystallineAbacus storage.
 * 
 * Key insight: Inference IS just forward pass without backward pass!
 * We reuse the same worker_process_forward() infrastructure from training.
 */

#include "cllm_inference.h"
#include "cllm.h"
#include "hierarchical_threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Signal all threads to start processing work
 */
static void signal_all_threads(HierarchicalThreadPool* pool) {
    if (!pool) return;
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 0; dim < 12; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
}

/**
 * Wait for all threads to complete their work
 */
static void wait_for_completion(HierarchicalThreadPool* pool) {
    if (!pool) return;
    
    // Wait on global barrier for all threads to finish
    pthread_barrier_wait(&pool->global_barrier);
}

/**
 * Sample token from probability distribution using top-k sampling
 */
static uint32_t sample_top_k(const double* probs, uint32_t vocab_size, int k) {
    if (!probs || k <= 0) return 0;
    
    // Create array of (index, prob) pairs
    typedef struct {
        uint32_t index;
        double prob;
    } IndexProb;
    
    IndexProb* pairs = (IndexProb*)malloc(vocab_size * sizeof(IndexProb));
    if (!pairs) return 0;
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        pairs[i].index = i;
        pairs[i].prob = probs[i];
    }
    
    // Sort by probability (descending) - simple bubble sort for now
    for (uint32_t i = 0; i < vocab_size - 1; i++) {
        for (uint32_t j = 0; j < vocab_size - i - 1; j++) {
            if (pairs[j].prob < pairs[j + 1].prob) {
                IndexProb temp = pairs[j];
                pairs[j] = pairs[j + 1];
                pairs[j + 1] = temp;
            }
        }
    }
    
    // Keep only top k
    int actual_k = (k < (int)vocab_size) ? k : (int)vocab_size;
    
    // Renormalize top k probabilities
    double sum = 0.0;
    for (int i = 0; i < actual_k; i++) {
        sum += pairs[i].prob;
    }
    
    // Sample from top k
    double r = ((double)rand() / RAND_MAX) * sum;
    double cumsum = 0.0;
    uint32_t selected = pairs[0].index;
    
    for (int i = 0; i < actual_k; i++) {
        cumsum += pairs[i].prob;
        if (r <= cumsum) {
            selected = pairs[i].index;
            break;
        }
    }
    
    free(pairs);
    return selected;
}

/**
 * Sample token from probability distribution using top-p (nucleus) sampling
 */
static uint32_t sample_top_p(const double* probs, uint32_t vocab_size, double p) {
    if (!probs || p <= 0.0 || p > 1.0) return 0;
    
    // Create array of (index, prob) pairs
    typedef struct {
        uint32_t index;
        double prob;
    } IndexProb;
    
    IndexProb* pairs = (IndexProb*)malloc(vocab_size * sizeof(IndexProb));
    if (!pairs) return 0;
    
    for (uint32_t i = 0; i < vocab_size; i++) {
        pairs[i].index = i;
        pairs[i].prob = probs[i];
    }
    
    // Sort by probability (descending)
    for (uint32_t i = 0; i < vocab_size - 1; i++) {
        for (uint32_t j = 0; j < vocab_size - i - 1; j++) {
            if (pairs[j].prob < pairs[j + 1].prob) {
                IndexProb temp = pairs[j];
                pairs[j] = pairs[j + 1];
                pairs[j + 1] = temp;
            }
        }
    }
    
    // Find cutoff where cumulative probability exceeds p
    double cumsum = 0.0;
    int cutoff = 0;
    for (uint32_t i = 0; i < vocab_size; i++) {
        cumsum += pairs[i].prob;
        cutoff = i + 1;
        if (cumsum >= p) break;
    }
    
    // Renormalize
    double sum = 0.0;
    for (int i = 0; i < cutoff; i++) {
        sum += pairs[i].prob;
    }
    
    // Sample
    double r = ((double)rand() / RAND_MAX) * sum;
    cumsum = 0.0;
    uint32_t selected = pairs[0].index;
    
    for (int i = 0; i < cutoff; i++) {
        cumsum += pairs[i].prob;
        if (r <= cumsum) {
            selected = pairs[i].index;
            break;
        }
    }
    
    free(pairs);
    return selected;
}

// ============================================================================
// MAIN INFERENCE FUNCTIONS
// ============================================================================

/**
 * Generate a single token using thread-local computation
 * 
 * This function:
 * 1. Enqueues forward work for all context tokens
 * 2. Signals threads and waits for completion
 * 3. Collects logits from Layer 7 threads
 * 4. Applies temperature and softmax
 * 5. Samples next token
 * 
 * @param inference Inference context
 * @param context Array of context token IDs
 * @param context_len Number of tokens in context
 * @return Next token ID, or 0 on error
 */
uint32_t cllm_generate_token(CLLMInference* inference, 
                                       const uint32_t* context,
                                       int context_len) {
    if (!inference || !context || context_len <= 0) {
        fprintf(stderr, "ERROR: Invalid parameters for cllm_generate_token\n");
        return 0;
    }
    
    CLLMModel* model = inference->model;
    HierarchicalThreadPool* pool = model->threads;
    
    if (!pool) {
        fprintf(stderr, "FATAL: Cannot generate without 88D thread pool\n");
        return 0;
    }
    
    // ========================================================================
    // STEP 1: FORWARD PASS (Process all context tokens)
    // ========================================================================
    
    // Enqueue forward work for all context tokens
    for (int i = 0; i < context_len; i++) {
        uint32_t token_id = context[i];
        
        if (token_id >= model->vocab_size) {
            fprintf(stderr, "ERROR: Invalid token_id %u (vocab_size=%u)\n",
                    token_id, model->vocab_size);
            continue;
        }
        
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (thread) {
            hierarchical_thread_enqueue_work(thread, TRAINING_WORK_TYPE_FORWARD,
                                            token_id, 0);
        }
    }
    
    // Signal threads and wait for completion
    signal_all_threads(pool);
    wait_for_completion(pool);
    
    // ========================================================================
    // STEP 2: COLLECT LOGITS FROM LAYER 7
    // ========================================================================
    
    // Allocate logits array
    double* logits = (double*)calloc(model->vocab_size, sizeof(double));
    if (!logits) {
        fprintf(stderr, "ERROR: Failed to allocate logits array\n");
        return 0;
    }
    
    // Collect logits from Layer 7 threads
    if (collect_logits_from_layer7(pool, logits, model->vocab_size) != 0) {
        fprintf(stderr, "ERROR: Failed to collect logits from Layer 7\n");
        free(logits);
        return 0;
    }
    
    // ========================================================================
    // STEP 3: APPLY TEMPERATURE AND SOFTMAX
    // ========================================================================
    
    // Apply temperature scaling
    if (apply_temperature_to_logits(logits, model->vocab_size, 
                                     inference->temperature) != 0) {
        fprintf(stderr, "ERROR: Failed to apply temperature\n");
        free(logits);
        return 0;
    }
    
    // Apply softmax to get probabilities
    if (apply_softmax_to_logits(logits, model->vocab_size) != 0) {
        fprintf(stderr, "ERROR: Failed to apply softmax\n");
        free(logits);
        return 0;
    }
    
    // ========================================================================
    // STEP 4: SAMPLE NEXT TOKEN
    // ========================================================================
    
    // Sample next token (logits are now probabilities after softmax)
    uint32_t next_token;
    if (inference->top_k > 0) {
        next_token = sample_top_k(logits, model->vocab_size, inference->top_k);
    } else {
        next_token = sample_top_p(logits, model->vocab_size, inference->top_p);
    }
    
    // Cleanup
    free(logits);
    
    return next_token;
}

/**
 * Generate text using thread-local computation
 * 
 * This function:
 * 1. Tokenizes the prompt
 * 2. Generates tokens one by one using cllm_generate_token()
 * 3. Detokenizes and returns the generated text
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
                           int max_output_length) {
    if (!inference || !prompt || !output) {
        fprintf(stderr, "ERROR: Invalid parameters for cllm_generate\n");
        return -1;
    }
    
    CLLMModel* model = inference->model;
    if (!model) {
        fprintf(stderr, "ERROR: Model is NULL\n");
        strcpy(output, "Error: Invalid model state");
        return -1;
    }
    
    // Threading is ALWAYS enabled in 88D architecture
    
    // ========================================================================
    // STEP 1: TOKENIZE PROMPT
    // ========================================================================
    
    #define MAX_SEQUENCE_LENGTH 512
    uint32_t tokens[MAX_SEQUENCE_LENGTH];
    
    // Use existing tokenization function
    extern int cllm_tokenize(CLLMInference* inference, const char* text, 
                            uint32_t* tokens, int max_tokens);
    int num_tokens = cllm_tokenize(inference, prompt, tokens, MAX_SEQUENCE_LENGTH);
    
    if (num_tokens <= 0) {
        strcpy(output, "Error: Could not tokenize prompt");
        return -1;
    }
    
    // ========================================================================
    // STEP 2: GENERATE TOKENS
    // ========================================================================
    
    int tokens_generated = 0;
    while (tokens_generated < inference->max_tokens && 
           num_tokens < MAX_SEQUENCE_LENGTH) {
        
        // Generate next token
        uint32_t next_token = cllm_generate_token(inference, tokens, num_tokens);
        
        // Check for end of sequence or invalid token
        if (next_token == 0 || next_token >= model->vocab_size) {
            break;
        }
        
        // Add to sequence
        tokens[num_tokens++] = next_token;
        inference->generated_tokens[tokens_generated] = next_token;
        tokens_generated++;
        inference->num_generated = tokens_generated;
    }
    
    // ========================================================================
    // STEP 3: DETOKENIZE
    // ========================================================================
    
    // Use existing detokenization function
    extern void cllm_detokenize(CLLMInference* inference, uint32_t* tokens,
                               int num_tokens, char* output, int max_length);
    cllm_detokenize(inference, tokens, num_tokens, output, max_output_length);
    
    return tokens_generated;
}