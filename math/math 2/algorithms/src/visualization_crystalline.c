/**
 * Crystalline Visualization Projection
 * 
 * Implements crystalline structure projection using clock lattice lattice.
 * 
 * PHASE 2: Fixed include order to prevent type conflicts
 */

#include "visualization.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/constants.h"
// #include "../../include/clock_lattice.h"  // Legacy - not needed
#include <stdlib.h>
#include <string.h>

/**
 * Project to Crystalline structure
 * 
 * Projects points to crystalline lattice structure using clock lattice.
 * Position[0] = ring (0-N)
 * Position[1] = position within ring (0-positions_in_ring)
 * Position[2] = symmetry group (0-11)
 */
int viz_project_crystalline(VisualizationData* data) {
    if (!data) return -1;
    
    // Map each point to its clock position
    for (uint32_t i = 0; i < data->num_points; i++) {
        VisualizationPoint* point = &data->points[i];
        
        // TODO: Implement clock position mapping without legacy clock_lattice.h
        // For now, use simple circular mapping
        double angle = (2.0 * MATH_PI * i) / data->num_points;
        double radius = 1.0 + (i / 12.0);  // Ring number
        
        // Store clock coordinates
        point->position[0] = radius;
        point->position[1] = angle;
        point->position[2] = (double)point->symmetry_group;
    }
    
    // Update bounds
    // Find max ring
    double max_ring = 0.0;
    double max_position = 0.0;
    
    for (uint32_t i = 0; i < data->num_points; i++) {
        if (data->points[i].position[0] > max_ring) {
            max_ring = data->points[i].position[0];
        }
        if (data->points[i].position[1] > max_position) {
            max_position = data->points[i].position[1];
        }
    }
    
    data->min_bounds[0] = 0.0;
    data->max_bounds[0] = max_ring;
    data->min_bounds[1] = 0.0;
    data->max_bounds[1] = max_position;
    data->min_bounds[2] = 0.0;
    data->max_bounds[2] = 11.0;
    
    return 0;
}

/**
 * Get rendering data for Crystalline
 * 
 * Returns clock lattice positions (ring, position) for rendering.
 */
int viz_get_crystalline_rendering_data(
    const VisualizationData* data,
    double** ring_positions,
    uint32_t** connections,
    uint32_t* num_connections
) {
    if (!data || !ring_positions || !connections || !num_connections) return -1;
    
    // Allocate ring positions array (ring, position, symmetry_group for each point)
    *ring_positions = malloc(data->num_points * 3 * sizeof(double));
    if (!*ring_positions) return -1;
    
    // Copy positions
    for (uint32_t i = 0; i < data->num_points; i++) {
        (*ring_positions)[i * 3 + 0] = data->points[i].position[0]; // ring
        (*ring_positions)[i * 3 + 1] = data->points[i].position[1]; // position
        (*ring_positions)[i * 3 + 2] = data->points[i].position[2]; // symmetry
    }
    
    // Count total connections
    uint32_t total_conn = 0;
    for (uint32_t i = 0; i < data->num_points; i++) {
        total_conn += data->points[i].num_neighbors;
    }
    
    // Allocate connections array (pairs of point IDs)
    *connections = malloc(total_conn * 2 * sizeof(uint32_t));
    if (!*connections) {
        free(*ring_positions);
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
