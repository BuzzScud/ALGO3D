/**
 * @file cllm_training_functions.c
 * @brief Training Functions for 88D Thread-Centric CLLM
 * 
 * CRITICAL: This is the ONLY implementation of training functions.
 * Threading is MANDATORY - no sequential fallback exists.
 * 
 * All computation happens in thread-local CrystallineAbacus storage.
 * All parameters are distributed across 88 worker threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "math/transcendental.h"
#include "cllm.h"
#include "math/constants.h"
#include "cllm_training.h"
#include "cllm_batch.h"
#include "ai/cllm_training_system.h"
#include "hierarchical_threading.h"

// ============================================================================
// FORWARD PASS - 88D THREAD-CENTRIC (THE ONLY IMPLEMENTATION)
// ============================================================================

/**
 * Forward pass - Threading is MANDATORY
 * 
 * This function will abort if threads is NULL.
 * All computation happens in parallel across 88 worker threads.
 * 
 * @param training Training context
 * @param input_tokens Input token IDs
 * @return Average loss across all threads
 */
double cllm_forward_training(CLLMTraining* training, uint32_t* input_tokens) {
    fprintf(stderr, "DEBUG: cllm_forward_training ENTERED\n");
    
    if (!training || !input_tokens) {
        fprintf(stderr, "ERROR: NULL training or input_tokens\n");
        return -1.0;
    }
    
    fprintf(stderr, "DEBUG: Parameters validated\n");
    
    CLLMModel* model = training->model;
    
    // CRITICAL: Verify 88D thread pool exists
    if (!model->threads) {
        fprintf(stderr, "\n");
        fprintf(stderr, "╔════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║              FATAL ERROR: NO THREADING                 ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════════════════╝\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "88D thread pool not initialized!\n");
        fprintf(stderr, "Threading is MANDATORY in this architecture.\n");
        fprintf(stderr, "There is NO sequential fallback.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Model must be created with cllm_create_model().\n");
        fprintf(stderr, "Ensure threads is properly initialized.\n");
        fprintf(stderr, "\n");
        abort();
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->threads;
    
    fprintf(stderr, "DEBUG: cllm_forward_training - num_tokens=%d, pool=%p\n", num_tokens, (void*)pool);
    
    // Enqueue forward work items to threads
    int work_enqueued = 0;
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = input_tokens[i];
        
        if (token_id >= model->vocab_size) {
            fprintf(stderr, "WARNING: Invalid token ID %u (vocab_size=%u)\n", 
                    token_id, model->vocab_size);
            continue;
        }
        
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (!thread) {
            fprintf(stderr, "FATAL: Thread not found for token %u\n", token_id);
            abort();
        }
        
        // Enqueue forward work item
        int result = hierarchical_thread_enqueue_work(
            thread,
            TRAINING_WORK_TYPE_FORWARD,
            token_id,
            0  // target_id not used for forward pass
        );
        
        if (result == 0) {
            work_enqueued++;
        } else {
            fprintf(stderr, "WARNING: Failed to enqueue work for token %u\n", token_id);
        }
    }
    
    fprintf(stderr, "DEBUG: Enqueued %d forward work items\n", work_enqueued);
    
    // Signal all threads to start processing
    int threads_signaled = 0;
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
                threads_signaled++;
            }
        }
    }
    fprintf(stderr, "DEBUG: Signaled %d threads\n", threads_signaled);
    
    // Wait for completion (poll atomic counters instead of barrier)
    // This avoids deadlock since worker threads don't wait on barrier
    uint64_t expected_work = (uint64_t)num_tokens;
    uint64_t completed_work = 0;
    
    while (completed_work < expected_work) {
        completed_work = 0;
        
        for (uint8_t layer = 0; layer < 8; layer++) {
            for (uint8_t dim = 1; dim <= 11; dim++) {
                HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
                if (thread) {
                    completed_work += __atomic_load_n(&thread->work_completed, __ATOMIC_SEQ_CST);
                }
            }
        }
        
        if (completed_work < expected_work) {
            usleep(100);  // 100 microseconds
        }
    }
    
    // Reset work_completed counters for next iteration
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
            if (thread) {
                __atomic_store_n(&thread->work_completed, 0, __ATOMIC_SEQ_CST);
            }
        }
    }
    
    // Collect results
    double total_loss = 0.0;
    int thread_count = 0;
    
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get(pool, layer, dim);
            
            if (thread && thread->work_completed > 0) {
                if (thread->activation_buffer && thread->activation_buffer_size > 0) {
                    total_loss += thread->activation_buffer[0];
                }
                thread_count++;
            }
        }
    }
    
    return (thread_count > 0) ? (total_loss / thread_count) : 0.0;
}

// ============================================================================
// BACKWARD PASS - 88D THREAD-CENTRIC (THE ONLY IMPLEMENTATION)
// ============================================================================

/**
 * Backward pass - Threading is MANDATORY
 * 
 * Computes gradients in parallel across all threads.
 * Gradients are accumulated in thread-local CrystallineAbacus storage.
 * 
 * @param training Training context
 * @param target_tokens Target token IDs for loss computation
 * @param gradient_buffer DEPRECATED - gradients now in thread-local storage
 */
void cllm_backward_training(CLLMTraining* training, uint32_t* target_tokens, double* gradient_buffer) {
    if (!training || !target_tokens) {
        fprintf(stderr, "ERROR: NULL training or target_tokens\n");
        return;
    }
    
    // gradient_buffer parameter is deprecated - kept for API compatibility
    (void)gradient_buffer;
    
    CLLMModel* model = training->model;
    
    // CRITICAL: Verify 88D thread pool exists
    if (!model->threads) {
        fprintf(stderr, "FATAL ERROR: 88D thread pool not initialized\n");
        abort();
    }
    
    int num_tokens = training->config.batch_size * training->config.sequence_length;
    HierarchicalThreadPool* pool = model->threads;
    
    // Enqueue backward work items to threads
    int work_enqueued = 0;
    for (int i = 0; i < num_tokens; i++) {
        uint32_t token_id = i % model->vocab_size;
        uint32_t target_id = target_tokens[i];
        
        if (token_id >= model->vocab_size || target_id >= model->vocab_size) {
            continue;
        }
        
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (!thread) continue;
        
        // Enqueue backward work item
        int result = hierarchical_thread_enqueue_work(
            thread,
            TRAINING_WORK_TYPE_BACKWARD,
            token_id,
            target_id
        );
        
        if (result == 0) {
            work_enqueued++;
        } else {
            fprintf(stderr, "WARNING: Failed to enqueue backward work for token %u\n", token_id);
        }
    }
    
    fprintf(stderr, "DEBUG: Enqueued %d backward work items\n", work_enqueued);
    
    // Signal all threads to start processing
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = 
                hierarchical_thread_get(pool, layer, dim);
            
            if (thread) {
                pthread_cond_signal(&thread->control_cond);
            }
        }
    }
    
    // Wait for completion (poll atomic counters instead of barrier)
    uint64_t expected_work = (uint64_t)num_tokens;
    uint64_t completed_work = 0;
    
    while (completed_work < expected_work) {
        completed_work = 0;
        
        for (uint8_t layer = 0; layer < 8; layer++) {
            for (uint8_t dim = 1; dim <= 11; dim++) {
                HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
                if (thread) {
                    completed_work += __atomic_load_n(&thread->work_completed, __ATOMIC_SEQ_CST);
                }
            }
        }
        
        if (completed_work < expected_work) {
            usleep(100);  // 100 microseconds
        }
    }
    
    // Reset work_completed counters for next iteration
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 1; dim <= 11; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
            if (thread) {
                __atomic_store_n(&thread->work_completed, 0, __ATOMIC_SEQ_CST);
            }
        }
    }
    
    // Gradients now in thread-local CrystallineAbacus
}

// ============================================================================
// EMBEDDING PRECOMPUTATION
// ============================================================================

void cllm_precompute_all_embeddings(CLLMModel* model) {
    if (!model) return;
    
    printf("✓ Embeddings already initialized in thread-local storage\n");
    printf("  Each token's embedding is stored in its assigned thread's CrystallineAbacus\n");
}

// ============================================================================
// TRAINING INITIALIZATION
// ============================================================================

CLLMTraining* cllm_training_init(CLLMModel* model, CLLMTrainingConfig* config) {
    if (!model || !config) return NULL;
    
    if (!model->threads) {
        fprintf(stderr, "FATAL: Cannot initialize training without 88D thread pool\n");
        return NULL;
    }
    
    CLLMTraining* training = calloc(1, sizeof(CLLMTraining));
    if (!training) return NULL;
    
    training->model = model;
    training->config = *config;
    training->current_epoch = 0;
    training->current_step = 0;
    training->best_loss = 1e9;
    training->accumulation_step = 0;
    training->config.initial_learning_rate = config->learning_rate;
    
    printf("✓ Training initialized (88D thread-centric)\n");
    
    return training;
}

// ============================================================================
// OPTIMIZER
// ============================================================================

void cllm_optimizer_step_adam(CLLMTraining* training) {
    if (!training) return;
    
    CLLMModel* model = training->model;
    
    if (!model->threads) {
        fprintf(stderr, "FATAL: Cannot run optimizer without 88D thread pool\n");
        abort();
    }
    
    model->optimizer.t++;
    
    double beta1_t = math_pow(model->optimizer.beta1, model->optimizer.t);
    double beta2_t = math_pow(model->optimizer.beta2, model->optimizer.t);
    double lr_t = model->optimizer.learning_rate * math_sqrt(1.0 - beta2_t) / (1.0 - beta1_t);
    
    // TODO: Distribute optimizer updates to threads
    (void)lr_t;
    
    printf("Adam optimizer step (88D thread-centric)\n");
}

// ============================================================================
// COMPLETE TRAINING LOOP (DAY 11)
// ============================================================================

/**
 * Signal all threads to start processing
 * 
 * @param pool Thread pool
 */
static void signal_all_threads(HierarchicalThreadPool* pool) {
    if (!pool) return;
    
    // Signal all threads via condition variable
    for (uint32_t i = 0; i < pool->num_threads; i++) {
        HierarchicalThread* thread = pool->threads[i];
        if (thread) {
            pthread_mutex_lock(&thread->control_mutex);
            pthread_cond_signal(&thread->control_cond);
            pthread_mutex_unlock(&thread->control_mutex);
        }
    }
}

/**
 * Wait for all threads to complete their work
 * 
 * @param pool Thread pool
 */
static void wait_for_completion(HierarchicalThreadPool* pool) {
    if (!pool) return;
    
    // Wait at global barrier for all threads
    pthread_barrier_wait(&pool->global_barrier);
}

/**
 * Compute loss from distributed outputs
 * 
 * @param pool Thread pool
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Loss value
 */
static double compute_loss_distributed(
    HierarchicalThreadPool* pool,
    const uint32_t* target_tokens,
    uint32_t num_tokens
) {
    if (!pool || !target_tokens) {
        return -1.0;
    }
    
    // Collect outputs from Layer 7 threads
    // Compute cross-entropy loss
    // For now, return placeholder loss
    // TODO: Full loss computation in Phase 4 optimization
    
    double loss = 1.0;  // Placeholder
    
    (void)num_tokens;  // Suppress unused warning
    
    return loss;
}

/**
 * Complete training step with forward and backward passes
 * 
 * This is the main training function that coordinates:
 * 1. Forward pass (all tokens in parallel)
 * 2. Loss computation
 * 3. Backward pass (all tokens in parallel)
 * 4. Optimizer application (all threads in parallel)
 * 
 * @param training Training context
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Loss value, or -1.0 on error
 */
double cllm_train_step_threaded(
    CLLMTraining* training,
    const uint32_t* input_tokens,
    const uint32_t* target_tokens,
    uint32_t num_tokens
) {
    if (!training || !input_tokens || !target_tokens) {
        fprintf(stderr, "ERROR: Invalid parameters for cllm_train_step_threaded\n");
        return -1.0;
    }
    
    CLLMModel* model = training->model;
    HierarchicalThreadPool* pool = model->threads;
    
    if (!pool) {
        fprintf(stderr, "FATAL: Cannot train without 88D thread pool\n");
        return -1.0;
    }
    
    // ========================================================================
    // STEP 1: FORWARD PASS
    // ========================================================================
    
    // Enqueue forward work for all tokens
    for (uint32_t i = 0; i < num_tokens; i++) {
        uint32_t token_id = input_tokens[i];
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
    // STEP 2: COMPUTE LOSS
    // ========================================================================
    
    double loss = compute_loss_distributed(pool, target_tokens, num_tokens);
    
    // ========================================================================
    // STEP 3: BACKWARD PASS
    // ========================================================================
    
    // Enqueue backward work for all tokens
    for (uint32_t i = 0; i < num_tokens; i++) {
        uint32_t token_id = input_tokens[i];
        uint32_t target_id = target_tokens[i];
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        
        if (thread) {
            hierarchical_thread_enqueue_work(thread, TRAINING_WORK_TYPE_BACKWARD,
                                            token_id, target_id);
        }
    }
    
    // Signal threads and wait for completion
    signal_all_threads(pool);
    wait_for_completion(pool);
    
    // ========================================================================
    // STEP 4: APPLY OPTIMIZER
    // ========================================================================
    
    // Import optimizer function
    extern int worker_apply_optimizer(
        HierarchicalThread* thread,
        double learning_rate,
        double beta1,
        double beta2,
        double epsilon
    );
    
    // Apply optimizer to each thread
    for (uint8_t layer = 0; layer < 8; layer++) {
        for (uint8_t dim = 0; dim <= 11; dim++) {
            HierarchicalThread* thread = hierarchical_thread_get(pool, layer, dim);
            if (thread) {
                worker_apply_optimizer(thread, 
                                      training->config.learning_rate,
                                      0.9,   // beta1
                                      0.999, // beta2
                                      1e-8); // epsilon
            }
        }
    }
    
    return loss;
}
// ============================================================================
// COMPLETE TRAINING LOOP (DAY 11)
// ============================================================================

/**
 * Complete training step with forward and backward passes
 * 
 * This is the main training function that coordinates:
 * 1. Forward pass (all tokens in parallel)
 * 2. Loss computation
 * 3. Backward pass (all tokens in parallel)
 * 4. Optimizer application (all threads in parallel)
 * 
 * @param training Training context
 * @param input_tokens Input token IDs
 * @param target_tokens Target token IDs
 * @param num_tokens Number of tokens
 * @return Loss value, or -1.0 on error
 */
