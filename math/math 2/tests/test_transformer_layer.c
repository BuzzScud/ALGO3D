/**
 * Test Complete Transformer Layer
 * 
 * Tests the full transformer layer implementation including:
 * - Single layer processing
 * - Multi-layer processing
 * - Logit computation
 * - Softmax and sampling
 */

#include "ai/cllm.h"
#include "ai/cllm_embedding_helpers.h"
#include "ai/cllm_transformer_layer.h"
#include "hierarchical_threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Test configuration
#define TEST_VOCAB_SIZE 1000
#define TEST_EMBEDDING_DIM 768
#define TEST_NUM_LAYERS 4
#define TEST_NUM_HEADS 12

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

/**
 * Test 1: Single Layer Processing
 */
int test_single_layer() {
    printf("\n%s=== Test 1: Single Layer Processing ===%s\n", COLOR_BLUE, COLOR_RESET);
    
    // Create model configuration
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .max_seq_len = 512
    };
    
    // Create model
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("%s✗ Failed to create model%s\n", COLOR_RED, COLOR_RESET);
        return -1;
    }
    
    // Get a test thread
    HierarchicalThread* thread = hierarchical_thread_get(model->pool_88d, 0, 1);
    if (!thread) {
        printf("%s✗ Failed to get thread%s\n", COLOR_RED, COLOR_RESET);
        cllm_free_model(model);
        return -1;
    }
    
    // Create test input
    double* input = (double*)calloc(TEST_EMBEDDING_DIM, sizeof(double));
    double* output = (double*)calloc(TEST_EMBEDDING_DIM, sizeof(double));
    
    // Initialize input with test values
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        input[i] = (double)(i % 10) / 10.0;
    }
    
    // Process through single layer
    int result = cllm_transformer_layer_forward(model, thread, 0, input, output);
    
    if (result != 0) {
        printf("%s✗ Layer processing failed%s\n", COLOR_RED, COLOR_RESET);
        free(input);
        free(output);
        cllm_free_model(model);
        return -1;
    }
    
    // Verify output is different from input
    int different = 0;
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        if (fabs(output[i] - input[i]) > 1e-6) {
            different = 1;
            break;
        }
    }
    
    if (!different) {
        printf("%s✗ Output identical to input (no transformation)%s\n", COLOR_RED, COLOR_RESET);
        free(input);
        free(output);
        cllm_free_model(model);
        return -1;
    }
    
    // Compute output statistics
    double sum = 0.0;
    double sum_sq = 0.0;
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        sum += output[i];
        sum_sq += output[i] * output[i];
    }
    double mean = sum / TEST_EMBEDDING_DIM;
    double variance = (sum_sq / TEST_EMBEDDING_DIM) - (mean * mean);
    
    printf("  Output mean: %.6f\n", mean);
    printf("  Output variance: %.6f\n", variance);
    printf("%s✓ Single layer processing successful%s\n", COLOR_GREEN, COLOR_RESET);
    
    free(input);
    free(output);
    cllm_free_model(model);
    return 0;
}

/**
 * Test 2: Multi-Layer Processing
 */
int test_multi_layer() {
    printf("\n%s=== Test 2: Multi-Layer Processing ===%s\n", COLOR_BLUE, COLOR_RESET);
    
    // Create model configuration
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .max_seq_len = 512
    };
    
    // Create model
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("%s✗ Failed to create model%s\n", COLOR_RED, COLOR_RESET);
        return -1;
    }
    
    // Get a test thread
    HierarchicalThread* thread = hierarchical_thread_get(model->pool_88d, 0, 1);
    if (!thread) {
        printf("%s✗ Failed to get thread%s\n", COLOR_RED, COLOR_RESET);
        cllm_free_model(model);
        return -1;
    }
    
    // Create test input
    double* input = (double*)calloc(TEST_EMBEDDING_DIM, sizeof(double));
    double* output = (double*)calloc(TEST_EMBEDDING_DIM, sizeof(double));
    
    // Initialize input with test values
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        input[i] = (double)(i % 10) / 10.0;
    }
    
    // Process through all layers
    int result = cllm_transformer_forward(model, thread, input, output);
    
    if (result != 0) {
        printf("%s✗ Multi-layer processing failed%s\n", COLOR_RED, COLOR_RESET);
        free(input);
        free(output);
        cllm_free_model(model);
        return -1;
    }
    
    // Compute output statistics
    double sum = 0.0;
    double sum_sq = 0.0;
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        sum += output[i];
        sum_sq += output[i] * output[i];
    }
    double mean = sum / TEST_EMBEDDING_DIM;
    double variance = (sum_sq / TEST_EMBEDDING_DIM) - (mean * mean);
    
    printf("  Processed through %d layers\n", TEST_NUM_LAYERS);
    printf("  Output mean: %.6f\n", mean);
    printf("  Output variance: %.6f\n", variance);
    printf("%s✓ Multi-layer processing successful%s\n", COLOR_GREEN, COLOR_RESET);
    
    free(input);
    free(output);
    cllm_free_model(model);
    return 0;
}

/**
 * Test 3: Logit Computation and Softmax
 */
int test_logits_and_softmax() {
    printf("\n%s=== Test 3: Logit Computation and Softmax ===%s\n", COLOR_BLUE, COLOR_RESET);
    
    // Create model configuration
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .max_seq_len = 512
    };
    
    // Create model
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("%s✗ Failed to create model%s\n", COLOR_RED, COLOR_RESET);
        return -1;
    }
    
    // Create test hidden state
    double* hidden = (double*)calloc(TEST_EMBEDDING_DIM, sizeof(double));
    double* logits = (double*)calloc(TEST_VOCAB_SIZE, sizeof(double));
    
    // Initialize hidden state
    for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
        hidden[i] = (double)(i % 10) / 10.0;
    }
    
    // Compute logits
    int result = cllm_compute_logits(model, hidden, logits);
    
    if (result != 0) {
        printf("%s✗ Logit computation failed%s\n", COLOR_RED, COLOR_RESET);
        free(hidden);
        free(logits);
        cllm_free_model(model);
        return -1;
    }
    
    // Apply softmax
    cllm_softmax(logits, TEST_VOCAB_SIZE);
    
    // Verify softmax properties
    double sum = 0.0;
    double max_prob = logits[0];
    uint32_t max_idx = 0;
    
    for (uint32_t i = 0; i < TEST_VOCAB_SIZE; i++) {
        sum += logits[i];
        if (logits[i] > max_prob) {
            max_prob = logits[i];
            max_idx = i;
        }
    }
    
    printf("  Probability sum: %.6f (should be ~1.0)\n", sum);
    printf("  Max probability: %.6f at token %u\n", max_prob, max_idx);
    
    // Check if sum is close to 1.0
    if (fabs(sum - 1.0) > 0.01) {
        printf("%s✗ Softmax sum not close to 1.0%s\n", COLOR_RED, COLOR_RESET);
        free(hidden);
        free(logits);
        cllm_free_model(model);
        return -1;
    }
    
    // Test sampling
    uint32_t sampled = cllm_sample_token(logits, TEST_VOCAB_SIZE, 1.0);
    printf("  Sampled token: %u\n", sampled);
    
    printf("%s✓ Logit computation and softmax successful%s\n", COLOR_GREEN, COLOR_RESET);
    
    free(hidden);
    free(logits);
    cllm_free_model(model);
    return 0;
}

/**
 * Main test runner
 */
int main() {
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n", 
           COLOR_BLUE, COLOR_RESET);
    printf("%s║     Complete Transformer Layer Test Suite             ║%s\n", 
           COLOR_BLUE, COLOR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", 
           COLOR_BLUE, COLOR_RESET);
    
    int passed = 0;
    int total = 3;
    
    // Run tests
    if (test_single_layer() == 0) passed++;
    if (test_multi_layer() == 0) passed++;
    if (test_logits_and_softmax() == 0) passed++;
    
    // Print summary
    printf("\n%s╔════════════════════════════════════════════════════════╗%s\n", 
           COLOR_BLUE, COLOR_RESET);
    printf("%s║                    Test Summary                        ║%s\n", 
           COLOR_BLUE, COLOR_RESET);
    printf("%s╚════════════════════════════════════════════════════════╝%s\n", 
           COLOR_BLUE, COLOR_RESET);
    
    if (passed == total) {
        printf("\n%s✓ ALL TESTS PASSED (%d/%d)%s\n\n", 
               COLOR_GREEN, passed, total, COLOR_RESET);
        return 0;
    } else {
        printf("\n%s✗ SOME TESTS FAILED (%d/%d passed)%s\n\n", 
               COLOR_RED, passed, total, COLOR_RESET);
        return 1;
    }
}