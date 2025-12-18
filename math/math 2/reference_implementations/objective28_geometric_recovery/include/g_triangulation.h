#ifndef G_TRIANGULATION_H
#define G_TRIANGULATION_H

#include <stdbool.h>
#include <stdint.h>
#include <openssl/ec.h>

/**
 * G Triangulation and Iterative Refinement
 * 
 * This implements the correct approach for ECDLP recovery:
 * 1. Start with known G (publicly available)
 * 2. Map G to clock lattice position
 * 3. Adjust Platonic solid anchors relative to G
 * 4. For each known (k, Q) pair, estimate k from adjusted anchors
 * 5. Track oscillation in k estimates
 * 6. Re-triangulate G from oscillation patterns
 * 7. Repeat until convergence
 */

typedef struct {
    double position[13];        // Position in 13D clock lattice
    uint64_t k_estimate;        // Estimated k value
    double confidence;          // Confidence score
} Anchor;

typedef struct {
    const EC_GROUP* group;
    EC_POINT* G;
    double G_position[13];
    Anchor* anchors;
    int num_anchors;
    int num_training_pairs;
    uint64_t* training_k;
    EC_POINT** training_Q;
    double** k_estimates_history;
    int max_iterations;
    int current_iteration;
    double g_movement;
    double k_oscillation;
    bool converged;
} GTriangulationContext;

/**
 * Create G triangulation context
 */
GTriangulationContext* create_g_triangulation_context(
    int curve_nid,
    int num_training_pairs,
    const uint64_t* training_k,
    int max_iterations
);

/**
 * Free G triangulation context
 */
void free_g_triangulation_context(GTriangulationContext* ctx);

/**
 * Estimate k from Q using adjusted anchors
 */
uint64_t estimate_k_from_q(
    GTriangulationContext* ctx,
    const EC_POINT* Q
);

/**
 * Perform one iteration of refinement
 */
void perform_refinement_iteration(GTriangulationContext* ctx);

/**
 * Check for convergence
 */
bool check_convergence(GTriangulationContext* ctx, double threshold);

/**
 * Map EC point to 13D clock lattice position
 */
void map_ec_point_to_lattice(
    const EC_POINT* point,
    const EC_GROUP* group,
    double position[13]
);

/**
 * Map k to clock lattice position using π×φ metric
 */
void map_k_to_lattice(uint64_t k, double position[13]);

/**
 * Compute distance between two positions
 */
double compute_distance(const double pos1[13], const double pos2[13]);

#endif // G_TRIANGULATION_H
/**
 * Phase 4: Refine G estimate using extracted p and q
 */
void refine_G_with_pq(
    GTriangulationContext* ctx,
    uint64_t p,
    uint64_t q,
    int p_index,
    int q_index
);

/**
 * Get refined G position after p/q refinement
 */
void get_refined_g_position(
    const GTriangulationContext* ctx,
    double position[13]
);

/**
 * Measure improvement from G refinement
 */
double measure_refinement_improvement(
    GTriangulationContext* ctx,
    const uint64_t* true_k_values,
    int num_samples
);

