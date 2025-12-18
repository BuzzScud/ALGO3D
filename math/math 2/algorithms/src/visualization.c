/**
 * Unified Visualization Framework - Core Implementation
 * 
 * Provides generic visualization infrastructure for 2D, 3D, and Crystalline modes.
 */

#include "visualization.h"
#include "math/transcendental.h"
// #include "../../include/clock_lattice.h"  // Legacy - not needed
#include <stdlib.h>
#include <string.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

VisualizationData* viz_create(VisualizationMode mode, uint32_t num_points) {
    if (num_points == 0) {
        fprintf(stderr, "ERROR: num_points must be > 0\n");
        return NULL;
    }
    
    VisualizationData* data = calloc(1, sizeof(VisualizationData));
    if (!data) {
        fprintf(stderr, "ERROR: Failed to allocate visualization data\n");
        return NULL;
    }
    
    data->mode = mode;
    data->num_points = num_points;
    
    // Allocate points
    data->points = calloc(num_points, sizeof(VisualizationPoint));
    if (!data->points) {
        fprintf(stderr, "ERROR: Failed to allocate points\n");
        free(data);
        return NULL;
    }
    
    // Initialize points
    for (uint32_t i = 0; i < num_points; i++) {
        data->points[i].id = i;
        data->points[i].symmetry_group = i % 12;
        data->points[i].num_neighbors = 0;
        memset(data->points[i].position, 0, sizeof(data->points[i].position));
        memset(data->points[i].neighbors, 0, sizeof(data->points[i].neighbors));
        data->points[i].label[0] = '\0';
    }
    
    // Initialize bounds
    for (int i = 0; i < VIZ_MAX_DIMENSIONS; i++) {
        data->min_bounds[i] = 0.0;
        data->max_bounds[i] = 1.0;
    }
    
    // Initialize statistics
    data->avg_value = 0.0;
    data->min_value = 0.0;
    data->max_value = 1.0;
    
    memset(data->symmetry_counts, 0, sizeof(data->symmetry_counts));
    
    data->title[0] = '\0';
    data->description[0] = '\0';
    
    return data;
}

void viz_free(VisualizationData* data) {
    if (!data) return;
    
    if (data->points) {
        free(data->points);
    }
    
    free(data);
}

// ============================================================================
// DATA POPULATION
// ============================================================================

int viz_set_point_position(
    VisualizationData* data,
    uint32_t point_id,
    const double* position
) {
    if (!data || !position) return -1;
    if (point_id >= data->num_points) return -1;
    
    int dims = (data->mode == VIZ_MODE_2D) ? 2 : 3;
    memcpy(data->points[point_id].position, position, dims * sizeof(double));
    
    return 0;
}

int viz_set_point_value(
    VisualizationData* data,
    uint32_t point_id,
    double value
) {
    if (!data) return -1;
    if (point_id >= data->num_points) return -1;
    
    data->points[point_id].value = value;
    
    return 0;
}

int viz_set_point_neighbors(
    VisualizationData* data,
    uint32_t point_id,
    const uint32_t* neighbors,
    uint32_t num_neighbors
) {
    if (!data || !neighbors) return -1;
    if (point_id >= data->num_points) return -1;
    if (num_neighbors > VIZ_MAX_CONNECTIONS) return -1;
    
    VisualizationPoint* point = &data->points[point_id];
    memcpy(point->neighbors, neighbors, num_neighbors * sizeof(uint32_t));
    point->num_neighbors = num_neighbors;
    
    return 0;
}

int viz_set_point_label(
    VisualizationData* data,
    uint32_t point_id,
    const char* label
) {
    if (!data || !label) return -1;
    if (point_id >= data->num_points) return -1;
    
    strncpy(data->points[point_id].label, label, 63);
    data->points[point_id].label[63] = '\0';
    
    return 0;
}

// ============================================================================
// STATISTICS
// ============================================================================

int viz_calculate_statistics(
    const VisualizationData* data,
    VisualizationStatistics* stats
) {
    if (!data || !stats) return -1;
    
    memset(stats, 0, sizeof(VisualizationStatistics));
    
    stats->num_points = data->num_points;
    
    // Count connections and calculate distribution
    uint32_t total_neighbors = 0;
    for (uint32_t i = 0; i < data->num_points; i++) {
        const VisualizationPoint* point = &data->points[i];
        total_neighbors += point->num_neighbors;
        stats->symmetry_distribution[point->symmetry_group] += 1.0;
    }
    
    stats->num_connections = total_neighbors;
    stats->avg_neighbors = (data->num_points > 0) ?
        (double)total_neighbors / data->num_points : 0.0;
    
    // Normalize symmetry distribution
    for (int i = 0; i < 12; i++) {
        stats->symmetry_distribution[i] /= data->num_points;
    }
    
    // Calculate load balance score
    double avg = (double)data->num_points / 12.0;
    double variance = 0.0;
    
    for (int i = 0; i < 12; i++) {
        double count = stats->symmetry_distribution[i] * data->num_points;
        double diff = count - avg;
        variance += diff * diff;
    }
    
    variance /= 12.0;
    double std_dev = math_sqrt(variance);
    stats->load_balance_score = (avg > 0) ? 1.0 / (1.0 + std_dev / avg) : 1.0;
    
    // Calculate spatial extent
    int dims = (data->mode == VIZ_MODE_2D) ? 2 : 3;
    for (int d = 0; d < dims; d++) {
        stats->spatial_extent[d] = data->max_bounds[d] - data->min_bounds[d];
    }
    
    return 0;
}

int viz_update_bounds(VisualizationData* data) {
    if (!data || data->num_points == 0) return -1;
    
    int dims = (data->mode == VIZ_MODE_2D) ? 2 : 3;
    
    // Initialize with first point
    for (int d = 0; d < dims; d++) {
        data->min_bounds[d] = data->points[0].position[d];
        data->max_bounds[d] = data->points[0].position[d];
    }
    
    // Find min/max
    for (uint32_t i = 1; i < data->num_points; i++) {
        for (int d = 0; d < dims; d++) {
            double pos = data->points[i].position[d];
            if (pos < data->min_bounds[d]) data->min_bounds[d] = pos;
            if (pos > data->max_bounds[d]) data->max_bounds[d] = pos;
        }
    }
    
    return 0;
}

int viz_calculate_symmetry_distribution(VisualizationData* data) {
    if (!data) return -1;
    
    memset(data->symmetry_counts, 0, sizeof(data->symmetry_counts));
    
    for (uint32_t i = 0; i < data->num_points; i++) {
        uint32_t group = data->points[i].symmetry_group;
        if (group < 12) {
            data->symmetry_counts[group]++;
        }
    }
    
    return 0;
}

// ============================================================================
// EXPORT
// ============================================================================

int viz_export_json(const VisualizationData* data, const char* filename) {
    if (!data || !filename) return -1;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "ERROR: Failed to open %s for writing\n", filename);
        return -1;
    }
    
    fprintf(f, "{\n");
    fprintf(f, "  &quot;mode&quot;: &quot;%s&quot;,\n", 
            data->mode == VIZ_MODE_2D ? "2D" :
            data->mode == VIZ_MODE_3D ? "3D" : "Crystalline");
    fprintf(f, "  &quot;num_points&quot;: %u,\n", data->num_points);
    
    if (data->title[0] != '\0') {
        fprintf(f, "  &quot;title&quot;: &quot;%s&quot;,\n", data->title);
    }
    
    fprintf(f, "  &quot;points&quot;: [\n");
    
    int dims = (data->mode == VIZ_MODE_2D) ? 2 : 3;
    
    for (uint32_t i = 0; i < data->num_points; i++) {
        const VisualizationPoint* point = &data->points[i];
        
        fprintf(f, "    {\n");
        fprintf(f, "      &quot;id&quot;: %u,\n", point->id);
        fprintf(f, "      &quot;position&quot;: [");
        for (int d = 0; d < dims; d++) {
            fprintf(f, "%.6f", point->position[d]);
            if (d < dims - 1) fprintf(f, ", ");
        }
        fprintf(f, "],\n");
        fprintf(f, "      &quot;value&quot;: %.6f,\n", point->value);
        fprintf(f, "      &quot;symmetry_group&quot;: %u,\n", point->symmetry_group);
        fprintf(f, "      &quot;neighbors&quot;: [");
        for (uint32_t j = 0; j < point->num_neighbors; j++) {
            fprintf(f, "%u", point->neighbors[j]);
            if (j < point->num_neighbors - 1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        
        if (point->label[0] != '\0') {
            fprintf(f, ",\n      &quot;label&quot;: &quot;%s&quot;", point->label);
        }
        
        fprintf(f, "\n    }");
        if (i < data->num_points - 1) fprintf(f, ",");
        fprintf(f, "\n");
    }
    
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

int viz_export_csv(const VisualizationData* data, const char* filename) {
    if (!data || !filename) return -1;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "ERROR: Failed to open %s for writing\n", filename);
        return -1;
    }
    
    int dims = (data->mode == VIZ_MODE_2D) ? 2 : 3;
    
    // Header
    fprintf(f, "id,");
    for (int d = 0; d < dims; d++) {
        fprintf(f, "%c,", 'x' + d);
    }
    fprintf(f, "value,symmetry_group,num_neighbors\n");
    
    // Data
    for (uint32_t i = 0; i < data->num_points; i++) {
        const VisualizationPoint* point = &data->points[i];
        fprintf(f, "%u,", point->id);
        for (int d = 0; d < dims; d++) {
            fprintf(f, "%.6f,", point->position[d]);
        }
        fprintf(f, "%.6f,%u,%u\n", 
                point->value, point->symmetry_group, point->num_neighbors);
    }
    
    fclose(f);
    return 0;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void viz_print_summary(const VisualizationData* data, FILE* output) {
    if (!data || !output) return;
    
    fprintf(output, "\n=== Visualization Summary ===\n");
    fprintf(output, "Mode: %s\n", 
            data->mode == VIZ_MODE_2D ? "2D" :
            data->mode == VIZ_MODE_3D ? "3D" : "Crystalline");
    fprintf(output, "Points: %u\n", data->num_points);
    
    if (data->title[0] != '\0') {
        fprintf(output, "Title: %s\n", data->title);
    }
    
    // Calculate total connections
    uint32_t total_connections = 0;
    for (uint32_t i = 0; i < data->num_points; i++) {
        total_connections += data->points[i].num_neighbors;
    }
    fprintf(output, "Connections: %u\n", total_connections);
    
    fprintf(output, "============================\n\n");
}

int viz_validate(const VisualizationData* data) {
    if (!data) return 0;
    if (!data->points) return 0;
    
    int errors = 0;
    
    // Verify all points
    for (uint32_t i = 0; i < data->num_points; i++) {
        const VisualizationPoint* point = &data->points[i];
        
        // Check symmetry group
        if (point->symmetry_group >= 12) {
            fprintf(stderr, "ERROR: Point %u has invalid symmetry group %u\n",
                    i, point->symmetry_group);
            errors++;
        }
        
        // Check neighbors
        for (uint32_t j = 0; j < point->num_neighbors; j++) {
            uint32_t neighbor = point->neighbors[j];
            if (neighbor >= data->num_points) {
                fprintf(stderr, "ERROR: Point %u has invalid neighbor %u\n",
                        i, neighbor);
                errors++;
            }
        }
    }
    
    return (errors == 0) ? 1 : 0;
}
