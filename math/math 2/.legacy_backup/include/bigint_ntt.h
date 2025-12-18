#ifndef BIGINT_NTT_H
#define BIGINT_NTT_H

#include "bigint_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * bigint_ntt.h - Number Theoretic Transform for BigInt
 * 
 * NTT is the modular arithmetic analog of FFT, perfect for prime-based systems.
 * 
 * Key advantages over FFT:
 * - Works in Z/pZ (modular arithmetic) - no complex numbers
 * - Uses primitive roots of unity in modular arithmetic
 * - Exact arithmetic (no floating point errors)
 * - Perfect fit for crystalline lattice prime-based coordinates
 * 
 * Applications:
 * - Fast BigInt multiplication: O(n log n) vs O(n²)
 * - Fast polynomial multiplication
 * - Convolution in modular arithmetic
 * - CLLM attention optimization (future)
 * 
 * Algorithm:
 * 1. Choose prime p where p = k·2^n + 1 (for some k, n)
 * 2. Find primitive 2^n-th root of unity ω in Z/pZ
 * 3. Apply NTT using ω (similar to FFT structure)
 * 4. Multiply pointwise
 * 5. Apply inverse NTT
 * 
 * Complexity: O(n log n) where n is the number of coefficients
 */

/* ============================================================================
 * NTT CONTEXT
 * ============================================================================ */

/**
 * NTT Context - stores precomputed values for efficient NTT
 */
typedef struct {
    BigInt prime;              // Prime modulus p = k·2^n + 1
    BigInt root;               // Primitive 2^n-th root of unity
    BigInt* roots_forward;     // Precomputed roots for forward NTT
    BigInt* roots_inverse;     // Precomputed roots for inverse NTT
    size_t n;                  // Transform size (power of 2)
    int initialized;           // Initialization flag
} NTTContext;

/* ============================================================================
 * INITIALIZATION & CLEANUP
 * ============================================================================ */

/**
 * Initialize NTT context
 * 
 * Finds suitable prime and primitive root for given transform size.
 * 
 * @param ctx NTT context to initialize
 * @param n Transform size (must be power of 2)
 * @return 1 if successful, 0 if failed
 */
int ntt_init(NTTContext* ctx, size_t n);

/**
 * Free NTT context
 * 
 * @param ctx NTT context to free
 */
void ntt_free(NTTContext* ctx);

/* ============================================================================
 * PRIMITIVE ROOT FINDING
 * ============================================================================ */

/**
 * Find primitive n-th root of unity modulo prime p
 * 
 * A primitive n-th root of unity ω satisfies:
 * - ω^n ≡ 1 (mod p)
 * - ω^k ≢ 1 (mod p) for 0 < k < n
 * 
 * Algorithm:
 * 1. Ensure p = k·n + 1 for some k
 * 2. Find generator g of (Z/pZ)*
 * 3. Compute ω = g^((p-1)/n) mod p
 * 
 * @param root Output: primitive n-th root of unity
 * @param n Root degree (must divide p-1)
 * @param p Prime modulus
 * @return 1 if found, 0 if not found
 */
int ntt_find_primitive_root(BigInt* root, size_t n, const BigInt* p);

/**
 * Find suitable NTT prime
 * 
 * Finds prime p of form p = k·2^m + 1 where m >= log2(n)
 * This ensures 2^m divides p-1, so primitive 2^m-th roots exist.
 * 
 * @param prime Output: suitable prime
 * @param n Minimum transform size needed
 * @param bits Desired bit size of prime
 * @return 1 if found, 0 if failed
 */
int ntt_find_prime(BigInt* prime, size_t n, int bits);

/**
 * Check if g is a primitive root modulo p
 * 
 * A primitive root g generates all elements of (Z/pZ)*
 * i.e., {g^0, g^1, ..., g^(p-2)} = {1, 2, ..., p-1}
 * 
 * @param g Candidate primitive root
 * @param p Prime modulus
 * @return 1 if primitive root, 0 otherwise
 */
int ntt_is_primitive_root(const BigInt* g, const BigInt* p);

/* ============================================================================
 * NTT TRANSFORMS
 * ============================================================================ */

/**
 * Forward Number Theoretic Transform
 * 
 * Transforms sequence a[0..n-1] to frequency domain A[0..n-1]
 * 
 * Formula: A[k] = Σ(i=0 to n-1) a[i] · ω^(ik) mod p
 * 
 * Algorithm (Cooley-Tukey):
 * 1. Bit-reverse permutation
 * 2. Butterfly operations with roots of unity
 * 3. All operations modulo p
 * 
 * Complexity: O(n log n)
 * 
 * @param ctx NTT context (must be initialized)
 * @param output Output: transformed sequence (length n)
 * @param input Input: sequence to transform (length n)
 * @param n Sequence length (must be power of 2)
 */
void ntt_forward(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n);

/**
 * Inverse Number Theoretic Transform
 * 
 * Transforms frequency domain A[0..n-1] back to time domain a[0..n-1]
 * 
 * Formula: a[i] = (1/n) · Σ(k=0 to n-1) A[k] · ω^(-ik) mod p
 * 
 * Algorithm: Same as forward NTT but with inverse roots and scaling by 1/n
 * 
 * Complexity: O(n log n)
 * 
 * @param ctx NTT context (must be initialized)
 * @param output Output: inverse transformed sequence (length n)
 * @param input Input: frequency domain sequence (length n)
 * @param n Sequence length (must be power of 2)
 */
void ntt_inverse(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n);

/* ============================================================================
 * NTT-BASED MULTIPLICATION
 * ============================================================================ */

/**
 * Fast BigInt multiplication using NTT
 * 
 * Multiplies two BigInts using Number Theoretic Transform.
 * 
 * Algorithm:
 * 1. Convert BigInts to coefficient arrays
 * 2. Pad to power of 2
 * 3. Apply forward NTT to both
 * 4. Pointwise multiply in frequency domain
 * 5. Apply inverse NTT
 * 6. Handle carries and convert back to BigInt
 * 
 * Complexity: O(n log n) where n = len(a) + len(b)
 * 
 * Comparison:
 * - Naive multiplication: O(n²)
 * - Karatsuba: O(n^1.585)
 * - NTT: O(n log n)
 * 
 * For n = 10,000 digits:
 * - Naive: 100,000,000 operations
 * - Karatsuba: ~398,000 operations
 * - NTT: ~130,000 operations (3x faster than Karatsuba!)
 * 
 * @param result Output: a × b
 * @param a First number
 * @param b Second number
 * @return 1 if successful, 0 if failed
 */
int big_ntt_multiply(BigInt* result, const BigInt* a, const BigInt* b);

/**
 * NTT-based polynomial multiplication
 * 
 * Multiplies two polynomials represented as coefficient arrays.
 * 
 * @param result Output: product polynomial (length n+m-1)
 * @param a First polynomial coefficients (length n)
 * @param n Length of first polynomial
 * @param b Second polynomial coefficients (length m)
 * @param m Length of second polynomial
 * @param ctx NTT context
 * @return 1 if successful, 0 if failed
 */
int ntt_poly_multiply(BigInt* result, const BigInt* a, size_t n,
                      const BigInt* b, size_t m, const NTTContext* ctx);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * Bit-reverse permutation
 * 
 * Rearranges array elements according to bit-reversed indices.
 * Required for Cooley-Tukey NTT algorithm.
 * 
 * Example for n=8:
 * Index: 0(000) 1(001) 2(010) 3(011) 4(100) 5(101) 6(110) 7(111)
 * Rev:   0(000) 4(100) 2(010) 6(110) 1(001) 5(101) 3(011) 7(111)
 * 
 * @param array Array to permute
 * @param n Array length (must be power of 2)
 */
void ntt_bit_reverse(BigInt* array, size_t n);

/**
 * Check if n is a power of 2
 * 
 * @param n Number to check
 * @return 1 if power of 2, 0 otherwise
 */
int ntt_is_power_of_2(size_t n);

/**
 * Round up to next power of 2
 * 
 * @param n Input number
 * @return Smallest power of 2 >= n
 */
size_t ntt_next_power_of_2(size_t n);

/**
 * Compute log2 of n (assuming n is power of 2)
 * 
 * @param n Power of 2
 * @return log2(n)
 */
int ntt_log2(size_t n);

/* ============================================================================
 * CRYSTALLINE LATTICE OPTIMIZATIONS
 * ============================================================================ */

/**
 * Find NTT prime using Ulam spiral structure
 * 
 * Uses crystalline lattice prime distribution to find suitable primes faster.
 * 
 * @param prime Output: suitable prime
 * @param n Transform size
 * @param bits Desired bit size
 * @return 1 if found, 0 if failed
 */
int ntt_find_prime_lattice(BigInt* prime, size_t n, int bits);

/**
 * Optimize NTT for crystalline lattice structure
 * 
 * Precomputes additional values for lattice-aware NTT.
 * 
 * @param ctx NTT context
 * @return 1 if successful, 0 if failed
 */
int ntt_optimize_for_lattice(NTTContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // BIGINT_NTT_H