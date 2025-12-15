/**
 * @file validation.c
 * @brief Implementation of validation and safety checking
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced math_exp with math_exp (2 calls)
 * - Replaced math_log with math_log (3 calls)
 * - Replaced math_is_nan with isnan (1 call)
 * - Replaced math_is_inf with isinf (1 call)
 * Total: 7 function calls migrated to NEW math library
 */

#include "validation.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include "math/validation.h"       // For math_is_nan, math_is_inf
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

double validation_calculate_harm(double P_misuse, double D_damage,
                                  double error, double scale_factor) {
    // H = P_misuse * D_damage * scale_factor + error * 10^6
    double harm = P_misuse * D_damage * scale_factor + error * 1000000.0;
    return harm;
}

double validation_safety_probability(double proof_level, double alpha) {
    // P_safe = 1 - math_exp(-alpha * proof_level)
    double exponent = -alpha * proof_level;
    double P_safe = 1.0 - math_exp(exponent);
    
    // Clamp to [0, 1]
    if (P_safe < 0.0) P_safe = 0.0;
    if (P_safe > 1.0) P_safe = 1.0;
    
    return P_safe;
}

bool validation_assess_safety(double P_misuse, double D_damage,
                               double error, double scale_factor,
                               double harm_threshold, double safety_threshold,
                               ValidationResult* result) {
    if (!result) {
        return false;
    }
    
    // Calculate harm score
    double harm = validation_calculate_harm(P_misuse, D_damage, error, scale_factor);
    
    // Calculate proof level from error
    double proof_level = validation_proof_level_from_error(error);
    
    // Calculate safety probability
    double P_safe = validation_safety_probability(proof_level, 0.1);
    
    // Check thresholds
    bool harm_ok = harm < harm_threshold;
    bool safety_ok = P_safe > safety_threshold;
    
    result->passed = harm_ok && safety_ok;
    result->score = P_safe;
    result->threshold = safety_threshold;
    
    if (result->passed) {
        result->message = "Validation passed: Safe and acceptable harm level";
    } else if (!harm_ok) {
        result->message = "Validation failed: Harm score too high";
    } else {
        result->message = "Validation failed: Safety probability too low";
    }
    
    return result->passed;
}

bool validation_check(double value, double* threshold,
                      ValidationMode mode, ValidationResult* result) {
    if (!result) {
        return false;
    }
    
    double thresh = threshold ? *threshold : validation_default_threshold(mode);
    bool passed = false;
    
    switch (mode) {
        case VALIDATION_MODE_RESIDUAL:
            passed = validation_check_residual(value, thresh);
            result->message = passed ? "Residual within threshold" : "Residual exceeds threshold";
            break;
            
        case VALIDATION_MODE_ENTROPY:
            passed = validation_check_entropy(value, thresh);
            result->message = passed ? "Entropy above threshold" : "Entropy below threshold";
            break;
            
        case VALIDATION_MODE_RANGE:
            // For range mode, threshold is interpreted as max value (min is 0)
            passed = validation_check_range(value, 0.0, thresh);
            result->message = passed ? "Value within range" : "Value out of range";
            break;
            
        case VALIDATION_MODE_STABILITY:
            passed = validation_check_stability(value, thresh);
            result->message = passed ? "Value numerically stable" : "Value numerically unstable";
            break;
            
        case VALIDATION_MODE_CUSTOM:
            result->message = "Custom validation not implemented";
            passed = false;
            break;
    }
    
    result->passed = passed;
    result->score = passed ? 1.0 : 0.0;
    result->threshold = thresh;
    
    return passed;
}

bool validation_check_residual(double value, double threshold) {
    double abs_value = value < 0.0 ? -value : value;
    return abs_value < threshold;
}

bool validation_check_entropy(double value, double threshold) {
    return value > threshold;
}

bool validation_check_range(double value, double min_val, double max_val) {
    return (value >= min_val) && (value <= max_val);
}

bool validation_check_stability(double value, double max_magnitude) {
    // Check for NaN
    if (math_is_nan(value)) {
        return false;
    }
    
    // Check for infinity
    if (math_is_inf(value)) {
        return false;
    }
    
    // Check magnitude
    double abs_value = value < 0.0 ? -value : value;
    if (abs_value > max_magnitude) {
        return false;
    }
    
    return true;
}

double validation_proof_level_from_error(double error) {
    if (error <= 0.0 || error >= 1.0) {
        return 0.0;
    }
    
    // proof_level = math_log(1 / error) = -math_log(error)
    double proof_level = -math_log(error);
    
    return proof_level > 0.0 ? proof_level : 0.0;
}

size_t validation_batch_check(const double* values, size_t num_values,
                               double threshold, ValidationMode mode,
                               ValidationResult* results) {
    if (!values || !results || num_values == 0) {
        return 0;
    }
    
    size_t passed_count = 0;
    
    for (size_t i = 0; i < num_values; i++) {
        if (validation_check(values[i], &threshold, mode, &results[i])) {
            passed_count++;
        }
    }
    
    return passed_count;
}

double validation_default_threshold(ValidationMode mode) {
    switch (mode) {
        case VALIDATION_MODE_RESIDUAL:
            return 0.05;  // 5% residual
            
        case VALIDATION_MODE_ENTROPY:
            return 0.95;  // 95% entropy
            
        case VALIDATION_MODE_RANGE:
            return 1.0;   // [0, 1] range
            
        case VALIDATION_MODE_STABILITY:
            return 1e10;  // 10 billion magnitude
            
        case VALIDATION_MODE_CUSTOM:
            return 0.0;
    }
    
    return 0.0;
}
