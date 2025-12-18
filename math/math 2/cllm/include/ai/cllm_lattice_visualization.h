#ifndef CLLM_KISSING_SPHERES_VISUALIZATION_H
#define CLLM_KISSING_SPHERES_VISUALIZATION_H

#include "cllm.h"
#include <stdio.h>
#include <stddef.h>

/**
 * Kissing Spheres Visualization and Debugging Tools
 * 
 * Provides comprehensive visualization and statistics for the kissing spheres
 * architecture, including neighbor connections, sphere statistics, and debugging
 * information.
 */

// Visualization configuration
typedef struct {
    int show_neighbors;           // Show neighbor connections
    int show_statistics;          // Show sphere statistics
    int show_symmetry_groups;     // Show symmetry group distribution
    int show_cache_info;          // Show cache locality information
    int show_load_balance;        // Show load balancing metrics
    int verbose;                  // Verbose output
} KissingSphereVisualizationConfig;

// Lattice point statistics (sphere statistics)
typedef struct {
    int point_id;
    int num_neighbors;
    int symmetry_group;           // 0-11 (12-fold symmetry)
    size_t tokens_processed;
    double avg_neighbor_distance; // Average distance to neighbors
    double cache_hit_rate;        // Estimated cache hit rate
    double load_factor;           // Relative load compared to average
} LatticePointStatistics;

// Neighbor connection information
typedef struct {
    int source_point;
    int target_point;
    double distance;              // Geometric distance
    double weight;                // Connection weight
    int symmetry_relation;        // Symmetry group relationship
} NeighborConnection;

// Overall system statistics
typedef struct {
    int total_points;
    int total_connections;
    double avg_neighbors_per_point;
    double load_balance_score;    // 0.0 (poor) to 1.0 (perfect)
    double cache_efficiency;      // Estimated cache efficiency
    size_t total_tokens_processed;
    double symmetry_distribution[12]; // Distribution across 12 groups
} SystemStatistics;

/**
 * Initialize visualization configuration with defaults
 */
KissingSphereVisualizationConfig cllm_visualization_config_default(void);

/**
 * Print detailed information about a single lattice point
 */
void cllm_print_point_info(
    const CLLMModel* model,
    int point_id,
    FILE* output
);

/**
 * Print all neighbor connections for a lattice point
 */
void cllm_print_point_neighbors(
    const CLLMModel* model,
    int point_id,
    FILE* output
);

/**
 * Print statistics for a single lattice point
 */
void cllm_print_point_statistics(
    const LatticePointStatistics* stats,
    FILE* output
);

/**
 * Collect statistics for a single lattice point
 */
LatticePointStatistics cllm_collect_point_statistics(
    const CLLMModel* model,
    int point_id
);

/**
 * Collect statistics for all lattice points
 */
LatticePointStatistics* cllm_collect_all_point_statistics(
    const CLLMModel* model,
    int* num_stats
);

/**
 * Print system-wide statistics
 */
void cllm_print_system_statistics(
    const SystemStatistics* stats,
    FILE* output
);

/**
 * Collect system-wide statistics
 */
SystemStatistics cllm_collect_system_statistics(
    const CLLMModel* model
);

/**
 * Visualize neighbor connections as ASCII art
 */
void cllm_visualize_neighbor_connections(
    const CLLMModel* model,
    int point_id,
    FILE* output
);

/**
 * Visualize symmetry group distribution
 */
void cllm_visualize_symmetry_distribution(
    const CLLMModel* model,
    FILE* output
);

/**
 * Visualize load balancing across lattice points
 */
void cllm_visualize_load_balance(
    const CLLMModel* model,
    FILE* output
);

/**
 * Print comprehensive visualization report
 */
void cllm_print_visualization_report(
    const CLLMModel* model,
    const KissingSphereVisualizationConfig* config,
    FILE* output
);

/**
 * Export visualization data to JSON format
 */
void cllm_export_visualization_json(
    const CLLMModel* model,
    const char* filename
);

/**
 * Debugging: Verify lattice structure integrity
 */
int cllm_verify_lattice_structure(
    const CLLMModel* model,
    FILE* error_output
);

/**
 * Debugging: Check for common issues
 */
void cllm_debug_check_lattice(
    const CLLMModel* model,
    FILE* output
);

/**
 * Debugging: Print memory layout information
 */
void cllm_debug_print_memory_layout(
    const CLLMModel* model,
    FILE* output
);

/**
 * Performance: Estimate cache efficiency
 */
double cllm_estimate_cache_efficiency(
    const CLLMModel* model
);

/**
 * Performance: Calculate load balance score
 */
double cllm_calculate_load_balance_score(
    const CLLMModel* model
);

/**
 * Utility: Get symmetry group for a lattice point
 */
int cllm_get_point_symmetry_group(int point_id);

/**
 * Utility: Get color code for visualization (ANSI)
 */
const char* cllm_get_symmetry_color(int symmetry_group);

#endif // CLLM_KISSING_SPHERES_VISUALIZATION_H