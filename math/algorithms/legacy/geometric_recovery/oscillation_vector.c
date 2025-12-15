/**
 * @file oscillation_vector.c
 * @brief Directional oscillation tracking using anchors as gradient
 */

#include "geometric_recovery.h"
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Compute directional oscillation using anchors as gradient
 * 
 * This is CRITICAL - we track not just distance but DIRECTION.
 * Uses anchors to compute gradient in each dimension.
 */
OscillationVector compute_oscillation_vector(
    const double* candidate_pos,
    const double* target_pos,
    const double** anchor_positions,
    uint32_t num_anchors,
    uint32_t num_dimensions
) {
    OscillationVector vec;
    memset(&vec, 0, sizeof(OscillationVector));
    
    // 1. Compute distance (magnitude)
    double dist_sq = 0.0;
    for (uint32_t d = 0; d < num_dimensions; d++) {
        double diff = candidate_pos[d] - target_pos[d];
        dist_sq += diff * diff;
    }
    vec.distance = math_sqrt(dist_sq);
    
    // 2. Compute direction using anchors as gradient
    // Find 3 nearest anchors to candidate
    uint32_t nearest[3] = {0, 0, 0};
    double distances[3] = {1e10, 1e10, 1e10};
    
    for (uint32_t a = 0; a < num_anchors; a++) {
        double anchor_dist_sq = 0.0;
        for (uint32_t d = 0; d < num_dimensions; d++) {
            double diff = candidate_pos[d] - anchor_positions[a][d];
            anchor_dist_sq += diff * diff;
        }
        double anchor_dist = math_sqrt(anchor_dist_sq);
        
        if (anchor_dist < distances[0]) {
            distances[2] = distances[1];
            nearest[2] = nearest[1];
            distances[1] = distances[0];
            nearest[1] = nearest[0];
            distances[0] = anchor_dist;
            nearest[0] = a;
        } else if (anchor_dist < distances[1]) {
            distances[2] = distances[1];
            nearest[2] = nearest[1];
            distances[1] = anchor_dist;
            nearest[1] = a;
        } else if (anchor_dist < distances[2]) {
            distances[2] = anchor_dist;
            nearest[2] = a;
        }
    }
    
    // Compute gradient direction in each dimension
    vec.gradient_strength = 0.0;
    for (uint32_t d = 0; d < num_dimensions; d++) {
        double gradient = 0.0;
        double total_weight = 0.0;
        
        // Use 3 nearest anchors to compute gradient
        for (uint32_t i = 0; i < 3; i++) {
            uint32_t anchor_idx = nearest[i];
            
            // Distance from anchor to target in this dimension
            double anchor_to_target = target_pos[d] - anchor_positions[anchor_idx][d];
            
            // Distance from candidate to target in this dimension
            double candidate_to_target = target_pos[d] - candidate_pos[d];
            
            // Gradient is the difference
            double local_gradient = anchor_to_target - candidate_to_target;
            
            // Weight by inverse distance (closer anchors have more influence)
            double weight = 1.0 / (distances[i] + 1e-6);
            gradient += local_gradient * weight;
            total_weight += weight;
        }
        
        vec.direction[d] = gradient / total_weight;
        vec.gradient_strength += vec.direction[d] * vec.direction[d];
    }
    vec.gradient_strength = math_sqrt(vec.gradient_strength);
    
    // 3. Normalize direction vector
    if (vec.gradient_strength > 1e-6) {
        for (uint32_t d = 0; d < num_dimensions; d++) {
            vec.direction[d] /= vec.gradient_strength;
        }
    }
    
    // 4. Compute velocity (rate of change)
    for (uint32_t d = 0; d < num_dimensions; d++) {
        vec.velocity[d] = vec.direction[d] * vec.distance;
    }
    
    // 5. Determine quadrant
    vec.quadrant = determine_quadrant(vec.direction, num_dimensions);
    
    // 6. Determine polarity based on quadrant
    vec.polarity = (vec.quadrant == 1 || vec.quadrant == 4) ? +1 : -1;
    
    // 7. Determine if moving toward or away from target
    // Dot product of direction and velocity
    double dot_product = 0.0;
    for (uint32_t d = 0; d < num_dimensions; d++) {
        // Direction toward target
        double toward_target = target_pos[d] - candidate_pos[d];
        dot_product += vec.direction[d] * toward_target;
    }
    vec.moving_toward = (dot_product > 0);  // Positive = moving toward
    
    return vec;
}

/**
 * Determine quadrant from position
 * 
 * Uses first two dimensions to determine quadrant:
 * Q1: (+, +)  polarity +1
 * Q2: (-, +)  polarity -1
 * Q3: (-, -)  polarity +1
 * Q4: (+, -)  polarity -1
 */
int determine_quadrant(const double* position, uint32_t num_dimensions) {
    if (num_dimensions < 2) return 1;
    
    bool x_positive = (position[0] >= 0);
    bool y_positive = (position[1] >= 0);
    
    if (x_positive && y_positive) return 1;
    if (!x_positive && y_positive) return 2;
    if (!x_positive && !y_positive) return 3;
    return 4;  // x_positive && !y_positive
}
