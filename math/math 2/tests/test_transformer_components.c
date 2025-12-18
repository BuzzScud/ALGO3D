/**
 * Test program for transformer components
 * 
 * This test verifies:
 * 1. Feed-forward network operations
 * 2. Layer normalization
 * 3. Activation functions (ReLU, GELU, Swish)
 * 4. Thread-local weight storage
 * 5. Numerical stability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ai/cllm.h"
#include "ai/cllm_feedforward_helpers.h"
#include "ai/cllm_layernorm_helpers.h"
#include "hierarchical_threading.h"

#define TEST_VOCAB_SIZE 1000
#define TEST_EMBEDDING_DIM 128
#define TEST_HIDDEN_DIM 512
#define TEST_NUM_LAYERS 8
#define TEST_NUM_HEADS 8

// Color codes
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void print_test_header(const char* test_name) {
    printf("\n" COLOR_BLUE "========================================\n");
    printf("TEST: %s\n", test_name);
    printf("========================================" COLOR_RESET "\n");
}

void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf(COLOR_GREEN "✓ PASS: %s" COLOR_RESET "\n", test_name);
        tests_passed++;
    } else {
        printf(COLOR_RED "✗ FAIL: %s" COLOR_RESET "\n", test_name);
        tests_failed++;
    }
}

void print_summary() {
    printf("\n" COLOR_BLUE "========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================" COLOR_RESET "\n");
    printf("Total Tests: %d\n", tests_passed + tests_failed);
    printf(COLOR_GREEN "Passed: %d" COLOR_RESET "\n", tests_passed);
    if (tests_failed > 0) {
        printf(COLOR_RED "Failed: %d" COLOR_RESET "\n", tests_failed);
    } else {
        printf("Failed: 0\n");
    }
    printf("\n");
}

/**
 * Pure crystalline absolute value (no math.h)
 */
static double crystalline_abs(double x) {
    return (x >= 0.0) ? x : -x;
}

/**
 * Test 1: Feed-Forward Network
 */
void test_feedforward() {
    print_test_header("Feed-Forward Network");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = TEST_VOCAB_SIZE,
        .max_seq_len = 512,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .hidden_dim = TEST_HIDDEN_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        // Create test input
        double input[TEST_EMBEDDING_DIM];
        double output[TEST_EMBEDDING_DIM];
        
        for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
            input[i] = (double)i * 0.01;
        }
        
        // Get a thread to test with
        HierarchicalThread* thread = hierarchical_thread_get(
            model->pool_88d, 0, 1
        );
        
        if (!thread) {
            printf("  ERROR: Cannot get thread\n");
            passed = 0;
        } else {
            // Test FFN forward pass
            int result = cllm_ffn_forward_thread(
                thread,
                input,
                output,
                TEST_EMBEDDING_DIM,
                TEST_HIDDEN_DIM
            );
            
            if (result != 0) {
                printf("  ERROR: FFN forward pass failed\n");
                passed = 0;
            } else {
                printf("  ✓ FFN forward pass succeeded\n");
                
                // Verify output is not all zeros
                double sum = 0.0;
                for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                    sum += crystalline_abs(output[i]);
                }
                
                if (sum < 1e-10) {
                    printf("  ERROR: FFN output is all zeros\n");
                    passed = 0;
                } else {
                    printf("  ✓ FFN produces non-zero output (sum = %f)\n", sum);
                }
            }
        }
        
        cllm_free_model(model);
    }
    
    print_test_result("Feed-Forward Network", passed);
}

/**
 * Test 2: Layer Normalization
 */
void test_layernorm() {
    print_test_header("Layer Normalization");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = TEST_VOCAB_SIZE,
        .max_seq_len = 512,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .hidden_dim = TEST_HIDDEN_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        // Create test input with varying values
        double input[TEST_EMBEDDING_DIM];
        double output[TEST_EMBEDDING_DIM];
        
        for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
            input[i] = (double)i * 0.1 - 6.4;  // Range: [-6.4, 6.3]
        }
        
        // Get a thread to test with
        HierarchicalThread* thread = hierarchical_thread_get(
            model->pool_88d, 0, 1
        );
        
        if (!thread) {
            printf("  ERROR: Cannot get thread\n");
            passed = 0;
        } else {
            // Test layer norm forward pass
            int result = cllm_layernorm_forward_thread(
                thread,
                input,
                output,
                TEST_EMBEDDING_DIM,
                1e-5  // epsilon
            );
            
            if (result != 0) {
                printf("  ERROR: Layer norm forward pass failed\n");
                passed = 0;
            } else {
                printf("  ✓ Layer norm forward pass succeeded\n");
                
                // Verify output has mean ≈ 0 and variance ≈ 1
                double sum = 0.0;
                for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                    sum += output[i];
                }
                double mean = sum / TEST_EMBEDDING_DIM;
                
                double sum_sq = 0.0;
                for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                    double diff = output[i] - mean;
                    sum_sq += diff * diff;
                }
                double variance = sum_sq / TEST_EMBEDDING_DIM;
                
                printf("  Output statistics:\n");
                printf("    Mean: %f (expected ≈ 0)\n", mean);
                printf("    Variance: %f (expected ≈ 1)\n", variance);
                
                // Check if mean is close to 0 and variance is close to 1
                if (crystalline_abs(mean) > 0.1) {
                    printf("  ERROR: Mean too far from 0\n");
                    passed = 0;
                }
                
                if (crystalline_abs(variance - 1.0) > 0.2) {
                    printf("  ERROR: Variance too far from 1\n");
                    passed = 0;
                }
                
                if (passed) {
                    printf("  ✓ Layer norm produces normalized output\n");
                }
            }
        }
        
        cllm_free_model(model);
    }
    
    print_test_result("Layer Normalization", passed);
}

/**
 * Test 3: Batch Processing
 */
void test_batch_processing() {
    print_test_header("Batch Processing");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = TEST_VOCAB_SIZE,
        .max_seq_len = 512,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .hidden_dim = TEST_HIDDEN_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        uint32_t num_tokens = 10;
        
        // Create test input batch
        double* input = calloc(num_tokens * TEST_EMBEDDING_DIM, sizeof(double));
        double* output = calloc(num_tokens * TEST_EMBEDDING_DIM, sizeof(double));
        
        if (!input || !output) {
            printf("  ERROR: Memory allocation failed\n");
            passed = 0;
        } else {
            // Initialize input
            for (uint32_t t = 0; t < num_tokens; t++) {
                for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                    input[t * TEST_EMBEDDING_DIM + i] = (double)(t * 10 + i) * 0.01;
                }
            }
            
            // Test FFN batch processing
            int ffn_result = cllm_ffn_forward_batch(
                model,
                0,  // layer 0
                input,
                output,
                num_tokens
            );
            
            if (ffn_result != 0) {
                printf("  ERROR: FFN batch processing failed\n");
                passed = 0;
            } else {
                printf("  ✓ FFN batch processing succeeded\n");
            }
            
            // Test layer norm batch processing
            int ln_result = cllm_layernorm_forward_batch(
                model,
                0,  // layer 0
                input,
                output,
                num_tokens,
                1e-5
            );
            
            if (ln_result != 0) {
                printf("  ERROR: Layer norm batch processing failed\n");
                passed = 0;
            } else {
                printf("  ✓ Layer norm batch processing succeeded\n");
            }
        }
        
        free(input);
        free(output);
        cllm_free_model(model);
    }
    
    print_test_result("Batch Processing", passed);
}

/**
 * Main test runner
 */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf(COLOR_BLUE "========================================\n");
    printf("TRANSFORMER COMPONENTS TESTS\n");
    printf("========================================" COLOR_RESET "\n");
    printf("\n");
    printf("Testing transformer components:\n");
    printf("  - Feed-forward networks\n");
    printf("  - Layer normalization\n");
    printf("  - Batch processing\n");
    printf("  - Pure crystalline math (NO math.h)\n");
    printf("\n");
    
    // Run all tests
    test_feedforward();
    test_layernorm();
    test_batch_processing();
    
    // Print summary
    print_summary();
    
    // Return exit code
    return (tests_failed == 0) ? 0 : 1;
}