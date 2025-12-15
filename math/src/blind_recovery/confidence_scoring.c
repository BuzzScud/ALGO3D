/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file confidence_scoring.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Confidence score computation for recovery quality assessment
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Computes confidence scores for vertices based on geometric consistency,
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * triangulation accuracy, and structural constraints.
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
 * Compute confidence score for a single vertex based on anchor distances
 * 
 * High confidence if:
 * - Distances to anchors are consistent with expected geometry
 * - Vertex position is stable across multiple triangulation attempts
 * - Geometric constraints (Euler's formula) are satisfied
 */
double compute_vertex_confidence(
    const double* vertex_position,
    const AnchorSystem* anchors,
    const double* expected_distances  // [num_anchors]
) {
    if (!vertex_position || !anchors || !expected_distances) {
        return 0.0;
    }
    
    if (anchors->num_anchors == 0) {
        return 0.0;
    }
    
    // Compute distance error to each anchor
    double total_error = 0.0;
    double max_error = 0.0;
    
    for (uint32_t i = 0; i < anchors->num_anchors; i++) {
        double actual_dist = distance_3d(vertex_position, anchors->anchors[i].position);
        double expected_dist = expected_distances[i];
        
        double error = math_abs(actual_dist - expected_dist);
        double relative_error = error / (expected_dist + 1e-6);
        
        total_error += relative_error;
        if (relative_error > max_error) {
            max_error = relative_error;
        }
    }
    
    double avg_error = total_error / anchors->num_anchors;
    
    // Confidence score: exponential decay with error
    // Perfect match (0 error) = 1.0 confidence
    // 10% error = ~0.9 confidence
    // 50% error = ~0.6 confidence
    double confidence = math_exp(-2.0 * avg_error);
    
    // Penalize if max error is very high (outlier detection)
    if (max_error > 0.5) {
        confidence *= 0.5;
    }
    
    return confidence;
}

/**
 * Compute confidence scores for all vertices
 */
void compute_all_confidence_scores(
    const double* vertex_positions,
    const AnchorSystem* anchors,
    const StructuralMap* structure,
    double* confidence_scores,
    uint32_t num_vertices
) {
    if (!vertex_positions || !anchors || !structure || !confidence_scores) {
        return;
    }
    
    // For each vertex, compute confidence based on anchor distances
    for (uint32_t v = 0; v < num_vertices; v++) {
        if (structure->corruption_mask[v]) {
            // Corrupted vertex - compute confidence from recovery
            
            // Estimate expected distances (use average anchor separation)
            double* expected_distances = (double*)malloc(
                anchors->num_anchors * sizeof(double)
            );
            
            double avg_anchor_dist = 0.0;
            uint32_t count = 0;
            for (uint32_t i = 0; i < anchors->num_anchors; i++) {
                for (uint32_t j = i + 1; j < anchors->num_anchors; j++) {
                    avg_anchor_dist += anchors->triangulation_matrix[
                        i * anchors->num_anchors + j
                    ];
                    count++;
                }
            }
            if (count > 0) {
                avg_anchor_dist /= count;
            }
            
            for (uint32_t i = 0; i < anchors->num_anchors; i++) {
                expected_distances[i] = avg_anchor_dist;
            }
            
            confidence_scores[v] = compute_vertex_confidence(
                &vertex_positions[v * 3],
                anchors,
                expected_distances
            );
            
            free(expected_distances);
        } else {
            // Uncorrupted vertex - high confidence
            confidence_scores[v] = 0.95;
        }
    }
}

/**
 * Compute global recovery quality metrics
 */
void compute_recovery_metrics(
    const double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices,
    RecoveryMetrics* metrics
) {
    if (!confidence_scores || !corruption_mask || !metrics) {
        return;
    }
    
    memset(metrics, 0, sizeof(RecoveryMetrics));
    
    uint32_t num_corrupted = 0;
    uint32_t num_recovered = 0;
    double sum_confidence = 0.0;
    double sum_corrupted_confidence = 0.0;
    double min_confidence = 1.0;
    double max_confidence = 0.0;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        double conf = confidence_scores[i];
        sum_confidence += conf;
        
        if (conf < min_confidence) min_confidence = conf;
        if (conf > max_confidence) max_confidence = conf;
        
        if (corruption_mask[i]) {
            num_corrupted++;
            sum_corrupted_confidence += conf;
            
            // Consider recovered if confidence > 0.6 (reasonable threshold)
            if (conf > 0.6) {
                num_recovered++;
            }
        }
    }
    
    metrics->total_vertices = num_vertices;
    metrics->corrupted_vertices = num_corrupted;
    metrics->recovered_vertices = num_recovered;
    metrics->avg_confidence = sum_confidence / num_vertices;
    metrics->min_confidence = min_confidence;
    metrics->max_confidence = max_confidence;
    
    if (num_corrupted > 0) {
        metrics->avg_corrupted_confidence = sum_corrupted_confidence / num_corrupted;
        metrics->recovery_rate = (double)num_recovered / num_corrupted;
    } else {
        metrics->avg_corrupted_confidence = 1.0;
        metrics->recovery_rate = 1.0;
    }
    
    metrics->corruption_percentage = (double)num_corrupted / num_vertices;
}

/**
 * Validate recovery quality against thresholds
 */
bool validate_recovery_quality(
    const RecoveryMetrics* metrics,
    double min_recovery_rate,
    double min_avg_confidence
) {
    if (!metrics) {
        return false;
    }
    
    // Check recovery rate
    if (metrics->recovery_rate < min_recovery_rate) {
        return false;
    }
    
    // Check average confidence
    if (metrics->avg_confidence < min_avg_confidence) {
        return false;
    }
    
    // Check that corrupted vertices have reasonable confidence
    if (metrics->corrupted_vertices > 0 && 
        metrics->avg_corrupted_confidence < 0.7) {
        return false;
    }
    
    return true;
}

/**
 * Compute confidence score based on structural consistency
 * 
 * Checks if vertex satisfies Euler's formula and geometric constraints
 */
double compute_structural_confidence(
    const StructuralMap* structure,
    uint32_t vertex_id
) {
    if (!structure || vertex_id >= structure->num_vertices) {
        return 0.0;
    }
    
    // Check Euler's formula: V - E + F = 2
    int euler_result = (int)structure->num_vertices - 
                      (int)structure->num_edges + 
                      (int)structure->num_faces;
    
    bool euler_satisfied = (euler_result == 2);
    
    // Base confidence from Euler's formula
    double confidence = euler_satisfied ? 0.9 : 0.5;
    
    // Adjust based on dimensional offsets
    if (structure->dimensional_offsets) {
        double offset = structure->dimensional_offsets[vertex_id];
        // Lower confidence if offset is large
        confidence *= math_exp(-offset);
    }
    
    return confidence;
}

/**
 * Update confidence scores based on iterative refinement
 */
void update_confidence_iterative(
    double* confidence_scores,
    const double* previous_scores,
    uint32_t num_vertices,
    double learning_rate
) {
    if (!confidence_scores || !previous_scores) {
        return;
    }
    
    // Blend current and previous scores
    for (uint32_t i = 0; i < num_vertices; i++) {
        confidence_scores[i] = (1.0 - learning_rate) * previous_scores[i] +
                               learning_rate * confidence_scores[i];
    }
}
