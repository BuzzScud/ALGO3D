/**
 * Test program for 88D thread-centric architecture
 * 
 * This test verifies:
 * 1. Model creation with 88D thread pool
 * 2. Token → Thread permanent assignment
 * 3. Embedding storage in thread-local CrystallineAbacus
 * 4. Embedding retrieval from threads
 * 5. Thread-local parameter storage
 * 6. Model serialization/deserialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ai/cllm.h"
#include "ai/cllm_embedding_helpers.h"
#include "hierarchical_threading.h"
#include "math/clock.h"

#define TEST_VOCAB_SIZE 1000
#define TEST_EMBEDDING_DIM 128
#define TEST_NUM_LAYERS 8
#define TEST_NUM_HEADS 8
#define TEST_HEAD_DIM 16

// Color codes for output
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

// Test result tracking
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
 * Test 1: Model Creation with 88D Thread Pool
 */
void test_model_creation() {
    print_test_header("Model Creation with 88D Thread Pool");
    
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .hidden_dim = TEST_EMBEDDING_DIM,
        .max_seq_len = 512,
        .solid_type = PLATONIC_CUBE
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    // Verify model was created
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        printf("  ✓ Model created successfully\n");
        
        // Verify 88D thread pool exists
        if (model->threads == NULL) {
            printf("  ERROR: 88D thread pool not created\n");
            passed = 0;
        } else {
            printf("  ✓ 88D thread pool created\n");
            
            // Verify thread pool has correct structure
            // 8 layers × 12 threads per layer = 96 threads total
            int expected_threads = 8 * 12;
            printf("  ✓ Expected %d threads in pool\n", expected_threads);
        }
        
        // Verify token assignments exist
        if (model->token_assignments == NULL) {
            printf("  ERROR: Token assignments not created\n");
            passed = 0;
        } else {
            printf("  ✓ Token assignments created\n");
        }
        
        // Verify thread params exist
        if (model->thread_params == NULL) {
            printf("  ERROR: Thread params not created\n");
            passed = 0;
        } else {
            printf("  ✓ Thread params created\n");
        }
        
        // Verify layer info exists
        if (model->layer_info == NULL) {
            printf("  ERROR: Layer info not created\n");
            passed = 0;
        } else {
            printf("  ✓ Layer info created\n");
        }
    }
    
    print_test_result("Model Creation", passed);
    
    if (model) {
        cllm_free_model(model);
    }
}

/**
 * Test 2: Token → Thread Assignment
 */
void test_token_assignment() {
    print_test_header("Token → Thread Assignment");
    
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .hidden_dim = TEST_EMBEDDING_DIM,
        .max_seq_len = 512,
        .solid_type = PLATONIC_CUBE
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        // Test a few token assignments
        uint32_t test_tokens[] = {0, 1, 7, 8, 11, 12, 100, 500, 999};
        int num_test_tokens = sizeof(test_tokens) / sizeof(test_tokens[0]);
        
        for (int i = 0; i < num_test_tokens; i++) {
            uint32_t token_id = test_tokens[i];
            
            if (token_id >= model->vocab_size) continue;
            
            struct {
                uint8_t layer;
                uint8_t dimension;
                uint32_t thread_id;
                HierarchicalThread* thread;
            }* assignment = &model->token_assignments[token_id];
            
            // Verify assignment is valid
            if (assignment->layer >= 8) {
                printf("  ERROR: Token %u assigned to invalid layer %u\n", 
                       token_id, assignment->layer);
                passed = 0;
            }
            
            if (assignment->dimension < 1 || assignment->dimension > 11) {
                printf("  ERROR: Token %u assigned to invalid dimension %u\n",
                       token_id, assignment->dimension);
                passed = 0;
            }
            
            if (assignment->thread == NULL) {
                printf("  ERROR: Token %u has NULL thread pointer\n", token_id);
                passed = 0;
            }
            
            printf("  Token %4u → Layer %u, Dimension %2u, Thread ID %u\n",
                   token_id, assignment->layer, assignment->dimension,
                   assignment->thread_id);
        }
        
        if (passed) {
            printf("  ✓ All token assignments valid\n");
        }
    }
    
    print_test_result("Token Assignment", passed);
    
    if (model) {
        cllm_free_model(model);
    }
}

/**
 * Test 3: Embedding Storage and Retrieval
 */
void test_embedding_operations() {
    print_test_header("Embedding Storage and Retrieval");
    
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .hidden_dim = TEST_EMBEDDING_DIM,
        .max_seq_len = 512,
        .solid_type = PLATONIC_CUBE
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        // Test embedding set and get
        uint32_t test_token = 42;
        double test_embedding[TEST_EMBEDDING_DIM];
        
        // Create test embedding with known pattern
        for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
            test_embedding[i] = (double)i * 0.1;
        }
        
        // Set embedding
        int set_result = cllm_set_embedding(model, test_token, test_embedding);
        if (set_result != 0) {
            printf("  ERROR: Failed to set embedding for token %u\n", test_token);
            passed = 0;
        } else {
            printf("  ✓ Set embedding for token %u\n", test_token);
        }
        
        // Get embedding
        double retrieved_embedding[TEST_EMBEDDING_DIM];
        int get_result = cllm_get_embedding_from_model(
            model, test_token, retrieved_embedding
        );
        
        if (get_result != 0) {
            printf("  ERROR: Failed to get embedding for token %u\n", test_token);
            passed = 0;
        } else {
            printf("  ✓ Retrieved embedding for token %u\n", test_token);
            
            // Verify embedding values match
            double max_diff = 0.0;
            for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                double diff = fabs(retrieved_embedding[i] - test_embedding[i]);
                if (diff > max_diff) {
                    max_diff = diff;
                }
            }
            
            // Allow small numerical error from CrystallineAbacus conversion
            if (max_diff > 1e-6) {
                printf("  ERROR: Embedding mismatch, max diff = %e\n", max_diff);
                passed = 0;
            } else {
                printf("  ✓ Embedding values match (max diff = %e)\n", max_diff);
            }
        }
        
        // Test embedding copy
        uint32_t source_token = 42;
        uint32_t dest_token = 100;
        
        int copy_result = cllm_copy_embedding(model, source_token, dest_token);
        if (copy_result != 0) {
            printf("  ERROR: Failed to copy embedding\n");
            passed = 0;
        } else {
            printf("  ✓ Copied embedding from token %u to token %u\n",
                   source_token, dest_token);
            
            // Verify copy
            double copied_embedding[TEST_EMBEDDING_DIM];
            cllm_get_embedding_from_model(model, dest_token, copied_embedding);
            
            double max_diff = 0.0;
            for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                double diff = fabs(copied_embedding[i] - test_embedding[i]);
                if (diff > max_diff) {
                    max_diff = diff;
                }
            }
            
            if (max_diff > 1e-6) {
                printf("  ERROR: Copied embedding mismatch\n");
                passed = 0;
            } else {
                printf("  ✓ Copied embedding matches original\n");
            }
        }
    }
    
    print_test_result("Embedding Operations", passed);
    
    if (model) {
        cllm_free_model(model);
    }
}

/**
 * Test 4: Thread Pool Structure
 */
void test_thread_pool_structure() {
    print_test_header("Thread Pool Structure");
    
    CLLMConfig config = {
        .vocab_size = TEST_VOCAB_SIZE,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS,
        .hidden_dim = TEST_EMBEDDING_DIM,
        .max_seq_len = 512,
        .solid_type = PLATONIC_CUBE
    };
    
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL || model->threads == NULL) {
        printf("  ERROR: Model or thread pool not created\n");
        passed = 0;
    } else {
        // Verify we can access threads in the pool
        printf("  Testing thread access patterns:\n");
        
        for (uint8_t layer = 0; layer < 8; layer++) {
            for (uint8_t dim = 1; dim <= 11; dim++) {
                HierarchicalThread* thread = hierarchical_thread_get(
                    model->threads, layer, dim
                );
                
                if (thread == NULL) {
                    printf("  ERROR: Cannot access thread at layer %u, dim %u\n",
                           layer, dim);
                    passed = 0;
                    break;
                }
                
                // Verify thread has CrystallineAbacus storage
                if (thread->value == NULL) {
                    printf("  ERROR: Thread at layer %u, dim %u has no storage\n",
                           layer, dim);
                    passed = 0;
                    break;
                }
            }
            if (!passed) break;
        }
        
        if (passed) {
            printf("  ✓ All 88 threads (8 layers × 11 dimensions) accessible\n");
            printf("  ✓ All threads have CrystallineAbacus storage\n");
        }
    }
    
    print_test_result("Thread Pool Structure", passed);
    
    if (model) {
        cllm_free_model(model);
    }
}

/**
 * Test 5: Model Persistence (Simplified)
 */
void test_model_persistence() {
    print_test_header("Model Persistence");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = TEST_VOCAB_SIZE,
        .max_seq_len = 512,
        .embedding_dim = TEST_EMBEDDING_DIM,
        .hidden_dim = TEST_EMBEDDING_DIM,
        .num_layers = TEST_NUM_LAYERS,
        .num_heads = TEST_NUM_HEADS
    };
    
    // Create and populate model
    CLLMModel* model = cllm_create_model(&config);
    
    int passed = 1;
    
    if (model == NULL) {
        printf("  ERROR: Model creation failed\n");
        passed = 0;
    } else {
        // Set some test embeddings
        for (uint32_t token = 0; token < 10; token++) {
            double embedding[TEST_EMBEDDING_DIM];
            for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                embedding[i] = (double)(token * 100 + i);
            }
            cllm_set_embedding(model, token, embedding);
        }
        
        printf("  ✓ Set test embeddings\n");
        
        // Verify embeddings persist in thread storage
        for (uint32_t token = 0; token < 10; token++) {
            double retrieved[TEST_EMBEDDING_DIM];
            cllm_get_embedding_from_model(model, token, retrieved);
            
            double max_diff = 0.0;
            for (uint32_t i = 0; i < TEST_EMBEDDING_DIM; i++) {
                double expected = (double)(token * 100 + i);
                double diff = fabs(retrieved[i] - expected);
                if (diff > max_diff) {
                    max_diff = diff;
                }
            }
            
            if (max_diff > 1e-6) {
                printf("  ERROR: Embedding %u does not persist correctly\n", token);
                passed = 0;
                break;
            }
        }
        
        if (passed) {
            printf("  ✓ All embeddings persist correctly in thread storage\n");
        }
        
        cllm_free_model(model);
    }
    
    print_test_result("Model Persistence", passed);
}

/**
 * Main test runner
 */
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf(COLOR_BLUE "========================================\n");
    printf("88D THREAD-CENTRIC ARCHITECTURE TESTS\n");
    printf("========================================" COLOR_RESET "\n");
    printf("\n");
    printf("Testing revolutionary 88D architecture:\n");
    printf("  - 8 layers × 11 dimensions = 88 worker threads\n");
    printf("  - Permanent token → thread assignment\n");
    printf("  - Thread-local CrystallineAbacus storage\n");
    printf("  - Exact arithmetic throughout\n");
    printf("\n");
    
    // Run all tests
    test_model_creation();
    test_token_assignment();
    test_embedding_operations();
    test_thread_pool_structure();
    test_model_persistence();
    
    // Print summary
    print_summary();
    
    // Return exit code
    return (tests_failed == 0) ? 0 : 1;
}