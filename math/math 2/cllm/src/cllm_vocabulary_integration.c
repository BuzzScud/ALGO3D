#include "cllm.h"
#include "cllm_vocabulary.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// VOCABULARY INTEGRATION
// ============================================================================

void cllm_set_vocabulary(CLLMModel* model, CLLMVocabulary* vocab) {
    if (!model || !vocab) return;
    
    // Free existing vocabulary if present
    if (model->vocabulary) {
        cllm_vocab_destroy(model->vocabulary);
    }
    
    model->vocabulary = vocab;
    
    // Update vocab_size if needed
    if (vocab->size > model->vocab_size) {
        // Need to reallocate embeddings and token positions
        // For now, just update the size
        model->vocab_size = vocab->size;
    }
}

CLLMVocabulary* cllm_get_vocabulary(CLLMModel* model) {
    if (!model) return NULL;
    return model->vocabulary;
}

uint32_t cllm_build_vocabulary_from_file(CLLMModel* model, const char* filename) {
    if (!model || !filename) return 0;
    
    // Create vocabulary if it doesn't exist
    if (!model->vocabulary) {
        model->vocabulary = cllm_vocab_create_with_special_tokens(
            10000,  // Initial capacity
            "[PAD]", "[UNK]", "[BOS]", "[EOS]"
        );
        
        if (!model->vocabulary) return 0;
    }
    
    uint32_t tokens_added = cllm_vocab_build_from_file(model->vocabulary, filename);
    
    // Update model vocab_size
    if (model->vocabulary->size > model->vocab_size) {
        model->vocab_size = model->vocabulary->size;
    }
    
    return tokens_added;
}

uint32_t cllm_build_vocabulary_from_files(
    CLLMModel* model,
    const char** filenames,
    uint32_t num_files
) {
    if (!model || !filenames) return 0;
    
    // Create vocabulary if it doesn't exist
    if (!model->vocabulary) {
        model->vocabulary = cllm_vocab_create_with_special_tokens(
            10000,  // Initial capacity
            "[PAD]", "[UNK]", "[BOS]", "[EOS]"
        );
        
        if (!model->vocabulary) return 0;
    }
    
    uint32_t tokens_added = cllm_vocab_build_from_files(
        model->vocabulary,
        filenames,
        num_files
    );
    
    // Update model vocab_size
    if (model->vocabulary->size > model->vocab_size) {
        model->vocab_size = model->vocabulary->size;
    }
    
    return tokens_added;
}

bool cllm_save_vocabulary(CLLMModel* model, const char* filename) {
    if (!model || !model->vocabulary || !filename) return false;
    return cllm_vocab_save(model->vocabulary, filename);
}

bool cllm_load_vocabulary(CLLMModel* model, const char* filename) {
    if (!model || !filename) return false;
    
    CLLMVocabulary* vocab = cllm_vocab_load(filename);
    if (!vocab) return false;
    
    cllm_set_vocabulary(model, vocab);
    return true;
}

uint32_t* cllm_tokenize_text(CLLMModel* model, const char* text, uint32_t* num_tokens) {
    if (!model || !model->vocabulary || !text || !num_tokens) return NULL;
    return cllm_vocab_tokenize(model->vocabulary, text, num_tokens);
}

char* cllm_detokenize_text(CLLMModel* model, const uint32_t* token_ids, uint32_t num_tokens) {
    if (!model || !model->vocabulary || !token_ids) return NULL;
    return cllm_vocab_detokenize(model->vocabulary, token_ids, num_tokens);
}