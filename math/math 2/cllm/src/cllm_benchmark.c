#include "cllm.h"
#include "cllm_inference.h"
#include "cllm_training.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

// Timing utilities
typedef struct {
    struct timeval start;
    struct timeval end;
    double elapsed_ms;
} Timer;

void timer_start(Timer* timer) {
    gettimeofday(&timer->start, NULL);
}

void timer_stop(Timer* timer) {
    gettimeofday(&timer->end, NULL);
    timer->elapsed_ms = (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 +
                        (timer->end.tv_usec - timer->start.tv_usec) / 1000.0;
}

// Memory usage utilities
typedef struct {
    size_t peak_rss_kb;      // Peak resident set size
    size_t current_rss_kb;   // Current resident set size
    size_t heap_size_kb;     // Heap size
} MemoryUsage;

void get_memory_usage(MemoryUsage* usage) {
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, &r_usage);
    
    usage->peak_rss_kb = r_usage.ru_maxrss;
    
    // On Linux, ru_maxrss is in kilobytes
    // On macOS, it's in bytes, so we need to convert
    #ifdef __APPLE__
    usage->peak_rss_kb /= 1024;
    #endif
    
    usage->current_rss_kb = usage->peak_rss_kb; // Approximation
    usage->heap_size_kb = 0; // Would need malloc_info on Linux
}

// Helper function to calculate total weights in model
static size_t cllm_calculate_num_weights(CLLMModel* model) {
    size_t total = 0;
    
    // Embeddings
    total += model->vocab_size * model->embedding_dim;
    
    // Positional encoding
    total += model->max_seq_len * model->embedding_dim;
    
    // Layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Attention weights (Q, K, V, output)
        total += 4 * model->embedding_dim * model->embedding_dim;
        
        // Feed-forward weights
        total += model->embedding_dim * model->hidden_dim;  // w1
        total += model->hidden_dim * model->embedding_dim;  // w2
        total += model->hidden_dim;  // b1
        total += model->embedding_dim;  // b2
        
        // Layer norm (2 per layer)
        total += 2 * model->embedding_dim;  // gamma
        total += 2 * model->embedding_dim;  // beta
    }
    
    // Output projection
    total += model->embedding_dim * model->vocab_size;
    total += model->vocab_size;  // bias
    
    return total;
}

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
                                                  size_t seq_length, int num_iterations) {
    BenchmarkResults results = {0};
    Timer timer;
    MemoryUsage mem_before, mem_after;
    
    if (!model || !input_ids || num_iterations <= 0) {
        fprintf(stderr, "Invalid benchmark parameters\n");
        return results;
    }
    
    printf("Benchmarking single token inference...\n");
    printf("  Sequence length: %zu\n", seq_length);
    printf("  Iterations: %d\n", num_iterations);
    
    // Allocate output buffer
    float* logits = (float*)malloc(model->vocab_size * sizeof(float));
    if (!logits) {
        fprintf(stderr, "Failed to allocate logits buffer\n");
        return results;
    }
    
    // Get initial memory usage
    get_memory_usage(&mem_before);
    
    // Warm-up run
    // In a real implementation, this would call the actual inference function
    // For now, we'll simulate it
    
    // Benchmark runs
    timer_start(&timer);
    
    for (int i = 0; i < num_iterations; i++) {
        // Simulate inference
        // In real implementation: cllm_forward(model, input_ids, seq_length, logits);
        
        // Dummy operation to prevent optimization
        for (size_t j = 0; j < model->vocab_size; j++) {
            logits[j] = (float)j / model->vocab_size;
        }
    }
    
    timer_stop(&timer);
    
    // Get final memory usage
    get_memory_usage(&mem_after);
    
    // Calculate results
    results.inference_time_ms = timer.elapsed_ms / num_iterations;
    results.tokens_per_second = 1000.0 / results.inference_time_ms;
    results.memory_mb = (mem_after.peak_rss_kb - mem_before.peak_rss_kb) / 1024.0;
    results.total_tokens = num_iterations;
    results.seq_length = seq_length;
    results.batch_size = 1;
    
    free(logits);
    
    printf("Results:\n");
    printf("  Average time per token: %.3f ms\n", results.inference_time_ms);
    printf("  Tokens per second: %.2f\n", results.tokens_per_second);
    printf("  Memory delta: %.2f MB\n", results.memory_mb);
    
    return results;
}

// Benchmark batch inference
BenchmarkResults cllm_benchmark_inference_batch(CLLMModel* model, uint32_t* input_ids,
                                                 size_t batch_size, size_t seq_length,
                                                 int num_iterations) {
    BenchmarkResults results = {0};
    Timer timer;
    MemoryUsage mem_before, mem_after;
    
    if (!model || !input_ids || num_iterations <= 0) {
        fprintf(stderr, "Invalid benchmark parameters\n");
        return results;
    }
    
    printf("Benchmarking batch inference...\n");
    printf("  Batch size: %zu\n", batch_size);
    printf("  Sequence length: %zu\n", seq_length);
    printf("  Iterations: %d\n", num_iterations);
    
    // Allocate output buffer
    float* logits = (float*)malloc(batch_size * model->vocab_size * sizeof(float));
    if (!logits) {
        fprintf(stderr, "Failed to allocate logits buffer\n");
        return results;
    }
    
    get_memory_usage(&mem_before);
    
    timer_start(&timer);
    
    for (int i = 0; i < num_iterations; i++) {
        // Simulate batch inference
        for (size_t b = 0; b < batch_size; b++) {
            for (size_t j = 0; j < model->vocab_size; j++) {
                logits[b * model->vocab_size + j] = (float)j / model->vocab_size;
            }
        }
    }
    
    timer_stop(&timer);
    
    get_memory_usage(&mem_after);
    
    // Calculate results
    results.inference_time_ms = timer.elapsed_ms / num_iterations;
    results.throughput_tokens_per_sec = (batch_size * seq_length * 1000.0) / results.inference_time_ms;
    results.memory_mb = (mem_after.peak_rss_kb - mem_before.peak_rss_kb) / 1024.0;
    results.total_tokens = num_iterations * batch_size * seq_length;
    results.batch_size = batch_size;
    results.seq_length = seq_length;
    
    free(logits);
    
    printf("Results:\n");
    printf("  Average time per batch: %.3f ms\n", results.inference_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", results.throughput_tokens_per_sec);
    printf("  Memory delta: %.2f MB\n", results.memory_mb);
    
    return results;
}

// Benchmark forward pass
BenchmarkResults cllm_benchmark_forward_pass(CLLMModel* model, size_t batch_size,
                                              size_t seq_length, int num_iterations) {
    BenchmarkResults results = {0};
    Timer timer;
    
    printf("Benchmarking forward pass...\n");
    printf("  Batch size: %zu\n", batch_size);
    printf("  Sequence length: %zu\n", seq_length);
    printf("  Iterations: %d\n", num_iterations);
    
    // Allocate buffers
    uint32_t* input_ids = (uint32_t*)malloc(batch_size * seq_length * sizeof(uint32_t));
    float* output = (float*)malloc(batch_size * seq_length * model->embedding_dim * sizeof(float));
    
    if (!input_ids || !output) {
        fprintf(stderr, "Failed to allocate buffers\n");
        if (input_ids) free(input_ids);
        if (output) free(output);
        return results;
    }
    
    // Initialize input with random token IDs
    for (size_t i = 0; i < batch_size * seq_length; i++) {
        input_ids[i] = rand() % model->vocab_size;
    }
    
    timer_start(&timer);
    
    for (int i = 0; i < num_iterations; i++) {
        // Simulate forward pass through all layers
        // In real implementation: cllm_forward_complete(model, input_ids, batch_size, seq_length, output);
        
        // Dummy computation
        for (size_t j = 0; j < batch_size * seq_length * model->embedding_dim; j++) {
            output[j] = (float)j / (batch_size * seq_length * model->embedding_dim);
        }
    }
    
    timer_stop(&timer);
    
    results.inference_time_ms = timer.elapsed_ms / num_iterations;
    results.throughput_tokens_per_sec = (batch_size * seq_length * 1000.0) / results.inference_time_ms;
    results.batch_size = batch_size;
    results.seq_length = seq_length;
    
    free(input_ids);
    free(output);
    
    printf("Results:\n");
    printf("  Average forward pass time: %.3f ms\n", results.inference_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", results.throughput_tokens_per_sec);
    
    return results;
}

// Benchmark training step
BenchmarkResults cllm_benchmark_training_step(CLLMModel* model, size_t batch_size,
                                               size_t seq_length, int num_iterations) {
    BenchmarkResults results = {0};
    Timer timer;
    MemoryUsage mem_before, mem_after;
    
    printf("Benchmarking training step...\n");
    printf("  Batch size: %zu\n", batch_size);
    printf("  Sequence length: %zu\n", seq_length);
    printf("  Iterations: %d\n", num_iterations);
    
    // Allocate buffers
    uint32_t* input_ids = (uint32_t*)malloc(batch_size * seq_length * sizeof(uint32_t));
    uint32_t* target_ids = (uint32_t*)malloc(batch_size * seq_length * sizeof(uint32_t));
    float* gradients = (float*)malloc(cllm_calculate_num_weights(model) * sizeof(float));
    
    if (!input_ids || !target_ids || !gradients) {
        fprintf(stderr, "Failed to allocate buffers\n");
        if (input_ids) free(input_ids);
        if (target_ids) free(target_ids);
        if (gradients) free(gradients);
        return results;
    }
    
    // Initialize inputs
    for (size_t i = 0; i < batch_size * seq_length; i++) {
        input_ids[i] = rand() % model->vocab_size;
        target_ids[i] = rand() % model->vocab_size;
    }
    
    get_memory_usage(&mem_before);
    
    timer_start(&timer);
    
    for (int i = 0; i < num_iterations; i++) {
        // Simulate training step (forward + backward + optimizer)
        // In real implementation:
        // 1. Forward pass
        // 2. Compute loss
        // 3. Backward pass
        // 4. Optimizer step
        
        // Dummy computation
        for (size_t j = 0; j < cllm_calculate_num_weights(model); j++) {
            gradients[j] = (float)j / cllm_calculate_num_weights(model);
        }
    }
    
    timer_stop(&timer);
    
    get_memory_usage(&mem_after);
    
    results.inference_time_ms = timer.elapsed_ms / num_iterations;
    results.throughput_tokens_per_sec = (batch_size * seq_length * 1000.0) / results.inference_time_ms;
    results.memory_mb = (mem_after.peak_rss_kb - mem_before.peak_rss_kb) / 1024.0;
    results.batch_size = batch_size;
    results.seq_length = seq_length;
    
    free(input_ids);
    free(target_ids);
    free(gradients);
    
    printf("Results:\n");
    printf("  Average training step time: %.3f ms\n", results.inference_time_ms);
    printf("  Throughput: %.2f tokens/sec\n", results.throughput_tokens_per_sec);
    printf("  Memory delta: %.2f MB\n", results.memory_mb);
    
    return results;
}

// Comprehensive benchmark suite
void cllm_run_benchmark_suite(CLLMModel* model) {
    if (!model) {
        fprintf(stderr, "Model is NULL\n");
        return;
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM Comprehensive Benchmark Suite                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    printf("Model Configuration:\n");
    printf("  Vocabulary Size: %lu\n", (unsigned long)model->vocab_size);
    printf("  Embedding Dimension: %lu\n", (unsigned long)model->embedding_dim);
    printf("  Number of Layers: %u\n", model->num_layers);
    printf("  Total Parameters: %lu\n", (unsigned long)cllm_calculate_num_weights(model));
    printf("\n");
    
    // Allocate test input
    size_t test_seq_len = 128;
    uint32_t* test_input = (uint32_t*)malloc(test_seq_len * sizeof(uint32_t));
    if (!test_input) {
        fprintf(stderr, "Failed to allocate test input\n");
        return;
    }
    
    for (size_t i = 0; i < test_seq_len; i++) {
        test_input[i] = rand() % model->vocab_size;
    }
    
    // 1. Single token inference
    printf("═══════════════════════════════════════════════════════════\n");
    printf("1. Single Token Inference Benchmark\n");
    printf("═══════════════════════════════════════════════════════════\n");
    cllm_benchmark_inference_single(model, test_input, test_seq_len, 100);
    printf("\n");
    
    // 2. Batch inference
    printf("═══════════════════════════════════════════════════════════\n");
    printf("2. Batch Inference Benchmark\n");
    printf("═══════════════════════════════════════════════════════════\n");
    cllm_benchmark_inference_batch(model, test_input, 8, test_seq_len, 50);
    printf("\n");
    
    // 3. Forward pass
    printf("═══════════════════════════════════════════════════════════\n");
    printf("3. Forward Pass Benchmark\n");
    printf("═══════════════════════════════════════════════════════════\n");
    cllm_benchmark_forward_pass(model, 4, test_seq_len, 50);
    printf("\n");
    
    // 4. Training step
    printf("═══════════════════════════════════════════════════════════\n");
    printf("4. Training Step Benchmark\n");
    printf("═══════════════════════════════════════════════════════════\n");
    cllm_benchmark_training_step(model, 4, test_seq_len, 20);
    printf("\n");
    
    free(test_input);
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              Benchmark Suite Complete                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// Profile memory usage over time
void cllm_profile_memory(CLLMModel* model, int duration_seconds) {
    if (!model) {
        fprintf(stderr, "Model is NULL\n");
        return;
    }
    
    printf("Profiling memory usage for %d seconds...\n", duration_seconds);
    
    time_t start_time = time(NULL);
    time_t current_time;
    MemoryUsage usage;
    
    size_t max_rss = 0;
    size_t min_rss = SIZE_MAX;
    
    while ((current_time = time(NULL)) - start_time < duration_seconds) {
        get_memory_usage(&usage);
        
        if (usage.current_rss_kb > max_rss) max_rss = usage.current_rss_kb;
        if (usage.current_rss_kb < min_rss) min_rss = usage.current_rss_kb;
        
        printf("  [%ld s] RSS: %.2f MB\n", 
               current_time - start_time,
               usage.current_rss_kb / 1024.0);
        
        // Sleep for 1 second
        struct timespec ts = {1, 0};
        nanosleep(&ts, NULL);
    }
    
    printf("\nMemory Profile Summary:\n");
    printf("  Peak RSS: %.2f MB\n", max_rss / 1024.0);
    printf("  Min RSS: %.2f MB\n", min_rss / 1024.0);
    printf("  Delta: %.2f MB\n", (max_rss - min_rss) / 1024.0);
}

// Generate performance report
void cllm_generate_performance_report(CLLMModel* model, const char* output_file) {
    if (!model) {
        fprintf(stderr, "Model is NULL\n");
        return;
    }
    
    FILE* fp = fopen(output_file, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open output file: %s\n", output_file);
        return;
    }
    
    fprintf(fp, "# CLLM Performance Report\n\n");
    fprintf(fp, "## Model Configuration\n\n");
    fprintf(fp, "- Vocabulary Size: %lu\n", (unsigned long)model->vocab_size);
    fprintf(fp, "- Embedding Dimension: %lu\n", (unsigned long)model->embedding_dim);
    fprintf(fp, "- Number of Layers: %u\n", model->num_layers);
    fprintf(fp, "- Total Parameters: %lu (%.2f M)\n", 
            (unsigned long)cllm_calculate_num_weights(model),
            cllm_calculate_num_weights(model) / 1000000.0);
    fprintf(fp, "\n");
    
    fprintf(fp, "## Memory Footprint\n\n");
    size_t model_size = cllm_calculate_num_weights(model) * sizeof(float);
    fprintf(fp, "- Model Weights: %.2f MB\n", model_size / (1024.0 * 1024.0));
    fprintf(fp, "- Tokens: %.2f KB\n", (model->vocab_size * sizeof(CLLMToken)) / 1024.0);
    fprintf(fp, "- Total Estimated: %.2f MB\n", 
            (model_size + model->vocab_size * sizeof(CLLMToken)) / (1024.0 * 1024.0));
    fprintf(fp, "\n");
    
    fprintf(fp, "## Benchmark Results\n\n");
    fprintf(fp, "*(Results would be populated by running actual benchmarks)*\n\n");
    
    fprintf(fp, "---\n");
    fprintf(fp, "Report generated: %s", ctime(&(time_t){time(NULL)}));
    
    fclose(fp);
    
    printf("Performance report written to: %s\n", output_file);
}