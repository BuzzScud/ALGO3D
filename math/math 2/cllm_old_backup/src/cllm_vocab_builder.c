/**
 * CLLM Vocabulary Builder
 * 
 * Builds vocabulary from training data and integrates with model
 */

#include "../include/cllm.h"
#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Build vocabulary from training file and store in model
 */
int cllm_build_vocabulary_from_file(CLLMModel* model, const char* filename) {
    if (!model || !filename) return -1;
    
    printf("Building vocabulary from: %s\n", filename);
    
    // Read file
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file: %s\n", filename);
        return -1;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Read content
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    // Create tokenizer
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(model->vocab_size);
    if (!tokenizer) {
        free(content);
        return -1;
    }
    
    // Build vocabulary
    cllm_build_vocab(tokenizer, content);
    free(content);
    
    // Get actual vocabulary size
    uint32_t actual_vocab_size = cllm_get_vocab_size(tokenizer);
    printf("  Built vocabulary with %u unique tokens\n", actual_vocab_size);
    
    // Allocate tokens array in model
    if (model->tokens) {
        // Free existing tokens (token_str is a fixed array, no need to free)
        free(model->tokens);
    }
    
    model->tokens = (CLLMToken*)calloc(model->vocab_size, sizeof(CLLMToken));
    if (!model->tokens) {
        cllm_free_tokenizer(tokenizer);
        return -1;
    }
    
    // Copy vocabulary to model
    for (uint32_t i = 0; i < actual_vocab_size && i < model->vocab_size; i++) {
        const char* token_str = cllm_get_token_string(tokenizer, i);
        strncpy(model->tokens[i].token_str, token_str, sizeof(model->tokens[i].token_str) - 1);
        model->tokens[i].token_str[sizeof(model->tokens[i].token_str) - 1] = '\0';
        model->tokens[i].token_id = i;
        model->tokens[i].frequency = 1; // Will be updated during training
    }
    
    // Fill remaining slots with special tokens
    for (uint32_t i = actual_vocab_size; i < model->vocab_size; i++) {
        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "<UNK_%u>", i);
        model->tokens[i].token_id = i;
        model->tokens[i].frequency = 0;
    }
    
    cllm_free_tokenizer(tokenizer);
    
    printf("  Vocabulary integrated into model successfully\n");
    return actual_vocab_size;
}

/**
 * Tokenize text using model's vocabulary
 */
int cllm_tokenize_with_vocab(CLLMModel* model, const char* text, uint32_t* tokens, int max_tokens) {
    if (!model || !text || !tokens || !model->tokens) return 0;
    
    int token_count = 0;
    char buffer[256];
    int buf_pos = 0;
    
    for (int i = 0; text[i] && token_count < max_tokens; i++) {
        char c = text[i];
        
        // Check for word boundaries
        if (c == ' ' || c == '\n' || c == '\t' || c == '.' || c == ',' || c == '!' || c == '?') {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                
                // Convert to lowercase
                for (int j = 0; j < buf_pos; j++) {
                    buffer[j] = tolower(buffer[j]);
                }
                
                // Find token in vocabulary
                bool found = false;
                for (uint32_t j = 0; j < model->vocab_size; j++) {
                    if (strcmp(model->tokens[j].token_str, buffer) == 0) {
                        tokens[token_count++] = j;
                        found = true;
                        break;
                    }
                }
                
                // If not found, use hash-based mapping
                if (!found) {
                    uint32_t hash = 0;
                    for (int j = 0; buffer[j]; j++) {
                        hash = hash * 31 + (uint32_t)buffer[j];
                    }
                    tokens[token_count++] = hash % model->vocab_size;
                }
                
                buf_pos = 0;
            }
        } else {
            if (buf_pos < 255) {
                buffer[buf_pos++] = c;
            }
        }
    }
    
    // Handle last token
    if (buf_pos > 0 && token_count < max_tokens) {
        buffer[buf_pos] = '\0';
        
        // Convert to lowercase
        for (int j = 0; j < buf_pos; j++) {
            buffer[j] = tolower(buffer[j]);
        }
        
        // Find token in vocabulary
        bool found = false;
        for (uint32_t j = 0; j < model->vocab_size; j++) {
            if (strcmp(model->tokens[j].token_str, buffer) == 0) {
                tokens[token_count++] = j;
                found = true;
                break;
            }
        }
        
        // If not found, use hash-based mapping
        if (!found) {
            uint32_t hash = 0;
            for (int j = 0; buffer[j]; j++) {
                hash = hash * 31 + (uint32_t)buffer[j];
            }
            tokens[token_count++] = hash % model->vocab_size;
        }
    }
    
    return token_count;
}

/**
 * Detokenize tokens using model's vocabulary
 */
void cllm_detokenize_with_vocab(CLLMModel* model, uint32_t* tokens, int num_tokens, 
                                 char* output, int max_length) {
    if (!model || !tokens || !output || !model->tokens) return;
    
    int pos = 0;
    for (int i = 0; i < num_tokens && pos < max_length - 1; i++) {
        if (tokens[i] < model->vocab_size) {
            const char* token_str = model->tokens[tokens[i]].token_str;
            int len = strlen(token_str);
            
            if (pos + len + 1 < max_length) {
                strcpy(&output[pos], token_str);
                pos += len;
                
                // Add space between tokens
                if (i < num_tokens - 1) {
                    output[pos++] = ' ';
                }
            }
        }
    }
    
    output[pos] = '\0';
}
