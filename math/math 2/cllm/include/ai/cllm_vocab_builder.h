#ifndef CLLM_VOCAB_BUILDER_H
#define CLLM_VOCAB_BUILDER_H

#include "cllm.h"

/**
 * Build vocabulary from training file and store in model
 * Returns number of unique tokens found, or -1 on error
 */
int cllm_build_vocabulary_from_file(CLLMModel* model, const char* filename);

/**
 * Tokenize text using model's vocabulary
 * Returns number of tokens generated
 */
int cllm_tokenize_with_vocab(CLLMModel* model, const char* text, uint32_t* tokens, int max_tokens);

/**
 * Detokenize tokens using model's vocabulary
 */
void cllm_detokenize_with_vocab(CLLMModel* model, uint32_t* tokens, int num_tokens, 
                                 char* output, int max_length);

#endif // CLLM_VOCAB_BUILDER_H