#ifndef CLLM_UTILS_H
#define CLLM_UTILS_H

#include "cllm.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Model Creation Utilities
 */

// Create a configuration
CLLMConfig* cllm_create_config(uint32_t vocab_size,
                               uint32_t embedding_dim,
                               uint32_t num_layers,
                               uint32_t num_heads,
                               uint32_t ff_dim);

// Free configuration
void cllm_free_config(CLLMConfig* config);

// Validate configuration
bool cllm_validate_config(const CLLMConfig* config);

// Print configuration
void cllm_print_config(const CLLMConfig* config);

// Create a model from configuration
CLLMModel* cllm_create_model(const CLLMConfig* config);

// Free model and all associated memory
void cllm_free_model(CLLMModel* model);

// Estimate memory usage for a model configuration
size_t cllm_estimate_memory(const CLLMConfig* config);

// Print model information
void cllm_print_model_info(const CLLMModel* model);

// Create pre-configured models
CLLMModel* cllm_create_small_model(void);
CLLMModel* cllm_create_medium_model(void);
CLLMModel* cllm_create_large_model(void);

// ============================================================================
// PLATONIC GEOMETRY MODEL CREATION (OBJECTIVE 25)
// ============================================================================

/**
 * Create a model based on Platonic solid geometry
 * 
 * Dimensions are automatically calculated from the Platonic solid:
 * - embedding_dim = vertices × 12
 * - ff_dim (hidden) = edges × 12
 * - num_layers = faces
 * - num_heads = 12 (always, for 12-fold symmetry)
 * 
 * @param solid_type Which Platonic solid (see ai/cllm_platonic.h for PlatonicSolidType)
 * @param vocab_size Vocabulary size
 * @param max_seq_len Maximum sequence length
 * @param enable_blind_recovery Enable blind recovery (OBJECTIVE 26)
 * @param enable_harmonic Enable harmonic integration (OBJECTIVE 27)
 * @param enable_ntt Enable NTT attention (OBJECTIVE 13D)
 * @return Newly created model with Platonic geometry, or NULL on failure
 */
CLLMModel* cllm_create_platonic_model(
    int solid_type,
    uint32_t vocab_size,
    uint32_t max_seq_len,
    bool enable_blind_recovery,
    bool enable_harmonic,
    bool enable_ntt
);

// Preset Platonic model configurations
CLLMModel* cllm_create_tetrahedron_model(uint32_t vocab_size, uint32_t max_seq_len);
CLLMModel* cllm_create_cube_model(uint32_t vocab_size, uint32_t max_seq_len);
CLLMModel* cllm_create_octahedron_model(uint32_t vocab_size, uint32_t max_seq_len);
CLLMModel* cllm_create_dodecahedron_model(uint32_t vocab_size, uint32_t max_seq_len);
CLLMModel* cllm_create_icosahedron_model(uint32_t vocab_size, uint32_t max_seq_len);

/*
 * Validation Utilities
 */

// Check for NaN or Inf values in array
bool cllm_check_numerical_stability(const double* array, size_t size, const char* name);

// Validate model weights for numerical stability
bool cllm_validate_weights(const CLLMModel* model);

// Validate layer normalization parameters
bool cllm_validate_layer_norms(const CLLMModel* model);

// Validate positional encodings
bool cllm_validate_positional_encodings(const CLLMModel* model);

// Comprehensive model validation
bool cllm_comprehensive_validation(const CLLMModel* model);

// Simple gradient check for a single weight
bool cllm_gradient_check_weight(CLLMModel* model, size_t weight_idx, float epsilon);

// Check gradient computation correctness
bool cllm_validate_gradients(CLLMModel* model, const double* gradients, size_t num_gradients);

// Validate inference output
bool cllm_validate_inference_output(const float* logits, size_t vocab_size);

// Validate training batch
bool cllm_validate_training_batch(const uint32_t* input_ids, const uint32_t* target_ids, 
                                   size_t batch_size, size_t seq_length, uint32_t vocab_size);

// Validate model consistency after operations
bool cllm_validate_model_consistency(const CLLMModel* model);

// Memory leak detection
void cllm_reset_memory_stats(void);
void cllm_print_memory_stats(void);

/*
 * Benchmarking Utilities
 */

// Benchmark results structure
typedef struct {
    double inference_time_ms;
    double tokens_per_second;
    double memory_mb;
    double throughput_tokens_per_sec;
    size_t total_tokens;
    size_t batch_size;
    size_t seq_length;
} BenchmarkResults;

// Benchmark single token inference
BenchmarkResults cllm_benchmark_inference_single(CLLMModel* model, uint32_t* input_ids, 
                                                  size_t seq_length, int num_iterations);

// Benchmark batch inference
BenchmarkResults cllm_benchmark_inference_batch(CLLMModel* model, uint32_t* input_ids,
                                                 size_t batch_size, size_t seq_length,
                                                 int num_iterations);

// Benchmark forward pass
BenchmarkResults cllm_benchmark_forward_pass(CLLMModel* model, size_t batch_size,
                                              size_t seq_length, int num_iterations);

// Benchmark training step
BenchmarkResults cllm_benchmark_training_step(CLLMModel* model, size_t batch_size,
                                               size_t seq_length, int num_iterations);

// Comprehensive benchmark suite
void cllm_run_benchmark_suite(CLLMModel* model);

// Profile memory usage over time
void cllm_profile_memory(CLLMModel* model, int duration_seconds);

// Generate performance report
void cllm_generate_performance_report(CLLMModel* model, const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // CLLM_UTILS_H