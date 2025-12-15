/**
 * 2D Visualization Projection
 * 
 * Implements 2D projection algorithms for visualization.
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_cos with math_cos (1 call)
 * - Replaced math_sin with math_sin (1 call)
 * Total: 2 function calls migrated to NEW math library
 */

#include "visualization.h"
#include "../../math/include/math/transcendental.h"  // NEW math library
#include <stdlib.h>
#include <string.h>

// Forward declarations to avoid type conflicts
typedef struct {
    int ring;
    int position;
    double angle;
    double radius;
} BabylonianClockPosition;

extern BabylonianClockPosition map_prime_index_to_clock(int prime_index);

/**
 * Project to 2D using circular layout
 * 
 * Projects points to a 2D circular layout based on symmetry groups.
 * Each symmetry group gets a sector of the circle.
 */
int viz_project_2d(VisualizationData* data) {
    if (!data) return -1;
    
    // Use circular layout based on symmetry groups
    for (uint32_t i = 0; i < data->num_points; i++) {
        VisualizationPoint* point = &data->points[i];
        
        // Map to clock position
        BabylonianClockPosition clock_pos = map_prime_index_to_clock(i);
        
        // Calculate angle based on symmetry group and position in ring
        double base_angle = (point->symmetry_group * 2.0 * 3.14159265358979323846) / 12.0;
        
        // Calculate radius based on ring
        double radius = 0.1 + clock_pos.ring * 0.15;
        
        // Add position offset within ring
        double positions_in_ring = 12.0;
        if (clock_pos.ring == 1 || clock_pos.ring == 2) positions_in_ring = 60.0;
        else if (clock_pos.ring == 3) positions_in_ring = 100.0;
        else if (clock_pos.ring > 3) positions_in_ring = 1000.0;
        
        double angle_offset = (clock_pos.position * 2.0 * 3.14159265358979323846) / positions_in_ring;
        double angle = base_angle + angle_offset;
        
        // Calculate 2D position
        point->position[0] = radius * math_cos(angle);
        point->position[1] = radius * math_sin(angle);
        point->position[2] = 0.0; // Not used in 2D
    }
    
    // Update bounds
    viz_update_bounds(data);
    
    return 0;
}

/**
 * Get rendering data for 2D
 */
int viz_get_2d_rendering_data(
    const VisualizationData* data,
    double** positions,
    uint32_t** connections,
    uint32_t* num_connections
) {
    if (!data || !positions || !connections || !num_connections) return -1;
    
    // Allocate positions array (x, y for each point)
    *positions = malloc(data->num_points * 2 * sizeof(double));
    if (!*positions) return -1;
    
    // Copy positions
    for (uint32_t i = 0; i < data->num_points; i++) {
        (*positions)[i * 2 + 0] = data->points[i].position[0];
        (*positions)[i * 2 + 1] = data->points[i].position[1];
    }
    
    // Count total connections
    uint32_t total_conn = 0;
    for (uint32_t i = 0; i < data->num_points; i++) {
        total_conn += data->points[i].num_neighbors;
    }
    
    // Allocate connections array (pairs of point IDs)
    *connections = malloc(total_conn * 2 * sizeof(uint32_t));
    if (!*connections) {
        free(*positions);
        return -1;
    }
    
    // Build connections
    uint32_t conn_idx = 0;
    for (uint32_t i = 0; i < data->num_points; i++) {
        const VisualizationPoint* point = &data->points[i];
        for (uint32_t j = 0; j < point->num_neighbors; j++) {
            (*connections)[conn_idx * 2 + 0] = i;
            (*connections)[conn_idx * 2 + 1] = point->neighbors[j];
            conn_idx++;
        }
    }
    
    *num_connections = total_conn;
    
    return 0;
}
