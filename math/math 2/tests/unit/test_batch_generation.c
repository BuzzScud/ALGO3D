/**
 * Test Batch Generation
 */

#include "cllm_batch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ANSI_GREEN "\033[32m"
#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"

int tests_passed = 0;
int tests_failed = 0;

void test_batch_create() {
    printf("Running test: batch_create... ");
    
    CLLMBatch* batch = cllm_batch_create(4, 8);
    assert(batch != NULL);
    assert(batch->batch_size == 4);
    assert(batch->seq_len == 8);
    assert(batch->input_ids != NULL);
    assert(batch->target_ids != NULL);
    assert(batch->attention_mask != NULL);
    
    cllm_batch_free(batch);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

void test_batch_from_tokens() {
    printf("Running test: batch_from_tokens... ");
    
    // Create simple token sequence: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    uint32_t tokens[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t num_tokens = 10;
    
    CLLMBatch* batch = cllm_create_batch_from_tokens(tokens, num_tokens, 2, 4);
    assert(batch != NULL);
    assert(batch->batch_size == 2);
    assert(batch->seq_len == 4);
    
    // First sequence: input [1,2,3,4], target [2,3,4,5]
    assert(batch->input_ids[0] == 1);
    assert(batch->input_ids[1] == 2);
    assert(batch->input_ids[2] == 3);
    assert(batch->input_ids[3] == 4);
    assert(batch->target_ids[0] == 2);
    assert(batch->target_ids[1] == 3);
    assert(batch->target_ids[2] == 4);
    assert(batch->target_ids[3] == 5);
    
    // Second sequence: input [5,6,7,8], target [6,7,8,9]
    assert(batch->input_ids[4] == 5);
    assert(batch->input_ids[5] == 6);
    assert(batch->input_ids[6] == 7);
    assert(batch->input_ids[7] == 8);
    assert(batch->target_ids[4] == 6);
    assert(batch->target_ids[5] == 7);
    assert(batch->target_ids[6] == 8);
    assert(batch->target_ids[7] == 9);
    
    // Check attention masks
    for (int i = 0; i < 8; i++) {
        assert(batch->attention_mask[i] == 1.0f);
    }
    
    assert(cllm_batch_validate(batch) == 1);
    
    cllm_batch_free(batch);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

void test_batch_with_padding() {
    printf("Running test: batch_with_padding... ");
    
    // Create token sequence that doesn't fill batch completely
    uint32_t tokens[] = {1, 2, 3, 4, 5};
    size_t num_tokens = 5;
    
    CLLMBatch* batch = cllm_create_batch_from_tokens(tokens, num_tokens, 2, 4);
    assert(batch != NULL);
    
    // First sequence should be valid: [1,2,3,4] -> [2,3,4,5]
    assert(batch->input_ids[0] == 1);
    assert(batch->attention_mask[0] == 1.0f);
    assert(batch->attention_mask[1] == 1.0f);
    assert(batch->attention_mask[2] == 1.0f);
    assert(batch->attention_mask[3] == 1.0f);
    
    // Second sequence should be padded
    assert(batch->input_ids[4] == 0);  // PAD_TOKEN
    assert(batch->attention_mask[4] == 0.0f);
    assert(batch->attention_mask[5] == 0.0f);
    assert(batch->attention_mask[6] == 0.0f);
    assert(batch->attention_mask[7] == 0.0f);
    
    assert(batch->num_valid_tokens == 4);
    assert(cllm_batch_validate(batch) == 1);
    
    cllm_batch_free(batch);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

void test_batch_iterator() {
    printf("Running test: batch_iterator... ");
    
    // Create token sequence
    uint32_t tokens[100];
    for (int i = 0; i < 100; i++) {
        tokens[i] = i + 1;
    }
    
    CLLMBatchIterator* iter = cllm_batch_iterator_create(tokens, 100, 4, 5, 0, 1);
    assert(iter != NULL);
    
    size_t num_batches = cllm_batch_iterator_num_batches(iter);
    assert(num_batches == 4);  // 100 tokens / (4 batch * 5 seq) = 5, but drop_last=1 so 4
    
    // Get first batch
    CLLMBatch* batch1 = cllm_batch_iterator_next(iter);
    assert(batch1 != NULL);
    assert(batch1->batch_size == 4);
    assert(batch1->seq_len == 5);
    assert(batch1->input_ids[0] == 1);
    assert(cllm_batch_validate(batch1) == 1);
    cllm_batch_free(batch1);
    
    // Get second batch
    CLLMBatch* batch2 = cllm_batch_iterator_next(iter);
    assert(batch2 != NULL);
    assert(batch2->input_ids[0] == 21);  // Should start at token 21
    assert(cllm_batch_validate(batch2) == 1);
    cllm_batch_free(batch2);
    
    // Get third batch
    CLLMBatch* batch3 = cllm_batch_iterator_next(iter);
    assert(batch3 != NULL);
    assert(cllm_batch_validate(batch3) == 1);
    cllm_batch_free(batch3);
    
    // Get fourth batch
    CLLMBatch* batch4 = cllm_batch_iterator_next(iter);
    assert(batch4 != NULL);
    assert(cllm_batch_validate(batch4) == 1);
    cllm_batch_free(batch4);
    
    // Should be no more batches
    CLLMBatch* batch5 = cllm_batch_iterator_next(iter);
    assert(batch5 == NULL);
    
    cllm_batch_iterator_free(iter);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

void test_batch_iterator_reset() {
    printf("Running test: batch_iterator_reset... ");
    
    uint32_t tokens[50];
    for (int i = 0; i < 50; i++) {
        tokens[i] = i + 1;
    }
    
    CLLMBatchIterator* iter = cllm_batch_iterator_create(tokens, 50, 2, 5, 0, 0);
    assert(iter != NULL);
    
    // Get first batch
    CLLMBatch* batch1 = cllm_batch_iterator_next(iter);
    assert(batch1 != NULL);
    assert(batch1->input_ids[0] == 1);
    cllm_batch_free(batch1);
    
    // Get second batch
    CLLMBatch* batch2 = cllm_batch_iterator_next(iter);
    assert(batch2 != NULL);
    assert(batch2->input_ids[0] == 11);
    cllm_batch_free(batch2);
    
    // Reset iterator
    cllm_batch_iterator_reset(iter);
    
    // Get first batch again
    CLLMBatch* batch3 = cllm_batch_iterator_next(iter);
    assert(batch3 != NULL);
    assert(batch3->input_ids[0] == 1);  // Should be back to start
    cllm_batch_free(batch3);
    
    cllm_batch_iterator_free(iter);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

void test_batch_validation() {
    printf("Running test: batch_validation... ");
    
    // Create valid batch
    uint32_t tokens[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CLLMBatch* batch = cllm_create_batch_from_tokens(tokens, 10, 2, 4);
    assert(batch != NULL);
    
    // Should be valid
    assert(cllm_batch_validate(batch) == 1);
    
    // Corrupt attention mask
    batch->attention_mask[0] = 1.0f;
    batch->input_ids[0] = 0;  // PAD but mask says valid
    assert(cllm_batch_validate(batch) == 0);  // Should fail
    
    cllm_batch_free(batch);
    
    printf(ANSI_GREEN "PASSED" ANSI_RESET "\n");
    tests_passed++;
}

int main() {
    printf("\n");
    printf("========================================\n");
    printf("  Batch Generation Tests\n");
    printf("========================================\n");
    printf("\n");
    
    test_batch_create();
    test_batch_from_tokens();
    test_batch_with_padding();
    test_batch_iterator();
    test_batch_iterator_reset();
    test_batch_validation();
    
    printf("\n");
    printf("========================================\n");
    printf("  Test Summary\n");
    printf("========================================\n");
    printf("Tests passed: " ANSI_GREEN "%d" ANSI_RESET "\n", tests_passed);
    printf("Tests failed: " ANSI_RED "%d" ANSI_RESET "\n", tests_failed);
    printf("\n");
    
    return tests_failed > 0 ? 1 : 0;
}