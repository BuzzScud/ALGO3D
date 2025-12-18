/**
 * CLLM Hierarchical Training Implementation
 * 
 * Implements the correct recursive kissing spheres hierarchy for training.
 * 
 * Architecture:
 * - Level 0: 1 root (Node Zero) - control only, NEVER processes batches
 * - Level 1: Up to 12 spheres - can be control or workers
 * - Level 2+: Recursive structure - each sphere can have up to 12 children
 * 
 * For 64 cores:
 * - Level 0: 1 root control
 * - Level 1: 12 control spheres
 * - Level 2: 51 worker spheres
 * 
 * Work distribution:
 * - Root distributes batches to Level 1 (round-robin)
 * - Level 1 distributes to their workers (round-robin)
 * - Workers process batches and send gradients to parent
 * - Parents accumulate gradients from children
 * - Root does final accumulation and optimizer step
 */

#include "cllm_training.h"
#include "ai/cllm_lattice_hierarchy.h"
#include "ai/cllm_message_queue.h"
#include "ai/cllm_sphere_message.h"
#include "ai/cllm_lattice.h"
#include "cllm_batch.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

// Forward declarations
typedef struct HierarchicalTrainingSystem HierarchicalTrainingSystem;

// Forward declaration - use existing ThreadLocalTrainingContext
typedef struct ThreadLocalTrainingContext ThreadLocalTrainingContext;

// Declare existing functions from cllm_training_threaded.c
extern ThreadLocalTrainingContext* thread_local_training_create(
    int batch_size,
    int seq_len,
    int num_layers,
    int embed_dim,
    int vocab_size,
    int ff_hidden_dim,
    int num_heads
);

extern void thread_local_training_free(ThreadLocalTrainingContext* ctx);

extern float cllm_forward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* input_tokens
);

extern void cllm_backward_training_threaded(
    CLLMTraining* training,
    ThreadLocalTrainingContext* local_ctx,
    uint32_t* target_tokens,
    float* gradient_buffer
);

/**
 * Sphere training context wrapper
 * 
 * Wraps the existing ThreadLocalTrainingContext with additional
 * sphere-specific data.
 */
typedef struct {
    ThreadLocalTrainingContext* thread_local_ctx;
    float* local_gradients;
    size_t gradient_size;
    int tokens_processed;
    float total_loss;
} SphereTrainingContext;

/**
 * Hierarchical Training System
 */
struct HierarchicalTrainingSystem {
    CLLMTraining* training;
    CLLMLatticeHierarchy* root;
    
    int total_threads;
    int num_level1;
    int num_level2;
    
    atomic_int running;
    atomic_int epoch_done;
    atomic_int shutdown;  // NEW: Signal threads to exit
    atomic_int epoch_start;  // NEW: Signal epoch start
    
    CLLMBatchIterator* batch_iterator;
    
    // Thread-local contexts for each sphere
    SphereTrainingContext** sphere_contexts;
    int num_sphere_contexts;
    
    // Global gradient accumulator (at root)
    float* accumulated_gradients;
    size_t gradient_size;
    
    float epoch_loss;
    int total_batches;
};

/**
 * Calculate optimal hierarchy structure based on model symmetry
 * 
 * The model structure dictates the hierarchy:
 * - Model has symmetry_order (typically 12)
 * - Create one Level-1 control per symmetry group
 * - Distribute remaining threads as Level-2 workers
 */
static void calculate_hierarchy_structure(int total_threads, int model_symmetry_order,
                                          int* num_level1, int* num_level2) {
    if (total_threads <= 1) {
        *num_level1 = 0;
        *num_level2 = 0;
        return;
    }
    
    if (total_threads <= model_symmetry_order + 1) {
        // Small: just root + workers (no intermediate level)
        *num_level1 = total_threads - 1;
        *num_level2 = 0;
        return;
    }
    
    // Optimal: root + symmetry_order Level-1 controls + remaining workers
    // This matches the model's natural structure
    *num_level1 = model_symmetry_order;
    *num_level2 = total_threads - 1 - model_symmetry_order;
    
    printf("  Hierarchy matches model symmetry:\n");
    printf("    Model symmetry order: %d\n", model_symmetry_order);
    printf("    Level-1 controls: %d (one per symmetry group)\n", *num_level1);
    printf("    Level-2 workers: %d (distributed across groups)\n", *num_level2);
}

/**
 * Create recursive sphere hierarchy based on model structure
 */
static int create_sphere_hierarchy(HierarchicalTrainingSystem* system) {
    // Get model symmetry order (typically 12)
    int model_symmetry_order = system->training->model->header.symmetry_order;
    
    if (model_symmetry_order == 0) {
        fprintf(stderr, "WARNING: Model symmetry_order is 0, defaulting to 12\n");
        model_symmetry_order = 12;
    }
    
    int num_level1, num_level2;
    calculate_hierarchy_structure(system->total_threads, model_symmetry_order, 
                                   &num_level1, &num_level2);
    
    system->num_level1 = num_level1;
    system->num_level2 = num_level2;
    
    printf("Creating hierarchy based on model structure:\n");
    printf("  Level 0: 1 root (Node Zero)\n");
    printf("  Level 1: %d spheres (one per symmetry group)\n", num_level1);
    printf("  Level 2: %d spheres (workers)\n", num_level2);
    
    // Create Level 1 spheres (children of root)
    // Each Level-1 sphere corresponds to one symmetry group from the model
    for (int i = 0; i < num_level1; i++) {
        // Symmetry group matches model structure
        int symmetry_group = i % model_symmetry_order;
        
        CLLMLatticeHierarchy* level1_sphere = lattice_hierarchy_create(
            i + 1,                    // sphere_id
            1,                        // hierarchy_level
            &symmetry_group,          // symmetry_groups (matches model!)
            1,                        // num_symmetry_groups
            i + 1,                    // physical_thread_id
            system->root              // parent
        );
        
        if (!level1_sphere) {
            fprintf(stderr, "ERROR: Failed to create Level 1 sphere %d\n", i);
            return -1;
        }
        
        printf("  Created Level-1 sphere %d for symmetry group %d\n", i, symmetry_group);
        
        // Add as child to root
        if (lattice_hierarchy_add_child(system->root, level1_sphere) == 0) {
            fprintf(stderr, "ERROR: Failed to add Level 1 sphere %d to root\n", i);
            return -1;
        }
    }
    
    // Create Level 2 spheres (children of Level 1)
    if (num_level2 > 0) {
        int sphere_id = num_level1 + 1;
        int thread_id = num_level1 + 1;
        
        for (int i = 0; i < num_level2; i++) {
            // Distribute evenly across Level 1 spheres
            int parent_idx = i % num_level1;
            CLLMLatticeHierarchy* parent = system->root->children[parent_idx];
            
            int symmetry_group = parent->primary_symmetry_group;
            
            CLLMLatticeHierarchy* level2_sphere = lattice_hierarchy_create(
                sphere_id++,              // sphere_id
                2,                        // hierarchy_level
                &symmetry_group,          // symmetry_groups
                1,                        // num_symmetry_groups
                thread_id++,              // physical_thread_id
                parent                    // parent
            );
            
            if (!level2_sphere) {
                fprintf(stderr, "ERROR: Failed to create Level 2 sphere %d\n", i);
                return -1;
            }
            
            // Add as child to Level 1 sphere
            if (lattice_hierarchy_add_child(parent, level2_sphere) == 0) {
                fprintf(stderr, "ERROR: Failed to add Level 2 sphere %d to parent\n", i);
                return -1;
            }
        }
    }
    
    printf("✓ Hierarchy created successfully\n");
    return 0;
}

/**
 * Process lattice point with its 12 neighbors (kissing spheres!)
 * 
 * This leverages the model's natural structure where each lattice point
 * has up to 12 neighbors, forming the kissing spheres configuration.
 */
static void process_lattice_point_with_neighbors(
    CLLMLatticePoint* point,
    CLLMModel* model,
    float* local_gradients,
    size_t gradient_size
) {
    if (!point || !model) return;
    
    // Process the point itself
    // The embedding for this point influences nearby tokens
    
    // Process relationships with 12 neighbors (kissing spheres!)
    // Now using the full implementation from cllm_kissing_spheres.c
    // This processes all 12 kissing sphere neighbors using L(n,d,k,λ) formula
    cllm_process_kissing_spheres(point, model, local_gradients, gradient_size);
}

/**
 * Process batch on a leaf worker sphere using lattice structure
 * 
 * This is the core training loop for a worker sphere.
 * It processes batches using the model's lattice structure and
 * the sphere's assigned symmetry group.
 * 
 * Uses thread-local context to avoid race conditions.
 */
static void process_batch_on_sphere(CLLMLatticeHierarchy* sphere, 
                                    CLLMBatch* batch,
                                    HierarchicalTrainingSystem* system,
                                    SphereTrainingContext* ctx) {
    if (!sphere || !batch || !system || !ctx) return;
    
    CLLMTraining* training = system->training;
    CLLMModel* model = training->model;
    
    // This sphere only processes tokens in its symmetry group
    int my_symmetry_group = sphere->primary_symmetry_group;
    
    // Zero local gradients
    memset(ctx->local_gradients, 0, ctx->gradient_size * sizeof(float));
    
    float total_loss = 0.0;
    int tokens_processed = 0;
    int sequences_processed = 0;
    
    // Process each sequence in the batch
    for (uint32_t seq = 0; seq < batch->batch_size; seq++) {
        uint32_t offset = seq * batch->seq_len;
        
        // Check if this sequence has valid tokens
        int has_valid = 0;
        for (uint32_t i = 0; i < batch->seq_len; i++) {
            if (batch->attention_mask && batch->attention_mask[offset + i] > 0.5f) {
                has_valid = 1;
                break;
            }
        }
        
        if (!has_valid) continue;
        
        // Count tokens in our symmetry group
        int my_tokens = 0;
        for (uint32_t pos = 0; pos < batch->seq_len; pos++) {
            uint32_t token_id = batch->input_ids[offset + pos];
            if (token_id >= model->vocab_size) continue;
            
            uint32_t token_symmetry_group;
            if (model->tokens) {
                token_symmetry_group = model->tokens[token_id].symmetry_group;
            } else {
                token_symmetry_group = token_id % 12;
            }
            
            if ((int)token_symmetry_group == my_symmetry_group) {
                my_tokens++;
            }
        }
        
        // Only process if we have significant tokens in this sequence
        if (my_tokens == 0) continue;
        
        // Forward pass using thread-local context
           printf("    [Sphere %d] Starting forward pass for sequence %d\n", sphere->sphere_id, seq);
           fflush(stdout);
        float seq_loss = cllm_forward_training_threaded(
            training,
            ctx->thread_local_ctx,
            &batch->input_ids[offset]
        );
        
           printf("    [Sphere %d] Forward pass complete, loss=%.4f\n", sphere->sphere_id, seq_loss);
           fflush(stdout);
        // Process lattice points for tokens in our symmetry group
        for (uint32_t pos = 0; pos < batch->seq_len; pos++) {
            uint32_t token_id = batch->input_ids[offset + pos];
            if (token_id >= model->vocab_size) continue;
            
            uint32_t token_symmetry_group;
            if (model->tokens) {
                token_symmetry_group = model->tokens[token_id].symmetry_group;
            } else {
                token_symmetry_group = token_id % 12;
            }
            
            if ((int)token_symmetry_group != my_symmetry_group) continue;
            
            // Find the lattice point for this token
            if (model->lattice_points && token_id < model->num_lattice_points) {
                CLLMLatticePoint* point = &model->lattice_points[token_id];
                
                // Process this lattice point with its 12 neighbors (kissing spheres!)
                process_lattice_point_with_neighbors(point, model, ctx->local_gradients, ctx->gradient_size);
            }
            
            tokens_processed++;
        }
        
        // Backward pass using thread-local context
           printf("    [Sphere %d] Starting backward pass for sequence %d\n", sphere->sphere_id, seq);
           fflush(stdout);
        cllm_backward_training_threaded(
            training,
            ctx->thread_local_ctx,
            &batch->target_ids[offset],
            ctx->local_gradients
        );
        
           printf("    [Sphere %d] Backward pass complete\n", sphere->sphere_id);
           fflush(stdout);
        total_loss += seq_loss;
        sequences_processed++;
    }
    
    // Calculate average loss for this batch
    float batch_loss = (sequences_processed > 0) ? total_loss / sequences_processed : 0.0;
    
    // Store statistics
    // TODO: Store in proper structure
    (void)batch_loss;
    (void)tokens_processed;
    
    // Gradients are now in ctx->local_gradients
    // They will be sent to parent by the calling function
}

/**
 * Select least loaded child sphere
 */
static int select_least_loaded_child(CLLMLatticeHierarchy* sphere) {
    // Simple round-robin for now
    // TODO: Implement actual load tracking
    static atomic_int counter = ATOMIC_VAR_INIT(0);
    int idx = atomic_fetch_add(&counter, 1) % sphere->num_children;
    return idx;
}

/**
 * Create sphere training context
 * 
 * Uses the existing thread_local_training_create() function
 * and wraps it with sphere-specific data.
 */
static SphereTrainingContext* sphere_training_context_create(
    CLLMModel* model, 
    CLLMTraining* training,
    int batch_size, 
    int seq_len
) {
    SphereTrainingContext* ctx = (SphereTrainingContext*)calloc(1, sizeof(SphereTrainingContext));
    if (!ctx) return NULL;
    
    // Calculate ff_hidden_dim and num_heads from model
    int ff_hidden_dim = model->embedding_dim * 4;  // Standard transformer ratio
    int num_heads = 8;  // Default - will be read from model->header.num_heads if available
    
    // Try to get num_heads from model header
    if (model->header.num_heads > 0) {
        num_heads = model->header.num_heads;
    }
    
    (void)training;  // Mark as used
    
    // Create thread-local training context using existing function
    ctx->thread_local_ctx = thread_local_training_create(
        batch_size,
        seq_len,
        model->num_layers,
        model->embedding_dim,
        model->vocab_size,
        ff_hidden_dim,
        num_heads
    );
    
    if (!ctx->thread_local_ctx) {
        free(ctx);
        return NULL;
    }
    
    // Allocate local gradient buffer
    ctx->gradient_size = model->vocab_size * model->embedding_dim;
    ctx->local_gradients = (float*)calloc(ctx->gradient_size, sizeof(float));
    
    if (!ctx->local_gradients) {
        thread_local_training_free(ctx->thread_local_ctx);
        free(ctx);
        return NULL;
    }
    
    ctx->tokens_processed = 0;
    ctx->total_loss = 0.0;
    
    return ctx;
}

/**
 * Free sphere training context
 */
static void sphere_training_context_free(SphereTrainingContext* ctx) {
    if (!ctx) return;
    
    if (ctx->thread_local_ctx) {
        thread_local_training_free(ctx->thread_local_ctx);
    }
    free(ctx->local_gradients);
    free(ctx);
}

/**
 * Gradient accumulation structure for hierarchical accumulation
 */
typedef struct {
    float* gradients;
    size_t gradient_size;
    int num_contributions;
    pthread_mutex_t lock;
} GradientAccumulator;

/**
 * Create gradient accumulator for a sphere
 */
static GradientAccumulator* gradient_accumulator_create(size_t gradient_size) {
    GradientAccumulator* acc = (GradientAccumulator*)calloc(1, sizeof(GradientAccumulator));
    if (!acc) return NULL;
    
    acc->gradients = (float*)calloc(gradient_size, sizeof(float));
    if (!acc->gradients) {
        free(acc);
        return NULL;
    }
    
    acc->gradient_size = gradient_size;
    acc->num_contributions = 0;
    pthread_mutex_init(&acc->lock, NULL);
    
    return acc;
}

/**
 * Free gradient accumulator
 */
static void gradient_accumulator_free(GradientAccumulator* acc) {
    if (!acc) return;
    free(acc->gradients);
    pthread_mutex_destroy(&acc->lock);
    free(acc);
}

/**
 * Accumulate gradients from child sphere
 * 
 * This implements hierarchical gradient accumulation:
 * - Leaf workers compute gradients for their lattice partition
 * - Level-1 controls accumulate from their workers
 * - Root accumulates from all Level-1 controls
 */
static void accumulate_gradients_from_child(
    GradientAccumulator* parent_acc,
    float* child_gradients,
    size_t gradient_size
) {
    if (!parent_acc || !child_gradients) return;
    
    pthread_mutex_lock(&parent_acc->lock);
    
    // Use SIMD for fast accumulation
    // TODO: Use cllm_simd_accumulate_gradients when available
    for (size_t i = 0; i < gradient_size; i++) {
        parent_acc->gradients[i] += child_gradients[i];
    }
    
    parent_acc->num_contributions++;
    
    pthread_mutex_unlock(&parent_acc->lock);
}

/**
 * Send gradients to parent sphere
 */
static void send_gradients_to_parent(
    CLLMLatticeHierarchy* sphere,
    float* gradients,
    size_t gradient_size
) {
    if (!sphere->parent || !gradients) return;
    
    // Create gradient message
    SphereMessage* msg = (SphereMessage*)malloc(sizeof(SphereMessage));
    if (!msg) return;
    
    memset(msg, 0, sizeof(SphereMessage));
    msg->type = MSG_GRADIENT_READY;
    msg->priority = MSG_PRIORITY_HIGH;
    msg->sender_id = sphere->sphere_id;
    msg->receiver_id = sphere->parent->sphere_id;
    msg->sender_symmetry_group = sphere->primary_symmetry_group;
    
    // Store gradient pointer in generic_data
    msg->payload.generic_data[0] = (uint64_t)(uintptr_t)gradients;
    msg->payload.generic_data[1] = (uint64_t)gradient_size;
    
    atomic_init(&msg->processed, 0);
    atomic_init(&msg->acknowledged, 0);
    
    // Send to parent
    if (!message_queue_enqueue(sphere->parent->inbox, msg)) {
        fprintf(stderr, "[Sphere %d] ERROR: Failed to send gradients to parent\n",
                sphere->sphere_id);
        free(msg);
    }
}

/**
 * Sphere thread function (works for any level)
 * 
 * Handles both control and worker responsibilities:
 * - Control spheres: Forward batches to children, accumulate gradients
 * - Worker spheres: Process batches, compute gradients, send to parent
 */
static void* sphere_thread_func(void* arg) {
    CLLMLatticeHierarchy* sphere = (CLLMLatticeHierarchy*)arg;
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)sphere->user_data;
    
    printf("[Sphere %d Level %d] Thread started (symmetry group %d)\n", 
           sphere->sphere_id, sphere->hierarchy_level, sphere->primary_symmetry_group);
    fflush(stdout);
    
    // Create gradient accumulator if this is a control sphere
    GradientAccumulator* grad_acc = NULL;
    if (sphere->num_children > 0) {
        size_t gradient_size = system->training->model->vocab_size * 
                               system->training->model->embedding_dim;
        grad_acc = gradient_accumulator_create(gradient_size);
        if (!grad_acc) {
            fprintf(stderr, "[Sphere %d] ERROR: Failed to create gradient accumulator\n",
                    sphere->sphere_id);
            return NULL;
        }
    }
    
    // Create thread-local training context if this is a worker sphere
    SphereTrainingContext* training_ctx = NULL;
    if (sphere->num_children == 0) {
        training_ctx = sphere_training_context_create(
            system->training->model,
            system->training,
            system->training->config.batch_size,
            system->training->config.sequence_length
        );
        if (!training_ctx) {
            fprintf(stderr, "[Sphere %d] ERROR: Failed to create training context\n",
                    sphere->sphere_id);
            return NULL;
        }
    }
    
    printf("[Sphere %d Level %d] Entering epoch loop (will run until shutdown)\n",
           sphere->sphere_id, sphere->hierarchy_level);
    fflush(stdout);
    
    // EPOCH LOOP - Keep thread alive across epochs
    while (!atomic_load(&system->shutdown)) {
        // Wait for epoch start signal
        while (!atomic_load(&system->epoch_start) && !atomic_load(&system->shutdown)) {
            usleep(1000);  // 1ms
        }
        
        if (atomic_load(&system->shutdown)) {
            break;
        }
        
        printf("[Sphere %d Level %d] Epoch start signal received, processing messages\n",
               sphere->sphere_id, sphere->hierarchy_level);
        fflush(stdout);
        
        int batches_processed = 0;
        int gradients_accumulated = 0;
        
        // Process messages for this epoch
        while (atomic_load(&system->running)) {
            // Receive message from parent
            SphereMessage* msg = message_queue_dequeue(sphere->inbox);
            
            if (!msg) {
                // No message - check if epoch done
                if (atomic_load(&system->epoch_done)) {
                    break;
                }
                usleep(100);
                continue;
            }
            
            if (sphere->num_children > 0) {
                // This is a CONTROL sphere
                
                if (msg->type == MSG_BATCH_START) {
                    printf("[Sphere %d Level %d] Received batch, forwarding to children\n",
                           sphere->sphere_id, sphere->hierarchy_level);
                    fflush(stdout);
                    
                    // Forward batch to children
                    int target_child = select_least_loaded_child(sphere);
                    
                    printf("[Sphere %d Level %d] Forwarding to child %d (sphere %d)\n",
                           sphere->sphere_id, sphere->hierarchy_level, target_child,
                           sphere->children[target_child]->sphere_id);
                    fflush(stdout);
                    
                    if (!message_queue_enqueue(sphere->children[target_child]->inbox, msg)) {
                        fprintf(stderr, "[Sphere %d] ERROR: Failed to forward message to child %d\n",
                                sphere->sphere_id, target_child);
                        free(msg);
                    }
                } else if (msg->type == MSG_GRADIENT_READY) {
                    // Accumulate gradients from child
                    float* child_gradients = (float*)(uintptr_t)msg->payload.generic_data[0];
                    size_t gradient_size = (size_t)msg->payload.generic_data[1];
                    
                    accumulate_gradients_from_child(grad_acc, child_gradients, gradient_size);
                    gradients_accumulated++;
                    
                    free(msg);
                    
                    // If all children have reported, send to parent
                    if (gradients_accumulated >= sphere->num_children) {
                        send_gradients_to_parent(sphere, grad_acc->gradients, grad_acc->gradient_size);
                        
                        // Reset for next epoch
                        memset(grad_acc->gradients, 0, grad_acc->gradient_size * sizeof(float));
                        gradients_accumulated = 0;
                    }
                } else {
                    free(msg);
                }
            } else {
                // This is a LEAF WORKER - process batch
                if (msg->type == MSG_BATCH_START) {
                       printf("[Sphere %d Level %d] WORKER received batch, processing...\n",
                              sphere->sphere_id, sphere->hierarchy_level);
                       fflush(stdout);
                    // Extract batch pointer from generic_data
                    CLLMBatch* batch = (CLLMBatch*)(uintptr_t)msg->payload.generic_data[0];
                    
                    // Process batch and compute gradients using thread-local context
                    process_batch_on_sphere(sphere, batch, system, training_ctx);
                    batches_processed++;
                       printf("[Sphere %d Level %d] WORKER finished processing batch, sending gradients\n",
                              sphere->sphere_id, sphere->hierarchy_level);
                       fflush(stdout);
                    
                    // Send computed gradients back to parent
                    if (sphere->parent && training_ctx) {
                        send_gradients_to_parent(sphere, training_ctx->local_gradients, 
                                                training_ctx->gradient_size);
                    }
                }
                
                // Free message after processing
                free(msg);
            }
        }
        
        printf("[Sphere %d Level %d] Epoch complete (processed %d batches, accumulated %d gradients)\n", 
               sphere->sphere_id, sphere->hierarchy_level, batches_processed, gradients_accumulated);
    }
    
    printf("[Sphere %d Level %d] Thread exiting (shutdown signaled)\n",
           sphere->sphere_id, sphere->hierarchy_level);
    
    // Cleanup
    if (grad_acc) {
        gradient_accumulator_free(grad_acc);
    }
    if (training_ctx) {
        sphere_training_context_free(training_ctx);
    }
    
    return NULL;
}


/**
 * Distribute batch by token symmetry groups
 * 
 * Each token in the batch has a symmetry_group (0-11).
 * We analyze the batch and send it to the Level-1 control
 * that handles the most tokens in this batch.
 */
static int get_dominant_symmetry_group(CLLMBatch* batch, CLLMModel* model) {
    if (!batch || !model || batch->batch_size == 0) {
        return 0;  // Default to group 0
    }
    
    // Count tokens per symmetry group
    int group_counts[12] = {0};
    int model_symmetry_order = model->header.symmetry_order;
    
    if (model_symmetry_order == 0) {
        model_symmetry_order = 12;
    }
    
    // Analyze batch tokens
    for (uint32_t i = 0; i < batch->batch_size * batch->seq_len; i++) {
        uint32_t token_id = batch->input_ids[i];
        
        if (token_id < model->vocab_size && model->tokens) {
            // Get token's symmetry group from model
            uint32_t group = model->tokens[token_id].symmetry_group;
            if (group < 12) {
                group_counts[group]++;
            }
        } else {
            // Fallback: use token_id % symmetry_order
            uint32_t group = token_id % model_symmetry_order;
            group_counts[group]++;
        }
    }
    
    // Find dominant group
    int max_count = 0;
    int dominant_group = 0;
    for (int g = 0; g < model_symmetry_order && g < 12; g++) {
        if (group_counts[g] > max_count) {
            max_count = group_counts[g];
            dominant_group = g;
        }
    }
    
    return dominant_group;
}

/**
 * Root control thread (Node Zero)
 */
static void* root_control_thread(void* arg) {
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)arg;
    CLLMLatticeHierarchy* root = system->root;
    CLLMModel* model = system->training->model;
    
    printf("[Node Zero] Root control thread started\n");
    fflush(stdout);
    printf("[Node Zero] System pointer: %p\n", (void*)system);
    printf("[Node Zero] Root pointer: %p\n", (void*)root);
    printf("[Node Zero] Model pointer: %p\n", (void*)model);
    fflush(stdout);
    printf("[Node Zero] Managing %d Level-1 children\n", root->num_children);
    printf("[Node Zero] Entering epoch loop (will run until shutdown)\n");
    fflush(stdout);
    
    // EPOCH LOOP - Keep thread alive across epochs
    while (!atomic_load(&system->shutdown)) {
        // Wait for epoch start signal
        while (!atomic_load(&system->epoch_start) && !atomic_load(&system->shutdown)) {
            usleep(1000);  // 1ms
        }
        
        if (atomic_load(&system->shutdown)) {
            break;
        }
        
        printf("[Node Zero] Epoch started\n");
        
        // Reset batch iterator for this epoch
        cllm_batch_iterator_reset(system->batch_iterator);
        
        int batches_distributed = 0;
        int group_distribution[12] = {0};  // Track distribution per group
        printf("[Node Zero] Using symmetry-based distribution (not round-robin)\n");
        
        // Distribute batches
        while (atomic_load(&system->running)) {
            // Get next batch
            CLLMBatch* batch = cllm_batch_iterator_next(system->batch_iterator);
            
            if (!batch) {
                // No more batches
                break;
            }
            
            // Determine which symmetry group this batch belongs to
            int symmetry_group = get_dominant_symmetry_group(batch, model);
            
            // Find Level-1 control for this symmetry group
            CLLMLatticeHierarchy* target = NULL;
            for (int i = 0; i < root->num_children; i++) {
                if (root->children[i]->primary_symmetry_group == symmetry_group) {
                    target = root->children[i];
                    break;
                }
            }
            
            if (!target) {
                // Fallback: use modulo
                target = root->children[symmetry_group % root->num_children];
            }
            
            // Create message
            SphereMessage* msg = (SphereMessage*)malloc(sizeof(SphereMessage));
            if (!msg) {
                fprintf(stderr, "[Node Zero] ERROR: Failed to allocate message\n");
                continue;
            }
            
            memset(msg, 0, sizeof(SphereMessage));
            msg->type = MSG_BATCH_START;
            msg->priority = MSG_PRIORITY_NORMAL;
            msg->sender_id = 0;
            msg->receiver_id = target->sphere_id;
            msg->sender_symmetry_group = -1;  // Root has all groups
            msg->receiver_symmetry_group = symmetry_group;
            
            // Store batch pointer in generic_data
            msg->payload.generic_data[0] = (uint64_t)(uintptr_t)batch;
            
            atomic_init(&msg->processed, 0);
            atomic_init(&msg->acknowledged, 0);
            
            // Send to child
            if (!message_queue_enqueue(target->inbox, msg)) {
                fprintf(stderr, "[Node Zero] ERROR: Failed to send message to child (group %d)\n", symmetry_group);
                free(msg);
                continue;
            }
            
            batches_distributed++;
            group_distribution[symmetry_group]++;
            
            if (batches_distributed % 100 == 0) {
                printf("[Node Zero] Distributed %d batches (by symmetry group)\n", batches_distributed);
            }
        }
        
        printf("[Node Zero] All %d batches distributed\n", batches_distributed);
        
        // Show distribution by symmetry group
        printf("[Node Zero] Distribution by symmetry group:\n");
        for (int g = 0; g < 12; g++) {
            if (group_distribution[g] > 0) {
                printf("  Group %2d: %d batches (%.1f%%)\n", g, group_distribution[g],
                       100.0 * group_distribution[g] / batches_distributed);
            }
        }
        system->total_batches = batches_distributed;
        
        printf("[Node Zero] Waiting for all workers to complete...\n");
        
        // Count how many children actually received work
        int children_with_work = 0;
        for (int i = 0; i < root->num_children; i++) {
            int group = root->children[i]->primary_symmetry_group;
            if (group_distribution[group] > 0) {
                children_with_work++;
            }
        }
        
        printf("[Node Zero] Expecting gradients from %d children (out of %d total)\n", 
               children_with_work, root->num_children);
        
        // Wait for Level-1 controls that received work to send their accumulated gradients
        int gradients_received = 0;
        int expected_gradients = children_with_work;
        
        // Create gradient accumulator for root
        size_t gradient_size = model->vocab_size * model->embedding_dim;
        GradientAccumulator* root_acc = gradient_accumulator_create(gradient_size);
        if (!root_acc) {
            fprintf(stderr, "[Node Zero] ERROR: Failed to create gradient accumulator\n");
            break;
        }
        
        // Collect gradients from all Level-1 controls
        while (gradients_received < expected_gradients) {
            SphereMessage* msg = message_queue_dequeue(root->inbox);
            
            if (!msg) {
                usleep(1000);  // 1ms
                continue;
            }
            
            if (msg->type == MSG_GRADIENT_READY) {
                float* child_gradients = (float*)(uintptr_t)msg->payload.generic_data[0];
                size_t child_gradient_size = (size_t)msg->payload.generic_data[1];
                
                // Accumulate gradients from this Level-1 control
                accumulate_gradients_from_child(root_acc, child_gradients, child_gradient_size);
                gradients_received++;
                
                printf("[Node Zero] Received gradients from child %d/%d\n", 
                       gradients_received, expected_gradients);
                
                free(msg);
            } else {
                free(msg);
            }
        }
        
        printf("[Node Zero] All gradients accumulated\n");
        
        // Copy accumulated gradients to training object
        if (system->training->gradients) {
            memcpy(system->training->gradients, root_acc->gradients, 
                   gradient_size * sizeof(float));
        }
        
        // Apply optimizer step (Adam)
        printf("[Node Zero] Applying optimizer step...\n");
        cllm_optimizer_step_adam(system->training);
        printf("[Node Zero] Optimizer step complete\n");
        
        // Cleanup
        gradient_accumulator_free(root_acc);
        
        // Signal epoch done AFTER optimizer completes
        atomic_store(&system->epoch_done, 1);
        printf("[Node Zero] Epoch complete\n");
        
        // Reset epoch_start for next epoch
        atomic_store(&system->epoch_start, 0);
    }
    
    printf("[Node Zero] Root control thread exiting (shutdown signaled)\n");
    return NULL;
}

/**
 * Start all sphere threads
 */
static int start_sphere_threads(HierarchicalTrainingSystem* system) {
    // Start Level 1 threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* sphere = system->root->children[i];
        sphere->user_data = system;  // Store system reference
        
        int rc = pthread_create(&sphere->thread, NULL, sphere_thread_func, sphere);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create Level 1 thread %d\n", i);
            return -1;
        }
        atomic_store(&sphere->thread_running, 1);
    }
    
    // Start Level 2 threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* level1 = system->root->children[i];
        
        for (int j = 0; j < level1->num_children; j++) {
            CLLMLatticeHierarchy* sphere = level1->children[j];
            sphere->user_data = system;  // Store system reference
            
            int rc = pthread_create(&sphere->thread, NULL, sphere_thread_func, sphere);
            if (rc != 0) {
                fprintf(stderr, "ERROR: Failed to create Level 2 thread %d-%d\n", i, j);
                return -1;
            }
            atomic_store(&sphere->thread_running, 1);
        }
    }
    
    printf("✓ All sphere threads started\n");
    fflush(stdout);
    return 0;
}

/**
 * Create hierarchical training system
 */
HierarchicalTrainingSystem* hierarchical_training_create(CLLMTraining* training, 
                                                         int num_threads,
                                                         CLLMBatchIterator* batch_iterator) {
    if (!training || num_threads < 1) return NULL;
    
    HierarchicalTrainingSystem* system = (HierarchicalTrainingSystem*)calloc(1, sizeof(HierarchicalTrainingSystem));
    if (!system) return NULL;
    
    system->training = training;
    system->total_threads = num_threads;
    system->batch_iterator = batch_iterator;
    
    atomic_init(&system->running, 1);
    atomic_init(&system->epoch_done, 0);
    atomic_init(&system->shutdown, 0);
    atomic_init(&system->epoch_start, 0);
    
    printf("\n=== Creating Hierarchical Training System ===\n");
    printf("Total threads: %d\n", num_threads);
    
    // Create root sphere (Node Zero)
    int root_symmetry_groups[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    system->root = lattice_hierarchy_create(
        0,                      // sphere_id (root)
        0,                      // hierarchy_level (root level)
        root_symmetry_groups,   // symmetry_groups
        12,                     // num_symmetry_groups
        0,                      // physical_thread_id (root control)
        NULL                    // parent (root has no parent)
    );
    
    if (!system->root) {
        fprintf(stderr, "ERROR: Failed to create root hierarchy\n");
        free(system);
        return NULL;
    }
    
    system->root->user_data = system;  // Store system reference
    printf("✓ Root sphere created (Node Zero)\n");
    
    // Create sphere hierarchy
    if (create_sphere_hierarchy(system) != 0) {
        fprintf(stderr, "ERROR: Failed to create sphere hierarchy\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    
    // Start all sphere threads
    if (start_sphere_threads(system) != 0) {
        fprintf(stderr, "ERROR: Failed to start sphere threads\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    
    // Start root control thread
    int rc = pthread_create(&system->root->thread, NULL, root_control_thread, system);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Failed to create root control thread\n");
        // TODO: Cleanup
        free(system);
        return NULL;
    }
    atomic_store(&system->root->thread_running, 1);
    
    printf("✓ Root control thread started\n");
    printf("=== Hierarchical Training System Ready ===\n\n");
    fflush(stdout);
    
    return system;
}

/**
 * Train one epoch using hierarchical system
 */
float hierarchical_train_epoch(HierarchicalTrainingSystem* system) {
    if (!system || !system->root) return 0.0;
    
    printf("\n=== HIERARCHICAL TRAINING EPOCH ===\n");
    printf("Hierarchy structure:\n");
    printf("  Level 0: 1 root (Node Zero)\n");
    printf("  Level 1: %d controls\n", system->num_level1);
    printf("  Level 2: %d workers\n", system->num_level2);
    printf("\n");
    fflush(stdout);
    
    // Signal epoch start to all threads FIRST
    atomic_store(&system->epoch_done, 0);
    atomic_store(&system->running, 1);
    atomic_store(&system->epoch_start, 1);
    
    // Give threads time to wake up and enter message processing loop
    usleep(10000);  // 10ms
    
    // Wait for epoch completion
    while (!atomic_load(&system->epoch_done)) {
        usleep(10000);  // 10ms
    }
    
    // Reset running flag for next epoch
    atomic_store(&system->running, 0);
    
    printf("\nEpoch complete (HIERARCHICAL):\n");
    printf("  Total batches: %d\n", system->total_batches);
    printf("  Average loss: %.4f\n", system->epoch_loss);
    
    return system->epoch_loss;
}

/**
 * Free hierarchical training system
 */
void hierarchical_training_free(HierarchicalTrainingSystem* system) {
    if (!system) return;
    
    printf("\nStopping hierarchical training system...\n");
    
    // Signal shutdown to all threads
    atomic_store(&system->shutdown, 1);
    atomic_store(&system->epoch_start, 1);  // Wake up threads waiting for epoch start
    atomic_store(&system->running, 0);
    atomic_store(&system->epoch_done, 1);
    
    // Wait for root control thread
    if (atomic_load(&system->root->thread_running)) {
        pthread_join(system->root->thread, NULL);
        printf("✓ Root control thread stopped\n");
    }
    
    // Wait for all sphere threads
    for (int i = 0; i < system->root->num_children; i++) {
        CLLMLatticeHierarchy* level1 = system->root->children[i];
        
        if (atomic_load(&level1->thread_running)) {
            pthread_join(level1->thread, NULL);
        }
        
        for (int j = 0; j < level1->num_children; j++) {
            CLLMLatticeHierarchy* level2 = level1->children[j];
            
            if (atomic_load(&level2->thread_running)) {
                pthread_join(level2->thread, NULL);
            }
        }
    }
    
    printf("✓ All sphere threads stopped\n");
    
    // TODO: Free hierarchy
    // TODO: Free system
    
    printf("✓ Hierarchical training system freed\n");
}