/**
 * @file abacus_array_utils.h
 * @brief Crystalline Abacus Array Utilities
 * 
 * Helper functions for allocating and managing CrystallineAbacus arrays.
 * Replaces the old BigFixed array utilities with NEW math library.
 */

#ifndef ABACUS_ARRAY_UTILS_H
#define ABACUS_ARRAY_UTILS_H

#include "math/abacus.h"
#include "math/types.h"
#include "math/clock.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create an array of CrystallineAbacus values
 * @param size Number of elements in the array
 * @param base Number base (12, 60, 100, or any >= 2)
 * @param ctx Clock context for operations
 * @return Array of CrystallineAbacus* pointers, each initialized to zero
 */
CrystallineAbacus** abacus_array_create(size_t size, uint32_t base, ClockContext* ctx);

/**
 * @brief Free an array of CrystallineAbacus values
 * @param array Array to free
 * @param size Number of elements in the array
 */
void abacus_array_free(CrystallineAbacus** array, size_t size);

/**
 * @brief Set all elements of a CrystallineAbacus array to zero
 * @param array Array to zero
 * @param size Number of elements in the array
 */
void abacus_array_zero(CrystallineAbacus** array, size_t size);

/**
 * @brief Copy a CrystallineAbacus array
 * @param dest Destination array
 * @param src Source array
 * @param size Number of elements to copy
 */
void abacus_array_copy(CrystallineAbacus** dest, CrystallineAbacus** src, size_t size);

/**
 * @brief Convert float array to CrystallineAbacus array
 * @param dest Destination Abacus array
 * @param src Source float array
 * @param size Number of elements
 * @param base Number base
 * @param precision Fractional precision (negative exponent)
 * @param ctx Clock context
 */
void abacus_array_from_float(CrystallineAbacus** dest, const float* src, size_t size,
                             uint32_t base, int32_t precision, ClockContext* ctx);

/**
 * @brief Convert CrystallineAbacus array to float array
 * @param dest Destination float array
 * @param src Source Abacus array
 * @param size Number of elements
 */
void abacus_array_to_float(float* dest, CrystallineAbacus** src, size_t size);

#ifdef __cplusplus
}
#endif

#endif // ABACUS_ARRAY_UTILS_H