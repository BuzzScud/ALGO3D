#ifndef CLOCK_RECOVERY_H
#define CLOCK_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Clock position for k value
 * Maps k to Babylonian clock structure using π×φ metric
 */
typedef struct {
    int ring;           // 0=hours(12), 1=minutes(60), 2=seconds(60), 3=milliseconds(100)
    int position;       // Position on that ring (1-based)
    double angle;       // Angle in radians (θ = k·π(1+√5))
    double radius;      // Normalized radius (0.25, 0.50, 0.75, 1.00)
} ClockPosition;

/**
 * Pythagorean triple structure
 * Based on Plimpton 322: ψ(p,q) = (p²-q², 2pq, p²+q²)
 */
typedef struct {
    uint64_t a;         // p²-q²
    uint64_t b;         // 2pq
    uint64_t c;         // p²+q²
    uint32_t p;         // First parameter
    uint32_t q;         // Second parameter (coprime with p, not both odd)
} PythagoreanTriple;

/**
 * Dimensional frequency structure
 * φᵢ = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, ...]
 */
typedef struct {
    uint32_t dimension;     // Dimension index (0-12)
    double frequency;       // Frequency φᵢ
    double alignment;       // cos(θ·φᵢ) - alignment score
} DimensionalFrequency;

/**
 * Clock recovery context
 */
typedef struct {
    ClockPosition* anchors;         // Known anchor positions
    uint32_t num_anchors;           // Number of anchors
    DimensionalFrequency* dims;     // 13 dimensional frequencies
    double entropy_threshold;       // Threshold for recursion
    uint32_t max_recursion_depth;   // Maximum recursion levels
    uint32_t current_depth;         // Current recursion depth
} ClockRecoveryContext;

// ============================================================================
// PHASE 1: CLOCK POSITION MAPPING
// ============================================================================

/**
 * Map k value to clock position using π×φ metric
 * 
 * Computes θ = k·π(1+√5) and maps to Babylonian clock structure
 * 
 * @param k The k value (private key)
 * @return Clock position
 */
ClockPosition map_k_to_clock(const BIGNUM* k);

/**
 * Map k value to clock position (uint64_t version)
 */
ClockPosition map_k_to_clock_u64(uint64_t k);

/**
 * Compute angular position using π×φ metric
 * θ = k·π(1+√5)
 * 
 * @param k The k value
 * @return Angular position in radians
 */
double compute_pi_phi_angle(const BIGNUM* k);

/**
 * Compute angular position (uint64_t version)
 */
double compute_pi_phi_angle_u64(uint64_t k);

/**
 * Map angle to Babylonian ring structure
 * 
 * @param angle Angular position in radians
 * @return Clock position
 */
ClockPosition map_angle_to_ring(double angle);

// ============================================================================
// PHASE 2: PYTHAGOREAN TRIPLE DETECTION
// ============================================================================

/**
 * Check if three clock positions form a Pythagorean triple
 * 
 * Uses Plimpton 322 formula: ψ(p,q) = (p²-q², 2pq, p²+q²)
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @param pos3 Third position
 * @return true if they form a Pythagorean triple
 */
bool is_pythagorean_triple(ClockPosition pos1, ClockPosition pos2, ClockPosition pos3);

/**
 * Find Pythagorean triple from three anchors
 * 
 * @param pos1 First position
 * @param pos2 Second position
 * @param pos3 Third position
 * @param triple Output: Pythagorean triple
 * @return true if triple found
 */
bool find_pythagorean_triple(ClockPosition pos1, ClockPosition pos2, ClockPosition pos3,
                             PythagoreanTriple* triple);

/**
 * Triangulate k position using Pythagorean triple
 * 
 * @param anchor1 First anchor
 * @param anchor2 Second anchor
 * @param anchor3 Third anchor
 * @param triple Pythagorean triple relationship
 * @return Estimated k position
 */
ClockPosition triangulate_with_triple(ClockPosition anchor1, ClockPosition anchor2,
                                     ClockPosition anchor3, PythagoreanTriple triple);

// ============================================================================
// PHASE 3: DIMENSIONAL FREQUENCY ANALYSIS
// ============================================================================

/**
 * Initialize dimensional frequencies
 * φᵢ = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, ...]
 * 
 * @param dims Output: array of 13 dimensional frequencies
 */
void init_dimensional_frequencies(DimensionalFrequency dims[13]);

/**
 * Compute dimensional alignment
 * 
 * For dimension d with frequency φᵢ:
 * alignment = cos(θ·φᵢ)
 * 
 * @param angle Angular position θ
 * @param frequency Dimensional frequency φᵢ
 * @return Alignment score [-1, 1]
 */
double compute_dimensional_alignment(double angle, double frequency);

/**
 * Compute total alignment across all dimensions
 * 
 * Product: ∏(i=1 to 13) cos(θ·φᵢ)
 * 
 * @param angle Angular position θ
 * @param dims Dimensional frequencies
 * @return Total alignment score
 */
double compute_total_alignment(double angle, DimensionalFrequency dims[13]);

/**
 * Compute entropy from alignment
 * 
 * Low alignment = high entropy (unstable)
 * High alignment = low entropy (stable)
 * 
 * @param alignment Total alignment score
 * @return Entropy value [0, 1]
 */
double compute_entropy_from_alignment(double alignment);

// ============================================================================
// PHASE 4: 3-GROWTH RECURSIVE REFINEMENT
// ============================================================================

/**
 * Check if recursion is needed
 * 
 * @param ctx Recovery context
 * @param entropy Current entropy
 * @return true if recursion needed
 */
bool needs_recursion(ClockRecoveryContext* ctx, double entropy);

/**
 * Perform 3-growth recursion
 * 
 * Adds new ring with 3× positions:
 * 100 → 300 → 900 → 2700 → ...
 * 
 * @param ctx Recovery context
 */
void recurse_with_3_growth(ClockRecoveryContext* ctx);

/**
 * Apply Möbius twist
 * Γ(k) = (-1)^k
 * 
 * @param k The k value
 * @return Möbius twist value (-1 or 1)
 */
int apply_mobius_twist(const BIGNUM* k);

/**
 * Compute distance between two clock positions using π×φ metric
 * 
 * @param pos1 First clock position
 * @param pos2 Second clock position
 * @return Distance in π×φ metric
 */
double clock_distance_pi_phi(ClockPosition pos1, ClockPosition pos2);

// ============================================================================
// PHASE 5: COMPLETE K RECOVERY
// ============================================================================

/**
 * Initialize clock recovery context
 * 
 * @param num_anchors Number of known k values (anchors)
 * @return Initialized context
 */
ClockRecoveryContext* init_clock_recovery(uint32_t num_anchors);

/**
 * Free clock recovery context
 */
void free_clock_recovery(ClockRecoveryContext* ctx);

/**
 * Add anchor to recovery context
 * 
 * @param ctx Recovery context
 * @param k Known k value
 * @param index Anchor index
 */
void add_anchor(ClockRecoveryContext* ctx, const BIGNUM* k, uint32_t index);

/**
 * Recover k from Q using clock-based algorithm
 * 
 * Complete pipeline:
 * 1. Map anchors to clock positions
 * 2. Detect clock alignment (entropy)
 * 3. Find Pythagorean triple anchors
 * 4. Triangulate target k position
 * 5. Recursive refinement if needed
 * 6. Apply Möbius twist
 * 7. Verify k·G == Q
 * 
 * @param ctx Recovery context
 * @param Q Public key point
 * @param recovered_k Output: recovered k value
 * @return true if recovery successful
 */
bool recover_k_from_clock(ClockRecoveryContext* ctx, const BIGNUM* Q, BIGNUM* recovered_k);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_RECOVERY_H