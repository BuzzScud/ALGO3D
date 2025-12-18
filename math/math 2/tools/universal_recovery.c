/*
 * Universal Recovery Tool v2.0
 * 
 * Production-grade recovery using librecovery_core
 * 
 * Features:
 * - Configurable sample count
 * - Multiple sample files
 * - Progress reporting
 * - Thread-safe
 * - Production error handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "recovery_core.h"

typedef struct {
    char* q_file;
    char** sample_files;
    int num_sample_files;
    char* output_file;
    int max_iterations;
    double threshold;
    int verbose;
    int num_samples;  // NEW: User can specify how many samples to generate
} options_t;

void print_usage(const char* prog) {
    printf("Universal Recovery Tool v2.0 - Production Grade\n\n");
    printf("Usage: %s [OPTIONS]\n\n", prog);
    printf("Required:\n");
    printf("  -q, --q-data FILE         Q data (public/output/corrupted)\n");
    printf("  -s, --sample FILE         Sample file (can be used multiple times)\n");
    printf("  -o, --output FILE         Output recovered data\n\n");
    printf("Optional:\n");
    printf("  -n, --num-samples N       Number of samples to use (0 = all, default)\n");
    printf("  -i, --iterations N        Max iterations (default: 10000)\n");
    printf("  -t, --threshold T         Convergence threshold (default: 0.001)\n");
    printf("  -v, --verbose             Verbose output (use -vv for debug)\n");
    printf("  -h, --help                Show this help\n\n");
    printf("Examples:\n");
    printf("  # Basic recovery\n");
    printf("  %s -q pubkey.bin -s partial_key.bin -o recovered.bin\n\n", prog);
    printf("  # Multiple samples for better convergence\n");
    printf("  %s -q data.bin -s sample1.bin -s sample2.bin -s sample3.bin -o out.bin\n\n", prog);
    printf("  # Limit to first 50 samples for faster convergence\n");
    printf("  %s -q data.bin -s samples.bin -n 50 -o out.bin\n\n", prog);
    printf("  # High precision recovery\n");
    printf("  %s -q data.bin -s samples.bin -i 50000 -t 0.0001 -vv -o out.bin\n\n", prog);
}

uint8_t* read_file(const char* filename, size_t* len) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t* data = malloc(*len);
    if (!data) {
        fclose(f);
        return NULL;
    }
    
    fread(data, 1, *len, f);
    fclose(f);
    
    return data;
}

int write_file(const char* filename, const uint8_t* data, size_t len) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot create file %s\n", filename);
        return 0;
    }
    
    fwrite(data, 1, len, f);
    fclose(f);
    return 1;
}

int main(int argc, char** argv) {
    options_t opts = {
        .q_file = NULL,
        .sample_files = NULL,
        .num_sample_files = 0,
        .output_file = NULL,
        .max_iterations = 10000,
        .threshold = 0.001,
        .verbose = 0,
        .num_samples = 0  // 0 = use all
    };
    
    // Allocate space for sample files
    opts.sample_files = malloc(100 * sizeof(char*));
    
    static struct option long_options[] = {
        {"q-data", required_argument, 0, 'q'},
        {"sample", required_argument, 0, 's'},
        {"output", required_argument, 0, 'o'},
        {"num-samples", required_argument, 0, 'n'},
        {"iterations", required_argument, 0, 'i'},
        {"threshold", required_argument, 0, 't'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "q:s:o:n:i:t:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'q':
                opts.q_file = optarg;
                break;
            case 's':
                opts.sample_files[opts.num_sample_files++] = optarg;
                break;
            case 'o':
                opts.output_file = optarg;
                break;
            case 'n':
                opts.num_samples = atoi(optarg);
                break;
            case 'i':
                opts.max_iterations = atoi(optarg);
                break;
            case 't':
                opts.threshold = atof(optarg);
                break;
            case 'v':
                opts.verbose++;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Validate required arguments
    if (!opts.q_file || opts.num_sample_files == 0 || !opts.output_file) {
        fprintf(stderr, "Error: Missing required arguments\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("=== Universal Recovery Tool v2.0 ===\n\n");
    printf("Configuration:\n");
    printf("  Q data: %s\n", opts.q_file);
    printf("  Sample files: %d\n", opts.num_sample_files);
    for (int i = 0; i < opts.num_sample_files; i++) {
        printf("    - %s\n", opts.sample_files[i]);
    }
    printf("  Output: %s\n", opts.output_file);
    printf("  Max iterations: %d\n", opts.max_iterations);
    printf("  Convergence threshold: %.6f\n", opts.threshold);
    if (opts.num_samples > 0) {
        printf("  Sample limit: %d\n", opts.num_samples);
    }
    printf("  Verbose: %d\n", opts.verbose);
    printf("\n");
    
    // Initialize recovery context
    recovery_config_t config = recovery_default_config();
    config.max_iterations = opts.max_iterations;
    config.convergence_threshold = opts.threshold;
    config.num_samples = opts.num_samples;
    config.verbose = opts.verbose;
    
    recovery_context_t* ctx = recovery_init(&config);
    if (!ctx) {
        fprintf(stderr, "Error: Failed to initialize recovery context\n");
        return 1;
    }
    
    // Load Q data
    size_t q_len;
    uint8_t* q_data = read_file(opts.q_file, &q_len);
    if (!q_data) {
        recovery_free(ctx);
        return 1;
    }
    
    recovery_error_t err = recovery_set_q(ctx, q_data, q_len);
    if (err != RECOVERY_OK) {
        fprintf(stderr, "Error: %s\n", recovery_error_string(err));
        free(q_data);
        recovery_free(ctx);
        return 1;
    }
    
    printf("Loaded Q data: %zu bytes\n", q_len);
    
    // Load samples
    int samples_loaded = 0;
    for (int i = 0; i < opts.num_sample_files; i++) {
        size_t sample_len;
        uint8_t* sample_data = read_file(opts.sample_files[i], &sample_len);
        if (!sample_data) continue;
        
        err = recovery_add_sample(ctx, sample_data, sample_len, 0, 1.0);
        if (err != RECOVERY_OK) {
            fprintf(stderr, "Warning: Failed to add sample %s: %s\n",
                   opts.sample_files[i], recovery_error_string(err));
            free(sample_data);
            continue;
        }
        
        samples_loaded++;
        printf("Loaded sample %d: %zu bytes\n", samples_loaded, sample_len);
        free(sample_data);
        
        // Stop if we've reached the sample limit
        if (opts.num_samples > 0 && samples_loaded >= opts.num_samples) {
            printf("Reached sample limit (%d samples)\n", opts.num_samples);
            break;
        }
    }
    
    if (samples_loaded == 0) {
        fprintf(stderr, "Error: No samples loaded\n");
        free(q_data);
        recovery_free(ctx);
        return 1;
    }
    
    printf("\nStarting recovery with %d samples...\n\n", samples_loaded);
    
    // Run recovery
    err = recovery_run(ctx);
    if (err != RECOVERY_OK && err != RECOVERY_ERROR_NOT_CONVERGED) {
        fprintf(stderr, "Error: Recovery failed: %s\n", recovery_error_string(err));
        free(q_data);
        recovery_free(ctx);
        return 1;
    }
    
    // Get result
    recovery_result_t* result = recovery_get_result(ctx);
    if (!result) {
        fprintf(stderr, "Error: Failed to get result\n");
        free(q_data);
        recovery_free(ctx);
        return 1;
    }
    
    printf("\n=== Recovery Complete ===\n");
    printf("  Iterations: %d\n", result->iterations);
    printf("  Final oscillation: %.6f\n", result->final_oscillation);
    printf("  Converged: %s\n", result->converged ? "YES" : "NO");
    printf("  Time: %.3f seconds\n", result->time_seconds);
    printf("  Quality score: %.1f%%\n", result->quality_score * 100);
    printf("  Convergence rate: %.1f iter/sec\n", result->convergence_rate);
    printf("\n");
    
    // Save result
    if (write_file(opts.output_file, result->data, result->length)) {
        printf("✓ Saved recovered data to: %s (%zu bytes)\n", 
               opts.output_file, result->length);
    } else {
        fprintf(stderr, "Error: Failed to save result\n");
    }
    
    // Cleanup
    free(q_data);
    recovery_free_result(result);
    recovery_free(ctx);
    free(opts.sample_files);
    
    return 0;
}