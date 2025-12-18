/**
 * @file cllm_lattice_helpers.c
 * @brief Helper functions for CLLM lattice operations
 * 
 * Migrated from src/core/cllm_mathematical_constants.c
 * Only includes functions needed by CLLM that don't use OLD BigInt/BigFixed library
 */

#include "math/types.h"
#include "math/constants.h"
#include <stdint.h>
#include <stdio.h>

// Dimensional frequencies (from Babylonian mathematics)
// These match the extern declarations in cllm_mathematical_constants.h
const uint64_t DIMENSIONAL_FREQUENCIES[] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37,
    41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89
};

const size_t DIMENSIONAL_FREQUENCIES_COUNT = 
    sizeof(DIMENSIONAL_FREQUENCIES) / sizeof(DIMENSIONAL_FREQUENCIES[0]);

/**
 * @brief Get dimensional frequency for a given dimension
 * 
 * Based on Babylonian mathematics and 12-fold symmetry.
 * 
 * @param dimension Dimension index (0-based)
 * @return Frequency value
 */
uint64_t cllm_get_dimensional_frequency(size_t dimension) {
    if (dimension >= DIMENSIONAL_FREQUENCIES_COUNT) {
        // For dimensions beyond our table, return a reasonable default
        return DIMENSIONAL_FREQUENCIES[DIMENSIONAL_FREQUENCIES_COUNT - 1];
    }
    return DIMENSIONAL_FREQUENCIES[dimension];
}

/**
 * @brief Check if prime is near 144000 boundary
 * 
 * The 144000 boundary is significant in Babylonian mathematics
 * and represents a vector culmination point.
 * 
 * @param prime Prime value to check
 * @return 1 if near boundary, 0 otherwise
 */
int cllm_is_near_144000_boundary(uint64_t prime) {
    // Define "near" as within 100 of the boundary
    const uint64_t boundary_range = 100;
    
    // VECTOR_CULMINATION, TWIN_PRIME_LOWER, TWIN_PRIME_UPPER are defined in prime_types.h
    if (prime >= VECTOR_CULMINATION - boundary_range &&
        prime <= VECTOR_CULMINATION + boundary_range) {
        return 1;
    }
    
    // Also check if it's one of the twin primes near 144000
    if (prime == TWIN_PRIME_LOWER || prime == TWIN_PRIME_UPPER) {
        return 1;
    }
    
    return 0;
}
