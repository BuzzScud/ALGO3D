/**
 * @file kissing_spheres.h
 * @brief Kissing Sphere Operations for Geometric Recovery
 * 
 * Public API for kissing sphere system.
 */

#ifndef KISSING_SPHERES_H
#define KISSING_SPHERES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct KissingSystem KissingSystem;

/**
 * @brief Create kissing sphere system with optimal packing
 */
KissingSystem* kissing_create_optimal_system(double radius);

/**
 * @brief Free kissing system
 */
void kissing_free_system(KissingSystem* system);

/**
 * @brief Get number of contacts for a sphere
 */
uint32_t kissing_get_num_contacts(const KissingSystem* system, uint32_t sphere_id);

/**
 * @brief Verify optimal packing
 */
bool kissing_verify_optimal_packing(const KissingSystem* system);

/**
 * @brief Print kissing system statistics
 */
void kissing_print_stats(const KissingSystem* system);

#ifdef __cplusplus
}
#endif

#endif /* KISSING_SPHERES_H */