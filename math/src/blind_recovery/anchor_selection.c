/**
 * @file anchor_selection.c
 * @brief Anchor point selection for blind recovery triangulation
 * 
 * Selects optimal anchor points (uncorrupted vertices) for triangulation-based
 * recovery. Uses geometric separation and Platonic solid symmetry.
 */

#include "blind_recovery.h"
#include "math/types.h"           // For MATH_PI and MATH_INFINITY
#include "math/transcendental.h"  // For math_sqrt
#include <stdlib.h>
#include <string.h>

/**
 * Compute Euclidean distance between two 3D points
 */
static double compute_distance(const double* p1, const double* p2) {
    double dx = p1[0] - p2[0];
    double dy = p1[1] - p2[1];
    double dz = p1[2] - p2[2];
    return math_sqrt(dx*dx + dy*dy + dz*dz);
}

/**
 * Compute minimum distance from point to set of anchors
 */
static double min_distance_to_anchors(
    const double* point,
    const AnchorPoint* anchors,
    uint32_t num_anchors
) {
    if (num_anchors == 0) return MATH_INFINITY;
    
    double min_dist = MATH_INFINITY;
    for (uint32_t i = 0; i < num_anchors; i++) {
        double dist = compute_distance(point, anchors[i].position);
        if (dist < min_dist) {
            min_dist = dist;
        }
    }
    return min_dist;
}

/**
 * Select optimal number of anchors based on Platonic solid type
 */
static uint32_t get_optimal_anchor_count(uint32_t num_vertices) {
    // Based on Platonic solid symmetry:
    // Tetrahedron (4V): 3 anchors (triangle)
    // Cube (8V): 4 anchors (tetrahedron inscribed)
    // Octahedron (6V): 4 anchors (square)
    // Dodecahedron (20V): 6 anchors (octahedron inscribed)
    // Icosahedron (12V): 6 anchors (octahedron inscribed)
    
    if (num_vertices <= 4) return 3;        // Tetrahedron
    else if (num_vertices <= 6) return 4;   // Octahedron
    else if (num_vertices <= 8) return 4;   // Cube
    else if (num_vertices <= 12) return 6;  // Icosahedron
    else return 6;                          // Dodecahedron or larger
}

/**
 * Create anchor system with initial allocation
 */
AnchorSystem* create_anchor_system(uint32_t max_anchors) {
    AnchorSystem* system = (AnchorSystem*)calloc(1, sizeof(AnchorSystem));
    if (!system) return NULL;
    
    system->anchors = (AnchorPoint*)calloc(max_anchors, sizeof(AnchorPoint));
    if (!system->anchors) {
        free(system);
        return NULL;
    }
    
    // Allocate triangulation matrix (distances between anchors)
    system->triangulation_matrix = (double*)calloc(
        max_anchors * max_anchors, 
        sizeof(double)
    );
    if (!system->triangulation_matrix) {
        free(system->anchors);
        free(system);
        return NULL;
    }
    
    system->num_anchors = 0;
    system->global_confidence = 0.0;
    
    return system;
}

/**
 * Free anchor system
 */
void free_anchor_system(AnchorSystem* system) {
    if (!system) return;
    
    // Free neighbor arrays in each anchor
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (system->anchors[i].neighbor_ids) {
            free(system->anchors[i].neighbor_ids);
        }
        if (system->anchors[i].position) {
            free(system->anchors[i].position);
        }
    }
    
    free(system->anchors);
    free(system->triangulation_matrix);
    free(system);
}

/**
 * Select anchor points using greedy maximum separation algorithm
 * 
 * Algorithm:
 * 1. Start with highest confidence uncorrupted vertex
 * 2. Iteratively add vertices that maximize minimum distance to existing anchors
 * 3. Stop when optimal anchor count reached
 */
AnchorSystem* select_anchors(
    const StructuralMap* structure,
    const double* vertex_positions,  // [num_vertices * 3]
    const double* confidence_scores,  // [num_vertices]
    uint32_t num_vertices
) {
    if (!structure || !vertex_positions || !confidence_scores) {
        return NULL;
    }
    
    // Determine optimal anchor count
    uint32_t target_anchors = get_optimal_anchor_count(num_vertices);
    
    // Create anchor system
    AnchorSystem* system = create_anchor_system(target_anchors);
    if (!system) return NULL;
    
    // Find uncorrupted vertices (confidence > 0.8)
    uint32_t* candidates = (uint32_t*)malloc(num_vertices * sizeof(uint32_t));
    uint32_t num_candidates = 0;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        if (!structure->corruption_mask[i] && confidence_scores[i] > 0.8) {
            candidates[num_candidates++] = i;
        }
    }
    
    if (num_candidates < target_anchors) {
        // Not enough uncorrupted vertices - use best available
        target_anchors = num_candidates;
    }
    
    if (num_candidates == 0) {
        free(candidates);
        free_anchor_system(system);
        return NULL;
    }
    
    // Select first anchor (highest confidence)
    uint32_t best_idx = 0;
    double best_confidence = confidence_scores[candidates[0]];
    for (uint32_t i = 1; i < num_candidates; i++) {
        if (confidence_scores[candidates[i]] > best_confidence) {
            best_confidence = confidence_scores[candidates[i]];
            best_idx = i;
        }
    }
    
    // Add first anchor
    uint32_t first_vertex = candidates[best_idx];
    system->anchors[0].vertex_id = first_vertex;
    system->anchors[0].position = (double*)malloc(3 * sizeof(double));
    memcpy(system->anchors[0].position, 
           &vertex_positions[first_vertex * 3], 
           3 * sizeof(double));
    system->anchors[0].confidence = confidence_scores[first_vertex];
    system->anchors[0].is_corrupted = false;
    system->anchors[0].num_neighbors = 0;
    system->anchors[0].neighbor_ids = NULL;
    system->num_anchors = 1;
    
    // Remove first anchor from candidates
    candidates[best_idx] = candidates[num_candidates - 1];
    num_candidates--;
    
    // Greedily select remaining anchors (maximize minimum distance)
    while (system->num_anchors < target_anchors && num_candidates > 0) {
        double max_min_dist = -1.0;
        uint32_t best_candidate_idx = 0;
        
        // Find candidate with maximum minimum distance to existing anchors
        for (uint32_t i = 0; i < num_candidates; i++) {
            uint32_t vertex_id = candidates[i];
            const double* pos = &vertex_positions[vertex_id * 3];
            
            double min_dist = min_distance_to_anchors(
                pos, 
                system->anchors, 
                system->num_anchors
            );
            
            if (min_dist > max_min_dist) {
                max_min_dist = min_dist;
                best_candidate_idx = i;
            }
        }
        
        // Add best candidate as anchor
        uint32_t vertex_id = candidates[best_candidate_idx];
        uint32_t anchor_idx = system->num_anchors;
        
        system->anchors[anchor_idx].vertex_id = vertex_id;
        system->anchors[anchor_idx].position = (double*)malloc(3 * sizeof(double));
        memcpy(system->anchors[anchor_idx].position,
               &vertex_positions[vertex_id * 3],
               3 * sizeof(double));
        system->anchors[anchor_idx].confidence = confidence_scores[vertex_id];
        system->anchors[anchor_idx].is_corrupted = false;
        system->anchors[anchor_idx].num_neighbors = 0;
        system->anchors[anchor_idx].neighbor_ids = NULL;
        
        system->num_anchors++;
        
        // Remove from candidates
        candidates[best_candidate_idx] = candidates[num_candidates - 1];
        num_candidates--;
    }
    
    free(candidates);
    
    // Build triangulation matrix (pairwise distances)
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        for (uint32_t j = 0; j < system->num_anchors; j++) {
            if (i == j) {
                system->triangulation_matrix[i * system->num_anchors + j] = 0.0;
            } else {
                double dist = compute_distance(
                    system->anchors[i].position,
                    system->anchors[j].position
                );
                system->triangulation_matrix[i * system->num_anchors + j] = dist;
            }
        }
    }
    
    // Compute global confidence (average of anchor confidences)
    double sum_confidence = 0.0;
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        sum_confidence += system->anchors[i].confidence;
    }
    system->global_confidence = sum_confidence / system->num_anchors;
    
    return system;
}

/**
 * Validate anchor system quality
 */
bool validate_anchor_system(const AnchorSystem* system) {
    if (!system || system->num_anchors < 3) {
        return false;
    }
    
    // Check global confidence
    if (system->global_confidence < 0.7) {
        return false;
    }
    
    // Check that anchors are well-separated
    // Minimum distance should be > 0.1 * max_distance
    double max_dist = 0.0;
    double min_dist = MATH_INFINITY;
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        for (uint32_t j = i + 1; j < system->num_anchors; j++) {
            double dist = system->triangulation_matrix[i * system->num_anchors + j];
            if (dist > max_dist) max_dist = dist;
            if (dist < min_dist) min_dist = dist;
        }
    }
    
    if (min_dist < 0.1 * max_dist) {
        return false;  // Anchors too close together
    }
    
    return true;
}
