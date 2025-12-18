/**
 * @file thread_parameters.c
 * @brief Thread-Local Parameter Storage Implementation
 * 
 * PHASE 2: Thread-Centric Parameter Management
 * 
 * This module implements thread-local parameter storage, eliminating the need
 * for global parameter arrays. Each thread owns its parameters in
 * CrystallineAbacus format.
 */

#include "thread_parameters.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declaration to avoid circular dependency
// CLLMModel is defined in cllm/include/ai/cllm.h
// We only need the structure definition for pointer access
typedef struct CLLMModel CLLMModel;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Calculate total elements from shape
 */
static size_t calculate_total_elements(const uint32_t* shape, uint32_t num_dims) {
    if (!shape || num_dims == 0) return 0;
    
    size_t total = 1;
    for (uint32_t i = 0; i < num_dims; i++) {
        total *= shape[i];
    }
    return total;
}

/**
 * Generate random value for initialization
 */
static double random_uniform(double min, double max, uint64_t* seed) {
    // Simple LCG random number generator
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    double r = (double)(*seed) / (double)0x7fffffff;
    return min + r * (max - min);
}

/**
 * Generate random normal value (Box-Muller transform)
 * Uses CrystallineAbacus for all operations (RULE 1 compliance)
 */
static double random_normal(double mean, double stddev, uint64_t* seed) {
    double u1 = random_uniform(0.0, 1.0, seed);
    double u2 = random_uniform(0.0, 1.0, seed);
    
    // Avoid log(0) and log(1) - clamp to safe range
    if (u1 < 1e-10) u1 = 1e-10;
    if (u1 > (1.0 - 1e-10)) u1 = 1.0 - 1e-10;
    
    // Box-Muller transform using CrystallineAbacus
    CrystallineAbacus* temp1 = abacus_new(60);
    CrystallineAbacus* temp2 = abacus_new(60);
    CrystallineAbacus* result_abacus = abacus_new(60);
    
    // Create abacus for u1
    CrystallineAbacus* u1_abacus = abacus_from_double(u1, 60, 10);
    
    // temp1 = log(u1)
    math_log_abacus(temp1, u1_abacus, 10);
    
    // temp1 = -2 * log(u1)
    CrystallineAbacus* neg_two = abacus_from_double(-2.0, 60, 10);
    CrystallineAbacus* log_result = abacus_new(60);
    abacus_mul(log_result, neg_two, temp1);
    
    // temp1 = sqrt(-2 * log(u1))
    abacus_sqrt(temp1, log_result);
    
    // temp2 = cos(2*pi*u2)
    CrystallineAbacus* angle = abacus_from_double(2.0 * M_PI * u2, 60, 10);
    math_cos_abacus(temp2, angle, 10);
    
    // result = temp1 * temp2
    abacus_mul(result_abacus, temp1, temp2);
    
    double z0;
    abacus_to_double(result_abacus, &z0);
    
    abacus_free(temp1);
    abacus_free(temp2);
    abacus_free(result_abacus);
    abacus_free(u1_abacus);
    abacus_free(neg_two);
    abacus_free(log_result);
    abacus_free(angle);
    
    return mean + stddev * z0;
}

// ============================================================================
// PARAMETER ALLOCATION
// ============================================================================

int thread_allocate_parameter(
    HierarchicalThread* thread,
    const char* name,
    const uint32_t* shape,
    uint32_t num_dims
) {
    if (!thread || !name || !shape || num_dims == 0) {
        fprintf(stderr, "ERROR: Invalid arguments to thread_allocate_parameter\n");
        return -1;
    }
    
    // Lock parameter list
    pthread_mutex_lock(&thread->param_list_lock);
    
    // Check if parameter already exists
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        if (strcmp(thread->param_metadata[i].name, name) == 0) {
            fprintf(stderr, "ERROR: Parameter '%s' already exists in thread %u\n",
                    name, thread->thread_id);
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
    }
    
    // Check capacity
    if (thread->num_parameters >= thread->max_parameters) {
        // Expand arrays
        uint32_t new_capacity = thread->max_parameters * 2;
        if (new_capacity == 0) new_capacity = 8;
        
        // Reallocate arrays
        thread->parameters = (CrystallineAbacus**)realloc(
            thread->parameters, new_capacity * sizeof(CrystallineAbacus*));
        thread->gradients = (CrystallineAbacus**)realloc(
            thread->gradients, new_capacity * sizeof(CrystallineAbacus*));
        thread->momentum = (CrystallineAbacus**)realloc(
            thread->momentum, new_capacity * sizeof(CrystallineAbacus*));
        thread->velocity = (CrystallineAbacus**)realloc(
            thread->velocity, new_capacity * sizeof(CrystallineAbacus*));
        // Reallocate metadata array
        void* new_metadata = realloc(thread->param_metadata, 
                                     new_capacity * sizeof(*thread->param_metadata));
        if (!new_metadata) {
            fprintf(stderr, "ERROR: Failed to reallocate metadata array\n");
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
        thread->param_metadata = new_metadata;
        thread->param_locks = (pthread_mutex_t*)realloc(
            thread->param_locks, new_capacity * sizeof(pthread_mutex_t));
        
        if (!thread->parameters || !thread->gradients || !thread->momentum ||
            !thread->velocity || !thread->param_metadata || !thread->param_locks) {
            fprintf(stderr, "ERROR: Failed to expand parameter arrays\n");
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
        
        // Initialize new locks
        for (uint32_t i = thread->max_parameters; i < new_capacity; i++) {
            pthread_mutex_init(&thread->param_locks[i], NULL);
        }
        
        thread->max_parameters = new_capacity;
    }
    
    // Calculate total elements
    size_t total_elements = calculate_total_elements(shape, num_dims);
    
    // Allocate array of POINTERS to CrystallineAbacus (one per matrix element)
    // This properly handles matrices by flattening them into 1D arrays
    // CRITICAL: Store pointers, not structures, because CrystallineAbacus contains pointers
    thread->parameters[thread->num_parameters] = (CrystallineAbacus*)calloc(total_elements, sizeof(CrystallineAbacus*));
    thread->gradients[thread->num_parameters] = (CrystallineAbacus*)calloc(total_elements, sizeof(CrystallineAbacus*));
    thread->momentum[thread->num_parameters] = (CrystallineAbacus*)calloc(total_elements, sizeof(CrystallineAbacus*));
    thread->velocity[thread->num_parameters] = (CrystallineAbacus*)calloc(total_elements, sizeof(CrystallineAbacus*));
    
    if (!thread->parameters[thread->num_parameters] ||
        !thread->gradients[thread->num_parameters] ||
        !thread->momentum[thread->num_parameters] ||
        !thread->velocity[thread->num_parameters]) {
        fprintf(stderr, "ERROR: Failed to allocate parameter storage for %zu elements\n", total_elements);
        pthread_mutex_unlock(&thread->param_list_lock);
        return -1;
    }
    
    // Initialize each CrystallineAbacus pointer in the arrays
    CrystallineAbacus** param_ptrs = (CrystallineAbacus**)thread->parameters[thread->num_parameters];
    CrystallineAbacus** grad_ptrs = (CrystallineAbacus**)thread->gradients[thread->num_parameters];
    CrystallineAbacus** mom_ptrs = (CrystallineAbacus**)thread->momentum[thread->num_parameters];
    CrystallineAbacus** vel_ptrs = (CrystallineAbacus**)thread->velocity[thread->num_parameters];
    
    for (size_t i = 0; i < total_elements; i++) {
        // Create and store parameter abacus pointer
        param_ptrs[i] = abacus_new(60);
        if (!param_ptrs[i]) {
            fprintf(stderr, "ERROR: Failed to create abacus for parameter element %zu\n", i);
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
        
        // Create and store gradient abacus pointer
        grad_ptrs[i] = abacus_new(60);
        if (!grad_ptrs[i]) {
            fprintf(stderr, "ERROR: Failed to create abacus for gradient element %zu\n", i);
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
        
        // Create and store momentum abacus pointer
        mom_ptrs[i] = abacus_new(60);
        if (!mom_ptrs[i]) {
            fprintf(stderr, "ERROR: Failed to create abacus for momentum element %zu\n", i);
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
        
        // Create and store velocity abacus pointer
        vel_ptrs[i] = abacus_new(60);
        if (!vel_ptrs[i]) {
            fprintf(stderr, "ERROR: Failed to create abacus for velocity element %zu\n", i);
            pthread_mutex_unlock(&thread->param_list_lock);
            return -1;
        }
    }
    
    // Initialize metadata
    strncpy(thread->param_metadata[thread->num_parameters].name, name, 63);
    thread->param_metadata[thread->num_parameters].name[63] = '\0';
    thread->param_metadata[thread->num_parameters].num_dims = num_dims;
    thread->param_metadata[thread->num_parameters].total_elements = total_elements;
    thread->param_metadata[thread->num_parameters].requires_grad = true;
    thread->param_metadata[thread->num_parameters].is_initialized = false;
    
    // Copy shape
    thread->param_metadata[thread->num_parameters].shape = (uint32_t*)malloc(num_dims * sizeof(uint32_t));
    if (!thread->param_metadata[thread->num_parameters].shape) {
        fprintf(stderr, "ERROR: Failed to allocate shape array\n");
        pthread_mutex_unlock(&thread->param_list_lock);
        return -1;
    }
    memcpy(thread->param_metadata[thread->num_parameters].shape, shape, num_dims * sizeof(uint32_t));
    
    thread->num_parameters++;
    
    pthread_mutex_unlock(&thread->param_list_lock);
    
    return 0;
}

int thread_allocate_all_parameters(
    HierarchicalThread* thread,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    uint32_t vocab_size,
    uint32_t num_tokens_assigned
) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread\n");
        return -1;
    }
    
    // Determine thread role based on layer
    if (thread->layer == 0) {
        // Layer 0: Embeddings
        // Each thread stores embeddings for tokens assigned to it
        if (num_tokens_assigned > 0) {
            uint32_t shape[2] = {num_tokens_assigned, embedding_dim};
            if (thread_allocate_parameter(thread, "embeddings", shape, 2) != 0) {
                return -1;
            }
        }
        
    } else if (thread->layer >= 1 && thread->layer <= 6) {
        // Layer 1-6: Transformer layers
        uint32_t shape_qkv[2] = {embedding_dim, embedding_dim};
        uint32_t shape_ffn1[2] = {embedding_dim, hidden_dim};
        uint32_t shape_ffn2[2] = {hidden_dim, embedding_dim};
        uint32_t shape_ln[1] = {embedding_dim};
        
        // Allocate attention weights
        if (thread_allocate_parameter(thread, "W_q", shape_qkv, 2) != 0) return -1;
        if (thread_allocate_parameter(thread, "W_k", shape_qkv, 2) != 0) return -1;
        if (thread_allocate_parameter(thread, "W_v", shape_qkv, 2) != 0) return -1;
        if (thread_allocate_parameter(thread, "W_o", shape_qkv, 2) != 0) return -1;
        
        // Allocate FFN weights
        if (thread_allocate_parameter(thread, "W_ffn1", shape_ffn1, 2) != 0) return -1;
        if (thread_allocate_parameter(thread, "W_ffn2", shape_ffn2, 2) != 0) return -1;
        
        // Allocate layer norm parameters
        if (thread_allocate_parameter(thread, "gamma", shape_ln, 1) != 0) return -1;
        if (thread_allocate_parameter(thread, "beta", shape_ln, 1) != 0) return -1;
        
    } else if (thread->layer == 7) {
        // Layer 7: Output layer
        // Each thread handles a portion of the vocabulary
        uint32_t vocab_per_thread = vocab_size / 12;
        uint32_t shape[2] = {embedding_dim, vocab_per_thread};
        
        if (thread_allocate_parameter(thread, "W_out", shape, 2) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// ============================================================================
// PARAMETER INITIALIZATION
// ============================================================================

int thread_initialize_parameter(
    HierarchicalThread* thread,
    const char* name,
    ParameterInitMethod method,
    uint64_t seed
) {
    if (!thread || !name) {
        fprintf(stderr, "ERROR: Invalid arguments to thread_initialize_parameter\n");
        return -1;
    }
    
    // Find parameter
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) {
        fprintf(stderr, "ERROR: Parameter '%s' not found in thread %u\n",
                name, thread->thread_id);
        return -1;
    }
    
    // Lock parameter
    pthread_mutex_lock(&thread->param_locks[idx]);
    
    // Get pointer array (parameters are now stored as array of pointers)
    CrystallineAbacus** param_ptrs = (CrystallineAbacus**)thread->parameters[idx];
    size_t total_elements = thread->param_metadata[idx].total_elements;
    
    // Initialize based on method
    // TODO: Implement proper CrystallineAbacus initialization
    // For now, we'll use a placeholder
    
    switch (method) {
        case PARAM_INIT_ZEROS:
            // Already zero from calloc
            break;
            
        case PARAM_INIT_ONES:
            // Set all to 1.0
            for (size_t i = 0; i < total_elements; i++) {
                CrystallineAbacus* one = abacus_from_double(1.0, 60, 10);
                if (one) {
                    abacus_free(param_ptrs[i]); param_ptrs[i] = one;
                    // one is now owned by param_ptrs[i]
                }
            }
            break;
            
        case PARAM_INIT_RANDOM:
            // Random uniform [-0.1, 0.1]
            for (size_t i = 0; i < total_elements; i++) {
                double val = random_uniform(-0.1, 0.1, &seed);
                // Store value in CrystallineAbacus
                CrystallineAbacus* val_abacus = abacus_from_double(val, 60, 10);
                if (val_abacus) {
                    abacus_free(param_ptrs[i]); param_ptrs[i] = val_abacus;
                    // val_abacus is now owned by param_ptrs[i]
                }
            }
            break;
            
        case PARAM_INIT_XAVIER:
            // Xavier initialization: Simplified for testing
            // TODO: Implement proper N(0, sqrt(2 / (fan_in + fan_out))) after debugging math functions
            for (size_t i = 0; i < total_elements; i++) {
                double val = random_uniform(-0.1, 0.1, &seed);
                CrystallineAbacus* val_abacus = abacus_from_double(val, 60, 10);
                if (val_abacus) {
                    abacus_free(param_ptrs[i]);
                    param_ptrs[i] = val_abacus;
                }
            }
            break;
            
        case PARAM_INIT_HE:
            // He initialization: Simplified for testing
            // TODO: Implement proper N(0, sqrt(2 / fan_in)) after debugging math functions
            for (size_t i = 0; i < total_elements; i++) {
                double val = random_uniform(-0.1, 0.1, &seed);
                CrystallineAbacus* val_abacus = abacus_from_double(val, 60, 10);
                if (val_abacus) {
                    abacus_free(param_ptrs[i]);
                    param_ptrs[i] = val_abacus;
                }
            }
            break;
            
        case PARAM_INIT_NORMAL:
            // Normal distribution: Simplified for testing
            // TODO: Implement proper N(0, 0.02) after debugging random_normal
            for (size_t i = 0; i < total_elements; i++) {
                double val = random_uniform(-0.02, 0.02, &seed);
                CrystallineAbacus* val_abacus = abacus_from_double(val, 60, 10);
                if (val_abacus) {
                    abacus_free(param_ptrs[i]);
                    param_ptrs[i] = val_abacus;
                }
            }
            break;
    }
    
    thread->param_metadata[idx].is_initialized = true;
    
    pthread_mutex_unlock(&thread->param_locks[idx]);
    
    return 0;
}

int thread_initialize_all_parameters(
    HierarchicalThread* thread,
    ParameterInitMethod method
) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread\n");
        return -1;
    }
    
    uint64_t seed = thread->thread_id + time(NULL);
    
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        if (thread_initialize_parameter(thread, thread->param_metadata[i].name,
                                       method, seed) != 0) {
            return -1;
        }
    }
    
    return 0;
}

// ============================================================================
// PARAMETER ACCESS
// ============================================================================

CrystallineAbacus* thread_get_parameter(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return NULL;
    
    return thread->parameters[idx];
}

int thread_get_parameter_index(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return -1;
    
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        if (strcmp(thread->param_metadata[i].name, name) == 0) {
            return (int)i;
        }
    }
    
    return -1;
}

ThreadParameterMetadata* thread_get_parameter_metadata(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return NULL;
    
    return &thread->param_metadata[idx];
}

// ============================================================================
// GRADIENT ACCESS
// ============================================================================

CrystallineAbacus* thread_get_gradient(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return NULL;
    
    return thread->gradients[idx];
}

int thread_clear_gradients(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        pthread_mutex_lock(&thread->param_locks[i]);
        
        // TODO: Clear gradient CrystallineAbacus
        // For now, just mark as cleared
        
        pthread_mutex_unlock(&thread->param_locks[i]);
    }
    
    return 0;
}

int thread_accumulate_gradient(
    HierarchicalThread* thread,
    const char* name,
    const CrystallineAbacus* grad
) {
    if (!thread || !name || !grad) return -1;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return -1;
    
    pthread_mutex_lock(&thread->param_locks[idx]);
    
    // TODO: Accumulate gradient using CrystallineAbacus addition
    // gradients[idx] += grad
    
    pthread_mutex_unlock(&thread->param_locks[idx]);
    
    return 0;
}

// ============================================================================
// OPTIMIZER STATE ACCESS
// ============================================================================

CrystallineAbacus* thread_get_momentum(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return NULL;
    
    return thread->momentum[idx];
}

CrystallineAbacus* thread_get_velocity(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    int idx = thread_get_parameter_index(thread, name);
    if (idx < 0) return NULL;
    
    return thread->velocity[idx];
}

int thread_clear_optimizer_state(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        pthread_mutex_lock(&thread->param_locks[i]);
        
        // TODO: Clear momentum and velocity CrystallineAbacus
        
        pthread_mutex_unlock(&thread->param_locks[i]);
    }
    
    thread->optimizer_step = 0;
    
    return 0;
}

// ============================================================================
// PARAMETER STATISTICS
// ============================================================================

int thread_get_parameter_stats(
    HierarchicalThread* thread,
    ThreadParameterStats* stats
) {
    if (!thread || !stats) return -1;
    
    memset(stats, 0, sizeof(ThreadParameterStats));
    
    stats->num_parameters = thread->num_parameters;
    
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        stats->total_memory_bytes += thread->param_metadata[i].total_elements * sizeof(double);
        
        if (thread->param_metadata[i].is_initialized) {
            stats->num_initialized++;
        }
        
        // TODO: Calculate norms using CrystallineAbacus
    }
    
    return 0;
}

void thread_print_parameter_stats(HierarchicalThread* thread) {
    if (!thread) return;
    
    ThreadParameterStats stats;
    if (thread_get_parameter_stats(thread, &stats) != 0) {
        return;
    }
    
    printf("Thread %u Parameter Statistics:\n", thread->thread_id);
    printf("  Parameters: %u\n", stats.num_parameters);
    printf("  Initialized: %u\n", stats.num_initialized);
    printf("  Memory: %.2f MB\n", stats.total_memory_bytes / (1024.0 * 1024.0));
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool thread_has_parameter(
    HierarchicalThread* thread,
    const char* name
) {
    return thread_get_parameter_index(thread, name) >= 0;
}

uint32_t thread_get_num_parameters(HierarchicalThread* thread) {
    if (!thread) return 0;
    return thread->num_parameters;
}

size_t thread_get_parameter_memory(HierarchicalThread* thread) {
    if (!thread) return 0;
    
    size_t total = 0;
    for (uint32_t i = 0; i < thread->num_parameters; i++) {
        total += thread->param_metadata[i].total_elements * sizeof(double);
    }
    
    return total;
}