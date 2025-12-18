#ifndef CLOCK_LATTICE_H
#define CLOCK_LATTICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Babylonian clock position structure
 * Represents a position on the Babylonian clock with rings
 * 
 * NOTE: If NEW math library is included, these types are already defined.
 * We use conditional compilation to prevent redefinition.
 */
#ifndef MATH_TYPES_H  // Only define if NEW math library not included

typedef struct {
    int ring;           // 0=hours, 1=minutes, 2=seconds, 3=milliseconds
    int position;       // Position on that ring (1-based)
    double angle;       // Angle in radians (12 o'clock = -π/2, 3 o'clock = 0)
    double radius;      // Normalized radius (0.0 to 1.0)
} BabylonianClockPosition;

/**
 * Modular arithmetic relationships
 * Encodes prime's relationship to clock divisions
 */
typedef struct {
    uint64_t prime;
    int mod_12;         // prime % 12 (hours)
    int mod_60;         // prime % 60 (minutes/seconds)
    int mod_100;        // prime % 100 (milliseconds)
} PrimeModular;

/**
 * 3D sphere coordinates
 * Result of folding clock into sphere
 */
typedef struct {
    float x, y, z;
} SphereCoord;

#else  // MATH_TYPES_H is defined - use NEW math library types

// Map OLD type names to NEW type names for compatibility
typedef ClockPosition BabylonianClockPosition;
// PrimeModular and SphereCoord are already defined in math/types.h

#endif  // MATH_TYPES_H

/**
 * Map prime index to clock position
 * 
 * @param prime_index 1-based prime index
 * @return Clock position
 */
BabylonianClockPosition map_prime_index_to_clock(int prime_index);

/**
 * Get modular arithmetic relationships for a prime
 * 
 * @param prime Prime number
 * @return Modular relationships
 */
PrimeModular get_prime_modular(uint64_t prime);

/**
 * Fold clock position into 3D sphere
 * 
 * @param clock_pos Clock position
 * @return 3D sphere coordinates
 */
SphereCoord fold_clock_to_sphere(BabylonianClockPosition clock_pos);

/**
 * Check if position is sacred
 * 
 * @param pos Clock position
 * @return true if sacred (π, 12 o'clock, or related to 144,000)
 */
bool is_sacred_position(BabylonianClockPosition pos);

/**
 * Map token to clock-based lattice coordinates
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id, uint64_t prime, 
                                int prime_index, double* coords);

/**
 * Get ring radius for visualization
 * 
 * @param ring Ring number (0-3)
 * @param base_radius Base radius for scaling
 * @return Radius for this ring
 */
double get_ring_radius_for_visualization(int ring, double base_radius);

// ============================================================================
// MEMORY MAPPING (Phase 3, Day 9)
// ============================================================================

/**
 * Clock-based memory position
 * Maps threads to memory using Babylonian clock structure
 */
typedef struct {
    uint32_t ring;           // Which ring (0-4+)
    uint32_t position;       // Position in ring
    size_t memory_offset;    // Offset in memory (bytes)
    size_t segment_size;     // Size of this segment (bytes)
} ClockMemoryPosition;

/**
 * Map thread to memory position using clock structure
 * 
 * Uses Babylonian clock structure:
 * - Ring 0: 12 positions (hours)
 * - Ring 1: 60 positions (minutes)
 * - Ring 2: 60 positions (seconds)
 * - Ring 3: 100 positions (milliseconds)
 * - Ring 4+: 1000 positions each
 * 
 * @param thread_id Thread ID
 * @param hierarchy_level Level in hierarchy (0 = root)
 * @param total_memory Total memory available (bytes)
 * @return Memory position for this thread
 */
ClockMemoryPosition map_thread_to_memory(
    int thread_id,
    int hierarchy_level,
    size_t total_memory
);

/**
 * Calculate clock position for thread in memory mapping
 * 
 * @param thread_id Thread ID
 * @param hierarchy_level Level in hierarchy
 * @return Position in ring (0-based)
 */
uint32_t calculate_thread_clock_position(int thread_id, int hierarchy_level);

/**
 * Calculate memory offset for ring position
 * 
 * @param ring Ring number
 * @param position Position in ring
 * @param total_memory Total memory available
 * @return Memory offset in bytes
 */
size_t calculate_memory_offset(uint32_t ring, uint32_t position, size_t total_memory);

/**
 * Get number of positions in ring
 * 
 * @param ring Ring number
 * @return Number of positions in this ring
 */
uint32_t get_ring_positions(uint32_t ring);

/**
 * Get total positions up to ring
 * 
 * @param ring Ring number
 * @return Total positions in all rings up to and including this ring
 */
uint32_t get_cumulative_positions(uint32_t ring);

/**
 * Check if clock position is structurally valid
 * 
 * In deterministic system, valid position guarantees prime.
 * This checks structural validity, not primality.
 * 
 * @param pos Clock position to validate
 * @return true if position is structurally valid
 */
bool is_valid_clock_position(BabylonianClockPosition pos);

/**
 * Estimate prime index from prime value
 * 
 * Uses prime number theorem: π(n) ≈ n / ln(n)
 * This is an approximation - exact index requires lookup.
 * 
 * @param prime_value Prime number value
 * @return Estimated prime index (1-based)
 */
uint32_t estimate_prime_index(uint64_t prime_value);

/**
 * Validate that a number is prime using clock lattice structure
 * 
 * In deterministic system, if number maps to valid clock position,
 * it's structurally consistent with being prime.
 * 
 * This is NOT a complete primality test - it's a structural validation.
 * In a pure deterministic system, valid position = guaranteed prime.
 * 
 * @param candidate Number to validate
 * @return true if structurally consistent with being prime
 */
bool validate_prime_by_clock_position(uint64_t candidate);

/**
 * Get prime index from clock position (reverse mapping)
 * 
 * This calculates the prime index from a clock position.
 * Useful for reverse lookups.
 * 
 * @param pos Clock position
 * @return Prime index (1-based)
 */
uint32_t get_prime_index_from_position(BabylonianClockPosition pos);

/**
 * Get prime at specific index (HYBRID DETERMINISTIC SYSTEM)
 * 
 * This is the main function for deterministic prime generation.
 * 
 * Strategy:
 * 1. For first 232 primes: O(1) lookup table
 * 2. For cached primes: O(1) cache lookup  
 * 3. For new primes: Generate with sieve, validate with clock lattice
 * 
 * @param index Prime index (1-based: 1 = first prime = 2)
 * @return Prime value, or 0 on error
 */
uint64_t get_prime_at_index_deterministic(uint32_t index);

/**
 * Get multiple primes at once (batch operation)
 * 
 * More efficient than calling get_prime_at_index_deterministic() multiple times.
 * 
 * @param start_index First prime index (1-based)
 * @param count Number of primes to get
 * @param output Output array (must have space for count primes)
 * @return Number of primes retrieved
 */
uint32_t get_primes_batch(uint32_t start_index, uint32_t count, uint64_t* output);

/**
 * Cleanup prime cache (call on shutdown)
 */
void cleanup_prime_cache(void);

/**
 * Get cache statistics (for debugging/monitoring)
 * 
 * @param size Output: number of primes in cache
 * @param capacity Output: cache capacity
 */
void get_prime_cache_stats(uint32_t* size, uint32_t* capacity);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_LATTICE_H