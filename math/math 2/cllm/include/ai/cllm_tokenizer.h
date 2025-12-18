#ifndef CLLM_TOKENIZER_H
#define CLLM_TOKENIZER_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

/**
 * Tokenizer Structure with 12-Fold Symmetry Thread Safety
 * 
 * MASTER PLAN COMPLIANCE:
 * - 12 vocabulary partitions (one per symmetry group)
 * - Lock-free token lookup using atomic operations
 * - Thread-safe token addition with per-partition locks
 * - Kissing spheres architecture for parallel vocabulary building
 */
typedef struct {
    // Legacy single vocabulary (for backward compatibility)
    char** vocab;
    uint32_t* token_counts;
    uint32_t vocab_size;
    uint32_t max_vocab_size;
    
    // 12-fold symmetry partitions for thread-safe access
    char** vocab_partitions[12];           // 12 vocabulary partitions
    uint32_t* count_partitions[12];        // 12 count arrays
    uint32_t partition_sizes[12];          // Size of each partition (use atomic ops)
    uint32_t partition_capacities[12];     // Capacity of each partition
    pthread_mutex_t partition_locks[12];   // One lock per partition
    
    // Global consolidated vocabulary (built after parallel phase)
    int consolidated;                      // Flag: has vocab been consolidated?
    
    // Persistent hash table for O(1) token lookup (built during consolidation)
    void* hash_table;                      // TokenHashEntry* (opaque to avoid exposing internals)
    uint32_t hash_table_size;              // Size of hash table
} CLLMTokenizer;

/**
 * Create Tokenizer
 */
CLLMTokenizer* cllm_create_tokenizer(uint32_t max_vocab_size);

/**
 * Free Tokenizer
 */
void cllm_free_tokenizer(CLLMTokenizer* tokenizer);

/**
 * Find Token in Vocabulary (Linear Search - O(n))
 * Use cllm_find_token_fast() for O(1) lookups after consolidation
 */
uint32_t cllm_find_token(CLLMTokenizer* tokenizer, const char* token);

/**
 * Fast Token Lookup Using Hash Table (O(1) average case)
 * Thread-safe read-only operation after vocabulary consolidation.
 * Falls back to linear search if vocabulary not yet consolidated.
 */
uint32_t cllm_find_token_fast(CLLMTokenizer* tokenizer, const char* token);

/**
 * Add Token to Vocabulary (Thread-Safe with 12-Fold Symmetry)
 * 
 * Uses hash-based partitioning to distribute tokens across 12 partitions.
 * Each thread can add tokens to different partitions concurrently.
 */
uint32_t cllm_add_token(CLLMTokenizer* tokenizer, const char* token);

/**
 * Add Token to Specific Partition (Thread-Safe)
 * 
 * Directly adds token to specified partition (0-11).
 * Used by kissing spheres workers to minimize lock contention.
 */
uint32_t cllm_add_token_to_partition(CLLMTokenizer* tokenizer, const char* token, int partition_id);

/**
 * Consolidate Partitioned Vocabulary into Single Vocabulary
 * 
 * Merges all 12 partitions into the main vocabulary.
 * Call this after parallel vocabulary building is complete.
 */
void cllm_consolidate_vocabulary(CLLMTokenizer* tokenizer);

/**
 * Encode Text to Token IDs
 */
uint32_t* cllm_tokenizer_encode(CLLMTokenizer* tokenizer, const char* text, uint32_t* num_tokens);

/**
 * Decode Token IDs to Text
 */
char* cllm_tokenizer_decode(CLLMTokenizer* tokenizer, uint32_t* tokens, uint32_t num_tokens);

/**
 * Build Vocabulary from Text
 */
void cllm_build_vocab(CLLMTokenizer* tokenizer, const char* text);

/**
 * Save Vocabulary to File
 */
int cllm_save_vocab(CLLMTokenizer* tokenizer, const char* filename);

/**
 * Load Vocabulary from File
 */
int cllm_load_vocab(CLLMTokenizer* tokenizer, const char* filename);

/**
 * Get Vocabulary Size
 */
uint32_t cllm_get_vocab_size(CLLMTokenizer* tokenizer);

/**
 * Get Token String
 */
const char* cllm_get_token_string(CLLMTokenizer* tokenizer, uint32_t token_id);

/**
 * Print Vocabulary Statistics
 */
void cllm_print_vocab_stats(CLLMTokenizer* tokenizer);

/**
 * Encode with Special Tokens
 */
uint32_t* cllm_encode_with_special(CLLMTokenizer* tokenizer, const char* text, uint32_t* num_tokens);

#endif // CLLM_TOKENIZER_H