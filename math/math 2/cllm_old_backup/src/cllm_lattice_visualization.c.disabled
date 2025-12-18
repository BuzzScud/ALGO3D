#include "cllm_lattice_visualization.h"
#include "prime_math.h"
#include "prime_float_math.h"
#include <stdlib.h>
#include <string.h>

// ANSI color codes for visualization
static const char* SYMMETRY_COLORS[12] = {
    "\033[91m",  // Red
    "\033[92m",  // Green
    "\033[93m",  // Yellow
    "\033[94m",  // Blue
    "\033[95m",  // Magenta
    "\033[96m",  // Cyan
    "\033[91m",  // Light Red
    "\033[92m",  // Light Green
    "\033[93m",  // Light Yellow
    "\033[94m",  // Light Blue
    "\033[95m",  // Light Magenta
    "\033[96m"   // Light Cyan
};
static const char* COLOR_RESET = "\033[0m";

KissingSphereVisualizationConfig cllm_visualization_config_default(void) {
    KissingSphereVisualizationConfig config;
    config.show_neighbors = 1;
    config.show_statistics = 1;
    config.show_symmetry_groups = 1;
    config.show_cache_info = 1;
    config.show_load_balance = 1;
    config.verbose = 0;
    return config;
}

void cllm_print_point_info(
    const CLLMModel* model,
    int point_id,
    FILE* output
) {
    if (!model || !model->lattice_points || point_id < 0 || point_id >= (int)model->num_lattice_points) {
        fprintf(output, "Invalid point ID: %d\n", point_id);
        return;
    }

    int symmetry_group = cllm_get_point_symmetry_group(point_id);
    const char* color = cllm_get_symmetry_color(symmetry_group);
    CLLMLatticePoint* point = &model->lattice_points[point_id];

    fprintf(output, "\n%s=== Lattice Point %d ===%s\n", color, point_id, COLOR_RESET);
    fprintf(output, "Symmetry Group: %d (12-fold)\n", symmetry_group);
    fprintf(output, "Number of Neighbors: %u\n", point->num_neighbors);
    fprintf(output, "Point ID: %u\n", point->point_id);
    fprintf(output, "Prime: %lu\n", (unsigned long)point->prime);
}

void cllm_print_point_neighbors(
    const CLLMModel* model,
    int point_id,
    FILE* output
) {
    if (!model || !model->lattice_points || point_id < 0 || point_id >= (int)model->num_lattice_points) {
        return;
    }

    CLLMLatticePoint* point = &model->lattice_points[point_id];
    
    fprintf(output, "\nNeighbor Connections for Point %d:\n", point_id);
    fprintf(output, "%-10s %-15s %-15s\n", "Neighbor", "Symmetry", "Relation");
    fprintf(output, "----------------------------------------\n");

    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        int neighbor_symmetry = cllm_get_point_symmetry_group(neighbor_id);
        int point_symmetry = cllm_get_point_symmetry_group(point_id);
        
        const char* color = cllm_get_symmetry_color(neighbor_symmetry);
        
        int relation = (neighbor_symmetry - point_symmetry + 12) % 12;
        
        fprintf(output, "%s%-10u %-15d %-15d%s\n", 
                color, neighbor_id, neighbor_symmetry, relation, COLOR_RESET);
    }
}

LatticePointStatistics cllm_collect_point_statistics(
    const CLLMModel* model,
    int point_id
) {
    LatticePointStatistics stats;
    memset(&stats, 0, sizeof(LatticePointStatistics));
    
    if (!model || !model->lattice_points || point_id < 0 || point_id >= (int)model->num_lattice_points) {
        return stats;
    }

    CLLMLatticePoint* point = &model->lattice_points[point_id];
    
    stats.point_id = point_id;
    stats.num_neighbors = point->num_neighbors;
    stats.symmetry_group = cllm_get_point_symmetry_group(point_id);
    
    // Calculate average neighbor distance
    double total_distance = 0.0;
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        int distance = abs((int)neighbor_id - point_id);
        total_distance += (double)distance;
    }
    stats.avg_neighbor_distance = (point->num_neighbors > 0) ? 
        total_distance / point->num_neighbors : 0.0;
    
    // Estimate cache hit rate
    stats.cache_hit_rate = 1.0 / (1.0 + stats.avg_neighbor_distance / 10.0);
    stats.load_factor = 1.0;
    stats.tokens_processed = 0;
    
    return stats;
}

LatticePointStatistics* cllm_collect_all_point_statistics(
    const CLLMModel* model,
    int* num_stats
) {
    if (!model || !model->lattice_points || !num_stats) {
        if (num_stats) *num_stats = 0;
        return NULL;
    }

    *num_stats = model->num_lattice_points;
    LatticePointStatistics* stats = malloc(sizeof(LatticePointStatistics) * model->num_lattice_points);
    
    if (!stats) {
        *num_stats = 0;
        return NULL;
    }

    for (size_t i = 0; i < model->num_lattice_points; i++) {
        stats[i] = cllm_collect_point_statistics(model, i);
    }

    return stats;
}

void cllm_print_point_statistics(
    const LatticePointStatistics* stats,
    FILE* output
) {
    if (!stats) {
        return;
    }

    const char* color = cllm_get_symmetry_color(stats->symmetry_group);
    
    fprintf(output, "\n%s--- Point %d Statistics ---%s\n", 
            color, stats->point_id, COLOR_RESET);
    fprintf(output, "Symmetry Group: %d\n", stats->symmetry_group);
    fprintf(output, "Neighbors: %d\n", stats->num_neighbors);
    fprintf(output, "Avg Neighbor Distance: %.2f\n", stats->avg_neighbor_distance);
    fprintf(output, "Cache Hit Rate: %.2f%%\n", stats->cache_hit_rate * 100.0);
    fprintf(output, "Load Factor: %.2f\n", stats->load_factor);
    fprintf(output, "Tokens Processed: %zu\n", stats->tokens_processed);
}

SystemStatistics cllm_collect_system_statistics(
    const CLLMModel* model
) {
    SystemStatistics stats;
    memset(&stats, 0, sizeof(SystemStatistics));
    
    if (!model || !model->lattice_points) {
        return stats;
    }

    stats.total_points = model->num_lattice_points;
    
    // Count total connections and calculate distribution
    int total_neighbors = 0;
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        int group = cllm_get_point_symmetry_group(i);
        stats.symmetry_distribution[group] += 1.0;
        total_neighbors += model->lattice_points[i].num_neighbors;
    }
    
    stats.total_connections = total_neighbors;
    stats.avg_neighbors_per_point = (model->num_lattice_points > 0) ?
        (double)total_neighbors / model->num_lattice_points : 0.0;
    
    // Normalize distribution
    for (int i = 0; i < 12; i++) {
        stats.symmetry_distribution[i] /= model->num_lattice_points;
    }
    
    stats.load_balance_score = cllm_calculate_load_balance_score(model);
    stats.cache_efficiency = cllm_estimate_cache_efficiency(model);
    stats.total_tokens_processed = 0;
    
    return stats;
}

void cllm_print_system_statistics(
    const SystemStatistics* stats,
    FILE* output
) {
    if (!stats) {
        return;
    }

    fprintf(output, "\n=== SYSTEM STATISTICS ===\n");
    fprintf(output, "Total Points: %d\n", stats->total_points);
    fprintf(output, "Total Connections: %d\n", stats->total_connections);
    fprintf(output, "Avg Neighbors/Point: %.2f\n", stats->avg_neighbors_per_point);
    fprintf(output, "Load Balance Score: %.2f%%\n", stats->load_balance_score * 100.0);
    fprintf(output, "Cache Efficiency: %.2f%%\n", stats->cache_efficiency * 100.0);
    fprintf(output, "Total Tokens Processed: %zu\n", stats->total_tokens_processed);
}

void cllm_visualize_neighbor_connections(
    const CLLMModel* model,
    int point_id,
    FILE* output
) {
    if (!model || !model->lattice_points || point_id < 0 || point_id >= (int)model->num_lattice_points) {
        return;
    }

    CLLMLatticePoint* point = &model->lattice_points[point_id];
    
    fprintf(output, "\n=== Neighbor Connection Visualization ===\n");
    fprintf(output, "Center: Point %d\n\n", point_id);
    
    // ASCII art representation
    fprintf(output, "        [N0]  [N1]  [N2]\n");
    fprintf(output, "           \\   |   /\n");
    fprintf(output, "  [N11]---- [%3d] ----[N3]\n", point_id);
    fprintf(output, "           /   |   \\\n");
    fprintf(output, "       [N10] [N9]  [N4]\n");
    fprintf(output, "         |    |    |\n");
    fprintf(output, "       [N8] [N7]  [N5]\n");
    fprintf(output, "           \\  |  /\n");
    fprintf(output, "            [N6]\n\n");
    
    // List actual neighbor IDs
    fprintf(output, "Neighbor IDs:\n");
    for (uint32_t i = 0; i < point->num_neighbors && i < 12; i++) {
        uint32_t neighbor_id = point->neighbors[i];
        int symmetry = cllm_get_point_symmetry_group(neighbor_id);
        const char* color = cllm_get_symmetry_color(symmetry);
        
        fprintf(output, "  %sN%-2d: Point %-4u (Symmetry %2d)%s\n", 
                color, i, neighbor_id, symmetry, COLOR_RESET);
    }
}

void cllm_visualize_symmetry_distribution(
    const CLLMModel* model,
    FILE* output
) {
    if (!model || !model->lattice_points) {
        return;
    }

    fprintf(output, "\n=== 12-Fold Symmetry Distribution ===\n\n");
    
    // Count points in each symmetry group
    int counts[12] = {0};
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        int group = cllm_get_point_symmetry_group(i);
        counts[group]++;
    }
    
    // Find max count for scaling
    int max_count = 0;
    for (int i = 0; i < 12; i++) {
        if (counts[i] > max_count) {
            max_count = counts[i];
        }
    }
    
    // Print bar chart
    for (int i = 0; i < 12; i++) {
        const char* color = cllm_get_symmetry_color(i);
        fprintf(output, "%sGroup %2d: %s", color, i, COLOR_RESET);
        
        int bar_length = (max_count > 0) ? (counts[i] * 50 / max_count) : 0;
        for (int j = 0; j < bar_length; j++) {
            fprintf(output, "%s█%s", color, COLOR_RESET);
        }
        fprintf(output, " %d (%.1f%%)\n", counts[i], 
                100.0 * counts[i] / model->num_lattice_points);
    }
}

void cllm_visualize_load_balance(
    const CLLMModel* model,
    FILE* output
) {
    if (!model) {
        return;
    }

    fprintf(output, "\n=== Load Balance Visualization ===\n\n");
    fprintf(output, "Load Balance Score: %.2f%%\n", 
            cllm_calculate_load_balance_score(model) * 100.0);
}

void cllm_print_visualization_report(
    const CLLMModel* model,
    const KissingSphereVisualizationConfig* config,
    FILE* output
) {
    if (!model || !config) {
        return;
    }

    fprintf(output, "\n");
    fprintf(output, "╔════════════════════════════════════════════════════════════╗\n");
    fprintf(output, "║     KISSING SPHERES VISUALIZATION REPORT                  ║\n");
    fprintf(output, "╚════════════════════════════════════════════════════════════╝\n");
    
    if (config->show_statistics) {
        SystemStatistics sys_stats = cllm_collect_system_statistics(model);
        cllm_print_system_statistics(&sys_stats, output);
    }
    
    if (config->show_symmetry_groups) {
        cllm_visualize_symmetry_distribution(model, output);
    }
    
    if (config->show_load_balance) {
        cllm_visualize_load_balance(model, output);
    }
    
    if (config->show_neighbors && model->num_lattice_points > 0) {
        size_t points_to_show = (model->num_lattice_points < 5) ? model->num_lattice_points : 5;
        for (size_t i = 0; i < points_to_show; i++) {
            cllm_visualize_neighbor_connections(model, i, output);
        }
    }
    
    if (config->show_cache_info) {
        fprintf(output, "\n=== Cache Efficiency ===\n");
        fprintf(output, "Estimated Cache Efficiency: %.2f%%\n", 
                cllm_estimate_cache_efficiency(model) * 100.0);
    }
    
    fprintf(output, "\n");
}

void cllm_export_visualization_json(
    const CLLMModel* model,
    const char* filename
) {
    if (!model || !model->lattice_points || !filename) {
        return;
    }

    FILE* f = fopen(filename, "w");
    if (!f) {
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  &quot;num_points&quot;: %zu,\n", model->num_lattice_points);
    fprintf(f, "  &quot;points&quot;: [\n");
    
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        LatticePointStatistics stats = cllm_collect_point_statistics(model, i);
        
        fprintf(f, "    {\n");
        fprintf(f, "      &quot;id&quot;: %zu,\n", i);
        fprintf(f, "      &quot;point_id&quot;: %u,\n", point->point_id);
        fprintf(f, "      &quot;prime&quot;: %lu,\n", (unsigned long)point->prime);
        fprintf(f, "      &quot;symmetry_group&quot;: %d,\n", stats.symmetry_group);
        fprintf(f, "      &quot;neighbors&quot;: [");
        
        for (uint32_t j = 0; j < point->num_neighbors && j < 12; j++) {
            fprintf(f, "%u", point->neighbors[j]);
            if (j + 1 < point->num_neighbors && j < 11) {
                fprintf(f, ", ");
            }
        }
        
        fprintf(f, "],\n");
        fprintf(f, "      &quot;avg_neighbor_distance&quot;: %.2f,\n", stats.avg_neighbor_distance);
        fprintf(f, "      &quot;cache_hit_rate&quot;: %.2f\n", stats.cache_hit_rate);
        fprintf(f, "    }");
        
        if (i < model->num_lattice_points - 1) {
            fprintf(f, ",");
        }
        fprintf(f, "\n");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
}

int cllm_verify_lattice_structure(
    const CLLMModel* model,
    FILE* error_output
) {
    if (!model) {
        fprintf(error_output, "ERROR: NULL model pointer\n");
        return 0;
    }

    if (!model->lattice_points) {
        fprintf(error_output, "ERROR: NULL lattice_points\n");
        return 0;
    }

    int errors = 0;
    
    // Verify each point's neighbors
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        
        if (point->num_neighbors != 12) {
            fprintf(error_output, 
                    "WARNING: Point %zu has %d neighbors (expected 12)\n",
                    i, point->num_neighbors);
        }
        
        for (uint32_t j = 0; j < point->num_neighbors && j < 12; j++) {
            uint32_t neighbor = point->neighbors[j];
            
            if (neighbor >= model->num_lattice_points) {
                fprintf(error_output, 
                        "ERROR: Point %zu has invalid neighbor %u at position %d\n",
                        i, neighbor, j);
                errors++;
            }
        }
    }
    
    if (errors == 0) {
        fprintf(error_output, "✓ Lattice structure verification passed\n");
    } else {
        fprintf(error_output, "✗ Found %d errors in lattice structure\n", errors);
    }
    
    return errors == 0;
}

void cllm_debug_check_lattice(
    const CLLMModel* model,
    FILE* output
) {
    if (!model) {
        fprintf(output, "ERROR: NULL model pointer\n");
        return;
    }

    fprintf(output, "\n=== DEBUG: Lattice Structure Check ===\n\n");
    fprintf(output, "Number of Points: %zu\n", model->num_lattice_points);
    
    fprintf(output, "\nStructure Verification:\n");
    cllm_verify_lattice_structure(model, output);
    
    // Check symmetry distribution
    fprintf(output, "\nSymmetry Distribution Check:\n");
    int counts[12] = {0};
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        int group = cllm_get_point_symmetry_group(i);
        counts[group]++;
    }
    
    int min_count = counts[0];
    int max_count = counts[0];
    for (int i = 1; i < 12; i++) {
        if (counts[i] < min_count) min_count = counts[i];
        if (counts[i] > max_count) max_count = counts[i];
    }
    
    fprintf(output, "Min points in group: %d\n", min_count);
    fprintf(output, "Max points in group: %d\n", max_count);
    fprintf(output, "Balance ratio: %.2f\n", (max_count > 0) ? (double)min_count / max_count : 0.0);
    
    if (max_count - min_count <= 1) {
        fprintf(output, "✓ Symmetry distribution is well-balanced\n");
    } else {
        fprintf(output, "⚠ Symmetry distribution could be more balanced\n");
    }
}

void cllm_debug_print_memory_layout(
    const CLLMModel* model,
    FILE* output
) {
    if (!model) {
        return;
    }

    fprintf(output, "\n=== DEBUG: Memory Layout ===\n\n");
    fprintf(output, "CLLMModel structure:\n");
    fprintf(output, "  Address: %p\n", (void*)model);
    fprintf(output, "  Lattice points: %zu\n", model->num_lattice_points);
    fprintf(output, "  Lattice points array: %p\n", (void*)model->lattice_points);
}

double cllm_estimate_cache_efficiency(
    const CLLMModel* model
) {
    if (!model || !model->lattice_points || model->num_lattice_points == 0) {
        return 0.0;
    }

    double total_distance = 0.0;
    int count = 0;
    
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        CLLMLatticePoint* point = &model->lattice_points[i];
        for (uint32_t j = 0; j < point->num_neighbors && j < 12; j++) {
            uint32_t neighbor = point->neighbors[j];
            total_distance += abs((int)neighbor - (int)i);
            count++;
        }
    }
    
    if (count == 0) return 0.0;
    
    double avg_distance = total_distance / count;
    double efficiency = 1.0 / (1.0 + avg_distance / 16.0);
    
    return efficiency;
}

double cllm_calculate_load_balance_score(
    const CLLMModel* model
) {
    if (!model || !model->lattice_points || model->num_lattice_points == 0) {
        return 0.0;
    }

    int counts[12] = {0};
    for (size_t i = 0; i < model->num_lattice_points; i++) {
        int group = cllm_get_point_symmetry_group(i);
        counts[group]++;
    }
    
    double avg = (double)model->num_lattice_points / 12.0;
    double variance = 0.0;
    
    for (int i = 0; i < 12; i++) {
        double diff = counts[i] - avg;
        variance += diff * diff;
    }
    
    variance /= 12.0;
    double std_dev = prime_sqrt(variance);
    
    double score = (avg > 0) ? 1.0 / (1.0 + std_dev / avg) : 0.0;
    
    return score;
}

int cllm_get_point_symmetry_group(int point_id) {
    return point_id % 12;
}

const char* cllm_get_symmetry_color(int symmetry_group) {
    if (symmetry_group < 0 || symmetry_group >= 12) {
        return COLOR_RESET;
    }
    return SYMMETRY_COLORS[symmetry_group];
}