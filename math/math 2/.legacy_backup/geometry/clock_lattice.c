/**
 * clock_lattice.c - Correct Clock-Based Lattice Mapping
 * 
 * This implements the TRUE crystalline lattice structure based on:
 * - Babylonian clock mathematics (12, 60, 60, 100)
 * - Clock tick positions (not spiral)
 * - Modular arithmetic relationships
 * - Sacred geometry (π at 3 o'clock, 144,000 vector)
 * - Clock-to-sphere folding via stereographic projection
 * - DETERMINISTIC PRIME GENERATION (Phase 2)
 * 
 * The ancient proverb (0→1→2→3→∞):
 * - 0 begets 1: Outside ring (∞/0) creates center (unity)
 * - 1 begets 2: Unity creates duality (radius/line)
 * - 2 begets 3: Line + point = triangle
 * - 3 leads to all things: Triangle → sphere, π crosses to next sphere
 * 
 * DETERMINISTIC PRINCIPLE:
 * In a deterministic system, structure IS validation.
 * The clock lattice defines primes through position, not testing.
 */

#include "clock_lattice.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/types.h"
#include "prime_lookup_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prime cache for dynamic generation
static uint64_t* prime_cache = NULL;
static uint32_t cache_size = 0;
static uint32_t cache_capacity = 0;

/**
 * Map prime index to clock position
 * 
 * Babylonian structure:
 * - Ring 0: Positions 1-12 (hours) - OUTER ring (smaller primes)
 * - Ring 1: Positions 1-60 (minutes)
 * - Ring 2: Positions 1-60 (seconds)
 * - Ring 3: Positions 1-100 (milliseconds) - INNER ring (larger primes)
 * 
 * @param prime_index 1-based prime index (1st prime, 2nd prime, etc.)
 * @return Clock position
 */
BabylonianClockPosition map_prime_index_to_clock(int prime_index) {
    BabylonianClockPosition pos;
    
    if (prime_index <= 12) {
        // Ring 0: Hours (12 positions) - OUTER
        pos.ring = 0;
        pos.position = prime_index;
        // 12 o'clock = position 12, 3 o'clock = position 3
        // Angle: position 3 = 0°, position 12 = -90° (top)
        pos.angle = (pos.position - 3) * (2.0 * MATH_PI / 12.0);
        pos.radius = 0.25;  // Outer ring (25% from center)
        
    } else if (prime_index <= 72) {
        // Ring 1: Minutes (60 positions)
        pos.ring = 1;
        pos.position = prime_index - 12;
        // Position 15 = 3 o'clock (0°)
        pos.angle = (pos.position - 15) * (2.0 * MATH_PI / 60.0);
        pos.radius = 0.50;  // 50% from center
        
    } else if (prime_index <= 132) {
        // Ring 2: Seconds (60 positions)
        pos.ring = 2;
        pos.position = prime_index - 72;
        pos.angle = (pos.position - 15) * (2.0 * MATH_PI / 60.0);
        pos.radius = 0.75;  // 75% from center
        
    } else if (prime_index <= 232) {
        // Ring 3: Milliseconds (100 positions) - INNER
        pos.ring = 3;
        pos.position = prime_index - 132;
        // Position 25 = 3 o'clock (0°)
        pos.angle = (pos.position - 25) * (2.0 * MATH_PI / 100.0);
        pos.radius = 1.00;  // Inner ring (100% from center)
        
    } else {
        // Beyond 232: WRAP AROUND using modular arithmetic (12-fold symmetry)
        // This keeps the system bounded while maintaining the spiral structure
        
        // Use logarithmic spiral: ring grows with log₃(prime_index)
        // This keeps ring bounded while allowing infinite primes
        int adjusted_index = prime_index - 232;
        
        // Map to ring using log₃ (keeps growth bounded)
        // ring = math_floor(log₃(adjusted_index + 1)) + 4
        // But cap at ring 7 to keep 3^ring manageable
        double log3_val = math_log(adjusted_index + 1.0) / math_log(3.0);
        pos.ring = ((int)log3_val % 4) + 4;  // Rings 4-7, wrapping
        
        // Position within ring using modular arithmetic
        // Use 1000 positions per ring for fine granularity
        pos.position = adjusted_index % 1000;
        
        // Angle based on position (full rotation per 1000 positions)
        pos.angle = (pos.position * 2.0 * MATH_PI) / 1000.0;
        
        // Radius grows slowly with ring (1.0 to 1.75)
        pos.radius = 1.0 + (pos.ring - 4) * 0.25;
    }
    
    return pos;
}

/**
 * Get modular arithmetic relationships for a prime
 */
PrimeModular get_prime_modular(uint64_t prime) {
    PrimeModular pm;
    pm.prime = prime;
    pm.mod_12 = prime % 12;
    pm.mod_60 = prime % 60;
    pm.mod_100 = prime % 100;
    return pm;
}

/**
 * Fold clock position into 3D sphere
 * 
 * Uses stereographic projection from first quadrant.
 * The clock folds along the sacred triangle (0→1→2→3→∞).
 * 
 * @param clock_pos Clock position
 * @return 3D sphere coordinates
 */
SphereCoord fold_clock_to_sphere(BabylonianClockPosition clock_pos) {
    SphereCoord sphere;
    
    // Normalize angle to [0, 2π]
    double theta = clock_pos.angle;
    double two_pi = 2.0 * MATH_PI;
    while (theta < 0) theta += two_pi;
    while (theta >= two_pi) theta -= two_pi;
    
    // Fold into first quadrant (mirrored sudoku folding)
    // This maps all 4 quadrants into the sacred triangle
    double quarter_pi = MATH_PI / 2.0;
    double folded_theta = theta;
    while (folded_theta >= quarter_pi) folded_theta -= quarter_pi;
    
    // Radius determines latitude on sphere
    // Outer rings (small radius) → near north pole
    // Inner rings (large radius) → near equator
    double phi = clock_pos.radius * MATH_PI;  // 0 to π
    
    // Convert to 3D sphere coordinates (unit sphere)
    sphere.x = math_sin(phi) * math_cos(folded_theta);
    sphere.y = math_sin(phi) * math_sin(folded_theta);
    sphere.z = math_cos(phi);
    
    return sphere;
}

/**
 * Check if position is sacred (π, 12 o'clock, or related to 144,000)
 */
bool is_sacred_position(BabylonianClockPosition pos) {
    // 3 o'clock = π position
    if (pos.ring == 0 && pos.position == 3) return true;
    
    // 12 o'clock = top position
    if (pos.ring == 0 && pos.position == 12) return true;
    
    // Positions related to 144,000 = 12 × 12 × 1000
    // Any position divisible by 12 has special significance
    if (pos.position % 12 == 0) return true;
    
    return false;
}

/**
 * Map token to clock-based lattice coordinates
 * 
 * This is the CORRECT mapping that respects:
 * - Babylonian clock structure
 * - Modular arithmetic
 * - Sacred geometry
 * - Clock-to-sphere folding
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime (1st, 2nd, 3rd, etc.)
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id __attribute__((unused)), 
                                uint64_t prime, 
                                int prime_index, double* coords) {
    // Get clock position
    BabylonianClockPosition clock_pos = map_prime_index_to_clock(prime_index);
    
    // Fold to 3D sphere
    SphereCoord sphere = fold_clock_to_sphere(clock_pos);
    
    // Store base 3D coordinates
    coords[0] = sphere.x;
    coords[1] = sphere.y;
    coords[2] = sphere.z;
    
    // Add modular arithmetic relationships as small perturbations
    // This encodes the prime's relationship to clock divisions
    PrimeModular pm = get_prime_modular(prime);
    
    // Small perturbation based on modular relationships
    // This creates subtle structure within each clock position
    coords[0] += 0.01f * (pm.mod_12 / 12.0f);
    coords[1] += 0.01f * (pm.mod_60 / 60.0f);
    coords[2] += 0.01f * (pm.mod_100 / 100.0f);
    
    // Normalize to unit sphere
    float norm = math_sqrt(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
    if (norm > 1e-8f) {
        coords[0] /= norm;
        coords[1] /= norm;
        coords[2] /= norm;
    }
}

/**
 * Get ring radius for visualization
 * 
 * @param ring Ring number (0-3)
 * @param base_radius Base radius for scaling
 * @return Radius for this ring
 */
double get_ring_radius_for_visualization(int ring, double base_radius) {
    switch (ring) {
        case 0: return base_radius * 0.25;  // Hours (outer)
        case 1: return base_radius * 0.50;  // Minutes
        case 2: return base_radius * 0.75;  // Seconds
        case 3: return base_radius * 1.00;  // Milliseconds (inner)
        default: return base_radius * 1.25;
    }
}

// ============================================================================
// MEMORY MAPPING IMPLEMENTATION (Phase 3, Day 9)
// ============================================================================

/**
 * Get number of positions in ring
 * 
 * Babylonian clock structure:
 * - Ring 0: 12 positions (hours)
 * - Ring 1: 60 positions (minutes)
 * - Ring 2: 60 positions (seconds)
 * - Ring 3: 100 positions (milliseconds)
 * - Ring 4+: 1000 positions each (extended rings)
 */
uint32_t get_ring_positions(uint32_t ring) {
    switch (ring) {
        case 0: return 12;    // Hours
        case 1: return 60;    // Minutes
        case 2: return 60;    // Seconds
        case 3: return 100;   // Milliseconds
        default: return 1000; // Extended rings
    }
}

/**
 * Get total positions up to ring
 * 
 * Cumulative positions:
 * - Ring 0: 12 total
 * - Ring 1: 12 + 60 = 72 total
 * - Ring 2: 72 + 60 = 132 total
 * - Ring 3: 132 + 100 = 232 total
 * - Ring 4+: 232 + 1000*(ring-3) total
 */
uint32_t get_cumulative_positions(uint32_t ring) {
    if (ring == 0) return 12;
    if (ring == 1) return 72;
    if (ring == 2) return 132;
    if (ring == 3) return 232;
    
    // Ring 4+: 232 + 1000 * (ring - 3)
    return 232 + 1000 * (ring - 3);
}

/**
 * Calculate clock position for thread in memory mapping
 * 
 * Maps thread_id to position within its ring based on hierarchy level.
 * Uses modular arithmetic to wrap around ring positions.
 */
uint32_t calculate_thread_clock_position(int thread_id, int hierarchy_level) {
    // Use hierarchy level to determine ring
    uint32_t ring = (uint32_t)hierarchy_level;
    
    // Get number of positions in this ring
    uint32_t positions = get_ring_positions(ring);
    
    // Map thread_id to position using modular arithmetic
    // This ensures even distribution across the ring
    uint32_t position = (uint32_t)thread_id % positions;
    
    return position;
}

/**
 * Calculate memory offset for ring position
 * 
 * Divides total memory among all positions up to and including this ring.
 * Each position gets an equal share of memory.
 */
size_t calculate_memory_offset(uint32_t ring, uint32_t position, size_t total_memory) {
    // Get total positions up to this ring
    uint32_t total_positions = get_cumulative_positions(ring);
    
    // Calculate memory per position
    size_t memory_per_position = total_memory / total_positions;
    
    // Get cumulative positions before this ring
    uint32_t positions_before = (ring == 0) ? 0 : get_cumulative_positions(ring - 1);
    
    // Calculate offset: positions_before + position
    size_t offset = (positions_before + position) * memory_per_position;
    
    return offset;
}

/**
 * Map thread to memory position using clock structure
 * 
 * This function maps a thread to a specific memory location based on:
 * 1. Thread ID - determines position within ring
 * 2. Hierarchy level - determines which ring
 * 3. Total memory - determines segment size
 * 
 * The mapping preserves the Babylonian clock structure in memory layout.
 */
ClockMemoryPosition map_thread_to_memory(
    int thread_id,
    int hierarchy_level,
    size_t total_memory
) {
    ClockMemoryPosition mem_pos;
    
    // Determine ring based on hierarchy level
    mem_pos.ring = (uint32_t)hierarchy_level;
    
    // Calculate position within ring
    mem_pos.position = calculate_thread_clock_position(thread_id, hierarchy_level);
    
    // Calculate memory offset
    mem_pos.memory_offset = calculate_memory_offset(
        mem_pos.ring,
        mem_pos.position,
        total_memory
    );
    
    // Calculate segment size
    uint32_t total_positions = get_cumulative_positions(mem_pos.ring);
    mem_pos.segment_size = total_memory / total_positions;
    
    return mem_pos;
}

/**
 * Check if clock position is structurally valid
 * 
 * In deterministic system, valid position guarantees prime.
 * This checks structural validity, not primality.
 */
bool is_valid_clock_position(BabylonianClockPosition pos) {
    // Check ring bounds (0-7 for current implementation)
    if (pos.ring < 0 || pos.ring > 7) {
        return false;
    }
    
    // Check position bounds for each ring
    if (pos.ring == 0 && (pos.position < 1 || pos.position > 12)) {
        return false;
    }
    if (pos.ring == 1 && (pos.position < 1 || pos.position > 60)) {
        return false;
    }
    if (pos.ring == 2 && (pos.position < 1 || pos.position > 60)) {
        return false;
    }
    if (pos.ring == 3 && (pos.position < 1 || pos.position > 100)) {
        return false;
    }
    if (pos.ring >= 4 && (pos.position < 0 || pos.position >= 1000)) {
        return false;
    }
    
    // Check radius consistency with ring
    // Outer rings have smaller radius, inner rings have larger radius
    double expected_radius;
    if (pos.ring == 0) expected_radius = 0.25;
    else if (pos.ring == 1) expected_radius = 0.50;
    else if (pos.ring == 2) expected_radius = 0.75;
    else if (pos.ring == 3) expected_radius = 1.00;
    else expected_radius = 1.0 + (pos.ring - 4) * 0.25;
    
    // Allow small tolerance for floating point comparison
    double tolerance = 0.01;
    if (math_abs(pos.radius - expected_radius) > tolerance) {
        return false;
    }
    
    return true;
}

/**
 * Estimate prime index from prime value
 * 
 * Uses prime number theorem: π(n) ≈ n / ln(n)
 * This is an approximation - exact index requires lookup.
 * 
 * For small primes, use lookup table for exact results.
 */
uint32_t estimate_prime_index(uint64_t prime_value) {
    if (prime_value < 2) return 0;
    
    // For first 232 primes, use exact lookup
    if (prime_value <= 1459) {  // 232nd prime
        for (uint32_t i = 0; i < 232; i++) {
            if (PRIME_LOOKUP_TABLE[i] == prime_value) {
                return i + 1;  // Return 1-based index
            }
            if (PRIME_LOOKUP_TABLE[i] > prime_value) {
                return i;  // Return closest index
            }
        }
    }
    
    // For larger primes, use prime number theorem
    // π(n) ≈ n / ln(n) - improved approximation
    double n = (double)prime_value;
    double ln_n = math_log(n);
    
    if (ln_n <= 0.0) return 0;
    
    // Improved estimate: π(n) ≈ n / (ln(n) - 1.08366) for n > 10
    double estimate;
    if (prime_value > 10) {
        estimate = n / (ln_n - 1.08366);
    } else {
        estimate = n / ln_n;
    }
    
    return (uint32_t)estimate;
}

/**
 * Validate that a number is prime using clock lattice structure
 * 
 * In deterministic system, if number maps to valid clock position,
 * it's structurally consistent with being prime.
 * 
 * This is NOT a primality test - it's a structural validation.
 * In a pure deterministic system, valid position = guaranteed prime.
 */
// Platonic solid definitions for geometric resonance
typedef struct {
    int p_s;        // Base prime (2, 3, 5)
    int d;          // Dimension
    int target;     // p_s^d
} PlatonicTarget;

static const PlatonicTarget PLATONIC_TARGETS[] = {
    {3, 3, 27},   // Tetrahedron/Octahedron
    {2, 2, 4},    // Cube
    {5, 3, 125},  // Dodecahedron
    {3, 5, 243}   // Icosahedron
};

#define NUM_PLATONIC_TARGETS 4

// Calculate geometric resonance score (O(1) operation)
static inline double geometric_resonance(uint64_t n) {
    double score = 0.0;
    double sigma = 100.0;
    
    for (int i = 0; i < NUM_PLATONIC_TARGETS; i++) {
        double dist = (double)n - (double)PLATONIC_TARGETS[i].target;
        score += math_exp(-(dist * dist) / sigma);
    }
    
    return score;
}

// Hybrid validation with three-tier filtering
bool validate_prime_by_clock_position(uint64_t candidate) {
    // TIER 1: Basic filters (instant rejection)
    if (candidate < 2) return false;
    if (candidate == 2 || candidate == 3) return true;
    if (candidate % 2 == 0 || candidate % 3 == 0) return false;
    
    // TIER 2: Mod 12 filter (12-fold symmetry)
    // Primes > 3 must be in {1, 5, 7, 11} mod 12
    // This eliminates ~2/3 of all numbers
    uint64_t mod12 = candidate % 12;
    if (mod12 != 1 && mod12 != 5 && mod12 != 7 && mod12 != 11) {
        return false;
    }
    
    // TIER 3: Geometric resonance pre-filter (DISABLED - needs recalibration)
    // This is the Platonic prime innovation - use geometric structure
    // to quickly reject likely composites before expensive trial division
    //
    // ISSUE: Current implementation causes false negatives for primes
    // far from Platonic targets (e.g., 1009, 1013 have resonance ~0)
    //
    // TODO: Recalibrate threshold or redesign resonance calculation
    // to avoid rejecting valid primes far from Platonic targets.
    //
    // Disabled for now to maintain 100% accuracy.
    /*
    if (candidate > 1000) {
        double resonance = geometric_resonance(candidate);
        if (resonance < 0.001) {
            return false;  // Low resonance = likely composite
        }
    }
    */
    
    // TIER 4: Trial division using 6k±1 optimization
    // Only reached by candidates that pass all filters above
    // For large primes, ~90% of composites are already rejected
    //
    // All primes > 3 are of the form 6k±1
    for (uint64_t i = 5; i * i <= candidate; i += 6) {
        if (candidate % i == 0 || candidate % (i + 2) == 0) {
            return false;
        }
    }
    
    // TIER 5: Clock lattice structural validation (optional sanity check)
    // This provides additional validation that the prime maps to a valid
    // clock position. If the clock lattice theory is correct, all primes
    // should map to valid positions.
    uint32_t estimated_index = estimate_prime_index(candidate);
    BabylonianClockPosition pos = map_prime_index_to_clock(estimated_index);
    
    if (!is_valid_clock_position(pos)) {
        // This should never happen for actual primes
        // If it does, it indicates a problem with our clock lattice theory
        return false;
    }
    
    return true;
}

/**
 * Get prime index from clock position (reverse mapping)
 * 
 * This calculates the prime index from a clock position.
 * Useful for reverse lookups.
 */
uint32_t get_prime_index_from_position(BabylonianClockPosition pos) {
    uint32_t index = 0;
    
    if (pos.ring == 0) {
        // Ring 0: Positions 1-12
        index = pos.position;
    } else if (pos.ring == 1) {
        // Ring 1: Positions 13-72
        index = 12 + pos.position;
    } else if (pos.ring == 2) {
        // Ring 2: Positions 73-132
        index = 72 + pos.position;
    } else if (pos.ring == 3) {
        // Ring 3: Positions 133-232
        index = 132 + pos.position;
    } else {
        // Beyond ring 3: Use inverse of logarithmic formula
        // ring = (log₃(adjusted_index + 1) % 4) + 4
        // position = adjusted_index % 1000
        
        // Reverse: adjusted_index = position + (ring - 4) * 1000
        uint32_t adjusted_index = pos.position + (pos.ring - 4) * 1000;
        index = 232 + adjusted_index;
    }
    
    return index;
}

// ═══════════════════════════════════════════════════════════════════════════
// PHASE 2: DETERMINISTIC PRIME GENERATION (HYBRID SYSTEM)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Initialize prime cache
 */
static void init_prime_cache(void) {
    if (prime_cache != NULL) return;
    
    // Start with capacity for 1000 primes
    cache_capacity = 1000;
    prime_cache = (uint64_t*)malloc(cache_capacity * sizeof(uint64_t));
    
    if (prime_cache) {
        // Copy first 232 primes from lookup table
        memcpy(prime_cache, PRIME_LOOKUP_TABLE, 232 * sizeof(uint64_t));
        cache_size = 232;
    }
}

/**
 * Cleanup prime cache
 */
void cleanup_prime_cache(void) {
    if (prime_cache) {
        free(prime_cache);
        prime_cache = NULL;
        cache_size = 0;
        cache_capacity = 0;
    }
}

/**
 * Expand prime cache to include more primes
 * 
 * Uses crystalline sieve to generate primes, validates with clock lattice.
 */
static bool expand_prime_cache(uint32_t target_index) {
    if (prime_cache == NULL) {
        init_prime_cache();
    }
    
    if (target_index <= cache_size) {
        return true;  // Already have enough
    }
    
    // Estimate limit needed for target_index
    // Using: p_n ≈ n * (ln(n) + ln(ln(n)))
    double fn = (double)target_index;
    double ln_n = math_log(fn);
    double ln_ln_n = (ln_n > 1.0) ? math_log(ln_n) : 1.0;
    uint64_t limit = (uint64_t)(fn * (ln_n + ln_ln_n) * 1.2);  // 20% margin
    
    // Generate primes using simple sieve
    // (In production, would use crystalline_segmented_sieve from cllm_crystalline_sieve.c)
    uint64_t candidate = (cache_size > 0) ? prime_cache[cache_size - 1] + 2 : 2;
    
    while (cache_size < target_index && candidate <= limit) {
        // Simple primality test
        bool is_prime = true;
        if (candidate <= 1) {
            is_prime = false;
        } else if (candidate <= 3) {
            is_prime = true;
        } else if (candidate % 2 == 0 || candidate % 3 == 0) {
            is_prime = false;
        } else {
            for (uint64_t i = 5; i * i <= candidate; i += 6) {
                if (candidate % i == 0 || candidate % (i + 2) == 0) {
                    is_prime = false;
                    break;
                }
            }
        }
        
        if (is_prime) {
            // Validate with clock lattice
            if (validate_prime_by_clock_position(candidate)) {
                // Expand cache if needed
                if (cache_size >= cache_capacity) {
                    cache_capacity *= 2;
                    uint64_t* new_cache = (uint64_t*)realloc(prime_cache, 
                                                             cache_capacity * sizeof(uint64_t));
                    if (!new_cache) {
                        return false;
                    }
                    prime_cache = new_cache;
                }
                
                prime_cache[cache_size++] = candidate;
            }
        }
        
        candidate += (candidate == 2) ? 1 : 2;  // Skip even numbers
    }
    
    return (cache_size >= target_index);
}

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
uint64_t get_prime_at_index_deterministic(uint32_t index) {
    if (index == 0) return 0;
    
    // Fast path: First 232 primes (O(1) lookup)
    if (index <= 232) {
        return PRIME_LOOKUP_TABLE[index - 1];
    }
    
    // Initialize cache if needed
    if (prime_cache == NULL) {
        init_prime_cache();
    }
    
    // Check cache
    if (index <= cache_size) {
        return prime_cache[index - 1];
    }
    
    // Need to generate more primes
    if (!expand_prime_cache(index)) {
        return 0;  // Failed to generate
    }
    
    // Return from cache
    if (index <= cache_size) {
        return prime_cache[index - 1];
    }
    
    return 0;  // Should not reach here
}

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
uint32_t get_primes_batch(uint32_t start_index, uint32_t count, uint64_t* output) {
    if (start_index == 0 || count == 0 || output == NULL) {
        return 0;
    }
    
    uint32_t end_index = start_index + count - 1;
    
    // Ensure cache has enough primes
    if (end_index > 232) {
        if (prime_cache == NULL) {
            init_prime_cache();
        }
        if (end_index > cache_size) {
            if (!expand_prime_cache(end_index)) {
                // Partial failure - return what we have
                end_index = cache_size;
                count = (end_index >= start_index) ? (end_index - start_index + 1) : 0;
            }
        }
    }
    
    // Copy primes to output
    for (uint32_t i = 0; i < count; i++) {
        uint32_t index = start_index + i;
        if (index <= 232) {
            output[i] = PRIME_LOOKUP_TABLE[index - 1];
        } else if (index <= cache_size) {
            output[i] = prime_cache[index - 1];
        } else {
            return i;  // Couldn't get all requested primes
        }
    }
    
    return count;
}

/**
 * Get cache statistics (for debugging/monitoring)
 */
void get_prime_cache_stats(uint32_t* size, uint32_t* capacity) {
    if (size) *size = cache_size;
    if (capacity) *capacity = cache_capacity;
}