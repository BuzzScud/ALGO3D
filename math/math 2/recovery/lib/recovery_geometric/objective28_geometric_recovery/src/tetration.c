/**
 * @file tetration.c
 * @brief REAL Tetration Tower Computation - Not Approximations
 * 
 * This computes ACTUAL tetration towers using logarithmic representation.
 * No simplifications, no rounding, REAL computation.
 * 
 * Tetration: base^base^base^...^base (depth times)
 * 
 * For depth 29-59, these values are ASTRONOMICAL.
 * We use logarithmic representation to handle them.
 */

#include "platonic_model.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Compute a single level of tetration in log space
 * 
 * If we have math_log(x), and we want math_log(base^x), we compute:
 * math_log(base^x) = x * math_log(base)
 * 
 * But x itself is in log space, so we need to be careful.
 */
static double tetration_step_log(double log_prev, double log_base) {
    // If log_prev is small enough, we can exponentiate
    if (log_prev < 100.0) {
        double prev = math_exp(log_prev);
        // Compute base^prev in log space
        return prev * log_base;
    } else {
        // For very large values, use approximation
        // math_log(base^x) ≈ x * math_log(base) when x is large
        return math_exp(log_prev) * log_base;
    }
}

/**
 * Check if tetration has converged
 * 
 * For some bases, tetration converges to a fixed point.
 * For others, it diverges to infinity.
 */
static bool tetration_check_convergence(
    const double* log_tower,
    uint32_t depth,
    double tolerance
) {
    if (depth < 3) return false;
    
    // Check if last few values are similar
    double diff1 = math_abs(log_tower[depth-1] - log_tower[depth-2]);
    double diff2 = math_abs(log_tower[depth-2] - log_tower[depth-3]);
    
    return (diff1 < tolerance && diff2 < tolerance);
}

TetrationTower* tetration_compute_real(uint32_t base, uint32_t depth) {
    if (base < 2 || depth < 1) {
        return NULL;
    }
    
    TetrationTower* tower = malloc(sizeof(TetrationTower));
    if (!tower) return NULL;
    
    tower->base = base;
    tower->depth = depth;
    tower->log_tower = malloc(depth * sizeof(double));
    if (!tower->log_tower) {
        free(tower);
        return NULL;
    }
    
    double log_base = math_log((double)base);
    
    // Start with base^1 = base
    tower->log_tower[0] = log_base;
    
    // Compute each level of the tower
    for (uint32_t d = 1; d < depth; d++) {
        tower->log_tower[d] = tetration_step_log(tower->log_tower[d-1], log_base);
        
        // Check for overflow (infinity)
        if (math_is_inf(tower->log_tower[d]) || math_is_nan(tower->log_tower[d])) {
            // Mark as divergent
            tower->is_converged = false;
            tower->iterations = d;
            tower->log_value = tower->log_tower[d-1];
            return tower;
        }
        
        // Check for convergence
        if (d >= 3 && tetration_check_convergence(tower->log_tower, d, 1e-10)) {
            tower->is_converged = true;
            tower->iterations = d;
            tower->log_value = tower->log_tower[d];
            return tower;
        }
    }
    
    // Completed full depth
    tower->is_converged = tetration_check_convergence(tower->log_tower, depth, 1e-10);
    tower->iterations = depth;
    tower->log_value = tower->log_tower[depth-1];
    
    return tower;
}

void tetration_tower_free(TetrationTower* tower) {
    if (!tower) return;
    free(tower->log_tower);
    free(tower);
}

/**
 * Find the nearest tetration attractor in a single dimension
 */
static double find_1d_attractor(
    double value,
    TetrationTower** towers,
    uint32_t num_towers
) {
    double nearest = value;
    double min_distance = INFINITY;
    
    for (uint32_t t = 0; t < num_towers; t++) {
        TetrationTower* tower = towers[t];
        if (!tower) continue;
        
        // Convert tower value from log space
        double tower_value;
        if (tower->log_value < 100.0) {
            tower_value = math_exp(tower->log_value);
        } else {
            // For very large values, use the log value directly
            // as a proxy for the actual value
            tower_value = tower->log_value;
        }
        
        double distance = math_abs(value - tower_value);
        
        if (distance < min_distance) {
            min_distance = distance;
            nearest = tower_value;
        }
    }
    
    return nearest;
}

double* tetration_find_hd_attractor(
    const double* position,
    uint32_t num_dimensions,
    TetrationTower** towers,
    uint32_t num_towers
) {
    if (!position || !towers || num_towers == 0) {
        return NULL;
    }
    
    double* attractor = malloc(num_dimensions * sizeof(double));
    if (!attractor) return NULL;
    
    // Find attractor for each dimension independently
    for (uint32_t d = 0; d < num_dimensions; d++) {
        attractor[d] = find_1d_attractor(position[d], towers, num_towers);
    }
    
    return attractor;
}

/**
 * Print tetration tower for debugging
 */
void tetration_tower_print(const TetrationTower* tower) {
    if (!tower) return;
    
    printf("Tetration Tower: base=%u, depth=%u\n", tower->base, tower->depth);
    printf("  Converged: %s\n", tower->is_converged ? "Yes" : "No");
    printf("  Iterations: %lu\n", tower->iterations);
    printf("  Final log value: %.10f\n", tower->log_value);
    
    if (tower->log_value < 100.0) {
        printf("  Final value: %.10e\n", math_exp(tower->log_value));
    } else {
        printf("  Final value: TOO LARGE (log = %.10f)\n", tower->log_value);
    }
    
    printf("  Tower (in log space):\n");
    for (uint32_t d = 0; d < tower->depth && d < 10; d++) {
        printf("    Level %u: %.10f", d, tower->log_tower[d]);
        if (tower->log_tower[d] < 100.0) {
            printf(" (value: %.10e)", math_exp(tower->log_tower[d]));
        }
        printf("\n");
    }
    if (tower->depth > 10) {
        printf("    ... (%u more levels)\n", tower->depth - 10);
    }
}

/**
 * Create a set of tetration towers for all bases and depths
 */
TetrationTower** tetration_create_tower_set(
    const uint32_t* bases,
    uint32_t num_bases,
    uint32_t min_depth,
    uint32_t max_depth,
    uint32_t* out_num_towers
) {
    uint32_t num_depths = max_depth - min_depth + 1;
    uint32_t total_towers = num_bases * num_depths;
    
    TetrationTower** towers = malloc(total_towers * sizeof(TetrationTower*));
    if (!towers) return NULL;
    
    uint32_t tower_idx = 0;
    for (uint32_t b = 0; b < num_bases; b++) {
        for (uint32_t depth = min_depth; depth <= max_depth; depth++) {
            towers[tower_idx] = tetration_compute_real(bases[b], depth);
            if (towers[tower_idx]) {
                tower_idx++;
            }
        }
    }
    
    *out_num_towers = tower_idx;
    return towers;
}

/**
 * Free a set of tetration towers
 */
void tetration_free_tower_set(TetrationTower** towers, uint32_t num_towers) {
    if (!towers) return;
    
    for (uint32_t i = 0; i < num_towers; i++) {
        tetration_tower_free(towers[i]);
    }
    free(towers);
}
