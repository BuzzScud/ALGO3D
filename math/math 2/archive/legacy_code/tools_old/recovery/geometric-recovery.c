#include "recovery_common/recovery_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/*
 * geometric-recovery - Geometric Data Recovery Tool
 * 
 * Recovers corrupted geometric data including:
 * - Platonic solids (tetrahedron, cube, octahedron, dodecahedron, icosahedron)
 * - Archimedean solids (13 types)
 * - Johnson solids (92 types)
 * - Geodesic spheres (all frequencies)
 * - 4D polytopes (tesseract, 120-cell, 600-cell, etc.)
 * 
 * Usage:
 *   geometric-recovery --input corrupted.obj --output recovered.obj
 *   geometric-recovery --input corrupted.json --validate euler,symmetry
 *   geometric-recovery --input corrupted.stl --algorithm phase4 --benchmark
 */

// ============================================================================
// COMMAND LINE OPTIONS
// ============================================================================

typedef struct {
    char* input_file;
    char* output_file;
    RecoveryAlgorithm algorithm;
    bool validate;
    bool visualize;
    bool benchmark;
    bool verbose;
    char* report_file;
} Options;

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Geometric Data Recovery Tool\n");
    printf("\n");
    printf("Options:\n");
    printf("  -i, --input FILE       Input file (corrupted geometry)\n");
    printf("  -o, --output FILE      Output file (recovered geometry)\n");
    printf("  -a, --algorithm ALG    Recovery algorithm (phase1-6, auto)\n");
    printf("  -v, --validate         Validate recovered geometry\n");
    printf("  -V, --visualize        Visualize geometry (ASCII art)\n");
    printf("  -b, --benchmark        Run benchmark\n");
    printf("  -r, --report FILE      Save report to file\n");
    printf("  -d, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help message\n");
    printf("\n");
    printf("Algorithms:\n");
    printf("  phase1    - Oscillation Analysis (< 5%% corruption)\n");
    printf("  phase2    - Symmetry Detection (< 10%% corruption)\n");
    printf("  phase3    - Topology Reconstruction (< 15%% corruption)\n");
    printf("  phase4    - Recursive Stabilization (< 20%% corruption)\n");
    printf("  phase5    - Dynamic Model Expansion (< 25%% corruption)\n");
    printf("  phase6    - Hyper-Dimensional Analysis (>= 25%% corruption)\n");
    printf("  auto      - Automatic selection (default)\n");
    printf("\n");
    printf("Supported Formats:\n");
    printf("  Input:  OBJ, STL, PLY, JSON\n");
    printf("  Output: OBJ, STL, PLY, JSON\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -i corrupted.obj -o recovered.obj\n", program_name);
    printf("  %s -i corrupted.stl -o recovered.stl -v -V\n", program_name);
    printf("  %s -i corrupted.json -a phase4 -b -r report.txt\n", program_name);
    printf("\n");
}

static RecoveryAlgorithm parse_algorithm(const char* str) {
    if (strcmp(str, "phase1") == 0) return RECOVERY_PHASE1;
    if (strcmp(str, "phase2") == 0) return RECOVERY_PHASE2;
    if (strcmp(str, "phase3") == 0) return RECOVERY_PHASE3;
    if (strcmp(str, "phase4") == 0) return RECOVERY_PHASE4;
    if (strcmp(str, "phase5") == 0) return RECOVERY_PHASE5;
    if (strcmp(str, "phase6") == 0) return RECOVERY_PHASE6;
    if (strcmp(str, "auto") == 0) return RECOVERY_AUTO;
    return RECOVERY_AUTO;
}

static bool parse_options(int argc, char** argv, Options* opts) {
    // Initialize options
    memset(opts, 0, sizeof(Options));
    opts->algorithm = RECOVERY_AUTO;
    
    static struct option long_options[] = {
        {"input",      required_argument, 0, 'i'},
        {"output",     required_argument, 0, 'o'},
        {"algorithm",  required_argument, 0, 'a'},
        {"validate",   no_argument,       0, 'v'},
        {"visualize",  no_argument,       0, 'V'},
        {"benchmark",  no_argument,       0, 'b'},
        {"report",     required_argument, 0, 'r'},
        {"verbose",    no_argument,       0, 'd'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "i:o:a:vVbr:dh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                opts->input_file = strdup(optarg);
                break;
            case 'o':
                opts->output_file = strdup(optarg);
                break;
            case 'a':
                opts->algorithm = parse_algorithm(optarg);
                break;
            case 'v':
                opts->validate = true;
                break;
            case 'V':
                opts->visualize = true;
                break;
            case 'b':
                opts->benchmark = true;
                break;
            case 'r':
                opts->report_file = strdup(optarg);
                break;
            case 'd':
                opts->verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return false;
            default:
                print_usage(argv[0]);
                return false;
        }
    }
    
    // Validate required options
    if (!opts->input_file) {
        fprintf(stderr, "Error: Input file is required\n");
        print_usage(argv[0]);
        return false;
    }
    
    return true;
}

static void free_options(Options* opts) {
    if (opts->input_file) free(opts->input_file);
    if (opts->output_file) free(opts->output_file);
    if (opts->report_file) free(opts->report_file);
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main(int argc, char** argv) {
    Options opts;
    int exit_code = 0;
    
    // Parse command line options first
    if (!parse_options(argc, argv, &opts)) {
        return 1;
    }
    
    // Initialize logging
    init_logging(NULL, opts.verbose ? LOG_DEBUG : LOG_INFO);
    
    log_message(LOG_INFO, "Geometric Recovery Tool");
    log_message(LOG_INFO, "Input: %s", opts.input_file);
    
    // Load corrupted data
    log_message(LOG_INFO, "Loading corrupted geometry...");
    DataType type;
    FileFormat format;
    GenericData* corrupted = load_data(opts.input_file, &type, &format);
    
    if (!corrupted) {
        log_message(LOG_ERROR, "Failed to load input file: %s", get_error_message(get_last_error()));
        free_options(&opts);
        close_logging();
        return 1;
    }
    
    if (type != DATA_TYPE_GEOMETRIC) {
        log_message(LOG_ERROR, "Input file is not geometric data (type: %s)", get_data_type_name(type));
        free_generic_data(type, corrupted);
        free_options(&opts);
        close_logging();
        return 1;
    }
    
    log_message(LOG_INFO, "Loaded %s file: V=%zu, E=%zu, F=%zu",
               get_format_name(format),
               corrupted->geometric.num_vertices,
               corrupted->geometric.num_edges,
               corrupted->geometric.num_faces);
    
    // Detect corruption
    log_message(LOG_INFO, "Detecting corruption...");
    CorruptionReport* corruption = detect_data_corruption(type, corrupted);
    
    if (corruption) {
        log_message(LOG_INFO, "Corruption detected: %s", corruption->description);
        log_message(LOG_INFO, "Severity: %.1f%%, Type: %s",
                   corruption->severity * 100.0,
                   get_corruption_type_name(corruption->type));
    }
    
    // Visualize corrupted geometry
    if (opts.visualize) {
        char viz_buffer[2048];
        visualize_geometric_ascii(&corrupted->geometric, viz_buffer, sizeof(viz_buffer));
        printf("\nCorrupted Geometry:\n%s\n", viz_buffer);
    }
    
    // Recover data
    log_message(LOG_INFO, "Recovering geometry using %s...", get_algorithm_name(opts.algorithm));
    
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
        log_message(LOG_INFO, "Quality: %.1f%%, RMSE: %.6f, Time: %.3fs",
                   result->quality * 100.0, result->rmse, result->time_seconds);
        
        // Visualize recovered geometry
        if (opts.visualize) {
            char viz_buffer[2048];
            visualize_geometric_ascii(&recovered->geometric, viz_buffer, sizeof(viz_buffer));
            printf("\nRecovered Geometry:\n%s\n", viz_buffer);
        }
        
        // Validate recovered geometry
        if (opts.validate) {
            log_message(LOG_INFO, "Validating recovered geometry...");
            ValidationResult* validation = validate_geometric(&recovered->geometric);
            
            if (validation) {
                log_message(LOG_INFO, "Validation: %s", validation->report);
                log_message(LOG_INFO, "Quality Score: %.1f%%", validation->quality_score * 100.0);
                free_validation_result(validation);
            }
        }
        
        // Save recovered data
        if (opts.output_file) {
            log_message(LOG_INFO, "Saving recovered geometry to %s...", opts.output_file);
            
            if (save_data(opts.output_file, type, recovered, format)) {
                log_message(LOG_INFO, "Saved successfully");
            } else {
                log_message(LOG_ERROR, "Failed to save: %s", get_error_message(get_last_error()));
                exit_code = 1;
            }
        }
        
        // Save report
        if (opts.report_file) {
            FILE* report = fopen(opts.report_file, "w");
            if (report) {
                fprintf(report, "Geometric Recovery Report\n");
                fprintf(report, "=========================\n\n");
                fprintf(report, "Input: %s\n", opts.input_file);
                fprintf(report, "Output: %s\n", opts.output_file ? opts.output_file : "N/A");
                fprintf(report, "Format: %s\n", get_format_name(format));
                fprintf(report, "\nCorruption:\n");
                if (corruption) {
                    fprintf(report, "  Type: %s\n", get_corruption_type_name(corruption->type));
                    fprintf(report, "  Severity: %.1f%%\n", corruption->severity * 100.0);
                    fprintf(report, "  Description: %s\n", corruption->description);
                }
                fprintf(report, "\nRecovery:\n");
                fprintf(report, "  Algorithm: %s\n", get_algorithm_name(result->algorithm));
                fprintf(report, "  Quality: %.1f%%\n", result->quality * 100.0);
                fprintf(report, "  RMSE: %.6f\n", result->rmse);
                fprintf(report, "  Max Error: %.6f\n", result->max_error);
                fprintf(report, "  Iterations: %zu\n", result->iterations);
                fprintf(report, "  Time: %.3f seconds\n", result->time_seconds);
                fclose(report);
                log_message(LOG_INFO, "Report saved to %s", opts.report_file);
            }
        }
    }
    
    // Cleanup
    if (result) free_recovery_result(result);
    if (corruption) free_corruption_report(corruption);
    free_generic_data(type, recovered);
    free_generic_data(type, corrupted);
    free_options(&opts);
    close_logging();
    
    return exit_code;
}
