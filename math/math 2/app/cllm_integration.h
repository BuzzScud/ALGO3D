/**
 * CLLM Integration Header
 * 
 * Function declarations for CLLM integration with the application
 */

#ifndef CLLM_INTEGRATION_H
#define CLLM_INTEGRATION_H

#include "app_common.h"
#include "cllm.h"
#include "cllm_inference.h"
#include "cllm_training.h"
#include "cllm_format.h"
#include "prime_math.h"
#include "prime_rainbow.h"

// Global abacus (rainbow table) management
int app_initialize_global_abacus(void);
PrimeRainbowTable* app_get_global_abacus(void);
bool app_is_abacus_initialized(void);
void app_cleanup_global_abacus(void);

// Model creation and initialization
// Model creation functions
CLLMModel* app_create_cllm_model_default(void);        // Now uses SMALL (117M params)
CLLMModel* app_create_cllm_model_tiny(void);           // 25M parameters (ultra-fast testing)
CLLMModel* app_create_cllm_model_small(void);          // 117M parameters (GPT-2 small)
CLLMModel* app_create_cllm_model_medium(void);         // 345M parameters (GPT-2 medium)
CLLMModel* app_create_cllm_model_large(void);          // 762M parameters (GPT-2 large)
CLLMModel* app_create_cllm_model_huge(void);           // 1.5B parameters (GPT-2 XL)
CLLMModel* app_create_cllm_model_massive(void);        // 3B parameters (GPT-3 small)
CLLMModel* app_create_cllm_model_astronomical(void);   // 7B parameters (LLaMA-7B)
CLLMModel* app_create_cllm_model_auto(size_t dataset_size_mb);  // Auto-size based on data
CLLMModel* app_create_cllm_model_custom(uint32_t vocab_size,
                                         uint32_t embedding_dim,
                                         uint32_t num_layers,
                                         uint32_t num_heads,
                                         uint32_t ff_dim);

// Inference and training initialization
CLLMInference* app_init_inference(CLLMModel* model);
CLLMTraining* app_init_training(CLLMModel* model, const char* data_path);

// Text generation
int app_generate_text(AppState* state, const char* prompt, char* output, size_t output_size);

// Training
float app_train_epoch(AppState* state);

// Model I/O
int app_save_model(CLLMModel* model, const char* filepath);
CLLMModel* app_load_model(const char* filepath);

// Model management
void app_free_cllm_model(CLLMModel* model);
void app_get_model_info(CLLMModel* model, char* buffer, size_t buffer_size);

#endif /* CLLM_INTEGRATION_H */