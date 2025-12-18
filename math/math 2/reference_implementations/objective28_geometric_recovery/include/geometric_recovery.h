/**
 * @file geometric_recovery.h
 * @brief Complete Geometric Recovery System for Bitcoin k→Q
 * 
 * Implements the full mathematical framework:
 * - Directional oscillation tracking using anchors as gradient
 * - Shared geometry filtering (vertices/faces all k share)
 * - Torus intersection curves through shared vertices
 * - Quadrant polarity system with mirrored sudoku folding
 * - +1 bit extension for boundary crossing (257+ bits)
 * - Multi-scale fractal search at every layer
 * - Fractal partition bounds
 * - Torus orbit search along 2^40 points
 * - Tetration attractor bias
 * - SFT integration
 * - Q-based validation with elliptic curve constraint
 */

#ifndef GEOMETRIC_RECOVERY_H
#define GEOMETRIC_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include "platonic_model.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION
// ============================================================================

#define GEO_NUM_DIMENSIONS 52
#define GEO_NUM_QUADRANTS 4
#define GEO_BITS_EXTENDED 257  // 256 + 1 for boundary crossing
#define GEO_MAX_SCALES 10      // Multi-scale fractal search
#define GEO_ORBIT_SAMPLES 1000 // Samples per torus orbit

// ============================================================================
// DIRECTIONAL OSCILLATION TRACKING
// ============================================================================

/**
 * Oscillation Vector - tracks distance AND direction
 */
typedef struct {
    double distance;                    // How far off (magnitude)
    double direction[GEO_NUM_DIMENSIONS]; // Which direction in each dimension
    double velocity[GEO_NUM_DIMENSIONS];  // Rate of change
    int quadrant;                       // Which quadrant (1-4)
    int polarity;                       // +1 or -1
    bool moving_toward;                 // Moving toward or away from target
    double gradient_strength;           // Strength of gradient
} OscillationVector;

/**
 * Compute directional oscillation using anchors as gradient
 */
OscillationVector compute_oscillation_vector(
    const double* candidate_pos,
    const double* target_pos,
    const double** anchor_positions,
    uint32_t num_anchors,
    uint32_t num_dimensions
);

// ============================================================================
// QUADRANT POLARITY SYSTEM
// ============================================================================

/**
 * Quadrant Polarity - mirrored sudoku folding
 */
typedef struct {
    int quadrant;                       // 1, 2, 3, or 4
    int polarity;                       // +1 or -1
    bool x_flip;                        // X-axis reflection
    bool y_flip;                        // Y-axis reflection
    int mirror_quadrants[3];            // Which quadrants this mirrors
    double fold_angle;                  // Möbius fold angle at 2π
} QuadrantPolarity;

/**
 * Determine quadrant from position
 */
int determine_quadrant(const double* position, uint32_t num_dimensions);

/**
 * Create quadrant polarity system
 */
QuadrantPolarity create_quadrant_polarity(int quadrant);

/**
 * Apply quadrant polarity correction to position
 */
void apply_quadrant_polarity(
    double* position,
    const QuadrantPolarity* polarity,
    uint32_t num_dimensions
);

/**
 * Fold position using mirrored sudoku (Q2/Q3/Q4 → Q1)
 */
void fold_to_first_quadrant(
    double* position,
    QuadrantPolarity* polarity_out,
    uint32_t num_dimensions
);

// ============================================================================
// SHARED GEOMETRY (VERTICES AND FACES)
// ============================================================================

/**
 * Shared Vertex - vertex shared by multiple k values
 */
typedef struct {
    uint32_t vertex_id;
    double position[GEO_NUM_DIMENSIONS];
    uint32_t* face_ids;                 // Which faces this vertex belongs to
    uint32_t num_faces;
    uint32_t* k_indices;                // Indices of k values at this vertex
    uint32_t num_k_values;
    double confidence;                  // Confidence this is a shared vertex
} SharedVertex;

/**
 * Shared Face - face shared by multiple k values
 */
typedef struct {
    uint32_t face_id;
    uint32_t* vertex_ids;               // Vertices of this face
    uint32_t num_vertices;
    uint32_t* k_indices;                // Indices of k values on this face
    uint32_t num_k_values;
    double normal[GEO_NUM_DIMENSIONS];  // Face normal
    double confidence;                  // Confidence this is a shared face
} SharedFace;

/**
 * Find shared vertices (vertices with multiple k values)
 */
SharedVertex* find_shared_vertices(
    const double** k_positions,
    uint32_t num_k_values,
    PlatonicModel* model,
    uint32_t* num_shared_out
);

/**
 * Find shared faces (faces containing shared vertices)
 */
SharedFace* find_shared_faces(
    const SharedVertex* shared_vertices,
    uint32_t num_shared_vertices,
    PlatonicModel* model,
    uint32_t* num_faces_out
);

// ============================================================================
// TORUS INTERSECTION CURVES
// ============================================================================

/**
 * Torus Intersection Curve - curve where two tori intersect
 */
typedef struct {
    uint32_t torus1_id;
    uint32_t torus2_id;
    double* curve_points;               // Points along curve [num_points * num_dimensions]
    uint32_t num_points;
    uint32_t* vertex_ids;               // Vertices the curve passes through
    uint32_t num_vertices;
    uint32_t* face_ids;                 // Faces the curve lies on
    uint32_t num_faces;
    double arc_length;                  // Total arc length of curve
} TorusIntersectionCurve;

/**
 * Torus Orbit - represents a torus in high-dimensional space
 */
typedef struct {
    uint32_t dimension1;                // Primary dimension
    uint32_t dimension2;                // Secondary dimension (for planar tori)
    double axis[GEO_NUM_DIMENSIONS];    // Axis of rotation
    double center[GEO_NUM_DIMENSIONS];  // Center point
    double radius;                      // Radius
    double frequency;                   // Oscillation frequency
    uint64_t complexity;                // 2^40 per torus
    bool is_planar;                     // True if in (d1,d2) plane
} TorusOrbit;

/**
 * Find intersection curve between two tori
 */
TorusIntersectionCurve* find_torus_intersection_curve(
    const TorusOrbit* torus1,
    const TorusOrbit* torus2,
    PlatonicModel* model
);

/**
 * Sample point along torus orbit
 */
void sample_torus_orbit(
    const TorusOrbit* torus,
    double t,  // Parameter [0, 1]
    double* point_out
);

/**
 * Search along torus orbit (2^40 points)
 */
BIGNUM* search_torus_orbit(
    const TorusOrbit* torus,
    const TorusIntersectionCurve* curve,
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    uint32_t num_samples,
    const double** anchor_positions,
    const BIGNUM** anchor_k_values,
    uint32_t num_anchors,
    uint32_t num_dimensions
);

// ============================================================================
// FRACTAL PARTITION BOUNDS
// ============================================================================

/**
 * Fractal Partition - bounds on k search space
 */
typedef struct {
    uint32_t min_dimensions;            // Minimum dimensions needed
    uint32_t max_dimensions;            // Maximum dimensions needed
    double min_bounds[GEO_NUM_DIMENSIONS]; // Lower bounds
    double max_bounds[GEO_NUM_DIMENSIONS]; // Upper bounds
    uint64_t partition_size;            // Size of partition (2^bits)
    double confidence;                  // Confidence in bounds
    uint32_t num_k_in_partition;        // Number of k values in partition
} FractalPartition;

/**
 * Compute fractal partition bounds
 */
FractalPartition compute_fractal_partition(
    const double** k_positions,
    uint32_t num_k_values,
    const double* target_position,
    uint32_t num_dimensions
);

// ============================================================================
// TETRATION ATTRACTORS
// ============================================================================

/**
 * Tetration Attractor - attractor in high-dimensional space
 */
typedef struct {
    uint32_t base;                      // 2, 3, 5, 7, 11, 13
    uint32_t depth;                     // 29-59
    double log_value;                   // Logarithmic representation
    double position[GEO_NUM_DIMENSIONS]; // Position in space
    double attractor_strength;          // How strongly it attracts
} TetrationAttractor;

/**
 * Create tetration attractors
 */
TetrationAttractor* create_tetration_towers(uint32_t* num_towers_out);

/**
 * Compute tetration attractor score for a position
 */
double compute_tetration_score(
    const double* position,
    const TetrationAttractor* towers,
    uint32_t num_towers,
    uint32_t num_dimensions
);

/**
 * Bias position toward nearest tetration attractor
 */
void bias_toward_attractor(
    double* position,
    const TetrationAttractor* towers,
    uint32_t num_towers,
    double bias_strength,
    uint32_t num_dimensions
);

// ============================================================================
// MULTI-SCALE FRACTAL SEARCH
// ============================================================================

/**
 * Scale Level - for multi-scale fractal search
 */
typedef struct {
    uint32_t scale_index;               // 0, 1, 2, ... (0 = full scale)
    double scale_factor;                // 1.0, 0.5, 0.25, ...
    uint32_t num_candidates;            // Candidates at this scale
    BIGNUM** candidates;                // Candidate k values
    double* scores;                     // Score for each candidate
} ScaleLevel;

/**
 * Perform multi-scale fractal search
 */
BIGNUM* multi_scale_fractal_search(
    const double* start_position,
    const TorusOrbit* tori,
    uint32_t num_tori,
    const TetrationAttractor* towers,
    uint32_t num_towers,
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    uint32_t max_scales
);

// ============================================================================
// SFT (SIEVE-FREE TESTING) INTEGRATION
// ============================================================================

/**
 * SFT Parameters - for systematic candidate generation
 */
typedef struct {
    uint64_t nonce_start;               // Starting nonce
    uint64_t nonce_end;                 // Ending nonce
    uint32_t step_size;                 // Step between nonces
    bool use_tetration_bias;            // Bias toward attractors
    bool use_quadrant_folding;          // Apply quadrant folding
} SFTParams;

/**
 * Generate candidate using SFT
 */
BIGNUM* sft_generate_candidate(
    const double* base_position,
    uint64_t nonce,
    const TetrationAttractor* towers,
    uint32_t num_towers,
    const SFTParams* params,
    uint32_t num_dimensions
);

/**
 * SFT systematic search
 */
BIGNUM* sft_systematic_search(
    const double* start_position,
    const SFTParams* params,
    const TetrationAttractor* towers,
    uint32_t num_towers,
    EC_GROUP* ec_group,
    EC_POINT* target_Q,
    uint32_t num_dimensions
);

// ============================================================================
// Q-BASED VALIDATION
// ============================================================================

/**
 * Validate candidate k by computing Q and measuring distance
 */
double validate_candidate_k(
    BIGNUM* candidate_k,
    EC_GROUP* ec_group,
    EC_POINT* target_Q
);

/**
 * Measure distance between two EC points
 */
double measure_ec_point_distance(
    EC_GROUP* ec_group,
    EC_POINT* point1,
    EC_POINT* point2
);

// ============================================================================
// COMPLETE GEOMETRIC RECOVERY SYSTEM
// ============================================================================

/**
 * Geometric Recovery Context - complete system state
 */
typedef struct {
    // EC curve
    EC_GROUP* ec_group;
    
    // Anchors (known k/Q pairs)
    BIGNUM** anchor_k_values;
    EC_POINT** anchor_Q_values;
    double** anchor_k_positions;
    double** anchor_Q_positions;
    uint32_t num_anchors;
    
    // Tori
    TorusOrbit* tori;
    uint32_t num_tori;
    
    // Tetration attractors
    TetrationAttractor* towers;
    uint32_t num_towers;
    
    // Shared geometry
    SharedVertex* shared_vertices;
    uint32_t num_shared_vertices;
    SharedFace* shared_faces;
    uint32_t num_shared_faces;
    
    // Torus intersection curves
    TorusIntersectionCurve** curves;
    uint32_t num_curves;
    
    // Platonic model
    PlatonicModel* model;
    
    // Configuration
    uint32_t num_dimensions;
    uint32_t max_scales;
    uint32_t orbit_samples;
    
} GeometricRecoveryContext;

/**
 * Create geometric recovery context
 */
GeometricRecoveryContext* geometric_recovery_create(
    EC_GROUP* ec_group,
    uint32_t num_anchors,
    uint32_t num_dimensions
);

/**
 * Add anchor (known k/Q pair)
 */
void geometric_recovery_add_anchor(
    GeometricRecoveryContext* ctx,
    BIGNUM* k,
    EC_POINT* Q
);

/**
 * Initialize system (detect tori, find shared geometry, etc.)
 */
bool geometric_recovery_initialize(GeometricRecoveryContext* ctx);

/**
 * Recover k from Q using complete geometric system
 */
BIGNUM* geometric_recovery_recover_k(
    GeometricRecoveryContext* ctx,
    EC_POINT* target_Q,
    double* confidence_out
);

/**
 * Iterative Q→k recovery with verification (from Python research)
 * 
 * Implements your OscillationAccelerator.reverse_q_to_k():
 * 1. Generate deterministic target from Q
 * 2. Reduce entropy recursively (2^(bit_scale/2) → 2^16)
 * 3. Find prime near target (CrystalAbacus)
 * 4. Verify QR condition (Legendre symbol)
 * 5. Verify k produces Q (critical verification loop)
 */
BIGNUM* iterative_recover_k_from_Q(
    GeometricRecoveryContext* ctx,
    EC_POINT* target_Q,
    uint32_t bit_scale,
    double* confidence_out
);

/**
 * Free geometric recovery context
 */
void geometric_recovery_free(GeometricRecoveryContext* ctx);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Map k (BIGNUM) to position in high-dimensional space
 */
void map_k_to_position(
    BIGNUM* k,
    double* position_out,
    uint32_t num_dimensions
);

/**
 * Map position to k (BIGNUM) with +1 bit extension
 */
/**
 * Triangulate k from anchors using weighted interpolation
 * This is the CORRECT approach - uses actual anchor k values!
 */
BIGNUM* triangulate_k_from_anchors(
    const double* position,
    const double** anchor_positions,
    const BIGNUM** anchor_k_values,
    uint32_t num_anchors,
    uint32_t num_dimensions,
    EC_GROUP* ec_group
);

/**
 * DEPRECATED: Use triangulate_k_from_anchors() instead!
 */
BIGNUM* map_position_to_k(
    const double* position,
    uint32_t num_dimensions
);

// ============================================================================
// ITERATIVE RECOVERY WITH DYNAMIC SCALING (Version 2)
// ============================================================================

/**
 * Triangulate k with correct endianness and truncation (Phase 2)
 * - Uses 257 bits (+1 for boundary crossing)
 * - Computes in REVERSE (as user specified)
 * - Truncates to actual order size
 */
BIGNUM* triangulate_k_with_truncation(
    const double* position,
    const double** anchor_positions,
    const BIGNUM** anchor_k_values,
    uint32_t num_anchors,
    uint32_t num_dimensions,
    EC_GROUP* ec_group
);

/**
 * Verify if candidate k produces target Q (Phase 1)
 * CRITICAL: Checks if candidate_k * G == target_Q
 */
bool verify_candidate_produces_Q(
    BIGNUM* candidate_k,
    EC_POINT* target_Q,
    EC_GROUP* ec_group
);

/**
 * Measure distance between two points (for oscillation tracking)
 */
double measure_point_distance(
    EC_POINT* Q1,
    EC_POINT* Q2,
    EC_GROUP* ec_group
);

/**
 * Iterative recovery with verification loop (Phase 1)
 * Iterates until finding k that produces target_Q
 */
BIGNUM* geometric_recovery_iterative(
    GeometricRecoveryContext* ctx,
    EC_POINT* target_Q,
    uint32_t max_iterations,
    double* confidence_out,
    uint32_t* iterations_out
);

/**
 * Measure oscillation magnitude in the system
 */
double measure_oscillation_magnitude(GeometricRecoveryContext* ctx);

/**
 * Recovery with dynamic scaling (Phase 3)
 * Starts with low complexity, scales up until oscillations stabilize
 */
BIGNUM* geometric_recovery_with_dynamic_scaling(
    EC_POINT* target_Q,
    EC_GROUP* ec_group,
    uint32_t initial_anchors,
    uint32_t initial_dimensions,
    double* confidence_out,
    uint32_t* final_dimensions_out,
    uint32_t* final_anchors_out
);

/**
 * Reduce entropy iteratively (Phase 4 - from user's Python code)
 */
uint64_t reduce_entropy(uint64_t initial_space, uint64_t threshold);

/**
 * Generate deterministic target from Q (Phase 4)
 * Note: This is a static function in iterative_recovery_v2.c
 */

/**
 * Compute Hamming distance between two k values
 */
double compute_k_hamming_distance(BIGNUM* k1, BIGNUM* k2);

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_RECOVERY_H