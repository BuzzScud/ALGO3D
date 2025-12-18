/**
 * @file tetration_attractors.h
 * @brief Tetration Attractor System for Geometric Recovery
 * 
 * Public API for tetration attractor system.
 */

#ifndef TETRATION_ATTRACTORS_H
#define TETRATION_ATTRACTORS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct TetrationTower TetrationTower;
typedef struct TetrationSystem TetrationSystem;

/**
 * @brief Create tetration attractor system
 * 
 * Creates 186 tetration towers (6 bases × 31 depths)
 * 
 * @return Tetration system, or NULL on error
 */
TetrationSystem* tetration_create_system(void);

/**
 * @brief Free tetration system
 */
void tetration_free_system(TetrationSystem* system);

/**
 * @brief Get tower by index
 */
const TetrationTower* tetration_get_tower(const TetrationSystem* system, uint32_t index);

/**
 * @brief Find nearest attractor to a point
 * 
 * @param system Tetration system
 * @param point Point in 3D space
 * @return Index of nearest attractor, or -1 if none found
 */
int tetration_find_nearest_attractor(const TetrationSystem* system, const double point[3]);

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
);

/**
 * @brief Print tetration system statistics
 */
void tetration_print_stats(const TetrationSystem* system);

#ifdef __cplusplus
}
#endif

#endif /* TETRATION_ATTRACTORS_H */