/**
 * Production Smoke Test Suite
 * Quick validation tests for production deployment
 */

#include "ai/cllm_lattice_embeddings.h"
#include "ai/cllm_angular_attention.h"
#include "ai/cllm_ntt_attention.h"
#include "ai/cllm_cymatic_training.h"
#include "../production_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SMOKE_TEST_PASSED 0
#define SMOKE_TEST_FAILED 1

// Test result tracking
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Test macros
#define RUN_SMOKE_TEST(test_func) do { \
    printf("\n[TEST] Running: %s\n", #test_func); \
    tests_run++; \
    if (test_func() == SMOKE_TEST_PASSED) { \
        printf("✅ PASSED: %s\n", #test_func); \
        tests_passed++; \
    } else { \
        printf("❌ FAILED: %s\n", #test_func); \
        tests_failed++; \
    } \
} while(0)

/**
 * Smoke Test 1: Model Creation and Initialization
 */
static int smoke_test_model_creation() {
    ProductionConfig prod_config = get_default_production_config();
    
    CLLMConfig config = {
        .vocab_size = prod_config.vocab_size,
        .embed_dim = prod_config.embed_dim,
        .num_heads = prod_config.num_heads,
        .num_layers = prod_config.num_layers,
        .max_seq_len = prod_config.max_seq_len,
        .dropout = prod_config.dropout,
        .learning_rate = prod_config.learning_rate
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("  ERROR: Failed to create model\n");
        return SMOKE_TEST_FAILED;
    }
    
    // Verify model structure
    if (model->config.vocab_size != config.vocab_size) {
        printf("  ERROR: Vocab size mismatch\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    if (!model->embeddings) {
        printf("  ERROR: Embeddings not allocated\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    cllm_free_model(model);
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 2: Lattice Embeddings Initialization
 */
static int smoke_test_lattice_initialization() {
    ProductionConfig prod_config = get_default_production_config();
    
    CLLMConfig config = {
        .vocab_size = 1000,
        .embed_dim = 128,
        .num_heads = 4,
        .num_layers = 2,
        .max_seq_len = 512,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("  ERROR: Failed to create model\n");
        return SMOKE_TEST_FAILED;
    }
    
    int result = cllm_init_lattice_embeddings(
        model,
        prod_config.lattice_dimension,
        prod_config.kissing_number,
        prod_config.lambda
    );
    
    if (result != 0) {
        printf("  ERROR: Lattice initialization failed\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    // Verify embeddings are within valid range
    for (int i = 0; i < 100; i++) {
        float val = model->embeddings[i];
        if (val < -1.0f || val > 1.0f) {
            printf("  ERROR: Embedding value out of range: %.6f\n", val);
            cllm_free_model(model);
            return SMOKE_TEST_FAILED;
        }
    }
    
    cllm_free_model(model);
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 3: NTT Attention Forward Pass
 */
static int smoke_test_ntt_attention() {
    int seq_len = 256;
    int embed_dim = 128;
    int num_heads = 4;
    
    size_t matrix_size = seq_len * embed_dim * sizeof(float);
    float* query = (float*)malloc(matrix_size);
    float* key = (float*)malloc(matrix_size);
    float* value = (float*)malloc(matrix_size);
    float* output = (float*)malloc(matrix_size);
    
    if (!query || !key || !value || !output) {
        printf("  ERROR: Memory allocation failed\n");
        free(query); free(key); free(value); free(output);
        return SMOKE_TEST_FAILED;
    }
    
    // Initialize with test data
    for (int i = 0; i < seq_len * embed_dim; i++) {
        query[i] = 0.1f;
        key[i] = 0.1f;
        value[i] = 0.1f;
    }
    
    int result = cllm_attention_ntt_forward(
        query, key, value, output,
        seq_len, embed_dim, num_heads
    );
    
    if (result != 0) {
        printf("  ERROR: NTT attention failed\n");
        free(query); free(key); free(value); free(output);
        return SMOKE_TEST_FAILED;
    }
    
    // Verify output is not all zeros
    int non_zero_count = 0;
    for (int i = 0; i < seq_len * embed_dim; i++) {
        if (output[i] != 0.0f) {
            non_zero_count++;
        }
    }
    
    if (non_zero_count == 0) {
        printf("  ERROR: Output is all zeros\n");
        free(query); free(key); free(value); free(output);
        return SMOKE_TEST_FAILED;
    }
    
    free(query); free(key); free(value); free(output);
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 4: Cymatic Training Initialization
 */
static int smoke_test_cymatic_training() {
    ProductionConfig prod_config = get_default_production_config();
    
    CLLMConfig config = {
        .vocab_size = 1000,
        .embed_dim = 128,
        .num_heads = 4,
        .num_layers = 2,
        .max_seq_len = 512,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("  ERROR: Failed to create model\n");
        return SMOKE_TEST_FAILED;
    }
    
    CymaticConfig cymatic_config = {
        .base_frequency = prod_config.base_frequency,
        .harmonic_count = prod_config.harmonic_count,
        .resonance_threshold = DEFAULT_RESONANCE_THRESHOLD,
        .phase_shift = DEFAULT_PHASE_SHIFT
    };
    
    int result = cllm_init_cymatic_training(model, &cymatic_config);
    if (result != 0) {
        printf("  ERROR: Cymatic training initialization failed\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    if (!model->cymatic_state) {
        printf("  ERROR: Cymatic state not initialized\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    cllm_free_model(model);
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 5: Forward Pass
 */
static int smoke_test_forward_pass() {
    CLLMConfig config = {
        .vocab_size = 1000,
        .embed_dim = 128,
        .num_heads = 4,
        .num_layers = 2,
        .max_seq_len = 64,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("  ERROR: Failed to create model\n");
        return SMOKE_TEST_FAILED;
    }
    
    // Create test input
    int seq_len = 32;
    int* tokens = (int*)malloc(seq_len * sizeof(int));
    if (!tokens) {
        printf("  ERROR: Failed to allocate tokens\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    for (int i = 0; i < seq_len; i++) {
        tokens[i] = i % config.vocab_size;
    }
    
    // Run forward pass
    float* logits = cllm_forward(model, tokens, seq_len);
    if (!logits) {
        printf("  ERROR: Forward pass failed\n");
        free(tokens);
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    // Verify logits are reasonable
    int valid_count = 0;
    for (int i = 0; i < config.vocab_size; i++) {
        if (logits[i] >= -100.0f && logits[i] <= 100.0f) {
            valid_count++;
        }
    }
    
    if (valid_count < config.vocab_size / 2) {
        printf("  ERROR: Too many invalid logits\n");
        free(tokens);
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    free(tokens);
    cllm_free_model(model);
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 6: Memory Management
 */
static int smoke_test_memory_management() {
    // Create and destroy multiple models to test memory management
    for (int i = 0; i < 10; i++) {
        CLLMConfig config = {
            .vocab_size = 1000,
            .embed_dim = 128,
            .num_heads = 4,
            .num_layers = 2,
            .max_seq_len = 512,
            .dropout = 0.1f,
            .learning_rate = 0.001f
        };
        
        CLLMModel* model = cllm_create_model(&config);
        if (!model) {
            printf("  ERROR: Failed to create model (iteration %d)\n", i);
            return SMOKE_TEST_FAILED;
        }
        
        cllm_free_model(model);
    }
    
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 7: Configuration Validation
 */
static int smoke_test_configuration() {
    ProductionConfig config = get_default_production_config();
    
    // Validate configuration values
    if (config.vocab_size <= 0 || config.vocab_size > MAX_VOCAB_SIZE) {
        printf("  ERROR: Invalid vocab_size: %d\n", config.vocab_size);
        return SMOKE_TEST_FAILED;
    }
    
    if (config.embed_dim <= 0 || config.embed_dim > MAX_EMBED_DIM) {
        printf("  ERROR: Invalid embed_dim: %d\n", config.embed_dim);
        return SMOKE_TEST_FAILED;
    }
    
    if (config.num_heads <= 0 || config.num_heads > MAX_NUM_HEADS) {
        printf("  ERROR: Invalid num_heads: %d\n", config.num_heads);
        return SMOKE_TEST_FAILED;
    }
    
    if (config.num_layers <= 0 || config.num_layers > MAX_NUM_LAYERS) {
        printf("  ERROR: Invalid num_layers: %d\n", config.num_layers);
        return SMOKE_TEST_FAILED;
    }
    
    if (config.learning_rate <= 0.0f || config.learning_rate > 1.0f) {
        printf("  ERROR: Invalid learning_rate: %.6f\n", config.learning_rate);
        return SMOKE_TEST_FAILED;
    }
    
    return SMOKE_TEST_PASSED;
}

/**
 * Smoke Test 8: End-to-End Pipeline
 */
static int smoke_test_end_to_end() {
    ProductionConfig prod_config = get_default_production_config();
    
    CLLMConfig config = {
        .vocab_size = 1000,
        .embed_dim = 128,
        .num_heads = 4,
        .num_layers = 2,
        .max_seq_len = 64,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    // Create model
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        printf("  ERROR: Failed to create model\n");
        return SMOKE_TEST_FAILED;
    }
    
    // Initialize lattice embeddings
    if (cllm_init_lattice_embeddings(model, 8, 24, 1.0f) != 0) {
        printf("  ERROR: Lattice initialization failed\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    // Initialize cymatic training
    CymaticConfig cymatic_config = {
        .base_frequency = 432.0f,
        .harmonic_count = 8,
        .resonance_threshold = 0.7f,
        .phase_shift = 0.0f
    };
    
    if (cllm_init_cymatic_training(model, &cymatic_config) != 0) {
        printf("  ERROR: Cymatic initialization failed\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    // Run forward pass
    int seq_len = 32;
    int* tokens = (int*)malloc(seq_len * sizeof(int));
    if (!tokens) {
        printf("  ERROR: Failed to allocate tokens\n");
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    for (int i = 0; i < seq_len; i++) {
        tokens[i] = i % config.vocab_size;
    }
    
    float* logits = cllm_forward(model, tokens, seq_len);
    if (!logits) {
        printf("  ERROR: Forward pass failed\n");
        free(tokens);
        cllm_free_model(model);
        return SMOKE_TEST_FAILED;
    }
    
    free(tokens);
    cllm_free_model(model);
    return SMOKE_TEST_PASSED;
}

/**
 * Main smoke test runner
 */
int main(int argc, char** argv) {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║     CRYSTALLINE CLLM - PRODUCTION SMOKE TESTS                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Run all smoke tests
    RUN_SMOKE_TEST(smoke_test_configuration);
    RUN_SMOKE_TEST(smoke_test_model_creation);
    RUN_SMOKE_TEST(smoke_test_lattice_initialization);
    RUN_SMOKE_TEST(smoke_test_ntt_attention);
    RUN_SMOKE_TEST(smoke_test_cymatic_training);
    RUN_SMOKE_TEST(smoke_test_forward_pass);
    RUN_SMOKE_TEST(smoke_test_memory_management);
    RUN_SMOKE_TEST(smoke_test_end_to_end);
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                    SMOKE TEST SUMMARY                          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Total Tests:  %d\n", tests_run);
    printf("  Passed:       %d (%.1f%%)\n", tests_passed, 
           (tests_passed * 100.0f) / tests_run);
    printf("  Failed:       %d (%.1f%%)\n", tests_failed,
           (tests_failed * 100.0f) / tests_run);
    printf("\n");
    
    if (tests_failed == 0) {
        printf("✅ ALL SMOKE TESTS PASSED - READY FOR PRODUCTION\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED - NOT READY FOR PRODUCTION\n");
        return 1;
    }
}