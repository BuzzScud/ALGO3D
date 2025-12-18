/**
 * Thread-Safe Tokenizer with 12-Fold Symmetry
 * 
 * MASTER PLAN COMPLIANCE:
 * - 12 vocabulary partitions (one per symmetry group)
 * - Lock-free token lookup using hash-based partitioning
 * - Thread-safe token addition with per-partition locks
 * - Kissing spheres architecture for parallel vocabulary building
 */

#include "ai/cllm_tokenizer.h"
#include "ai/cllm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

// Special token IDs (must match cllm_tokenizer.c)
#define TOKEN_PAD 0
#define TOKEN_UNK 1
#define TOKEN_BOS 2
#define TOKEN_EOS 3
#define TOKEN_MASK 4

/**
 * Hash function to determine partition (0-11) for a token
 * Uses simple string hash modulo 12 for even distribution
 */
static int token_hash_to_partition(const char* token) {
    unsigned long hash = 5381;
    int c;
    
    while ((c = *token++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % 12;
}

/**
 * Add token to specific partition (thread-safe)
 * Each partition has its own lock, allowing 12 concurrent additions
 */
uint32_t cllm_add_token_to_partition(CLLMTokenizer* tokenizer, const char* token, int partition_id) {
    if (!tokenizer || !token || partition_id < 0 || partition_id >= 12) {
        return TOKEN_UNK;
    }
    
    pthread_mutex_lock(&tokenizer->partition_locks[partition_id]);
    
    // Search within this partition
    for (uint32_t i = 0; i < tokenizer->partition_sizes[partition_id]; i++) {
        if (tokenizer->vocab_partitions[partition_id][i] && 
            strcmp(tokenizer->vocab_partitions[partition_id][i], token) == 0) {
            // Token exists - increment count
            tokenizer->count_partitions[partition_id][i]++;
            pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
            return i;  // Return local partition index
        }
    }
    
    // Token doesn't exist - add it
    if (tokenizer->partition_sizes[partition_id] >= tokenizer->partition_capacities[partition_id]) {
        // Partition full
        pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
        return TOKEN_UNK;
    }
    
    uint32_t idx = tokenizer->partition_sizes[partition_id];
    
    // Verify we have space
    if (idx >= tokenizer->partition_capacities[partition_id]) {
        fprintf(stderr, "[ERROR] Partition %d overflow: idx=%u, capacity=%u\n",
                partition_id, idx, tokenizer->partition_capacities[partition_id]);
        pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
        return TOKEN_UNK;
    }
    
    char* token_copy = strdup(token);
    
    if (!token_copy) {
        // strdup failed - out of memory
        fprintf(stderr, "[ERROR] strdup failed for token in partition %d\n", partition_id);
        pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
        return TOKEN_UNK;
    }
    
    tokenizer->vocab_partitions[partition_id][idx] = token_copy;
    tokenizer->count_partitions[partition_id][idx] = 1;
    tokenizer->partition_sizes[partition_id]++;
    
    pthread_mutex_unlock(&tokenizer->partition_locks[partition_id]);
    
    return idx;
}

/**
 * Add token using hash-based partitioning (thread-safe)
 * Automatically determines partition based on token hash
 */
uint32_t cllm_add_token_threadsafe(CLLMTokenizer* tokenizer, const char* token) {
    if (!tokenizer || !token) return TOKEN_UNK;
    
    int partition_id = token_hash_to_partition(token);
    return cllm_add_token_to_partition(tokenizer, token, partition_id);
}

/**
 * Consolidate all partitions into single vocabulary
 * Call this after parallel vocabulary building is complete
 */
// Simple hash table for O(1) token lookup during consolidation
typedef struct {
    const char* token;
    uint32_t vocab_idx;
} TokenHashEntry;

// Aligned with mathematical framework's vector culmination: 3 × 12³ × (250/9) = 144,000
// Using 2^17 = 131,072 (closest power of 2) for efficient modulo operations
#define HASH_TABLE_SIZE 131072  // 2^17, aligned with 144,000 vector culmination point

static uint32_t hash_token_for_lookup(const char* token) {
    uint32_t hash = 5381;
    int c;
    while ((c = *token++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

void cllm_consolidate_vocabulary(CLLMTokenizer* tokenizer) {
    if (!tokenizer || tokenizer->consolidated) return;
    
    printf("Consolidating 12 vocabulary partitions...\n");
    
    // Count total tokens across all partitions
    uint32_t total_tokens = 0;
    for (int i = 0; i < 12; i++) {
        total_tokens += tokenizer->partition_sizes[i];
        printf("  Partition %d: %u tokens\n", i, tokenizer->partition_sizes[i]);
    }
    
    printf("Total tokens across partitions: %u\n", total_tokens);
    
    // Create hash table for O(1) lookups
    TokenHashEntry* hash_table = (TokenHashEntry*)calloc(HASH_TABLE_SIZE, sizeof(TokenHashEntry));
    if (!hash_table) {
        fprintf(stderr, "Failed to allocate hash table for consolidation\n");
        return;
    }
    
    // Initialize hash table with special tokens
    for (uint32_t i = 0; i < 5 && i < tokenizer->vocab_size; i++) {
        if (tokenizer->vocab[i]) {
            uint32_t hash = hash_token_for_lookup(tokenizer->vocab[i]);
            // Linear probing for collisions
            while (hash_table[hash].token != NULL) {
                hash = (hash + 1) % HASH_TABLE_SIZE;
            }
            hash_table[hash].token = tokenizer->vocab[i];
            hash_table[hash].vocab_idx = i;
        }
    }
    
    // Reset main vocabulary (keep special tokens)
    tokenizer->vocab_size = 5;
    
    // Merge all partitions into main vocabulary using hash table
    uint32_t collisions = 0;
    for (int partition = 0; partition < 12; partition++) {
        for (uint32_t i = 0; i < tokenizer->partition_sizes[partition]; i++) {
            const char* token = tokenizer->vocab_partitions[partition][i];
            uint32_t count = tokenizer->count_partitions[partition][i];
            
            if (!token) continue;
            
            // Hash lookup - O(1) average case
            uint32_t hash = hash_token_for_lookup(token);
            uint32_t original_hash = hash;
            uint32_t existing_idx = TOKEN_UNK;
            
            // Linear probing to find token or empty slot
            while (hash_table[hash].token != NULL) {
                if (strcmp(hash_table[hash].token, token) == 0) {
                    existing_idx = hash_table[hash].vocab_idx;
                    break;
                }
                hash = (hash + 1) % HASH_TABLE_SIZE;
                if (hash == original_hash) break;  // Table full
            }
            
            if (existing_idx != TOKEN_UNK) {
                // Token exists - add counts
                tokenizer->token_counts[existing_idx] += count;
            } else {
                // New token - add to main vocabulary
                if (tokenizer->vocab_size < tokenizer->max_vocab_size) {
                    tokenizer->vocab[tokenizer->vocab_size] = strdup(token);
                    if (!tokenizer->vocab[tokenizer->vocab_size]) {
                        fprintf(stderr, "Failed to allocate token during consolidation\n");
                        continue;
                    }
                    tokenizer->token_counts[tokenizer->vocab_size] = count;
                    
                    // Add to hash table
                    hash = hash_token_for_lookup(token);
                    while (hash_table[hash].token != NULL) {
                        hash = (hash + 1) % HASH_TABLE_SIZE;
                        collisions++;
                    }
                    hash_table[hash].token = tokenizer->vocab[tokenizer->vocab_size];
                    hash_table[hash].vocab_idx = tokenizer->vocab_size;
                    
                    tokenizer->vocab_size++;
                }
            }
        }
    }
    
    // CRITICAL: Keep hash table for O(1) lookups during tokenization
    // DO NOT free(hash_table) - store it in tokenizer for future use
    tokenizer->hash_table = (void*)hash_table;
    tokenizer->hash_table_size = HASH_TABLE_SIZE;
    tokenizer->consolidated = 1;
    
    printf("Consolidated vocabulary: %u unique tokens (hash collisions: %u)\n", 
           tokenizer->vocab_size, collisions);
    printf("Hash table retained for O(1) token lookups during tokenization\n");
}
/**
 * Fast Token Lookup Using Hash Table (O(1) average case)
 * 
 * Thread-safe read-only operation after consolidation.
 * Uses the persistent hash table built during consolidation.
 */
uint32_t cllm_find_token_fast(CLLMTokenizer* tokenizer, const char* token) {
    if (!tokenizer || !token) return TOKEN_UNK;
    
    // If not consolidated yet, fall back to linear search
    if (!tokenizer->consolidated || !tokenizer->hash_table) {
        return cllm_find_token(tokenizer, token);
    }
    
    TokenHashEntry* hash_table = (TokenHashEntry*)tokenizer->hash_table;
    uint32_t hash = hash_token_for_lookup(token);
    uint32_t original_hash = hash;
    
    // Linear probing to find token
    while (hash_table[hash].token != NULL) {
        if (strcmp(hash_table[hash].token, token) == 0) {
            return hash_table[hash].vocab_idx;
        }
        hash = (hash + 1) % tokenizer->hash_table_size;
        if (hash == original_hash) break;  // Wrapped around, not found
    }
    
    return TOKEN_UNK;
}
