/**
 * CLLM Tokenizer
 * 
 * Basic tokenization utilities for the CLLM.
 * Provides:
 * - Whitespace tokenization
 * - Vocabulary building
 * - Token encoding/decoding
 * - Special tokens handling
 * 
 * Note: This is a basic implementation. For production use,
 * consider more sophisticated tokenizers like BPE or WordPiece.
 */

#include "../include/cllm.h"
#include "../include/cllm_tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// External fast lookup function from cllm_tokenizer_threadsafe.c
extern uint32_t cllm_find_token_fast(CLLMTokenizer* tokenizer, const char* token);

// Special token IDs
#define TOKEN_PAD 0
#define TOKEN_UNK 1
#define TOKEN_BOS 2
#define TOKEN_EOS 3
#define TOKEN_MASK 4

/**
 * Tokenizer Structure
 */
// CLLMTokenizer definition moved to header to resolve forward declaration issues

/**
 * Create Tokenizer
 * 
 * Initializes a new tokenizer with special tokens
 */
CLLMTokenizer* cllm_create_tokenizer(uint32_t max_vocab_size) {
    CLLMTokenizer* tokenizer = (CLLMTokenizer*)malloc(sizeof(CLLMTokenizer));
    if (!tokenizer) return NULL;
    
    tokenizer->max_vocab_size = max_vocab_size;
    tokenizer->vocab_size = 0;
    
    // Allocate vocabulary
    tokenizer->vocab = (char**)calloc(max_vocab_size, sizeof(char*));
    tokenizer->token_counts = (uint32_t*)calloc(max_vocab_size, sizeof(uint32_t));
    
    if (!tokenizer->vocab || !tokenizer->token_counts) {
        cllm_free_tokenizer(tokenizer);
        return NULL;
    }
    
    // 12-fold symmetry partitions for thread-safe parallel vocabulary building
    uint32_t partition_capacity = max_vocab_size / 12 + 10000;  // Extra space per partition
    
    printf("Initializing 12-fold symmetry tokenizer:\n");
    printf("  Max vocab size: %u\n", max_vocab_size);
    printf("  Partition capacity: %u tokens each\n", partition_capacity);
    printf("  Total capacity: %u tokens across 12 partitions\n", partition_capacity * 12);
    
    for (int i = 0; i < 12; i++) {
        tokenizer->vocab_partitions[i] = (char**)calloc(partition_capacity, sizeof(char*));
        tokenizer->count_partitions[i] = (uint32_t*)calloc(partition_capacity, sizeof(uint32_t));
        
        if (!tokenizer->vocab_partitions[i] || !tokenizer->count_partitions[i]) {
            fprintf(stderr, "ERROR: Failed to allocate partition %d (capacity=%u)\n", i, partition_capacity);
            // Cleanup on failure
            for (int j = 0; j <= i; j++) {
                free(tokenizer->vocab_partitions[j]);
                free(tokenizer->count_partitions[j]);
            }
            cllm_free_tokenizer(tokenizer);
            return NULL;
        }
        
        tokenizer->partition_sizes[i] = 0;
        tokenizer->partition_capacities[i] = partition_capacity;
        pthread_mutex_init(&tokenizer->partition_locks[i], NULL);
    }
    
    printf("  Successfully allocated all 12 partitions\n");
    
    tokenizer->consolidated = 0;
    tokenizer->hash_table = NULL;
    tokenizer->hash_table_size = 0;
    
    // Add special tokens
    tokenizer->vocab[TOKEN_PAD] = strdup("<PAD>");
    tokenizer->vocab[TOKEN_UNK] = strdup("<UNK>");
    tokenizer->vocab[TOKEN_BOS] = strdup("<BOS>");
    tokenizer->vocab[TOKEN_EOS] = strdup("<EOS>");
    tokenizer->vocab[TOKEN_MASK] = strdup("<MASK>");
    tokenizer->vocab_size = 5;
    
    return tokenizer;
}

/**
 * Free Tokenizer
 */
void cllm_free_tokenizer(CLLMTokenizer* tokenizer) {
    if (!tokenizer) return;
    
    if (tokenizer->vocab) {
        for (uint32_t i = 0; i < tokenizer->vocab_size; i++) {
            if (tokenizer->vocab[i]) {
                free(tokenizer->vocab[i]);
            }
        }
        free(tokenizer->vocab);
    }
    
    if (tokenizer->token_counts) {
        free(tokenizer->token_counts);
    }
    
    // Free 12-fold symmetry partitions
    for (int i = 0; i < 12; i++) {
        if (tokenizer->vocab_partitions[i]) {
            for (uint32_t j = 0; j < tokenizer->partition_sizes[i]; j++) {
                if (tokenizer->vocab_partitions[i][j]) {
                    free(tokenizer->vocab_partitions[i][j]);
                }
            }
            free(tokenizer->vocab_partitions[i]);
        }
        if (tokenizer->count_partitions[i]) {
            free(tokenizer->count_partitions[i]);
        }
        pthread_mutex_destroy(&tokenizer->partition_locks[i]);
    }
    // Free persistent hash table (if exists)
    if (tokenizer->hash_table) {
        free(tokenizer->hash_table);
    }

    
    free(tokenizer);
}
    

/**
 * Find Token in Vocabulary
 * 
 * Returns token ID if found, TOKEN_UNK otherwise
 */
uint32_t cllm_find_token(CLLMTokenizer* tokenizer, const char* token) {
    if (!tokenizer || !token) return TOKEN_UNK;
    
    for (uint32_t i = 0; i < tokenizer->vocab_size; i++) {
        if (tokenizer->vocab[i] && strcmp(tokenizer->vocab[i], token) == 0) {
            return i;
        }
    }
    
    return TOKEN_UNK;
}

/**
 * Add Token to Vocabulary
 * 
 * Returns token ID (new or existing)
 */
uint32_t cllm_add_token(CLLMTokenizer* tokenizer, const char* token) {
    if (!tokenizer || !token) return TOKEN_UNK;
    
    // Check if token already exists
    uint32_t existing = cllm_find_token(tokenizer, token);
    if (existing != TOKEN_UNK) {
        tokenizer->token_counts[existing]++;
        return existing;
    }
    
    // Check if vocabulary is full
    if (tokenizer->vocab_size >= tokenizer->max_vocab_size) {
        return TOKEN_UNK;
    }
    
    // Add new token
    tokenizer->vocab[tokenizer->vocab_size] = strdup(token);
    tokenizer->token_counts[tokenizer->vocab_size] = 1;
    
    return tokenizer->vocab_size++;
}

/**
 * Tokenize Text (Whitespace-based)
 * 
 * Simple whitespace tokenization
 * Returns array of token IDs and sets num_tokens
 */
uint32_t* cllm_tokenizer_encode(CLLMTokenizer* tokenizer, const char* text, uint32_t* num_tokens) {
    if (!tokenizer || !text || !num_tokens) {
        if (num_tokens) *num_tokens = 0;
        return NULL;
    }
    
    // Count tokens (rough estimate)
    uint32_t max_tokens = 1;
    for (const char* p = text; *p; p++) {
        if (isspace(*p)) max_tokens++;
    }
    
    uint32_t* tokens = (uint32_t*)malloc(max_tokens * sizeof(uint32_t));
    if (!tokens) {
        *num_tokens = 0;
        return NULL;
    }
    
    // Tokenize
    char* text_copy = strdup(text);
    if (!text_copy) {
        free(tokens);
        *num_tokens = 0;
        return NULL;
    }
    
    uint32_t count = 0;
    char* saveptr = NULL;  // For strtok_r thread safety
    char* token = strtok_r(text_copy, " \t\n\r", &saveptr);
    
    while (token && count < max_tokens) {
        // Convert to lowercase
        for (char* p = token; *p; p++) {
            *p = tolower(*p);
        }
        
        // Find token using fast O(1) hash lookup (falls back to linear if not consolidated)
        uint32_t token_id = cllm_find_token_fast(tokenizer, token);
        tokens[count++] = token_id;
        
        token = strtok_r(NULL, " \t\n\r", &saveptr);
    }
    
    free(text_copy);
    *num_tokens = count;
    
    return tokens;
}

/**
 * Detokenize (Convert token IDs back to text)
 * 
 * Returns allocated string (caller must free)
 */
char* cllm_tokenizer_decode(CLLMTokenizer* tokenizer, uint32_t* tokens, uint32_t num_tokens) {
    if (!tokenizer || !tokens || num_tokens == 0) {
        return strdup("");
    }
    
    // Estimate size
    size_t total_size = num_tokens * 20;  // Rough estimate
    char* result = (char*)malloc(total_size);
    if (!result) return NULL;
    
    result[0] = '\0';
    size_t pos = 0;
    
    for (uint32_t i = 0; i < num_tokens; i++) {
        uint32_t token_id = tokens[i];
        
        // Skip special tokens
        if (token_id == TOKEN_PAD || token_id == TOKEN_BOS || 
            token_id == TOKEN_EOS || token_id == TOKEN_MASK) {
            continue;
        }
        
        if (token_id >= tokenizer->vocab_size) {
            token_id = TOKEN_UNK;
        }
        
        const char* token_str = tokenizer->vocab[token_id];
        size_t token_len = strlen(token_str);
        
        // Check if we need to resize
        if (pos + token_len + 2 > total_size) {
            total_size *= 2;
            char* new_result = (char*)realloc(result, total_size);
            if (!new_result) {
                free(result);
                return NULL;
            }
            result = new_result;
        }
        
        // Add space if not first token
        if (i > 0) {
            result[pos++] = ' ';
        }
        
        // Add token
        strcpy(result + pos, token_str);
        pos += token_len;
    }
    
    result[pos] = '\0';
    return result;
}

/**
 * Build Vocabulary from Text
 * 
 * Scans text and builds vocabulary
 */
void cllm_build_vocab(CLLMTokenizer* tokenizer, const char* text) {
    if (!tokenizer || !text) return;
    
    char* text_copy = strdup(text);
    if (!text_copy) return;
    
    char* token = strtok(text_copy, " \t\n\r");
    
    while (token) {
        // Convert to lowercase
        for (char* p = token; *p; p++) {
            *p = tolower(*p);
        }
        
        // Add to vocabulary
        cllm_add_token(tokenizer, token);
        
        token = strtok(NULL, " \t\n\r");
    }
    
    free(text_copy);
}

/**
 * Save Vocabulary to File
 * 
 * Saves vocabulary in simple text format (one token per line)
 */
int cllm_save_vocab(CLLMTokenizer* tokenizer, const char* filename) {
    if (!tokenizer || !filename) return 0;
    
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    
    for (uint32_t i = 0; i < tokenizer->vocab_size; i++) {
        if (tokenizer->vocab[i]) {
            fprintf(f, "%s\t%u\n", tokenizer->vocab[i], tokenizer->token_counts[i]);
        }
    }
    
    fclose(f);
    return 1;
}

/**
 * Load Vocabulary from File
 * 
 * Loads vocabulary from text file
 */
int cllm_load_vocab(CLLMTokenizer* tokenizer, const char* filename) {
    if (!tokenizer || !filename) return 0;
    
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // Parse line: token\tcount
        char* tab = strchr(line, '\t');
        if (tab) {
            *tab = '\0';
            uint32_t count = atoi(tab + 1);
            
            // Add token
            uint32_t token_id = cllm_add_token(tokenizer, line);
            if (token_id != TOKEN_UNK) {
                tokenizer->token_counts[token_id] = count;
            }
        }
    }
    
    fclose(f);
    return 1;
}

/**
 * Get Vocabulary Size
 */
uint32_t cllm_get_vocab_size(CLLMTokenizer* tokenizer) {
    return tokenizer ? tokenizer->vocab_size : 0;
}

/**
 * Get Token String
 * 
 * Returns token string for given ID
 */
const char* cllm_get_token_string(CLLMTokenizer* tokenizer, uint32_t token_id) {
    if (!tokenizer || token_id >= tokenizer->vocab_size) {
        return "<UNK>";
    }
    
    return tokenizer->vocab[token_id];
}

/**
 * Print Vocabulary Statistics
 */
void cllm_print_vocab_stats(CLLMTokenizer* tokenizer) {
    if (!tokenizer) return;
    
    printf("Vocabulary Statistics:\n");
    printf("  Size: %u / %u\n", tokenizer->vocab_size, tokenizer->max_vocab_size);
    
    // Find most common tokens
    uint32_t max_count = 0;
    uint32_t total_count = 0;
    
    for (uint32_t i = 0; i < tokenizer->vocab_size; i++) {
        total_count += tokenizer->token_counts[i];
        if (tokenizer->token_counts[i] > max_count) {
            max_count = tokenizer->token_counts[i];
        }
    }
    
    printf("  Total tokens seen: %u\n", total_count);
    printf("  Most frequent count: %u\n", max_count);
    
    // Print top 10 tokens
    printf("\n  Top 10 tokens:\n");
    for (int rank = 0; rank < 10 && rank < (int)tokenizer->vocab_size; rank++) {
        uint32_t best_id = 0;
        uint32_t best_count = 0;
        
        for (uint32_t i = 5; i < tokenizer->vocab_size; i++) {  // Skip special tokens
            int already_printed = 0;
            for (int r = 0; r < rank; r++) {
                // Check if already printed (simplified)
            }
            
            if (!already_printed && tokenizer->token_counts[i] > best_count) {
                best_id = i;
                best_count = tokenizer->token_counts[i];
            }
        }
        
        if (best_count > 0) {
            printf("    %2d. %-20s %u\n", rank + 1, 
                   tokenizer->vocab[best_id], best_count);
        }
    }
}

/**
 * Encode Text to Token IDs with Special Tokens
 * 
 * Adds BOS and EOS tokens
 */
uint32_t* cllm_encode_with_special(CLLMTokenizer* tokenizer, 
                                    const char* text, 
                                    uint32_t* num_tokens) {
    if (!tokenizer || !text || !num_tokens) {
        if (num_tokens) *num_tokens = 0;
        return NULL;
    }
    
    // Tokenize text
    uint32_t text_tokens_count;
    uint32_t* text_tokens = cllm_tokenizer_encode(tokenizer, text, &text_tokens_count);
    
    if (!text_tokens) {
        *num_tokens = 0;
        return NULL;
    }
    
    // Allocate with space for BOS and EOS
    uint32_t* tokens = (uint32_t*)malloc((text_tokens_count + 2) * sizeof(uint32_t));
    if (!tokens) {
        free(text_tokens);
        *num_tokens = 0;
        return NULL;
    }
    
    // Add BOS
    tokens[0] = TOKEN_BOS;
    
    // Copy text tokens
    memcpy(tokens + 1, text_tokens, text_tokens_count * sizeof(uint32_t));
    
    // Add EOS
    tokens[text_tokens_count + 1] = TOKEN_EOS;
    
    free(text_tokens);
    *num_tokens = text_tokens_count + 2;
    
    return tokens;
}
