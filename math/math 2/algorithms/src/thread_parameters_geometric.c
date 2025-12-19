/**
 * @file thread_parameters_geometric.c
 * @brief Implementation of Thread Parameter Management using Geometric Matrices
 */

#define _USE_MATH_DEFINES
#include <math.h>  // Include standard math.h first
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "thread_parameters_geometric.h"
#include "geometric_matrix.h"
#include "hierarchical_threading.h"
#include "math/transcendental.h"  // For math_pow, math_sqrt

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Helper to copy abacus value
 */
static void copy_abacus_value(CrystallineAbacus* dest, const CrystallineAbacus* src) {
    if (!dest || !src) return;
    
    dest->num_beads = src->num_beads;
    dest->negative = src->negative;
    for (size_t i = 0; i < src->num_beads && i < dest->capacity; i++) {
        dest->beads[i] = src->beads[i];
    }
}

// ============================================================================
// PARAMETER ALLOCATION
// ============================================================================

int thread_allocate_geometric_parameter(
    HierarchicalThread* thread,
    const char* name,
    uint32_t rows,
    uint32_t cols
) {
    if (!thread || !name) {
        fprintf(stderr, "ERROR: Invalid arguments to thread_allocate_geometric_parameter\n");
        return -1;
    }
    
    // Check if we need to expand the array
    if (thread->num_geometric_params >= thread->max_geometric_params) {
        uint32_t new_max = thread->max_geometric_params == 0 ? 8 : thread->max_geometric_params * 2;
        
        GeometricMatrix** new_params = (GeometricMatrix**)realloc(
            thread->geometric_params,
            new_max * sizeof(GeometricMatrix*)
        );
        
        if (!new_params) {
            fprintf(stderr, "ERROR: Failed to expand geometric parameter array\n");
            return -1;
        }
        
        thread->geometric_params = new_params;
        thread->max_geometric_params = new_max;
        
        // Also expand gradients, momentum, velocity
        GeometricMatrix** new_grads = (GeometricMatrix**)realloc(
            thread->geometric_gradients,
            new_max * sizeof(GeometricMatrix*)
        );
        GeometricMatrix** new_momentum = (GeometricMatrix**)realloc(
            thread->geometric_momentum,
            new_max * sizeof(GeometricMatrix*)
        );
        GeometricMatrix** new_velocity = (GeometricMatrix**)realloc(
            thread->geometric_velocity,
            new_max * sizeof(GeometricMatrix*)
        );
        
        if (!new_grads || !new_momentum || !new_velocity) {
            fprintf(stderr, "ERROR: Failed to expand gradient/optimizer arrays\n");
            return -1;
        }
        
        thread->geometric_gradients = new_grads;
        thread->geometric_momentum = new_momentum;
        thread->geometric_velocity = new_velocity;
    }
    
    // Create the geometric matrix
    GeometricMatrix* param = geometric_matrix_create(rows, cols, name);
    if (!param) {
        fprintf(stderr, "ERROR: Failed to create geometric matrix for %s\n", name);
        return -1;
    }
    
    // Create corresponding gradient, momentum, velocity matrices
    char grad_name[128], momentum_name[128], velocity_name[128];
    snprintf(grad_name, sizeof(grad_name), "%s_grad", name);
    snprintf(momentum_name, sizeof(momentum_name), "%s_momentum", name);
    snprintf(velocity_name, sizeof(velocity_name), "%s_velocity", name);
    
    GeometricMatrix* grad = geometric_matrix_create(rows, cols, grad_name);
    GeometricMatrix* momentum = geometric_matrix_create(rows, cols, momentum_name);
    GeometricMatrix* velocity = geometric_matrix_create(rows, cols, velocity_name);
    
    if (!grad || !momentum || !velocity) {
        fprintf(stderr, "ERROR: Failed to create gradient/optimizer matrices\n");
        geometric_matrix_free(param);
        geometric_matrix_free(grad);
        geometric_matrix_free(momentum);
        geometric_matrix_free(velocity);
        return -1;
    }
    
    // Initialize gradients, momentum, velocity to zeros
    geometric_matrix_init_zeros(grad);
    geometric_matrix_init_zeros(momentum);
    geometric_matrix_init_zeros(velocity);
    
    // Store in arrays
    uint32_t idx = thread->num_geometric_params;
    thread->geometric_params[idx] = param;
    thread->geometric_gradients[idx] = grad;
    thread->geometric_momentum[idx] = momentum;
    thread->geometric_velocity[idx] = velocity;
    thread->num_geometric_params++;
    
    printf("Thread %u: Allocated geometric parameter '%s' [%u×%u] (index %u)\n",
           thread->thread_id, name, rows, cols, idx);
    
    return (int)idx;
}

int thread_allocate_all_geometric_parameters(
    HierarchicalThread* thread,
    uint32_t embedding_dim,
    uint32_t hidden_dim,
    uint32_t vocab_size
) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread\n");
        return -1;
    }
    
    // Determine layer from thread role
    // Assuming thread_id maps to layer: 0-11 = layer 0, 12-23 = layer 1, etc.
    uint32_t layer = thread->thread_id / 12;
    
    printf("Thread %u: Allocating geometric parameters for layer %u\n",
           thread->thread_id, layer);
    
    if (layer == 0) {
        // Layer 0: Embeddings
        // Each thread handles a subset of vocabulary
        uint32_t tokens_per_thread = vocab_size / 12;
        thread_allocate_geometric_parameter(thread, "embedding",
                                           tokens_per_thread, embedding_dim);
    } else if (layer >= 1 && layer <= 6) {
        // Layers 1-6: Transformer layers
        thread_allocate_geometric_parameter(thread, "W_q",
                                           embedding_dim, embedding_dim);
        thread_allocate_geometric_parameter(thread, "W_k",
                                           embedding_dim, embedding_dim);
        thread_allocate_geometric_parameter(thread, "W_v",
                                           embedding_dim, embedding_dim);
        thread_allocate_geometric_parameter(thread, "W_o",
                                           embedding_dim, embedding_dim);
        thread_allocate_geometric_parameter(thread, "W_ffn1",
                                           embedding_dim, hidden_dim);
        thread_allocate_geometric_parameter(thread, "W_ffn2",
                                           hidden_dim, embedding_dim);
        thread_allocate_geometric_parameter(thread, "gamma",
                                           1, embedding_dim);
        thread_allocate_geometric_parameter(thread, "beta",
                                           1, embedding_dim);
    } else if (layer == 7) {
        // Layer 7: Output layer
        uint32_t output_per_thread = vocab_size / 12;
        thread_allocate_geometric_parameter(thread, "W_out",
                                           embedding_dim, output_per_thread);
    }
    
    return 0;
}

void thread_free_geometric_parameters(HierarchicalThread* thread) {
    if (!thread) return;
    
    if (thread->geometric_params) {
        for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
            geometric_matrix_free(thread->geometric_params[i]);
        }
        free(thread->geometric_params);
        thread->geometric_params = NULL;
    }
    
    if (thread->geometric_gradients) {
        for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
            geometric_matrix_free(thread->geometric_gradients[i]);
        }
        free(thread->geometric_gradients);
        thread->geometric_gradients = NULL;
    }
    
    if (thread->geometric_momentum) {
        for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
            geometric_matrix_free(thread->geometric_momentum[i]);
        }
        free(thread->geometric_momentum);
        thread->geometric_momentum = NULL;
    }
    
    if (thread->geometric_velocity) {
        for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
            geometric_matrix_free(thread->geometric_velocity[i]);
        }
        free(thread->geometric_velocity);
        thread->geometric_velocity = NULL;
    }
    
    thread->num_geometric_params = 0;
    thread->max_geometric_params = 0;
}

// ============================================================================
// PARAMETER INITIALIZATION
// ============================================================================

int thread_initialize_geometric_parameter(
    HierarchicalThread* thread,
    uint32_t param_idx,
    GeometricInitMethod method,
    uint64_t seed
) {
    if (!thread || param_idx >= thread->num_geometric_params) {
        fprintf(stderr, "ERROR: Invalid parameter index\n");
        return -1;
    }
    
    GeometricMatrix* param = thread->geometric_params[param_idx];
    if (!param) {
        fprintf(stderr, "ERROR: Parameter is NULL\n");
        return -1;
    }
    
    int result = 0;
    
    switch (method) {
        case GEOMETRIC_INIT_ZEROS:
            result = geometric_matrix_init_zeros(param);
            break;
            
        case GEOMETRIC_INIT_ONES:
            result = geometric_matrix_init_ones(param);
            break;
            
        case GEOMETRIC_INIT_UNIFORM:
            result = geometric_matrix_init_uniform(param, -0.1, 0.1, seed);
            break;
            
        case GEOMETRIC_INIT_XAVIER:
            result = geometric_matrix_init_xavier(param, param->rows, param->cols, seed);
            break;
            
        case GEOMETRIC_INIT_HE:
            result = geometric_matrix_init_he(param, param->rows, seed);
            break;
            
        case GEOMETRIC_INIT_NORMAL:
            // Use uniform as approximation for now
            result = geometric_matrix_init_uniform(param, -0.02, 0.02, seed);
            break;
            
        default:
            fprintf(stderr, "ERROR: Unknown initialization method\n");
            return -1;
    }
    
    if (result == 0) {
        printf("Thread %u: Initialized parameter '%s' with method %d\n",
               thread->thread_id, param->name, method);
    }
    
    return result;
}

int thread_initialize_all_geometric_parameters(
    HierarchicalThread* thread,
    GeometricInitMethod method,
    uint64_t seed
) {
    if (!thread) {
        fprintf(stderr, "ERROR: Invalid thread\n");
        return -1;
    }
    
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        if (thread_initialize_geometric_parameter(thread, i, method, seed + i) != 0) {
            fprintf(stderr, "ERROR: Failed to initialize parameter %u\n", i);
            return -1;
        }
    }
    
    return 0;
}

// ============================================================================
// PARAMETER ACCESS
// ============================================================================

GeometricMatrix* thread_get_geometric_parameter(
    HierarchicalThread* thread,
    const char* name
) {
    if (!thread || !name) return NULL;
    
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        if (thread->geometric_params[i] &&
            strcmp(thread->geometric_params[i]->name, name) == 0) {
            return thread->geometric_params[i];
        }
    }
    
    return NULL;
}

GeometricMatrix* thread_get_geometric_parameter_by_index(
    HierarchicalThread* thread,
    uint32_t param_idx
) {
    if (!thread || param_idx >= thread->num_geometric_params) {
        return NULL;
    }
    
    return thread->geometric_params[param_idx];
}

int thread_get_geometric_value(
    HierarchicalThread* thread,
    uint32_t param_idx,
    uint32_t row,
    uint32_t col,
    CrystallineAbacus* result
) {
    if (!thread || !result) return -1;
    
    GeometricMatrix* param = thread_get_geometric_parameter_by_index(thread, param_idx);
    if (!param) return -1;
    
    return geometric_matrix_get(param, row, col, result);
}

int thread_accumulate_geometric_gradient(
    HierarchicalThread* thread,
    uint32_t param_idx,
    uint32_t row,
    uint32_t col,
    const CrystallineAbacus* gradient
) {
    if (!thread || !gradient) return -1;
    
    if (param_idx >= thread->num_geometric_params) return -1;
    
    GeometricMatrix* grad = thread->geometric_gradients[param_idx];
    if (!grad) return -1;
    
    return geometric_matrix_accumulate_gradient(grad, row, col, gradient);
}

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================

int thread_clear_geometric_gradients(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        if (thread->geometric_gradients[i]) {
            geometric_matrix_init_zeros(thread->geometric_gradients[i]);
        }
    }
    
    return 0;
}

int thread_apply_geometric_optimizer(
    HierarchicalThread* thread,
    double learning_rate,
    double beta1,
    double beta2,
    double epsilon
) {
    if (!thread) return -1;
    
    // Increment optimizer step
    thread->optimizer_step++;
    
    // Bias correction
    double bias_correction1 = 1.0 - math_pow(beta1, (double)thread->optimizer_step);  // Use custom math library
    double bias_correction2 = 1.0 - math_pow(beta2, (double)thread->optimizer_step);  // Use custom math library
    
    // Apply Adam optimizer to each parameter's vertices
    for (uint32_t p = 0; p < thread->num_geometric_params; p++) {
        GeometricMatrix* param = thread->geometric_params[p];
        GeometricMatrix* grad = thread->geometric_gradients[p];
        GeometricMatrix* momentum = thread->geometric_momentum[p];
        GeometricMatrix* velocity = thread->geometric_velocity[p];
        
        if (!param || !grad || !momentum || !velocity) continue;
        
        // Update each vertex
        for (uint32_t v = 0; v < param->num_vertices; v++) {
            CrystallineAbacus* p_val = param->vertex_values[v];
            CrystallineAbacus* g_val = grad->vertex_values[v];
            CrystallineAbacus* m_val = momentum->vertex_values[v];
            CrystallineAbacus* v_val = velocity->vertex_values[v];
            
            // Convert to doubles for computation
            double p_double, g_double, m_double, v_double;
            abacus_to_double(p_val, &p_double);
            abacus_to_double(g_val, &g_double);
            abacus_to_double(m_val, &m_double);
            abacus_to_double(v_val, &v_double);
            
            // Adam update
            // m = β₁*m + (1-β₁)*grad
            m_double = beta1 * m_double + (1.0 - beta1) * g_double;
            
            // v = β₂*v + (1-β₂)*grad²
            v_double = beta2 * v_double + (1.0 - beta2) * g_double * g_double;
            
            // Bias-corrected moments
            double m_hat = m_double / bias_correction1;
            double v_hat = v_double / bias_correction2;
            
            // Update parameter
            // param -= lr * m_hat / (√v_hat + ε)
            p_double -= learning_rate * m_hat / (math_sqrt(v_hat) + epsilon);  // Use custom math library
            
            // Convert back to abacus
            CrystallineAbacus* new_p = abacus_from_double(p_double, 60, 10);
            CrystallineAbacus* new_m = abacus_from_double(m_double, 60, 10);
            CrystallineAbacus* new_v = abacus_from_double(v_double, 60, 10);
            
            if (new_p && new_m && new_v) {
                copy_abacus_value(p_val, new_p);
                copy_abacus_value(m_val, new_m);
                copy_abacus_value(v_val, new_v);
            }
            
            abacus_free(new_p);
            abacus_free(new_m);
            abacus_free(new_v);
        }
    }
    
    return 0;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

size_t thread_geometric_memory_usage(const HierarchicalThread* thread) {
    if (!thread) return 0;
    
    size_t total = 0;
    
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        if (thread->geometric_params[i]) {
            total += geometric_matrix_memory_usage(thread->geometric_params[i]);
        }
        if (thread->geometric_gradients[i]) {
            total += geometric_matrix_memory_usage(thread->geometric_gradients[i]);
        }
        if (thread->geometric_momentum[i]) {
            total += geometric_matrix_memory_usage(thread->geometric_momentum[i]);
        }
        if (thread->geometric_velocity[i]) {
            total += geometric_matrix_memory_usage(thread->geometric_velocity[i]);
        }
    }
    
    return total;
}

void thread_print_geometric_info(const HierarchicalThread* thread) {
    if (!thread) return;
    
    printf("\n=== Thread %u Geometric Parameters ===\n", thread->thread_id);
    printf("Number of parameters: %u\n", thread->num_geometric_params);
    printf("Total memory usage: %zu bytes (%.2f KB)\n",
           thread_geometric_memory_usage(thread),
           thread_geometric_memory_usage(thread) / 1024.0);
    
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        if (thread->geometric_params[i]) {
            printf("\nParameter %u:\n", i);
            geometric_matrix_print_info(thread->geometric_params[i]);
        }
    }
    
    printf("=====================================\n\n");
}

uint32_t thread_get_num_geometric_parameters(const HierarchicalThread* thread) {
    return thread ? thread->num_geometric_params : 0;
}

// ============================================================================
// MIGRATION HELPERS
// ============================================================================

int thread_migrate_to_geometric(HierarchicalThread* thread) {
    if (!thread) return -1;
    
    // TODO: Implement migration from flat arrays to geometric matrices
    fprintf(stderr, "ERROR: Migration not yet implemented\n");
    return -1;
}

bool thread_is_using_geometric(const HierarchicalThread* thread) {
    if (!thread) return false;
    
    return thread->geometric_params != NULL && thread->num_geometric_params > 0;
}