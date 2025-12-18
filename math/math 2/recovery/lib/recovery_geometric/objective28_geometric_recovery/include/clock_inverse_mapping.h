#ifndef K_RECOVERY_ENHANCED_H
#define K_RECOVERY_ENHANCED_H

#include "clock_recovery.h"
#include "platonic_solids.h"
#include <openssl/bn.h>
#include <openssl/ec.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enhanced inverse mapping from clock position to k value
 * 
 * Uses multi-step correction:
 * 1. Basic inverse from angle
 * 2. Ring-specific correction
 * 3. Position-based adjustment
 * 4. Anchor-based refinement
 * 
 * @param pos Target clock position
 * @param anchors Array of anchor positions
 * @param num_anchors Number of anchors
 * @return Estimated k value
 */
double inverse_map_k_from_clock(ClockPosition pos,
                                ClockPosition* anchors,
                                uint32_t num_anchors);

/**
 * Recover k using optimal Platonic solid anchors
 * 
 * Uses shared vertices or Icosahedron vertices as optimal anchors
 * 
 * @param ctx Recovery context
 * @param overlay Platonic solid overlay
 * @param target Target clock position
 * @param recovered_k Output: recovered k value
 * @return true if recovery successful
 */
bool recover_k_with_platonic_anchors(ClockRecoveryContext* ctx,
                                     PlatonicOverlay* overlay,
                                     ClockPosition target,
                                     BIGNUM* recovered_k);

/**
 * Verify k recovery by checking k·G == Q
 * 
 * @param k Recovered k value
 * @param Q Public key point
 * @param group EC group
 * @return true if k·G == Q
 */
bool verify_k_recovery(const BIGNUM* k, const EC_POINT* Q, EC_GROUP* group);

/**
 * Iterative refinement of k recovery
 * 
 * Adjusts k iteratively until k·G == Q or max iterations reached
 * 
 * @param ctx Recovery context
 * @param Q Public key point
 * @param k Input/output: k value to refine
 * @param group EC group
 * @return true if converged
 */
bool refine_k_recovery(ClockRecoveryContext* ctx,
                       const EC_POINT* Q,
                       BIGNUM* k,
                       EC_GROUP* group);

/**
 * Complete k recovery pipeline
 * 
 * Executes full recovery:
 * 1. Recover k using Platonic anchors
 * 2. Verify k·G == Q
 * 3. Refine if needed
 * 
 * @param overlay Platonic solid overlay
 * @param target Target clock position
 * @param Q Public key point (optional, for verification)
 * @param group EC group (optional, for verification)
 * @param recovered_k Output: recovered k value
 * @return true if recovery successful
 */
bool complete_k_recovery(PlatonicOverlay* overlay,
                        ClockPosition target,
                        const EC_POINT* Q,
                        EC_GROUP* group,
                        BIGNUM* recovered_k);

#ifdef __cplusplus
}
#endif

#endif // K_RECOVERY_ENHANCED_H
