/**
 * recover_k.c - CLI tool for recovering k from Q using trained model
 * 
 * Usage: ./recover_k --model <model.bin> --Q <point> [--verbose]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include "../include/micro_model.h"

#define VERSION "1.0.0"

typedef struct {
    char model_file[512];
    uint64_t Q;
    uint64_t true_k;  // Optional, for validation
    bool has_true_k;
    bool verbose;
} RecoverConfig;

void print_usage(const char* program_name) {
    printf("Recover k from Q using Geometric Recovery v%s\n\n", VERSION);
    printf("Usage: %s --model <file> --Q <point> [options]\n\n", program_name);
    printf("Required Arguments:\n");
    printf("  --model <file>      Trained model file (.bin)\n");
    printf("  --Q <point>         Q point value (hex or decimal)\n\n");
    printf("Optional Arguments:\n");
    printf("  --true-k <value>    True k value (for validation)\n");
    printf("  --verbose           Enable verbose output\n");
    printf("  --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  # Recover k from Q\n");
    printf("  %s --model model.bin --Q 0x1234567890abcdef\n\n", program_name);
    printf("  # Recover with validation\n");
    printf("  %s --model model.bin --Q 12345 --true-k 67890 --verbose\n\n", program_name);
}

int parse_arguments(int argc, char** argv, RecoverConfig* config) {
    // Set defaults
    memset(config, 0, sizeof(RecoverConfig));
    config->verbose = false;
    config->has_true_k = false;
    
    static struct option long_options[] = {
        {"model", required_argument, 0, 'm'},
        {"Q", required_argument, 0, 'q'},
        {"true-k", required_argument, 0, 'k'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "m:q:k:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                strncpy(config->model_file, optarg, sizeof(config->model_file) - 1);
                break;
            case 'q':
                config->Q = strtoull(optarg, NULL, 0);
                break;
            case 'k':
                config->true_k = strtoull(optarg, NULL, 0);
                config->has_true_k = true;
                break;
            case 'v':
                config->verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return -1;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    // Validate required arguments
    if (strlen(config->model_file) == 0) {
        fprintf(stderr, "Error: --model file is required\n");
        return -1;
    }
    
    if (config->Q == 0) {
        fprintf(stderr, "Error: --Q point is required\n");
        return -1;
    }
    
    return 0;
}

int main(int argc, char** argv) {
    RecoverConfig config;
    
    // Parse arguments
    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Geometric Recovery - k Recovery v%s\n", VERSION);
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    // Load model
    if (config.verbose) {
        printf("Loading model from: %s\n", config.model_file);
    }
    
    MicroModel* model = micro_model_load(config.model_file);
    if (!model) {
        fprintf(stderr, "Error: Failed to load model from %s\n", config.model_file);
        return 1;
    }
    
    printf("✓ Model loaded successfully\n");
    
    if (config.verbose) {
        printf("\nModel Information:\n");
        printf("  Name: %s\n", model->name);
        printf("  Bit length: %u\n", model->bit_length);
        printf("  Curve order: %lu\n", model->n);
        printf("  Number of tori: %u\n", model->num_tori);
        printf("  G estimate: %.4f (confidence: %.4f)\n", 
               model->g_estimate, model->g_confidence);
        printf("  Clock lattice: p=%lu, q=%lu\n", 
               model->clock_info.p, model->clock_info.q);
    }
    
    // Perform recovery
    printf("\n");
    if (config.verbose) {
        printf("Performing recovery for Q = %lu (0x%lx)\n", config.Q, config.Q);
    }
    
    uint64_t k_min, k_max;
    if (micro_model_recover(model, config.Q, &k_min, &k_max) != 0) {
        fprintf(stderr, "Error: Recovery failed\n");
        micro_model_free(model);
        return 1;
    }
    
    printf("✓ Recovery complete\n");
    
    // Calculate metrics
    uint64_t search_space = k_max - k_min + 1;
    double reduction_factor = (double)model->n / (double)search_space;
    double percentage_eliminated = (1.0 - 1.0/reduction_factor) * 100.0;
    
    // Print results
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Recovery Results\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    printf("Q point:             %lu (0x%lx)\n", config.Q, config.Q);
    printf("k bounds:            [%lu, %lu]\n", k_min, k_max);
    printf("Search space:        %lu values\n", search_space);
    printf("Original space:      %lu values\n", model->n);
    printf("Reduction factor:    %.2fx\n", reduction_factor);
    printf("Space eliminated:    %.2f%%\n", percentage_eliminated);
    
    // Validation if true k provided
    if (config.has_true_k) {
        printf("\n");
        printf("Validation:\n");
        printf("  True k:            %lu\n", config.true_k);
        
        bool captured = (config.true_k >= k_min && config.true_k <= k_max);
        printf("  Captured:          %s\n", captured ? "✓ YES" : "✗ NO");
        
        if (captured) {
            uint64_t offset_from_min = config.true_k - k_min;
            double position_in_range = (double)offset_from_min / (double)search_space * 100.0;
            printf("  Position in range: %.2f%%\n", position_in_range);
        } else {
            printf("  ⚠ WARNING: True k not captured in bounds!\n");
        }
    }
    
    printf("\n");
    
    // Provide search guidance
    if (config.verbose) {
        printf("Search Strategy:\n");
        printf("  1. Start at k_min = %lu\n", k_min);
        printf("  2. Increment k by 1\n");
        printf("  3. Test each k: Q = k * G\n");
        printf("  4. Stop when match found or k > k_max\n");
        printf("  5. Expected iterations: %lu (vs %lu without recovery)\n", 
               search_space, model->n);
        printf("\n");
    }
    
    // Cleanup
    micro_model_free(model);
    
    printf("✓ Recovery complete!\n\n");
    
    return 0;
}
