#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/cllm.h"
#include "include/cllm_format.h"
#include "include/cllm_vocabulary.h"

int main() {
    printf("=== Vocabulary Integration Test ===\n\n");
    
    // Create a simple model
    CLLMConfig config = cllm_default_config(PLATONIC_CUBE, 100);
    config.enable_blind_recovery = false;
    config.enable_harmonic_integration = false;
    config.enable_ntt_attention = false;
    config.enable_kissing_spheres = false;
    
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // Create and populate vocabulary
    printf("Creating vocabulary...\n");
    CLLMVocabulary* vocab = cllm_vocab_create_with_special_tokens(
        100, "<PAD>", "<UNK>", "<BOS>", "<EOS>"
    );
    
    // Add some test tokens
    const char* test_tokens[] = {
        "hello", "world", "the", "a", "is", "are", "was", "were",
        "cat", "dog", "house", "tree", "run", "jump", "eat", "sleep"
    };
    
    for (int i = 0; i < 16; i++) {
        cllm_vocab_add_token(vocab, test_tokens[i]);
    }
    
    printf("✓ Vocabulary created with %u tokens\n", vocab->size);
    printf("  Special tokens: PAD=%u, UNK=%u, BOS=%u, EOS=%u\n",
           vocab->pad_token_id, vocab->unk_token_id, 
           vocab->bos_token_id, vocab->eos_token_id);
    
    // Set vocabulary in model
    cllm_set_vocabulary(model, vocab);
    printf("✓ Vocabulary attached to model\n\n");
    
    // Save model with vocabulary
    printf("Saving model with vocabulary...\n");
    const char* test_file = "test_vocab_model.cllm";
    if (cllm_write_model(model, test_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
        cllm_free_model(model);
        return 1;
    }
    printf("\n");
    
    // Free original model
    cllm_free_model(model);
    printf("✓ Original model freed\n\n");
    
    // Load model back
    printf("Loading model with vocabulary...\n");
    CLLMModel* loaded_model = cllm_read_model(test_file);
    if (!loaded_model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    printf("\n");
    
    // Verify vocabulary was loaded
    if (!loaded_model->vocabulary) {
        fprintf(stderr, "✗ Vocabulary was NOT loaded!\n");
        cllm_free_model(loaded_model);
        return 1;
    }
    
    printf("✓ Vocabulary loaded successfully!\n");
    printf("  Size: %u tokens\n", loaded_model->vocabulary->size);
    printf("  Special tokens: PAD=%u, UNK=%u, BOS=%u, EOS=%u\n",
           loaded_model->vocabulary->pad_token_id,
           loaded_model->vocabulary->unk_token_id,
           loaded_model->vocabulary->bos_token_id,
           loaded_model->vocabulary->eos_token_id);
    
    // Verify tokens match
    printf("\n  Verifying tokens:\n");
    bool all_match = true;
    for (int i = 0; i < 16 && i < loaded_model->vocabulary->size; i++) {
        const char* token = cllm_vocab_get_token(loaded_model->vocabulary, i + 4); // +4 for special tokens
        if (token && strcmp(token, test_tokens[i]) == 0) {
            printf("    ✓ Token %d: '%s'\n", i, token);
        } else {
            printf("    ✗ Token %d: expected '%s', got '%s'\n", i, test_tokens[i], token ? token : "NULL");
            all_match = false;
        }
    }
    
    if (all_match) {
        printf("\n✅ ALL TESTS PASSED! Vocabulary integration working correctly.\n");
    } else {
        printf("\n❌ SOME TESTS FAILED! Vocabulary integration has issues.\n");
    }
    
    // Cleanup
    cllm_free_model(loaded_model);
    remove(test_file);
    
    return all_match ? 0 : 1;
}