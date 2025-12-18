#ifndef LATTICE_ENTROPY_H
#define LATTICE_ENTROPY_H

/**
 * Lattice Entropy System - Γ(n,d) Implementation
 * 
 * Implements the Γ(n,d) term in the complete lattice formula:
 * L(n,d,k,λ,ω,ψ) = 3^O × ∏cos(θ×φᵢ) × Γ(k) × ν(λ) × (ω) × Ψ(ψ) × Γ(n,d)
 * 
 * Complete Formula:
 * Γ(n,d) = log₂(count of abacus primes in dimension d / entropy of lattice points)
 * 
 * This measures the information density of the lattice structure.
 */

#include <stdint.h>
#include <stddef.h>

/**
 * Count primes in a given dimension
 * 
 * Counts the number of primes that map to dimension d in the
 * crystalline lattice structure.
 * 
 * @param d Dimension (0-12)
 * @param max_prime Maximum prime to consider
 * @return Count of primes in dimension d
 */
uint64_t count_primes_in_dimension(uint32_t d, uint64_t max_prime);

/**
 * Calculate Shannon entropy of lattice points
 * 
 * Measures the information entropy of the distribution of
 * lattice points across dimensions.
 * 
 * H = -Σ p(i) log₂ p(i)
 * 
 * @param n Prime index
 * @param d Dimension
 * @param max_prime Maximum prime to consider
 * @return Shannon entropy value
 */
double calculate_point_entropy(uint32_t n, uint32_t d, uint64_t max_prime);

/**
 * Calculate complete lattice entropy Γ(n,d)
 * 
 * Implements the complete formula:
 * Γ(n,d) = log₂(count of abacus primes in dimension / entropy of lattice points)
 * 
 * @param n Prime index
 * @param d Dimension
 * @return Lattice entropy value
 */
double calculate_lattice_entropy(uint32_t n, uint32_t d);

/**
 * Calculate lattice density (simplified version)
 * 
 * Faster approximation of entropy for performance-critical paths.
 * 
 * @param n Prime index
 * @param d Dimension
 * @return Approximate lattice density
 */
double calculate_lattice_density(uint32_t n, uint32_t d);

/**
 * Get cached entropy value
 * 
 * Returns cached entropy if available, otherwise calculates and caches.
 * 
 * @param n Prime index
 * @param d Dimension
 * @return Cached or calculated entropy
 */
double get_cached_entropy(uint32_t n, uint32_t d);

/**
 * Initialize entropy cache
 * 
 * Pre-calculates entropy values for common dimensions.
 * Call once at startup for better performance.
 */
void initialize_entropy_cache(void);

/**
 * Clear entropy cache
 * 
 * Frees memory used by entropy cache.
 */
void clear_entropy_cache(void);

#endif // LATTICE_ENTROPY_H