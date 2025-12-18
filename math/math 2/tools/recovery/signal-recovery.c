/*
 * signal-recovery.c - Signal Data Recovery Tool
 * 
 * Production-grade CLI tool for recovering corrupted signal data.
 * Handles: Audio, Time Series, Frequency Domain, Multi-Channel, Oscillations
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
    printf("Signal Recovery Tool - Production-Grade Signal Data Recovery\n\n");
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Required Options:\n");
    printf("  -i, --input FILE       Input corrupted signal file (WAV, CSV, JSON)\n");
    printf("  -o, --output FILE      Output recovered signal file\n\n");
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
    printf("  -V, --visualize        Visualize signal (waveform)\n");
    printf("  -b, --benchmark        Run performance benchmark\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -i corrupted.wav -o recovered.wav\n", program_name);
    printf("  %s -i corrupted.csv -o recovered.csv -v -V\n", program_name);
    printf("  %s -i corrupted.json -o recovered.json -a phase4 -r report.txt\n\n", program_name);
    printf("Supported Formats:\n");
    printf("  Audio: WAV, FLAC, MP3\n");
    printf("  Time Series: CSV, JSON\n");
    printf("  Multi-Channel: WAV (stereo/surround), CSV (multi-column)\n\n");
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
static void generate_report(const char* filename, const SignalData* original,
                           const SignalData* recovered, const RecoveryResult* result) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        log_message(LOG_WARN, "Could not create report file: %s", filename);
        return;
    }

    fprintf(f, "Signal Recovery Report\n");
    fprintf(f, "======================\n\n");

    fprintf(f, "Input Signal:\n");
    fprintf(f, "  Samples: %zu\n", original->num_samples);
    fprintf(f, "  Channels: %zu\n", original->num_channels);
    fprintf(f, "  Sample Rate: %.0f Hz\n", original->sample_rate);
    fprintf(f, "  Duration: %.2f seconds\n", original->num_samples / original->sample_rate);
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

    fprintf(f, "Output Signal:\n");
    fprintf(f, "  Samples: %zu\n", recovered->num_samples);
    fprintf(f, "  Channels: %zu\n", recovered->num_channels);
    fprintf(f, "  Sample Rate: %.0f Hz\n", recovered->sample_rate);
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
    printf("║          Signal Recovery Tool - Production System              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    log_message(LOG_INFO, "Input: %s", opts.input_file);
    log_message(LOG_INFO, "Output: %s", opts.output_file);

    /* Load corrupted data */
    log_message(LOG_INFO, "Loading corrupted signal...");
    DataType type;
    FileFormat format;
    GenericData* corrupted = load_data(opts.input_file, &type, &format);

    if (!corrupted) {
        log_message(LOG_ERROR, "Failed to load input file: %s", get_error_message(get_last_error()));
        free_options(&opts);
        close_logging();
        return 1;
    }

    if (type != DATA_TYPE_SIGNAL) {
        log_message(LOG_ERROR, "Input file is not signal data (type: %s)", get_data_type_name(type));
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    log_message(LOG_INFO, "Loaded %s file: %zu samples, %zu channels, %.0f Hz",
               get_format_name(format),
               corrupted->signal.num_samples,
               corrupted->signal.num_channels,
               corrupted->signal.sample_rate);

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

    /* Visualize corrupted signal */
    if (opts.visualize) {
        char viz_buffer[2048];
        visualize_signal_ascii(&corrupted->signal, viz_buffer, sizeof(viz_buffer));
        printf("Corrupted Signal:\n%s\n\n", viz_buffer);
    }

    /* Recover data */
    log_message(LOG_INFO, "Recovering signal using %s...", get_algorithm_name(opts.algorithm));

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

        /* Visualize recovered signal */
        if (opts.visualize) {
            char viz_buffer[2048];
            visualize_signal_ascii(&recovered->signal, viz_buffer, sizeof(viz_buffer));
            printf("Recovered Signal:\n%s\n\n", viz_buffer);
        }

        /* Validate recovered signal */
        log_message(LOG_INFO, "Validating recovered signal...");
        ValidationResult* validation = validate_signal(&recovered->signal, &corrupted->signal);

        if (validation && validation->valid) {
            printf("✓ Signal validation: PASSED (Quality: %.2f%%)\n\n",
                   validation->quality_score * 100.0);
        } else {
            printf("✗ Signal validation: FAILED\n\n");
            log_message(LOG_WARN, "Validation failed, but continuing with export");
        }

        if (validation) free_validation_result(validation);

        /* Save recovered signal */
        log_message(LOG_INFO, "Saving recovered signal...");
        if (!save_data(opts.output_file, type, recovered, format)) {
            log_message(LOG_ERROR, "Failed to save output file: %s", get_error_message(get_last_error()));
            exit_code = 1;
        } else {
            log_message(LOG_INFO, "Recovered signal saved to: %s", opts.output_file);
        }

        /* Generate report if requested */
        if (opts.report_file) {
            generate_report(opts.report_file, &corrupted->signal, &recovered->signal, result);
        }

        /* Benchmark if requested */
        if (opts.benchmark) {
            double cpu_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            printf("┌─ Performance Benchmark ───────────────────────────────────────┐\n");
            printf("│ CPU Time: %.3f seconds                                         \n", cpu_time);
            printf("│ Wall Time: %.3f seconds                                        \n", result->time_seconds);
            printf("│ Samples Processed: %zu                                         \n", corrupted->signal.num_samples);
            printf("│ Processing Rate: %.0f samples/second                           \n",
                   corrupted->signal.num_samples / result->time_seconds);
            printf("│ Memory Used: %.2f MB                                           \n",
                   (corrupted->signal.num_samples * corrupted->signal.num_channels * sizeof(double)) / (1024.0 * 1024.0));
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