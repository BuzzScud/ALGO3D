/*
 * ml-recovery.c - Machine Learning Data Recovery Tool
 * 
 * Production-grade CLI tool for recovering corrupted ML data.
 * Handles: NN Weights, Embeddings, Checkpoints, Datasets, Architecture
 * 
 * Part of OBJECTIVE 29: Universal Recovery Toolkit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "../../algorithms/include/recovery_common/recovery_common.h"

typedef struct {
    char* input_file;
    char* output_file;
    char* report_file;
    RecoveryAlgorithm algorithm;
    int verbose;
    int visualize;
    int benchmark;
} Options;

static void init_options(Options* opts) {
    memset(opts, 0, sizeof(Options));
    opts->algorithm = RECOVERY_AUTO;
}

static void free_options(Options* opts) {
    if (opts->input_file) free(opts->input_file);
    if (opts->output_file) free(opts->output_file);
    if (opts->report_file) free(opts->report_file);
}

static void print_usage(const char *program_name) {
    printf("ML Recovery Tool - Production-Grade Machine Learning Data Recovery\n\n");
    printf("Usage: %s -i INPUT -o OUTPUT [OPTIONS]\n\n", program_name);
    printf("Required:\n");
    printf("  -i, --input FILE       Input corrupted ML file (PT, H5, NPY, ONNX, JSON)\n");
    printf("  -o, --output FILE      Output recovered ML file\n\n");
    printf("Optional:\n");
    printf("  -a, --algorithm ALG    Recovery algorithm (auto, phase1-6)\n");
    printf("  -r, --report FILE      Generate recovery report\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -V, --visualize        Visualize weights/embeddings\n");
    printf("  -b, --benchmark        Run performance benchmark\n");
    printf("  -h, --help             Show this help\n\n");
}

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

static int parse_options(int argc, char** argv, Options* opts) {
    int c;
    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"algorithm", required_argument, 0, 'a'},
        {"report", required_argument, 0, 'r'},
        {"verbose", no_argument, 0, 'v'},
        {"visualize", no_argument, 0, 'V'},
        {"benchmark", no_argument, 0, 'b'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    init_options(opts);
    while ((c = getopt_long(argc, argv, "i:o:a:r:vVbh", long_options, NULL)) != -1) {
        switch (c) {
            case 'i': opts->input_file = strdup(optarg); break;
            case 'o': opts->output_file = strdup(optarg); break;
            case 'a': opts->algorithm = parse_algorithm(optarg); break;
            case 'r': opts->report_file = strdup(optarg); break;
            case 'v': opts->verbose = 1; break;
            case 'V': opts->visualize = 1; break;
            case 'b': opts->benchmark = 1; break;
            case 'h': print_usage(argv[0]); exit(0);
            default: return 0;
        }
    }
    if (!opts->input_file || !opts->output_file) {
        fprintf(stderr, "Error: Both input and output files required\n\n");
        print_usage(argv[0]);
        return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    Options opts;
    if (!parse_options(argc, argv, &opts)) return 1;

    init_logging(NULL, opts.verbose ? LOG_DEBUG : LOG_INFO);

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║          ML Recovery Tool - Production System                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    DataType type;
    FileFormat format;
    GenericData* corrupted = load_data(opts.input_file, &type, &format);

    if (!corrupted || type != DATA_TYPE_ML) {
        log_message(LOG_ERROR, "Failed to load ML data");
        if (corrupted) free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    log_message(LOG_INFO, "Loaded ML data");

    GenericData* recovered = (GenericData*)calloc(1, sizeof(GenericData));
    RecoveryResult* result = recover_data(type, corrupted, recovered, opts.algorithm);

    if (result && result->success) {
        log_message(LOG_INFO, "Recovery successful! Quality: %.2f%%", result->quality * 100.0);
        save_data(opts.output_file, type, recovered, format);
        printf("✓ Recovery Complete!\n");
    } else {
        log_message(LOG_ERROR, "Recovery failed");
    }

    if (result) free(result);
    free_generic_data(type, corrupted);
    free_generic_data(type, recovered);
    free_options(&opts);
    close_logging();
    return 0;
}