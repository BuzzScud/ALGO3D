#ifndef CRYSTAL_ABACUS_H
#define CRYSTAL_ABACUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * Crystal Abacus - Prime Generation with Lattice Embedding
 * 
 * Based on the Python implementation with:
 * - 15D lattice embedding (mod first 15 primes)
 * - Recursive depth for self-similarity
 * - Dynamic gates as kissing spheres
 * - Wheel structure (mod 30) with golden ratio scaling
 * - Quadratic residue filtering
 */

// Golden ratio constant
#define PHI 1.618033988749895

// First 50 small primes for hard rejects
extern const uint32_t SMALL_PRIMES[50];

// Wheel structure (mod 30)
#define WHEEL 30
#define NUM_VIABLE_RESIDUES 8
extern const uint32_t VIABLE_RESIDUES[NUM_VIABLE_RESIDUES];

// Dynamic gates (kissing spheres)
#define NUM_BASE_GATES 5
extern const uint32_t BASE_GATES[NUM_BASE_GATES];

// Epsilon for tangent/kissing check
#define EPSILON 1e-6

// Lattice embedding structure (recursive)
typedef struct LatticeEmbedding {
    uint64_t residues[15];              // k mod first 15 primes
    uint32_t depth;                     // Recursion depth
    struct LatticeEmbedding* sub;       // Sub-embedding (recursive)
} LatticeEmbedding;

// Dynamic gate structure
typedef struct {
    uint64_t* centers;                  // Gate centers (scaled)
    double radius;                      // Gate radius
    uint32_t num_centers;               // Number of centers
} DynamicGate;

// Crystal Abacus structure
typedef struct {
    uint64_t* primes;                   // Generated primes
    uint32_t num_primes;                // Number of primes
    uint32_t capacity;                  // Array capacity
    LatticeEmbedding** seen;            // Seen embeddings (for uniqueness)
    uint32_t num_seen;                  // Number of seen embeddings
    uint32_t seen_capacity;             // Seen array capacity
    uint64_t candidate;                 // Current candidate
} CrystalAbacus;

/**
 * Create a new Crystal Abacus instance
 */
CrystalAbacus* abacus_create(void);

/**
 * Free Crystal Abacus and all resources
 */
void abacus_free(CrystalAbacus* abacus);

/**
 * Generate next prime using lattice-based filtering
 */
uint64_t abacus_next_prime(CrystalAbacus* abacus);

/**
 * Check if a number is prime (generates primes up to sqrt(m) as needed)
 */
bool abacus_is_prime(CrystalAbacus* abacus, uint64_t m);

/**
 * Embed k into 15D lattice with recursive depth
 */
LatticeEmbedding* embed_k(uint64_t k, uint32_t depth);

/**
 * Free lattice embedding (recursive)
 */
void free_embedding(LatticeEmbedding* emb);

/**
 * Compare two embeddings for equality
 */
bool embeddings_equal(const LatticeEmbedding* a, const LatticeEmbedding* b);

/**
 * Compute dynamic gate for k (kissing spheres)
 */
DynamicGate* compute_dynamic_gate(uint64_t k);

/**
 * Free dynamic gate
 */
void free_dynamic_gate(DynamicGate* gate);

/**
 * Check if k is near a dynamic gate (kissing/tangent)
 * Recursive with depth for infinite layers
 */
bool near_dynamic_gate(uint64_t k, uint32_t recurse_depth);

/**
 * Get wheel position (mod 30)
 */
uint32_t wheel_position(uint64_t k);

/**
 * Recursive clock check with golden ratio scaling
 */
bool recursive_clock_check(uint64_t k, uint32_t depth);

/**
 * Compute quadratic residue mod 30
 */
uint32_t quadratic_residue_mod30(uint64_t k);

/**
 * Pre-filter candidate using all geometric filters
 */
bool pre_filter_candidate(uint64_t k);

/**
 * Vedic exemptions for small primes
 */
bool vedic_small(uint64_t k);

/**
 * Origami Q1 check (adapted to mod 30)
 */
bool origami_q1(uint64_t k);

#endif // CRYSTAL_ABACUS_H
