#ifndef MATH_WRAPPERS_H
#define MATH_WRAPPERS_H

/**
 * Math Wrappers for NEW Math Library
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Provides standard API for arbitrary precision math
 * - Uses CrystallineAbacus from NEW math library
 * - Supports traditional calculator-style functions
 * - Integrates with Babylonian mathematics framework
 */

#include "math/abacus.h"

/**
 * @brief Natural logarithm with arbitrary precision
 * @param result Output abacus (will be freed and replaced)
 * @param x Input value
 * @param precision Decimal precision (number of fractional digits)
 * @return New abacus with ln(x)
 */
CrystallineAbacus* math_wrapper_ln(const CrystallineAbacus* x, int precision);

/**
 * @brief Square root with arbitrary precision using Newton's method
 * @param result Output abacus (will be freed and replaced)
 * @param x Input value
 * @param precision Decimal precision (number of fractional digits)
 * @return New abacus with sqrt(x)
 */
CrystallineAbacus* math_wrapper_sqrt(const CrystallineAbacus* x, int precision);

/**
 * @brief Exponential function with arbitrary precision
 * @param result Output abacus (will be freed and replaced)
 * @param x Input value
 * @param precision Decimal precision (number of fractional digits)
 * @return New abacus with exp(x)
 */
CrystallineAbacus* math_wrapper_exp(const CrystallineAbacus* x, int precision);

/**
 * @brief Hyperbolic tangent with arbitrary precision
 * @param result Output abacus (will be freed and replaced)
 * @param x Input value
 * @param precision Decimal precision (number of fractional digits)
 * @return New abacus with tanh(x)
 */
CrystallineAbacus* math_wrapper_tanh(const CrystallineAbacus* x, int precision);

/**
 * @brief Sine function with arbitrary precision
 * @param x Input value
 * @param precision Decimal precision
 * @return New abacus with sin(x)
 */
CrystallineAbacus* math_wrapper_sin(const CrystallineAbacus* x, int precision);

/**
 * @brief Cosine function with arbitrary precision
 * @param x Input value
 * @param precision Decimal precision
 * @return New abacus with cos(x)
 */
CrystallineAbacus* math_wrapper_cos(const CrystallineAbacus* x, int precision);

/**
 * @brief Power function with arbitrary precision
 * @param base Base value
 * @param exponent Exponent value
 * @param precision Decimal precision
 * @return New abacus with base^exponent
 */
CrystallineAbacus* math_wrapper_pow(const CrystallineAbacus* base, const CrystallineAbacus* exponent, int precision);

/**
 * @brief Absolute value with arbitrary precision
 * @param x Input value
 * @return New abacus with |x|
 */
CrystallineAbacus* math_wrapper_abs(const CrystallineAbacus* x);

#endif /* MATH_WRAPPERS_H */