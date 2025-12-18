/*
 * network-recovery.c - Network Topology Recovery Tool
 * 
 * Production-grade CLI tool for recovering corrupted network data.
 * Handles: Social Networks, Computer Networks, Biological Networks, Transportation, Graphs
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

static void init_options(Options* opts) {
    opts->input_file = NULL;
    opts->output_file = NULL;
    opts->report_file = NULL;
    opts->algorithm = RECOVERY_AUTO;
    opts->verbose = 0;
    opts->visualize = 0;
    opts->benchmark = 0;
}

static void free_options(Options* opts) {
    if (opts->input_file) free(opts->input_file);
    if (opts->output_file) free(opts->output_file);
    if (opts->report_file) free(opts->report_file);
}

static void print_usage(const char *program_name) {
    printf("Network Recovery Tool - Production-Grade Network Topology Recovery\n\n");
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Required Options:\n");
    printf("  -i, --input FILE       Input corrupted network file (GML, GraphML, JSON)\n");
    printf("  -o, --output FILE      Output recovered network file\n\n");
    printf("Optional Options:\n");
    printf("  -a, --algorithm ALG    Recovery algorithm (auto, phase1-6)\n");
    printf("  -r, --report FILE      Generate recovery report\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -V, --visualize        Visualize network graph\n");
    printf("  -b, --benchmark        Run performance benchmark\n");
    printf("  -h, --help             Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -i corrupted.gml -o recovered.gml\n", program_name);
    printf("  %s -i corrupted.json -o recovered.json -v -V\n\n", program_name);
    printf("Supported Formats:\n");
    printf("  Network: GML, GraphML, JSON\n\n");
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
        fprintf(stderr, "Error: Both input and output files are required\n\n");
        print_usage(argv[0]);
        return 0;
    }
    return 1;
}

static void generate_report(const char* filename, const NetworkData* original,
                           const NetworkData* recovered, const RecoveryResult* result) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        log_message(LOG_WARN, "Could not create report file: %s", filename);
        return;
    }

    fprintf(f, "Network Recovery Report\n");
    fprintf(f, "=======================\n\n");
    fprintf(f, "Input Network:\n");
    fprintf(f, "  Nodes: %zu\n", original->num_nodes);
    fprintf(f, "  Edges: %zu\n", original->num_edges);
    fprintf(f, "\n");
    fprintf(f, "Recovery Algorithm: %s\n", get_algorithm_name(result->algorithm));
    fprintf(f, "\n");
    fprintf(f, "Recovery Metrics:\n");
    fprintf(f, "  Quality: %.2f%%\n", result->quality * 100.0);
    fprintf(f, "  RMSE: %.6f\n", result->rmse);
    fprintf(f, "  Recovery Time: %.3f seconds\n", result->time_seconds);
    fprintf(f, "  Status: %s\n", result->success ? "SUCCESS" : "FAILED");
    fprintf(f, "\n");
    fprintf(f, "Output Network:\n");
    fprintf(f, "  Nodes: %zu\n", recovered->num_nodes);
    fprintf(f, "  Edges: %zu\n", recovered->num_edges);
    fprintf(f, "\n");

    fclose(f);
    log_message(LOG_INFO, "Report saved to: %s", filename);
}

int main(int argc, char** argv) {
    Options opts;
    int exit_code = 0;

    if (!parse_options(argc, argv, &opts)) return 1;

    init_logging(NULL, opts.verbose ? LOG_DEBUG : LOG_INFO);

    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║         Network Recovery Tool - Production System              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    log_message(LOG_INFO, "Input: %s", opts.input_file);

    DataType type;
    FileFormat format;
    GenericData* corrupted = load_data(opts.input_file, &type, &format);

    if (!corrupted) {
        log_message(LOG_ERROR, "Failed to load input file: %s", get_error_message(get_last_error()));
        free_options(&opts);
        close_logging();
        return 1;
    }

    if (type != DATA_TYPE_NETWORK) {
        log_message(LOG_ERROR, "Input file is not network data (type: %s)", get_data_type_name(type));
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    log_message(LOG_INFO, "Loaded network: %zu nodes, %zu edges",
               corrupted->network.num_nodes, corrupted->network.num_edges);

    CorruptionReport* corruption = detect_data_corruption(type, corrupted);
    if (corruption) {
        printf("\n┌─ Corruption Analysis ─────────────────────────────────────────┐\n");
        printf("│ Severity: %.1f%%\n", corruption->severity * 100.0);
        printf("│ Type: %s\n", get_corruption_type_name(corruption->type));
        printf("└────────────────────────────────────────────────────────────────┘\n\n");
    }

    if (opts.visualize) {
        char viz_buffer[4096];
        visualize_network_ascii(&corrupted->network, viz_buffer, sizeof(viz_buffer));
        printf("Corrupted Network:\n%s\n\n", viz_buffer);
    }

    log_message(LOG_INFO, "Recovering network...");
    GenericData* recovered = (GenericData*)calloc(1, sizeof(GenericData));
    if (!recovered) {
        log_message(LOG_ERROR, "Out of memory");
        if (corruption) free_corruption_report(corruption);
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }

    RecoveryResult* result = recover_data(type, corrupted, recovered, opts.algorithm);

    if (!result || !result->success) {
        log_message(LOG_ERROR, "Recovery failed: %s", result ? result->message : "Unknown error");
        exit_code = 1;
    } else {
        log_message(LOG_INFO, "Recovery successful!");
        printf("┌─ Recovery Results ────────────────────────────────────────────┐\n");
        printf("│ Quality: %.2f%%\n", result->quality * 100.0);
        printf("│ Recovery Time: %.3f seconds\n", result->time_seconds);
        printf("└────────────────────────────────────────────────────────────────┘\n\n");

        if (opts.visualize) {
            char viz_buffer[4096];
            visualize_network_ascii(&recovered->network, viz_buffer, sizeof(viz_buffer));
            printf("Recovered Network:\n%s\n\n", viz_buffer);
        }

        ValidationResult* validation = validate_network(&recovered->network);
        if (validation && validation->valid) {
            printf("✓ Network validation: PASSED\n\n");
        } else {
            printf("✗ Network validation: FAILED\n\n");
        }
        if (validation) free_validation_result(validation);

        if (!save_data(opts.output_file, type, recovered, format)) {
            log_message(LOG_ERROR, "Failed to save output file");
            exit_code = 1;
        } else {
            log_message(LOG_INFO, "Recovered network saved to: %s", opts.output_file);
        }

        if (opts.report_file) {
            generate_report(opts.report_file, &corrupted->network, &recovered->network, result);
        }
    }

    if (exit_code == 0) {
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║                    Recovery Complete!                          ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
    }

    if (result) free(result);
    if (corruption) free_corruption_report(corruption);
    free_generic_data(type, corrupted);
    free_generic_data(type, recovered);
    free_options(&opts);
    close_logging();

    return exit_code;
}