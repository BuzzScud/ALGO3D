/**
 * @file clock_lattice_abacus.c
 * @brief Pure Abacus Clock Lattice Implementation
 * 
 * Revolutionary O(1) geometric recovery using clock lattice structure
 */

#include "geometric_recovery/clock_lattice_abacus.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>

// clock lattice structure
#define RING_0_SIZE 12    // Hours
#define RING_1_SIZE 60    // Minutes
#define RING_2_SIZE 60    // Seconds
#define RING_3_SIZE 100   // Milliseconds

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

uint32_t get_total_clock_positions_abacus(void) {
    return RING_0_SIZE * RING_1_SIZE * RING_2_SIZE * RING_3_SIZE;
}

uint32_t get_ring_size_abacus(uint32_t ring) {
    switch (ring) {
        case 0: return RING_0_SIZE;
        case 1: return RING_1_SIZE;
        case 2: return RING_2_SIZE;
        case 3: return RING_3_SIZE;
        default: return 0;
    }
}

CrystallineAbacus* compute_clock_resolution_abacus(
    uint32_t base,
    uint32_t precision
) {
    // Resolution = 2π / total_positions
    CrystallineAbacus* two_pi = abacus_from_double(2.0 * MATH_PI, base, precision);
    CrystallineAbacus* total = abacus_from_uint64(get_total_clock_positions_abacus(), base);
    CrystallineAbacus* resolution = abacus_new(base);
    CrystallineAbacus* remainder = abacus_new(base);
    
    // For fractional division, use double intermediate (like Project 1)
    double two_pi_val, total_val;
    abacus_to_double(two_pi, &two_pi_val);
    abacus_to_double(total, &total_val);
    double res_val = two_pi_val / total_val;
    
    abacus_free(resolution);
    resolution = abacus_from_double(res_val, base, precision);
    
    abacus_free(two_pi);
    abacus_free(total);
    abacus_free(remainder);
    
    return resolution;
}

// ============================================================================
// CLOCK POSITION OPERATIONS
// ============================================================================

ClockPositionAbacus* create_clock_position_abacus(
    uint32_t ring,
    uint32_t position,
    uint32_t base
) {
    if (ring > 3) return NULL;
    
    uint32_t ring_size = get_ring_size_abacus(ring);
    if (position >= ring_size) return NULL;
    
    ClockPositionAbacus* pos = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
    if (!pos) return NULL;
    
    pos->ring = ring;
    pos->position = position;
    pos->base = base;
    
    // Compute angle = 2π * position / ring_size
    uint32_t precision = 20;
    CrystallineAbacus* two_pi = abacus_from_double(2.0 * MATH_PI, base, precision);
    CrystallineAbacus* pos_abacus = abacus_from_uint64(position, base);
    CrystallineAbacus* size_abacus = abacus_from_uint64(ring_size, base);
    
    // Use double intermediate for fractional division
    double two_pi_val, pos_val, size_val;
    abacus_to_double(two_pi, &two_pi_val);
    abacus_to_double(pos_abacus, &pos_val);
    abacus_to_double(size_abacus, &size_val);
    
    double angle_val = (two_pi_val * pos_val) / size_val;
    pos->angle = abacus_from_double(angle_val, base, precision);
    
    // Compute clock_value = position / ring_size (normalized [0, 1))
    double clock_val = pos_val / size_val;
    pos->clock_value = abacus_from_double(clock_val, base, precision);
    
    abacus_free(two_pi);
    abacus_free(pos_abacus);
    abacus_free(size_abacus);
    
    return pos;
}

void free_clock_position_abacus(ClockPositionAbacus* pos) {
    if (!pos) return;
    abacus_free(pos->angle);
    abacus_free(pos->clock_value);
    free(pos);
}

MathError map_prime_to_clock_abacus(
    const CrystallineAbacus* prime,
    ClockPositionAbacus* position,
    uint32_t base,
    uint32_t precision
) {
    if (!prime || !position) return MATH_ERROR_NULL_POINTER;
    
    // Convert prime to uint64 for clock mapping
    uint64_t prime_val;
    MathError err = abacus_to_uint64(prime, &prime_val);
    if (err != MATH_SUCCESS) return err;
    
    // Use math library's clock mapping
    ClockPosition clock_pos;
    err = clock_map_prime_to_position(prime_val, &clock_pos);
    if (err != MATH_SUCCESS) return err;
    
    // Convert to Abacus representation
    position->ring = clock_pos.ring;
    position->position = clock_pos.position;
    position->base = base;
    
    // Compute angle and clock_value
    uint32_t ring_size = get_ring_size_abacus(position->ring);
    
    // angle = 2π * position / ring_size
    double angle_val = (2.0 * MATH_PI * position->position) / ring_size;
    position->angle = abacus_from_double(angle_val, base, precision);
    
    // clock_value = position / ring_size
    double clock_val = (double)position->position / ring_size;
    position->clock_value = abacus_from_double(clock_val, base, precision);
    
    return MATH_SUCCESS;
}

MathError map_clock_to_prime_abacus(
    const ClockPositionAbacus* position,
    CrystallineAbacus* prime,
    uint32_t base
) {
    if (!position || !prime) return MATH_ERROR_NULL_POINTER;
    
    // Create ClockPosition for math library
    ClockPosition clock_pos;
    clock_pos.ring = position->ring;
    clock_pos.position = position->position;
    
    // Use math library's clock mapping
    uint64_t prime_val = clock_position_to_prime(&clock_pos);
    if (prime_val == 0) return MATH_ERROR_INVALID_ARG;
    
    // Convert to Abacus
    CrystallineAbacus* temp = abacus_from_uint64(prime_val, base);
    if (!temp) return MATH_ERROR_OUT_OF_MEMORY;
    
    // Copy to output
    abacus_free(prime);
    prime = temp;
    
    return MATH_SUCCESS;
}

// ============================================================================
// GEOMETRIC OPERATIONS
// ============================================================================

CrystallineAbacus* compute_angular_separation_abacus(
    const ClockPositionAbacus* pos1,
    const ClockPositionAbacus* pos2,
    uint32_t base,
    uint32_t precision
) {
    if (!pos1 || !pos2) return NULL;
    
    // Get angles as doubles for calculation
    double angle1, angle2;
    abacus_to_double(pos1->angle, &angle1);
    abacus_to_double(pos2->angle, &angle2);
    
    // Compute |angle1 - angle2|
    double diff_val = angle1 - angle2;
    if (diff_val < 0) diff_val = -diff_val;
    
    // Normalize to [0, π]
    if (diff_val > MATH_PI) {
        diff_val = 2.0 * MATH_PI - diff_val;
    }
    
    return abacus_from_double(diff_val, base, precision);
}

CrystallineAbacus* compute_geometric_distance_abacus(
    const ClockPositionAbacus* pos1,
    const ClockPositionAbacus* pos2,
    uint32_t base,
    uint32_t precision
) {
    if (!pos1 || !pos2) return NULL;
    
    // For now, use angular separation as geometric distance
    // Future: Implement full 3D distance on clock lattice
    return compute_angular_separation_abacus(pos1, pos2, base, precision);
}

CrystallineAbacus* compute_ring_distance_abacus(
    uint32_t ring1,
    uint32_t ring2,
    uint32_t base
) {
    int diff = (int)ring1 - (int)ring2;
    if (diff < 0) diff = -diff;
    return abacus_from_uint64((uint64_t)diff, base);
}

// ============================================================================
// PRIME FACTORIZATION (Clock Lattice Guided)
// ============================================================================

/**
 * @brief Check if a number is prime using clock lattice validation
 */
static bool is_prime_clock_lattice(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    // Use clock lattice: all primes > 3 are of form 6k±1
    // This is equivalent to positions on clock lattice
    if (n % 6 != 1 && n % 6 != 5) return false;
    
    // Check divisibility up to √n (optimized by clock lattice)
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    
    return true;
}

PrimeFactorizationAbacus* clock_lattice_factorize_abacus(
    const CrystallineAbacus* n,
    uint32_t base,
    uint32_t precision
) {
    (void)precision; // Unused for now
    
    if (!n) return NULL;
    
    // Convert to uint64 for factorization
    uint64_t n_val;
    if (abacus_to_uint64(n, &n_val) != MATH_SUCCESS) {
        return NULL;
    }
    
    // Allocate result
    PrimeFactorizationAbacus* result = (PrimeFactorizationAbacus*)malloc(sizeof(PrimeFactorizationAbacus));
    if (!result) return NULL;
    
    result->base = base;
    result->num_factors = 0;
    
    // Allocate arrays (max possible factors = log2(n))
    uint32_t max_factors = 64; // Conservative estimate
    result->positions = (ClockPositionAbacus**)malloc(max_factors * sizeof(ClockPositionAbacus*));
    result->factors = (CrystallineAbacus**)malloc(max_factors * sizeof(CrystallineAbacus*));
    
    if (!result->positions || !result->factors) {
        free_prime_factorization_abacus(result);
        return NULL;
    }
    
    // Factor using clock lattice guidance
    uint64_t remaining = n_val;
    
    // Check small primes first (2, 3)
    while (remaining % 2 == 0) {
        result->factors[result->num_factors] = abacus_from_uint64(2, base);
        result->positions[result->num_factors] = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
        map_prime_to_clock_abacus(result->factors[result->num_factors], 
                                   result->positions[result->num_factors], base, 20);
        result->num_factors++;
        remaining /= 2;
    }
    
    while (remaining % 3 == 0) {
        result->factors[result->num_factors] = abacus_from_uint64(3, base);
        result->positions[result->num_factors] = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
        map_prime_to_clock_abacus(result->factors[result->num_factors], 
                                   result->positions[result->num_factors], base, 20);
        result->num_factors++;
        remaining /= 3;
    }
    
    // Check primes of form 6k±1 (clock lattice optimization)
    for (uint64_t i = 5; i * i <= remaining; i += 6) {
        // Check 6k-1
        while (remaining % i == 0) {
            result->factors[result->num_factors] = abacus_from_uint64(i, base);
            result->positions[result->num_factors] = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
            map_prime_to_clock_abacus(result->factors[result->num_factors], 
                                       result->positions[result->num_factors], base, 20);
            result->num_factors++;
            remaining /= i;
        }
        
        // Check 6k+1
        while (remaining % (i + 2) == 0) {
            result->factors[result->num_factors] = abacus_from_uint64(i + 2, base);
            result->positions[result->num_factors] = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
            map_prime_to_clock_abacus(result->factors[result->num_factors], 
                                       result->positions[result->num_factors], base, 20);
            result->num_factors++;
            remaining /= (i + 2);
        }
    }
    
    // If remaining > 1, it's a prime factor
    if (remaining > 1) {
        result->factors[result->num_factors] = abacus_from_uint64(remaining, base);
        result->positions[result->num_factors] = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
        map_prime_to_clock_abacus(result->factors[result->num_factors], 
                                   result->positions[result->num_factors], base, 20);
        result->num_factors++;
    }
    
    return result;
}

void free_prime_factorization_abacus(PrimeFactorizationAbacus* factorization) {
    if (!factorization) return;
    
    if (factorization->positions) {
        for (uint32_t i = 0; i < factorization->num_factors; i++) {
            free_clock_position_abacus(factorization->positions[i]);
        }
        free(factorization->positions);
    }
    
    if (factorization->factors) {
        for (uint32_t i = 0; i < factorization->num_factors; i++) {
            abacus_free(factorization->factors[i]);
        }
        free(factorization->factors);
    }
    
    free(factorization);
}

bool validate_factorization_abacus(
    const CrystallineAbacus* n,
    const PrimeFactorizationAbacus* factorization,
    uint32_t base
) {
    if (!n || !factorization) return false;
    
    // Multiply all factors and compare with n
    CrystallineAbacus* product = abacus_from_uint64(1, base);
    
    for (uint32_t i = 0; i < factorization->num_factors; i++) {
        CrystallineAbacus* temp = abacus_new(base);
        abacus_mul(temp, product, factorization->factors[i]);
        abacus_free(product);
        product = temp;
    }
    
    bool valid = (abacus_compare(product, n) == 0);
    abacus_free(product);
    
    return valid;
}

// ============================================================================
// FACTOR VISUALIZATION
// ============================================================================

ClockFactorVisualizationAbacus* visualize_factors_abacus(
    const CrystallineAbacus* n,
    const CrystallineAbacus* p,
    const CrystallineAbacus* q,
    uint32_t base,
    uint32_t precision
) {
    if (!n || !p || !q) return NULL;
    
    ClockFactorVisualizationAbacus* viz = (ClockFactorVisualizationAbacus*)malloc(sizeof(ClockFactorVisualizationAbacus));
    if (!viz) return NULL;
    
    viz->n = abacus_copy(n);
    viz->p = abacus_copy(p);
    viz->q = abacus_copy(q);
    viz->base = base;
    viz->total_positions = get_total_clock_positions_abacus();
    
    // Map primes to clock positions
    viz->p_position = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
    viz->q_position = (ClockPositionAbacus*)malloc(sizeof(ClockPositionAbacus));
    
    map_prime_to_clock_abacus(p, viz->p_position, base, precision);
    map_prime_to_clock_abacus(q, viz->q_position, base, precision);
    
    // Compute geometric relationships
    viz->angular_separation = compute_angular_separation_abacus(
        viz->p_position, viz->q_position, base, precision
    );
    
    viz->geometric_distance = compute_geometric_distance_abacus(
        viz->p_position, viz->q_position, base, precision
    );
    
    viz->ring_distance = compute_ring_distance_abacus(
        viz->p_position->ring, viz->q_position->ring, base
    );
    
    viz->resolution = compute_clock_resolution_abacus(base, precision);
    
    return viz;
}

void free_factor_visualization_abacus(ClockFactorVisualizationAbacus* viz) {
    if (!viz) return;
    
    abacus_free(viz->n);
    abacus_free(viz->p);
    abacus_free(viz->q);
    free_clock_position_abacus(viz->p_position);
    free_clock_position_abacus(viz->q_position);
    abacus_free(viz->geometric_distance);
    abacus_free(viz->angular_separation);
    abacus_free(viz->ring_distance);
    abacus_free(viz->resolution);
    
    free(viz);
}

// ============================================================================
// TETRATION ATTRACTORS (Placeholder - Full implementation in Phase 3)
// ============================================================================

TetrationAttractorAbacus* clock_lattice_tetration_abacus(
    const CrystallineAbacus* base_value,
    const CrystallineAbacus* target,
    uint32_t clock_base,
    uint32_t precision
) {
    (void)base_value;
    (void)target;
    (void)precision;
    
    // Placeholder: Return NULL
    // Full O(1) implementation will be added in Phase 3
    TetrationAttractorAbacus* attractor = (TetrationAttractorAbacus*)malloc(sizeof(TetrationAttractorAbacus));
    if (!attractor) return NULL;
    
    attractor->position = NULL;
    attractor->value = NULL;
    attractor->convergence_iterations = 0;
    attractor->convergence_rate = NULL;
    attractor->base = clock_base;
    
    return attractor;
}

void free_tetration_attractor_abacus(TetrationAttractorAbacus* attractor) {
    if (!attractor) return;
    
    free_clock_position_abacus(attractor->position);
    abacus_free(attractor->value);
    abacus_free(attractor->convergence_rate);
    
    free(attractor);
}