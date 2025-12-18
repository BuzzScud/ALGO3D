#ifndef RECURSIVE_RECOVERY_H
#define RECURSIVE_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

// π×φ constant
#define PI_PHI 5.083203692315259

// Torus state structure
typedef struct {
    double center[13];           // Torus center in 13D
    double major_radius;         // Distance from origin (π×φ metric)
    double minor_radius;         // Tube radius (π×φ metric)
    double entropy;              // Stability measure
    bool is_stable;              // Entropy < threshold
    uint32_t resolution_level;   // Current graph resolution
    uint32_t num_anchors;        // Current anchor count
    uint32_t num_dimensions;     // Current dimensions
    double uniform_percentage;   // % of uniform π×φ distances
    int recursion_depth;         // Current recursion level
} TorusState;

// Recursion parameters
typedef struct {
    uint32_t min_resolution;     // Starting resolution (2^16)
    uint32_t max_resolution;     // Maximum resolution (2^32)
    uint32_t min_anchors;        // Starting anchors (100)
    uint32_t max_anchors;        // Maximum anchors (10000)
    uint32_t min_dimensions;     // Starting dimensions (13)
    uint32_t max_dimensions;     // Maximum dimensions (26)
    double entropy_threshold;    // Stability threshold (0.05)
    double uniform_threshold;    // Uniform distance threshold (0.50)
    int max_recursion_depth;     // Maximum recursion levels (5)
} RecursionParams;

// Recovery context for recursive algorithm
typedef struct {
    EC_GROUP* ec_group;
    BN_CTX* bn_ctx;
    BIGNUM* order;
    
    // Current state
    TorusState* current_torus;
    RecursionParams* params;
    
    // Anchors (dynamic)
    BIGNUM** anchor_k;
    EC_POINT** anchor_Q;
    double** anchor_positions;
    uint32_t num_anchors;
    
    // Known k samples for torus mapping
    BIGNUM** known_k_samples;
    double** known_k_positions;
    uint32_t num_samples;
    
    // Platonic model (recreated at each recursion)
    void* platonic_model;  // PlatonicModel*
    
} RecursiveRecoveryContext;

/**
 * Create recursive recovery context
 */
RecursiveRecoveryContext* recursive_recovery_create(
    EC_GROUP* ec_group,
    RecursionParams* params
);

/**
 * Free recursive recovery context
 */
void recursive_recovery_free(RecursiveRecoveryContext* ctx);

/**
 * Add known k sample for torus mapping
 */
void recursive_recovery_add_sample(
    RecursiveRecoveryContext* ctx,
    BIGNUM* k,
    EC_POINT* Q
);

/**
 * Map torus structure from known k samples (recursive)
 * Returns stable torus or NULL if max recursion reached
 */
TorusState* map_torus_recursive(
    RecursiveRecoveryContext* ctx,
    int current_depth
);

/**
 * Recover k using stable torus structure
 */
BIGNUM* recover_k_with_torus(
    RecursiveRecoveryContext* ctx,
    EC_POINT* target_Q,
    TorusState* stable_torus
);

/**
 * Compute π×φ distance between two points
 */
double pi_phi_distance(const double* p1, const double* p2, int dims);

/**
 * Compute distance from origin using π×φ metric
 */
double distance_from_origin(const double* p, int dims);

/**
 * Scale up parameters for next recursion level
 */
void scale_up_parameters(RecursiveRecoveryContext* ctx);

/**
 * Analyze partition boundaries using π×φ metric
 * Returns percentage of uniform distances
 */
double analyze_partition_boundaries(
    RecursiveRecoveryContext* ctx,
    double** positions,
    int num_positions
);

/**
 * Detect additional oscillations (multiple tori)
 */
bool detect_multiple_tori(
    RecursiveRecoveryContext* ctx,
    TorusState* torus
);

/**
 * Main entry point: Recursive k recovery
 */
BIGNUM* recursive_k_recovery(
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    BIGNUM** known_k_samples,
    EC_POINT** known_Q_samples,
    int num_samples,
    RecursionParams* params
);

#endif // RECURSIVE_RECOVERY_H