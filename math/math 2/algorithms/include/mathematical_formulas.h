/**
 * @file mathematical_formulas.h
 * @brief Collection of mathematical formulas from research prototype
 * 
 * This module contains 36 mathematical formulas extracted from the Python
 * research prototype. These formulas cover:
 * - Entropy and information theory
 * - Wave functions and harmonics
 * - Tetration and geometric operations
 * - Balance and quantum mechanics
 * - Harmonic resonance
 * - Text and linguistic analysis
 * - Advanced mathematical operations
 * 
 * All formulas use pure crystalline mathematics (no math.h).
 */

#ifndef MATHEMATICAL_FORMULAS_H
#define MATHEMATICAL_FORMULAS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Entropy & Information Theory (6 formulas)
 * ============================================================================ */

/**
 * @brief Log-based score (LBS)
 * 
 * Computes: LBS = -Σ p_i * log₂(p_i)
 * Similar to Shannon entropy but with specific application context.
 * 
 * @param p_i Probability distribution
 * @param len Length of distribution
 * @return LBS score
 */
double formula_lbs(const double* p_i, size_t len);

/**
 * @brief Matrix entropy
 * 
 * Computes: H = P^T × f × G
 * Matrix-based entropy calculation.
 * 
 * @param P Probability matrix (len x len)
 * @param f Frequency vector (len)
 * @param G Geometry matrix (len x len)
 * @param len Matrix dimension
 * @return Matrix entropy
 */
double formula_matrix_entropy(const double* P, const double* f, 
                               const double* G, size_t len);

/**
 * @brief Harmonic phase score (HPS)
 * 
 * Computes: HPS = P * log(P) * f
 * 
 * @param P Probability
 * @param f Frequency
 * @return Harmonic phase score
 */
double formula_hps(double P, double f);

/**
 * @brief Entropy approximation
 * 
 * Computes: E ≈ log(P) / T
 * 
 * @param P Probability
 * @param T Temperature/time parameter
 * @return Entropy approximation
 */
double formula_e_approx(double P, double T);

/**
 * @brief Lattice entropy score (LES)
 * 
 * Computes: LES = E_approx(P, T) * f
 * 
 * @param P Probability
 * @param T Temperature
 * @param f Frequency
 * @return Lattice entropy score
 */
double formula_les(double P, double T, double f);

/**
 * @brief Temporal lattice modulation (TLM)
 * 
 * Computes: TLM = HPS(P, f)
 * Alias for HPS with temporal context.
 * 
 * @param P Probability
 * @param f Frequency
 * @return Temporal modulation
 */
double formula_tlm(double P, double f);

/* ============================================================================
 * Wave Functions (6 formulas)
 * ============================================================================ */

/**
 * @brief Wave function Z
 * 
 * Computes: Z = sin(P1*π*x)*cos(P2*π*y) + sin(P3*π*x)*cos(P4*π*y)
 * 
 * @param P1 First prime parameter
 * @param P2 Second prime parameter
 * @param P3 Third prime parameter
 * @param P4 Fourth prime parameter
 * @param x X coordinate
 * @param y Y coordinate
 * @return Wave value
 */
double formula_wave_z(double P1, double P2, double P3, double P4, 
                      double x, double y);

/**
 * @brief 2D wave function ψ(m,n)
 * 
 * Computes: ψ = sin(m*π*x/L) * sin(n*π*y/W)
 * 
 * @param m Mode number in x
 * @param n Mode number in y
 * @param x X coordinate
 * @param y Y coordinate
 * @param L Length in x
 * @param W Width in y
 * @return Wave amplitude
 */
double formula_psi_mn(uint32_t m, uint32_t n, double x, double y, 
                      double L, double W);

/**
 * @brief Prime sine modulation (PSM)
 * 
 * Computes: PSM = P * sin(P*π*x)
 * 
 * @param P Prime parameter
 * @param x Position
 * @return Modulated value
 */
double formula_psm(double P, double x);

/**
 * @brief Eleventh harmonic generator
 * 
 * Computes: H11 = sin(11*π*x) + dissonant
 * 
 * @param x Position
 * @param dissonant Dissonance factor
 * @return 11th harmonic value
 */
double formula_eleventh_hg(double x, double dissonant);

/**
 * @brief Harmonic division (HD)
 * 
 * Computes: HD = harmonic / P
 * 
 * @param harmonic Harmonic number
 * @param P Prime divisor
 * @return Divided harmonic
 */
double formula_hd(double harmonic, double P);

/**
 * @brief Dimensional phase shift (DPS)
 * 
 * Computes: DPS = sin(P*π*x/L) * sin(n*π*y/W)
 * 
 * @param P Prime parameter
 * @param n Mode number
 * @param x X coordinate
 * @param y Y coordinate
 * @param L Length
 * @param W Width
 * @return Phase-shifted value
 */
double formula_dps(double P, uint32_t n, double x, double y, 
                   double L, double W);

/* ============================================================================
 * Tetration & Geometry (7 formulas)
 * ============================================================================ */

/**
 * @brief Base geometric alignment (BGA)
 * 
 * Computes: BGA = φ * tetration(2, T)
 * 
 * @param T Tetration height
 * @param p_mod Modulus for tetration
 * @return Geometric alignment value
 */
double formula_bga(uint32_t T, uint64_t p_mod);

/**
 * @brief Tetration value (TV)
 * 
 * Computes: TV = tetration(P, T) mod p_mod
 * 
 * @param P Base prime
 * @param T Height
 * @param p_mod Modulus
 * @return Tetration value
 */
uint64_t formula_tv(uint64_t P, uint32_t T, uint64_t p_mod);

/**
 * @brief Tetration value geometric (TVG)
 * 
 * Alias for TV with geometric context.
 * 
 * @param P Base prime
 * @param T Height
 * @param p_mod Modulus
 * @return Tetration value
 */
uint64_t formula_tvg(uint64_t P, uint32_t T, uint64_t p_mod);

/**
 * @brief Tetration value with pi (TV_π)
 * 
 * Computes: TV_π = tetration(P, T) * π
 * 
 * @param P Base prime
 * @param T Height
 * @param p_mod Modulus
 * @return Tetration value scaled by π
 */
double formula_tv_pi(uint64_t P, uint32_t T, uint64_t p_mod);

/**
 * @brief Recursive information folding (RIF)
 * 
 * Computes: RIF = tetration(P, T) * SE
 * 
 * @param P Base prime
 * @param T Height
 * @param SE Shannon entropy
 * @param p_mod Modulus
 * @return Folded information value
 */
double formula_rif(uint64_t P, uint32_t T, double SE, uint64_t p_mod);

/**
 * @brief Information vector geometry (IVG)
 * 
 * Computes: IVG = Π(P_i^T_i) * fractal
 * 
 * @param P_i Array of primes
 * @param T_i Array of heights
 * @param len Array length
 * @param fractal Fractal scaling factor
 * @return Geometric information value
 */
double formula_ivg(const uint64_t* P_i, const uint32_t* T_i, 
                   size_t len, double fractal);

/**
 * @brief Tetration log decay (TLD)
 * 
 * Computes: TLD = log(P) / T
 * 
 * @param P Prime
 * @param T Time/height parameter
 * @return Decay value
 */
double formula_tld(uint64_t P, uint32_t T);

/* ============================================================================
 * Balance & Quantum (6 formulas)
 * ============================================================================ */

/**
 * @brief Balance function BN1
 * 
 * Computes: BN1 = (O1 + O2) / (1 + |O1 - O2| / P)
 * 
 * @param O1 First observable
 * @param O2 Second observable
 * @param P Prime normalizer
 * @return Balanced value
 */
double formula_balance_bn1(double O1, double O2, double P);

/**
 * @brief Angular velocity difference (AVD)
 * 
 * Alias for balance_bn1 with angular context.
 * 
 * @param A1 First angle
 * @param A2 Second angle
 * @param P Prime normalizer
 * @return Angular difference
 */
double formula_avd(double A1, double A2, double P);

/**
 * @brief Normalized difference calculation (NDC)
 * 
 * Alias for balance_bn1 with normalization context.
 * 
 * @param O1 First value
 * @param O2 Second value
 * @param P Prime normalizer
 * @return Normalized difference
 */
double formula_ndc(double O1, double O2, double P);

/**
 * @brief Quantum state score (QSS)
 * 
 * Computes: QSS = H + C / √P
 * 
 * @param H Hamiltonian
 * @param C Constant
 * @param P Prime parameter
 * @return Quantum score
 */
double formula_qss(double H, double C, double P);

/**
 * @brief Prime resonance energy (PRE)
 * 
 * Computes: PRE = φ^n * P
 * 
 * @param n Exponent
 * @param P Prime
 * @return Resonance energy
 */
double formula_pre(uint32_t n, double P);

/**
 * @brief Gravitational number resonance (GNR)
 * 
 * Computes: GNR = P * √G
 * 
 * @param P Prime
 * @param G_val Gravitational constant (default: 6.67430e-11)
 * @return Gravitational resonance
 */
double formula_gnr(double P, double G_val);

/* ============================================================================
 * Harmonic & Resonance (5 formulas)
 * ============================================================================ */

/**
 * @brief Sacred time modulation (STM)
 * 
 * Computes: STM = (396 + 21*k) mod P
 * 
 * @param k Harmonic index
 * @param P Prime modulus
 * @return Time modulation value
 */
uint64_t formula_stm(uint32_t k, uint64_t P);

/**
 * @brief Universal harmonic hash (UHH)
 * 
 * Computes: UHH = (3 + 6 + 9) mod P = 18 mod P
 * 
 * @param P Prime modulus
 * @return Harmonic hash
 */
uint64_t formula_uhh(uint64_t P);

/**
 * @brief Prime gap harmonic (PGH)
 * 
 * Computes: PGH = Σ (p_{i+1} - p_i) / log(p_i)
 * 
 * @param primes Array of primes
 * @param len Number of primes
 * @return Gap harmonic sum
 */
double formula_pgh(const uint64_t* primes, size_t len);

/**
 * @brief Fractal harmonic series (FHS)
 * 
 * Computes: FHS = Σ_{i=1}^k 1 / (i * log(P))
 * 
 * @param k Series length
 * @param P Prime parameter
 * @return Harmonic series sum
 */
double formula_fhs(uint32_t k, uint64_t P);

/**
 * @brief Harmonic score
 * 
 * Computes: score = H + (5 - cycle_unique) * 2
 * 
 * @param H Base harmonic value
 * @param cycle_unique Number of unique cycles
 * @return Harmonic score
 */
double formula_harm_score(double H, uint32_t cycle_unique);

/* ============================================================================
 * Text & Linguistics (5 formulas)
 * ============================================================================ */

/**
 * @brief Glyph strokes sum
 * 
 * Computes: sum(strokes) mod B
 * 
 * @param strokes Array of stroke counts
 * @param len Number of glyphs
 * @param B Base modulus
 * @return Stroke sum modulo B
 */
uint64_t formula_glyph_strokes(const uint32_t* strokes, size_t len, uint64_t B);

/**
 * @brief TF-IDF (Term Frequency - Inverse Document Frequency)
 * 
 * Computes: TF-IDF = tf * log(N / df)
 * 
 * @param tf Term frequency
 * @param N Total documents
 * @param df Document frequency
 * @return TF-IDF score
 */
double formula_tfidf(double tf, uint32_t N, uint32_t df);

/**
 * @brief Word geometry (WG)
 * 
 * Looks up geometry value from table.
 * 
 * @param R Row index
 * @param S Column index
 * @param table Geometry table (R x S)
 * @param table_size Total table size
 * @return Geometry value
 */
double formula_wg(uint32_t R, uint32_t S, const double* table, size_t table_size);

/**
 * @brief Transition probability
 * 
 * Looks up transition probability from table.
 * 
 * @param g_prev Previous glyph
 * @param g_n Current glyph
 * @param T Transition table
 * @param T_size Table size
 * @return Transition probability
 */
double formula_trans_prob(uint32_t g_prev, uint32_t g_n, 
                          const double* T, size_t T_size);

/**
 * @brief Edit distance
 * 
 * Computes minimum edit operations.
 * 
 * @param ops Operation costs
 * @param C Source sequence
 * @param L Target sequence
 * @param len Sequence length
 * @return Minimum edit distance
 */
double formula_edit_dist(const uint32_t* ops, const uint32_t* C, 
                         const uint32_t* L, size_t len);

/* ============================================================================
 * Advanced (3 formulas)
 * ============================================================================ */

/**
 * @brief Entropy-adjusted alignment (EAA)
 * 
 * Computes: EAA = sum(H_P) / constraint
 * 
 * @param H_P Entropy array
 * @param len Array length
 * @param constraint Constraint value
 * @return Adjusted alignment
 */
double formula_eaa(const double* H_P, size_t len, double constraint);

/**
 * @brief Quantum resonance unit (QRU)
 * 
 * Computes: QRU = sum(ψ_P) * dx * f
 * 
 * @param psi_P Wave function array
 * @param len Array length
 * @param dx Differential element
 * @param f Frequency
 * @return Resonance unit
 */
double formula_qru(const double* psi_P, size_t len, double dx, double f);

/**
 * @brief Cumulative distribution (C_D)
 * 
 * Computes: C_D(r, d) = 1 - exp(-r^d / Γ(d/2 + 1))
 * 
 * @param r Radius
 * @param d Dimension
 * @return Cumulative probability
 */
double formula_c_d(double r, uint32_t d);

#ifdef __cplusplus
}
#endif

#endif /* MATHEMATICAL_FORMULAS_H */
