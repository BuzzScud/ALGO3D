/**
 * @file mathematical_formulas.c
 * @brief Implementation of 36 mathematical formulas from research prototype
 * 
 * PHASE 1 WEEK 2: Migrated to NEW math library
 * - Replaced prime_log2 with math_log2 (1 call)
 * - Replaced math_log with math_log (7 calls)
 * - Replaced math_sin with math_sin (8 calls)
 * - Replaced math_cos with math_cos (2 calls)
 * - Replaced math_sqrt with math_sqrt (5 calls)
 * - Replaced math_pow with math_pow (4 calls)
 * - Replaced math_exp with math_exp (1 call)
 * Total: 28 function calls migrated to NEW math library
 */

#include "mathematical_formulas.h"
#include "math/constants.h"
#include "math/types.h"
#include "math/constants.h"
#include "math/transcendental.h"  // PHASE 1: NEW math library
#include "math/constants.h"
#include "math/arithmetic.h"       // PHASE 1: NEW math library
#include "math/constants.h"
#include <stdlib.h>
#include <stdint.h>

/* ============================================================================
 * Entropy & Information Theory (6 formulas)
 * ============================================================================ */

double formula_lbs(const double* p_i, size_t len) {
    if (!p_i || len == 0) return 0.0;
    
    double lbs = 0.0;
    for (size_t i = 0; i < len; i++) {
        if (p_i[i] > 0.0) {
            lbs -= p_i[i] * math_log2(p_i[i]);
        }
    }
    
    return lbs;
}

double formula_matrix_entropy(const double* P, const double* f, 
                               const double* G, size_t len) {
    if (!P || !f || !G || len == 0) return 0.0;
    
    // H = P^T × f × G
    // First: temp = P^T × f (vector)
    double* temp = (double*)malloc(len * sizeof(double));
    if (!temp) return 0.0;
    
    for (size_t i = 0; i < len; i++) {
        temp[i] = 0.0;
        for (size_t j = 0; j < len; j++) {
            temp[i] += P[j * len + i] * f[j];  // P^T
        }
    }
    
    // Second: result = temp × G (scalar)
    double result = 0.0;
    for (size_t i = 0; i < len; i++) {
        for (size_t j = 0; j < len; j++) {
            result += temp[i] * G[i * len + j];
        }
    }
    
    free(temp);
    return result;
}

double formula_hps(double P, double f) {
    if (P <= 0.0) return 0.0;
    return P * math_log(P) * f;
}

double formula_e_approx(double P, double T) {
    if (T == 0.0 || P <= 0.0) return 0.0;
    return math_log(P) / T;
}

double formula_les(double P, double T, double f) {
    return formula_e_approx(P, T) * f;
}

double formula_tlm(double P, double f) {
    return formula_hps(P, f);
}

/* ============================================================================
 * Wave Functions (6 formulas)
 * ============================================================================ */

double formula_wave_z(double P1, double P2, double P3, double P4, 
                      double x, double y) {
    double term1 = math_sin(P1 * MATH_PI * x) * math_cos(P2 * MATH_PI * y);
    double term2 = math_sin(P3 * MATH_PI * x) * math_cos(P4 * MATH_PI * y);
    return term1 + term2;
}

double formula_psi_mn(uint32_t m, uint32_t n, double x, double y, 
                      double L, double W) {
    if (L == 0.0 || W == 0.0) return 0.0;
    
    double term1 = math_sin((double)m * MATH_PI * x / L);
    double term2 = math_sin((double)n * MATH_PI * y / W);
    return term1 * term2;
}

double formula_psm(double P, double x) {
    return P * math_sin(P * MATH_PI * x);
}

double formula_eleventh_hg(double x, double dissonant) {
    return math_sin(11.0 * MATH_PI * x) + dissonant;
}

double formula_hd(double harmonic, double P) {
    if (P == 0.0) return 0.0;
    return harmonic / P;
}

double formula_dps(double P, uint32_t n, double x, double y, 
                   double L, double W) {
    if (L == 0.0 || W == 0.0) return 0.0;
    
    double term1 = math_sin(P * MATH_PI * x / L);
    double term2 = math_sin((double)n * MATH_PI * y / W);
    return term1 * term2;
}

/* ============================================================================
 * Tetration & Geometry (7 formulas)
 * ============================================================================ */

double formula_bga(uint32_t T, uint64_t p_mod) {
    uint64_t tet = formula_tv(2, T, p_mod);
    return MATH_PHI * (double)tet;
}

uint64_t formula_tv(uint64_t P, uint32_t T, uint64_t p_mod) {
    if (T == 0) return 1;
    if (T == 1) return P % p_mod;
    
    // Tetration with modular arithmetic: P^P^P^... (T times) mod p_mod
    // Use iterative approach with golden ratio damping for stability
    uint64_t result = P % p_mod;
    
    for (uint32_t i = 1; i < T; i++) {
        // Apply golden ratio damping to prevent overflow
        // Damping factor: φ^(-i) where φ = 1.618...
        double damping = 1.0;
        for (uint32_t j = 0; j < i; j++) {
            damping /= MATH_PHI;
        }
        
        // Compute P^result mod p_mod with damping
        uint64_t damped_exp = (uint64_t)(result * damping);
        if (damped_exp == 0) damped_exp = 1;
        
        // Modular exponentiation: P^damped_exp mod p_mod
        uint64_t base = P % p_mod;
        uint64_t exp = damped_exp;
        result = 1;
        
        while (exp > 0) {
            if (exp & 1) {
                result = (result * base) % p_mod;
            }
            base = (base * base) % p_mod;
            exp >>= 1;
        }
    }
    
    return result;
}

uint64_t formula_tvg(uint64_t P, uint32_t T, uint64_t p_mod) {
    return formula_tv(P, T, p_mod);
}

double formula_tv_pi(uint64_t P, uint32_t T, uint64_t p_mod) {
    uint64_t tv = formula_tv(P, T, p_mod);
    return (double)tv * MATH_PI;
}

double formula_rif(uint64_t P, uint32_t T, double SE, uint64_t p_mod) {
    uint64_t tv = formula_tv(P, T, p_mod);
    return (double)tv * SE;
}

double formula_ivg(const uint64_t* P_i, const uint32_t* T_i, 
                   size_t len, double fractal) {
    if (!P_i || !T_i || len == 0) return 0.0;
    
    double product = 1.0;
    
    for (size_t i = 0; i < len; i++) {
        double term = math_pow((double)P_i[i], (double)T_i[i]);
        product *= term;
    }
    
    return product * fractal;
}

double formula_tld(uint64_t P, uint32_t T) {
    if (T == 0 || P == 0) return 0.0;
    return math_log((double)P) / (double)T;
}

/* ============================================================================
 * Balance & Quantum (6 formulas)
 * ============================================================================ */

double formula_balance_bn1(double O1, double O2, double P) {
    if (P == 0.0) return 0.0;
    
    double numerator = O1 + O2;
    double diff = O1 - O2;
    if (diff < 0.0) diff = -diff;
    double denominator = 1.0 + diff / P;
    
    return numerator / denominator;
}

double formula_avd(double A1, double A2, double P) {
    return formula_balance_bn1(A1, A2, P);
}

double formula_ndc(double O1, double O2, double P) {
    return formula_balance_bn1(O1, O2, P);
}

double formula_qss(double H, double C, double P) {
    if (P <= 0.0) return H;
    return H + C / math_sqrt(P);
}

double formula_pre(uint32_t n, double P) {
    return math_pow(MATH_PHI, (double)n) * P;
}

double formula_gnr(double P, double G_val) {
    return P * math_sqrt(G_val);
}

/* ============================================================================
 * Harmonic & Resonance (5 formulas)
 * ============================================================================ */

uint64_t formula_stm(uint32_t k, uint64_t P) {
    uint64_t value = 396 + 21 * k;
    return value % P;
}

uint64_t formula_uhh(uint64_t P) {
    return 18 % P;  // 3 + 6 + 9 = 18
}

double formula_pgh(const uint64_t* primes, size_t len) {
    if (!primes || len < 2) return 0.0;
    
    double sum = 0.0;
    
    for (size_t i = 0; i < len - 1; i++) {
        double gap = (double)(primes[i+1] - primes[i]);
        double log_p = math_log((double)primes[i]);
        
        if (log_p > 0.0) {
            sum += gap / log_p;
        }
    }
    
    return sum;
}

double formula_fhs(uint32_t k, uint64_t P) {
    if (P == 0 || k == 0) return 0.0;
    
    double log_P = math_log((double)P);
    if (log_P == 0.0) return 0.0;
    
    double sum = 0.0;
    for (uint32_t i = 1; i <= k; i++) {
        sum += 1.0 / ((double)i * log_P);
    }
    
    return sum;
}

double formula_harm_score(double H, uint32_t cycle_unique) {
    if (cycle_unique > 5) cycle_unique = 5;
    return H + (double)(5 - cycle_unique) * 2.0;
}

/* ============================================================================
 * Text & Linguistics (5 formulas)
 * ============================================================================ */

uint64_t formula_glyph_strokes(const uint32_t* strokes, size_t len, uint64_t B) {
    if (!strokes || len == 0 || B == 0) return 0;
    
    uint64_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += strokes[i];
    }
    
    return sum % B;
}

double formula_tfidf(double tf, uint32_t N, uint32_t df) {
    if (df == 0 || N == 0) return 0.0;
    return tf * math_log((double)N / (double)df);
}

double formula_wg(uint32_t R, uint32_t S, const double* table, size_t table_size) {
    if (!table || table_size == 0) return 0.0;
    
    // Assume table is row-major, need to know number of columns
    // For simplicity, assume square table
    size_t cols = (size_t)math_sqrt((double)table_size);
    if (cols == 0) return 0.0;
    
    size_t index = (size_t)R * cols + (size_t)S;
    if (index >= table_size) return 0.0;
    
    return table[index];
}

double formula_trans_prob(uint32_t g_prev, uint32_t g_n, 
                          const double* T, size_t T_size) {
    if (!T || T_size == 0) return 0.0;
    
    // Assume T is transition matrix, need dimensions
    // For simplicity, assume square matrix
    size_t dim = (size_t)math_sqrt((double)T_size);
    if (dim == 0) return 0.0;
    
    size_t index = (size_t)g_prev * dim + (size_t)g_n;
    if (index >= T_size) return 0.0;
    
    return T[index];
}

double formula_edit_dist(const uint32_t* ops, const uint32_t* C, 
                         const uint32_t* L, size_t len) {
    if (!ops || !C || !L || len == 0) return 0.0;
    
    // Simple edit distance: sum of operation costs
    double min_cost = 0.0;
    
    for (size_t i = 0; i < len; i++) {
        // ops[C[i], L[i]] - simplified as ops[i]
        min_cost += (double)ops[i];
    }
    
    return min_cost;
}

/* ============================================================================
 * Advanced (3 formulas)
 * ============================================================================ */

double formula_eaa(const double* H_P, size_t len, double constraint) {
    if (!H_P || len == 0 || constraint == 0.0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < len; i++) {
        sum += H_P[i];
    }
    
    return sum / constraint;
}

double formula_qru(const double* psi_P, size_t len, double dx, double f) {
    if (!psi_P || len == 0) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < len; i++) {
        sum += psi_P[i];
    }
    
    return sum * dx * f;
}

double formula_c_d(double r, uint32_t d) {
    if (d == 0) return 0.0;
    
    // C_D(r, d) = 1 - exp(-r^d / Γ(d/2 + 1))
    double r_d = math_pow(r, (double)d);
    
    // Approximate Γ(d/2 + 1) using Stirling's approximation
    // For small d, use direct calculation
    double gamma_val;
    double x = (double)d / 2.0 + 1.0;
    
    if (x <= 1.0) {
        gamma_val = 1.0;
    } else if (x <= 2.0) {
        gamma_val = 1.0;  // Γ(2) = 1
    } else {
        // Stirling: Γ(x) ≈ √(2π/x) * (x/e)^x
        gamma_val = math_sqrt(MATH_TWO_PI / x) * math_pow(x / MATH_E, x);
    }
    
    double exponent = -r_d / gamma_val;
    return 1.0 - math_exp(exponent);
}
