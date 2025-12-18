#include "cllm_vocabulary.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

static char* strdup_safe(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

__attribute__((unused))
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

// ============================================================================
// VOCABULARY CREATION & DESTRUCTION
// ============================================================================

CLLMVocabulary* cllm_vocab_create(uint32_t capacity) {
    CLLMVocabulary* vocab = calloc(1, sizeof(CLLMVocabulary));
    if (!vocab) return NULL;
    
    vocab->tokens = calloc(capacity, sizeof(char*));
    vocab->frequencies = calloc(capacity, sizeof(uint32_t));
    
    if (!vocab->tokens || !vocab->frequencies) {
        free(vocab->tokens);
        free(vocab->frequencies);
        free(vocab);
        return NULL;
    }
    
    vocab->capacity = capacity;
    vocab->size = 0;
    vocab->total_tokens = 0;
    
    // Initialize special token IDs to invalid values
    vocab->pad_token_id = UINT32_MAX;
    vocab->unk_token_id = UINT32_MAX;
    vocab->bos_token_id = UINT32_MAX;
    vocab->eos_token_id = UINT32_MAX;
    
    vocab->name = strdup_safe("default");
    
    return vocab;
}

CLLMVocabulary* cllm_vocab_create_with_special_tokens(
    uint32_t capacity,
    const char* pad_token,
    const char* unk_token,
    const char* bos_token,
    const char* eos_token
) {
    CLLMVocabulary* vocab = cllm_vocab_create(capacity);
    if (!vocab) return NULL;
    
    // Add special tokens
    if (pad_token) vocab->pad_token_id = cllm_vocab_add_token(vocab, pad_token);
    if (unk_token) vocab->unk_token_id = cllm_vocab_add_token(vocab, unk_token);
    if (bos_token) vocab->bos_token_id = cllm_vocab_add_token(vocab, bos_token);
    if (eos_token) vocab->eos_token_id = cllm_vocab_add_token(vocab, eos_token);
    
    return vocab;
}

void cllm_vocab_destroy(CLLMVocabulary* vocab) {
    if (!vocab) return;
    
    // Free all token strings
    for (uint32_t i = 0; i < vocab->size; i++) {
        free(vocab->tokens[i]);
    }
    
    free(vocab->tokens);
    free(vocab->frequencies);
    free(vocab->name);
    free(vocab);
}

// ============================================================================
// TOKEN MANAGEMENT
// ============================================================================

uint32_t cllm_vocab_add_token(CLLMVocabulary* vocab, const char* token) {
    if (!vocab || !token) return UINT32_MAX;
    
    // Check if token already exists
    for (uint32_t i = 0; i < vocab->size; i++) {
        if (strcmp(vocab->tokens[i], token) == 0) {
            vocab->frequencies[i]++;
            vocab->total_tokens++;
            return i;
        }
    }
    
    // Add new token
    if (vocab->size >= vocab->capacity) {
        // Resize if needed
        if (!cllm_vocab_resize(vocab, vocab->capacity * 2)) {
            return UINT32_MAX;
        }
    }
    
    vocab->tokens[vocab->size] = strdup_safe(token);
    if (!vocab->tokens[vocab->size]) {
        return UINT32_MAX;
    }
    
    vocab->frequencies[vocab->size] = 1;
    vocab->total_tokens++;
    
    return vocab->size++;
}

uint32_t cllm_vocab_get_id(const CLLMVocabulary* vocab, const char* token) {
    if (!vocab || !token) return vocab ? vocab->unk_token_id : UINT32_MAX;
    
    for (uint32_t i = 0; i < vocab->size; i++) {
        if (strcmp(vocab->tokens[i], token) == 0) {
            return i;
        }
    }
    
    return vocab->unk_token_id;
}

const char* cllm_vocab_get_token(const CLLMVocabulary* vocab, uint32_t id) {
    if (!vocab || id >= vocab->size) return NULL;
    return vocab->tokens[id];
}

bool cllm_vocab_contains(const CLLMVocabulary* vocab, const char* token) {
    return cllm_vocab_get_id(vocab, token) != vocab->unk_token_id;
}

uint32_t cllm_vocab_get_frequency(const CLLMVocabulary* vocab, uint32_t id) {
    if (!vocab || id >= vocab->size) return 0;
    return vocab->frequencies[id];
}

// ============================================================================
// TOKENIZATION
// ============================================================================

uint32_t* cllm_vocab_tokenize(
    CLLMVocabulary* vocab,
    const char* text,
    uint32_t* num_tokens
) {
    if (!vocab || !text || !num_tokens) return NULL;
    
    *num_tokens = 0;
    
    // Allocate temporary buffer for tokens
    uint32_t* tokens = malloc(strlen(text) * sizeof(uint32_t));
    if (!tokens) return NULL;
    
    char buffer[256];
    int buf_pos = 0;
    
    for (int i = 0; text[i]; i++) {
        char c = text[i];
        
        // Check if character is alphanumeric or apostrophe
        if (isalnum(c) || c == '\'') {
            if (buf_pos < 255) {
                buffer[buf_pos++] = tolower(c);
            }
        } else {
            // End of word
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                tokens[*num_tokens] = cllm_vocab_add_token(vocab, buffer);
                (*num_tokens)++;
                buf_pos = 0;
            }
        }
    }
    
    // Handle last word
    if (buf_pos > 0) {
        buffer[buf_pos] = '\0';
        tokens[*num_tokens] = cllm_vocab_add_token(vocab, buffer);
        (*num_tokens)++;
    }
    
    // Resize to actual size
    if (*num_tokens == 0) {
        free(tokens);
        return NULL;
    }
    
    uint32_t* result = realloc(tokens, (*num_tokens) * sizeof(uint32_t));
    return result ? result : tokens;
}

char* cllm_vocab_detokenize(
    const CLLMVocabulary* vocab,
    const uint32_t* token_ids,
    uint32_t num_tokens
) {
    if (!vocab || !token_ids || num_tokens == 0) return NULL;
    
    // Calculate total length needed
    size_t total_len = 0;
    for (uint32_t i = 0; i < num_tokens; i++) {
        const char* token = cllm_vocab_get_token(vocab, token_ids[i]);
        if (token) {
            total_len += strlen(token) + 1; // +1 for space
        }
    }
    
    if (total_len == 0) return NULL;
    
    // Allocate result string
    char* result = malloc(total_len + 1);
    if (!result) return NULL;
    
    // Build string
    size_t pos = 0;
    for (uint32_t i = 0; i < num_tokens; i++) {
        const char* token = cllm_vocab_get_token(vocab, token_ids[i]);
        if (token) {
            size_t token_len = strlen(token);
            memcpy(result + pos, token, token_len);
            pos += token_len;
            
            if (i < num_tokens - 1) {
                result[pos++] = ' ';
            }
        }
    }
    
    result[pos] = '\0';
    return result;
}

// ============================================================================
// BATCH TOKENIZATION
// ============================================================================

uint32_t** cllm_vocab_tokenize_batch(
    CLLMVocabulary* vocab,
    const char** texts,
    uint32_t num_texts,
    uint32_t** num_tokens_per_text
) {
    if (!vocab || !texts || num_texts == 0) return NULL;
    
    uint32_t** token_arrays = malloc(num_texts * sizeof(uint32_t*));
    *num_tokens_per_text = malloc(num_texts * sizeof(uint32_t));
    
    if (!token_arrays || !*num_tokens_per_text) {
        free(token_arrays);
        free(*num_tokens_per_text);
        return NULL;
    }
    
    for (uint32_t i = 0; i < num_texts; i++) {
        token_arrays[i] = cllm_vocab_tokenize(vocab, texts[i], &(*num_tokens_per_text)[i]);
    }
    
    return token_arrays;
}

void cllm_vocab_free_batch(uint32_t** token_arrays, uint32_t num_arrays) {
    if (!token_arrays) return;
    
    for (uint32_t i = 0; i < num_arrays; i++) {
        free(token_arrays[i]);
    }
    free(token_arrays);
}

// ============================================================================
// VOCABULARY BUILDING
// ============================================================================

uint32_t cllm_vocab_build_from_file(
    CLLMVocabulary* vocab,
    const char* filename
) {
    if (!vocab || !filename) return 0;
    
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    
    uint32_t initial_size = vocab->size;
    char buffer[4096];
    
    while (fgets(buffer, sizeof(buffer), f)) {
        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // Tokenize line
        uint32_t num_tokens;
        uint32_t* tokens = cllm_vocab_tokenize(vocab, buffer, &num_tokens);
        free(tokens);
    }
    
    fclose(f);
    return vocab->size - initial_size;
}

uint32_t cllm_vocab_build_from_files(
    CLLMVocabulary* vocab,
    const char** filenames,
    uint32_t num_files
) {
    if (!vocab || !filenames) return 0;
    
    uint32_t initial_size = vocab->size;
    
    for (uint32_t i = 0; i < num_files; i++) {
        cllm_vocab_build_from_file(vocab, filenames[i]);
    }
    
    return vocab->size - initial_size;
}

uint32_t cllm_vocab_build_from_texts(
    CLLMVocabulary* vocab,
    const char** texts,
    uint32_t num_texts
) {
    if (!vocab || !texts) return 0;
    
    uint32_t initial_size = vocab->size;
    
    for (uint32_t i = 0; i < num_texts; i++) {
        uint32_t num_tokens;
        uint32_t* tokens = cllm_vocab_tokenize(vocab, texts[i], &num_tokens);
        free(tokens);
    }
    
    return vocab->size - initial_size;
}

// ============================================================================
// VOCABULARY PERSISTENCE
// ============================================================================

bool cllm_vocab_save(const CLLMVocabulary* vocab, const char* filename) {
    if (!vocab || !filename) return false;
    
    FILE* f = fopen(filename, "w");
    if (!f) return false;
    
    // Write header
    fprintf(f, "CLLM_VOCAB_V1\n");
    fprintf(f, "%u %u %lu\n", vocab->size, vocab->capacity, vocab->total_tokens);
    fprintf(f, "%u %u %u %u\n", 
            vocab->pad_token_id, vocab->unk_token_id,
            vocab->bos_token_id, vocab->eos_token_id);
    
    if (vocab->name) {
        fprintf(f, "%s\n", vocab->name);
    } else {
        fprintf(f, "unnamed\n");
    }
    
    // Write tokens and frequencies
    for (uint32_t i = 0; i < vocab->size; i++) {
        fprintf(f, "%s\t%u\n", vocab->tokens[i], vocab->frequencies[i]);
    }
    
    fclose(f);
    return true;
}

CLLMVocabulary* cllm_vocab_load(const char* filename) {
    if (!filename) return NULL;
    
    FILE* f = fopen(filename, "r");
    if (!f) return NULL;
    
    char header[64];
    if (!fgets(header, sizeof(header), f) || 
        strncmp(header, "CLLM_VOCAB_V1", 13) != 0) {
        fclose(f);
        return NULL;
    }
    
    uint32_t size, capacity;
    uint64_t total_tokens;
    if (fscanf(f, "%u %u %lu\n", &size, &capacity, &total_tokens) != 3) {
        fclose(f);
        return NULL;
    }
    
    CLLMVocabulary* vocab = cllm_vocab_create(capacity);
    if (!vocab) {
        fclose(f);
        return NULL;
    }
    
    vocab->size = size;
    vocab->total_tokens = total_tokens;
    
    // Read special token IDs
    if (fscanf(f, "%u %u %u %u\n",
               &vocab->pad_token_id, &vocab->unk_token_id,
               &vocab->bos_token_id, &vocab->eos_token_id) != 4) {
        cllm_vocab_destroy(vocab);
        fclose(f);
        return NULL;
    }
    
    // Read name
    char name[256];
    if (fgets(name, sizeof(name), f)) {
        size_t len = strlen(name);
        if (len > 0 && name[len-1] == '\n') {
            name[len-1] = '\0';
        }
        free(vocab->name);
        vocab->name = strdup_safe(name);
    }
    
    // Read tokens and frequencies
    for (uint32_t i = 0; i < size; i++) {
        char token[256];
        uint32_t freq;
        
        if (fscanf(f, "%255s\t%u\n", token, &freq) != 2) {
            cllm_vocab_destroy(vocab);
            fclose(f);
            return NULL;
        }
        
        vocab->tokens[i] = strdup_safe(token);
        vocab->frequencies[i] = freq;
    }
    
    fclose(f);
    return vocab;
}

// ============================================================================
// VOCABULARY STATISTICS
// ============================================================================

VocabStats cllm_vocab_get_stats(const CLLMVocabulary* vocab) {
    VocabStats stats = {0};
    
    if (!vocab) return stats;
    
    stats.vocab_size = vocab->size;
    stats.total_tokens = vocab->total_tokens;
    stats.unique_tokens = vocab->size;
    
    if (vocab->size == 0) return stats;
    
    uint32_t total_len = 0;
    stats.max_token_length = 0;
    stats.min_token_length = UINT32_MAX;
    
    for (uint32_t i = 0; i < vocab->size; i++) {
        uint32_t len = strlen(vocab->tokens[i]);
        total_len += len;
        
        if (len > stats.max_token_length) {
            stats.max_token_length = len;
        }
        if (len < stats.min_token_length) {
            stats.min_token_length = len;
        }
    }
    
    stats.avg_token_length = (double)total_len / vocab->size;
    
    return stats;
}

void cllm_vocab_print_stats(const CLLMVocabulary* vocab) {
    if (!vocab) return;
    
    VocabStats stats = cllm_vocab_get_stats(vocab);
    
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  VOCABULARY STATISTICS\n");
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    printf("Name: %s\n", vocab->name ? vocab->name : "unnamed");
    printf("Vocabulary Size: %u unique tokens\n", stats.vocab_size);
    printf("Total Tokens Processed: %u\n", stats.total_tokens);
    printf("Average Token Length: %.2f characters\n", stats.avg_token_length);
    printf("Token Length Range: %u - %u characters\n", 
           stats.min_token_length, stats.max_token_length);
    printf("Capacity: %u tokens\n", vocab->capacity);
    printf("Usage: %.1f%%\n", (double)vocab->size / vocab->capacity * 100);
    
    printf("\n");
}

uint32_t* cllm_vocab_get_most_common(
    const CLLMVocabulary* vocab,
    uint32_t top_k
) {
    if (!vocab || top_k == 0) return NULL;
    
    if (top_k > vocab->size) top_k = vocab->size;
    
    // Create sorted index array
    uint32_t* indices = malloc(vocab->size * sizeof(uint32_t));
    if (!indices) return NULL;
    
    for (uint32_t i = 0; i < vocab->size; i++) {
        indices[i] = i;
    }
    
    // Bubble sort by frequency (descending)
    for (uint32_t i = 0; i < vocab->size - 1; i++) {
        for (uint32_t j = 0; j < vocab->size - i - 1; j++) {
            if (vocab->frequencies[indices[j]] < vocab->frequencies[indices[j + 1]]) {
                uint32_t temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Return only top_k
    uint32_t* result = malloc(top_k * sizeof(uint32_t));
    if (result) {
        memcpy(result, indices, top_k * sizeof(uint32_t));
    }
    
    free(indices);
    return result;
}

// ============================================================================
// VOCABULARY UTILITIES
// ============================================================================

CLLMVocabulary* cllm_vocab_merge(
    const CLLMVocabulary* vocab1,
    const CLLMVocabulary* vocab2
) {
    if (!vocab1 || !vocab2) return NULL;
    
    CLLMVocabulary* merged = cllm_vocab_create(vocab1->size + vocab2->size);
    if (!merged) return NULL;
    
    // Add all tokens from vocab1
    for (uint32_t i = 0; i < vocab1->size; i++) {
        for (uint32_t j = 0; j < vocab1->frequencies[i]; j++) {
            cllm_vocab_add_token(merged, vocab1->tokens[i]);
        }
    }
    
    // Add all tokens from vocab2
    for (uint32_t i = 0; i < vocab2->size; i++) {
        for (uint32_t j = 0; j < vocab2->frequencies[i]; j++) {
            cllm_vocab_add_token(merged, vocab2->tokens[i]);
        }
    }
    
    return merged;
}

CLLMVocabulary* cllm_vocab_filter_by_frequency(
    const CLLMVocabulary* vocab,
    uint32_t min_frequency
) {
    if (!vocab) return NULL;
    
    // Count tokens that meet threshold
    uint32_t count = 0;
    for (uint32_t i = 0; i < vocab->size; i++) {
        if (vocab->frequencies[i] >= min_frequency) {
            count++;
        }
    }
    
    CLLMVocabulary* filtered = cllm_vocab_create(count);
    if (!filtered) return NULL;
    
    // Add tokens that meet threshold
    for (uint32_t i = 0; i < vocab->size; i++) {
        if (vocab->frequencies[i] >= min_frequency) {
            for (uint32_t j = 0; j < vocab->frequencies[i]; j++) {
                cllm_vocab_add_token(filtered, vocab->tokens[i]);
            }
        }
    }
    
    return filtered;
}

bool cllm_vocab_resize(CLLMVocabulary* vocab, uint32_t new_capacity) {
    if (!vocab || new_capacity < vocab->size) return false;
    
    char** new_tokens = realloc(vocab->tokens, new_capacity * sizeof(char*));
    uint32_t* new_frequencies = realloc(vocab->frequencies, new_capacity * sizeof(uint32_t));
    
    if (!new_tokens || !new_frequencies) {
        return false;
    }
    
    vocab->tokens = new_tokens;
    vocab->frequencies = new_frequencies;
    vocab->capacity = new_capacity;
    
    // Zero out new space
    for (uint32_t i = vocab->size; i < new_capacity; i++) {
        vocab->tokens[i] = NULL;
        vocab->frequencies[i] = 0;
    }
    
    return true;
}