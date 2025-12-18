/**
 * @file geometric_number.h
 * @brief Pure Geometric Number System - Thesis-Aligned Implementation
 * 
 * This implements the PURE geometric model from the thesis:
 * - Numbers as (position, magnitude) pairs
 * - O(1) operations for fixed-precision
 * - O(log m) operations for arbitrary precision magnitudes
 * - Geometric transformations, not digit-by-digit algorithms
 * 
 * THREE IMPLEMENTATIONS:
 * 1. GeometricNumber64: Fast O(1) for 64-bit magnitudes
 * 2. GeometricNumberBig: Arbitrary precision with O(log m) magnitude ops
 * 3. BabylonianNumber: Pure 4-ring Babylonian system
 */

#ifndef MATH_GEOMETRIC_NUMBER_H
#define MATH_GEOMETRIC_NUMBER_H

#include "math/types.h"
#include "math/clock.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * GEOMETRIC NUMBER - FAST 64-BIT VERSION
 * ============================================================================
 * 
 * Pure geometric model with 64-bit magnitude.
 * TRUE O(1) operations as described in thesis.
 * 
 * Representation: n = magnitude × base + position
 * 
 * Example (base 12):
 *   157 = 13×12 + 1
 *   position = 1
 *   magnitude = 13
 */

typedef struct {
    uint64_t magnitude;      /**< Number of complete rotations around clock */
    uint8_t position;        /**< Position on clock (0 to base-1) */
    uint8_t base;           /**< Number base (12, 60, 100, or any base >= 2) */
    bool negative;          /**< Sign of the number */
} GeometricNumber64;

/**
 * @brief Create geometric number from integer
 * @param value Integer value
 * @param base Number base (12, 60, 100, etc.)
 * @return Geometric number
 */
GeometricNumber64 geo64_from_int(int64_t value, uint8_t base);

/**
 * @brief Convert geometric number to integer
 * @param num Geometric number
 * @return Integer value
 */
int64_t geo64_to_int(const GeometricNumber64* num);

/**
 * @brief Add two geometric numbers (O(1))
 * @param result Output
 * @param a First operand
 * @param b Second operand
 * @return Error code
 * 
 * Algorithm:
 *   p_sum = a.position + b.position
 *   carry = p_sum / base
 *   result.position = p_sum % base
 *   result.magnitude = a.magnitude + b.magnitude + carry
 * 
 * Complexity: O(1) - constant time regardless of magnitude size
 */
MathError geo64_add(GeometricNumber64* result, 
                    const GeometricNumber64* a, 
                    const GeometricNumber64* b);

/**
 * @brief Subtract two geometric numbers (O(1))
 * @param result Output
 * @param a First operand
 * @param b Second operand
 * @return Error code
 */
MathError geo64_sub(GeometricNumber64* result,
                    const GeometricNumber64* a,
                    const GeometricNumber64* b);

/**
 * @brief Multiply two geometric numbers (O(1))
 * @param result Output
 * @param a First operand
 * @param b Second operand
 * @return Error code
 * 
 * Algorithm:
 *   Angle addition: result.position = (a.position * b.position) % base
 *   Magnitude scaling: result.magnitude = a.magnitude * b.magnitude
 * 
 * Complexity: O(1) for 64-bit magnitudes
 */
MathError geo64_mul(GeometricNumber64* result,
                    const GeometricNumber64* a,
                    const GeometricNumber64* b);

/**
 * @brief Divide two geometric numbers (O(1))
 * @param quotient Output quotient
 * @param remainder Output remainder (can be NULL)
 * @param a Dividend
 * @param b Divisor
 * @return Error code
 * 
 * Algorithm:
 *   Uses geometric triangulation (not iterative subtraction)
 *   quotient.magnitude = a.magnitude / b.magnitude
 *   remainder.magnitude = a.magnitude % b.magnitude
 * 
 * Complexity: O(1) for 64-bit magnitudes
 */
MathError geo64_div(GeometricNumber64* quotient,
                    GeometricNumber64* remainder,
                    const GeometricNumber64* a,
                    const GeometricNumber64* b);

/**
 * @brief Compare two geometric numbers
 * @return -1 if a < b, 0 if a == b, 1 if a > b
 */
int geo64_compare(const GeometricNumber64* a, const GeometricNumber64* b);

/* ============================================================================
 * BABYLONIAN NUMBER - PURE 4-RING SYSTEM
 * ============================================================================
 * 
 * Pure Babylonian clock lattice with 4 rings:
 * - Ring 0: 12 positions (hours)
 * - Ring 1: 60 positions (minutes)
 * - Ring 2: 60 positions (seconds)
 * - Ring 3: 100 positions (milliseconds)
 * 
 * Total precision: 12 × 60 × 60 × 100 = 4,320,000 positions
 * 
 * This is the EXACT model from the thesis.
 */

typedef struct {
    uint8_t ring0;   /**< Hours (0-11) */
    uint8_t ring1;   /**< Minutes (0-59) */
    uint8_t ring2;   /**< Seconds (0-59) */
    uint8_t ring3;   /**< Milliseconds (0-99) */
    bool negative;   /**< Sign */
} BabylonianNumber;

/**
 * @brief Create Babylonian number from integer
 * @param value Integer value (must fit in 4,320,000)
 * @return Babylonian number
 */
BabylonianNumber babylon_from_int(int32_t value);

/**
 * @brief Convert Babylonian number to integer
 * @param num Babylonian number
 * @return Integer value
 */
int32_t babylon_to_int(const BabylonianNumber* num);

/**
 * @brief Add two Babylonian numbers (O(1))
 * @param result Output
 * @param a First operand
 * @param b Second operand
 * @return Error code
 * 
 * Algorithm:
 *   Add each ring with carry propagation
 *   Ring 3: carry to ring 2 at 100
 *   Ring 2: carry to ring 1 at 60
 *   Ring 1: carry to ring 0 at 60
 *   Ring 0: overflow at 12
 * 
 * Complexity: O(1) - exactly 4 ring operations
 */
MathError babylon_add(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b);

/**
 * @brief Subtract two Babylonian numbers (O(1))
 */
MathError babylon_sub(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b);

/**
 * @brief Multiply two Babylonian numbers (O(1))
 */
MathError babylon_mul(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b);

/**
 * @brief Divide two Babylonian numbers (O(1))
 */
MathError babylon_div(BabylonianNumber* quotient,
                      BabylonianNumber* remainder,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b);

/* ============================================================================
 * GEOMETRIC NUMBER - ARBITRARY PRECISION VERSION
 * ============================================================================
 * 
 * Geometric model with arbitrary precision magnitude.
 * Uses CrystallineAbacus internally for magnitude storage.
 * 
 * Complexity: O(1) structure + O(log m) magnitude operations
 * 
 * This bridges the gap between pure O(1) and arbitrary precision.
 */

typedef struct GeometricNumberBig GeometricNumberBig;

/**
 * @brief Create arbitrary precision geometric number
 * @param base Number base
 * @return New geometric number (must be freed)
 */
GeometricNumberBig* geobig_new(uint8_t base);

/**
 * @brief Free arbitrary precision geometric number
 */
void geobig_free(GeometricNumberBig* num);

/**
 * @brief Create from integer
 */
GeometricNumberBig* geobig_from_int64(int64_t value, uint8_t base);

/**
 * @brief Convert to integer (if it fits)
 */
MathError geobig_to_int64(const GeometricNumberBig* num, int64_t* value);

/**
 * @brief Add two arbitrary precision geometric numbers
 * @param result Output
 * @param a First operand
 * @param b Second operand
 * @return Error code
 * 
 * Complexity: O(1) structure + O(log m) magnitude addition
 * where m = max(magnitude_a, magnitude_b)
 */
MathError geobig_add(GeometricNumberBig* result,
                     const GeometricNumberBig* a,
                     const GeometricNumberBig* b);

/**
 * @brief Subtract two arbitrary precision geometric numbers
 */
MathError geobig_sub(GeometricNumberBig* result,
                     const GeometricNumberBig* a,
                     const GeometricNumberBig* b);

/**
 * @brief Multiply two arbitrary precision geometric numbers
 * 
 * Complexity: O(1) structure + O(log m × log n) magnitude multiplication
 */
MathError geobig_mul(GeometricNumberBig* result,
                     const GeometricNumberBig* a,
                     const GeometricNumberBig* b);

/**
 * @brief Divide two arbitrary precision geometric numbers
 * 
 * Uses geometric triangulation, not iterative subtraction.
 * Complexity: O(1) structure + O(log m × log n) magnitude division
 */
MathError geobig_div(GeometricNumberBig* quotient,
                     GeometricNumberBig* remainder,
                     const GeometricNumberBig* a,
                     const GeometricNumberBig* b);

/* ============================================================================
 * UNIFIED INTERFACE
 * ============================================================================
 * 
 * Automatically chooses the right implementation based on size:
 * - Small numbers (< 2^32): BabylonianNumber (O(1))
 * - Medium numbers (< 2^64): GeometricNumber64 (O(1))
 * - Large numbers (>= 2^64): GeometricNumberBig (O(log m))
 */

typedef enum {
    GEO_TYPE_BABYLON,   /**< 4-ring Babylonian (< 4,320,000) */
    GEO_TYPE_64BIT,     /**< 64-bit magnitude (< 2^64) */
    GEO_TYPE_BIGINT     /**< Arbitrary precision (>= 2^64) */
} GeometricType;

typedef struct {
    GeometricType type;
    union {
        BabylonianNumber babylon;
        GeometricNumber64 geo64;
        GeometricNumberBig* geobig;
    } data;
} GeometricNumber;

/**
 * @brief Create geometric number (auto-selects type)
 */
GeometricNumber* geo_new(uint8_t base);

/**
 * @brief Free geometric number
 */
void geo_free(GeometricNumber* num);

/**
 * @brief Create from integer (auto-selects type)
 */
GeometricNumber* geo_from_int64(int64_t value, uint8_t base);

/**
 * @brief Add (auto-dispatches to correct implementation)
 */
MathError geo_add(GeometricNumber* result,
                  const GeometricNumber* a,
                  const GeometricNumber* b);

/**
 * @brief Subtract (auto-dispatches)
 */
MathError geo_sub(GeometricNumber* result,
                  const GeometricNumber* a,
                  const GeometricNumber* b);

/**
 * @brief Multiply (auto-dispatches)
 */
MathError geo_mul(GeometricNumber* result,
                  const GeometricNumber* a,
                  const GeometricNumber* b);

/**
 * @brief Divide (auto-dispatches)
 */
MathError geo_div(GeometricNumber* quotient,
                  GeometricNumber* remainder,
                  const GeometricNumber* a,
                  const GeometricNumber* b);

#ifdef __cplusplus
}
#endif

#endif /* MATH_GEOMETRIC_NUMBER_H */