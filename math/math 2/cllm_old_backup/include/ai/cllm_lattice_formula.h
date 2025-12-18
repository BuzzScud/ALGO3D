/**
 * @file cllm_lattice_formula.h
 * @brief Complete unabridged L(n,d,k,λ,ω,ψ) lattice formula
 * 
 * This is the complete mathematical foundation of the crystalline CLLM system.
 * It integrates all components into a single unified formula.
 * 
 * Complete Formula:
 * L(n,d,k,λ,ω,ψ) = 3^O(n,k,λ) · ∏ᵢ₌₁ᵈ cos(θ·φᵢ) · Γ(k) · ν(λ) · Λ · Ψ(ψ) · Γ(n,d)
 * 
 * Where:
 * - O(n,k,λ): Octahedral symmetry function
 * - θ(n,k,λ,ω,ψ): Angular position with cymatic and Plimpton corrections
 * - φᵢ: Dimensional frequency for dimension i
 * - Γ(k): Möbius twist = (-1)^k
 * - ν(λ): Phonetic value mapping
 * - Λ: Einstein's Lambda correction (3/144000)
 * - Ψ(ψ): Plimpton 322 ratios
 * - Γ(n,d): Lattice entropy
 */

#ifndef CLLM_LATTICE_FORMULA_H
#define CLLM_LATTICE_FORMULA_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Complete lattice formula parameters
 */
typedef struct {
    uint64_t n;              /**< Prime index / position in lattice */
    uint32_t d;              /**< Dimension (1-12) */
    uint32_t k;              /**< Spiral index / symmetry group */
    const char* lambda;      /**< Phonetic layer identifier */
    double omega;            /**< Cymatic frequency (Hz) */
    uint64_t p;              /**< Plimpton generator p */
    uint64_t q;              /**< Plimpton generator q */
} LatticeFormulaParams;

/**
 * @brief Lattice formula computation context
 */
typedef struct {
    void* entropy_ctx;       /**< Entropy calculation context (optional) */
    bool use_cache;          /**< Enable caching for performance */
    bool verbose;            /**< Enable verbose output for debugging */
} LatticeFormulaContext;

/**
 * @brief Calculate complete unabridged L(n,d,k,λ,ω,ψ) formula
 * 
 * This is the main function that computes the complete lattice formula
 * integrating all mathematical components.
 * 
 * @param params Formula parameters
 * @param ctx Computation context (can be NULL)
 * @return Lattice value L(n,d,k,λ,ω,ψ)
 */
double L_lattice_complete(const LatticeFormulaParams *params, LatticeFormulaContext *ctx);

/**
 * @brief Calculate octahedral symmetry function O(n,k,λ)
 * 
 * Computes the octahedral symmetry contribution to the formula.
 * 
 * @param n Prime index
 * @param k Spiral index
 * @param lambda Phonetic layer
 * @return Octahedral symmetry value
 */
double calculate_O(uint64_t n, uint32_t k, const char* lambda);

/**
 * @brief Get dimensional frequency φᵢ for dimension i
 * 
 * Returns the characteristic frequency for a given dimension.
 * Based on the 12-fold symmetry structure.
 * 
 * @param dimension Dimension index (1-12)
 * @return Dimensional frequency
 */
uint64_t get_dimensional_frequency(uint32_t dimension);

/**
 * @brief Calculate Möbius twist Γ(k)
 * 
 * Simple alternating function: (-1)^k
 * 
 * @param k Spiral index
 * @return Möbius twist value (+1 or -1)
 */
double calculate_mobius_twist(uint32_t k);

/**
 * @brief Calculate complete angular position θ(n,k,λ,ω,ψ)
 * 
 * Integrates all corrections: base angular position, cymatic frequency,
 * and Plimpton triple corrections.
 * 
 * @param n Prime index
 * @param k Spiral index
 * @param lambda Phonetic layer
 * @param omega Cymatic frequency
 * @param p Plimpton generator p
 * @param q Plimpton generator q
 * @return Complete angular position
 */
double angular_position_complete(uint64_t n, uint32_t k, const char* lambda, 
                                 double omega, uint64_t p, uint64_t q);

/**
 * @brief Initialize lattice formula context
 * 
 * @param ctx Context to initialize
 * @param use_cache Enable caching
 * @param verbose Enable verbose output
 * @return true on success, false on failure
 */
bool lattice_formula_context_init(LatticeFormulaContext *ctx, bool use_cache, bool verbose);

/**
 * @brief Destroy lattice formula context
 * 
 * @param ctx Context to destroy
 */
void lattice_formula_context_destroy(LatticeFormulaContext *ctx);

/**
 * @brief Calculate simplified L_lattice for backward compatibility
 * 
 * Simplified version with default parameters for existing code.
 * 
 * @param n Prime index
 * @param d Dimension
 * @param k Spiral index
 * @return Lattice value
 */
double L_lattice_simple(uint64_t n, uint32_t d, uint32_t k);

/**
 * @brief Validate lattice formula parameters
 * 
 * Checks if parameters are within valid ranges.
 * 
 * @param params Parameters to validate
 * @return true if valid, false otherwise
 */
bool validate_lattice_params(const LatticeFormulaParams *params);

/**
 * @brief Get formula component breakdown for debugging
 * 
 * Computes each component of the formula separately for analysis.
 * 
 * @param params Formula parameters
 * @param O_out Output: O(n,k,λ) value
 * @param theta_out Output: θ value
 * @param product_out Output: Product term
 * @param mobius_out Output: Möbius twist
 * @param nu_out Output: Phonetic value
 * @param einstein_out Output: Einstein correction
 * @param psi_out Output: Plimpton ratio
 * @param gamma_out Output: Entropy value
 */
void L_lattice_breakdown(const LatticeFormulaParams *params,
                        double *O_out, double *theta_out, double *product_out,
                        double *mobius_out, double *nu_out, double *einstein_out,
                        double *psi_out, double *gamma_out);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_LATTICE_FORMULA_H */
