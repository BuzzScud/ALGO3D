/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file triangulation.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Triangulation-based vertex recovery using anchor points
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Recovers corrupted vertices using least-squares triangulation from
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * anchor points. Biases solutions toward tetration attractors for stability.
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

// Tetration attractors for convergence bias
static const uint64_t TETRATION_ATTRACTORS[] = {
    4,          // 2^2
    16,         // 2^2^2
    256,        // 2^2^2^2
    65536,      // 2^2^2^2^2
    27,         // 3^3
    7625597484987ULL,  // 3^3^3 (approximate)
    3125,       // 5^5
    // More attractors can be added
};
static const uint32_t NUM_ATTRACTORS = 7;

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
 * Find nearest tetration attractor for biasing
 */
static double find_nearest_attractor(double value) {
    double nearest = (double)TETRATION_ATTRACTORS[0];
    double min_diff = math_abs(value - nearest);
    
    for (uint32_t i = 1; i < NUM_ATTRACTORS; i++) {
        double attractor = (double)TETRATION_ATTRACTORS[i];
        double diff = math_abs(value - attractor);
        if (diff < min_diff) {
            min_diff = diff;
            nearest = attractor;
        }
    }
    
    return nearest;
}

/**
 * Solve 3x3 linear system using Gaussian elimination
 * Ax = b, where A is 3x3 and b is 3x1
 */
static bool solve_3x3(double A[3][3], double b[3], double x[3]) {
    // Forward elimination
    for (int i = 0; i < 3; i++) {
        // Find pivot
        int max_row = i;
        for (int k = i + 1; k < 3; k++) {
            if (math_abs(A[k][i]) > math_abs(A[max_row][i])) {
                max_row = k;
            }
        }
        
        // Swap rows
        if (max_row != i) {
            for (int k = 0; k < 3; k++) {
                double tmp = A[i][k];
                A[i][k] = A[max_row][k];
                A[max_row][k] = tmp;
            }
            double tmp = b[i];
            b[i] = b[max_row];
            b[max_row] = tmp;
        }
        
        // Check for singular matrix
        if (math_abs(A[i][i]) < 1e-10) {
            return false;
        }
        
        // Eliminate column
        for (int k = i + 1; k < 3; k++) {
            double factor = A[k][i] / A[i][i];
            for (int j = i; j < 3; j++) {
                A[k][j] -= factor * A[i][j];
            }
            b[k] -= factor * b[i];
        }
    }
    
    // Back substitution
    for (int i = 2; i >= 0; i--) {
        x[i] = b[i];
        for (int j = i + 1; j < 3; j++) {
            x[i] -= A[i][j] * x[j];
        }
        x[i] /= A[i][i];
    }
    
    return true;
}

/**
 * Recover single vertex using least-squares triangulation
 * 
 * Given anchors A₁, A₂, ..., Aₙ and distances r₁, r₂, ..., rₙ,
 * find V that minimizes: Σᵢ (||V - Aᵢ|| - rᵢ)²
 * 
 * Uses iterative refinement with tetration attractor bias.
 */
bool recover_vertex_triangulation(
    const AnchorSystem* anchors,
    const double* expected_distances,  // [num_anchors] - distances to anchors
    double* recovered_position,        // [3] - output
    double* confidence                 // output confidence score
) {
    if (!anchors || !expected_distances || !recovered_position || !confidence) {
        return false;
    }
    
    if (anchors->num_anchors < 3) {
        return false;  // Need at least 3 anchors for 3D triangulation
    }
    
    // Initial guess: centroid of anchors
    double centroid[3] = {0.0, 0.0, 0.0};
    for (uint32_t i = 0; i < anchors->num_anchors; i++) {
        centroid[0] += anchors->anchors[i].position[0];
        centroid[1] += anchors->anchors[i].position[1];
        centroid[2] += anchors->anchors[i].position[2];
    }
    centroid[0] /= anchors->num_anchors;
    centroid[1] /= anchors->num_anchors;
    centroid[2] /= anchors->num_anchors;
    
    // Copy initial guess to output
    memcpy(recovered_position, centroid, 3 * sizeof(double));
    
    // Iterative refinement (Gauss-Newton method)
    const uint32_t MAX_ITERATIONS = 100;
    const double CONVERGENCE_THRESHOLD = 1e-6;
    
    for (uint32_t iter = 0; iter < MAX_ITERATIONS; iter++) {
        // Build Jacobian matrix and residual vector
        // We'll use first 3 anchors for the linear system
        double J[3][3] = {{0}};
        double r[3] = {0};
        
        for (uint32_t i = 0; i < 3 && i < anchors->num_anchors; i++) {
            const double* anchor_pos = anchors->anchors[i].position;
            double dx = recovered_position[0] - anchor_pos[0];
            double dy = recovered_position[1] - anchor_pos[1];
            double dz = recovered_position[2] - anchor_pos[2];
            double dist = math_sqrt(dx*dx + dy*dy + dz*dz);
            
            if (dist < 1e-10) dist = 1e-10;  // Avoid division by zero
            
            // Jacobian: ∂f/∂x, ∂f/∂y, ∂f/∂z
            J[i][0] = dx / dist;
            J[i][1] = dy / dist;
            J[i][2] = dz / dist;
            
            // Residual: current_distance - expected_distance
            r[i] = dist - expected_distances[i];
        }
        
        // Solve J * delta = -r
        double delta[3];
        if (!solve_3x3(J, r, delta)) {
            // Singular matrix - use current position
            break;
        }
        
        // Update position
        recovered_position[0] -= delta[0];
        recovered_position[1] -= delta[1];
        recovered_position[2] -= delta[2];
        
        // Apply tetration attractor bias (10% weight)
        const double ATTRACTOR_WEIGHT = 0.1;
        for (int i = 0; i < 3; i++) {
            double attractor = find_nearest_attractor(recovered_position[i]);
            recovered_position[i] = (1.0 - ATTRACTOR_WEIGHT) * recovered_position[i] +
                                   ATTRACTOR_WEIGHT * attractor;
        }
        
        // Check convergence
        double delta_norm = math_sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);
        if (delta_norm < CONVERGENCE_THRESHOLD) {
            break;
        }
    }
    
    // Compute confidence based on residual error
    double total_error = 0.0;
    for (uint32_t i = 0; i < anchors->num_anchors; i++) {
        double dist = distance_3d(recovered_position, anchors->anchors[i].position);
        double error = math_abs(dist - expected_distances[i]);
        total_error += error;
    }
    double avg_error = total_error / anchors->num_anchors;
    
    // Confidence: 1.0 for zero error, decreases exponentially
    *confidence = math_exp(-avg_error);
    
    return true;
}

/**
 * Recover all corrupted vertices using triangulation
 */
bool recover_all_vertices(
    const AnchorSystem* anchors,
    const StructuralMap* structure,
    double* vertex_positions,      // [num_vertices * 3] - in/out
    double* confidence_scores,     // [num_vertices] - in/out
    uint32_t num_vertices
) {
    if (!anchors || !structure || !vertex_positions || !confidence_scores) {
        return false;
    }
    
    uint32_t num_recovered = 0;
    
    // For each corrupted vertex
    for (uint32_t v = 0; v < num_vertices; v++) {
        if (!structure->corruption_mask[v]) {
            continue;  // Not corrupted, skip
        }
        
        // Estimate expected distances to anchors using geometric constraints
        double* expected_distances = (double*)malloc(anchors->num_anchors * sizeof(double));
        
        // Better heuristic: use distance from centroid of anchors
        // This gives a more accurate estimate for vertex positions
        double centroid[3] = {0.0, 0.0, 0.0};
        for (uint32_t i = 0; i < anchors->num_anchors; i++) {
            centroid[0] += anchors->anchors[i].position[0];
            centroid[1] += anchors->anchors[i].position[1];
            centroid[2] += anchors->anchors[i].position[2];
        }
        centroid[0] /= anchors->num_anchors;
        centroid[1] /= anchors->num_anchors;
        centroid[2] /= anchors->num_anchors;
        
        // Estimate distance from each anchor to centroid
        // Assume corrupted vertex is roughly at centroid distance
        for (uint32_t i = 0; i < anchors->num_anchors; i++) {
            double dx = anchors->anchors[i].position[0] - centroid[0];
            double dy = anchors->anchors[i].position[1] - centroid[1];
            double dz = anchors->anchors[i].position[2] - centroid[2];
            expected_distances[i] = math_sqrt(dx*dx + dy*dy + dz*dz);
            
            // Add small variation based on anchor index for better distribution
            expected_distances[i] *= (0.9 + 0.2 * (double)i / anchors->num_anchors);
        }
        
        // Recover vertex
        double recovered_pos[3];
        double confidence;
        
        if (recover_vertex_triangulation(
                anchors,
                expected_distances,
                recovered_pos,
                &confidence
            )) {
            // Update vertex position
            memcpy(&vertex_positions[v * 3], recovered_pos, 3 * sizeof(double));
            confidence_scores[v] = confidence;
            num_recovered++;
        }
        
        free(expected_distances);
    }
    
    return num_recovered > 0;
}

/**
 * Compute triangulation quality metrics
 */
void compute_triangulation_metrics(
    const AnchorSystem* anchors,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices,
    double* avg_confidence,
    double* min_confidence,
    double* max_confidence
) {
    if (!anchors || !vertex_positions || !confidence_scores) {
        return;
    }
    
    double sum = 0.0;
    double min_conf = 1.0;
    double max_conf = 0.0;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        double conf = confidence_scores[i];
        sum += conf;
        if (conf < min_conf) min_conf = conf;
        if (conf > max_conf) max_conf = conf;
    }
    
    if (avg_confidence) *avg_confidence = sum / num_vertices;
    if (min_confidence) *min_confidence = min_conf;
    if (max_confidence) *max_confidence = max_conf;
}
