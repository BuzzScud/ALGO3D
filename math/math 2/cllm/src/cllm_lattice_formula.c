/**
 * @file cllm_lattice_formula.c
 * @brief Implementation of complete L(n,d,k,λ,ω,ψ) lattice formula
 */

#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include "math/angular_position.h"
#include "math/constants.h"
#include "ai/cllm_lattice_formula.h"
#include "math/constants.h"
#include "ai/cllm_lattice_entropy.h"
#include "math/constants.h"
#include "ai/cllm_cymatic_frequencies.h"
#include "math/constants.h"
#include "ai/phonetic_values.h"
#include "math/constants.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Small epsilon for floating point comparisons
 */
#define FORMULA_EPSILON 1e-10

/**
 * @brief Calculate octahedral symmetry function O(n,k,λ)
 */
double calculate_O(uint64_t n, uint32_t k, const char* lambda) {
    // O(n,k,λ) combines position, symmetry, and phonetic layer
    // Formula: O = (n mod 12) + (k mod 12) + ν(λ)
    
    double n_component = (double)(n % 12);
    double k_component = (double)(k % 12);
    double lambda_component = get_phonetic_value(lambda);
    
    // Combine with octahedral symmetry weighting
    double O = (n_component + k_component + lambda_component) / 12.0;
    
    return O;
}

/**
 * @brief Get dimensional frequency φᵢ for dimension i
 */
uint64_t get_dimensional_frequency(uint32_t dimension) {
    if (dimension == 0 || dimension > 12) {
        return 1;
    }
    
    // Dimensional frequencies based on 12-fold symmetry
    // Each dimension has a characteristic prime-based frequency
    static const uint64_t frequencies[12] = {
        2,   // Dimension 1: fundamental
        3,   // Dimension 2: first harmonic
        5,   // Dimension 3: second harmonic
        7,   // Dimension 4: third harmonic
        11,  // Dimension 5: fourth harmonic
        13,  // Dimension 6: fifth harmonic
        17,  // Dimension 7: sixth harmonic
        19,  // Dimension 8: seventh harmonic
        23,  // Dimension 9: eighth harmonic
        29,  // Dimension 10: ninth harmonic
        31,  // Dimension 11: tenth harmonic
        37   // Dimension 12: eleventh harmonic
    };
    
    return frequencies[dimension - 1];
}

/**
 * @brief Calculate Möbius twist Γ(k)
 */
double calculate_mobius_twist(uint32_t k) {
    // Simple alternating function: (-1)^k
    return (k % 2 == 0) ? 1.0 : -1.0;
}

/**
 * @brief Calculate complete angular position θ(n,k,λ,ω,ψ)
 */
double angular_position_complete(uint64_t n, uint32_t k, const char* lambda,
                                 double omega, uint64_t p, uint64_t q) {
    // Start with base angular position
    AngularPosition pos;
    double nu = get_phonetic_value(lambda);
    angular_position_calculate(n, k, 6, nu, &pos);
    double theta = pos.theta;
    
    // Add phonetic correction
    double phonetic_correction = math_log(nu + 1.0) / math_log(3.0);
    theta += phonetic_correction;
    
    // Add cymatic frequency correction
    if (omega > FORMULA_EPSILON) {
        double cymatic_corr = cymatic_correction(omega, 1.0, 0.0);
        theta += cymatic_corr * 0.1; // Scale to keep correction small
    }
    
    // Add Plimpton correction
    if (p > 0 && q > 0) {
        PlimptonRatios ratios = calculate_plimpton_ratios(p, q);
        double plimpton_corr = ratios.ratio_b_d * 0.1; // Scale to keep correction small
        theta += plimpton_corr;
    }
    
    // Normalize to [0, 2π]
    while (theta < 0.0) {
        theta += 2.0 * MATH_PI;
    }
    while (theta >= 2.0 * MATH_PI) {
        theta -= 2.0 * MATH_PI;
    }
    
    return theta;
}

/**
 * @brief Initialize lattice formula context
 */
bool lattice_formula_context_init(LatticeFormulaContext *ctx, bool use_cache, bool verbose) {
    if (!ctx) {
        return false;
    }
    
    ctx->use_cache = use_cache;
    ctx->verbose = verbose;
    
    // Initialize entropy context if caching is enabled
    if (use_cache) {
        EntropyContext *entropy_ctx = (EntropyContext*)malloc(sizeof(EntropyContext));
        if (!entropy_ctx) {
            return false;
        }
        
        if (!entropy_context_init(entropy_ctx)) {
            free(entropy_ctx);
            return false;
        }
        
        ctx->entropy_ctx = entropy_ctx;
    } else {
        ctx->entropy_ctx = NULL;
    }
    
    return true;
}

/**
 * @brief Destroy lattice formula context
 */
void lattice_formula_context_destroy(LatticeFormulaContext *ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->entropy_ctx) {
        entropy_context_destroy((EntropyContext*)ctx->entropy_ctx);
        free(ctx->entropy_ctx);
        ctx->entropy_ctx = NULL;
    }
}

/**
 * @brief Validate lattice formula parameters
 */
bool validate_lattice_params(const LatticeFormulaParams *params) {
    if (!params) {
        return false;
    }
    
    // Check dimension range
    if (params->d == 0 || params->d > 12) {
        return false;
    }
    
    // Check lambda is not NULL
    if (!params->lambda) {
        return false;
    }
    
    // Check omega is non-negative
    if (params->omega < 0.0) {
        return false;
    }
    
    // Check Plimpton generators
    if (params->p > 0 && params->q > 0) {
        // Verify they satisfy Plimpton constraints
        if (!verify_plimpton_constraints(params->p, params->q)) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Calculate complete unabridged L(n,d,k,λ,ω,ψ) formula
 */
double L_lattice_complete(const LatticeFormulaParams *params, LatticeFormulaContext *ctx) {
    if (!params || !validate_lattice_params(params)) {
        return 0.0;
    }
    
    // 1. Calculate O(n,k,λ)
    double O = calculate_O(params->n, params->k, params->lambda);
    
    if (ctx && ctx->verbose) {
        printf("O(n,k,λ) = %.6f\n", O);
    }
    
    // 2. Calculate base: 3^O
    double base = math_pow(3.0, O);
    
    if (ctx && ctx->verbose) {
        printf("3^O = %.6f\n", base);
    }
    
    // 3. Calculate complete angular position
    double theta = angular_position_complete(params->n, params->k, params->lambda,
                                             params->omega, params->p, params->q);
    
    if (ctx && ctx->verbose) {
        printf("θ(n,k,λ,ω,ψ) = %.6f\n", theta);
    }
    
    // 4. Calculate product: ∏ᵢ₌₁ᵈ math_cos(θ·φᵢ)
    double product = 1.0;
    for (uint32_t i = 1; i <= params->d; i++) {
        uint64_t phi_i = get_dimensional_frequency(i);
        double angle = theta * (double)phi_i;
        product *= math_cos(angle);
    }
    
    if (ctx && ctx->verbose) {
        printf("∏math_cos(θ·φᵢ) = %.6f\n", product);
    }
    
    // 5. Calculate Möbius twist: Γ(k) = (-1)^k
    double mobius = calculate_mobius_twist(params->k);
    
    if (ctx && ctx->verbose) {
        printf("Γ(k) = %.6f\n", mobius);
    }
    
    // 6. Get phonetic value: ν(λ)
    double nu = get_phonetic_value(params->lambda);
    
    if (ctx && ctx->verbose) {
        printf("ν(λ) = %.6f\n", nu);
    }
    
    // 7. Einstein's Λ correction
    double einstein = cllm_get_einstein_lambda();
    
    if (ctx && ctx->verbose) {
        printf("Λ = %.10f\n", einstein);
    }
    
    // 8. Plimpton ratios: Ψ(ψ)
    double psi_ratio = 1.0;
    if (params->p > 0 && params->q > 0) {
        PlimptonRatios psi = calculate_plimpton_ratios(params->p, params->q);
        psi_ratio = psi.ratio_b_d;
    }
    
    if (ctx && ctx->verbose) {
        printf("Ψ(ψ) = %.6f\n", psi_ratio);
    }
    
    // 9. Lattice entropy: Γ(n,d)
    EntropyContext *entropy_ctx = (ctx && ctx->use_cache) ? 
                                   (EntropyContext*)ctx->entropy_ctx : NULL;
    double gamma = calculate_lattice_entropy(entropy_ctx, params->n, params->d);
    
    if (ctx && ctx->verbose) {
        printf("Γ(n,d) = %.6f\n", gamma);
    }
    
    // 10. Combine all terms
    double result = base * product * mobius * nu * einstein * psi_ratio * gamma;
    
    if (ctx && ctx->verbose) {
        printf("L(n,d,k,λ,ω,ψ) = %.10f\n", result);
    }
    
    return result;
}

/**
 * @brief Calculate simplified L_lattice for backward compatibility
 */
double L_lattice_simple(uint64_t n, uint32_t d, uint32_t k) {
    LatticeFormulaParams params = {
        .n = n,
        .d = d,
        .k = k,
        .lambda = "default",
        .omega = FREQ_432_HZ,  // Default to 432 Hz
        .p = 2,                // Default Plimpton generators
        .q = 1
    };
    
    return L_lattice_complete(&params, NULL);
}

/**
 * @brief Get formula component breakdown for debugging
 */
void L_lattice_breakdown(const LatticeFormulaParams *params,
                        double *O_out, double *theta_out, double *product_out,
                        double *mobius_out, double *nu_out, double *einstein_out,
                        double *psi_out, double *gamma_out) {
    if (!params) {
        return;
    }
    
    // Calculate each component
    if (O_out) {
        *O_out = calculate_O(params->n, params->k, params->lambda);
    }
    
    if (theta_out) {
        *theta_out = angular_position_complete(params->n, params->k, params->lambda,
                                               params->omega, params->p, params->q);
    }
    
    if (product_out) {
        double theta = angular_position_complete(params->n, params->k, params->lambda,
                                                 params->omega, params->p, params->q);
        double product = 1.0;
        for (uint32_t i = 1; i <= params->d; i++) {
            uint64_t phi_i = get_dimensional_frequency(i);
            product *= math_cos(theta * (double)phi_i);
        }
        *product_out = product;
    }
    
    if (mobius_out) {
        *mobius_out = calculate_mobius_twist(params->k);
    }
    
    if (nu_out) {
        *nu_out = get_phonetic_value(params->lambda);
    }
    
    if (einstein_out) {
        *einstein_out = cllm_get_einstein_lambda();
    }
    
    if (psi_out) {
        if (params->p > 0 && params->q > 0) {
            PlimptonRatios psi = calculate_plimpton_ratios(params->p, params->q);
            *psi_out = psi.ratio_b_d;
        } else {
            *psi_out = 1.0;
        }
    }
    
    if (gamma_out) {
        *gamma_out = calculate_lattice_entropy(NULL, params->n, params->d);
    }
}