/*
 * image-recovery.c - Image Data Recovery Tool
 * 
 * Production-grade CLI tool for recovering corrupted image data.
 * Handles: Natural Images, Medical Images, Synthetic Patterns, Multi-Resolution, Video
 * 
 * Part of OBJECTIVE 29: Universal Recovery Toolkit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "../../algorithms/include/recovery_common/recovery_common.h"

/* Command-line options */
typedef struct {
    char* input_file;
    char* output_file;
    char* report_file;
    RecoveryAlgorithm algorithm;
    int verbose;
    int visualize;
    int benchmark;
} Options;

/* Initialize options with defaults */
static void init_options(Options* opts) {
    opts->input_file = NULL;
    opts->output_file = NULL;
    opts->report_file = NULL;
    opts->algorithm = RECOVERY_AUTO;
    opts->verbose = 0;
    opts->visualize = 0;
    opts->benchmark = 0;
}

/* Free allocated options */
static void free_options(Options* opts) {
    if (opts->input_file) free(opts->input_file);
    if (opts->output_file) free(opts->output_file);
    if (opts->report_file) free(opts->report_file);
}

/* Print usage information */
static void print_usage(const char *program_name) {
    printf("Image Recovery Tool - Production-Grade Image Data Recovery\n\n");
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Required Options:\n");
    printf("  -i, --input FILE       Input corrupted image file (JPG, PNG, TIFF, DICOM)\n");
    printf("  -o, --output FILE      Output recovered image file\n\n");
    printf("Optional Options:\n");
    printf("  -a, --algorithm ALG    Recovery algorithm:\n");
    printf("                         auto: Automatic selection (default)\n");
    printf("                         phase1: Oscillation Analysis (< 5%% corruption)\n");
    printf("                         phase2: Symmetry Detection (< 10%% corruption)\n");
    printf("                         phase3: Topology Reconstruction (< 15%% corruption)\n");
    printf("                         phase4: Recursive Stabilization (< 20%% corruption)\n");
    printf("                         phase5: Dynamic Model Expansion (< 25%% corruption)\n");
    printf("                         phase6: Hyper-Dimensional Analysis (>= 25%% corruption)\n");
    printf("  -r, --report FILE      Generate recovery report\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -V, --visualize        Visualize image (ASCII preview)\n");
    printf("  -b, --benchmark        Run performance benchmark\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -i corrupted.jpg -o recovered.jpg\n", program_name);
    printf("  %s -i corrupted.png -o recovered.png -v -V\n", program_name);
    printf("  %s -i corrupted.dcm -o recovered.dcm -a phase4 -r report.txt\n\n", program_name);
    printf("Supported Formats:\n");
    printf("  Natural Images: JPG, PNG, TIFF\n");
    printf("  Medical Images: DICOM\n");
    printf("  Multi-Channel: RGB, RGBA, Grayscale\n\n");
}

/* Parse algorithm name */
static RecoveryAlgorithm parse_algorithm(const char* name) {
    if (strcmp(name, "auto") == 0) return RECOVERY_AUTO;
    if (strcmp(name, "phase1") == 0) return RECOVERY_PHASE1;
    if (strcmp(name, "phase2") == 0) return RECOVERY_PHASE2;
    if (strcmp(name, "phase3") == 0) return RECOVERY_PHASE3;
    if (strcmp(name, "phase4") == 0) return RECOVERY_PHASE4;
    if (strcmp(name, "phase5") == 0) return RECOVERY_PHASE5;
    if (strcmp(name, "phase6") == 0) return RECOVERY_PHASE6;
    return RECOVERY_AUTO;
}

/* Parse command-line arguments */
static int parse_options(int argc, char** argv, Options* opts) {
    int c;
    static struct option long_options[] = {
        {"input",      required_argument, 0, 'i'},
        {"output",     required_argument, 0, 'o'},
        {"algorithm",  required_argument, 0, 'a'},
        {"report",     required_argument, 0, 'r'},
        {"verbose",    no_argument,       0, 'v'},
        {"visualize",  no_argument,       0, 'V'},
        {"benchmark",  no_argument,       0, 'b'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    init_options(opts);

    while ((c = getopt_long(argc, argv, "i:o:a:r:vVbh", long_options, NULL)) != -1) {
        switch (c) {
            case 'i':
                opts->input_file = strdup(optarg);
                break;
            case 'o':
                opts->output_file = strdup(optarg);
                break;
            case 'a':
                opts->algorithm = parse_algorithm(optarg);
                break;
            case 'r':
                opts->report_file = strdup(optarg);
                break;
            case 'v':
                opts->verbose = 1;
                break;
            case 'V':
                opts->visualize = 1;
                break;
            case 'b':
                opts->benchmark = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            case '?':
                return 0;
            default:
                return 0;
        }
    }

    if (!opts->input_file || !opts->output_file) {
        fprintf(stderr, "Error: Both input and output files are required\n\n");
        print_usage(argv[0]);
        return 0;
    }

    return 1;
}

/* Generate recovery report */
static void generate_report(const char* filename, const ImageData* original,
                           const ImageData* recovered, const RecoveryResult* result) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        log_message(LOG_WARN, "Could not create report file: %s", filename);
        return;
    }

    fprintf(f, "Image Recovery Report\n");
    fprintf(f, "=====================\n\n");

    fprintf(f, "Input Image:\n");
    fprintf(f, "  Width: %zu pixels\n", original->width);
    fprintf(f, "  Height: %zu pixels\n", original->height);
    fprintf(f, "  Channels: %zu\n", original->channels);
    fprintf(f, "  Total Pixels: %zu\n", original->width * original->height);
    fprintf(f, "\n");

    fprintf(f, "Recovery Algorithm: %s\n", get_algorithm_name(result->algorithm));
    fprintf(f, "\n");

    fprintf(f, "Recovery Metrics:\n");
    fprintf(f, "  Quality: %.2f%%\n", result->quality * 100.0);
    fprintf(f, "  RMSE: %.6f\n", result->rmse);
    fprintf(f, "  Max Error: %.6f\n", result->max_error);
    fprintf(f, "  Iterations: %zu\n", result->iterations);
    fprintf(f, "  Recovery Time: %.3f seconds\n", result->time_seconds);
    fprintf(f, "  Status: %s\n", result->success ? "SUCCESS" : "FAILED");
    fprintf(f, "  Message: %s\n", result->message);
    fprintf(f, "\n");

    fprintf(f, "Output Image:\n");
    fprintf(f, "  Width: %zu pixels\n", recovered->width);
    fprintf(f, "  Height: %zu pixels\n", recovered->height);
    fprintf(f, "  Channels: %zu\n", recovered->channels);
    fprintf(f, "\n");

    fclose(f);
    log_message(LOG_INFO, "Report saved to: %s", filename);
}

/* Main program */
int main(int argc, char** argv) {
    Options opts;
    int exit_code = 0;
    clock_t start_time, end_time;

    /* Parse command line options */
    if (!parse_options(argc, argv, &opts)) {
        return 1;
    }

    /* Initialize logging */
    init_logging(NULL, opts.verbose ? LOG_DEBUG : LOG_INFO);

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║          Image Recovery Tool - Production System               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    log_message(LOG_INFO, "Input: %s", opts.input_file);
    log_message(LOG_INFO, "Output: %s", opts.output_file);

    /* Load corrupted data */
    log_message(LOG_INFO, "Loading corrupted image...");
    DataType type;
    FileFormat format;
    GenericData* corrupted = load_data(opts.input_file, &type, &format);

    if (!corrupted) {
        log_message(LOG_ERROR, "Failed to load input file: %s", get_error_message(get_last_error()));
        free_options(&opts);
        close_logging();
        return 1;
    }

    if (type != DATA_TYPE_IMAGE) {
        log_message(LOG_ERROR, "Input file is not image data (type: %s)", get_data_type_name(type));
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    log_message(LOG_INFO, "Loaded %s file: %zux%zu, %zu channels",
               get_format_name(format),
               corrupted->image.width,
               corrupted->image.height,
               corrupted->image.channels);

    /* Detect corruption */
    log_message(LOG_INFO, "Detecting corruption...");
    CorruptionReport* corruption = detect_data_corruption(type, corrupted);

    if (corruption) {
        printf("\n┌─ Corruption Analysis ─────────────────────────────────────────┐\n");
        printf("│ Severity: %.1f%%                                               \n",
               corruption->severity * 100.0);
        printf("│ Type: %s                                                       \n",
               get_corruption_type_name(corruption->type));
        printf("│ Description: %s\n", corruption->description);
        printf("└────────────────────────────────────────────────────────────────┘\n\n");
    }

    /* Visualize corrupted image */
    if (opts.visualize) {
        char viz_buffer[4096];
        visualize_image_ascii(&corrupted->image, viz_buffer, sizeof(viz_buffer));
        printf("Corrupted Image (ASCII Preview):\n%s\n\n", viz_buffer);
    }

    /* Recover data */
    log_message(LOG_INFO, "Recovering image using %s...", get_algorithm_name(opts.algorithm));

    GenericData* recovered = (GenericData*)calloc(1, sizeof(GenericData));
    if (!recovered) {
        log_message(LOG_ERROR, "Out of memory");
        if (corruption) free_corruption_report(corruption);
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    start_time = clock();
    RecoveryResult* result = recover_data(type, corrupted, recovered, opts.algorithm);
    end_time = clock();

    if (!result || !result->success) {
        log_message(LOG_ERROR, "Recovery failed: %s", result ? result->message : "Unknown error");
        exit_code = 1;
    } else {
        log_message(LOG_INFO, "Recovery successful!");

        printf("┌─ Recovery Results ────────────────────────────────────────────┐\n");
        printf("│ Quality: %.2f%%                                                \n",
               result->quality * 100.0);
        printf("│ RMSE: %.6f                                                     \n",
               result->rmse);
        printf("│ Max Error: %.6f                                                \n",
               result->max_error);
        printf("│ Recovery Time: %.3f seconds                                    \n",
               result->time_seconds);
        printf("└────────────────────────────────────────────────────────────────┘\n\n");

        /* Visualize recovered image */
        if (opts.visualize) {
            char viz_buffer[4096];
            visualize_image_ascii(&recovered->image, viz_buffer, sizeof(viz_buffer));
            printf("Recovered Image (ASCII Preview):\n%s\n\n", viz_buffer);
        }

        /* Validate recovered image */
        log_message(LOG_INFO, "Validating recovered image...");
        ValidationResult* validation = validate_image(&recovered->image, &corrupted->image);

        if (validation && validation->valid) {
            printf("✓ Image validation: PASSED (Quality: %.2f%%)\n\n",
                   validation->quality_score * 100.0);
        } else {
            printf("✗ Image validation: FAILED\n\n");
            log_message(LOG_WARN, "Validation failed, but continuing with export");
        }

        if (validation) free_validation_result(validation);

        /* Save recovered image */
        log_message(LOG_INFO, "Saving recovered image...");
        if (!save_data(opts.output_file, type, recovered, format)) {
            log_message(LOG_ERROR, "Failed to save output file: %s", get_error_message(get_last_error()));
            exit_code = 1;
        } else {
            log_message(LOG_INFO, "Recovered image saved to: %s", opts.output_file);
        }

        /* Generate report if requested */
        if (opts.report_file) {
            generate_report(opts.report_file, &corrupted->image, &recovered->image, result);
        }

        /* Benchmark if requested */
        if (opts.benchmark) {
            double cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            size_t total_pixels = corrupted->image.width * corrupted->image.height;
            printf("┌─ Performance Benchmark ───────────────────────────────────────┐\n");
            printf("│ CPU Time: %.3f seconds                                         \n", cpu_time);
            printf("│ Wall Time: %.3f seconds                                        \n", result->time_seconds);
            printf("│ Pixels Processed: %zu                                          \n", total_pixels);
            printf("│ Processing Rate: %.0f pixels/second                            \n",
                   total_pixels / result->time_seconds);
            printf("│ Memory Used: %.2f MB                                           \n",
                   (total_pixels * corrupted->image.channels) / (1024.0 * 1024.0));
            printf("└────────────────────────────────────────────────────────────────┘\n\n");
        }
    }

    if (exit_code == 0) {
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║                    Recovery Complete!                          ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
    }

    /* Cleanup */
    if (result) free(result);
    if (corruption) free_corruption_report(corruption);
    free_generic_data(type, corrupted);
    free_generic_data(type, recovered);
    free_options(&opts);
    close_logging();

    return exit_code;
}
