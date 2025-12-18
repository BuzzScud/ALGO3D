#ifndef CLLM_VOCABULARY_H
#define CLLM_VOCABULARY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// VOCABULARY STRUCTURE
// ============================================================================

/**
 * Vocabulary for CLLM models
 * Maps tokens (strings) to IDs (integers) and vice versa
 */
typedef struct {
    char** tokens;              // Array of token strings
    uint32_t* frequencies;      // Frequency count for each token
    uint32_t size;              // Current number of tokens
    uint32_t capacity;          // Maximum capacity
    
    // Special tokens
    uint32_t pad_token_id;      // Padding token ID
    uint32_t unk_token_id;      // Unknown token ID
    uint32_t bos_token_id;      // Beginning of sequence token ID
    uint32_t eos_token_id;      // End of sequence token ID
    
    // Metadata
    char* name;                 // Vocabulary name
    uint64_t total_tokens;      // Total tokens processed
} CLLMVocabulary;

// ============================================================================
// VOCABULARY CREATION & DESTRUCTION
// ============================================================================

/**
 * Create a new vocabulary with specified capacity
 */
CLLMVocabulary* cllm_vocab_create(uint32_t capacity);

/**
 * Create vocabulary with special tokens
 */
CLLMVocabulary* cllm_vocab_create_with_special_tokens(
    uint32_t capacity,
    const char* pad_token,
    const char* unk_token,
    const char* bos_token,
    const char* eos_token
);

/**
 * Destroy vocabulary and free all memory
 */
void cllm_vocab_destroy(CLLMVocabulary* vocab);

// ============================================================================
// TOKEN MANAGEMENT
// ============================================================================

/**
 * Add a token to the vocabulary
 * Returns the token ID (existing or new)
 */
uint32_t cllm_vocab_add_token(CLLMVocabulary* vocab, const char* token);

/**
 * Get token ID for a string
 * Returns unk_token_id if not found
 */
uint32_t cllm_vocab_get_id(const CLLMVocabulary* vocab, const char* token);

/**
 * Get token string for an ID
 * Returns NULL if ID is invalid
 */
const char* cllm_vocab_get_token(const CLLMVocabulary* vocab, uint32_t id);

/**
 * Check if token exists in vocabulary
 */
bool cllm_vocab_contains(const CLLMVocabulary* vocab, const char* token);

/**
 * Get token frequency
 */
uint32_t cllm_vocab_get_frequency(const CLLMVocabulary* vocab, uint32_t id);

// ============================================================================
// TOKENIZATION
// ============================================================================

/**
 * Tokenize a string into token IDs
 * Returns array of token IDs (caller must free)
 * num_tokens is set to the number of tokens
 */
uint32_t* cllm_vocab_tokenize(
    CLLMVocabulary* vocab,
    const char* text,
    uint32_t* num_tokens
);

/**
 * Detokenize token IDs back to string
 * Returns allocated string (caller must free)
 */
char* cllm_vocab_detokenize(
    const CLLMVocabulary* vocab,
    const uint32_t* token_ids,
    uint32_t num_tokens
);

// ============================================================================
// BATCH TOKENIZATION
// ============================================================================

/**
 * Tokenize multiple strings
 * Returns array of token ID arrays (caller must free)
 */
uint32_t** cllm_vocab_tokenize_batch(
    CLLMVocabulary* vocab,
    const char** texts,
    uint32_t num_texts,
    uint32_t** num_tokens_per_text
);

/**
 * Free batch tokenization results
 */
void cllm_vocab_free_batch(uint32_t** token_arrays, uint32_t num_arrays);

// ============================================================================
// VOCABULARY BUILDING
// ============================================================================

/**
 * Build vocabulary from text file
 * Returns number of tokens added
 */
uint32_t cllm_vocab_build_from_file(
    CLLMVocabulary* vocab,
    const char* filename
);

/**
 * Build vocabulary from multiple files
 */
uint32_t cllm_vocab_build_from_files(
    CLLMVocabulary* vocab,
    const char** filenames,
    uint32_t num_files
);

/**
 * Build vocabulary from text array
 */
uint32_t cllm_vocab_build_from_texts(
    CLLMVocabulary* vocab,
    const char** texts,
    uint32_t num_texts
);

// ============================================================================
// VOCABULARY PERSISTENCE
// ============================================================================

/**
 * Save vocabulary to file
 */
bool cllm_vocab_save(const CLLMVocabulary* vocab, const char* filename);

/**
 * Load vocabulary from file
 */
CLLMVocabulary* cllm_vocab_load(const char* filename);

// ============================================================================
// VOCABULARY STATISTICS
// ============================================================================

/**
 * Get vocabulary statistics
 */
typedef struct {
    uint32_t vocab_size;
    uint32_t total_tokens;
    uint32_t unique_tokens;
    double avg_token_length;
    uint32_t max_token_length;
    uint32_t min_token_length;
} VocabStats;

VocabStats cllm_vocab_get_stats(const CLLMVocabulary* vocab);

/**
 * Print vocabulary statistics
 */
void cllm_vocab_print_stats(const CLLMVocabulary* vocab);

/**
 * Get most common tokens
 * Returns array of token IDs sorted by frequency (caller must free)
 */
uint32_t* cllm_vocab_get_most_common(
    const CLLMVocabulary* vocab,
    uint32_t top_k
);

// ============================================================================
// VOCABULARY UTILITIES
// ============================================================================

/**
 * Merge two vocabularies
 */
CLLMVocabulary* cllm_vocab_merge(
    const CLLMVocabulary* vocab1,
    const CLLMVocabulary* vocab2
);

/**
 * Filter vocabulary by minimum frequency
 */
CLLMVocabulary* cllm_vocab_filter_by_frequency(
    const CLLMVocabulary* vocab,
    uint32_t min_frequency
);

/**
 * Resize vocabulary capacity
 */
bool cllm_vocab_resize(CLLMVocabulary* vocab, uint32_t new_capacity);

#ifdef __cplusplus
}
#endif

#endif // CLLM_VOCABULARY_H