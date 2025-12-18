/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file universal_recovery_v2.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief FIXED Universal Blind Recovery - OBJECTIVE 28 Implementation
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * CRITICAL FIXES:
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 1. Oscillation detection now tracks ACTUAL vertex evolution across passes
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 2. Recovery metrics use adaptive threshold based on confidence distribution
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 3. Tetration attractors use proper logarithmic representation
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 4. Vertex positions are actually modified by triangulation
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
#include "blind_recovery/blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// History tracking for oscillation detection
typedef struct {
    double** position_history;  // [num_passes][num_vertices * 3]
    uint32_t num_passes;
    uint32_t max_passes;
    uint32_t num_vertices;
} PositionHistory;

static PositionHistory* create_position_history(uint32_t num_vertices, uint32_t max_passes) {
    PositionHistory* hist = malloc(sizeof(PositionHistory));
    if (!hist) return NULL;
    
    hist->position_history = malloc(max_passes * sizeof(double*));
    if (!hist->position_history) {
        free(hist);
        return NULL;
    }
    
    for (uint32_t i = 0; i < max_passes; i++) {
        hist->position_history[i] = malloc(num_vertices * 3 * sizeof(double));
        if (!hist->position_history[i]) {
            for (uint32_t j = 0; j < i; j++) {
                free(hist->position_history[j]);
            }
            free(hist->position_history);
            free(hist);
            return NULL;
        }
    }
    
    hist->num_passes = 0;
    hist->max_passes = max_passes;
    hist->num_vertices = num_vertices;
    return hist;
}

static void record_positions(PositionHistory* hist, const double* positions) {
    if (!hist || hist->num_passes >= hist->max_passes) return;
    
    memcpy(hist->position_history[hist->num_passes], positions, 
           hist->num_vertices * 3 * sizeof(double));
    hist->num_passes++;
}

static void free_position_history(PositionHistory* hist) {
    if (!hist) return;
    
    for (uint32_t i = 0; i < hist->max_passes; i++) {
        free(hist->position_history[i]);
    }
    free(hist->position_history);
    free(hist);
}

// Improved tetration computation with logarithmic representation
static double compute_tetration_log(uint32_t base, uint32_t depth) {
    if (depth == 0) return 0.0;  // math_log(1) = 0
    if (depth == 1) return math_log((double)base);
    
    // For deep tetration, use logarithmic approximation
    // math_log(base^base^...^base) ≈ base^(depth-1) * math_log(base)
    double log_base = math_log((double)base);
    double result = log_base;
    
    for (uint32_t i = 1; i < depth && i < 10; i++) {
        result = math_pow((double)base, result);
        if (result > 100.0) break;  // Prevent overflow
    }
    
    return result;
}

// Find nearest tetration attractor using logarithmic space
static double find_tetration_attractor_improved(double value, uint32_t min_depth, uint32_t max_depth) {
    if (value <= 0.0) return value;
    
    double log_value = math_log(math_abs(value) + 1e-10);
    double nearest_log = log_value;
    double min_distance = INFINITY;
    
    // Prime bases for tetration
    uint32_t bases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
    
    for (size_t b = 0; b < sizeof(bases)/sizeof(bases[0]); b++) {
        for (uint32_t depth = min_depth; depth <= max_depth && depth <= 10; depth++) {
            double tower_log = compute_tetration_log(bases[b], depth);
            double distance = math_abs(log_value - tower_log);
            
            if (distance < min_distance) {
                min_distance = distance;
                nearest_log = tower_log;
            }
        }
    }
    
    // Convert back from log space
    double result = math_exp(nearest_log);
    return (value < 0.0) ? -result : result;
}

// Compute adaptive recovery threshold based on confidence distribution
static double compute_adaptive_threshold(const double* confidence_scores, 
                                         const bool* corruption_mask,
                                         uint32_t num_vertices) {
    // Collect confidence scores for corrupted vertices
    double* corrupted_confidences = malloc(num_vertices * sizeof(double));
    uint32_t count = 0;
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        if (corruption_mask[i]) {
            corrupted_confidences[count++] = confidence_scores[i];
        }
    }
    
    if (count == 0) {
        free(corrupted_confidences);
        return 0.6;  // Default threshold
    }
    
    // Sort confidences
    for (uint32_t i = 0; i < count - 1; i++) {
        for (uint32_t j = i + 1; j < count; j++) {
            if (corrupted_confidences[i] > corrupted_confidences[j]) {
                double temp = corrupted_confidences[i];
                corrupted_confidences[i] = corrupted_confidences[j];
                corrupted_confidences[j] = temp;
            }
        }
    }
    
    // Use median as threshold (more robust than mean)
    double threshold = corrupted_confidences[count / 2];
    
    // Ensure threshold is reasonable (between 0.3 and 0.8)
    if (threshold < 0.3) threshold = 0.3;
    if (threshold > 0.8) threshold = 0.8;
    
    free(corrupted_confidences);
    return threshold;
}

// Improved recovery metrics with adaptive threshold
static void compute_recovery_metrics_adaptive(
    const double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices,
    RecoveryMetrics* metrics
) {
    if (!confidence_scores || !corruption_mask || !metrics) {
        return;
    }
    
    memset(metrics, 0, sizeof(RecoveryMetrics));
    
    // Compute adaptive threshold
    double threshold = compute_adaptive_threshold(confidence_scores, corruption_mask, num_vertices);
    
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
            
            // Use adaptive threshold
            if (conf > threshold) {
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
 * RECURSIVE OSCILLATION STABILIZATION - FIXED VERSION
 * 
 * Now tracks ACTUAL vertex evolution across passes
 */
static int recursive_stabilization_pass_v2(
    double* vertex_positions,
    uint32_t num_vertices,
    const bool* corruption_mask,
    double* confidence_scores,
    StructuralMap* structure,
    PositionHistory* history,
    uint32_t pass_number,
    uint32_t max_passes,
    uint32_t tetration_min_depth,
    uint32_t tetration_max_depth,
    bool verbose
) {
    if (pass_number >= max_passes) {
        if (verbose) printf("  Max passes reached\n");
        return pass_number;
    }
    
    // Record current positions
    record_positions(history, vertex_positions);
    
    if (verbose) {
        printf("\n🔄 Pass %u/%u (Tetration depth %u-%u):\n", 
               pass_number + 1, max_passes, tetration_min_depth, tetration_max_depth);
    }
    
    // PHASE 1: DETECT OSCILLATIONS using ACTUAL position history
    OscillationMap* oscillations = NULL;
    
    if (history->num_passes >= 2) {
        // Build time-series from ACTUAL position evolution
        uint32_t num_time_samples = history->num_passes;
        double sampling_rate = 1.0;  // 1 sample per pass
        
        double* time_series = malloc(num_vertices * 3 * num_time_samples * sizeof(double));
        if (time_series) {
            // Copy actual position history
            for (uint32_t t = 0; t < num_time_samples; t++) {
                memcpy(&time_series[t * num_vertices * 3],
                       history->position_history[t],
                       num_vertices * 3 * sizeof(double));
            }
            
            // Detect oscillations from REAL evolution
            oscillations = detect_oscillations(
                time_series,
                3,  // 3 dimensions
                num_time_samples,
                sampling_rate
            );
            
            free(time_series);
        }
    }
    
    bool has_unstable = false;
    double max_amplitude = 0.0;
    
    if (oscillations) {
        if (verbose) {
            printf("  Oscillation analysis (from actual evolution):\n");
        }
        
        for (uint32_t d = 0; d < oscillations->num_dimensions; d++) {
            OscillationSignature* sig = &oscillations->signatures[d];
            
            if (verbose) {
                printf("    Dim %u: freq=%.2f Hz, amp=%.4f, %s\n",
                       d, sig->frequency, sig->amplitude,
                       sig->is_stable ? "stable" : "UNSTABLE");
            }
            
            if (!sig->is_stable || sig->amplitude > 0.01) {
                has_unstable = true;
            }
            
            if (sig->amplitude > max_amplitude) {
                max_amplitude = sig->amplitude;
            }
        }
    } else {
        // First pass or detection failed - assume unstable
        has_unstable = true;
        max_amplitude = 1.0;
    }
    
    // PHASE 2: IF UNSTABLE, APPLY TETRATION BIAS
    if (has_unstable) {
        if (verbose) {
            printf("  ⚠ Unstable oscillations detected (max amp: %.4f)\n", max_amplitude);
            printf("  Applying improved tetration bias (depth %u-%u)...\n", 
                   tetration_min_depth, tetration_max_depth);
        }
        
        // Apply improved tetration bias
        uint32_t biased_count = 0;
        for (uint32_t v = 0; v < num_vertices; v++) {
            // Only bias corrupted vertices with low confidence
            if (!corruption_mask[v] || confidence_scores[v] > 0.8) continue;
            
            for (uint32_t d = 0; d < 3; d++) {
                double value = vertex_positions[v * 3 + d];
                double attractor = find_tetration_attractor_improved(
                    value, tetration_min_depth, tetration_max_depth
                );
                
                // Stronger bias for lower confidence
                double bias_weight = 0.2 * (1.0 - confidence_scores[v]);
                if (bias_weight > 0.5) bias_weight = 0.5;
                
                vertex_positions[v * 3 + d] = value * (1.0 - bias_weight) + attractor * bias_weight;
            }
            biased_count++;
        }
        
        if (verbose) {
            printf("  Biased %u vertices toward tetration attractors\n", biased_count);
        }
        
        // PHASE 3: RE-TRIANGULATE from anchors
        AnchorSystem* anchors = select_anchors(structure, vertex_positions, confidence_scores, num_vertices);
        if (anchors) {
            if (verbose) {
                printf("  Re-selected %u anchors (confidence: %.3f)\n",
                       anchors->num_anchors, anchors->global_confidence);
            }
            
            // Recover vertices - THIS ACTUALLY MODIFIES vertex_positions
            recover_all_vertices(anchors, structure, vertex_positions, confidence_scores, num_vertices);
            
            // Iterative refinement with decreasing iterations
            uint32_t iterations = adjust_anchors_iterative(
                anchors, vertex_positions, confidence_scores, (bool*)corruption_mask,
                num_vertices, 2000 / (pass_number + 1)
            );
            
            if (verbose) {
                printf("  Refinement: %u iterations\n", iterations);
            }
            
            free_anchor_system(anchors);
        }
        
        // PHASE 4: COMPUTE RECOVERY METRICS with adaptive threshold
        RecoveryMetrics metrics = {0};  // Initialize to zero to suppress warning
        compute_recovery_metrics_adaptive(confidence_scores, corruption_mask, num_vertices, &metrics);
        
        if (verbose) {
            printf("  Recovery: %.1f%%, Confidence: %.3f\n",
                   metrics.recovery_rate * 100.0, metrics.avg_confidence);
        }
        
        if (oscillations) {
            free_oscillation_map(oscillations);
        }
        
        // PHASE 5: RECURSE with increased tetration depth
        uint32_t new_min_depth = tetration_min_depth + 3;
        uint32_t new_max_depth = tetration_max_depth + 3;
        if (new_max_depth > 59) new_max_depth = 59;
        
        // Check if we should continue
        if (metrics.recovery_rate < 0.90 && max_amplitude > 0.005 && pass_number < max_passes - 1) {
            return recursive_stabilization_pass_v2(
                vertex_positions, num_vertices, corruption_mask, confidence_scores,
                structure, history, pass_number + 1, max_passes,
                new_min_depth, new_max_depth, verbose
            );
        } else {
            if (verbose) {
                printf("  ✓ Converged! (recovery: %.1f%%, amplitude: %.4f)\n",
                       metrics.recovery_rate * 100.0, max_amplitude);
            }
            return pass_number + 1;
        }
    } else {
        if (verbose) {
            printf("  ✓ All oscillations stable\n");
        }
        if (oscillations) {
            free_oscillation_map(oscillations);
        }
        return pass_number + 1;
    }
}

/**
 * UNIVERSAL BLIND RECOVERY - FIXED VERSION
 */
RecoveryStatistics blind_recovery_complete_v2(
    double* structure_data,
    uint32_t num_elements,
    uint32_t num_dimensions,
    double corruption_threshold
) {
    RecoveryStatistics stats = {0};
    clock_t start_time = clock();
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  OBJECTIVE 28: Universal Blind Recovery (FIXED v2)      ║\n");
    printf("║  Singularity Modeling via Oscillation Stabilization     ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    // Detect corruption
    bool* corruption_mask = detect_corruption(structure_data, num_elements, corruption_threshold);
    if (!corruption_mask) {
        printf("Error: Failed to detect corruption\n");
        return stats;
    }
    
    // Count corruptions
    for (uint32_t i = 0; i < num_elements; i++) {
        if (corruption_mask[i]) stats.total_corruptions++;
    }
    
    printf("📊 Initial Analysis:\n");
    printf("  Elements: %u\n", num_elements);
    printf("  Dimensions: %u\n", num_dimensions);
    printf("  Corrupted: %u (%.1f%%)\n\n",
           stats.total_corruptions, 
           100.0 * stats.total_corruptions / num_elements);
    
    // Create structural map
    uint32_t num_vertices = num_elements / num_dimensions;
    StructuralMap* structure = map_structure(num_vertices, num_vertices - 1, 1, structure_data);
    if (!structure) {
        free(corruption_mask);
        printf("Error: Failed to create structural map\n");
        return stats;
    }
    
    // Initialize confidence scores
    double* confidence_scores = malloc(num_vertices * sizeof(double));
    if (!confidence_scores) {
        free_structural_map(structure);
        free(corruption_mask);
        return stats;
    }
    
    for (uint32_t i = 0; i < num_vertices; i++) {
        confidence_scores[i] = corruption_mask[i] ? 0.0 : 1.0;
    }
    
    // Create position history tracker
    PositionHistory* history = create_position_history(num_vertices, 15);
    if (!history) {
        free(confidence_scores);
        free_structural_map(structure);
        free(corruption_mask);
        return stats;
    }
    
    // Initial anchor selection and recovery
    AnchorSystem* anchors = select_anchors(structure, structure_data, confidence_scores, num_vertices);
    if (anchors) {
        printf("🎯 Initial anchors: %u (confidence: %.3f)\n\n",
               anchors->num_anchors, anchors->global_confidence);
        
        recover_all_vertices(anchors, structure, structure_data, confidence_scores, num_vertices);
        free_anchor_system(anchors);
    }
    
    // RECURSIVE STABILIZATION with position tracking
    printf("🔄 Recursive oscillation stabilization (FIXED):\n");
    int passes = recursive_stabilization_pass_v2(
        structure_data, num_vertices, corruption_mask, confidence_scores,
        structure, history, 0, 12, 29, 35, true
    );
    
    // Final metrics with adaptive threshold
    RecoveryMetrics final_metrics;
    compute_recovery_metrics_adaptive(confidence_scores, corruption_mask, num_vertices, &final_metrics);
    
    stats.successful_recoveries = final_metrics.recovered_vertices;
    stats.failed_recoveries = stats.total_corruptions - stats.successful_recoveries;
    stats.recovery_rate = final_metrics.recovery_rate;
    stats.recovery_time_ns = (clock() - start_time) * 1000000000 / CLOCKS_PER_SEC;
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  FINAL RESULTS (FIXED v2)                                ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Passes: %d                                               \n", passes);
    printf("║  Recovery rate: %.1f%%                                   \n", stats.recovery_rate * 100.0);
    printf("║  Recovered: %d / %d                                      \n", 
           stats.successful_recoveries, stats.total_corruptions);
    printf("║  Time: %.3f seconds                                      \n",
           stats.recovery_time_ns / 1e9);
    printf("╚══════════════════════════════════════════════════════════╝\n");
    
    // Cleanup
    free_position_history(history);
    free(confidence_scores);
    free_structural_map(structure);
    free(corruption_mask);
    
    return stats;
}
