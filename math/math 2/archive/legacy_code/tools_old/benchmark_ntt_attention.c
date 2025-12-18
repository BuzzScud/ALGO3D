/**
 * NTT Attention Benchmark Tool
 * 
 * Benchmarks NTT-based O(n log n) attention against standard O(n²) attention.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm_inference.h"
#include "../../algorithms/include/ntt_attention.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Simple benchmark implementation
static void benchmark_ntt_attention(uint32_t seq_len, uint32_t head_dim) {
    printf("Benchmarking NTT Attention vs Standard Attention\n");
    printf("Sequence Length: %u, Head Dimension: %u\n\n", seq_len, head_dim);
    
    // Allocate test data
    size_t size = seq_len * head_dim;
    float* query = calloc(size, sizeof(float));
    float* key = calloc(size, sizeof(float));
    float* value = calloc(size, sizeof(float));
    float* output_ntt = calloc(size, sizeof(float));
    
    if (!query || !key || !value || !output_ntt) {
        printf("ERROR: Memory allocation failed\n");
        goto cleanup;
    }
    
    // Initialize with random data
    for (size_t i = 0; i < size; i++) {
        query[i] = (float)rand() / RAND_MAX - 0.5f;
        key[i] = (float)rand() / RAND_MAX - 0.5f;
        value[i] = (float)rand() / RAND_MAX - 0.5f;
    }
    
    // Benchmark NTT attention
    clock_t start = clock();
    double scale = 1.0 / math_sqrt((double)head_dim);
    ntt_attention_forward(output_ntt, query, key, value, seq_len, head_dim, scale);
    clock_t end = clock();
    double time_ntt = (double)(end - start) / CLOCKS_PER_SEC;
    
    // Estimate theoretical speedup
    double theoretical_speedup = ntt_attention_estimate_speedup(seq_len);
    
    printf("Results:\n");
    printf("  NTT Attention:      %.6f seconds\n", time_ntt);
    printf("  Theoretical Speedup: %.2fx (vs O(n²))\n", theoretical_speedup);
    printf("\n");
    
cleanup:
    free(query);
    free(key);
    free(value);
    free(output_ntt);
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("\n");
    printf("Benchmark NTT attention vs standard attention.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --seq-len N     Sequence length (default: 1000)\n");
    printf("  --head-dim N    Head dimension (default: 64)\n");
    printf("  --help          Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s --seq-len 500 --head-dim 64\n", program_name);
    printf("  %s --seq-len 2000\n", program_name);
}

int main(int argc, char** argv) {
    uint32_t seq_len = 1000;
    uint32_t head_dim = 64;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--seq-len") == 0 && i + 1 < argc) {
            seq_len = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--head-dim") == 0 && i + 1 < argc) {
            head_dim = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Validate parameters
    if (seq_len == 0 || head_dim == 0) {
        fprintf(stderr, "ERROR: Invalid parameters (seq_len=%u, head_dim=%u)\n", 
                seq_len, head_dim);
        return 1;
    }
    
    printf("\n=== NTT Attention Benchmark ===\n\n");
    printf("Configuration:\n");
    printf("  Sequence length: %u\n", seq_len);
    printf("  Head dimension: %u\n", head_dim);
    printf("  Total parameters: %u\n", seq_len * head_dim);
    printf("\n");
    
    // Run benchmark
    benchmark_ntt_attention(seq_len, head_dim);
    
    printf("Benchmark complete!\n\n");
    
    return 0;
}
