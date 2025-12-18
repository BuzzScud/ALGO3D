#ifndef CLLM_DATA_LOADER_H
#define CLLM_DATA_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "cllm_tokenizer.h"

/**
 * Data Loader Structure
 */
typedef struct {
    CLLMTokenizer* tokenizer;
    char** documents;
    size_t num_documents;
    size_t capacity;
    
    // Statistics
    size_t total_chars;
    size_t total_tokens;
    size_t total_lines;
    
    // Configuration
    int min_token_length;
    int max_token_length;
    int lowercase;
    int remove_punctuation;
    int remove_numbers;
} CLLMDataLoader;

/**
 * Token Dataset Structure
 */
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    size_t capacity;
} TokenDataset;

/**
 * Create Data Loader
 */
CLLMDataLoader* cllm_data_loader_create(CLLMTokenizer* tokenizer);

/**
 * Free Data Loader
 */
void cllm_data_loader_free(CLLMDataLoader* loader);

/**
 * Add Document
 */
int cllm_data_loader_add_document(CLLMDataLoader* loader, const char* text);

/**
 * Load Text File
 */
int cllm_data_loader_load_file(CLLMDataLoader* loader, const char* filename);

/**
 * Load Directory
 */
int cllm_data_loader_load_directory(CLLMDataLoader* loader, const char* dirname);

/**
 * Build Vocabulary
 */
void cllm_data_loader_build_vocab(CLLMDataLoader* loader);

/**
 * Create Training Dataset
 */
TokenDataset* cllm_data_loader_create_dataset(CLLMDataLoader* loader);

/**
 * Free Token Dataset
 */
void cllm_token_dataset_free(TokenDataset* dataset);

/**
 * Save Dataset
 */
int cllm_token_dataset_save(TokenDataset* dataset, const char* filename);

/**
 * Load Dataset
 */
TokenDataset* cllm_token_dataset_load(const char* filename);

/**
 * Print Statistics
 */
void cllm_data_loader_print_stats(CLLMDataLoader* loader);

#endif // CLLM_DATA_LOADER_H