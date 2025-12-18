/**
 * Large-Scale Performance Benchmark Suite
 * Tests the mathematical framework with production-scale datasets
 */

#include "ai/cllm_lattice_embeddings.h"
#include "ai/cllm_angular_attention.h"
#include "ai/cllm_ntt_attention.h"
#include "ai/cllm_cymatic_training.h"
#include "ai/cllm_lattice.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Benchmark configuration
typedef struct {
    int seq_length;
    int vocab_size;
    int embed_dim;
    int num_heads;
    int num_iterations;
    const char* test_name;
} BenchmarkConfig;

// Benchmark results
typedef struct {
    double avg_time_ms;
    double min_time_ms;
    double max_time_ms;
    double throughput_tokens_per_sec;
    size_t memory_usage_bytes;
    int success_count;
    int failure_count;
} BenchmarkResult;

// Timer utilities
static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// Memory tracking
static size_t current_memory_usage = 0;
static size_t peak_memory_usage = 0;

static void track_allocation(size_t size) {
    current_memory_usage += size;
    if (current_memory_usage > peak_memory_usage) {
        peak_memory_usage = current_memory_usage;
    }
}

static void track_deallocation(size_t size) {
    current_memory_usage -= size;
}

/**
 * Benchmark 1: Lattice Embedding Initialization at Scale
 */
static BenchmarkResult benchmark_lattice_embeddings(BenchmarkConfig* config) {
    printf("\n=== Benchmark: Lattice Embeddings (seq_len=%d, vocab=%d, dim=%d) ===\n",
           config->seq_length, config->vocab_size, config->embed_dim);
    
    BenchmarkResult result = {0};
    result.min_time_ms = 1e9;
    
    CLLMConfig cllm_config = {
        .vocab_size = config->vocab_size,
        .embed_dim = config->embed_dim,
        .num_heads = config->num_heads,
        .num_layers = 4,
        .max_seq_len = config->seq_length,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    for (int iter = 0; iter < config->num_iterations; iter++) {
        // Allocate model
        CLLMModel* model = cllm_create_model(&cllm_config);
        if (!model) {
            result.failure_count++;
            continue;
        }
        
        size_t model_size = sizeof(CLLMModel) + 
                           config->vocab_size * config->embed_dim * sizeof(float);
        track_allocation(model_size);
        
        // Time the lattice embedding initialization
        double start = get_time_ms();
        
        int init_result = cllm_init_lattice_embeddings(
            model,
            8,      // dimension
            24,     // kissing number
            1.0f    // lambda
        );
        
        double end = get_time_ms();
        double elapsed = end - start;
        
        if (init_result == 0) {
            result.success_count++;
            result.avg_time_ms += elapsed;
            if (elapsed < result.min_time_ms) result.min_time_ms = elapsed;
            if (elapsed > result.max_time_ms) result.max_time_ms = elapsed;
        } else {
            result.failure_count++;
        }
        
        // Cleanup
        cllm_free_model(model);
        track_deallocation(model_size);
    }
    
    if (result.success_count > 0) {
        result.avg_time_ms /= result.success_count;
        result.throughput_tokens_per_sec = 
            (config->vocab_size * 1000.0) / result.avg_time_ms;
    }
    result.memory_usage_bytes = peak_memory_usage;
    
    printf("Results:\n");
    printf("  Success: %d/%d\n", result.success_count, config->num_iterations);
    printf("  Avg Time: %.2f ms\n", result.avg_time_ms);
    printf("  Min Time: %.2f ms\n", result.min_time_ms);
    printf("  Max Time: %.2f ms\n", result.max_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", result.throughput_tokens_per_sec);
    printf("  Peak Memory: %.2f MB\n", peak_memory_usage / (1024.0 * 1024.0));
    
    return result;
}

/**
 * Benchmark 2: NTT Attention at Scale
 */
static BenchmarkResult benchmark_ntt_attention_scale(BenchmarkConfig* config) {
    printf("\n=== Benchmark: NTT Attention (seq_len=%d, heads=%d, dim=%d) ===\n",
           config->seq_length, config->num_heads, config->embed_dim);
    
    BenchmarkResult result = {0};
    result.min_time_ms = 1e9;
    
    // Allocate attention matrices
    size_t matrix_size = config->seq_length * config->embed_dim * sizeof(float);
    float* query = (float*)malloc(matrix_size);
    float* key = (float*)malloc(matrix_size);
    float* value = (float*)malloc(matrix_size);
    float* output = (float*)malloc(matrix_size);
    
    if (!query || !key || !value || !output) {
        printf("ERROR: Failed to allocate attention matrices\n");
        free(query); free(key); free(value); free(output);
        return result;
    }
    
    track_allocation(matrix_size * 4);
    
    // Initialize with random data
    for (int i = 0; i < config->seq_length * config->embed_dim; i++) {
        query[i] = (float)rand() / RAND_MAX;
        key[i] = (float)rand() / RAND_MAX;
        value[i] = (float)rand() / RAND_MAX;
    }
    
    for (int iter = 0; iter < config->num_iterations; iter++) {
        double start = get_time_ms();
        
        int attn_result = cllm_attention_ntt_forward(
            query, key, value, output,
            config->seq_length,
            config->embed_dim,
            config->num_heads
        );
        
        double end = get_time_ms();
        double elapsed = end - start;
        
        if (attn_result == 0) {
            result.success_count++;
            result.avg_time_ms += elapsed;
            if (elapsed < result.min_time_ms) result.min_time_ms = elapsed;
            if (elapsed > result.max_time_ms) result.max_time_ms = elapsed;
        } else {
            result.failure_count++;
        }
    }
    
    if (result.success_count > 0) {
        result.avg_time_ms /= result.success_count;
        result.throughput_tokens_per_sec = 
            (config->seq_length * 1000.0) / result.avg_time_ms;
    }
    result.memory_usage_bytes = peak_memory_usage;
    
    printf("Results:\n");
    printf("  Success: %d/%d\n", result.success_count, config->num_iterations);
    printf("  Avg Time: %.2f ms\n", result.avg_time_ms);
    printf("  Min Time: %.2f ms\n", result.min_time_ms);
    printf("  Max Time: %.2f ms\n", result.max_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", result.throughput_tokens_per_sec);
    printf("  Peak Memory: %.2f MB\n", peak_memory_usage / (1024.0 * 1024.0));
    
    // Cleanup
    free(query);
    free(key);
    free(value);
    free(output);
    track_deallocation(matrix_size * 4);
    
    return result;
}

/**
 * Benchmark 3: Full Forward Pass at Scale
 */
static BenchmarkResult benchmark_full_forward_pass(BenchmarkConfig* config) {
    printf("\n=== Benchmark: Full Forward Pass (seq_len=%d) ===\n",
           config->seq_length);
    
    BenchmarkResult result = {0};
    result.min_time_ms = 1e9;
    
    CLLMConfig cllm_config = {
        .vocab_size = config->vocab_size,
        .embed_dim = config->embed_dim,
        .num_heads = config->num_heads,
        .num_layers = 4,
        .max_seq_len = config->seq_length,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    CLLMModel* model = cllm_create_model(&cllm_config);
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return result;
    }
    
    // Initialize with lattice embeddings
    cllm_init_lattice_embeddings(model, 8, 24, 1.0f);
    
    // Allocate input tokens
    int* tokens = (int*)malloc(config->seq_length * sizeof(int));
    if (!tokens) {
        printf("ERROR: Failed to allocate tokens\n");
        cllm_free_model(model);
        return result;
    }
    
    // Initialize tokens
    for (int i = 0; i < config->seq_length; i++) {
        tokens[i] = rand() % config->vocab_size;
    }
    
    for (int iter = 0; iter < config->num_iterations; iter++) {
        double start = get_time_ms();
        
        float* logits = cllm_forward(model, tokens, config->seq_length);
        
        double end = get_time_ms();
        double elapsed = end - start;
        
        if (logits) {
            result.success_count++;
            result.avg_time_ms += elapsed;
            if (elapsed < result.min_time_ms) result.min_time_ms = elapsed;
            if (elapsed > result.max_time_ms) result.max_time_ms = elapsed;
        } else {
            result.failure_count++;
        }
    }
    
    if (result.success_count > 0) {
        result.avg_time_ms /= result.success_count;
        result.throughput_tokens_per_sec = 
            (config->seq_length * 1000.0) / result.avg_time_ms;
    }
    result.memory_usage_bytes = peak_memory_usage;
    
    printf("Results:\n");
    printf("  Success: %d/%d\n", result.success_count, config->num_iterations);
    printf("  Avg Time: %.2f ms\n", result.avg_time_ms);
    printf("  Min Time: %.2f ms\n", result.min_time_ms);
    printf("  Max Time: %.2f ms\n", result.max_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", result.throughput_tokens_per_sec);
    printf("  Peak Memory: %.2f MB\n", peak_memory_usage / (1024.0 * 1024.0));
    
    // Cleanup
    free(tokens);
    cllm_free_model(model);
    
    return result;
}

/**
 * Benchmark 4: Cymatic Resonance at Scale
 */
static BenchmarkResult benchmark_cymatic_resonance(BenchmarkConfig* config) {
    printf("\n=== Benchmark: Cymatic Resonance (seq_len=%d) ===\n",
           config->seq_length);
    
    BenchmarkResult result = {0};
    result.min_time_ms = 1e9;
    
    CLLMConfig cllm_config = {
        .vocab_size = config->vocab_size,
        .embed_dim = config->embed_dim,
        .num_heads = config->num_heads,
        .num_layers = 4,
        .max_seq_len = config->seq_length,
        .dropout = 0.1f,
        .learning_rate = 0.001f
    };
    
    CLLMModel* model = cllm_create_model(&cllm_config);
    if (!model) {
        printf("ERROR: Failed to create model\n");
        return result;
    }
    
    // Initialize cymatic training
    CymaticConfig cymatic_config = {
        .base_frequency = 432.0f,
        .harmonic_count = 8,
        .resonance_threshold = 0.7f,
        .phase_shift = 0.0f
    };
    
    int init_result = cllm_init_cymatic_training(model, &cymatic_config);
    if (init_result != 0) {
        printf("ERROR: Failed to initialize cymatic training\n");
        cllm_free_model(model);
        return result;
    }
    
    // Allocate gradients
    size_t grad_size = config->vocab_size * config->embed_dim * sizeof(float);
    float* gradients = (float*)malloc(grad_size);
    if (!gradients) {
        printf("ERROR: Failed to allocate gradients\n");
        cllm_free_model(model);
        return result;
    }
    
    // Initialize gradients
    for (int i = 0; i < config->vocab_size * config->embed_dim; i++) {
        gradients[i] = (float)rand() / RAND_MAX - 0.5f;
    }
    
    for (int iter = 0; iter < config->num_iterations; iter++) {
        double start = get_time_ms();
        
        int apply_result = cllm_apply_cymatic_resonance(
            model,
            gradients,
            config->vocab_size * config->embed_dim,
            iter
        );
        
        double end = get_time_ms();
        double elapsed = end - start;
        
        if (apply_result == 0) {
            result.success_count++;
            result.avg_time_ms += elapsed;
            if (elapsed < result.min_time_ms) result.min_time_ms = elapsed;
            if (elapsed > result.max_time_ms) result.max_time_ms = elapsed;
        } else {
            result.failure_count++;
        }
    }
    
    if (result.success_count > 0) {
        result.avg_time_ms /= result.success_count;
        result.throughput_tokens_per_sec = 
            (config->vocab_size * 1000.0) / result.avg_time_ms;
    }
    result.memory_usage_bytes = peak_memory_usage;
    
    printf("Results:\n");
    printf("  Success: %d/%d\n", result.success_count, config->num_iterations);
    printf("  Avg Time: %.2f ms\n", result.avg_time_ms);
    printf("  Min Time: %.2f ms\n", result.min_time_ms);
    printf("  Max Time: %.2f ms\n", result.max_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", result.throughput_tokens_per_sec);
    printf("  Peak Memory: %.2f MB\n", peak_memory_usage / (1024.0 * 1024.0));
    
    // Cleanup
    free(gradients);
    cllm_free_model(model);
    
    return result;
}

/**
 * Run comprehensive benchmark suite
 */
static void run_benchmark_suite() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║     CRYSTALLINE CLLM - LARGE SCALE BENCHMARK SUITE            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Test configurations for different scales
    BenchmarkConfig configs[] = {
        // Small scale (baseline)
        {512, 10000, 256, 8, 10, "Small Scale (512 tokens)"},
        
        // Medium scale
        {1024, 10000, 256, 8, 10, "Medium Scale (1K tokens)"},
        
        // Large scale
        {4096, 10000, 512, 8, 5, "Large Scale (4K tokens)"},
        
        // Very large scale
        {8192, 10000, 512, 8, 3, "Very Large Scale (8K tokens)"},
        
        // Extreme scale
        {16384, 10000, 512, 8, 2, "Extreme Scale (16K tokens)"}
    };
    
    int num_configs = sizeof(configs) / sizeof(configs[0]);
    
    // Run benchmarks for each configuration
    for (int i = 0; i < num_configs; i++) {
        printf("\n");
        printf("════════════════════════════════════════════════════════════════\n");
        printf("  TEST SUITE: %s\n", configs[i].test_name);
        printf("════════════════════════════════════════════════════════════════\n");
        
        // Reset memory tracking
        current_memory_usage = 0;
        peak_memory_usage = 0;
        
        // Run all benchmarks
        benchmark_lattice_embeddings(&configs[i]);
        benchmark_ntt_attention_scale(&configs[i]);
        benchmark_full_forward_pass(&configs[i]);
        benchmark_cymatic_resonance(&configs[i]);
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║              BENCHMARK SUITE COMPLETED                         ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
}

int main(int argc, char** argv) {
    srand(time(NULL));
    
    printf("Crystalline CLLM - Large Scale Performance Benchmark\n");
    printf("====================================================\n\n");
    
    run_benchmark_suite();
    
    return 0;
}