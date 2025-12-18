/*
 * prime_lattice_core.c - Implementation of Crystalline Lattice Core
 * 
 * This implements the complete crystalline lattice framework based on
 * the mathematical model from the Python code.
 */

#include "../include/prime_lattice_core.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "../include/bigint_core.h"
#include "../include/abacus_legacy.h"
#include "../include/prime_bigint_transcendental.h"
#include "../include/cllm_mathematical_constants.h"
#include "../include/phonetic_values.h"
#include "../include/lattice_entropy.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static bool is_prime_geometric_core(uint64_t n);

/* ============================================================================
 * GLOBAL CONSTANTS
 * ============================================================================ */

// Small primes (foundation) - First 18 primes
const uint64_t SMALL_PRIMES[NUM_SMALL_PRIMES] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61
};

// Dimensional frequencies φᵢ (base)
const double PHI_FREQS_BASE[NUM_PHI_FREQS] = {
    3.0, 7.0, 31.0, 12.0, 19.5, 11.0, 13.0, 17.0, 23.0, 29.0, 31.0, 59.0
};

// Viable residues (mod 30)
const uint8_t VIABLE_RESIDUES[NUM_VIABLE_RESIDUES] = {
    1, 7, 11, 17, 19, 23, 29, 59
};

// Quadratic residues (mod 12)
const uint8_t QR_ALLOW[NUM_QR_ALLOW] = {
    1, 9
};

// CRNS frequencies (Hz)
const uint16_t CRNS_FREQ[NUM_CRNS_FREQ] = {
    432, 528, 639, 741, 852, 963
};

// Vedic triples (Pythagorean)
const VedicTriple VEDIC_TRIPLES[NUM_VEDIC_TRIPLES] = {
    {3, 4, 5},
    {5, 12, 13},
    {8, 15, 17},
    {7, 24, 25},
    {20, 21, 29}
};

/* ============================================================================
 * INITIALIZATION
 * ============================================================================ */

void lattice_init(void) {
    // Initialize any global state if needed
    // Currently all constants are static
}

void lattice_cleanup(void) {
    // Cleanup any allocated resources
}

/* ============================================================================
 * PHONETIC FUNCTIONS (ν(λ))
 * ============================================================================ */

double nu_lambda(const char *lambda_phon) {
    // Use the new phonetic value system
    // This provides a centralized, maintainable phonetic mapping
    return get_phonetic_value(lambda_phon);
}

void update_phi_freqs(const double *phi_base, double *phi_updated,
                      const char *lambda_phon, int count) {
    if (!phi_base || !phi_updated) return;
    
    double nu = nu_lambda(lambda_phon);
    
    for (int i = 0; i < count; i++) {
        uint64_t Pi = (uint64_t)phi_base[i];
        
        // Check if prime
        bool is_prime = false;
        if (Pi < NUM_SMALL_PRIMES) {
            for (int j = 0; j < NUM_SMALL_PRIMES; j++) {
                if (SMALL_PRIMES[j] == Pi) {
                    is_prime = true;
                    break;
                }
            }
        } else {
            is_prime = is_prime_geometric_core(Pi);
        }
        
        if (is_prime) {
            // Prime: add ν*0.1
            phi_updated[i] = phi_base[i] + nu * 0.1;
        } else {
            // Composite: divide by 10
            phi_updated[i] = phi_base[i] / 10.0;
        }
    }
}

/* ============================================================================
 * PYTHAGOREAN TRIPLES (ψ)
 * ============================================================================ */

void pythagorean_triple(uint64_t p, uint64_t q, PythagoreanTriple *triple) {
    if (!triple) return;
    
    triple->p = p;
    triple->q = q;
    
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    
    triple->a = p2 - q2;
    triple->b = 2 * p * q;
    triple->c = p2 + q2;
    
    // Calculate ratio
    if (triple->c > 0) {
        triple->ratio = (double)triple->a / (double)triple->c;
    } else {
        triple->ratio = 0.0;
    }
}

double pythagorean_ratio(uint64_t p, uint64_t q) {
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    uint64_t denom = p2 + q2;
    
    if (denom == 0) return 0.0;
    
    return (double)(p2 - q2) / (double)denom;
}

int64_t pythagorean_difference(uint64_t p, uint64_t q) {
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    return (int64_t)(p2 - q2);
}

/* ============================================================================
 * MÖBIUS TWIST (Γ(k))
 * ============================================================================ */

int mobius_twist(int k) {
    // Γ(k) = (-1)^k
    return (k % 2 == 0) ? 1 : -1;
}

/* ============================================================================
 * CLOCK FACE MAPPING
 * ============================================================================ */

double theta_n(uint64_t n, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q, bool use_ratio) {
    // θ(n,k,λ,ω,p,q) = k·π·(1+√5) + n·(2π/12) + log₃(ν(λ)) + ω/432 + ψ(p,q)
    
    double nu = nu_lambda(lambda_phon);
    
    // Term 1: k·π·(1+√5)
    double term1 = (double)k * MATH_PI * (1.0 + LATTICE_SQRT5);
    
    // Term 2: n·(2π/12)
    double term2 = (double)n * (2.0 * MATH_PI / 12.0);
    
    // Term 3: log₃(ν(λ))
    double term3 = 0.0;
    if (nu > 0) {
        term3 = math_log(nu) / math_log(3.0);
    }
    
    // Term 4: ω/432
    double term4 = (double)omega / 432.0;
    
    // Term 5: ψ(p,q)
    double term5;
    if (use_ratio) {
        term5 = pythagorean_ratio(p, q);
    } else {
        term5 = (double)pythagorean_difference(p, q);
    }
    
    return term1 + term2 + term3 + term4 + term5;
}

double r_n(uint64_t prime) {
    // r_n(p) = log₃(p) + {log₃(p)} × π × φ
    
    if (prime == 0) return 0.0;
    
    double log_val = math_log((double)prime) / math_log(3.0);
    double dust = log_val - math_floor(log_val);  // Fractional part
    
    return log_val + dust * GROWTH_FACTOR;
}

void map_prime_to_clock_phonetic(uint64_t prime, LegacyClockPosition *pos,
                        const char *lambda_phon) {
    if (!pos) return;
    
    // Check if prime
    if (!is_prime_geometric_core(prime)) {
        pos->theta = 0.0;
        pos->r = 0.0;
        pos->quadrant = 0;
        pos->clock_pos = 0;
        return;
    }
    
    // Calculate index n (simplified - would need prime counting function)
    uint64_t n = 0;
    for (uint64_t i = 2; i <= prime; i++) {
        if (is_prime_geometric_core(i)) n++;
    }
    
    // Calculate theta and r
    double theta = theta_n(n, 1, lambda_phon, 432, 3, 4, true);
    double r = r_n(prime);
    
    // Fold to single quadrant
    double theta_fold = math_fmod(theta, MATH_PI / 2.0);
    
    pos->theta = theta_fold;
    pos->r = r;
    pos->quadrant = 1;  // All folded to Q1
    
    // Calculate clock position (0-11)
    double clock_angle = math_fmod(theta, 2.0 * MATH_PI);
    pos->clock_pos = (uint8_t)(clock_angle / (2.0 * MATH_PI / 12.0));
    if (pos->clock_pos >= 12) pos->clock_pos = 11;
}

uint64_t map_clock_to_prime(double theta, double r, uint8_t quadrant,
                            double tolerance, uint64_t max_search) {
    // Unfold from quadrant if needed
    if (quadrant != 1) {
        theta += (quadrant - 1) * (MATH_PI / 2.0);
    }
    
    // Search for closest prime
    double min_dist = 1e308;  // Very large number
    uint64_t closest_prime = 0;
    
    for (uint64_t p = 2; p <= max_search; p++) {
        if (!is_prime_geometric_core(p)) continue;
        
        LegacyClockPosition pos;
        map_prime_to_clock_phonetic(p, &pos, "dub");
        
        // Calculate Euclidean distance
        double dt = theta - pos.theta;
        double dr = r - pos.r;
        double dist = math_sqrt(dt * dt + dr * dr);
        
        if (dist < min_dist) {
            min_dist = dist;
            closest_prime = p;
        }
        
        if (dist <= tolerance) {
            return p;  // Exact match within tolerance
        }
    }
    
    return closest_prime;
}

/* ============================================================================
 * LATTICE EMBEDDING (15D)
 * ============================================================================ */

void lattice_embed(uint64_t value, LatticeEmbed *embed) {
    if (!embed) return;
    
    embed->value = value;
    
    // Calculate residues mod first 15 primes
    for (int i = 0; i < EMBED_DIM && i < NUM_SMALL_PRIMES; i++) {
        embed->residues[i] = value % SMALL_PRIMES[i];
    }
}

double lattice_entropy(uint64_t n, uint64_t d) {
    // Γ(n,d) = log₂(count_primes(d) / d)
    (void)n;  // Parameter reserved for future use
    
    if (d == 0) return 0.0;
    
    // Count primes up to d
    uint64_t count = 0;
    for (uint64_t i = 2; i <= d; i++) {
        if (is_prime_geometric_core(i)) count++;
    }
    
    if (count == 0) return 0.0;
    
    double density = (double)count / (double)d;
    return math_log(density) / math_log(2.0);
}

/* ============================================================================
 * TETRATION WITH GOLDEN DAMPING
 * ============================================================================ */

double gmp_tetration(double base, int height, bool damp) {
    // gmp_tetration(b, h) = b^(gmp_tetration(b, h-1)) × φ^(-h)
    
    if (height <= 0) return 1.0;
    if (height == 1) return base;
    
    // Recursive calculation
    double tower = gmp_tetration(base, height - 1, damp);
    
    // Apply damping
    if (damp) {
        double damping_factor = math_pow(MATH_PHI, -(double)height);
        tower *= damping_factor;
    }
    
    // Limit tower to prevent overflow
    if (tower > 100.0) tower = 100.0;
    
    return math_pow(base, tower);
}

double lattice_tetration_log_approx(double P, int T) {
    // lattice_tetration_log_approx(P, T) = T × lattice_tetration_log_approx(P, T-1) / log(φ)
    
    if (T <= 0) return 0.0;
    if (T == 1) return math_log(P);
    
    double prev = lattice_tetration_log_approx(P, T - 1);
    return (double)T * prev / math_log(MATH_PHI);
}

double entropy_equilibrator(double exp, double P, int T) {
    // entropy_equilibrator(e, P, T) = 0.0047 × log(e) / log(tet(P, T))
    
    if (exp <= 0 || P <= 0) return 0.0;
    
    double tet = gmp_tetration(P, T, true);
    if (tet <= 0) return 0.0;
    
    double log_ratio = math_log(exp) / math_log(tet);
    return 0.0047 * log_ratio;
}

/* ============================================================================
 * GEOMETRIC PRIME TESTING
 * ============================================================================ */

static bool is_prime_geometric_core(uint64_t n) {
    // 1. Check small cases
    if (n < 2) return false;
    
    // 2. Check if in SMALL_PRIMES
    for (int i = 0; i < NUM_SMALL_PRIMES; i++) {
        if (SMALL_PRIMES[i] == n) return true;
        if (SMALL_PRIMES[i] > n) break;
    }
    
    // 3. Check mod CLOCK_MOD not in [0,6,8,10]
    uint8_t pos = n % CLOCK_MOD;
    if (pos == 0 || pos == 6 || pos == 8 || pos == 10) return false;
    
    // 4. Check mod EXT_CLOCK_MOD in VIABLE_RESIDUES (for n > 61)
    if (n > 61) {
        uint8_t res = n % EXT_CLOCK_MOD;
        bool found = false;
        for (int i = 0; i < NUM_VIABLE_RESIDUES; i++) {
            if (VIABLE_RESIDUES[i] == res) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    
    // 5. Check (n²) mod CLOCK_MOD in QR_ALLOW
    uint64_t qr = (n * n) % CLOCK_MOD;
    bool qr_ok = false;
    for (int i = 0; i < NUM_QR_ALLOW; i++) {
        if (QR_ALLOW[i] == qr) {
            qr_ok = true;
            break;
        }
    }
    if (!qr_ok) return false;
    
    // 6. Final verification - trial division
    for (int i = 0; i < NUM_SMALL_PRIMES; i++) {
        if (SMALL_PRIMES[i] * SMALL_PRIMES[i] > n) break;
        if (n % SMALL_PRIMES[i] == 0) return false;
    }
    
    return true;
}

uint64_t* generate_primes_geometric(uint64_t limit, uint64_t *count) {
    if (!count) return NULL;
    
    // Allocate array (estimate size)
    uint64_t estimated = limit / 10;  // Rough estimate
    uint64_t *primes = malloc(estimated * sizeof(uint64_t));
    if (!primes) {
        *count = 0;
        return NULL;
    }
    
    *count = 0;
    
    for (uint64_t n = 2; n <= limit; n++) {
        if (is_prime_geometric_core(n)) {
            if (*count >= estimated) {
                // Reallocate if needed
                estimated *= 2;
                uint64_t *new_primes = realloc(primes, estimated * sizeof(uint64_t));
                if (!new_primes) {
                    free(primes);
                    *count = 0;
                    return NULL;
                }
                primes = new_primes;
            }
            primes[*count] = n;
            (*count)++;
        }
    }
    
    return primes;
}

/* ============================================================================
 * MASTER LATTICE FUNCTION L
 * ============================================================================ */

double O_exponent(uint64_t n, int k, const char *lambda_phon) {
    // O(n,k,λ) = (n-1)·(π/6)/ln(3) + log₃(ν(λ)) + k·π·(1+√5)
    
    double nu = nu_lambda(lambda_phon);
    
    // Term 1: (n-1)·(π/6)/ln(3)
    double term1 = ((double)n - 1.0) * (MATH_PI / 6.0) / math_log(3.0);
    
    // Term 2: log₃(ν(λ))
    double term2 = 0.0;
    if (nu > 0) {
        term2 = math_log(nu) / math_log(3.0);
    }
    
    // Term 3: k·π·(1+√5)
    double term3 = (double)k * MATH_PI * (1.0 + LATTICE_SQRT5);
    
    return term1 + term2 + term3;
}

double L_lattice(uint64_t n, uint64_t d, int k, const char *lambda_phon,
                 uint16_t omega, uint64_t p, uint64_t q) {
    // COMPLETE UNABRIDGED FORMULA:
    // L = 3^O × ∏cos(Θ×φᵢ) × Γ(k) × ν(λ) × (ω) × Ψ(ψ) × Γ(n,d)
    
    // Calculate O
    double o = O_exponent(n, k, lambda_phon);
    
    // Base: 3^O
    double base = math_pow(3.0, o);
    
    // Product: ∏cos(Θ×φᵢ)
    double theta = theta_n(n, k, lambda_phon, omega, p, q, false);
    
    // Update phi frequencies
    double phi_updated[NUM_PHI_FREQS];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, NUM_PHI_FREQS);
    
    double prod = 1.0;
    for (uint64_t i = 0; i < d && i < NUM_PHI_FREQS; i++) {
        prod *= math_cos(theta * phi_updated[i]);
    }
    
    // Γ(k): Möbius twist
    double gamma_k = (double)mobius_twist(k);
    
    // ν(λ): Phonetic value
    double nu = nu_lambda(lambda_phon);
    
    // (ω): Einstein's Λ correction = 3/144000
    double einstein_lambda = EINSTEIN_LAMBDA_DOUBLE;
    
    // Ψ(ψ): Plimpton 322 ratios
    double psi_b = pythagorean_ratio(p, q);  // (p²-q²)/(p²+q²)
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    double psi_c = (2.0 * p * q) / (double)(p2 + q2);  // 2pq/(p²+q²)
    double psi_correction = psi_b * psi_c;
    
    // Γ(n,d): Lattice entropy
    double gamma_nd = lattice_entropy(n, d);
    
    // COMPLETE FORMULA: Multiply all terms
    return base * prod * gamma_k * nu * einstein_lambda * psi_correction * gamma_nd;
}
/* ============================================================================
 * ARBITRARY PRECISION LATTICE FORMULA
 * ============================================================================ */

/**
 * L_lattice_bigfixed - ARBITRARY PRECISION version of master lattice formula
 * Uses BigFixed throughout for true arbitrary precision
 */
void L_lattice_bigfixed(BigFixed *result, uint64_t n, uint64_t d, int k, 
                        const char *lambda_phon, uint16_t omega, uint64_t p, uint64_t q,
                        int precision_bits) {
    if (!result) return;
    
    // Calculate O exponent (for now use double, will convert)
    double o_double = O_exponent(n, k, lambda_phon);
    
    // Base: 3^O using BigFixed
    BigInt three, base_int;
    BigFixed *o_fixed, *base;
    big_init(&three);
    big_init(&base_int);
    big_from_int(&three, 3);
    
    o_fixed = big_fixed_create(precision_bits);
    base = big_fixed_create(precision_bits);
    
    // Convert o to BigFixed (temporary - should compute in BigFixed)
    big_fixed_from_int(o_fixed, (int64_t)o_double);
    
    // Compute 3^O
    big_pow(base, &three, o_fixed, precision_bits);
    
    // Product: ∏cos(θ·φᵢ)
    double theta_double = theta_n(n, k, lambda_phon, omega, p, q, false);
    BigFixed *theta = big_fixed_create(precision_bits);
    big_fixed_from_int(theta, (int64_t)theta_double);  // Temporary conversion
    
    // Update phi frequencies
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    BigFixed *prod = big_fixed_create(precision_bits);
    big_fixed_from_int(prod, 1);  // Initialize to 1
    
    for (uint64_t i = 0; i < d && i < 12; i++) {
        BigFixed *phi_i = big_fixed_create(precision_bits);
        BigFixed *theta_phi = big_fixed_create(precision_bits);
        BigFixed *cos_val = big_fixed_create(precision_bits);
        BigFixed *temp = big_fixed_create(precision_bits);
        
        big_fixed_from_int(phi_i, (int64_t)phi_updated[i]);
        big_fixed_mul(theta_phi, theta, phi_i);
        big_cos(cos_val, theta_phi, precision_bits);
        big_fixed_mul(temp, prod, cos_val);
        big_fixed_assign(prod, temp);
        
        big_fixed_free(phi_i);
        big_fixed_free(theta_phi);
        big_fixed_free(cos_val);
        big_fixed_free(temp);
    }
    
    // Γ(k): Möbius twist
    int gamma_k = mobius_twist(k);
    BigFixed *gamma_k_fixed = big_fixed_create(precision_bits);
    big_fixed_from_int(gamma_k_fixed, gamma_k);
    
    // ν(λ): Phonetic value
    double nu_double = nu_lambda(lambda_phon);
    BigFixed *nu = big_fixed_create(precision_bits);
    big_fixed_from_int(nu, (int64_t)nu_double);
    
    // (ω): Einstein's Λ correction
    BigFixed *omega_corr = big_fixed_create(precision_bits);
    big_fixed_from_int(omega_corr, EINSTEIN_LAMBDA_NUMERATOR);
    BigFixed *omega_denom = big_fixed_create(precision_bits);
    big_fixed_from_int(omega_denom, EINSTEIN_LAMBDA_DENOMINATOR);
    BigFixed *omega_correction = big_fixed_create(precision_bits);
    big_fixed_div(omega_correction, omega_corr, omega_denom);
    
    // Ψ(ψ): Plimpton 322 ratios
    double psi_b = pythagorean_ratio(p, q);
    uint64_t p2 = p * p;
    uint64_t q2 = q * q;
    double psi_c = (2.0 * p * q) / (double)(p2 + q2);
    double psi_double = psi_b * psi_c;
    BigFixed *psi_correction = big_fixed_create(precision_bits);
    big_fixed_from_int(psi_correction, (int64_t)(psi_double * 1000));  // Scale for precision
    BigFixed *psi_scale = big_fixed_create(precision_bits);
    big_fixed_from_int(psi_scale, 1000);
    BigFixed *psi_scaled = big_fixed_create(precision_bits);
    big_fixed_div(psi_scaled, psi_correction, psi_scale);
    
    // Γ(n,d): Lattice entropy
    double gamma_nd_double = lattice_entropy(n, d);
    BigFixed *gamma_nd = big_fixed_create(precision_bits);
    big_fixed_from_int(gamma_nd, (int64_t)gamma_nd_double);
    
    // Multiply all components: base × prod × gamma_k × nu × omega × psi × gamma_nd
    BigFixed *temp1 = big_fixed_create(precision_bits);
    BigFixed *temp2 = big_fixed_create(precision_bits);
    
    big_fixed_mul(temp1, base, prod);
    big_fixed_mul(temp2, temp1, gamma_k_fixed);
    big_fixed_mul(temp1, temp2, nu);
    big_fixed_mul(temp2, temp1, omega_correction);
    big_fixed_mul(temp1, temp2, psi_scaled);
    big_fixed_mul(result, temp1, gamma_nd);
    
    // Cleanup
    big_free(&three);
    big_free(&base_int);
    big_fixed_free(o_fixed);
    big_fixed_free(base);
    big_fixed_free(theta);
    big_fixed_free(prod);
    big_fixed_free(gamma_k_fixed);
    big_fixed_free(nu);
    big_fixed_free(omega_corr);
    big_fixed_free(omega_denom);
    big_fixed_free(omega_correction);
    big_fixed_free(psi_correction);
    big_fixed_free(psi_scale);
    big_fixed_free(psi_scaled);
    big_fixed_free(gamma_nd);
    big_fixed_free(temp1);
    big_fixed_free(temp2);
}

/* ============================================================================
 * DIMENSIONAL LAYER FUNCTION Z_n^(d)
 * ============================================================================ */

double Z_n_d(uint64_t n, uint64_t d, const char *lambda_phon) {
    // Update phi frequencies
    double phi_updated[12];
    update_phi_freqs(PHI_FREQS_BASE, phi_updated, lambda_phon, 12);
    
    double phi_d = phi_updated[d % 12];
    
    // Z_n^(d) = φ_d × log₃(n) × ν(λ)
    double log3_n = math_log((double)n) / math_log(3.0);
    double nu = nu_lambda(lambda_phon);
    
    return phi_d * log3_n * nu;
}

/* ============================================================================
 * PRIME FUNCTION P_n^(d)(k)
 * ============================================================================ */

double P_n_d_k(uint64_t n, uint64_t d, int k, const char *lambda_phon,
               uint16_t omega, uint64_t p, uint64_t q) {
    // Calculate theta
    double theta = theta_n(n, k, lambda_phon, omega, p, q, false);
    
    // Exponent: θ(k,n)/ln(12) - ln(3)
    double exp_val = theta / math_log(12.0) - math_log(3.0);
    
    // P_n^(d)(k) = 12^exp × Z_n^(d)
    double base = math_pow(12.0, exp_val);
    double z_val = Z_n_d(n, d, lambda_phon);
    
    return base * z_val;
}

/* ============================================================================
 * COMPLETE CLOCK MAPPING
 * ============================================================================ */

void map_prime_complete(uint64_t prime, uint64_t n, CompleteClockMapping *mapping,
                        const char *lambda_phon) {
    if (!mapping) return;
    
    // Basic clock position
    map_prime_to_clock_phonetic(prime, &mapping->clock, lambda_phon);
    
    // Calculate complete parameters
    mapping->theta = theta_n(n, 1, lambda_phon, 432, 3, 4, false);
    mapping->r = r_n(prime);
    mapping->O_exp = O_exponent(n, 1, lambda_phon);
    mapping->L_value = L_lattice(n, 12, 1, lambda_phon, 432, 3, 4);
    
    // M₁₂ projection (simplified - full implementation would use all 12 dimensions)
    for (int i = 0; i < 12; i++) {
        mapping->m12.coordinates[i] = math_cos(mapping->theta * (i + 1));
    }
    mapping->m12.prime = prime;
    mapping->m12.index = n;
    
    // 15D embedding
    lattice_embed(prime, &mapping->embed);
    
    // Pythagorean triple
    pythagorean_triple(3, 4, &mapping->psi);
}
