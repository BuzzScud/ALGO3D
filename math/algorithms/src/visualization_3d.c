/**
 * 3D Visualization Projection
 * 
 * Implements 3D projection algorithms for visualization.
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_cos with math_cos (2 calls)
 * - Replaced math_sin with math_sin (2 calls)
 * Total: 4 function calls migrated to NEW math library
 */

#include "visualization.h"
#include "sphere_packing.h"
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
 * Project to 3D using spherical layout
 * 
 * Projects points to 3D space using spherical coordinates.
 * Uses clock lattice structure for positioning.
 */
int viz_project_3d(VisualizationData* data) {
    if (!data) return -1;
    
    // Use spherical layout based on clock structure
    for (uint32_t i = 0; i < data->num_points; i++) {
        VisualizationPoint* point = &data->points[i];
        
        // Map to clock position
        BabylonianClockPosition clock_pos = map_prime_index_to_clock(i);
        
        // Calculate spherical coordinates
        // Radius based on ring
        double radius = 0.2 + clock_pos.ring * 0.2;
        
        // Theta (azimuthal angle) based on symmetry group
        double theta = (point->symmetry_group * 2.0 * 3.14159265358979323846) / 12.0;
        
        // Phi (polar angle) based on position in ring
        double positions_in_ring = 12.0;
        if (clock_pos.ring == 1 || clock_pos.ring == 2) positions_in_ring = 60.0;
        else if (clock_pos.ring == 3) positions_in_ring = 100.0;
        else if (clock_pos.ring > 3) positions_in_ring = 1000.0;
        
        double phi = (clock_pos.position * 3.14159265358979323846) / positions_in_ring;
        
        // Convert to Cartesian coordinates
        double sin_phi = math_sin(phi);
        point->position[0] = radius * sin_phi * math_cos(theta);
        point->position[1] = radius * sin_phi * math_sin(theta);
        point->position[2] = radius * math_cos(phi);
    }
    
    // Update bounds
    viz_update_bounds(data);
    
    return 0;
}

/**
 * Get rendering data for 3D
 */
int viz_get_3d_rendering_data(
    const VisualizationData* data,
    double** positions,
    uint32_t** connections,
    uint32_t* num_connections
) {
    if (!data || !positions || !connections || !num_connections) return -1;
    
    // Allocate positions array (x, y, z for each point)
    *positions = malloc(data->num_points * 3 * sizeof(double));
    if (!*positions) return -1;
    
    // Copy positions
    for (uint32_t i = 0; i < data->num_points; i++) {
        (*positions)[i * 3 + 0] = data->points[i].position[0];
        (*positions)[i * 3 + 1] = data->points[i].position[1];
        (*positions)[i * 3 + 2] = data->points[i].position[2];
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
