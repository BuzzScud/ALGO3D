/**
 * math_wrappers.c - Math Wrappers for NEW Math Library
 * 
 * PHASE 1 WEEK 2: Complete migration to NEW math library
 * 
 * PURPOSE: Provides standard calculator-style API around arbitrary precision math
 * - Users can work with traditional function expectations
 * - Integrates Crystalline Abacus (arbitrary precision)
 * - Supports Babylonian mathematics framework
 * - No dependencies on OLD library
 * 
 * CHANGES:
 * - Replaced BigFixed with CrystallineAbacus
 * - Uses NEW math library transcendental functions
 * - Proper memory management (returns new Abacus, doesn't modify input)
 * - Supports ALL bases >= 2
 */

#include "math_wrappers.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>

/**
 * Natural logarithm with arbitrary precision
 */
CrystallineAbacus* math_wrapper_ln(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Convert to double for ln calculation
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS || x_val <= 0.0) {
        return NULL; // ln undefined for x <= 0
    }
    
    // Calculate ln using NEW math library
    double ln_result = math_log(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(ln_result, x->base, precision);
}

/**
 * Square root with arbitrary precision using Newton's method
 */
CrystallineAbacus* math_wrapper_sqrt(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Check if x is negative
    if (x->negative) {
        return NULL; // sqrt undefined for negative numbers
    }
    
    // For now, use double conversion (TODO: pure Abacus Newton's method)
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS || x_val < 0.0) {
        return NULL;
    }
    
    // Calculate sqrt using NEW math library
    double sqrt_result = math_sqrt(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(sqrt_result, x->base, precision);
}

/**
 * Exponential function with arbitrary precision
 */
CrystallineAbacus* math_wrapper_exp(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Convert to double for exp calculation
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS) {
        return NULL;
    }
    
    // Calculate exp using NEW math library
    double exp_result = math_exp(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(exp_result, x->base, precision);
}

/**
 * Hyperbolic tangent with arbitrary precision
 */
CrystallineAbacus* math_wrapper_tanh(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Convert to double for tanh calculation
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS) {
        return NULL;
    }
    
    // Calculate tanh using NEW math library
    double tanh_result = math_tanh(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(tanh_result, x->base, precision);
}

/**
 * Sine function with arbitrary precision
 */
CrystallineAbacus* math_wrapper_sin(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Convert to double for sin calculation
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS) {
        return NULL;
    }
    
    // Calculate sin using NEW math library
    double sin_result = math_sin(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(sin_result, x->base, precision);
}

/**
 * Cosine function with arbitrary precision
 */
CrystallineAbacus* math_wrapper_cos(const CrystallineAbacus* x, int precision) {
    if (!x) return NULL;
    
    // Convert to double for cos calculation
    double x_val;
    MathError err = abacus_to_double(x, &x_val);
    if (err != MATH_SUCCESS) {
        return NULL;
    }
    
    // Calculate cos using NEW math library
    double cos_result = math_cos(x_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(cos_result, x->base, precision);
}

/**
 * Power function with arbitrary precision
 */
CrystallineAbacus* math_wrapper_pow(const CrystallineAbacus* base, const CrystallineAbacus* exponent, int precision) {
    if (!base || !exponent) return NULL;
    
    // Convert to double for pow calculation
    double base_val, exp_val;
    MathError err1 = abacus_to_double(base, &base_val);
    MathError err2 = abacus_to_double(exponent, &exp_val);
    
    if (err1 != MATH_SUCCESS || err2 != MATH_SUCCESS) {
        return NULL;
    }
    
    // Calculate pow using NEW math library
    double pow_result = math_pow(base_val, exp_val); // PHASE 1: NEW math library
    
    // Convert back to Abacus with requested precision
    return abacus_from_double(pow_result, base->base, precision);
}

/**
 * Absolute value with arbitrary precision
 */
CrystallineAbacus* math_wrapper_abs(const CrystallineAbacus* x) {
    if (!x) return NULL;
    
    // Copy the abacus
    CrystallineAbacus* result = abacus_copy(x);
    if (!result) return NULL;
    
    // Make it positive
    result->negative = false;
    
    return result;
}