/**
 * @file validation.h
 * @brief General-purpose validation and safety checking algorithms
 * 
 * This module provides validation functions for assessing the safety,
 * reliability, and quality of computational results. Applications include:
 * - AI model validation (safety, bias, reliability)
 * - Signal processing (noise, distortion, quality)
 * - Numerical analysis (stability, convergence, accuracy)
 * - Data integrity (corruption, outliers, consistency)
 * 
 * The validation system uses:
 * - Harm/risk scoring based on probability and impact
 * - Safety probability calculations
 * - Residual and entropy-based thresholds
 * - Multi-mode validation strategies
 */

#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validation mode
 */
typedef enum {
    VALIDATION_MODE_RESIDUAL,   /**< Check if residual is below threshold */
    VALIDATION_MODE_ENTROPY,    /**< Check if entropy is above threshold */
    VALIDATION_MODE_RANGE,      /**< Check if value is within range */
    VALIDATION_MODE_STABILITY,  /**< Check numerical stability */
    VALIDATION_MODE_CUSTOM      /**< Custom validation function */
} ValidationMode;

/**
 * @brief Validation result
 */
typedef struct {
    bool passed;                /**< Whether validation passed */
    double score;               /**< Validation score (0.0 to 1.0) */
    double threshold;           /**< Threshold used */
    const char* message;        /**< Validation message */
} ValidationResult;

/**
 * @brief Custom validation function callback
 * 
 * @param value Value to validate
 * @param threshold Threshold to use
 * @param user_data User context
 * @return Validation result
 */
typedef ValidationResult (*CustomValidationFn)(double value, double threshold, void* user_data);

/**
 * @brief Calculate harm/risk score
 * 
 * Computes: H = P_misuse * D_damage * scale_factor + error * 10^6
 * 
 * This provides a quantitative measure of potential harm or risk.
 * Lower scores indicate safer/better results.
 * 
 * @param P_misuse Probability of misuse (0.0 to 1.0)
 * @param D_damage Damage/impact potential (scale 1-100)
 * @param error Error tolerance/magnitude
 * @param scale_factor Scaling factor for harm calculation
 * @return Harm score
 */
double validation_calculate_harm(double P_misuse, double D_damage,
                                  double error, double scale_factor);

/**
 * @brief Calculate safety probability
 * 
 * Computes: P_safe = 1 - exp(-alpha * proof_level)
 * 
 * Higher proof levels indicate more rigorous validation,
 * resulting in higher safety probability.
 * 
 * @param proof_level Level of proof/validation (higher = safer)
 * @param alpha Sensitivity parameter (default: 0.1)
 * @return Safety probability (0.0 to 1.0)
 */
double validation_safety_probability(double proof_level, double alpha);

/**
 * @brief Comprehensive harm and safety assessment
 * 
 * Combines harm scoring and safety probability to determine
 * if a result passes validation.
 * 
 * @param P_misuse Probability of misuse
 * @param D_damage Damage potential
 * @param error Error magnitude
 * @param scale_factor Harm scaling factor
 * @param harm_threshold Maximum acceptable harm (default: 100.0)
 * @param safety_threshold Minimum safety probability (default: 0.94)
 * @param result Output validation result
 * @return true if validation passes
 */
bool validation_assess_safety(double P_misuse, double D_damage,
                               double error, double scale_factor,
                               double harm_threshold, double safety_threshold,
                               ValidationResult* result);

/**
 * @brief Validate value using specified mode
 * 
 * @param value Value to validate
 * @param threshold Threshold (NULL for default)
 * @param mode Validation mode
 * @param result Output validation result
 * @return true if validation passes
 */
bool validation_check(double value, double* threshold,
                      ValidationMode mode, ValidationResult* result);

/**
 * @brief Validate residual value
 * 
 * Checks if |value| < threshold
 * 
 * @param value Residual value
 * @param threshold Maximum acceptable residual (default: 0.05)
 * @return true if residual is acceptable
 */
bool validation_check_residual(double value, double threshold);

/**
 * @brief Validate entropy value
 * 
 * Checks if value > threshold
 * 
 * @param value Entropy value
 * @param threshold Minimum acceptable entropy (default: 0.95)
 * @return true if entropy is acceptable
 */
bool validation_check_entropy(double value, double threshold);

/**
 * @brief Validate value is within range
 * 
 * Checks if min_val <= value <= max_val
 * 
 * @param value Value to check
 * @param min_val Minimum acceptable value
 * @param max_val Maximum acceptable value
 * @return true if value is in range
 */
bool validation_check_range(double value, double min_val, double max_val);

/**
 * @brief Validate numerical stability
 * 
 * Checks for NaN, infinity, and extreme values
 * 
 * @param value Value to check
 * @param max_magnitude Maximum acceptable magnitude
 * @return true if value is numerically stable
 */
bool validation_check_stability(double value, double max_magnitude);

/**
 * @brief Calculate proof level from error magnitude
 * 
 * Computes: proof_level = log(1 / error)
 * 
 * @param error Error magnitude
 * @return Proof level
 */
double validation_proof_level_from_error(double error);

/**
 * @brief Batch validation of multiple values
 * 
 * @param values Array of values to validate
 * @param num_values Number of values
 * @param threshold Threshold to use
 * @param mode Validation mode
 * @param results Output array of results (pre-allocated)
 * @return Number of values that passed validation
 */
size_t validation_batch_check(const double* values, size_t num_values,
                               double threshold, ValidationMode mode,
                               ValidationResult* results);

/**
 * @brief Get default threshold for validation mode
 * 
 * @param mode Validation mode
 * @return Default threshold
 */
double validation_default_threshold(ValidationMode mode);

#ifdef __cplusplus
}
#endif

#endif /* VALIDATION_H */
