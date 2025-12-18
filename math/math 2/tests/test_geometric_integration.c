/**
 * @file test_geometric_integration.c
 * @brief Integration test for geometric matrix with thread parameters
 * 
 * Tests the complete geometric parameter system including:
 * - Thread parameter allocation
 * - Geometric matrix operations
 * - Memory usage verification
 * - Barycentric interpolation accuracy
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../algorithms/include/hierarchical_threading.h"
#include "../algorithms/include/thread_parameters_geometric.h"
#include "../algorithms/include/geometric_matrix.h"
#include "math/abacus.h"

// Helper to create a minimal thread for testing
HierarchicalThread* create_test_thread(uint32_t thread_id) {
    HierarchicalThread* thread = (HierarchicalThread*)calloc(1, sizeof(HierarchicalThread));
    if (!thread) return NULL;
    
    thread->thread_id = thread_id;
    thread->num_geometric_params = 0;
    thread->max_geometric_params = 0;
    thread->geometric_params = NULL;
    thread->geometric_gradients = NULL;
    thread->geometric_momentum = NULL;
    thread->geometric_velocity = NULL;
    thread->optimizer_step = 0;
    
    return thread;
}

void free_test_thread(HierarchicalThread* thread) {
    if (!thread) return;
    thread_free_geometric_parameters(thread);
    free(thread);
}

int test_basic_allocation(void) {
    printf("\n=== Test 1: Basic Parameter Allocation ===\n");
    
    HierarchicalThread* thread = create_test_thread(0);
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate a small parameter
    int idx = thread_allocate_geometric_parameter(thread, "test_param", 10, 10);
    if (idx < 0) {
        fprintf(stderr, "FAIL: Could not allocate parameter\n");
        free_test_thread(thread);
        return -1;
    }
    
    printf("PASS: Allocated parameter at index %d\n", idx);
    
    // Verify parameter exists
    GeometricMatrix* param = thread_get_geometric_parameter(thread, "test_param");
    if (!param) {
        fprintf(stderr, "FAIL: Could not retrieve parameter\n");
        free_test_thread(thread);
        return -1;
    }
    
    printf("PASS: Retrieved parameter '%s'\n", param->name);
    printf("  Dimensions: %u × %u\n", param->rows, param->cols);
    printf("  Vertices: %u\n", param->num_vertices);
    printf("  Memory: %zu bytes\n", geometric_matrix_memory_usage(param));
    
    free_test_thread(thread);
    return 0;
}

int test_initialization(void) {
    printf("\n=== Test 2: Parameter Initialization ===\n");
    
    HierarchicalThread* thread = create_test_thread(1);
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate parameter
    int idx = thread_allocate_geometric_parameter(thread, "W_test", 128, 128);
    if (idx < 0) {
        fprintf(stderr, "FAIL: Could not allocate parameter\n");
        free_test_thread(thread);
        return -1;
    }
    
    // Test different initialization methods
    printf("Testing initialization methods:\n");
    
    // Zeros
    if (thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_ZEROS, 12345) != 0) {
        fprintf(stderr, "FAIL: Zeros initialization failed\n");
        free_test_thread(thread);
        return -1;
    }
    printf("  ✓ Zeros initialization\n");
    
    // Xavier
    if (thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_XAVIER, 12345) != 0) {
        fprintf(stderr, "FAIL: Xavier initialization failed\n");
        free_test_thread(thread);
        return -1;
    }
    printf("  ✓ Xavier initialization\n");
    
    // He
    if (thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_HE, 12345) != 0) {
        fprintf(stderr, "FAIL: He initialization failed\n");
        free_test_thread(thread);
        return -1;
    }
    printf("  ✓ He initialization\n");
    
    printf("PASS: All initialization methods work\n");
    
    free_test_thread(thread);
    return 0;
}

int test_value_access(void) {
    printf("\n=== Test 3: Value Access via Barycentric Interpolation ===\n");
    
    HierarchicalThread* thread = create_test_thread(2);
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate and initialize parameter
    int idx = thread_allocate_geometric_parameter(thread, "W_access", 10, 10);
    if (idx < 0) {
        fprintf(stderr, "FAIL: Could not allocate parameter\n");
        free_test_thread(thread);
        return -1;
    }
    
    thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_UNIFORM, 54321);
    
    // Get some values
    CrystallineAbacus* value = abacus_new(60);
    
    printf("Testing value access at different positions:\n");
    for (uint32_t i = 0; i < 10; i += 3) {
        for (uint32_t j = 0; j < 10; j += 3) {
            if (thread_get_geometric_value(thread, idx, i, j, value) != 0) {
                fprintf(stderr, "FAIL: Could not get value at (%u, %u)\n", i, j);
                abacus_free(value);
                free_test_thread(thread);
                return -1;
            }
            
            double val_double;
            abacus_to_double(value, &val_double);
            printf("  Value at (%u, %u): %.6f\n", i, j, val_double);
        }
    }
    
    printf("PASS: Value access works\n");
    
    abacus_free(value);
    free_test_thread(thread);
    return 0;
}

int test_gradient_accumulation(void) {
    printf("\n=== Test 4: Gradient Accumulation ===\n");
    
    HierarchicalThread* thread = create_test_thread(3);
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate parameter
    int idx = thread_allocate_geometric_parameter(thread, "W_grad", 10, 10);
    if (idx < 0) {
        fprintf(stderr, "FAIL: Could not allocate parameter\n");
        free_test_thread(thread);
        return -1;
    }
    
    thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_ZEROS, 11111);
    
    // Accumulate some gradients
    CrystallineAbacus* grad = abacus_from_double(0.01, 60, 10);
    
    printf("Accumulating gradients:\n");
    for (uint32_t i = 0; i < 5; i++) {
        if (thread_accumulate_geometric_gradient(thread, idx, i, i, grad) != 0) {
            fprintf(stderr, "FAIL: Could not accumulate gradient at (%u, %u)\n", i, i);
            abacus_free(grad);
            free_test_thread(thread);
            return -1;
        }
        printf("  ✓ Accumulated gradient at (%u, %u)\n", i, i);
    }
    
    printf("PASS: Gradient accumulation works\n");
    
    abacus_free(grad);
    free_test_thread(thread);
    return 0;
}

int test_memory_usage(void) {
    printf("\n=== Test 5: Memory Usage Verification ===\n");
    
    HierarchicalThread* thread = create_test_thread(12); // Layer 1 thread
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate typical transformer layer parameters
    printf("Allocating transformer layer parameters:\n");
    
    thread_allocate_geometric_parameter(thread, "W_q", 128, 128);
    thread_allocate_geometric_parameter(thread, "W_k", 128, 128);
    thread_allocate_geometric_parameter(thread, "W_v", 128, 128);
    thread_allocate_geometric_parameter(thread, "W_o", 128, 128);
    thread_allocate_geometric_parameter(thread, "W_ffn1", 128, 512);
    thread_allocate_geometric_parameter(thread, "W_ffn2", 512, 128);
    
    // Initialize all
    for (uint32_t i = 0; i < thread->num_geometric_params; i++) {
        thread_initialize_geometric_parameter(thread, i, GEOMETRIC_INIT_XAVIER, 99999);
    }
    
    // Calculate memory usage
    size_t total_memory = thread_geometric_memory_usage(thread);
    
    printf("\nMemory Usage Analysis:\n");
    printf("  Number of parameters: %u\n", thread->num_geometric_params);
    printf("  Total memory (params + grads + optimizer): %zu bytes (%.2f KB)\n",
           total_memory, total_memory / 1024.0);
    
    // Compare to flat array
    size_t flat_memory = 0;
    flat_memory += 4 * (128 * 128 * sizeof(CrystallineAbacus)); // W_q, W_k, W_v, W_o
    flat_memory += 2 * (128 * 512 * sizeof(CrystallineAbacus)); // W_ffn1, W_ffn2
    flat_memory *= 4; // params + grads + momentum + velocity
    
    printf("  Flat array would use: %zu bytes (%.2f MB)\n",
           flat_memory, flat_memory / (1024.0 * 1024.0));
    printf("  Reduction: %.1fx\n", (double)flat_memory / (double)total_memory);
    
    if (total_memory > 1024 * 1024) { // More than 1 MB
        fprintf(stderr, "FAIL: Memory usage too high: %zu bytes\n", total_memory);
        free_test_thread(thread);
        return -1;
    }
    
    printf("PASS: Memory usage is acceptable\n");
    
    free_test_thread(thread);
    return 0;
}

int test_layer_allocation(void) {
    printf("\n=== Test 6: Layer-Based Automatic Allocation ===\n");
    
    // Test different layers
    uint32_t embedding_dim = 128;
    uint32_t hidden_dim = 512;
    uint32_t vocab_size = 50000;
    
    printf("Testing automatic allocation for different layers:\n");
    
    // Layer 0 (embeddings)
    printf("\nLayer 0 (Embeddings):\n");
    HierarchicalThread* thread0 = create_test_thread(0);
    if (thread_allocate_all_geometric_parameters(thread0, embedding_dim, hidden_dim, vocab_size) != 0) {
        fprintf(stderr, "FAIL: Layer 0 allocation failed\n");
        free_test_thread(thread0);
        return -1;
    }
    printf("  Allocated %u parameters\n", thread0->num_geometric_params);
    thread_print_geometric_info(thread0);
    
    // Layer 1 (transformer)
    printf("\nLayer 1 (Transformer):\n");
    HierarchicalThread* thread1 = create_test_thread(12);
    if (thread_allocate_all_geometric_parameters(thread1, embedding_dim, hidden_dim, vocab_size) != 0) {
        fprintf(stderr, "FAIL: Layer 1 allocation failed\n");
        free_test_thread(thread0);
        free_test_thread(thread1);
        return -1;
    }
    printf("  Allocated %u parameters\n", thread1->num_geometric_params);
    thread_print_geometric_info(thread1);
    
    // Layer 7 (output)
    printf("\nLayer 7 (Output):\n");
    HierarchicalThread* thread7 = create_test_thread(84);
    if (thread_allocate_all_geometric_parameters(thread7, embedding_dim, hidden_dim, vocab_size) != 0) {
        fprintf(stderr, "FAIL: Layer 7 allocation failed\n");
        free_test_thread(thread0);
        free_test_thread(thread1);
        free_test_thread(thread7);
        return -1;
    }
    printf("  Allocated %u parameters\n", thread7->num_geometric_params);
    thread_print_geometric_info(thread7);
    
    printf("PASS: Layer-based allocation works\n");
    
    free_test_thread(thread0);
    free_test_thread(thread1);
    free_test_thread(thread7);
    return 0;
}

int test_optimizer(void) {
    printf("\n=== Test 7: Adam Optimizer on Vertices ===\n");
    
    HierarchicalThread* thread = create_test_thread(4);
    if (!thread) {
        fprintf(stderr, "FAIL: Could not create test thread\n");
        return -1;
    }
    
    // Allocate and initialize parameter
    int idx = thread_allocate_geometric_parameter(thread, "W_opt", 10, 10);
    thread_initialize_geometric_parameter(thread, idx, GEOMETRIC_INIT_UNIFORM, 77777);
    
    // Accumulate some gradients
    CrystallineAbacus* grad = abacus_from_double(0.1, 60, 10);
    for (uint32_t i = 0; i < 5; i++) {
        thread_accumulate_geometric_gradient(thread, idx, i, i, grad);
    }
    abacus_free(grad);
    
    // Apply optimizer
    printf("Applying Adam optimizer:\n");
    double lr = 0.001;
    double beta1 = 0.9;
    double beta2 = 0.999;
    double epsilon = 1e-8;
    
    if (thread_apply_geometric_optimizer(thread, lr, beta1, beta2, epsilon) != 0) {
        fprintf(stderr, "FAIL: Optimizer failed\n");
        free_test_thread(thread);
        return -1;
    }
    
    printf("  ✓ Optimizer step completed\n");
    printf("  Optimizer step count: %lu\n", thread->optimizer_step);
    
    // Clear gradients
    if (thread_clear_geometric_gradients(thread) != 0) {
        fprintf(stderr, "FAIL: Could not clear gradients\n");
        free_test_thread(thread);
        return -1;
    }
    
    printf("  ✓ Gradients cleared\n");
    printf("PASS: Optimizer works\n");
    
    free_test_thread(thread);
    return 0;
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     Geometric Matrix Integration Test Suite                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    int failed = 0;
    
    if (test_basic_allocation() != 0) failed++;
    if (test_initialization() != 0) failed++;
    if (test_value_access() != 0) failed++;
    if (test_gradient_accumulation() != 0) failed++;
    if (test_memory_usage() != 0) failed++;
    if (test_layer_allocation() != 0) failed++;
    if (test_optimizer() != 0) failed++;
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║                  ALL TESTS PASSED ✓                          ║\n");
    } else {
        printf("║                  %d TESTS FAILED ✗                            ║\n", failed);
    }
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return failed;
}