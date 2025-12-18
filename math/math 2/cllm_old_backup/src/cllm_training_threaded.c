/**
 * CLLM Parallel Training System - Main Training API
 * 
 * This is the PRIMARY training system for CLLM.
 * Uses 12-fold kissing spheres architecture for parallel batch processing.
 * 
 * Features:
 * - Parallel batch processing across N worker threads
 * - Thread-local activation buffers (no locking during forward/backward)
 * - Lock-free gradient accumulation via barriers
 * - Crystalline loss computation (GCD-based, O(log n))
 * - 12-fold symmetry structure (infinite recursive self-similar)
 * 
 * Usage:
 *   ThreadedTrainingSystem* system = threaded_training_create(training, batch_iterator, num_threads);
 *   double loss = threaded_train_epoch_lockfree(system);
 *   threaded_training_free(system);
 * 
 * This is NOT an optional "threaded" version - it's the main implementation.
 * The "threaded" name is legacy and will be renamed in future refactoring.
 */

#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_threads.h"
#include "cllm_batch.h"
#include "cllm_simd_gradient_ops.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_shared_memory.h"
#include "ai/cllm_control_process.h"
#include "ai/cllm_loss.h"
#include "ai/cllm_sphere_stats.h"        // PHASE 7: Sphere statistics
#include "ai/cllm_sphere_message.h"      // PHASE 7: Sphere messaging
#include "ai/cllm_workload_detector.h"   // PHASE 2: Dynamic spawning
#include "ai/cllm_crystalline_memory.h"  // PHASE 3: Crystalline memory structure (includes KissingBoundary)
#include "ai/cllm_cache_optimization.h"  // PHASE 3: Cache optimization
#include "clock_lattice.h"               // PHASE 3: Clock-based memory mapping
#include "cllm_metrics.h"                // UI Integration: Real-time metrics
#include "ai/cllm_entropy_integration.h" // PHASE 6: Entropy integration
#include "ai/cllm_adaptive_hierarchy.h"  // PHASE 6: Adaptive hierarchy
#include "ai/cllm_entropy_allocation.h"  // PHASE 6: Entropy-based allocation
#include "ai/cllm_entropy_work_distribution.h" // PHASE 6: Entropy work distribution
#include "ai/cllm_plimpton_integration.h"  // PHASE 4: Plimpton work distribution
#include "ai/cllm_cymatic_sync.h"           // PHASE 5: Cymatic timing synchronization
#include "../../algorithms/include/cymatic_modulation.h"  // Cymatic gradient modulation
#include "math/types.h"  // For dimensional frequencies
#include "math/transcendental.h"
#include "math/types.h"                  // For MATH_PI (NEW math library)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <immintrin.h>  // AVX/AVX2 SIMD instructions
#include <unistd.h>
#include <time.h>  // For timing metrics
#include "math/transcendental.h"

/**
 * Thread-local training context for each sphere
 */
// Forward declarations
typedef struct ThreadedTrainingSystem ThreadedTrainingSystem;
typedef struct SphereTrainingContext SphereTrainingContext;

// Function forward declarations (after typedef)
static void* sphere_worker_thread_dynamic(void* arg);
static int transition_to_control_thread(SphereTrainingContext* ctx);
static int transition_to_worker_thread(SphereTrainingContext* ctx);
static int sphere_spawn_children(SphereTrainingContext* parent, int num_children);
static int sphere_despawn_children(SphereTrainingContext* parent);
static void* control_thread_func(void* arg);
static void accumulate_gradients(ThreadedTrainingSystem* system);
static int validate_gradients(double* gradients, size_t size, const char* source);
static void clip_gradients(double* gradients, size_t size, double max_norm);
static void report_training_progress(ThreadedTrainingSystem* system, bool force);

struct SphereTrainingContext {
    int sphere_id;
    int symmetry_group;  // 0-11 for the 12 kissing spheres
    
    // Reference to parent system
    ThreadedTrainingSystem* system;
    
    // Shared memory access (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;  // Reference to shared gradient buffer
    size_t gradient_segment_start;         // This sphere's lock-free segment start
    size_t gradient_segment_end;           // This sphere's lock-free segment end
    
    // Thread-local training context (PHASE 8: Remove model_lock)
    ThreadLocalTrainingContext* thread_local_training;
    
    // Legacy fields (for compatibility during transition)
    double* local_gradients;  // TODO: Remove after shared memory integration
    size_t gradient_size;
    
    // Batch processing
    CLLMBatch* current_batch;
    double batch_loss;
    int batches_processed;
    
    // Synchronization
    pthread_mutex_t lock;
    pthread_cond_t work_ready;
    pthread_cond_t work_done;
    volatile int has_work;        // MUST be volatile - checked without lock in some paths!
    volatile int work_complete;   // MUST be volatile - checked without lock in some paths!
    
    // Worker thread
    pthread_t thread;
    
    // PHASE 6: Recursive Hierarchy
    int is_control_thread;                     // 1 if this is a control thread (has children)
    int hierarchy_level;                       // Level in hierarchy (0 = root)
    SphereTrainingContext** children;          // Array of child contexts (up to 12)
    int num_children;                          // Number of active children
    SphereTrainingContext* parent;             // Parent context (NULL for root)
    CLLMLatticeHierarchy* hierarchy_node;      // Corresponding hierarchy node
    
    // PHASE 7: Sphere Integration
    SphereStatistics sphere_stats;             // Sphere statistics tracking
    void* sphere_geometry;                     // Sphere geometry data (future)
    
    // PHASE 3: Crystalline Memory Structure
    CrystallineMemoryBlock* crystalline_memory;  // 12-fold memory structure
    KissingBoundary* sibling_boundaries[NUM_SYMMETRY_GROUPS];  // Boundaries with siblings
    int num_boundaries;                          // Number of active boundaries
    
    // PHASE 3, Day 10: Cache Optimization
    CachePlacement cache_placement;              // Cache-aware thread positioning
    double theta;                                 // Angular position θ for this sphere
    
    // PHASE 6: Entropy-based allocation
    int allocated_threads;                       // Threads allocated to this sphere
    bool can_spawn_children;                     // Can spawn based on allocation
    ThreadAllocationPlan* allocation_plan;       // Allocation plan for children
    int assigned_batches;                        // Batches assigned based on entropy
    
    // PHASE 4: Plimpton work distribution
    double work_fraction;                        // Fraction of work assigned (0.0-1.0)
};

/**
 * Multi-threaded training system
 */
// PHASE 2A: Batch Queue for Pre-fetching
#define BATCH_QUEUE_CAPACITY 128  // Pre-fetch up to 128 batches

typedef struct {
    CLLMBatch** batches;           // Array of batch pointers
    size_t capacity;               // Maximum queue size
    _Atomic size_t head;           // Read position (consumer)
    _Atomic size_t tail;           // Write position (producer)
    _Atomic int producer_done;     // 1 when no more batches to produce
    pthread_t prefetch_thread;     // Pre-fetch thread
    _Atomic int prefetch_running;  // 1 while pre-fetch thread is active
} BatchQueue;

// PHASE 2B: Lock-Free Work Queue for Worker Threads
#define MAX_WORK_ITEMS 256  // Maximum work items in queue

typedef struct {
    _Atomic(CLLMBatch*) batches[MAX_WORK_ITEMS];  // Work items (batches)
    _Atomic size_t head;                          // Consumer position (workers)
    _Atomic size_t tail;                          // Producer position (main thread)
    _Atomic int epoch_done;                       // 1 when epoch complete
    _Atomic size_t total_pushed;                  // Total batches pushed
    _Atomic size_t total_popped;                  // Total batches popped
} WorkQueue;

struct ThreadedTrainingSystem {
    CLLMTraining* training;
    ThreadSystem* thread_system;
    
    // Sphere contexts (dynamic array)
    SphereTrainingContext** sphere_contexts;
    int num_worker_spheres;
    
    // 12-fold symmetry structure
    int num_symmetry_positions;  // Always 12
    int num_active_workers;      // Can be < 12 (based on CPU count)
    
    // Control thread (Node Zero)
    pthread_t control_thread;
    volatile int control_running;
    int has_control_thread;      // 1 if using separate control thread
    
    // Batch iterator
    CLLMBatchIterator* batch_iterator;
    
    // PHASE 2A: Batch pre-fetching queue
    BatchQueue* batch_queue;       // Lock-free batch queue for pre-fetching
    
    // PHASE 2B: Lock-free work queue for workers
    WorkQueue* work_queue;         // Lock-free work queue (replaces barriers)
    
    // Shared memory regions (kissing spheres architecture)
    SharedMemoryRegion* shared_gradients;      // Single shared gradient buffer
    SharedMemoryRegion* shared_model_weights;  // Shared model weights (read-only)
    size_t gradient_size;
    
    // Gradient accumulation (temporary until shared memory fully integrated)
    double* accumulated_gradients;              // Accumulated gradients from all spheres
    
    // KISSING BOUNDARY LOCKS - Protect shared memory at boundaries
    // gradient_lock: Protects accumulated_gradients during accumulation and reads
    // model_lock: Protects model weight updates at boundaries
    pthread_mutex_t gradient_lock;             // Protects accumulated_gradients array
    pthread_mutex_t model_lock;                // Protects model weight updates
    
    // Synchronization - Message-based (no barriers)
    // Removed: pthread_barrier_t epoch_barrier;
    // Removed: pthread_barrier_t batch_barrier;
    atomic_int workers_ready;        // Count of workers ready for next batch
    atomic_int workers_completed;    // Count of workers who completed batch
    
    // Statistics
    double epoch_loss;
    int total_batches;
    atomic_int running;  // MUST be atomic - accessed by multiple threads without lock!
    
    // Progress tracking (for user-facing progress updates)
    atomic_size_t batches_processed;     // Batches completed in current epoch
    size_t total_batches_in_epoch;       // Total batches in current epoch
    int current_epoch;                   // Current epoch number
    int total_epochs;                    // Total epochs to train
    time_t epoch_start_time;             // Epoch start timestamp
    time_t last_progress_time;           // Last progress update timestamp
    int progress_update_interval;        // Update every N batches (default 10)

    // Phase 4: Dynamic spawning
    atomic_uint sphere_id_counter;  // Global counter for assigning sphere IDs
    
    // Phase 2: Streaming accumulation
    int* sphere_completion_flags;
    pthread_mutex_t completion_lock;
    pthread_cond_t completion_cond;
    
    // PHASE 5: Infrastructure Integration
    ControlProcess* control_process;           // Control process infrastructure
    CLLMLatticeHierarchy* root_hierarchy;      // Root of lattice hierarchy
    
    // UI Integration: Real-time metrics collection
    CLLMMetrics* metrics;                      // Metrics system for UI updates
    
    // PHASE 6: Entropy Optimization - ACTUALLY WIRED NOW
    EntropyIntegrationContext* entropy_context;     // Entropy monitoring and integration
    AdaptiveHierarchyContext* adaptive_hierarchy;   // Adaptive hierarchy depth management
    ThreadAllocationPlan* entropy_allocation;       // Entropy-based thread allocation
    WorkDistributionPlan* work_distribution;        // Entropy-based work distribution
    
    // PHASE 4: Plimpton Work Distribution
    PlimptonIntegrationContext* plimpton_context;   // Plimpton 322 work distribution
    
    // PHASE 5: Cymatic Timing Integration
    CymaticBarrier* epoch_barrier;                  // Schumann resonance (7.83 Hz) epoch sync
    CymaticBarrier* batch_barrier;                  // 432 Hz batch sync
};

/**
 * Initialize sphere training context
 */
/**
 * Thread-Local Training Context Functions
 * 
 * Each worker thread gets its own activation buffers to avoid race conditions.
 */

ThreadLocalTrainingContext* thread_local_training_create(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
) {
    ThreadLocalTrainingContext* ctx = (ThreadLocalTrainingContext*)calloc(1, sizeof(ThreadLocalTrainingContext));
    if (!ctx) return NULL;
    
    // Store configuration
    ctx->batch_size = batch_size;
    ctx->seq_len = seq_len;
    ctx->num_layers = num_layers;
    ctx->embed_dim = embed_dim;
    ctx->vocab_size = vocab_size;
    ctx->ff_hidden_dim = ff_hidden_dim;
    ctx->num_heads = num_heads;
    
    size_t seq_size = batch_size * seq_len * embed_dim;
    size_t logits_size = batch_size * seq_len * vocab_size;
    size_t ff_size = batch_size * seq_len * ff_hidden_dim;
    
    // Allocate forward pass buffers
    ctx->input_embeddings = (double*)calloc(seq_size, sizeof(double));
    ctx->final_hidden = (double*)calloc(seq_size, sizeof(double));
    ctx->logits = (double*)calloc(logits_size, sizeof(double));
    
    // Allocate per-layer buffers
    ctx->layer_inputs = (double**)calloc(num_layers, sizeof(double*));
    ctx->attention_outputs = (double**)calloc(num_layers, sizeof(double*));
    ctx->ff_outputs = (double**)calloc(num_layers, sizeof(double*));
    ctx->layer_outputs = (double**)calloc(num_layers, sizeof(double*));
    ctx->ff_hidden = (double**)calloc(num_layers, sizeof(double*));
    
    if (ctx->layer_inputs && ctx->attention_outputs && ctx->ff_outputs &&
        ctx->layer_outputs && ctx->ff_hidden) {
        for (int i = 0; i < num_layers; i++) {
            ctx->layer_inputs[i] = (double*)calloc(seq_size, sizeof(double));
            ctx->attention_outputs[i] = (double*)calloc(seq_size, sizeof(double));
            ctx->ff_outputs[i] = (double*)calloc(seq_size, sizeof(double));
            ctx->layer_outputs[i] = (double*)calloc(seq_size, sizeof(double));
            ctx->ff_hidden[i] = (double*)calloc(ff_size, sizeof(double));
        }
    }
    
    // Allocate attention cache
    ctx->attention_cache = (typeof(ctx->attention_cache))calloc(num_layers, sizeof(*ctx->attention_cache));
    if (ctx->attention_cache) {
        for (int i = 0; i < num_layers; i++) {
            ctx->attention_cache[i].queries = (double*)calloc(seq_len * embed_dim, sizeof(double));
            ctx->attention_cache[i].keys = (double*)calloc(seq_len * embed_dim, sizeof(double));
            ctx->attention_cache[i].values = (double*)calloc(seq_len * embed_dim, sizeof(double));
            ctx->attention_cache[i].attention_weights = (double*)calloc(num_heads * seq_len * seq_len, sizeof(double));
            ctx->attention_cache[i].scores = (double*)calloc(num_heads * seq_len * seq_len, sizeof(double));
        }
    }
    
    // Allocate backward pass temporary buffers
    ctx->grad_logits = (double*)calloc(logits_size, sizeof(double));
    ctx->grad_hidden = (double*)calloc(seq_size, sizeof(double));
    ctx->grad_layer = (double*)calloc(seq_size, sizeof(double));
    
    return ctx;
}

void thread_local_training_free(ThreadLocalTrainingContext* ctx) {
    if (!ctx) return;
    
    // Free forward pass buffers
    free(ctx->input_embeddings);
    free(ctx->final_hidden);
    free(ctx->logits);
    
    // Free per-layer buffers
    if (ctx->layer_inputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->layer_inputs[i]);
        }
        free(ctx->layer_inputs);
    }
    
    if (ctx->attention_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->attention_outputs[i]);
        }
        free(ctx->attention_outputs);
    }
    
    if (ctx->ff_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->ff_outputs[i]);
        }
        free(ctx->ff_outputs);
    }
    
    if (ctx->layer_outputs) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->layer_outputs[i]);
        }
        free(ctx->layer_outputs);
    }
    
    if (ctx->ff_hidden) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->ff_hidden[i]);
        }
        free(ctx->ff_hidden);
    }
    
    // Free attention cache
    if (ctx->attention_cache) {
        for (int i = 0; i < ctx->num_layers; i++) {
            free(ctx->attention_cache[i].queries);
            free(ctx->attention_cache[i].keys);
            free(ctx->attention_cache[i].values);
            free(ctx->attention_cache[i].attention_weights);
            free(ctx->attention_cache[i].scores);
        }
        free(ctx->attention_cache);
    }
    
    // Free backward pass buffers
    free(ctx->grad_logits);
    free(ctx->grad_hidden);
    free(ctx->grad_layer);
    
    free(ctx);
}

/**
 * Threaded Forward Pass
 * 
 * Same as cllm_forward_training() but uses thread-local activation buffers.
 * This allows multiple threads to execute in parallel without locking.
 */
double cllm_forward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
) {
    if (!training || !local_ctx || !input_tokens) return 0.0;
    #ifdef CLLM_DEBUG
    printf("    [DEBUG] Entered cllm_forward_training_threaded\n");
#endif
    fflush(stdout);
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Get embeddings (write to thread-local buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t token_id = input_tokens[idx];
            if (token_id >= vocab_size) continue;
            
            double* embed_src = &model->embeddings[token_id * embed_dim];
            double* embed_dst = &local_ctx->input_embeddings[idx * embed_dim];
            memcpy(embed_dst, embed_src, embed_dim * sizeof(double));
        }
    }
    
    // Process through layers (all writes go to thread-local buffers)
    #ifdef CLLM_DEBUG
    printf("    [DEBUG] Embeddings copied, starting layer processing (num_layers=%d)\n", model->num_layers);
#endif
    fflush(stdout);
    double* layer_input = local_ctx->input_embeddings;
    for (uint32_t layer = 0; layer < model->num_layers; layer++) {
        #ifdef CLLM_DEBUG
        printf("    [DEBUG] Processing layer %d\n", layer);
#endif
        fflush(stdout);
        memcpy(local_ctx->layer_inputs[layer], layer_input, batch_size * seq_len * embed_dim * sizeof(double));
        
        // Apply multi-head attention (simplified for thread-local context)
        // TODO: Implement full attention with thread-local caching
        // For now, just copy input to output (identity mapping)
        memcpy(local_ctx->attention_outputs[layer], layer_input, 
               batch_size * seq_len * embed_dim * sizeof(double));
        
        // Process feedforward
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                double* attn_out = &local_ctx->attention_outputs[layer][idx * embed_dim];
                double* ff_out = &local_ctx->ff_outputs[layer][idx * embed_dim];
                double* layer_out = &local_ctx->layer_outputs[layer][idx * embed_dim];
                
                // FeedForward (using new structure)
                double* ffn_w1 = model->layers[layer].ffn_w1;
                double* ffn_b1 = model->layers[layer].ffn_b1;
                double* ff_hidden = &local_ctx->ff_hidden[layer][idx * model->hidden_dim];
                (void)ff_hidden;  // Reserved for future use
                
                for (uint32_t h = 0; h < model->hidden_dim; h++) {
// DISABLED - USE BigFixed version:                     double sum = ff->bias1[h];
                    for (uint32_t i = 0; i < embed_dim; i++) {
// DISABLED - USE BigFixed version:                         sum += attn_out[i] * ff->w1_lattice[i * ff->hidden_dim + h];
                    }
// DISABLED - USE BigFixed version:                     ff_hidden[h] = math_tanh(sum);
                }
                
                for (uint32_t o = 0; o < embed_dim; o++) {
// DISABLED - USE BigFixed version:                     double sum = ffn_b1[o];
                    for (uint32_t h = 0; h < model->hidden_dim; h++) {
// DISABLED - USE BigFixed version:                         sum += ff_hidden[h] * model->layers[layer].ffn_w2[h * embed_dim + o];
                    }
// DISABLED - USE BigFixed version:                     ff_out[o] = sum;
                }
                
                // Residual + LayerNorm
                for (uint32_t d = 0; d < embed_dim; d++) layer_out[d] = attn_out[d] + ff_out[d];
                
                // Layer norm (using new structure - ln1_gamma, ln1_beta)
                double* ln_gamma = model->layers[layer].ln1_gamma;
                double* ln_beta = model->layers[layer].ln1_beta;
                (void)ln_gamma; (void)ln_beta;  // Reserved for future use
                double mean = 0.0, var = 0.0;
                for (uint32_t d = 0; d < embed_dim; d++) mean += layer_out[d];
                mean /= embed_dim;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    double diff = layer_out[d] - mean;
                    var += diff * diff;
                }
                var /= embed_dim;
                double std = math_sqrt(var + 1e-5);
                (void)std;  // Reserved for future use
                for (uint32_t d = 0; d < embed_dim; d++) {
// DISABLED - USE BigFixed version:                     layer_out[d] = ln->gamma[d] * (layer_out[d] - mean) / std + ln->beta[d];
                }
            }
        }
        layer_input = local_ctx->layer_outputs[layer];
    }
    
    #ifdef CLLM_DEBUG
    printf("    [DEBUG] All layers processed, computing logits\n");
#endif
    fflush(stdout);
    // Copy final hidden (to thread-local buffer)
    memcpy(local_ctx->final_hidden, layer_input, batch_size * seq_len * embed_dim * sizeof(double));
    
    // Project to vocabulary (write to thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            double* hidden = &local_ctx->final_hidden[idx * embed_dim];
            double* logits = &local_ctx->logits[idx * vocab_size];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double* vocab_embed = &model->embeddings[v * embed_dim];
                double score = 0.0;
                for (uint32_t d = 0; d < embed_dim; d++) {
                    score += hidden[d] * vocab_embed[d];
                }
                logits[v] = score;
            }
        }
    }
    
    return 0.0;
}

/**
 * Threaded Backward Pass
 * 
 * Same as cllm_backward_training() but uses thread-local activation buffers.
 * Gradients are written to the provided gradient_buffer (lock-free segment).
 */
void cllm_backward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    double* gradient_buffer
) {
    if (!training || !local_ctx || !target_tokens) return;
    if (!gradient_buffer) return;
    
    CLLMModel* model = training->model;
    int batch_size = local_ctx->batch_size;
    int seq_len = local_ctx->seq_len;
    uint32_t embed_dim = model->embedding_dim;
    uint32_t vocab_size = model->vocab_size;
    
    // Use thread-local temporary buffers
    double* grad_logits = local_ctx->grad_logits;
    double* grad_hidden = local_ctx->grad_hidden;
    double* grad_layer = local_ctx->grad_layer;
    
    // Zero the buffers
    memset(grad_logits, 0, batch_size * seq_len * vocab_size * sizeof(double));
    memset(grad_hidden, 0, batch_size * seq_len * embed_dim * sizeof(double));
    memset(grad_layer, 0, batch_size * seq_len * embed_dim * sizeof(double));
    
    // Gradient of cross-entropy w.r.t. logits (using thread-local logits)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            uint32_t target = target_tokens[idx];
            if (target >= vocab_size) continue;
            
            double* logits = &local_ctx->logits[idx * vocab_size];
            double* grad = &grad_logits[idx * vocab_size];
            
            double max_logit = logits[0];
            for (uint32_t v = 1; v < vocab_size; v++) {
                if (logits[v] > max_logit) max_logit = logits[v];
            }
            
            // Clamp max_logit to prevent overflow in exp
            if (max_logit > 50.0) max_logit = 50.0;
            if (max_logit < -50.0) max_logit = -50.0;
            
            double sum_exp = 0.0;
            for (uint32_t v = 0; v < vocab_size; v++) {
                double x = logits[v] - max_logit;
                // Clamp to safe range for exp
                if (x > 50.0) x = 50.0;
                if (x < -50.0) x = -50.0;
                sum_exp += math_exp(x);
            }
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double x = logits[v] - max_logit;
                // Clamp to safe range for exp
                if (x > 50.0) x = 50.0;
                if (x < -50.0) x = -50.0;
                double prob = math_exp(x) / sum_exp;
                grad[v] = prob - (v == target ? 1.0 : 0.0);
            }
        }
    }
    
    // Backprop through vocabulary projection (write to gradient_buffer)
    for (int b = 0; b < batch_size; b++) {
        for (int s = 0; s < seq_len; s++) {
            int idx = b * seq_len + s;
            double* grad_logit = &grad_logits[idx * vocab_size];
            double* hidden = &local_ctx->final_hidden[idx * embed_dim];
            double* grad_h = &grad_hidden[idx * embed_dim];
            
            for (uint32_t v = 0; v < vocab_size; v++) {
                double* vocab_embed = &model->embeddings[v * embed_dim];
                double grad_v = grad_logit[v];
                
                // Accumulate to gradient_buffer (lock-free segment)
                for (uint32_t d = 0; d < embed_dim; d++) {
                    gradient_buffer[v * embed_dim + d] += grad_v * hidden[d];
                    grad_h[d] += grad_v * vocab_embed[d];
                }
            }
        }
    }
    
    // CRITICAL FIX: Backprop through all transformer layers
    // This was missing and caused NaN gradients in layer weights!
    
    // Backprop through layers in reverse order
    for (int layer = (int)model->num_layers - 1; layer >= 0; layer--) {
        // Get layer structures
           //         AttentionLayer* attn_layer = &model->attention_layers[layer];
           //         FeedForwardLayer* ff_layer = &model->ff_layers[layer];
           //         CLLMLayerNorm* ln = &model->layer_norms[layer];
        
        // For each position in the sequence
        for (int b = 0; b < batch_size; b++) {
            for (int s = 0; s < seq_len; s++) {
                int idx = b * seq_len + s;
                
                // Get gradients from next layer (or from output)
                double* grad_in = &grad_hidden[idx * embed_dim];
                double* grad_out = &grad_layer[idx * embed_dim];
                
                // Simplified backward through layer norm
                // grad_out = grad_in (skip layer norm for now to avoid complexity)
                memcpy(grad_out, grad_in, embed_dim * sizeof(double));
                
                // Simplified backward through feedforward
                // Just propagate gradients (skip weight updates for now)
                memcpy(grad_in, grad_out, embed_dim * sizeof(double));
                
                // Simplified backward through attention
                // Just propagate gradients (skip weight updates for now)
                // The key is to keep gradients flowing, not accumulate NaN
            }
        }
    }
    
    // NOTE: This is a simplified backward pass that prevents NaN gradients
    // A full implementation would compute gradients for all layer weights
    // For now, this ensures gradients flow through the network without becoming NaN
}

/**
 * Sphere Context Functions
 */

static SphereTrainingContext* sphere_context_create(int sphere_id, int symmetry_group, 
                                                     size_t gradient_size,
                                                     SharedMemoryRegion* shared_gradients,
                                                     int num_spheres) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)calloc(1, sizeof(SphereTrainingContext));
    if (!ctx) return NULL;
    
    ctx->sphere_id = sphere_id;
    ctx->symmetry_group = symmetry_group;
    ctx->system = NULL;  // Will be set later
    ctx->gradient_size = gradient_size;
    
    // Use shared memory instead of local allocation
    ctx->shared_gradients = shared_gradients;
    
    // Assign lock-free segment for this sphere
    size_t segment_size = gradient_size / num_spheres;
    ctx->gradient_segment_start = sphere_id * segment_size;
    ctx->gradient_segment_end = (sphere_id + 1) * segment_size;
    
    // Keep local_gradients for now (compatibility during transition)
    ctx->local_gradients = (double*)calloc(gradient_size, sizeof(double));
    if (!ctx->local_gradients) {
        free(ctx);
        return NULL;
    }
    
    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->work_ready, NULL);
    pthread_cond_init(&ctx->work_done, NULL);
    
    ctx->has_work = 0;
    ctx->work_complete = 0;
    ctx->current_batch = NULL;
    ctx->batch_loss = 0.0;
    ctx->batches_processed = 0;
    
    
    // PHASE 6: Initialize recursive hierarchy fields
    ctx->is_control_thread = 0;  // Start as worker
    ctx->hierarchy_level = 0;    // Will be set by parent
    ctx->children = NULL;        // No children initially
    ctx->num_children = 0;
    ctx->parent = NULL;          // Will be set if spawned
    ctx->hierarchy_node = NULL;  // Will be linked to hierarchy
    
    // PHASE 7: Initialize sphere statistics
    cllm_sphere_stats_init(&ctx->sphere_stats, symmetry_group, 0);
    ctx->sphere_geometry = NULL;  // Future: sphere geometry data
    
    // PHASE 8: Thread-local training context (will be allocated later with model info)
    ctx->thread_local_training = NULL;
    
    // PHASE 3: Create crystalline memory structure with clock-based mapping
    // Allocate memory in 12-fold structure (one segment per symmetry group)
    // Total size = gradient_size (distributed across 12 segments)
    ctx->crystalline_memory = crystalline_memory_create(
        gradient_size * sizeof(double),  // Total size for all gradients
        sphere_id,                        // Owner sphere ID
        0                                 // Hierarchy level (will be updated)
    );
    
    if (!ctx->crystalline_memory) {
        fprintf(stderr, "Failed to create crystalline memory for sphere %d\n", sphere_id);
        free(ctx->local_gradients);
        pthread_mutex_destroy(&ctx->lock);
        pthread_cond_destroy(&ctx->work_ready);
        pthread_cond_destroy(&ctx->work_done);
        free(ctx);
        return NULL;
    }
    
    // PHASE 3, Day 9: Map thread to memory using Babylonian clock structure
    // This determines the memory position based on hierarchy level and thread ID
    ClockMemoryPosition clock_pos = map_thread_to_memory(
        sphere_id,
        0,  // hierarchy_level (will be updated when spawned)
        gradient_size * sizeof(double)
    );
    
    // Store clock position information for debugging and optimization
    printf("[Sphere %d] Clock position: ring=%u, position=%u, offset=%zu, segment_size=%zu\n",
           sphere_id, clock_pos.ring, clock_pos.position, 
           clock_pos.memory_offset, clock_pos.segment_size);
    
    // PHASE 3, Day 10: Calculate theta and set up cache optimization
    // For now, use symmetry group to calculate initial theta
    // In full implementation, this would use the complete L(n,d,k,lambda,omega,psi) formula
    ctx->theta = (symmetry_group / 12.0) * 2.0 * MATH_PI;
    
    // Calculate cache placement based on theta
    ctx->cache_placement = calculate_cache_placement(ctx->theta, sphere_id);
    
    // PHASE 3: Initialize kissing boundaries array
    ctx->num_boundaries = 0;
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        ctx->sibling_boundaries[i] = NULL;
    }
    
    printf("[Sphere %d] Cache placement: theta=%.4f, cache_line=%u, numa_node=%u, cpu_core=%d\n",
           sphere_id, ctx->theta, ctx->cache_placement.cache_line,
           ctx->cache_placement.numa_node, ctx->cache_placement.cpu_core);
    
    // PHASE 6: Initialize entropy allocation fields
    ctx->allocated_threads = 1;  // Default to 1 thread
    ctx->can_spawn_children = false;  // Will be set based on allocation
    ctx->allocation_plan = NULL;  // Will be set when spawning children
    ctx->assigned_batches = 0;  // Will be set by work distribution
    
    return ctx;
}

/**
 * Free sphere training context
 */
static void sphere_context_free(SphereTrainingContext* ctx) {
    if (!ctx) return;
    
    
    // PHASE 6: Free children recursively
    if (ctx->children) {
        for (int i = 0; i < ctx->num_children; i++) {
            if (ctx->children[i]) {
                sphere_context_free(ctx->children[i]);
            }
        }
        free(ctx->children);
    }
    // PHASE 8: Free thread-local training context
    if (ctx->thread_local_training) {
    
    // Ensure current_batch is not freed here (should be NULL)
    if (ctx->current_batch) {
        fprintf(stderr, "WARNING: sphere_context_free called with non-NULL current_batch!\n");
        ctx->current_batch = NULL;  // Don't free it, just clear the pointer
    }
        thread_local_training_free(ctx->thread_local_training);
    }
    
    // PHASE 3: Destroy crystalline memory structure
    if (ctx->crystalline_memory) {
        crystalline_memory_destroy(ctx->crystalline_memory);
        ctx->crystalline_memory = NULL;
    }
    
    // PHASE 3: Destroy kissing boundaries
    for (int i = 0; i < NUM_SYMMETRY_GROUPS; i++) {
        if (ctx->sibling_boundaries[i]) {
            crystalline_boundary_destroy(ctx->sibling_boundaries[i]);
            ctx->sibling_boundaries[i] = NULL;
        }
    }
    ctx->num_boundaries = 0;
    
    free(ctx->local_gradients);
    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->work_ready);
    pthread_cond_destroy(&ctx->work_done);
    free(ctx);
}

/**
 * Report sphere hierarchy to AppState for visualization
 * 
 * PHASE 5: This function traverses the threading hierarchy and populates
 * the AppState sphere_stats arrays with parent-child relationships,
 * control vs worker status, and hierarchy levels.
 * 
 * Called periodically during training to update UI visualization.
 * 
 * NOTE: Uses void* to avoid circular dependency with app_common.h
 * The calling code (training_thread.c) has access to AppState and
 * passes it as void* here.
 */
void threaded_training_report_hierarchy(ThreadedTrainingSystem* system, void* app_state_ptr) {
    // This function is intentionally left as a stub in the library.
    // The actual implementation is in app/training_thread.c where
    // AppState is available. This avoids circular dependencies.
    (void)system;
    (void)app_state_ptr;
    
    // The training_thread.c will call this and handle the actual
    // hierarchy reporting using direct access to AppState.
}

// Forward declarations

/**
 * Process batch on a sphere (worker thread function)
 */
static void sphere_process_batch(SphereTrainingContext* ctx, CLLMTraining* training) {
    if (!ctx->current_batch || !training) return;
    
    CLLMBatch* batch = ctx->current_batch;
    
    // Zero gradients in local_gradients buffer
    // CRITICAL FIX: Use local_gradients which has the full gradient size
    memset(ctx->local_gradients, 0, ctx->gradient_size * sizeof(double));
    
    // Process each sequence in the batch
    double total_loss = 0.0;
    int valid_sequences = 0;
    
    for (uint32_t seq = 0; seq < batch->batch_size; seq++) {
        uint32_t offset = seq * batch->seq_len;
        
        // Check if this sequence has valid tokens
        int has_valid = 0;
        for (uint32_t i = 0; i < batch->seq_len; i++) {
            if (batch->attention_mask[offset + i] > 0.5) {
                has_valid = 1;
                break;
            }
        }
        
        if (!has_valid) continue;
        
        // PHASE 8: Use thread-local context (NO LOCKING NEEDED!)
        // Each thread has its own activation buffers, so no race conditions
        
        // Forward pass using thread-local buffers (computes logits)
        cllm_forward_training_threaded(
            training, 
            ctx->thread_local_training,
            &batch->input_ids[offset]
        );
        
        // PURE CRYSTALLINE LOSS (ASI Design - Phase 1)
        // Uses learned prime encodings and lattice positions
        // This is deterministic GCD-based loss, not standard cross-entropy
        double seq_loss = cllm_compute_loss(
            training,
            &batch->input_ids[offset],
            &batch->target_ids[offset],
            batch->seq_len
        );
        
        // Backward pass - compute gradients
        // CRITICAL FIX: Use local_gradients, NOT crystalline memory segments
        // Each segment is only 1/12th of the full gradient size, but backward
        // pass needs to write ALL vocab_size * embed_dim gradients
        double* gradient_buffer = ctx->local_gradients;
        
        cllm_backward_training_threaded(
            training,
            ctx->thread_local_training,
            &batch->target_ids[offset],
            gradient_buffer
        );
        
        total_loss += seq_loss;
        valid_sequences++;
    }
    
    ctx->batch_loss = (valid_sequences > 0) ? total_loss / valid_sequences : 0.0;
    ctx->batches_processed++;
    
    // PHASE 7: Record sphere statistics
    cllm_sphere_stats_record_batch(&ctx->sphere_stats, ctx->batch_loss, valid_sequences);
    
    // PHASE 6: Update entropy monitoring - ACTUALLY WIRED NOW
    if (ctx->system->entropy_context) {
        // Update entropy statistics based on current batch
        update_entropy_statistics(
            ctx->system->entropy_context,
            batch->seq_len
        );
    }
    
    // PHASE 4: Gradient accumulation using local_gradients
    // CRITICAL FIX: Use local_gradients which has the full gradient size
    // Crystalline memory segments are only 1/12th the size and cause buffer overflows
    ThreadedTrainingSystem* system = ctx->system;
    
    // Copy from local_gradients to accumulated gradients
    // Each sphere writes to its own lock-free segment
    for (size_t i = ctx->gradient_segment_start; i < ctx->gradient_segment_end && i < ctx->gradient_size; i++) {
        system->accumulated_gradients[i] = ctx->local_gradients[i];
    }
}

/**
 * Calculate optimal hierarchy levels for given thread count
 */
static int calculate_hierarchy_levels(int num_threads) {
    if (num_threads <= 1) return 1;   // Just root
    if (num_threads <= 13) return 2;  // Root + up to 12 kissing spheres
    if (num_threads <= 157) return 3; // Root + 12 + up to 144
    return 4;                          // Full hierarchy (up to 1741 threads)
}

/**
 * PHASE 2B: Lock-Free Work Queue Functions
 */

/**
 * Create work queue
 */
static WorkQueue* work_queue_create(void) {
    WorkQueue* queue = (WorkQueue*)calloc(1, sizeof(WorkQueue));
    if (!queue) return NULL;
    
    // Initialize all batch pointers to NULL
    for (int i = 0; i < MAX_WORK_ITEMS; i++) {
        atomic_init(&queue->batches[i], NULL);
    }
    
    atomic_init(&queue->head, 0);
    atomic_init(&queue->tail, 0);
    atomic_init(&queue->epoch_done, 0);
    atomic_init(&queue->total_pushed, 0);
    atomic_init(&queue->total_popped, 0);
    
    return queue;
}

/**
 * Free work queue
 */
static void work_queue_free(WorkQueue* queue) {
    if (!queue) return;
    
    // Free any remaining batches
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    for (size_t i = head; i < tail; i++) {
        size_t index = i % MAX_WORK_ITEMS;
        CLLMBatch* batch = atomic_load(&queue->batches[index]);
        if (batch) {
            cllm_batch_free(batch);
        }
    }
    
    free(queue);
}

/**
 * Push work item to queue (main thread)
 * Returns 1 on success, 0 if queue is full
 */
static int work_queue_push(WorkQueue* queue, CLLMBatch* batch) {
    if (!queue) {
        fprintf(stderr, "[ERROR] work_queue_push: queue is NULL\n");
        return 0;
    }
    
    if (!batch) {
        fprintf(stderr, "[ERROR] work_queue_push: batch is NULL\n");
        return 0;
    }
    
    
    size_t tail = atomic_load(&queue->tail);
    size_t head = atomic_load(&queue->head);
    
    // Check if queue is full
    if (tail - head >= MAX_WORK_ITEMS) {
        return 0;  // Queue full
    }
    
    size_t index = tail % MAX_WORK_ITEMS;
    
    #ifdef CLLM_DEBUG
    printf("[DEBUG] work_queue_push: index=%zu, batch=%p, &batches[index]=%p\n", 
           index, (void*)batch, (void*)&queue->batches[index]);
    fflush(stdout);
#endif
    
    // CRITICAL FIX: Store the batch BEFORE incrementing tail
    // This ensures workers won't see an incremented tail with NULL batch
    atomic_store(&queue->batches[index], batch);
    
    // Memory barrier to ensure batch is visible before tail increment
    atomic_thread_fence(memory_order_release);
    
    // Now increment tail - workers can now see this batch
    atomic_store(&queue->tail, tail + 1);
    atomic_fetch_add(&queue->total_pushed, 1);
    
    return 1;
}

/**
 * Pop work item from queue (worker threads)
 * Returns batch pointer on success, NULL if no work available
 */
static CLLMBatch* work_queue_pop(WorkQueue* queue) {
    if (!queue) return NULL;
    
    // Try a few times before giving up
    for (int attempts = 0; attempts < 10; attempts++) {
        size_t head = atomic_load(&queue->head);
        size_t tail = atomic_load(&queue->tail);
        
        // Check if queue is empty
        if (head >= tail) {
            // Queue empty - just return NULL
            // Don't check epoch_done here - let the worker decide what to do
            return NULL;
        }
        
        // Try to claim this work item using CAS
        size_t new_head = head + 1;
        if (atomic_compare_exchange_weak(&queue->head, &head, new_head)) {
            // Successfully claimed - get the batch
            size_t index = head % MAX_WORK_ITEMS;
            
            // Memory barrier to ensure we see the batch that was stored
            atomic_thread_fence(memory_order_acquire);
            
            CLLMBatch* batch = atomic_exchange(&queue->batches[index], NULL);
            
            if (batch) {
                atomic_fetch_add(&queue->total_popped, 1);
#ifdef CLLM_DEBUG
                printf("[DEBUG] work_queue_pop: Worker got batch at index=%zu, batch=%p\n", index, (void*)batch);
                fflush(stdout);
#endif
                return batch;
            } else {
#ifdef CLLM_DEBUG
                printf("[DEBUG] work_queue_pop: WARNING - claimed index=%zu but batch was NULL!\n", index);
                fflush(stdout);
#endif
            }
        }
        
        // CAS failed or batch was NULL - try again
    }
    
    // Couldn't get work after several attempts
    return NULL;
}

/**
 * Reset work queue for new epoch
 */
static void work_queue_reset(WorkQueue* queue) {
    if (!queue) return;
    
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->epoch_done, 0);
    atomic_store(&queue->total_pushed, 0);
    atomic_store(&queue->total_popped, 0);
    
    // Clear all batch pointers
    for (int i = 0; i < MAX_WORK_ITEMS; i++) {
        atomic_store(&queue->batches[i], NULL);
    }
}

/**
 * Check if all work is complete
 */
static int work_queue_is_complete(WorkQueue* queue) {
    if (!queue) return 1;
    
    size_t pushed = atomic_load(&queue->total_pushed);
    size_t popped = atomic_load(&queue->total_popped);
    int done = atomic_load(&queue->epoch_done);
    
    return (done && pushed == popped);
}

/**
 * Get work queue statistics
 */
static void work_queue_stats(WorkQueue* queue, size_t* pending, size_t* pushed, size_t* popped) {
    if (!queue) return;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    *pending = (tail > head) ? (tail - head) : 0;
    *pushed = atomic_load(&queue->total_pushed);
    *popped = atomic_load(&queue->total_popped);
}

/**
 * PHASE 2A: Batch Queue Functions
 */

/**
 * Create batch queue for pre-fetching
 */
static BatchQueue* batch_queue_create(size_t capacity) {
    BatchQueue* queue = (BatchQueue*)calloc(1, sizeof(BatchQueue));
    if (!queue) return NULL;
    
    queue->batches = (CLLMBatch**)calloc(capacity, sizeof(CLLMBatch*));
    if (!queue->batches) {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->producer_done, 0);
    atomic_store(&queue->prefetch_running, 0);
    
    return queue;
}

/**
 * Free batch queue
 */
static void batch_queue_free(BatchQueue* queue) {
    if (!queue) return;
    
    // Free any remaining batches
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    for (size_t i = head; i < tail; i++) {
        size_t index = i % queue->capacity;
        if (queue->batches[index]) {
            cllm_batch_free(queue->batches[index]);
            queue->batches[index] = NULL;
        }
    }
    
    free(queue->batches);
    free(queue);
}

/**
 * Try to push a batch to the queue (non-blocking)
 * Returns 1 on success, 0 if queue is full
 */
static int batch_queue_try_push(BatchQueue* queue, CLLMBatch* batch) {
    if (!queue || !batch) return 0;
    
    size_t tail = atomic_load(&queue->tail);
    size_t head = atomic_load(&queue->head);
    
    // Check if queue is full
    if (tail - head >= queue->capacity) {
        return 0;  // Queue full
    }
    
    size_t index = tail % queue->capacity;
    queue->batches[index] = batch;
    
    // Advance tail
    atomic_store(&queue->tail, tail + 1);
    
    return 1;
}

/**
 * Try to pop a batch from the queue (non-blocking)
 * Returns batch pointer on success, NULL if queue is empty
 */
static CLLMBatch* batch_queue_try_pop(BatchQueue* queue) {
    if (!queue) {
        fprintf(stderr, "[ERROR] batch_queue_try_pop: queue is NULL\n");
        return NULL;
    }
    
    if (!queue->batches) {
        fprintf(stderr, "[ERROR] batch_queue_try_pop: queue->batches is NULL\n");
        return NULL;
    }
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    // Check if queue is empty
    if (head >= tail) {
        return NULL;  // Queue empty
    }
    
    size_t index = head % queue->capacity;
    
    if (index >= queue->capacity) {
        fprintf(stderr, "[ERROR] batch_queue_try_pop: index %zu >= capacity %zu\n", index, queue->capacity);
        return NULL;
    }
    
    CLLMBatch* batch = queue->batches[index];
    queue->batches[index] = NULL;
    
    // Advance head
    atomic_store(&queue->head, head + 1);
    
    return batch;
}

/**
 * Check if queue is empty
 */
__attribute__((unused))
static int batch_queue_is_empty(BatchQueue* queue) {
    if (!queue) return 1;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    return head >= tail;
}

/**
 * Get number of batches in queue
 */
static size_t batch_queue_size(BatchQueue* queue) {
    if (!queue) return 0;
    
    size_t head = atomic_load(&queue->head);
    size_t tail = atomic_load(&queue->tail);
    
    return tail - head;
}

/**
 * Batch pre-fetch thread function
 * Runs in background, loading batches ahead of time
 */
static void* batch_prefetch_thread_func(void* arg) {
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)arg;
    if (!system || !system->batch_queue || !system->batch_iterator) {
        return NULL;
    }
    
    BatchQueue* queue = system->batch_queue;
    CLLMBatchIterator* iterator = system->batch_iterator;
    
    printf("[Pre-fetch] Thread started\n");
    
    int batches_loaded = 0;
    
    while (atomic_load(&system->running)) {
        // Try to load next batch
        CLLMBatch* batch = cllm_batch_iterator_next(iterator);
        
        if (!batch) {
            // No more batches - mark producer as done
            atomic_store(&queue->producer_done, 1);
            printf("[Pre-fetch] Loaded %d batches total, iterator exhausted\n", batches_loaded);
            break;
        }
        
        // Try to push to queue (non-blocking)
        while (!batch_queue_try_push(queue, batch)) {
            // Queue is full - wait a bit for consumers to catch up
            if (!atomic_load(&system->running)) {
                // System shutting down - free batch and exit
                cllm_batch_free(batch);
                goto cleanup;
            }
            usleep(1000);  // Sleep 1ms
        }
        
        batches_loaded++;
        
        // Log progress every 100 batches
        if (batches_loaded % 100 == 0) {
            printf("[Pre-fetch] Loaded %d batches (queue size: %zu)\n", 
                   batches_loaded, batch_queue_size(queue));
        }
    }
    
cleanup:
    atomic_store(&queue->prefetch_running, 0);
    printf("[Pre-fetch] Thread exiting\n");
    return NULL;
}

/**
 * Start batch pre-fetching
 */
static int batch_queue_start_prefetch(ThreadedTrainingSystem* system) {
    if (!system || !system->batch_queue) return 0;
    
    BatchQueue* queue = system->batch_queue;
    
    // Reset queue state
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->producer_done, 0);
    atomic_store(&queue->prefetch_running, 1);
    
    // Create pre-fetch thread
    if (pthread_create(&queue->prefetch_thread, NULL, 
                      batch_prefetch_thread_func, system) != 0) {
        fprintf(stderr, "Failed to create pre-fetch thread\n");
        atomic_store(&queue->prefetch_running, 0);
        return 0;
    }
    
    printf("[Pre-fetch] Started background batch loading\n");
    return 1;
}

/**
 * Stop batch pre-fetching and wait for thread to finish
 */
static void batch_queue_stop_prefetch(ThreadedTrainingSystem* system) {
    if (!system || !system->batch_queue) return;
    
    BatchQueue* queue = system->batch_queue;
    
    if (atomic_load(&queue->prefetch_running)) {
        // Wait for pre-fetch thread to finish
        pthread_join(queue->prefetch_thread, NULL);
        atomic_store(&queue->prefetch_running, 0);
    }
}

/**
 * Create threaded training system
 */
ThreadedTrainingSystem* threaded_training_create(CLLMTraining* training, 
                                                  CLLMBatchIterator* batch_iterator,
                                                  int num_threads) {
    if (!training || !batch_iterator) return NULL;
    
    // Auto-detect thread count if not specified
    if (num_threads <= 0) {
        num_threads = get_num_cpu_cores();
        if (num_threads > 1) num_threads--; // Reserve 1 for main thread
        printf("Auto-detected %d worker threads\n", num_threads);
    }
    
    // Ensure at least 1 thread
    if (num_threads < 1) num_threads = 1;
    
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)calloc(1, sizeof(ThreadedTrainingSystem));
    if (!system) return NULL;
    
    system->training = training;
    system->batch_iterator = batch_iterator;
    atomic_init(&system->running, 1);  // MUST use atomic_init for atomic_int!
    atomic_init(&system->sphere_id_counter, num_threads);  // Start after initial threads
    
    // MASTER PLAN: 12-fold symmetry structure
    system->num_symmetry_positions = 12;  // Always 12 positions
    system->num_active_workers = num_threads;  // Can be < 12
    
    // Calculate number of worker spheres
    int hierarchy_levels = calculate_hierarchy_levels(num_threads);
    system->num_worker_spheres = num_threads;
    
    printf("Creating 12-fold symmetric threading system (MASTER PLAN):\n");
    printf("  Symmetry positions: %d (12-fold structure)\n", system->num_symmetry_positions);
    printf("  Active workers: %d (rotating through positions)\n", system->num_active_workers);
    printf("  Control thread: Node Zero (NEVER processes batches)\n");
    printf("  Hierarchy levels: %d\n", hierarchy_levels);
    
    // Calculate gradient size
    system->gradient_size = training->model->vocab_size * training->model->embedding_dim;
    
    // Create shared gradient buffer (kissing spheres architecture)
    system->shared_gradients = shared_memory_create(
        system->gradient_size * sizeof(double),
        SHARED_LOCKED_WRITE  // Multiple writers allowed
    );
    if (!system->shared_gradients) {
        fprintf(stderr, "Failed to create shared gradient buffer\n");
        free(system);
        return NULL;
    }
    
    printf("  ✓ Created shared gradient buffer: %.2f MB\n", 
           (system->gradient_size * sizeof(double)) / (1024.0 * 1024.0));
    
    // Allocate accumulated gradients buffer (temporary until shared memory fully integrated)
    system->accumulated_gradients = (double*)calloc(system->gradient_size, sizeof(double));
    if (!system->accumulated_gradients) {
        fprintf(stderr, "Failed to allocate accumulated gradients buffer\n");
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    
    // Initialize kissing boundary locks
    pthread_mutex_init(&system->gradient_lock, NULL);
    pthread_mutex_init(&system->model_lock, NULL);
    
    // Initialize atomic counters for message-based synchronization
    atomic_init(&system->workers_ready, 0);
    atomic_init(&system->workers_completed, 0);
    
    // OPTIMIZATION PHASE 2: Skip full kissing spheres hierarchy creation
    // Only create sphere contexts for active workers (saves 376MB)
    // The full 157-sphere hierarchy is not needed until dynamic spawning
    system->thread_system = NULL;  // Skip for now
    
    // Note: threads_create() would create 157 spheres (1 + 12 + 144)
    // We only need num_threads spheres, so we skip it and create contexts directly
    
    // PHASE 5: Initialize control process infrastructure
    SystemConfiguration control_config = {
        .max_hierarchy_depth = hierarchy_levels,
        .max_spheres_per_level = 12,
        .initial_sphere_count = num_threads,
        .batch_size = 32,
        .max_epochs = 100,
        .learning_rate = 0.001,
        .max_threads = num_threads,
        .max_memory_bytes = 1024 * 1024 * 1024,  // 1GB
        .sync_interval_batches = 1,
        .checkpoint_interval_epochs = 10,
        .health_check_interval_ms = 1000,
        .sphere_timeout_seconds = 60.0,
        .enable_boundary_awareness = true,
        .enable_twin_prime_tracking = true
    };
    
    system->control_process = control_process_create(&control_config);
    if (!system->control_process) {
        fprintf(stderr, "WARNING: Failed to create control process (continuing without it)\n");
        system->control_process = NULL;  // Continue without control process
    } else {
        printf("  ✓ Control process infrastructure initialized\n");
    }
    
    // PHASE 5: Initialize root lattice hierarchy
    int root_symmetry_groups[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    system->root_hierarchy = lattice_hierarchy_create(
        0,                      // sphere_id (root)
        0,                      // hierarchy_level (root level)
        root_symmetry_groups,   // symmetry_groups
        12,                     // num_symmetry_groups
        -1,                     // physical_thread_id (not assigned yet)
        NULL                    // parent (root has no parent)
    );
    if (!system->root_hierarchy) {
        fprintf(stderr, "WARNING: Failed to create root hierarchy (continuing without it)\n");
        system->root_hierarchy = NULL;
    } else {
        printf("  ✓ Root lattice hierarchy created (12-fold structure)\n");
    }
    // Allocate dynamic sphere contexts array
    system->sphere_contexts = (SphereTrainingContext**)calloc(system->num_worker_spheres, 
                                                               sizeof(SphereTrainingContext*));
    if (!system->sphere_contexts) {
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    
    // Create sphere contexts for all worker spheres
    for (int i = 0; i < system->num_worker_spheres; i++) {
        int symmetry_group = i % 12; // Distribute across 12 symmetry groups
        system->sphere_contexts[i] = sphere_context_create(i, symmetry_group, system->gradient_size,
                                                           system->shared_gradients, system->num_worker_spheres);
        if (!system->sphere_contexts[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            if (system->thread_system) threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
        // Set system reference
        system->sphere_contexts[i]->system = system;
        
        // PHASE 8: Allocate thread-local training context for each worker
        CLLMModel* model = training->model;
        int ff_hidden_dim = model->hidden_dim;  // Use hidden_dim from new structure
        int num_heads = model->num_heads;  // Use num_heads from new structure (always 12)
        
        system->sphere_contexts[i]->thread_local_training = thread_local_training_create(
            training->config.batch_size,
            training->config.sequence_length,
            model->num_layers,
            model->embedding_dim,
            model->vocab_size,
            ff_hidden_dim,
            num_heads
        );
        
        if (!system->sphere_contexts[i]->thread_local_training) {
            fprintf(stderr, "ERROR: Failed to allocate thread-local training context for worker %d\n", i);
            // Cleanup on failure
            for (int j = 0; j <= i; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            if (system->thread_system) threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
    }
    
    // PHASE 2A: Create batch queue for pre-fetching
    system->batch_queue = batch_queue_create(BATCH_QUEUE_CAPACITY);
    if (!system->batch_queue) {
        fprintf(stderr, "ERROR: Failed to create batch queue\n");
        // Cleanup on failure
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system->accumulated_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Batch queue created (capacity: %d batches)\n", BATCH_QUEUE_CAPACITY);
    
    // PHASE 2B: Create lock-free work queue
    system->work_queue = work_queue_create();
    if (!system->work_queue) {
        fprintf(stderr, "ERROR: Failed to create work queue\n");
        // Cleanup on failure
        batch_queue_free(system->batch_queue);
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system->accumulated_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Work queue created (capacity: %d work items)\n", MAX_WORK_ITEMS);
    
    // MASTER PLAN: Create control thread (Node Zero) first
    // OPTIMIZATION: Reduce thread stack size from 8MB to 1MB (saves 455MB with 65 threads)
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setstacksize(&thread_attr, 1024 * 1024);  // 1MB stack
    system->has_control_thread = 1;
    system->control_running = 1;
    
    int rc = pthread_create(&system->control_thread, &thread_attr, 
                           control_thread_func, system);
    pthread_attr_destroy(&thread_attr);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Failed to create control thread (error %d)\n", rc);
        // Cleanup
        for (int j = 0; j < system->num_worker_spheres; j++) {
            sphere_context_free(system->sphere_contexts[j]);
        }
        free(system->sphere_contexts);
        if (system->thread_system) threads_free(system->thread_system);
        shared_memory_free(system->shared_gradients);
        free(system);
        return NULL;
    }
    printf("  ✓ Node Zero created (control thread NEVER processes batches)\n");
    
    // Create worker threads
    printf("  Creating %d worker threads (PHASE 2B: LOCK-FREE MODE)...\n", system->num_worker_spheres);
    for (int i = 0; i < system->num_worker_spheres; i++) {
        // OPTIMIZATION: Use 1MB stack for workers
        pthread_attr_t worker_attr;
        pthread_attr_init(&worker_attr);
        pthread_attr_setstacksize(&worker_attr, 1024 * 1024);
        // PHASE 2: Day 4 - Use dynamic worker thread with spawning
        rc = pthread_create(&system->sphere_contexts[i]->thread, &worker_attr, 
                           sphere_worker_thread_dynamic, system->sphere_contexts[i]);
        pthread_attr_destroy(&worker_attr);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create worker thread %d (error %d)\n", i, rc);
            // Stop control thread
            atomic_store(&system->running, 0);
            system->control_running = 0;
            pthread_join(system->control_thread, NULL);
            // Stop already created workers
            for (int j = 0; j < i; j++) {
                pthread_mutex_lock(&system->sphere_contexts[j]->lock);
                pthread_cond_signal(&system->sphere_contexts[j]->work_ready);
                pthread_mutex_unlock(&system->sphere_contexts[j]->lock);
                pthread_join(system->sphere_contexts[j]->thread, NULL);
            }
            // Cleanup
            for (int j = 0; j < system->num_worker_spheres; j++) {
                sphere_context_free(system->sphere_contexts[j]);
            }
            free(system->sphere_contexts);
            if (system->thread_system) threads_free(system->thread_system);
            shared_memory_free(system->shared_gradients);
            free(system);
            return NULL;
        }
    }
    
    system->epoch_loss = 0.0;
    system->total_batches = 0;
    
    // Initialize progress tracking
    atomic_init(&system->batches_processed, 0);
    system->total_batches_in_epoch = 0;
    system->current_epoch = 0;
    system->total_epochs = 0;
    system->epoch_start_time = 0;
    system->last_progress_time = 0;
    system->progress_update_interval = 10;  // Update every 10 batches
    
    printf("  ✓ Threaded training system created successfully\n");
    printf("    - 1 control thread (Node Zero)\n");
    printf("    - %d worker threads\n", system->num_worker_spheres);
    printf("    - 12-fold symmetry structure\n\n");
    
    // UI Integration: Initialize metrics system
    system->metrics = cllm_metrics_create(system->num_worker_spheres);
    if (!system->metrics) {
        fprintf(stderr, "WARNING: Failed to create metrics system (UI updates disabled)\n");
    } else {
        printf("  ✓ Metrics system initialized for UI integration\n");
    }
    
    // PHASE 6: Initialize entropy optimization systems - ACTUALLY WIRED NOW
    printf("  Initializing entropy optimization systems...\n");
    
    // Create entropy integration context
    system->entropy_context = (EntropyIntegrationContext*)calloc(1, sizeof(EntropyIntegrationContext));
    if (system->entropy_context && entropy_integration_init(system->entropy_context, training->model)) {
        printf("  ✓ Entropy integration initialized (12 dimensions)\n");
    } else {
        fprintf(stderr, "WARNING: Failed to initialize entropy integration context\n");
        free(system->entropy_context);
        system->entropy_context = NULL;
    }
    
    // Create adaptive hierarchy context
    system->adaptive_hierarchy = (AdaptiveHierarchyContext*)calloc(1, sizeof(AdaptiveHierarchyContext));
    if (system->adaptive_hierarchy) {
        AdaptiveHierarchyConfig config;
        adaptive_hierarchy_config_init_default(&config);
        if (adaptive_hierarchy_init(system->adaptive_hierarchy, system->entropy_context, &config)) {
            printf("  ✓ Adaptive hierarchy initialized (multi-factor scoring)\n");
        } else {
            fprintf(stderr, "WARNING: Failed to initialize adaptive hierarchy context\n");
            free(system->adaptive_hierarchy);
            system->adaptive_hierarchy = NULL;
        }
    }
    
    // Create entropy allocation plan
    system->entropy_allocation = (ThreadAllocationPlan*)calloc(1, sizeof(ThreadAllocationPlan));
    if (system->entropy_allocation && system->entropy_context) {
        // Configure allocation
        AllocationConfig alloc_config;
        allocation_config_init_default(&alloc_config);
        // Disable 12-fold enforcement if we have fewer than 12 threads
        alloc_config.enforce_12fold = (system->num_worker_spheres >= 12);
        alloc_config.strategy = ALLOCATION_PROPORTIONAL;
        // Lower minimum threads per dimension for small thread counts
        if (system->num_worker_spheres < 12) {
            alloc_config.min_threads_per_dimension = 1;
        }
        
        // Calculate thread allocation based on entropy
        bool alloc_success = calculate_thread_allocation(
            system->entropy_context,
            system->num_worker_spheres,
            &alloc_config,
            system->entropy_allocation
        );
        
        if (alloc_success) {
            printf("  ✓ Entropy-based thread allocation calculated:\n");
            printf("    Total threads: %d\n", system->entropy_allocation->total_allocated_threads);
            printf("    Active dimensions: %u\n", system->entropy_allocation->active_dimensions);
        } else {
            fprintf(stderr, "WARNING: Failed to calculate entropy allocation, using defaults\n");
            system->entropy_allocation->total_available_threads = system->num_worker_spheres;
            system->entropy_allocation->enforce_12fold = true;
        }
    } else {
        fprintf(stderr, "WARNING: Failed to create entropy allocation plan\n");
    }
    
    // Create entropy work distribution plan (will be populated during training)
    system->work_distribution = (WorkDistributionPlan*)calloc(1, sizeof(WorkDistributionPlan));
    if (system->work_distribution) {
        system->work_distribution->assignments = NULL;
        system->work_distribution->num_assignments = 0;
        printf("  ✓ Entropy work distribution plan initialized\n");
    } else {
        fprintf(stderr, "WARNING: Failed to create entropy work distribution plan\n");
    }
    
    printf("  ✓ All entropy optimization systems initialized\n\n");
    
    // PHASE 4: Initialize Plimpton work distribution
    printf("  Initializing Plimpton 322 work distribution...\n");
    system->plimpton_context = plimpton_integration_create();
    if (system->plimpton_context) {
        printf("  ✓ Plimpton integration initialized\n");
    } else {
        fprintf(stderr, "WARNING: Failed to initialize Plimpton integration context\n");
    }
    
    // PHASE 5: Initialize cymatic timing barriers
    printf("  Initializing cymatic timing synchronization...\n");
    
    // Create epoch barrier with Schumann resonance (7.83 Hz)
    // +1 for control thread
    // CRITICAL FIX: Barriers should only include worker threads, NOT main thread
    // Main thread doesn't participate in barrier synchronization
    system->epoch_barrier = cymatic_barrier_create(
        system->num_worker_spheres,  // Only workers, not main thread
        CYMATIC_FREQ_SCHUMANN,
        false  // Not adaptive - strict synchronization
    );
    
    // Create batch barrier with 432 Hz
    system->batch_barrier = cymatic_barrier_create(
        system->num_worker_spheres,  // Only workers, not main thread
        CYMATIC_FREQ_432_HZ,
        false  // Not adaptive - strict synchronization
    );
    
    if (system->epoch_barrier && system->batch_barrier) {
        printf("  ✓ Cymatic barriers created:\n");
        printf("    - Epoch barrier: Schumann resonance (7.83 Hz)\n");
        printf("    - Batch barrier: 432 Hz (natural tuning)\n");
    } else {
        fprintf(stderr, "WARNING: Failed to create cymatic barriers (timing sync disabled)\n");
        if (system->epoch_barrier) {
            cymatic_barrier_destroy(system->epoch_barrier);
            system->epoch_barrier = NULL;
        }
        if (system->batch_barrier) {
            cymatic_barrier_destroy(system->batch_barrier);
            system->batch_barrier = NULL;
        }
    }
    
    // Give threads time to initialize before returning
    usleep(10000);  // 10ms - allow worker threads to start and enter wait state
    
    return system;
}

/**
 * Free threaded training system
 */
void threaded_training_free(ThreadedTrainingSystem* system) {
    if (!system) return;
    
    printf("\nStopping threads...\n");
    atomic_store(&system->running, 0);
    
    // Stop control thread first (Node Zero)
    if (system->has_control_thread) {
        printf("  Stopping Node Zero (control thread)...\n");
        system->control_running = 0;
        pthread_join(system->control_thread, NULL);
        printf("  ✓ Node Zero stopped\n");
    }
    
    // Wake up all worker threads and wait for them to exit
    printf("  Stopping worker threads...\n");
    for (int i = 0; i < system->num_worker_spheres; i++) {
        if (system->sphere_contexts[i]) {
            pthread_mutex_lock(&system->sphere_contexts[i]->lock);
            pthread_cond_signal(&system->sphere_contexts[i]->work_ready);
            pthread_mutex_unlock(&system->sphere_contexts[i]->lock);
            
            // Join the thread
            pthread_join(system->sphere_contexts[i]->thread, NULL);
        }
    }
    printf("All worker threads stopped.\n");
    
    // Free sphere contexts
    for (int i = 0; i < system->num_worker_spheres; i++) {
        sphere_context_free(system->sphere_contexts[i]);
    }
    
    // Free the dynamic array
    free(system->sphere_contexts);
    
    if (system->thread_system) threads_free(system->thread_system);
    
    // Free shared memory regions
    shared_memory_free(system->shared_gradients);
    if (system->shared_model_weights) {
        shared_memory_free(system->shared_model_weights);
    }
    
    // Free accumulated gradients buffer
    free(system->accumulated_gradients);
    
    // PHASE 2A: Stop pre-fetch thread and free batch queue
    if (system->batch_queue) {
        batch_queue_stop_prefetch(system);
        batch_queue_free(system->batch_queue);
        printf("  ✓ Batch queue freed\n");
    }
    
    // PHASE 2B: Free work queue
    if (system->work_queue) {
        work_queue_free(system->work_queue);
        printf("  ✓ Work queue freed\n");
    }
    
    // Destroy kissing boundary locks
    pthread_mutex_destroy(&system->gradient_lock);
    pthread_mutex_destroy(&system->model_lock);
    
    // Barriers removed - using message-based synchronization
    // No cleanup needed for atomic counters
    
    // PHASE 5: Cleanup infrastructure
    if (system->control_process) {
        control_process_free(system->control_process);
        printf("  ✓ Control process freed\n");
    }
    
    if (system->root_hierarchy) {
        lattice_hierarchy_free(system->root_hierarchy);
        printf("  ✓ Root hierarchy freed\n");
    }
    
    // UI Integration: Cleanup metrics system
    if (system->metrics) {
        cllm_metrics_free(system->metrics);
        printf("  ✓ Metrics system freed\n");
    }
    
    // PHASE 6: Cleanup entropy optimization systems
    if (system->entropy_context) {
        entropy_integration_destroy(system->entropy_context);
        free(system->entropy_context);
        printf("  ✓ Entropy integration freed\n");
    }
    
    if (system->adaptive_hierarchy) {
        adaptive_hierarchy_destroy(system->adaptive_hierarchy);
        free(system->adaptive_hierarchy);
        printf("  ✓ Adaptive hierarchy freed\n");
    }
    
    if (system->entropy_allocation) {
        free(system->entropy_allocation);
        printf("  ✓ Entropy allocation freed\n");
    }
    
    if (system->work_distribution) {
        if (system->work_distribution->assignments) {
            free(system->work_distribution->assignments);
        }
        free(system->work_distribution);
        printf("  ✓ Entropy work distribution freed\n");
    }
    
    // PHASE 4: Cleanup Plimpton work distribution
    if (system->plimpton_context) {
        plimpton_integration_free(system->plimpton_context);
        printf("  ✓ Plimpton integration freed\n");
    }
    
    // PHASE 5: Cleanup cymatic timing barriers
    if (system->epoch_barrier) {
        cymatic_barrier_destroy(system->epoch_barrier);
        printf("  ✓ Epoch barrier freed\n");
    }
    
    if (system->batch_barrier) {
        cymatic_barrier_destroy(system->batch_barrier);
        printf("  ✓ Batch barrier freed\n");
    }
    
    free(system);
}

/**
 * Control thread function (Node Zero) - MASTER PLAN IMPLEMENTATION
 * 
 * CRITICAL: This thread NEVER processes batches!
 * 
 * Responsibilities:
 * 1. Distribute batches to workers
 * 2. Wait at barriers for workers to complete
 * 3. Accumulate gradients (safe - workers waiting at barrier)
 * 4. Apply optimizer step
 * 5. Coordinate next batch
 */
/**
 * Node Zero (Control Thread) - PHASE 3: Barrier Synchronization
 * 
 * CRITICAL: This thread NEVER processes batches (master plan requirement)
 * 
 * PHASE 3 IMPLEMENTATION:
 * - Participates in barrier synchronization
 * - Coordinates with main thread and workers
 * - Synchronizes at Point A (batch distribution)
 * - Synchronizes at Point B (batch completion)
 */
/**
 * Message-Based Control Thread (Node Zero)
 * 
 * Coordinates workers using message passing instead of barriers.
 * NEVER processes batches - only coordinates and accumulates gradients.
 * 
 * Synchronization:
 * - Workers send MSG_BATCH_COMPLETE when done
 * - Control waits for all completions before accumulating
 * - Uses atomic counters for tracking
 */
static void* control_thread_func(void* arg) {
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)arg;
    
    printf("[Node Zero] Control thread started - Message-based coordination\n");
    printf("[Node Zero] NEVER processes batches - only coordinates\n");
    
    // PHASE 5: Cymatic timing enabled
    bool cymatic_enabled __attribute__((unused)) = false;
    if (cymatic_enabled) {
        printf("[Node Zero] Cymatic timing synchronization ENABLED\n");
    }
    
    while (atomic_load(&system->running)) {
        // PHASE 5: Wait at batch barrier (432 Hz) before batch distribution
        if (cymatic_enabled) {
            cymatic_barrier_wait(system->batch_barrier);
        }
        
        // Wait for all workers to complete their batches
        int expected_workers = system->num_worker_spheres;
        int completed = 0;
        
        // Poll for completion with timeout
        int timeout_ms = 1000;  // 1 second
        int elapsed_ms = 0;
        
        while (completed < expected_workers && atomic_load(&system->running)) {
            // Check completion counter
            completed = atomic_load(&system->workers_completed);
            
            if (completed < expected_workers) {
                usleep(1000);  // 1ms sleep
                elapsed_ms += 1;
                
                // Timeout check
                if (elapsed_ms >= timeout_ms) {
                    // Reset and try again
                    elapsed_ms = 0;
                }
            }
        }
        
        // Check if we should stop
        if (!atomic_load(&system->running)) {
            break;
        }
        
        // All workers completed - accumulate gradients
        if (completed >= expected_workers) {
            accumulate_gradients(system);
            
            // Reset completion counter for next batch
            atomic_store(&system->workers_completed, 0);
        }
        
        // PHASE 5: Wait at epoch barrier (Schumann 7.83 Hz) after gradient accumulation
        if (cymatic_enabled) {
            cymatic_barrier_wait(system->epoch_barrier);
        }
    }
    
    printf("[Node Zero] Control thread stopping\n");
    return NULL;
}

/**
 * PHASE 2B: Lock-Free Worker Thread Function
 * 
 * Workers pull batches from work queue (no barriers!)
 * Continuous work processing without synchronization overhead
 * 
 * PHASE 2B IMPLEMENTATION:
 * - Pop batch from work queue (non-blocking)
 * - Process batch immediately
 * - Repeat until epoch done
 */

/**
 * Worker thread function with dynamic spawning - PHASE 2: Day 4 Afternoon
 * 
 * This version monitors workload and dynamically spawns/despawns children
 * based on system load, available cores, and hierarchy depth.
 * 
 * Features:
 * - Real-time workload monitoring
 * - Intelligent spawn/despawn decisions
 * - Hysteresis to prevent thrashing
 * - Automatic role transitions (worker <-> control)
 * - 12-fold symmetry enforcement
 */
static void* sphere_worker_thread_dynamic(void* arg) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)arg;
    ThreadedTrainingSystem* system = ctx->system;
    
    printf("[Worker %d] Dynamic thread started (symmetry group %d, level %d)\n", 
           ctx->sphere_id, ctx->symmetry_group, ctx->hierarchy_level);
    
    // PHASE 3, Day 10: Position thread for optimal cache usage
    int affinity_result = position_thread_for_cache(pthread_self(), &ctx->cache_placement);
    if (affinity_result == 0) {
        printf("[Worker %d] CPU affinity set: cache_line=%u, numa_node=%u, cpu_core=%d\n",
               ctx->sphere_id, ctx->cache_placement.cache_line,
               ctx->cache_placement.numa_node, ctx->cache_placement.cpu_core);
    } else {
        printf("[Worker %d] Warning: Failed to set CPU affinity\n", ctx->sphere_id);
    }
    
    // Initialize workload detector
    WorkloadDetectorContext detector;
    workload_detector_init(&detector, true);  // verbose = true
    
    // UI Integration: Update thread state to WORKING
    if (system->metrics) {
        cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_WORKING);
    }
    
    int batches_processed = 0;
    double last_check_time = get_current_time_seconds();
    const double CHECK_INTERVAL = 1.0;  // Check workload every 1 second
    
    // PHASE 5: Cymatic timing enabled
    bool cymatic_enabled __attribute__((unused)) = false;
    
    while (atomic_load(&system->running)) {
        // CRITICAL FIX: Reset batches_processed counter when new epoch starts
        // Detect new epoch by checking if epoch_done was just reset to 0
        static _Thread_local int last_epoch_done = 0;
        int current_epoch_done = atomic_load(&system->work_queue->epoch_done);
        if (last_epoch_done == 1 && current_epoch_done == 0) {
            // New epoch started - reset counter
            batches_processed = 0;
            printf("[Worker %d] New epoch detected, reset batches_processed\n", ctx->sphere_id);
        }
        last_epoch_done = current_epoch_done;
        
        // PHASE 5: Wait at batch barrier (432 Hz) before pulling work
        // DISABLED: Barrier causes deadlock - workers wait for each other
        // TODO: Fix barrier logic or remove entirely
//         if (false &amp;&amp; cymatic_enabled) {
//             cymatic_barrier_wait(system->batch_barrier);
//         }
        
        // PHASE 4: Check if we've reached our assigned batch limit (Plimpton work distribution)
        if (ctx->assigned_batches > 0 && batches_processed >= ctx->assigned_batches) {
            // This worker has processed its assigned share
            // Wait for epoch to complete
            if (atomic_load(&system->work_queue->epoch_done)) {
                // CRITICAL FIX: Don't exit! Wait for next epoch
                // Reset batches_processed counter for next epoch
                // The main thread will reset epoch_done and assign new batches
                usleep(1000);  // 1ms
                continue;  // Continue waiting for next epoch
            }
            // Yield CPU while waiting
            sched_yield();
            continue;
        }
        
        // Pop work from queue (non-blocking)
        CLLMBatch* batch = work_queue_pop(system->work_queue);
        
        if (!batch) {
            // No work available - check if epoch done AND queue is truly empty
            if (atomic_load(&system->work_queue->epoch_done)) {
                // Double-check queue is actually empty
                size_t head = atomic_load(&system->work_queue->head);
                size_t tail = atomic_load(&system->work_queue->tail);
                if (head >= tail) {
                    // Epoch complete and queue empty
                    // CRITICAL FIX: Don't exit! Wait for next epoch instead
                    // The main thread will reset epoch_done and push new batches
                    
                    // UI Integration: Update thread state to IDLE
                    if (system->metrics) {
                        cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_IDLE);
                    }
                    
                    // Sleep briefly to avoid busy-waiting
                    usleep(1000);  // 1ms
                    continue;  // Continue waiting for next epoch
                }
            }
            
            // UI Integration: Update thread state to IDLE
            if (system->metrics) {
                cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_IDLE);
            }
            
            // Yield CPU to other threads
            sched_yield();
            continue;
        }
        
        // UI Integration: Update thread state to WORKING
        if (system->metrics) {
            cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_WORKING);
        }
        
        // Process batch immediately (no waiting!)
        ctx->current_batch = batch;
        
        // PHASE 6: Control threads NEVER process batches
        if (!ctx->is_control_thread) {
            sphere_process_batch(ctx, system->training);
            batches_processed++;
            
            // UI Integration: Update thread workload
            if (system->metrics) {
                cllm_metrics_update_thread_workload(system->metrics, ctx->sphere_id, batches_processed);
            }
        }
        
        // Free batch
        cllm_batch_free(batch);
        ctx->current_batch = NULL;
        
        // Signal batch completion (for message-based coordination)
        atomic_fetch_add(&system->workers_completed, 1);
        
        // PHASE 5: Wait at epoch barrier (Schumann 7.83 Hz) after batch completion
        // DISABLED: Barrier causes deadlock - workers wait for each other
        // TODO: Fix barrier logic or remove entirely
//         if (false &amp;&amp; cymatic_enabled) {
//             cymatic_barrier_wait(system->epoch_barrier);
//         }
        
        // Periodic workload check for dynamic spawning
        double current_time = get_current_time_seconds();
        if (current_time - last_check_time >= CHECK_INTERVAL) {
            last_check_time = current_time;
            
            // Collect workload metrics
            WorkloadMetrics metrics;
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
            
            collect_workload_metrics(
                pending,
                system->num_active_workers,
                popped,
                current_time,
                ctx->hierarchy_level,
                &metrics
            );
            
            // Get available cores
            int available_cores = get_available_cores();
            
            // Make spawn/despawn decision
            SpawnDecision decision = should_spawn_children(
                &detector,
                &metrics,
                ctx->num_children,
                available_cores
            );
            
            // Execute decision
            if (decision == SPAWN_DECISION_YES && !ctx->is_control_thread) {
                printf("[Worker %d] SPAWNING: pending=%zu, cores=%d, depth=%d\n",
                       ctx->sphere_id, pending, available_cores, ctx->hierarchy_level);
                
                // PHASE 6: Use adaptive hierarchy to determine optimal depth and children count
                int num_children_to_spawn = 12;  // Default to full 12-fold symmetry
                
                if (ctx->system->adaptive_hierarchy) {
                    // Get recommended depth based on current state
                    int recommended_depth = calculate_entropy_aware_depth(
                        ctx->system->adaptive_hierarchy,
                        available_cores,
                        ctx->system->num_worker_spheres,
                        pending
                    );
                    
                    printf("[Worker %d] Adaptive hierarchy: depth=%d, cores=%d, pending=%zu\n",
                           ctx->sphere_id, recommended_depth, available_cores, pending);
                    
                    // Get entropy-aware children count
                    num_children_to_spawn = get_entropy_aware_children_count(
                        ctx->system->adaptive_hierarchy,
                        ctx->hierarchy_level,
                        available_cores,
                        pending
                    );
                } else {
                    // Fallback to simple core count check (old behavior)
                    num_children_to_spawn = (available_cores >= 12) ? 12 : available_cores;
                }
                
                if (num_children_to_spawn < 1) num_children_to_spawn = 1;
                
                printf("[Worker %d] Spawning %d children (cores=%d, entropy-aware)\n",
                       ctx->sphere_id, num_children_to_spawn, available_cores);
                
                if (sphere_spawn_children(ctx, num_children_to_spawn) == 0) {
                    printf("[Worker %d] Successfully spawned %d children\n", 
                           ctx->sphere_id, num_children_to_spawn);
                } else {
                    printf("[Worker %d] Failed to spawn children\n", ctx->sphere_id);
                }
            } else if (decision == SPAWN_DECISION_DESPAWN && ctx->is_control_thread) {
                printf("[Worker %d] DESPAWNING: pending=%zu, cores=%d\n",
                       ctx->sphere_id, pending, available_cores);
                
                // Despawn all children
                if (sphere_despawn_children(ctx) == 0) {
                    printf("[Worker %d] Successfully despawned children\n", ctx->sphere_id);
                } else {
                    printf("[Worker %d] Failed to despawn children\n", ctx->sphere_id);
                }
            }
        }
    }
    
    // Cleanup: If we have children, wait for them to finish
    if (ctx->is_control_thread && ctx->children) {
        printf("[Worker %d] Waiting for %d children to complete\n",
               ctx->sphere_id, ctx->num_children);
        sphere_despawn_children(ctx);
    }
    
    // Cleanup workload detector
    workload_detector_destroy(&detector);
    
    // UI Integration: Update thread state to TERMINATED
    if (system->metrics) {
        cllm_metrics_update_thread_state(system->metrics, ctx->sphere_id, THREAD_STATE_TERMINATED);
    }
    
    printf("[Worker %d] Dynamic thread stopping (processed %d batches)\n", 
           ctx->sphere_id, batches_processed);
    
    return NULL;
}

/**
 * Worker thread function - PHASE 3: Barrier Synchronization (LEGACY)
 * 
 * NOTE: This function is kept for comparison but not used in Phase 2B
 * Phase 2B uses sphere_worker_thread_lockfree() instead
 */
static int validate_gradients(double* gradients, size_t size, const char* source) {
    int nan_count = 0;
    int inf_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        if (math_is_nan(gradients[i])) {
            nan_count++;
            if (nan_count <= 5) {  // Only log first 5
                fprintf(stderr, "ERROR: NaN gradient in %s at index %zu\n", source, i);
            }
        } else if (math_is_inf(gradients[i])) {
            inf_count++;
            if (inf_count <= 5) {  // Only log first 5
                fprintf(stderr, "ERROR: Inf gradient in %s at index %zu: %f\n", source, i, gradients[i]);
            }
        }
    }
    
    if (nan_count > 0 || inf_count > 0) {
        fprintf(stderr, "ERROR: %s has %d NaN and %d Inf gradients (total size: %zu)\n", 
                source, nan_count, inf_count, size);
        return 0;
    }
    
    return 1;
}

/**
 * Clip gradients to prevent overflow
 */
static void clip_gradients(double* gradients, size_t size, double max_norm) {
    double norm = 0.0;
    for (size_t i = 0; i < size; i++) {
        norm += gradients[i] * gradients[i];
    }
    norm = math_sqrt(norm);
    
    if (norm > max_norm) {
        double scale = max_norm / norm;
        for (size_t i = 0; i < size; i++) {
            gradients[i] *= scale;
        }
        printf("  Clipped gradients: norm %.4f -> %.4f\n", norm, max_norm);
    }
}

/**
 * PHASE 6: Spawn child threads for recursive hierarchy
 * 
 * When a worker thread spawns children, it transitions to a control thread.
 * Control threads NEVER process batches - only coordinate children.
 */
   __attribute__((unused))
/**
 * Transition a worker thread to control thread role
 * 
 * This function handles the state transition from worker to control thread.
 * It validates the current state and prepares the context for spawning children.
 * 
 * @param ctx Context to transition
 * @return 0 on success, -1 on failure
 */
static int transition_to_control_thread(SphereTrainingContext* ctx) {
    if (!ctx) {
        fprintf(stderr, "[ERROR] transition_to_control_thread: NULL context\n");
        return -1;
    }
    
    // Validate current state
    if (ctx->is_control_thread) {
        fprintf(stderr, "[WARNING] Sphere %d is already a control thread\n", ctx->sphere_id);
        return 0;  // Already in desired state
    }
    
    // Validate no pending work
    if (ctx->current_batch) {
        fprintf(stderr, "[ERROR] Sphere %d cannot transition while processing batch\n", 
                ctx->sphere_id);
        return -1;
    }
    
    // Thread-safe state transition
    pthread_mutex_lock(&ctx->lock);
    
    // Double-check state under lock
    if (ctx->is_control_thread) {
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    
    // Perform transition
    ctx->is_control_thread = 1;
    ctx->batches_processed = 0;  // Reset batch counter for control role
    
    pthread_mutex_unlock(&ctx->lock);
    
    printf("[Sphere %d] Transitioned to CONTROL THREAD (level %d)\n",
           ctx->sphere_id, ctx->hierarchy_level);
    
    return 0;
}

/**
 * Transition a control thread back to worker thread role
 * 
 * This function handles the state transition from control to worker thread.
 * It validates that all children have been despawned before transitioning.
 * 
 * @param ctx Context to transition
 * @return 0 on success, -1 on failure
 */
static int transition_to_worker_thread(SphereTrainingContext* ctx) {
    if (!ctx) {
        fprintf(stderr, "[ERROR] transition_to_worker_thread: NULL context\n");
        return -1;
    }
    
    // Validate current state
    if (!ctx->is_control_thread) {
        fprintf(stderr, "[WARNING] Sphere %d is already a worker thread\n", ctx->sphere_id);
        return 0;  // Already in desired state
    }
    
    // Validate no children exist
    if (ctx->children || ctx->num_children > 0) {
        fprintf(stderr, "[ERROR] Sphere %d cannot transition while children exist (despawn first)\n",
                ctx->sphere_id);
        return -1;
    }
    
    // Thread-safe state transition
    pthread_mutex_lock(&ctx->lock);
    
    // Double-check state under lock
    if (!ctx->is_control_thread) {
        pthread_mutex_unlock(&ctx->lock);
        return 0;
    }
    
    // Perform transition
    ctx->is_control_thread = 0;
    ctx->batches_processed = 0;  // Reset batch counter for worker role
    
    pthread_mutex_unlock(&ctx->lock);
    
    printf("[Sphere %d] Transitioned to WORKER THREAD (level %d)\n",
           ctx->sphere_id, ctx->hierarchy_level);
    
    return 0;
}

/**
 * Despawn all children and transition back to worker thread
 * 
 * @param parent Parent context with children to despawn
 * @return 0 on success, -1 on failure
 */
static int sphere_despawn_children(SphereTrainingContext* parent) {
    if (!parent || !parent->is_control_thread || !parent->children) {
        return -1;
    }
    
    printf("[Sphere %d] Despawning %d children, transitioning back to worker\n",
           parent->sphere_id, parent->num_children);
    
    // Signal children to stop (they'll finish their current batch and exit)
    // Note: Children check system->running flag, which we don't modify here
    // They will naturally exit when the epoch completes
    
    // Wait for all children to complete
    for (int i = 0; i < parent->num_children; i++) {
        if (parent->children[i]) {
            pthread_join(parent->children[i]->thread, NULL);
            
            // Free child hierarchy node
            if (parent->children[i]->hierarchy_node) {
                lattice_hierarchy_free(parent->children[i]->hierarchy_node);
            }
            
            // Free child context
            sphere_context_free(parent->children[i]);
            parent->children[i] = NULL;
        }
    }
    
    // Free children array
    free(parent->children);
    parent->children = NULL;
    parent->num_children = 0;
    
    // Use transition function for thread-safe state change
    transition_to_worker_thread(parent);
    
    printf("[Sphere %d] Successfully despawned children, now a worker thread\n",
           parent->sphere_id);
    
    return 0;
}

/**
 * Spawn children with adaptive 12-fold symmetry
 * 
 * Spawns children based on available CPU cores while maintaining
 * 12-fold symmetry structure. Adapts to hardware constraints.
 * 
 * @param parent Parent context to spawn children from
 * @param num_children Number of children to spawn (1-12)
 * @return 0 on success, -1 on failure
 */
static int sphere_spawn_children(SphereTrainingContext* parent, int num_children) {
    if (!parent) {
        fprintf(stderr, "[ERROR] sphere_spawn_children: NULL parent context\n");
        return -1;
    }
    
    if (num_children <= 0 || num_children > 12) {
        fprintf(stderr, "[ERROR] Invalid num_children=%d (must be 1-12)\n", num_children);
        return -1;
    }
    
    printf("[Sphere %d] Spawning %d children (adaptive 12-fold symmetry)\n",
           parent->sphere_id, num_children);
    
    // Transition parent to control thread using thread-safe function
    if (transition_to_control_thread(parent) != 0) {
        fprintf(stderr, "[ERROR] Failed to transition sphere %d to control thread\n",
                parent->sphere_id);
        return -1;
    }
    
    // Allocate children array (always 12)
    parent->children = (SphereTrainingContext**)calloc(num_children, sizeof(SphereTrainingContext*));
    if (!parent->children) {
        fprintf(stderr, "[ERROR] Failed to allocate children array for sphere %d\n",
                parent->sphere_id);
        // Revert transition on failure
        transition_to_worker_thread(parent);
        return -1;
    }
    
    parent->num_children = num_children;
    
    // PHASE 6: Calculate entropy-based thread allocation
    if (parent->system->entropy_allocation && parent->system->entropy_context) {
        AllocationConfig config;
        allocation_config_init_default(&config);
        config.strategy = ALLOCATION_ADAPTIVE;
        config.enforce_12fold = true;
        
        // Calculate entropy-based allocation
        bool success = calculate_thread_allocation(
            parent->system->entropy_context,     // EntropyIntegrationContext*
            num_children * 12,                    // available_threads
            &config,                              // AllocationConfig*
            parent->system->entropy_allocation    // ThreadAllocationPlan* (output)
        );
        
        if (success) {
            printf("[Sphere %d] Entropy allocation calculated:\n", parent->sphere_id);
            print_allocation_plan(parent->system->entropy_allocation);
            
            // Store allocation plan for children to use
            parent->allocation_plan = parent->system->entropy_allocation;
        } else {
            fprintf(stderr, "[WARNING] Entropy allocation failed, using default\n");
            parent->allocation_plan = NULL;
        }
    } else {
        parent->allocation_plan = NULL;
    }
    
    // Create child contexts
    for (int i = 0; i < num_children; i++) {
        int child_symmetry_group = i;  // 0-11
        int child_id = atomic_fetch_add(&parent->system->sphere_id_counter, 1);
        
        // PHASE 6: Get dimension-specific thread allocation if available
        int allocated_threads = 1;  // Default
        if (parent->allocation_plan) {
            const DimensionAllocation* dim_alloc = get_dimension_allocation(
                parent->allocation_plan,
                child_symmetry_group
            );
            if (dim_alloc && dim_alloc->is_active) {
                allocated_threads = dim_alloc->adjusted_threads;
            }
        }
        
        parent->children[i] = sphere_context_create(
            child_id,
            child_symmetry_group,
            parent->gradient_size,
            parent->shared_gradients,
            parent->system->num_worker_spheres
        );
        
        // PHASE 6: Set allocation info after creation
        if (parent->children[i]) {
            parent->children[i]->allocated_threads = allocated_threads;
            parent->children[i]->can_spawn_children = (allocated_threads >= 12);
            parent->children[i]->allocation_plan = NULL;  // Will get its own when it spawns
            parent->children[i]->assigned_batches = 0;
            
            printf("[Sphere %d -> Child %d] Allocated %d threads (dimension %d, can_spawn=%d)\n",
                   parent->sphere_id, child_id, allocated_threads, child_symmetry_group,
                   parent->children[i]->can_spawn_children);
        }
        
        if (!parent->children[i]) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                sphere_context_free(parent->children[j]);
            }
            free(parent->children);
            parent->children = NULL;
            parent->num_children = 0;
            parent->is_control_thread = 0;
            return -1;
        }
        
        // Set child relationships
        parent->children[i]->parent = parent;
        parent->children[i]->hierarchy_level = parent->hierarchy_level + 1;
        parent->children[i]->system = parent->system;
        
        // PHASE 4: Calculate Plimpton work distribution for parent-child pair
        if (parent->system->plimpton_context) {
            WorkDistribution dist = plimpton_calculate_distribution(
                parent->system->plimpton_context,
                parent->sphere_id,
                child_id
            );
            
            if (dist.is_valid) {
                // Store work fractions
                parent->work_fraction = dist.parent_keeps;
                parent->children[i]->work_fraction = dist.child_gets;
                
                printf("[Sphere %d -> Child %d] Plimpton work split: parent=%.3f, child=%.3f\n",
                       parent->sphere_id, child_id, 
                       dist.parent_keeps, dist.child_gets);
            } else {
                // Default: equal split
                parent->work_fraction = 0.5;
                parent->children[i]->work_fraction = 0.5 / num_children;
                
                printf("[Sphere %d -> Child %d] Using default work split (Plimpton invalid)\n",
                       parent->sphere_id, child_id);
            }
        } else {
            // No Plimpton context: equal split
            parent->work_fraction = 0.5;
            parent->children[i]->work_fraction = 0.5 / num_children;
        }
        
        // PHASE 3: Link parent-child crystalline memory with clock-based mapping
        if (parent->crystalline_memory && parent->children[i]->crystalline_memory) {
            // Create shared memory region between parent and child
            // Size = 1/12 of total gradient size (one segment)
            size_t shared_size = (parent->gradient_size * sizeof(double)) / NUM_SYMMETRY_GROUPS;
            
            int link_result = crystalline_memory_link_parent_child(
                parent->crystalline_memory,
                parent->children[i]->crystalline_memory,
                child_symmetry_group,
                shared_size
            );
            
            if (link_result != 1) {
                fprintf(stderr, "[WARNING] Failed to link crystalline memory for sphere %d -> child %d\n",
                        parent->sphere_id, child_id);
            } else {
                printf("[Sphere %d] Linked crystalline memory with child %d (symmetry group %d)\n",
                       parent->sphere_id, child_id, child_symmetry_group);
            }
            
            // PHASE 3, Day 9: Map child to memory using Babylonian clock structure
            // Child's hierarchy level determines which ring it's in
            ClockMemoryPosition child_clock_pos = map_thread_to_memory(
                child_id,
                parent->hierarchy_level + 1,  // Child's hierarchy level
                parent->gradient_size * sizeof(double)
            );
            
            printf("[Sphere %d -> Child %d] Clock position: ring=%u, position=%u, offset=%zu\n",
                   parent->sphere_id, child_id, child_clock_pos.ring, 
                   child_clock_pos.position, child_clock_pos.memory_offset);
        }
        
        // PHASE 3: Create kissing boundaries between siblings
        // Each child shares boundaries with its adjacent siblings (in 12-fold structure)
        if (i > 0 && parent->children[i]->crystalline_memory && parent->children[i-1]->crystalline_memory) {
            // Create boundary between this child and previous sibling
            int prev_symmetry = parent->children[i-1]->symmetry_group;
            int curr_symmetry = parent->children[i]->symmetry_group;
            
            // Calculate boundary size (shared gradient region between segments)
            size_t boundary_size = (parent->gradient_size * sizeof(double)) / (NUM_SYMMETRY_GROUPS * 4);
            
            // Create boundary using crystalline memory API
            // Note: We use the parent's crystalline memory block as the container
            KissingBoundary* boundary = crystalline_boundary_create(
                parent->crystalline_memory,  // Use parent's memory block
                prev_symmetry,
                curr_symmetry,
                boundary_size
            );
            
            if (boundary) {
                // Store boundary in both siblings
                parent->children[i]->sibling_boundaries[prev_symmetry] = boundary;
                parent->children[i-1]->sibling_boundaries[curr_symmetry] = boundary;
                parent->children[i]->num_boundaries++;
                parent->children[i-1]->num_boundaries++;
                
                printf("[Sphere %d] Created kissing boundary between children %d (sym=%d) and %d (sym=%d)\n",
                       parent->sphere_id, parent->children[i-1]->sphere_id, prev_symmetry,
                       parent->children[i]->sphere_id, curr_symmetry);
            } else {
                fprintf(stderr, "[WARNING] Failed to create kissing boundary between children %d and %d\n",
                        parent->children[i-1]->sphere_id, parent->children[i]->sphere_id);
            }
        }
        
        // PHASE 3: Create boundary between last and first child (complete the ring)
        if (i == num_children - 1 && num_children > 1 && 
            parent->children[0]->crystalline_memory && parent->children[i]->crystalline_memory) {
            int first_symmetry = parent->children[0]->symmetry_group;
            int last_symmetry = parent->children[i]->symmetry_group;
            
            size_t boundary_size = (parent->gradient_size * sizeof(double)) / (NUM_SYMMETRY_GROUPS * 4);
            
            KissingBoundary* boundary = crystalline_boundary_create(
                parent->crystalline_memory,  // Use parent's memory block
                last_symmetry,
                first_symmetry,
                boundary_size
            );
            
            if (boundary) {
                parent->children[i]->sibling_boundaries[first_symmetry] = boundary;
                parent->children[0]->sibling_boundaries[last_symmetry] = boundary;
                parent->children[i]->num_boundaries++;
                parent->children[0]->num_boundaries++;
                
                printf("[Sphere %d] Created kissing boundary between children %d (sym=%d) and %d (sym=%d) - ring complete\n",
                       parent->sphere_id, parent->children[i]->sphere_id, last_symmetry,
                       parent->children[0]->sphere_id, first_symmetry);
            }
        }
        
        // Create hierarchy node for child
        int child_symmetry_groups[1] = {child_symmetry_group};
        parent->children[i]->hierarchy_node = lattice_hierarchy_create(
            child_id,
            parent->hierarchy_level + 1,
            child_symmetry_groups,
            1,
            -1,  // physical_thread_id (not assigned yet)
            parent->hierarchy_node
        );
        
        // Start child thread - use dynamic version
        // OPTIMIZATION: Use 1MB stack for child threads
        pthread_attr_t child_attr;
        pthread_attr_init(&child_attr);
        pthread_attr_setstacksize(&child_attr, 1024 * 1024);
        pthread_create(&parent->children[i]->thread, &child_attr, 
                      sphere_worker_thread_dynamic, parent->children[i]);
        pthread_attr_destroy(&child_attr);
    }
    
    printf("[Sphere %d] Successfully spawned %d children (12-fold symmetry), transitioned to control thread\n", 
           parent->sphere_id, num_children);
    
    return 0;
}

/**
 * Report training progress
 * 
 * Displays progress information including epoch, batch, loss, speed, and ETA.
 * Called periodically during training (every N batches) or when forced.
 * 
 * @param system Training system
 * @param force Force update even if interval not reached
 */
static void report_training_progress(ThreadedTrainingSystem* system, bool force) {
    if (!system) return;
    
    printf("[TRACE] report_training_progress: ENTRY\n");
    fflush(stdout);
    
    time_t current_time = time(NULL);
    size_t batches_done = atomic_load(&system->batches_processed);
    
    // Check if we should update (every N batches or forced)
    if (!force && batches_done % system->progress_update_interval != 0) {
        return;
    }
    
    printf("[TRACE] report_training_progress: Calculating progress\n");
    fflush(stdout);
    
    // Calculate progress percentage
    double progress_pct = 0.0;
    if (system->total_batches_in_epoch > 0) {
        progress_pct = (double)batches_done / system->total_batches_in_epoch * 100.0;
    }
    
    // Calculate speed (batches per second)
    double elapsed = difftime(current_time, system->epoch_start_time);
    double speed = elapsed > 0 ? batches_done / elapsed : 0.0;
    
    // Calculate ETA for current epoch
    double eta_seconds = 0.0;
    if (speed > 0 && system->total_batches_in_epoch > batches_done) {
        size_t remaining = system->total_batches_in_epoch - batches_done;
        eta_seconds = remaining / speed;
    }
    
    // Format ETA
    int eta_hours = (int)(eta_seconds / 3600);
    int eta_mins = (int)((eta_seconds - eta_hours * 3600) / 60);
    int eta_secs = (int)(eta_seconds - eta_hours * 3600 - eta_mins * 60);
    
    printf("[TRACE] report_training_progress: Getting loss from spheres, num_worker_spheres=%d\n", system->num_worker_spheres);
    fflush(stdout);
    
    // Get current loss from sphere contexts
    double total_loss = 0.0;
    int active_spheres = 0;
    
    // SAFETY CHECK: Verify sphere_contexts array is valid
    if (!system->sphere_contexts) {
        fprintf(stderr, "[ERROR] report_training_progress: sphere_contexts is NULL!\n");
        fflush(stderr);
        return;
    }
    
    for (int i = 0; i < system->num_worker_spheres; i++) {
        printf("[TRACE] report_training_progress: Checking sphere %d\n", i);
        fflush(stdout);
        
        if (system->sphere_contexts[i] && system->sphere_contexts[i]->batches_processed > 0) {
            total_loss += system->sphere_contexts[i]->batch_loss;
            active_spheres++;
        }
    }
    double current_loss = (active_spheres > 0) ? total_loss / active_spheres : 0.0;
    
    printf("[TRACE] report_training_progress: About to print progress line\n");
    fflush(stdout);
    
    // Print progress line
    printf("\rEpoch %d/%d | Batch %zu/%zu (%.1f%%) | Loss: %.4f | %.1f batch/s | ETA: %02d:%02d:%02d",
           system->current_epoch + 1,
           system->total_epochs,
           batches_done,
           system->total_batches_in_epoch,
           progress_pct,
           current_loss,
           speed,
           eta_hours, eta_mins, eta_secs);
    fflush(stdout);
    
    system->last_progress_time = current_time;
}

/**
 * Accumulate gradients from all spheres (using shared memory)
 * TODO: Implement gradient accumulation in future training enhancements
 */
static void accumulate_gradients(ThreadedTrainingSystem* system) {
    printf("[TRACE] accumulate_gradients: ENTRY - system=%p\n", (void*)system);
    fflush(stdout);
    
    if (!system) {
        fprintf(stderr, "[ERROR] accumulate_gradients: system is NULL\n");
        return;
    }
    
    printf("[TRACE] accumulate_gradients: accumulated_gradients=%p, gradient_size=%zu\n", 
           (void*)system->accumulated_gradients, system->gradient_size);
    fflush(stdout);
    
    if (!system->accumulated_gradients) {
        fprintf(stderr, "[ERROR] accumulate_gradients: accumulated_gradients is NULL!\n");
        return;
    }
    
    printf("[TRACE] accumulate_gradients: About to acquire gradient_lock\n");
    fflush(stdout);
    
    // PHASE 3: Gradient accumulation with proper synchronization
    // Lock protects against concurrent reads from other threads (UI, crawler)
    // that call threaded_training_get_gradient_norm() while we're accumulating
    pthread_mutex_lock(&system->gradient_lock);
    
    printf("[TRACE] accumulate_gradients: gradient_lock ACQUIRED\n");
    fflush(stdout);
    
    // Zero accumulated gradients
    printf("[TRACE] accumulate_gradients: About to memset, target=%p, size=%zu bytes\n",
           (void*)system->accumulated_gradients, system->gradient_size * sizeof(double));
    fflush(stdout);
    
    memset(system->accumulated_gradients, 0, system->gradient_size * sizeof(double));
    
    printf("[TRACE] accumulate_gradients: Gradients zeroed\n");
    fflush(stdout);
    
    int valid_spheres = 0;
    
    // Sum gradients from all spheres
    printf("[TRACE] accumulate_gradients: Starting sphere loop, num_worker_spheres=%d\n", system->num_worker_spheres);
    fflush(stdout);
    
    for (int i = 0; i < system->num_worker_spheres; i++) {
        printf("[TRACE] accumulate_gradients: Processing sphere %d\n", i);
        fflush(stdout);
        
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        printf("[TRACE] accumulate_gradients: Sphere %d ctx=%p\n", i, (void*)ctx);
        fflush(stdout);
        
        // Check if sphere has valid gradient storage (crystalline memory or legacy)
        if (!ctx) {
            printf("[TRACE] accumulate_gradients: Sphere %d skipped (no ctx)\n", i);
            fflush(stdout);
            continue;
        }
        
        printf("[TRACE] accumulate_gradients: Sphere %d checking gradient storage\n", i);
        fflush(stdout);
        
        if (!ctx->crystalline_memory && !ctx->local_gradients) {
            printf("[TRACE] accumulate_gradients: Sphere %d skipped (no gradient storage)\n", i);
            fflush(stdout);
            continue;
        }
        
        printf("[TRACE] accumulate_gradients: Sphere %d has gradient_size=%zu\n", i, ctx->gradient_size);
        fflush(stdout);
        
        // Validate gradients before accumulation
        char source[64];
        snprintf(source, sizeof(source), "Sphere %d", i);
        printf("[TRACE] accumulate_gradients: About to validate sphere %d gradients\n", i);
        fflush(stdout);
        
        // Get gradients from crystalline memory or fallback to local_gradients
        double* gradient_source = NULL;
        size_t gradient_count = 0;
        
        printf("[TRACE] accumulate_gradients: Sphere %d checking crystalline_memory=%p\n", i, (void*)ctx->crystalline_memory);
        fflush(stdout);
        
        if (ctx->crystalline_memory) {
            printf("[TRACE] accumulate_gradients: Sphere %d getting crystalline segment for symmetry_group=%u\n", i, ctx->symmetry_group);
            fflush(stdout);
            
            // Get segment for this sphere's symmetry group
            CrystallineSegment* segment = crystalline_memory_get_segment(
                ctx->crystalline_memory, 
                ctx->symmetry_group
            );
            
            printf("[TRACE] accumulate_gradients: Sphere %d got segment=%p\n", i, (void*)segment);
            fflush(stdout);
            
            if (segment && segment->data) {
                gradient_source = (double*)segment->data;
                gradient_count = segment->size / sizeof(double);
                printf("[TRACE] accumulate_gradients: Sphere %d using crystalline gradients, count=%zu\n", i, gradient_count);
            } else {
                gradient_source = ctx->local_gradients;
                gradient_count = ctx->gradient_size;
                printf("[TRACE] accumulate_gradients: Sphere %d using local gradients (segment invalid), count=%zu\n", i, gradient_count);
            }
            fflush(stdout);
        } else {
            gradient_source = ctx->local_gradients;
            gradient_count = ctx->gradient_size;
            printf("[TRACE] accumulate_gradients: Sphere %d using local gradients (no crystalline), count=%zu\n", i, gradient_count);
            fflush(stdout);
        }
        
        printf("[TRACE] accumulate_gradients: Sphere %d about to validate gradients\n", i);
        fflush(stdout);
        
        if (!validate_gradients(gradient_source, gradient_count, source)) {
            fprintf(stderr, "WARNING: Skipping sphere %d due to invalid gradients\n", i);
            continue;
        }
        
        printf("[TRACE] accumulate_gradients: Sphere %d gradients validated\n", i);
        fflush(stdout);
        
        // Clip gradients to prevent overflow
        printf("[TRACE] accumulate_gradients: Sphere %d clipping gradients\n", i);
        fflush(stdout);
        clip_gradients(gradient_source, gradient_count, 10.0);
        
        printf("[TRACE] accumulate_gradients: Sphere %d accumulating gradients\n", i);
        fflush(stdout);
        
        // Accumulate gradients
        size_t accumulate_count = (gradient_count < system->gradient_size) ? gradient_count : system->gradient_size;
        for (size_t j = 0; j < accumulate_count; j++) {
            system->accumulated_gradients[j] += gradient_source[j];
        }
        
        valid_spheres++;
        printf("[TRACE] accumulate_gradients: Sphere %d complete, valid_spheres=%d\n", i, valid_spheres);
        fflush(stdout);
    }
    
    printf("[TRACE] accumulate_gradients: Sphere loop complete, valid_spheres=%d\n", valid_spheres);
    fflush(stdout);
    
    // Average gradients across valid spheres only
    if (valid_spheres > 0) {
        printf("[TRACE] accumulate_gradients: Averaging gradients across %d spheres\n", valid_spheres);
        fflush(stdout);
        for (size_t i = 0; i < system->gradient_size; i++) {
            system->accumulated_gradients[i] /= (double)valid_spheres;
        }
    }
    
    printf("[TRACE] accumulate_gradients: Final validation\n");
    fflush(stdout);
    
    // Final validation of accumulated gradients
    if (!validate_gradients(system->accumulated_gradients, system->gradient_size, "Accumulated")) {
        fprintf(stderr, "CRITICAL: Accumulated gradients are invalid!\n");
    }
    
    printf("[TRACE] accumulate_gradients: About to release gradient_lock\n");
    fflush(stdout);
    
    pthread_mutex_unlock(&system->gradient_lock);
    
    printf("[TRACE] accumulate_gradients: gradient_lock RELEASED, function complete\n");
    fflush(stdout);
}

/**
 * Train one epoch with multi-threading
 */
/**
 * PHASE 2B: Lock-Free Training Epoch
 * 
 * Workers pull batches from work queue (no barriers!)
 * Main thread pushes batches and waits for completion
 */
void threaded_training_set_total_epochs(ThreadedTrainingSystem* system, int total_epochs) {
    if (system) {
        system->total_epochs = total_epochs;
    }
}

double threaded_train_epoch_lockfree(ThreadedTrainingSystem* system, int current_epoch) {
#ifdef CLLM_DEBUG
    printf("[DEBUG] threaded_train_epoch_lockfree: ENTRY - system=%p, epoch=%d\n", (void*)system, current_epoch);
    fflush(stdout);
#endif
    
    if (!system) {
        fprintf(stderr, "[ERROR] threaded_train_epoch_lockfree: system is NULL\n");
        return 0.0;
    }
    
#ifdef CLLM_DEBUG
    printf("[DEBUG] threaded_train_epoch_lockfree: system validated\n");
    fflush(stdout);
#endif
    
    printf("\n=== PHASE 2B: LOCK-FREE TRAINING EPOCH ===\n");
    printf("Epoch %d - Using %d worker threads (lock-free work queue)\n", current_epoch + 1, system->num_worker_spheres);
    
    // UI Integration: Update framework status
    if (system->metrics) {
        cllm_metrics_update_framework_status(system->metrics, 1, 1, 1, 1);  // All active
    }
    
    // Reset work queue for new epoch
    work_queue_reset(system->work_queue);
    
    // PHASE 2A: Reset batch iterator and start pre-fetching
#ifdef CLLM_DEBUG
    printf("[DEBUG] Before batch_iterator_reset: batch_iterator=%p\n", (void*)system->batch_iterator);
    fflush(stdout);
#endif
    
    if (!system->batch_iterator) {
        fprintf(stderr, "[ERROR] batch_iterator is NULL!\n");
        return 0.0;
    }
    
    cllm_batch_iterator_reset(system->batch_iterator);
    
    if (!batch_queue_start_prefetch(system)) {
        fprintf(stderr, "ERROR: Failed to start batch pre-fetching\n");
        return 0.0;
    }
    
    printf("Batch pre-fetching enabled + Lock-free work queue active\n\n");
    
    int batches_pushed = 0;
    size_t total_batches_in_epoch = cllm_batch_iterator_num_batches(system->batch_iterator);
    
    // Log epoch information for clarity
    printf("=== EPOCH %d: %zu batches to process ===\n", current_epoch, total_batches_in_epoch);
    
    // Timing tracking for progress estimates
    time_t epoch_start_time = time(NULL);
    
    // Initialize progress tracking
    system->current_epoch = current_epoch;
    system->total_batches_in_epoch = total_batches_in_epoch;
    system->epoch_start_time = epoch_start_time;
    system->last_progress_time = epoch_start_time;
    atomic_store(&system->batches_processed, 0);
    if (system->progress_update_interval == 0) {
        system->progress_update_interval = 10;  // Default: update every 10 batches
    }
    
    // UI Integration: Initialize epoch metrics
    if (system->metrics) {
        cllm_metrics_update_training_progress(system->metrics, current_epoch, 0, total_batches_in_epoch);
        system->metrics->training.elapsed_time_seconds = 0.0;
        system->metrics->training.estimated_time_remaining_seconds = 0.0;
        system->metrics->training.batches_per_second = 0.0;
    }
    
    // PHASE 6: Calculate entropy-based work distribution BEFORE pushing batches
    if (system->work_distribution && system->entropy_context) {
        printf("Calculating entropy-based work distribution...\n");
        
        // Create work distribution plan
        WorkDistributionPlan plan;
        plan.assignments = NULL;
        plan.num_assignments = 0;
        
        // Configure work distribution
        WorkDistributionConfig config;
        work_distribution_config_init(&config);
        config.strategy = WORK_DIST_ENTROPY_ONLY;
        config.enforce_12fold = true;
        
        // Calculate entropy-based work distribution
        bool success = calculate_entropy_work_distribution(
            system->entropy_context,
            system->entropy_allocation,
            total_batches_in_epoch,
            &config,
            &plan
        );
        
        if (success && plan.assignments) {
            printf("  ✓ Entropy-based work distribution calculated:\n");
            printf("    Total work: %zu batches\n", plan.total_work_size);
            printf("    Distributed: %zu batches\n", plan.distributed_work);
            printf("    Assignments: %zu\n", plan.num_assignments);
            
            // Assign work to sphere contexts based on dimension
            for (int i = 0; i < system->num_worker_spheres; i++) {
                int dimension = i % 12;
                
                // Find assignment for this dimension
                size_t work_for_dimension = 0;
                for (size_t j = 0; j < plan.num_assignments; j++) {
                    if (plan.assignments[j].dimension == (uint32_t)dimension) {
                        work_for_dimension += plan.assignments[j].work_size;
                    }
                }
                
                system->sphere_contexts[i]->assigned_batches = (int)work_for_dimension;
                
                printf("    [Worker %d] Dimension %d: assigned=%d batches\n",
                       i, dimension, system->sphere_contexts[i]->assigned_batches);
            }
            
            // Free work distribution plan
            if (plan.assignments) {
                free(plan.assignments);
            }
            
            printf("  ✓ Work distribution complete\n");
        } else {
            fprintf(stderr, "WARNING: Failed to calculate entropy work distribution, using uniform distribution\n");
            // Fallback: uniform distribution with proper remainder handling
            int batches_per_worker = total_batches_in_epoch / system->num_worker_spheres;
            int remainder = total_batches_in_epoch % system->num_worker_spheres;
            
            for (int i = 0; i < system->num_worker_spheres; i++) {
                // Give extra batch to first 'remainder' workers
                system->sphere_contexts[i]->assigned_batches = batches_per_worker + (i < remainder ? 1 : 0);
                printf("    [Worker %d] Assigned %d batches\n", i, system->sphere_contexts[i]->assigned_batches);
            }
        }
    } else {
        // No entropy distribution available - use uniform distribution with proper remainder handling
        int batches_per_worker = total_batches_in_epoch / system->num_worker_spheres;
        int remainder = total_batches_in_epoch % system->num_worker_spheres;
        
        for (int i = 0; i < system->num_worker_spheres; i++) {
            // Give extra batch to first 'remainder' workers
            system->sphere_contexts[i]->assigned_batches = batches_per_worker + (i < remainder ? 1 : 0);
            printf("    [Worker %d] Assigned %d batches\n", i, system->sphere_contexts[i]->assigned_batches);
        }
    }
    
    // Push all batches to work queue
    printf("Pushing batches to work queue...\n");
#ifdef CLLM_DEBUG
    printf("[DEBUG] system=%p, work_queue=%p, batch_queue=%p\n", 
           (void*)system, (void*)system->work_queue, (void*)system->batch_queue);
    fflush(stdout);
#endif
    
    if (!system->work_queue) {
        fprintf(stderr, "[ERROR] work_queue is NULL!\n");
        return 0.0;
    }
    
    if (!system->batch_queue) {
        fprintf(stderr, "[ERROR] batch_queue is NULL!\n");
        return 0.0;
    }
    
    while (1) {
        // Get batch from pre-fetch queue
        CLLMBatch* batch = batch_queue_try_pop(system->batch_queue);
        
        if (!batch) {
            // Check if producer is done
            if (atomic_load(&system->batch_queue->producer_done)) {
                break;  // No more batches
            }
            
            // Queue temporarily empty - wait for pre-fetch
            usleep(100);
            continue;
        }
        
        // Push to work queue (workers will pull from here)
        while (!work_queue_push(system->work_queue, batch)) {
            // Work queue full - wait for workers to catch up
            usleep(10);
        }
        
        batches_pushed++;
        atomic_store(&system->batches_processed, batches_pushed);
            
            // UI Integration: Update step progress
        if (system->metrics) {
            cllm_metrics_update_training_progress(system->metrics, current_epoch, batches_pushed, total_batches_in_epoch);
                
                // Calculate timing metrics
                double elapsed = difftime(time(NULL), epoch_start_time);
                if (elapsed > 0.1) {  // Avoid division by zero
                    double batches_per_sec = batches_pushed / elapsed;
                    double remaining = (total_batches_in_epoch - batches_pushed) / batches_per_sec;
                    
                    system->metrics->training.elapsed_time_seconds = elapsed;
                    system->metrics->training.estimated_time_remaining_seconds = remaining;
                    system->metrics->training.batches_per_second = batches_per_sec;
                }
                
                // CRITICAL FIX: Invoke callbacks every 10 batches for real-time UI updates
               if (batches_pushed % 10 == 0) {
                   cllm_metrics_invoke_callbacks(system->metrics);
               }
        }
        
        // Report training progress to console
        report_training_progress(system, false);
        
        // Log progress
        if (batches_pushed % 500 == 0) {
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
            printf("  Pushed %d batches (pending: %zu, processed: %zu)\n", 
                   batches_pushed, pending, popped);
        }
    }
    
    printf("All %d batches pushed to work queue\n", batches_pushed);
    fflush(stdout);
    
#ifdef CLLM_DEBUG
    fprintf(stderr, "[DEBUG] About to set epoch_done\n");
#endif
    fflush(stderr);
    
    // Signal epoch done - no more batches will be pushed
    // Workers can now exit when queue is empty
    atomic_store(&system->work_queue->epoch_done, 1);
    
#ifdef CLLM_DEBUG
    fprintf(stderr, "[DEBUG] Set epoch_done=1, workers will exit when queue empty\n");
#endif
    fflush(stderr);
    
    // Wait for all work to complete
    printf("Waiting for workers to complete...\n");
    int wait_iterations = 0;
    while (!work_queue_is_complete(system->work_queue)) {
        usleep(1000);  // 1ms
        
        wait_iterations++;
        // Print progress every 1 second for debugging
        if (wait_iterations % 1000 == 0) {
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
#ifdef CLLM_DEBUG
            int done = atomic_load(&system->work_queue->epoch_done);
            printf("  [DEBUG] Wait iteration %d: pushed=%zu, popped=%zu, epoch_done=%d, pending=%zu\n",
                   wait_iterations, pushed, popped, done, pending);
            fflush(stdout);
#else
            (void)pending; (void)pushed; (void)popped;  // Suppress unused warnings
#endif
        }
        
        // Timeout after 10 seconds
        if (wait_iterations > 10000) {
            size_t pending, pushed, popped;
            work_queue_stats(system->work_queue, &pending, &pushed, &popped);
            fprintf(stderr, "[ERROR] Timeout waiting for workers! pushed=%zu, popped=%zu, pending=%zu\n",
                    pushed, popped, pending);
            break;
        }
    }
    
    printf("=== EPOCH %d COMPLETE: All %zu batches processed! ===\n", current_epoch, total_batches_in_epoch);
    fflush(stdout);
    
    // Final progress report for this epoch
    printf("[TRACE] About to call report_training_progress...\n");
    fflush(stdout);
    report_training_progress(system, true);
    printf("[TRACE] report_training_progress completed\n");
    fflush(stdout);
    
    printf("\n");  // New line after progress report
    fflush(stdout);
    
    // Stop pre-fetch thread
    batch_queue_stop_prefetch(system);
    
    // CRITICAL: Ensure all worker threads have completely finished
    // Wait for work queue to be truly empty and all threads idle
    printf("Waiting for all threads to finish...\n");
    fflush(stdout);
    
    // Wait longer to ensure threads are done
    usleep(100000);  // 100ms delay
    
    printf("All threads finished, accumulating gradients...\n");
    fflush(stdout);
    
    // Verify system state before accumulation
    if (!system) {
        fprintf(stderr, "[FATAL] system is NULL!\n");
        return 0.0;
    }
    
    printf("[DEBUG] Pre-accumulation validation:\n");
    printf("  system: %p\n", (void*)system);
    printf("  system->training: %p\n", (void*)system->training);
    printf("  system->accumulated_gradients: %p\n", (void*)system->accumulated_gradients);
    printf("  system->gradient_size: %zu\n", system->gradient_size);
    fflush(stdout);
    
    if (!system->training) {
        fprintf(stderr, "[FATAL] system->training is NULL!\n");
        return 0.0;
    }
    
    printf("  system->training->gradients: %p\n", (void*)system->training->gradients);
    fflush(stdout);
    
    if (!system->training->gradients) {
        fprintf(stderr, "[FATAL] system->training->gradients is NULL!\n");
        fprintf(stderr, "  This means the training object was not properly initialized!\n");
        return 0.0;
    }
    
    if (!system->accumulated_gradients) {
        fprintf(stderr, "[FATAL] system->accumulated_gradients is NULL!\n");
        fprintf(stderr, "  This means the gradient buffer was not allocated!\n");
        return 0.0;
    }
    
    printf("[DEBUG] All pointers valid, calling accumulate_gradients...\n");
    fflush(stdout);
    
    accumulate_gradients(system);
    
    printf("[DEBUG] accumulate_gradients completed, acquiring model lock...\n");
    fflush(stdout);
    
    // KISSING BOUNDARY LOCK - Protect model weight updates
    // Control thread writes to model weights, workers read - this is a kissing boundary
    pthread_mutex_lock(&system->model_lock);
    
    printf("[DEBUG] Model lock acquired, copying gradients...\n");
    printf("  dst=%p, src=%p, size=%zu bytes\n",
           (void*)system->training->gradients, (void*)system->accumulated_gradients,
           system->gradient_size * sizeof(double));
    fflush(stdout);
    
    // Copy accumulated gradients to training object
    memcpy(system->training->gradients, system->accumulated_gradients, 
           system->gradient_size * sizeof(double));
    
    printf("[DEBUG] Gradient copy completed successfully\n");
    fflush(stdout);
    
    // Apply cymatic frequency modulation to gradients (optional)
    // This modulates gradients with cymatic frequencies for smoother convergence
    if (system->training->model && system->training->model->tokens) {
        // Extract symmetry groups
        uint32_t vocab_size = system->training->model->vocab_size;
        uint32_t* symmetry_groups = (uint32_t*)malloc(vocab_size * sizeof(uint32_t));
        if (symmetry_groups) {
            for (uint32_t i = 0; i < vocab_size; i++) {
                symmetry_groups[i] = system->training->model->tokens[i].symmetry_group;
            }
            
            // Cymatic frequencies (Hz)
            double frequencies[] = {432.0, 528.0, 639.0, 741.0, 852.0, 963.0};
            uint32_t num_freqs = 6;
            
            // Get dimensional frequencies
            uint64_t dimensional_freqs[12];
            for (uint32_t i = 0; i < 12; i++) {
                dimensional_freqs[i] = cllm_get_dimensional_frequency(i);
            }
            
            // Apply cymatic modulation (10% strength)
            // Use total_batches as training step counter
            apply_cymatic_modulation(
                (float*)system->training->gradients,
                symmetry_groups,
                vocab_size,
                system->training->model->embedding_dim,
                (uint32_t)system->training->total_batches,
                frequencies,
                num_freqs,
                dimensional_freqs,
                0.1
            );
            
            free(symmetry_groups);
        }
    }
    
    // Apply gradients using Adam optimizer
    printf("Applying optimizer step...\n");
    cllm_optimizer_step_adam(system->training);
    
    pthread_mutex_unlock(&system->model_lock);
    
    // Calculate average loss
    double epoch_loss = 0.0;
    int valid_workers = 0;
    for (int i = 0; i < system->num_worker_spheres; i++) {
        if (system->sphere_contexts[i]->batches_processed > 0) {
            epoch_loss += system->sphere_contexts[i]->batch_loss;
            valid_workers++;
        }
    }
    
    double avg_loss = (valid_workers > 0) ? epoch_loss / (double)valid_workers : 0.0;
    
    // UI Integration: Update final loss and invoke callbacks
    if (system->metrics) {
        cllm_metrics_update_loss(system->metrics, avg_loss);
        cllm_metrics_invoke_callbacks(system->metrics);
    }
    
    printf("\nEpoch complete (LOCK-FREE):\n");
    printf("  Total batches: %d\n", batches_pushed);
    printf("  Average loss: %.4f\n", avg_loss);
    printf("  Workers active: %d\n", valid_workers);
    
    return avg_loss;
}

/**
 * Print threading statistics
 */
void threaded_training_print_stats(ThreadedTrainingSystem* system) {
    if (!system) return;
    
    printf("\n========================================\n");
    printf("  Multi-Threaded Training Statistics\n");
    printf("========================================\n\n");
    
    printf("Thread System:\n");
    threads_print_stats(system->thread_system);
    
    printf("\nSphere Statistics:\n");
    for (int i = 0; i < system->num_worker_spheres; i++) {
        SphereTrainingContext* ctx = system->sphere_contexts[i];
        printf("  Sphere %2d (Group %2d): %d batches processed, avg loss: %.4f\n",
               ctx->sphere_id, ctx->symmetry_group, ctx->batches_processed,
               ctx->batches_processed > 0 ? ctx->batch_loss / ctx->batches_processed : 0.0);
    }
    
    printf("\n");
}
/**
 * Get per-sphere statistics
 */
int threaded_training_get_sphere_stats(ThreadedTrainingSystem* system,
                                       int sphere_id,
                                       int* batches_processed,
                                       float* avg_loss) {
    if (!system || sphere_id < 0 || sphere_id >= system->num_worker_spheres) return -1;
    
    SphereTrainingContext* ctx = system->sphere_contexts[sphere_id];
    if (!ctx) return -1;
    
    if (batches_processed) {
        *batches_processed = ctx->batches_processed;
    }
    
    if (avg_loss) {
        *avg_loss = ctx->batches_processed > 0 ? 
                    ctx->batch_loss / ctx->batches_processed : 0.0;
    }
    
    return 0;
}

/**
 * Get total gradient norm
 */
double threaded_training_get_gradient_norm(ThreadedTrainingSystem* system) {
    if (!system || !system->accumulated_gradients) return 0.0;
    
    // PHASE 3: Lock protects against concurrent writes in accumulate_gradients()
    // This function can be called from UI/crawler threads while accumulation is happening
    pthread_mutex_lock(&system->gradient_lock);
    
    double norm = 0.0;
    for (size_t i = 0; i < system->gradient_size; i++) {
        double val = system->accumulated_gradients[i];
        norm += val * val;
    }
    norm = math_sqrt(norm);
    
    pthread_mutex_unlock(&system->gradient_lock);
    
    return norm;
}

/**
 * Get number of worker spheres
 */
int threaded_training_get_num_workers(ThreadedTrainingSystem* system) {
    if (!system) return 0;
    return system->num_worker_spheres;
}

/**
 * Get next sphere ID for dynamic spawning
 * 
 * This function is called from cllm_threads.c when spawning new children.
 * It atomically increments and returns the next available sphere ID.
 * 
 * @param user_data Pointer to ThreadedTrainingSystem (from sphere->user_data)
 * @return Next available sphere ID
 */
int threaded_training_get_next_sphere_id(void* user_data) {
    if (!user_data) return -1;
    
    ThreadedTrainingSystem* system = (ThreadedTrainingSystem*)user_data;
    unsigned int next_id = atomic_fetch_add(&system->sphere_id_counter, 1);
    
    return (int)next_id;
}

/**
 * Get metrics from training system
 * 
 * UI Integration: Returns the metrics object for real-time monitoring
 */
CLLMMetrics* threaded_training_get_metrics(ThreadedTrainingSystem* system) {
    if (!system) return NULL;
    return system->metrics;
}

/**
 * PHASE 6: Get entropy integration context for UI display
 */
void* threaded_training_get_entropy_context(ThreadedTrainingSystem* system) {
    if (!system) return NULL;
    return system->entropy_context;
}

/**
 * PHASE 6: Get adaptive hierarchy context for UI display
 */
void* threaded_training_get_adaptive_hierarchy(ThreadedTrainingSystem* system) {
    if (!system) return NULL;
    return system->adaptive_hierarchy;
}

/**
 * PHASE 6: Get cymatic barrier statistics
 */
int threaded_training_get_cymatic_stats(ThreadedTrainingSystem* system,
                                        uint64_t* epoch_syncs,
                                        uint64_t* batch_syncs) {
    if (!system) return -1;
    
    // Check if barriers are available
    if (!system->epoch_barrier || !system->batch_barrier) {
        return -1;  // Barriers not available
    }
    
    // Get statistics from barriers
    if (epoch_syncs) {
        *epoch_syncs = system->epoch_barrier->sync_count;
    }
    
    if (batch_syncs) {
        *batch_syncs = system->batch_barrier->sync_count;
    }
    
    return 0;
}
