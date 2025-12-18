/**
 * CLLM Multi-Threaded Training Header
 * 
 * Multi-threaded training with kissing spheres architecture
 */

#ifndef CLLM_TRAINING_THREADED_H
#define CLLM_TRAINING_THREADED_H

#include "cllm_training.h"
#include "cllm_batch.h"
#include "cllm_metrics.h"

/**
 * Thread-Local Training Context
 * 
 * Each worker thread gets its own activation buffers to avoid race conditions.
 * Model weights are shared (read-only), but activations are thread-local.
 * 
 * Memory per thread: ~6.1 MB
 * For 63 threads: ~386 MB (acceptable overhead)
 */
typedef struct {
    // Forward pass activation storage (thread-local)
    double* input_embeddings;        // [batch * seq * embed]
    double** layer_inputs;           // [num_layers][batch * seq * embed]
    double** attention_outputs;      // [num_layers][batch * seq * embed]
    double** ff_outputs;             // [num_layers][batch * seq * embed]
    double** layer_outputs;          // [num_layers][batch * seq * embed]
    double** ff_hidden;              // [num_layers][batch * seq * ff_hidden]
    double* final_hidden;            // [batch * seq * embed]
    double* logits;                  // [batch * seq * vocab]
    
    // Attention cache (thread-local)
    struct {
        double* attention_weights;   // [num_heads * seq * seq]
        double* queries;             // [seq * embed]
        double* keys;                // [seq * embed]
        double* values;              // [seq * embed]
        double* scores;              // [num_heads * seq * seq]
    }* attention_cache;              // [num_layers]
    
    // Backward pass temporary buffers (thread-local)
    double* grad_logits;             // [batch * seq * vocab]
    double* grad_hidden;             // [batch * seq * embed]
    double* grad_layer;              // [batch * seq * embed]
    
    // Configuration (copied from main training)
    int batch_size;
    int seq_len;
    int num_layers;
    int embed_dim;
    int vocab_size;
    int ff_hidden_dim;
    int num_heads;
} ThreadLocalTrainingContext;

/**
 * Get next sphere ID for dynamic spawning
 *
 * Helper function for cllm_threads.c to get the next available sphere ID
 * when dynamically spawning children.
 *
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID, or -1 on error
 */
int threaded_training_get_next_sphere_id(void* user_data);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Threaded Training System (opaque)
 */
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;

/**
 * Create threaded training system
 * 
 * @param training Training context
 * @param batch_iterator Batch iterator for data
 * @param num_threads Number of worker threads (0 = auto-detect from CPU cores)
 * @return Threaded training system or NULL on failure
 */
/**
 * Thread-Local Training Context Functions
 */
ThreadLocalTrainingContext* thread_local_training_create(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
);

void thread_local_training_free(ThreadLocalTrainingContext* ctx);

/**
 * Threaded Forward/Backward Functions
 * 
 * These functions use thread-local activation buffers instead of shared CLLMTraining buffers.
 * This allows multiple threads to execute forward/backward passes in parallel without locking.
 */
// PRECISION FIX: Changed from float to double for consistency
double cllm_forward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
);

void cllm_backward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    double* gradient_buffer
);

/**
 * Threaded Training System Functions
 */
ThreadedTrainingSystem* threaded_training_create(CLLMTraining* training, 
                                                  CLLMBatchIterator* batch_iterator,
                                                  int num_threads);

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system);

/**
 * Train one epoch with lock-free work queue
 * 
 * Uses lock-free work queue for optimal parallel batch processing.
 * Includes real-time UI metrics updates and timing estimates.
 * 
 * @param system Threaded training system
 * @param current_epoch Current epoch number (for metrics display)
 * @return Average epoch loss
 */
void threaded_training_set_total_epochs(ThreadedTrainingSystem* system, int total_epochs);
// PRECISION FIX: Changed from float to double for consistency
double threaded_train_epoch_lockfree(ThreadedTrainingSystem* system, int current_epoch);

/**
 * Print threading statistics
 */
void threaded_training_print_stats(ThreadedTrainingSystem* system);

/**
 * Get per-sphere statistics
 * 
 * @param system Threaded training system
 * @param sphere_id Sphere ID (0-11)
 * @param batches_processed Output: number of batches processed by this sphere
 * @param avg_loss Output: average loss for this sphere
 * @return 0 on success, -1 on failure
 */
int threaded_training_get_sphere_stats(ThreadedTrainingSystem* system,
                                       int sphere_id,
                                       int* batches_processed,
                                       float* avg_loss);

/**
 * Get total gradient norm
 * 
 * @param system Threaded training system
 * @return Total gradient norm
 */
double threaded_training_get_gradient_norm(ThreadedTrainingSystem* system);

/**
 * Get number of worker spheres
 * 
 * @param system Threaded training system
 * @return Number of worker spheres
 */
int threaded_training_get_num_workers(ThreadedTrainingSystem* system);

/**
 * Get next sphere ID for dynamic spawning
 *
 * Helper function for cllm_threads.c to get the next available sphere ID
 * when dynamically spawning children.
 *
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID, or -1 on error
 */
int threaded_training_get_next_sphere_id(void* user_data);

/**
 * Get metrics from training system
 * 
 * UI Integration: Returns the metrics object for real-time monitoring
 * 
 * @param system Threaded training system
 * @return Pointer to metrics object, or NULL if not available
 */
CLLMMetrics* threaded_training_get_metrics(ThreadedTrainingSystem* system);

/**
 * Report sphere hierarchy to AppState for visualization
 * 
 * PHASE 5: Traverses the threading hierarchy and populates AppState
 * sphere_stats arrays with parent-child relationships, control vs worker
 * status, and hierarchy levels for complete visualization.
 * 
 * @param system Threaded training system
 * @param state Application state (contains sphere_stats)
 */
void threaded_training_report_hierarchy(ThreadedTrainingSystem* system, void* state);

/**
 * PHASE 6: Get entropy integration context for UI display
 * 
 * @param system Threaded training system
 * @return Pointer to entropy context, or NULL if not available
 */
void* threaded_training_get_entropy_context(ThreadedTrainingSystem* system);

/**
 * PHASE 6: Get adaptive hierarchy context for UI display
 * 
 * @param system Threaded training system
 * @return Pointer to adaptive hierarchy context, or NULL if not available
 */
void* threaded_training_get_adaptive_hierarchy(ThreadedTrainingSystem* system);

/**
 * PHASE 6: Get cymatic barrier statistics
 * 
 * @param system Threaded training system
 * @param epoch_syncs Output: number of epoch barrier synchronizations
 * @param batch_syncs Output: number of batch barrier synchronizations
 * @return 0 on success, -1 if barriers not available
 */
int threaded_training_get_cymatic_stats(ThreadedTrainingSystem* system,
                                        uint64_t* epoch_syncs,
                                        uint64_t* batch_syncs);

#ifdef __cplusplus
}
#endif

#endif // CLLM_TRAINING_THREADED_H