// lattice_cache.h - Crystalline Lattice Prime Cache System
// Three-stage architecture: Seed → Geometric → On-Demand

#ifndef LATTICE_CACHE_H
#define LATTICE_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include "crystal_abacus.h"

// Single prime entry with all pre-calculated lattice values
typedef struct {
    uint64_t prime;       // The prime number
    uint64_t n;           // Prime index (1st prime, 2nd prime, etc.)
    double O;             // O_exponent(n, k, λ)
    double theta;         // theta_n(n, k, λ, ω, p, q)
    double L;             // L_lattice(n, d, k, λ, ω, p, q)
    double radius;        // 3^O (exponential radial distance)
    double angle;         // Full angular position
    int mobius;           // Γ(k) = (-1)^k (Möbius twist)
} PrimeLatticeEntry;

// The complete lattice cache
typedef struct {
    // Stage 1: Foundation (18 seed primes - hard-coded in library)
    const uint64_t *seed_primes;
    size_t seed_count;
    
    // Stage 2: Pre-calculated cache
    PrimeLatticeEntry *entries;
    size_t count;           // Current number of cached primes
    size_t capacity;        // Allocated capacity
    
    // Stage 3: On-demand generation state
    uint64_t max_prime;     // Highest prime currently cached
    bool auto_extend;       // Auto-generate more when needed
    
    // Statistics
    uint64_t lookups;       // Number of cache lookups
    uint64_t extensions;    // Number of times cache was extended
} LatticeCache;

// ═══════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════

// Create and initialize cache with primes up to max_prime
// Recommended: 144000 (~12K primes, 0.8MB, 90ms)
// Maximum: 500000 (~38K primes, 2.5MB, 400ms)
LatticeCache* lattice_cache_create(uint64_t max_prime);

// Free all cache resources
void lattice_cache_free(LatticeCache* cache);

// ═══════════════════════════════════════════════════════════════
// LOOKUP (O(1) - Lightning Fast!)
// ═══════════════════════════════════════════════════════════════

// Get lattice position for prime at given index
// Returns false if index out of range
bool lattice_cache_get_entry(LatticeCache* cache, size_t prime_index, 
                             PrimeLatticeEntry** entry);

// Get just radius and angle (most common use case)
bool lattice_cache_get_position(LatticeCache* cache, size_t prime_index,
                                double* radius, double* angle);

// Get prime number by index
bool lattice_cache_get_prime(LatticeCache* cache, size_t prime_index,
                             uint64_t* prime);

// Lookup entry by prime value (linear search - use sparingly)
PrimeLatticeEntry* lattice_cache_lookup(LatticeCache* cache, uint64_t prime);

// Find index of a specific prime number (binary search)
// Returns -1 if not found
int lattice_cache_find_prime(LatticeCache* cache, uint64_t prime);

// ═══════════════════════════════════════════════════════════════
// ON-DEMAND GENERATION
// ═══════════════════════════════════════════════════════════════

// Extend cache to include primes up to new_max
// Returns number of new primes added
size_t lattice_cache_extend(LatticeCache* cache, uint64_t new_max);

// Enable/disable automatic extension when accessing out-of-range indices
void lattice_cache_set_auto_extend(LatticeCache* cache, bool enable);

// Get current cache statistics
void lattice_cache_get_stats(LatticeCache* cache, 
                            size_t* count, 
                            uint64_t* max_prime,
                            size_t* memory_bytes);

// ═══════════════════════════════════════════════════════════════
// SPECIAL QUERIES
// ═══════════════════════════════════════════════════════════════

// Check if a prime is near the kissing sphere boundary (~144000)
bool lattice_cache_is_near_boundary(LatticeCache* cache, size_t prime_index,
                                   double threshold);

// Get all primes within a radius range
size_t lattice_cache_get_primes_in_radius_range(LatticeCache* cache,
                                                double min_radius,
                                                double max_radius,
                                                size_t** indices);

// Get all primes within an angular range
size_t lattice_cache_get_primes_in_angle_range(LatticeCache* cache,
                                              double min_angle,
                                              double max_angle,
                                              size_t** indices);

#endif // LATTICE_CACHE_H