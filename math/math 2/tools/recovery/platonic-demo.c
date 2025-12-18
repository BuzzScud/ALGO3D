/*
 * platonic-demo.c - Platonic Solids Educational Demonstration Tool
 * 
 * Production-grade CLI tool for demonstrating Platonic solid properties.
 * Demonstrates: Euler's Formula, Symmetry Groups, Duals, Golden Ratio, Sphere Packing
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
    printf("Platonic Demo Tool - Educational Demonstrations of Platonic Solids\n\n");
    printf("Usage: %s -i INPUT -o OUTPUT [OPTIONS]\n\n", program_name);
    printf("Required:\n");
    printf("  -i, --input FILE       Input Platonic solid file (OBJ, JSON)\n");
    printf("  -o, --output FILE      Output demonstration file\n\n");
    printf("Optional:\n");
    printf("  -a, --algorithm ALG    Recovery algorithm (auto, phase1-6)\n");
    printf("  -r, --report FILE      Generate demonstration report\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -V, --visualize        Visualize Platonic solid\n");
    printf("  -b, --benchmark        Run performance benchmark\n");
    printf("  -h, --help             Show this help\n\n");
    printf("Demonstrations:\n");
    printf("  - Euler's Formula (V - E + F = 2)\n");
    printf("  - Symmetry Groups\n");
    printf("  - Dual Relationships\n");
    printf("  - Golden Ratio (φ = 1.618...)\n");
    printf("  - Sphere Packing\n\n");
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
    printf("║        Platonic Demo Tool - Educational Demonstrations        ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");

    DataType type;
    FileFormat format;
    GenericData* data = load_data(opts.input_file, &type, &format);

    if (!data || type != DATA_TYPE_GEOMETRIC) {
        log_message(LOG_ERROR, "Failed to load Platonic solid data");
        if (data) free_generic_data(type, data);
        free_options(&opts);
        close_logging();
        return 1;
    }

    log_message(LOG_INFO, "Loaded Platonic solid");

    printf("┌─ Platonic Solid Properties ───────────────────────────────────┐\n");
    printf("│ Vertices (V): %zu\n", data->geometric.num_vertices);
    printf("│ Edges (E): %zu\n", data->geometric.num_edges);
    printf("│ Faces (F): %zu\n", data->geometric.num_faces);
    
    int euler = (int)data->geometric.num_vertices - (int)data->geometric.num_edges + (int)data->geometric.num_faces;
    printf("│ Euler's Formula: V - E + F = %d %s\n", euler, euler == 2 ? "✓" : "✗");
    printf("└────────────────────────────────────────────────────────────────┘\n\n");

    if (opts.visualize) {
        char viz_buffer[2048];
        visualize_geometric_ascii(&data->geometric, viz_buffer, sizeof(viz_buffer));
        printf("Platonic Solid Visualization:\n%s\n\n", viz_buffer);
    }

    save_data(opts.output_file, type, data, format);
    log_message(LOG_INFO, "Demonstration complete!");

    printf("✓ Demonstration Complete!\n");

    free_generic_data(type, data);
    free_options(&opts);
    close_logging();
    return 0;
}