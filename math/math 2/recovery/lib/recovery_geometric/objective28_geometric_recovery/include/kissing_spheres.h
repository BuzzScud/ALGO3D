#ifndef KISSING_SPHERES_H
#define KISSING_SPHERES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Kissing Spheres Hierarchy
 * 
 * Implements infinite self-similar kissing sphere structure:
 * - Each sphere has 12 neighbors (icosahedral packing)
 * - Recursive depth creates fractal hierarchy
 * - Golden ratio scaling at each level
 * - Used for anchor positioning in ECDLP recovery
 */

#define NUM_KISSING_NEIGHBORS 12  // Icosahedral packing

// Forward declaration
typedef struct KissingSphere KissingSphere;

// Kissing sphere structure (recursive)
struct KissingSphere {
    uint64_t center;                    // Center position (k value)
    double radius;                      // Sphere radius
    uint32_t depth;                     // Recursion depth
    KissingSphere** neighbors;          // 12 kissing neighbors
    uint32_t num_neighbors;             // Actual number of neighbors
    double* position_3d;                // 3D position [x, y, z]
    bool is_anchor;                     // Is this an anchor point?
    double confidence;                  // Confidence score [0, 1]
};

/**
 * Create kissing sphere hierarchy
 * 
 * @param center Center k value
 * @param depth Recursion depth (3-5 typical)
 * @return Root sphere with full hierarchy
 */
KissingSphere* create_kissing_sphere_hierarchy(uint64_t center, uint32_t depth);

/**
 * Free kissing sphere hierarchy (recursive)
 */
void free_kissing_sphere(KissingSphere* sphere);

/**
 * Compute radius for sphere at given depth
 * Uses golden ratio scaling
 */
double compute_sphere_radius(uint64_t k, uint32_t depth);

/**
 * Compute neighbor k value for icosahedral position
 * 
 * @param center Center k value
 * @param neighbor_idx Neighbor index (0-11)
 * @param depth Current depth
 * @return Neighbor k value
 */
uint64_t compute_neighbor_k(uint64_t center, uint32_t neighbor_idx, uint32_t depth);

/**
 * Compute 3D position for sphere using icosahedral geometry
 * 
 * @param k K value
 * @param depth Depth in hierarchy
 * @return 3D position [x, y, z] (caller must free)
 */
double* compute_3d_position(uint64_t k, uint32_t depth);

/**
 * Find nearest sphere to target k value
 * 
 * @param root Root of hierarchy
 * @param target Target k value
 * @return Nearest sphere (or NULL)
 */
KissingSphere* find_nearest_sphere(KissingSphere* root, uint64_t target);

/**
 * Find N nearest spheres to target
 * 
 * @param root Root of hierarchy
 * @param target Target k value
 * @param n Number of spheres to find
 * @param out_spheres Output array (caller must allocate)
 * @return Number of spheres found
 */
uint32_t find_n_nearest_spheres(KissingSphere* root, uint64_t target, 
                                uint32_t n, KissingSphere** out_spheres);

/**
 * Mark sphere as anchor point
 */
void mark_as_anchor(KissingSphere* sphere, double confidence);

/**
 * Get all anchor points from hierarchy
 * 
 * @param root Root of hierarchy
 * @param out_anchors Output array (caller must allocate)
 * @param max_anchors Maximum number of anchors
 * @return Number of anchors found
 */
uint32_t get_all_anchors(KissingSphere* root, KissingSphere** out_anchors, 
                         uint32_t max_anchors);

/**
 * Compute distance between two spheres
 */
double sphere_distance(const KissingSphere* a, const KissingSphere* b);

/**
 * Check if two spheres are kissing (tangent)
 */
bool spheres_kissing(const KissingSphere* a, const KissingSphere* b);

/**
 * Traverse hierarchy and apply function to each sphere
 * 
 * @param root Root sphere
 * @param func Function to apply (returns false to stop traversal)
 * @param user_data User data passed to function
 */
void traverse_hierarchy(KissingSphere* root, 
                       bool (*func)(KissingSphere*, void*),
                       void* user_data);

/**
 * Count total spheres in hierarchy
 */
uint32_t count_spheres(KissingSphere* root);

/**
 * Print hierarchy structure (for debugging)
 */
void print_hierarchy(KissingSphere* root, uint32_t indent);

#endif // KISSING_SPHERES_H
