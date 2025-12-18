/**
 * @file tetration_attractors.c
 * @brief Tetration Attractor System for Geometric Recovery
 * 
 * Implements 186 tetration towers for convergence analysis.
 * Uses NEW math library exclusively - NO legacy dependencies.
 * 
 * Thesis Reference: Chapter 16 - Geometric Recovery: Convergence Theory
 * 
 * Algorithm:
 * - 6 bases: {2, 3, 5, 7, 11, 13}
 * - 31 depths: 29-59
 * - Total: 186 towers
 * - Logarithmic representation to avoid overflow
 */

#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/abacus.h"
#include "math/prime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define NUM_TETRATION_BASES 6
#define MIN_TETRATION_DEPTH 29
#define MAX_TETRATION_DEPTH 59
#define NUM_TETRATION_DEPTHS (MAX_TETRATION_DEPTH - MIN_TETRATION_DEPTH + 1)
#define NUM_TETRATION_TOWERS (NUM_TETRATION_BASES * NUM_TETRATION_DEPTHS)

static const uint32_t TETRATION_BASES[NUM_TETRATION_BASES] = {2, 3, 5, 7, 11, 13};

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Tetration tower structure
 * 
 * Represents a single tetration tower: base^base^base^... (depth times)
 * Uses logarithmic representation to avoid overflow.
 */
typedef struct {
    uint32_t base;              // Base (2, 3, 5, 7, 11, or 13)
    uint32_t depth;             // Depth (29-59)
    double log_value;           // log(tetration) = depth * log(base)
    double position[3];         // Position in 3D space
    double convergence_rate;    // Convergence rate
    bool is_attractor;          // Is this an attractor point?
} TetrationTower;

/**
 * @brief Tetration attractor system
 */
typedef struct {
    TetrationTower* towers;     // Array of 186 towers
    uint32_t num_towers;        // Should be 186
    double* distance_matrix;    // Distance matrix between towers
    uint32_t* nearest_neighbors; // Nearest neighbor indices
} TetrationSystem;

/* ============================================================================
 * INTERNAL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Compute tetration value using logarithmic representation
 * 
 * tetration(base, depth) ≈ exp(depth * log(base))
 * 
 * For large depths, we use logarithmic representation to avoid overflow.
 */
static double compute_tetration_log(uint32_t base, uint32_t depth) {
    // log(base^base^...^base) ≈ depth * log(base)
    // This is an approximation that works well for convergence analysis
    return (double)depth * math_log((double)base);
}

/**
 * @brief Map tetration tower to 3D position
 * 
 * Maps tower to position in 3D space based on:
 * - x: base index (0-5)
 * - y: depth (29-59)
 * - z: log value
 */
static void map_tower_to_position(TetrationTower* tower) {
    // Find base index
    uint32_t base_idx = 0;
    for (uint32_t i = 0; i < NUM_TETRATION_BASES; i++) {
        if (TETRATION_BASES[i] == tower->base) {
            base_idx = i;
            break;
        }
    }
    
    // Map to 3D space
    tower->position[0] = (double)base_idx;
    tower->position[1] = (double)tower->depth;
    tower->position[2] = tower->log_value;
}

/**
 * @brief Compute convergence rate for tower
 * 
 * Convergence rate = 1 / log_value
 * Smaller log values converge faster
 */
static double compute_convergence_rate(const TetrationTower* tower) {
    if (tower->log_value < 1e-10) {
        return 1e10; // Very fast convergence
    }
    return 1.0 / tower->log_value;
}

/**
 * @brief Determine if tower is an attractor
 * 
 * A tower is an attractor if its convergence rate is above threshold
 */
static bool is_attractor_point(const TetrationTower* tower) {
    // Attractors have high convergence rates
    // Threshold: convergence_rate > 0.1
    return tower->convergence_rate > 0.1;
}

/**
 * @brief Compute distance between two towers
 */
static double compute_tower_distance(const TetrationTower* a, const TetrationTower* b) {
    double dx = a->position[0] - b->position[0];
    double dy = a->position[1] - b->position[1];
    double dz = a->position[2] - b->position[2];
    return math_sqrt(dx*dx + dy*dy + dz*dz);
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Create tetration attractor system
 * 
 * Creates 186 tetration towers (6 bases × 31 depths)
 * 
 * @return Tetration system, or NULL on error
 */
TetrationSystem* tetration_create_system(void) {
    TetrationSystem* system = (TetrationSystem*)calloc(1, sizeof(TetrationSystem));
    if (!system) {
        return NULL;
    }
    
    // Allocate towers
    system->num_towers = NUM_TETRATION_TOWERS;
    system->towers = (TetrationTower*)calloc(NUM_TETRATION_TOWERS, sizeof(TetrationTower));
    if (!system->towers) {
        free(system);
        return NULL;
    }
    
    // Create towers
    uint32_t tower_idx = 0;
    for (uint32_t base_idx = 0; base_idx < NUM_TETRATION_BASES; base_idx++) {
        uint32_t base = TETRATION_BASES[base_idx];
        
        for (uint32_t depth = MIN_TETRATION_DEPTH; depth <= MAX_TETRATION_DEPTH; depth++) {
            TetrationTower* tower = &system->towers[tower_idx++];
            
            // Set basic properties
            tower->base = base;
            tower->depth = depth;
            
            // Compute logarithmic value
            tower->log_value = compute_tetration_log(base, depth);
            
            // Map to 3D position
            map_tower_to_position(tower);
            
            // Compute convergence rate
            tower->convergence_rate = compute_convergence_rate(tower);
            
            // Determine if attractor
            tower->is_attractor = is_attractor_point(tower);
        }
    }
    
    // Allocate distance matrix (186 × 186)
    size_t matrix_size = NUM_TETRATION_TOWERS * NUM_TETRATION_TOWERS;
    system->distance_matrix = (double*)calloc(matrix_size, sizeof(double));
    if (!system->distance_matrix) {
        free(system->towers);
        free(system);
        return NULL;
    }
    
    // Compute distance matrix
    for (uint32_t i = 0; i < NUM_TETRATION_TOWERS; i++) {
        for (uint32_t j = 0; j < NUM_TETRATION_TOWERS; j++) {
            double dist = compute_tower_distance(&system->towers[i], &system->towers[j]);
            system->distance_matrix[i * NUM_TETRATION_TOWERS + j] = dist;
        }
    }
    
    // Allocate nearest neighbors
    system->nearest_neighbors = (uint32_t*)calloc(NUM_TETRATION_TOWERS, sizeof(uint32_t));
    if (!system->nearest_neighbors) {
        free(system->distance_matrix);
        free(system->towers);
        free(system);
        return NULL;
    }
    
    // Find nearest neighbor for each tower
    for (uint32_t i = 0; i < NUM_TETRATION_TOWERS; i++) {
        double min_dist = 1e100;
        uint32_t nearest = i;
        
        for (uint32_t j = 0; j < NUM_TETRATION_TOWERS; j++) {
            if (i == j) continue;
            
            double dist = system->distance_matrix[i * NUM_TETRATION_TOWERS + j];
            if (dist < min_dist) {
                min_dist = dist;
                nearest = j;
            }
        }
        
        system->nearest_neighbors[i] = nearest;
    }
    
    return system;
}

/**
 * @brief Free tetration system
 */
void tetration_free_system(TetrationSystem* system) {
    if (!system) return;
    
    if (system->towers) free(system->towers);
    if (system->distance_matrix) free(system->distance_matrix);
    if (system->nearest_neighbors) free(system->nearest_neighbors);
    
    free(system);
}

/**
 * @brief Get tower by index
 */
const TetrationTower* tetration_get_tower(const TetrationSystem* system, uint32_t index) {
    if (!system || index >= system->num_towers) {
        return NULL;
    }
    return &system->towers[index];
}

/**
 * @brief Find nearest attractor to a point
 * 
 * @param system Tetration system
 * @param point Point in 3D space
 * @return Index of nearest attractor, or -1 if none found
 */
int tetration_find_nearest_attractor(const TetrationSystem* system, const double point[3]) {
    if (!system) return -1;
    
    double min_dist = 1e100;
    int nearest = -1;
    
    for (uint32_t i = 0; i < system->num_towers; i++) {
        const TetrationTower* tower = &system->towers[i];
        
        // Only consider attractors
        if (!tower->is_attractor) continue;
        
        // Compute distance
        double dx = tower->position[0] - point[0];
        double dy = tower->position[1] - point[1];
        double dz = tower->position[2] - point[2];
        double dist = math_sqrt(dx*dx + dy*dy + dz*dz);
        
        if (dist < min_dist) {
            min_dist = dist;
            nearest = (int)i;
        }
    }
    
    return nearest;
}

/**
 * @brief Get convergence trajectory from point to nearest attractor
 * 
 * @param system Tetration system
 * @param start_point Starting point
 * @param trajectory Output trajectory (array of 3D points)
 * @param max_steps Maximum number of steps
 * @return Number of steps taken, or -1 on error
 */
int tetration_get_convergence_trajectory(
    const TetrationSystem* system,
    const double start_point[3],
    double (*trajectory)[3],
    uint32_t max_steps
) {
    if (!system || !trajectory || max_steps == 0) {
        return -1;
    }
    
    // Copy start point
    trajectory[0][0] = start_point[0];
    trajectory[0][1] = start_point[1];
    trajectory[0][2] = start_point[2];
    
    uint32_t step = 0;
    double current[3] = {start_point[0], start_point[1], start_point[2]};
    
    while (step < max_steps - 1) {
        // Find nearest attractor
        int nearest = tetration_find_nearest_attractor(system, current);
        if (nearest < 0) break;
        
        const TetrationTower* attractor = &system->towers[nearest];
        
        // Move toward attractor
        double dx = attractor->position[0] - current[0];
        double dy = attractor->position[1] - current[1];
        double dz = attractor->position[2] - current[2];
        double dist = math_sqrt(dx*dx + dy*dy + dz*dz);
        
        // Check convergence
        if (dist < 0.01) break;
        
        // Step size based on convergence rate
        double step_size = 0.1 * attractor->convergence_rate;
        if (step_size > 1.0) step_size = 1.0;
        
        // Update position
        current[0] += step_size * dx / dist;
        current[1] += step_size * dy / dist;
        current[2] += step_size * dz / dist;
        
        // Store in trajectory
        step++;
        trajectory[step][0] = current[0];
        trajectory[step][1] = current[1];
        trajectory[step][2] = current[2];
    }
    
    return (int)(step + 1);
}

/**
 * @brief Print tetration system statistics
 */
void tetration_print_stats(const TetrationSystem* system) {
    if (!system) return;
    
    printf("=== Tetration Attractor System ===\n");
    printf("Total towers: %u\n", system->num_towers);
    
    // Count attractors
    uint32_t num_attractors = 0;
    for (uint32_t i = 0; i < system->num_towers; i++) {
        if (system->towers[i].is_attractor) {
            num_attractors++;
        }
    }
    printf("Attractors: %u (%.1f%%)\n", num_attractors, 
           100.0 * num_attractors / system->num_towers);
    
    // Average convergence rate
    double avg_convergence = 0.0;
    for (uint32_t i = 0; i < system->num_towers; i++) {
        avg_convergence += system->towers[i].convergence_rate;
    }
    avg_convergence /= system->num_towers;
    printf("Average convergence rate: %.6f\n", avg_convergence);
    
    printf("==================================\n");
}