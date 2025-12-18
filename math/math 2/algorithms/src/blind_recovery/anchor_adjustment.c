/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file anchor_adjustment.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Iterative anchor position refinement and reselection
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Refines anchor positions based on recovery results and detects
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * when anchors themselves may be corrupted.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>

/**
 * Compute distance between two 3D points
 */
static double distance_3d(const double* p1, const double* p2) {
    double dx = p1[0] - p2[0];
    double dy = p1[1] - p2[1];
    double dz = p1[2] - p2[2];
    return math_sqrt(dx*dx + dy*dy + dz*dz);
}

/**
 * Detect if an anchor may be corrupted based on inconsistencies
 * 
 * An anchor is suspicious if:
 * 1. Its confidence score drops below threshold
 * 2. Distances to other anchors are inconsistent with triangulation matrix
 * 3. Recovery results using this anchor have high error
 */
bool detect_anchor_corruption(
    const AnchorSystem* system,
    uint32_t anchor_idx,
    const double* vertex_positions,
    uint32_t num_vertices
) {
    (void)vertex_positions;  // Unused in this implementation
    (void)num_vertices;      // Unused in this implementation
    
    if (!system || anchor_idx >= system->num_anchors) {
        return false;
    }
    
    const AnchorPoint* anchor = &system->anchors[anchor_idx];
    
    // Check 1: Low confidence
    if (anchor->confidence < 0.6) {
        return true;
    }
    
    // Check 2: Distance inconsistencies with other anchors
    double max_distance_error = 0.0;
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (i == anchor_idx) continue;
        
        double actual_dist = distance_3d(
            anchor->position,
            system->anchors[i].position
        );
        double expected_dist = system->triangulation_matrix[
            anchor_idx * system->num_anchors + i
        ];
        
        double error = math_abs(actual_dist - expected_dist) / expected_dist;
        if (error > max_distance_error) {
            max_distance_error = error;
        }
    }
    
    // If distance error > 10%, anchor may be corrupted
    if (max_distance_error > 0.1) {
        return true;
    }
    
    return false;
}

/**
 * Refine anchor position using neighboring vertices
 * 
 * Uses weighted average of nearby high-confidence vertices
 */
bool refine_anchor_position(
    AnchorPoint* anchor,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices
) {
    if (!anchor || !vertex_positions || !confidence_scores) {
        return false;
    }
    
    // Find nearby high-confidence vertices
    const double NEIGHBOR_RADIUS = 2.0;  // Adjust based on scale
    const double MIN_CONFIDENCE = 0.8;
    
    double weighted_sum[3] = {0.0, 0.0, 0.0};
    double total_weight = 0.0;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        if (confidence_scores[i] < MIN_CONFIDENCE) {
            continue;
        }
        
        const double* vertex_pos = &vertex_positions[i * 3];
        double dist = distance_3d(anchor->position, vertex_pos);
        
        if (dist < NEIGHBOR_RADIUS) {
            // Weight by confidence and inverse distance
            double weight = confidence_scores[i] / (dist + 0.1);
            
            weighted_sum[0] += weight * vertex_pos[0];
            weighted_sum[1] += weight * vertex_pos[1];
            weighted_sum[2] += weight * vertex_pos[2];
            total_weight += weight;
        }
    }
    
    if (total_weight < 1e-6) {
        return false;  // No nearby vertices found
    }
    
    // Update anchor position (blend with current position)
    const double BLEND_FACTOR = 0.3;  // 30% new, 70% old
    
    double new_pos[3];
    new_pos[0] = weighted_sum[0] / total_weight;
    new_pos[1] = weighted_sum[1] / total_weight;
    new_pos[2] = weighted_sum[2] / total_weight;
    
    anchor->position[0] = (1.0 - BLEND_FACTOR) * anchor->position[0] + 
                          BLEND_FACTOR * new_pos[0];
    anchor->position[1] = (1.0 - BLEND_FACTOR) * anchor->position[1] + 
                          BLEND_FACTOR * new_pos[1];
    anchor->position[2] = (1.0 - BLEND_FACTOR) * anchor->position[2] + 
                          BLEND_FACTOR * new_pos[2];
    
    return true;
}

/**
 * Update anchor confidence scores based on recovery results
 */
void update_anchor_confidence(
    AnchorSystem* system,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices
) {
    if (!system || !vertex_positions || !confidence_scores) {
        return;
    }
    
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        AnchorPoint* anchor = &system->anchors[i];
        
        // Check for corruption
        if (detect_anchor_corruption(system, i, vertex_positions, num_vertices)) {
            anchor->is_corrupted = true;
            anchor->confidence *= 0.5;  // Reduce confidence
        } else {
            // Increase confidence slightly if no issues detected
            anchor->confidence = (anchor->confidence * 1.05 < 1.0) ? anchor->confidence * 1.05 : 1.0;
        }
    }
    
    // Update global confidence
    double sum = 0.0;
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        sum += system->anchors[i].confidence;
    }
    system->global_confidence = sum / system->num_anchors;
}

/**
 * Replace corrupted anchor with new candidate
 */
bool replace_corrupted_anchor(
    AnchorSystem* system,
    uint32_t anchor_idx,
    const double* vertex_positions,
    const double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices
) {
    if (!system || anchor_idx >= system->num_anchors) {
        return false;
    }
    
    // Find best replacement candidate
    // Criteria: high confidence, not corrupted, far from other anchors
    
    double best_score = -1.0;
    uint32_t best_vertex = 0;
    
    for (uint32_t v = 0; v < num_vertices; v++) {
        if (corruption_mask[v] || confidence_scores[v] < 0.8) {
            continue;  // Skip corrupted or low-confidence vertices
        }
        
        const double* pos = &vertex_positions[v * 3];
        
        // Compute minimum distance to existing anchors (excluding the one being replaced)
        double min_dist = INFINITY;
        for (uint32_t i = 0; i < system->num_anchors; i++) {
            if (i == anchor_idx) continue;
            
            double dist = distance_3d(pos, system->anchors[i].position);
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
        
        // Score: combination of confidence and separation
        double score = confidence_scores[v] * min_dist;
        
        if (score > best_score) {
            best_score = score;
            best_vertex = v;
        }
    }
    
    if (best_score < 0.0) {
        return false;  // No suitable replacement found
    }
    
    // Replace anchor
    AnchorPoint* anchor = &system->anchors[anchor_idx];
    anchor->vertex_id = best_vertex;
    memcpy(anchor->position, 
           &vertex_positions[best_vertex * 3],
           3 * sizeof(double));
    anchor->confidence = confidence_scores[best_vertex];
    anchor->is_corrupted = false;
    
    // Update triangulation matrix
    for (uint32_t i = 0; i < system->num_anchors; i++) {
        if (i == anchor_idx) {
            system->triangulation_matrix[i * system->num_anchors + i] = 0.0;
        } else {
            double dist = distance_3d(anchor->position, system->anchors[i].position);
            system->triangulation_matrix[anchor_idx * system->num_anchors + i] = dist;
            system->triangulation_matrix[i * system->num_anchors + anchor_idx] = dist;
        }
    }
    
    return true;
}

/**
 * Perform iterative anchor adjustment
 * 
 * Returns number of anchors adjusted
 */
uint32_t adjust_anchors_iterative(
    AnchorSystem* system,
    double* vertex_positions,
    double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices,
    uint32_t max_iterations
) {
    if (!system || !vertex_positions || !confidence_scores || !corruption_mask) {
        return 0;
    }
    
    uint32_t total_adjustments = 0;
    
    for (uint32_t iter = 0; iter < max_iterations; iter++) {
        uint32_t adjustments_this_iter = 0;
        
        // Update confidence scores
        update_anchor_confidence(system, vertex_positions, confidence_scores, num_vertices);
        
        // Check each anchor
        for (uint32_t i = 0; i < system->num_anchors; i++) {
            AnchorPoint* anchor = &system->anchors[i];
            
            if (anchor->is_corrupted) {
                // Try to replace corrupted anchor
                if (replace_corrupted_anchor(
                        system, i, vertex_positions, confidence_scores,
                        corruption_mask, num_vertices)) {
                    adjustments_this_iter++;
                }
            } else {
                // Refine position
                if (refine_anchor_position(
                        anchor, vertex_positions, confidence_scores, num_vertices)) {
                    adjustments_this_iter++;
                }
            }
        }
        
        total_adjustments += adjustments_this_iter;
        
        // Stop if no adjustments made
        if (adjustments_this_iter == 0) {
            break;
        }
    }
    
    return total_adjustments;
}
