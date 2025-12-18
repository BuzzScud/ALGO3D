/**
 * train_model.c - CLI tool for training geometric recovery models
 * 
 * Usage: ./train_model --samples <dir> --output <model.bin> [--verbose]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <time.h>
#include "../include/micro_model.h"
#include "../include/ecdsa_sample_loader.h"

#define VERSION "1.0.0"
#define MAX_SAMPLES 1000

typedef struct {
    char samples_dir[512];
    char output_file[512];
    bool verbose;
    uint32_t bit_length;
    uint64_t curve_order;
} TrainConfig;

void print_usage(const char* program_name) {
    printf("Train Geometric Recovery Model v%s\n\n", VERSION);
    printf("Usage: %s --samples <dir> --output <file> [options]\n\n", program_name);
    printf("Required Arguments:\n");
    printf("  --samples <dir>     Directory containing ECDSA samples\n");
    printf("  --output <file>     Output model file (.bin)\n\n");
    printf("Optional Arguments:\n");
    printf("  --bit-length <n>    Bit length (default: 256)\n");
    printf("  --curve-order <n>   Curve order (default: secp256k1)\n");
    printf("  --verbose           Enable verbose output\n");
    printf("  --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  # Train on samples directory\n");
    printf("  %s --samples samples/ --output model.bin\n\n", program_name);
    printf("  # Train with verbose output\n");
    printf("  %s --samples samples/ --output model.bin --verbose\n\n", program_name);
}

int parse_arguments(int argc, char** argv, TrainConfig* config) {
    // Set defaults
    memset(config, 0, sizeof(TrainConfig));
    config->bit_length = 256;
    config->curve_order = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141ULL; // secp256k1
    config->verbose = false;
    
    static struct option long_options[] = {
        {"samples", required_argument, 0, 's'},
        {"output", required_argument, 0, 'o'},
        {"bit-length", required_argument, 0, 'b'},
        {"curve-order", required_argument, 0, 'c'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "s:o:b:c:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                strncpy(config->samples_dir, optarg, sizeof(config->samples_dir) - 1);
                break;
            case 'o':
                strncpy(config->output_file, optarg, sizeof(config->output_file) - 1);
                break;
            case 'b':
                config->bit_length = atoi(optarg);
                break;
            case 'c':
                config->curve_order = strtoull(optarg, NULL, 0);
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
    if (strlen(config->samples_dir) == 0) {
        fprintf(stderr, "Error: --samples directory is required\n");
        return -1;
    }
    
    if (strlen(config->output_file) == 0) {
        fprintf(stderr, "Error: --output file is required\n");
        return -1;
    }
    
    return 0;
}

int load_samples_from_directory(const char* dir, TrainingSample* samples, int max_samples) {
    DIR* d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Error: Cannot open directory %s\n", dir);
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(d)) != NULL && count < max_samples) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Only process .txt files
        if (strstr(entry->d_name, ".txt") == NULL) {
            continue;
        }
        
        // Build full path
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir, entry->d_name);
        
        // Load sample from file
        FILE* f = fopen(filepath, "r");
        if (!f) continue;
        
        // Parse sample format: k,Q,error
        uint64_t k, Q;
        double error;
        if (fscanf(f, "%lu,%lu,%lf", &k, &Q, &error) == 3) {
            samples[count].k = k;
            samples[count].Q = Q;
            samples[count].error = error;
            count++;
        }
        
        fclose(f);
    }
    
    closedir(d);
    return count;
}

int main(int argc, char** argv) {
    TrainConfig config;
    
    // Parse arguments
    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Geometric Recovery Model Training v%s\n", VERSION);
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    // Load samples
    if (config.verbose) {
        printf("Loading samples from: %s\n", config.samples_dir);
    }
    
    TrainingSample* samples = calloc(MAX_SAMPLES, sizeof(TrainingSample));
    if (!samples) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }
    
    int num_samples = load_samples_from_directory(config.samples_dir, samples, MAX_SAMPLES);
    
    if (num_samples == 0) {
        fprintf(stderr, "Error: No samples loaded from %s\n", config.samples_dir);
        free(samples);
        return 1;
    }
    
    printf("✓ Loaded %d training samples\n", num_samples);
    
    // Create model
    if (config.verbose) {
        printf("\nCreating model...\n");
        printf("  Bit length: %u\n", config.bit_length);
        printf("  Curve order: %lu\n", config.curve_order);
    }
    
    MicroModel* model = micro_model_create("trained_model", config.bit_length, config.curve_order);
    if (!model) {
        fprintf(stderr, "Error: Failed to create model\n");
        free(samples);
        return 1;
    }
    
    printf("✓ Model created\n");
    
    // Train model
    if (config.verbose) {
        printf("\nTraining model on %d samples...\n", num_samples);
    }
    
    clock_t start = clock();
    
    if (micro_model_train(model, samples, num_samples) != 0) {
        fprintf(stderr, "Error: Training failed\n");
        micro_model_free(model);
        free(samples);
        return 1;
    }
    
    clock_t end = clock();
    double training_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("✓ Training complete (%.2f seconds)\n", training_time);
    
    // Add example torus parameters (in real implementation, these would be computed from samples)
    if (config.verbose) {
        printf("\nAdding torus parameters...\n");
    }
    
    // Add primary torus
    double center = config.curve_order / 2.0;
    double amplitude = config.curve_order / 4.0;
    micro_model_add_torus(model, 1, center, amplitude, 2.5, 0.0, 0.90);
    
    // Add secondary torus
    micro_model_add_torus(model, 2, center, amplitude * 0.8, 3.0, 0.0, 0.85);
    
    printf("✓ Added %u torus parameters\n", model->num_tori);
    
    // Set G estimate (placeholder - would be computed from samples)
    micro_model_set_g_estimate(model, 7.0, 0.85);
    
    // Set clock info (placeholder - would be extracted from samples)
    micro_model_set_clock_info(model, 2, 5);
    
    // Save model
    if (config.verbose) {
        printf("\nSaving model to: %s\n", config.output_file);
    }
    
    if (micro_model_save(model, config.output_file) != 0) {
        fprintf(stderr, "Error: Failed to save model to %s\n", config.output_file);
        micro_model_free(model);
        free(samples);
        return 1;
    }
    
    printf("✓ Model saved successfully\n");
    
    // Print summary
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Training Summary\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    printf("Training samples:    %d\n", num_samples);
    printf("Training time:       %.2f seconds\n", training_time);
    printf("Training error:      %.4f\n", model->training_error);
    printf("Model file:          %s\n", config.output_file);
    printf("Model size:          %lu bytes\n", sizeof(MicroModel));
    printf("\n");
    
    // Cleanup
    micro_model_free(model);
    free(samples);
    
    printf("✓ Training complete!\n\n");
    
    return 0;
}
