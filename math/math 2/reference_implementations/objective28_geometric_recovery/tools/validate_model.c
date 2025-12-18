/**
 * validate_model.c - CLI tool for validating trained models
 * 
 * Usage: ./validate_model --model <model.bin> --samples <dir> --report <file>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <dirent.h>
#include <time.h>
#include "../include/micro_model.h"

#define VERSION "1.0.0"
#define MAX_SAMPLES 1000

typedef struct {
    char model_file[512];
    char samples_dir[512];
    char report_file[512];
    bool verbose;
} ValidateConfig;

typedef struct {
    int total_samples;
    int captured;
    int failed;
    double total_reduction;
    double best_reduction;
    double worst_reduction;
    double avg_search_space;
    double validation_time;
} ValidationResults;

void print_usage(const char* program_name) {
    printf("Validate Geometric Recovery Model v%s\n\n", VERSION);
    printf("Usage: %s --model <file> --samples <dir> [options]\n\n", program_name);
    printf("Required Arguments:\n");
    printf("  --model <file>      Trained model file (.bin)\n");
    printf("  --samples <dir>     Directory containing validation samples\n\n");
    printf("Optional Arguments:\n");
    printf("  --report <file>     Output report file (default: validation_report.txt)\n");
    printf("  --verbose           Enable verbose output\n");
    printf("  --help              Show this help message\n\n");
    printf("Examples:\n");
    printf("  # Validate model\n");
    printf("  %s --model model.bin --samples validation_samples/\n\n", program_name);
    printf("  # Validate with custom report\n");
    printf("  %s --model model.bin --samples samples/ --report my_report.txt\n\n", program_name);
}

int parse_arguments(int argc, char** argv, ValidateConfig* config) {
    // Set defaults
    memset(config, 0, sizeof(ValidateConfig));
    strcpy(config->report_file, "validation_report.txt");
    config->verbose = false;
    
    static struct option long_options[] = {
        {"model", required_argument, 0, 'm'},
        {"samples", required_argument, 0, 's'},
        {"report", required_argument, 0, 'r'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "m:s:r:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                strncpy(config->model_file, optarg, sizeof(config->model_file) - 1);
                break;
            case 's':
                strncpy(config->samples_dir, optarg, sizeof(config->samples_dir) - 1);
                break;
            case 'r':
                strncpy(config->report_file, optarg, sizeof(config->report_file) - 1);
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
    
    if (strlen(config->samples_dir) == 0) {
        fprintf(stderr, "Error: --samples directory is required\n");
        return -1;
    }
    
    return 0;
}

int load_validation_samples(const char* dir, ValidationSample* samples, int max_samples) {
    DIR* d = opendir(dir);
    if (!d) {
        fprintf(stderr, "Error: Cannot open directory %s\n", dir);
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(d)) != NULL && count < max_samples) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        if (strstr(entry->d_name, ".txt") == NULL) {
            continue;
        }
        
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir, entry->d_name);
        
        FILE* f = fopen(filepath, "r");
        if (!f) continue;
        
        // Parse: k,Q
        uint64_t k, Q;
        if (fscanf(f, "%lu,%lu", &k, &Q) == 2) {
            samples[count].k = k;
            samples[count].Q = Q;
            count++;
        }
        
        fclose(f);
    }
    
    closedir(d);
    return count;
}

void write_report(const char* filename, const ValidateConfig* config, 
                  const ValidationResults* results, const MicroModel* model) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Warning: Could not write report to %s\n", filename);
        return;
    }
    
    fprintf(f, "═══════════════════════════════════════════════════════════\n");
    fprintf(f, "  Geometric Recovery Model Validation Report\n");
    fprintf(f, "═══════════════════════════════════════════════════════════\n\n");
    
    time_t now = time(NULL);
    fprintf(f, "Date: %s\n", ctime(&now));
    fprintf(f, "Model: %s\n", config->model_file);
    fprintf(f, "Samples: %s\n\n", config->samples_dir);
    
    fprintf(f, "Model Information:\n");
    fprintf(f, "  Name: %s\n", model->name);
    fprintf(f, "  Bit length: %u\n", model->bit_length);
    fprintf(f, "  Curve order: %lu\n", model->n);
    fprintf(f, "  Number of tori: %u\n", model->num_tori);
    fprintf(f, "  G estimate: %.4f (confidence: %.4f)\n", 
            model->g_estimate, model->g_confidence);
    fprintf(f, "  Clock lattice: p=%lu, q=%lu\n\n", 
            model->clock_info.p, model->clock_info.q);
    
    fprintf(f, "Validation Results:\n");
    fprintf(f, "  Total samples: %d\n", results->total_samples);
    fprintf(f, "  Captured: %d\n", results->captured);
    fprintf(f, "  Failed: %d\n", results->failed);
    fprintf(f, "  Capture rate: %.2f%%\n", 
            (double)results->captured / results->total_samples * 100.0);
    fprintf(f, "\n");
    
    fprintf(f, "Reduction Metrics:\n");
    fprintf(f, "  Average reduction: %.2fx\n", 
            results->total_reduction / results->total_samples);
    fprintf(f, "  Best reduction: %.2fx\n", results->best_reduction);
    fprintf(f, "  Worst reduction: %.2fx\n", results->worst_reduction);
    fprintf(f, "  Average search space: %.0f values\n", 
            results->avg_search_space / results->total_samples);
    fprintf(f, "\n");
    
    fprintf(f, "Performance:\n");
    fprintf(f, "  Validation time: %.2f seconds\n", results->validation_time);
    fprintf(f, "  Time per sample: %.4f seconds\n", 
            results->validation_time / results->total_samples);
    fprintf(f, "\n");
    
    double percentage_eliminated = (1.0 - results->total_samples / 
                                   (results->total_reduction * results->total_samples)) * 100.0;
    fprintf(f, "Summary:\n");
    fprintf(f, "  Space eliminated: %.2f%%\n", percentage_eliminated);
    fprintf(f, "  Status: %s\n", 
            results->captured >= results->total_samples * 0.6 ? "✓ PASS" : "✗ FAIL");
    fprintf(f, "\n");
    
    fclose(f);
}

int main(int argc, char** argv) {
    ValidateConfig config;
    ValidationResults results = {0};
    
    // Parse arguments
    if (parse_arguments(argc, argv, &config) != 0) {
        return 1;
    }
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  Geometric Recovery Model Validation v%s\n", VERSION);
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
    
    printf("✓ Model loaded\n");
    
    // Load validation samples
    if (config.verbose) {
        printf("Loading validation samples from: %s\n", config.samples_dir);
    }
    
    ValidationSample* samples = calloc(MAX_SAMPLES, sizeof(ValidationSample));
    if (!samples) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        micro_model_free(model);
        return 1;
    }
    
    int num_samples = load_validation_samples(config.samples_dir, samples, MAX_SAMPLES);
    
    if (num_samples == 0) {
        fprintf(stderr, "Error: No samples loaded from %s\n", config.samples_dir);
        free(samples);
        micro_model_free(model);
        return 1;
    }
    
    printf("✓ Loaded %d validation samples\n", num_samples);
    
    // Validate model
    printf("\nValidating model...\n");
    
    clock_t start = clock();
    
    results.total_samples = num_samples;
    results.best_reduction = 0.0;
    results.worst_reduction = 1000000.0;
    
    for (int i = 0; i < num_samples; i++) {
        if (config.verbose && i % 10 == 0) {
            printf("  Progress: %d/%d (%.1f%%)\r", i, num_samples, 
                   (double)i / num_samples * 100.0);
            fflush(stdout);
        }
        
        uint64_t k_min, k_max;
        if (micro_model_recover(model, samples[i].Q, &k_min, &k_max) == 0) {
            // Check if true k is captured
            bool captured = (samples[i].k >= k_min && samples[i].k <= k_max);
            
            if (captured) {
                results.captured++;
            } else {
                results.failed++;
            }
            
            // Calculate reduction
            uint64_t search_space = k_max - k_min + 1;
            double reduction = (double)model->n / (double)search_space;
            
            results.total_reduction += reduction;
            results.avg_search_space += search_space;
            
            if (reduction > results.best_reduction) {
                results.best_reduction = reduction;
            }
            if (reduction < results.worst_reduction) {
                results.worst_reduction = reduction;
            }
        }
    }
    
    clock_t end = clock();
    results.validation_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    if (config.verbose) {
        printf("\n");
    }
    
    printf("✓ Validation complete\n");
    
    // Print results
    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Validation Results\n");
    printf("═══════════════════════════════════════════════════════════\n\n");
    
    double capture_rate = (double)results.captured / results.total_samples * 100.0;
    double avg_reduction = results.total_reduction / results.total_samples;
    
    printf("Total samples:       %d\n", results.total_samples);
    printf("Captured:            %d\n", results.captured);
    printf("Failed:              %d\n", results.failed);
    printf("Capture rate:        %.2f%%\n", capture_rate);
    printf("\n");
    printf("Average reduction:   %.2fx\n", avg_reduction);
    printf("Best reduction:      %.2fx\n", results.best_reduction);
    printf("Worst reduction:     %.2fx\n", results.worst_reduction);
    printf("Avg search space:    %.0f values\n", 
           results.avg_search_space / results.total_samples);
    printf("\n");
    printf("Validation time:     %.2f seconds\n", results.validation_time);
    printf("Time per sample:     %.4f seconds\n", 
           results.validation_time / results.total_samples);
    printf("\n");
    
    // Status
    bool passed = capture_rate >= 60.0;
    printf("Status:              %s\n", passed ? "✓ PASS (≥60%%)" : "✗ FAIL (<60%%)");
    printf("\n");
    
    // Write report
    if (config.verbose) {
        printf("Writing report to: %s\n", config.report_file);
    }
    
    write_report(config.report_file, &config, &results, model);
    printf("✓ Report written to %s\n", config.report_file);
    
    // Cleanup
    free(samples);
    micro_model_free(model);
    
    printf("\n✓ Validation complete!\n\n");
    
    return passed ? 0 : 1;
}