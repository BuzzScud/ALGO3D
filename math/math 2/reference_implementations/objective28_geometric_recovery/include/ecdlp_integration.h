/**
 * @file ecdlp_integration.h
 * @brief ECDLP Integration for Geometric Recovery
 * 
 * Phase 3: Integrate real elliptic curve operations with geometric recovery.
 * Uses OpenSSL for EC operations, crystalline math for everything else.
 * 
 * CRITICAL: This is the ONLY place where we use OpenSSL.
 * All other math uses PURE crystalline library (NO math.h).
 */

#ifndef ECDLP_INTEGRATION_H
#define ECDLP_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ECDLP Test Instance
 * 
 * Represents a discrete logarithm problem: Q = k*G
 * where G is the generator, k is the secret, Q is the public key
 */
typedef struct {
    EC_GROUP* group;        // Elliptic curve group
    EC_POINT* G;            // Generator point
    EC_POINT* Q;            // Public key (Q = k*G)
    BIGNUM* k;              // Secret scalar (what we're trying to recover)
    BIGNUM* order;          // Group order
    int nid;                // Curve NID (e.g., NID_secp256k1)
    char* curve_name;       // Human-readable name
} ECDLPInstance;

/**
 * EC Point Coordinates
 * 
 * Affine coordinates (x, y) of a point on the elliptic curve
 */
typedef struct {
    BIGNUM* x;
    BIGNUM* y;
    bool is_infinity;
} ECPointCoords;

/**
 * EC Lattice Embedding
 * 
 * Maps EC point to 15D lattice using crystalline math
 */
typedef struct {
    double coords[15];      // 15D lattice coordinates
    uint64_t primes[15];    // First 15 primes used for embedding
    double magnitude;       // Vector magnitude
    double angle;           // Angle in lattice space
} ECLatticeEmbedding;

// ============================================================================
// ECDLP INSTANCE MANAGEMENT
// ============================================================================

/**
 * Create ECDLP test instance
 * 
 * Generates a random k and computes Q = k*G
 * 
 * @param nid Curve NID (e.g., NID_secp256k1, NID_secp192k1)
 * @return ECDLP instance, or NULL on failure
 */
ECDLPInstance* ecdlp_create_instance(int nid);

/**
 * Create ECDLP instance with specific k
 * 
 * Uses provided k value instead of random generation
 * 
 * @param nid Curve NID
 * @param k_value Secret scalar value
 * @return ECDLP instance, or NULL on failure
 */
ECDLPInstance* ecdlp_create_instance_with_k(int nid, const BIGNUM* k_value);

/**
 * Free ECDLP instance
 * 
 * @param instance Instance to free
 */
void ecdlp_free_instance(ECDLPInstance* instance);

/**
 * Verify solution
 * 
 * Checks if k_candidate * G == Q
 * 
 * @param instance ECDLP instance
 * @param k_candidate Candidate scalar
 * @return true if k_candidate is correct, false otherwise
 */
bool ecdlp_verify_solution(const ECDLPInstance* instance, const BIGNUM* k_candidate);

// ============================================================================
// EC POINT OPERATIONS
// ============================================================================

/**
 * Get EC point coordinates
 * 
 * Extracts affine (x, y) coordinates from EC_POINT
 * 
 * @param group EC group
 * @param point EC point
 * @return Point coordinates, or NULL on failure
 */
ECPointCoords* ec_get_point_coords(const EC_GROUP* group, const EC_POINT* point);

/**
 * Free EC point coordinates
 * 
 * @param coords Coordinates to free
 */
void ec_free_point_coords(ECPointCoords* coords);

/**
 * Scalar multiply: result = k * point
 * 
 * @param group EC group
 * @param result Output point
 * @param k Scalar
 * @param point Input point
 * @return true on success, false on failure
 */
bool ec_scalar_mul(const EC_GROUP* group, EC_POINT* result, 
                   const BIGNUM* k, const EC_POINT* point);

/**
 * Point addition: result = p1 + p2
 * 
 * @param group EC group
 * @param result Output point
 * @param p1 First point
 * @param p2 Second point
 * @return true on success, false on failure
 */
bool ec_point_add(const EC_GROUP* group, EC_POINT* result,
                  const EC_POINT* p1, const EC_POINT* p2);

// ============================================================================
// LATTICE EMBEDDING (CRYSTALLINE MATH)
// ============================================================================

/**
 * Embed EC point into 15D lattice
 * 
 * Uses PURE crystalline mathematics (NO math.h)
 * Maps (x, y) coordinates to lattice using first 15 primes
 * 
 * Algorithm:
 * 1. Convert BIGNUM x, y to uint64_t (mod each prime)
 * 2. For each prime p_i: coords[i] = (x mod p_i) / p_i
 * 3. Normalize using prime_sqrt, prime_atan2
 * 
 * @param coords EC point coordinates
 * @param embedding Output lattice embedding
 * @return true on success, false on failure
 */
bool lattice_embed_ec_point(const ECPointCoords* coords, ECLatticeEmbedding* embedding);

/**
 * Compute lattice distance
 * 
 * Uses prime_sqrt for Euclidean distance in 15D space
 * 
 * @param emb1 First embedding
 * @param emb2 Second embedding
 * @return Distance between embeddings
 */
double lattice_distance(const ECLatticeEmbedding* emb1, const ECLatticeEmbedding* emb2);

/**
 * Compute lattice angle
 * 
 * Uses prime_acos for angle between vectors
 * 
 * @param emb1 First embedding
 * @param emb2 Second embedding
 * @return Angle in radians [0, π]
 */
double lattice_angle(const ECLatticeEmbedding* emb1, const ECLatticeEmbedding* emb2);

// ============================================================================
// K CANDIDATE GENERATION (FROM ATTRACTORS)
// ============================================================================

/**
 * Generate k candidates from tetration attractors
 * 
 * Uses tetration attractors to generate candidate k values
 * 
 * Algorithm:
 * 1. Compute 18 tetration attractors (6 bases × 3 heights)
 * 2. For each attractor, generate k candidates in neighborhood
 * 3. Use lattice embedding to guide search
 * 4. Return top N candidates by attractor bias
 * 
 * @param instance ECDLP instance
 * @param Q_embedding Lattice embedding of public key Q
 * @param candidates Output array for k candidates
 * @param max_candidates Maximum number of candidates to generate
 * @return Number of candidates generated
 */
int generate_k_candidates_from_attractors(
    const ECDLPInstance* instance,
    const ECLatticeEmbedding* Q_embedding,
    BIGNUM** candidates,
    int max_candidates
);

/**
 * Refine k candidate using lattice feedback
 * 
 * Iteratively refines k candidate by comparing lattice embeddings
 * 
 * @param instance ECDLP instance
 * @param k_candidate Current k candidate
 * @param Q_embedding Target lattice embedding
 * @param max_iterations Maximum refinement iterations
 * @return Refined k candidate
 */
BIGNUM* refine_k_candidate(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding,
    int max_iterations
);

// ============================================================================
// TESTING & VALIDATION
// ============================================================================

/**
 * Test ECDLP recovery on toy curve
 * 
 * Uses small curve (secp192k1 or custom) for fast testing
 * 
 * @param num_tests Number of test instances
 * @return Number of successful recoveries
 */
int test_ecdlp_recovery_toy(int num_tests);

/**
 * Test ECDLP recovery on secp256k1
 * 
 * Full-scale test on Bitcoin curve
 * 
 * @param num_tests Number of test instances
 * @return Number of successful recoveries
 */
int test_ecdlp_recovery_secp256k1(int num_tests);

/**
 * Benchmark ECDLP recovery
 * 
 * Measures time per recovery attempt
 * 
 * @param nid Curve NID
 * @param num_tests Number of tests
 * @return Average time per recovery (seconds)
 */
double benchmark_ecdlp_recovery(int nid, int num_tests);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Get curve name from NID
 * 
 * @param nid Curve NID
 * @return Human-readable curve name
 */
const char* get_curve_name(int nid);

/**
 * Print ECDLP instance info
 * 
 * @param instance ECDLP instance
 */
void print_ecdlp_instance(const ECDLPInstance* instance);

/**
 * Print lattice embedding
 * 
 * @param embedding Lattice embedding
 */
void print_lattice_embedding(const ECLatticeEmbedding* embedding);

#ifdef __cplusplus
}
#endif

#endif // ECDLP_INTEGRATION_H