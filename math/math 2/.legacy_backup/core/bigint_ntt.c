/*
 * bigint_ntt.c - Number Theoretic Transform for BigInt
 * 
 * Minimal implementation using EXISTING crystalline lattice functions.
 * 
 * Uses:
 * - big_powmod() for modular exponentiation
 * - big_is_prime_miller_rabin() for primality testing  
 * - big_mod_inverse() for modular inverse
 * - big_gcd() for GCD operations
 * - prime_next_power_of_2() for size calculations
 */

#include "../../include/bigint_ntt.h"
#include "../../include/bigint_core.h"
#include "../../include/prime_lowlevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

int ntt_is_power_of_2(size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

int ntt_log2(size_t n) {
    int log = 0;
    while (n > 1) {
        n >>= 1;
        log++;
    }
    return log;
}

size_t ntt_next_power_of_2(size_t n) {
    return (size_t)prime_next_power_of_2((uint64_t)n);
}

void ntt_bit_reverse(BigInt* array, size_t n) {
    if (!array || !ntt_is_power_of_2(n)) return;
    
    int bits = ntt_log2(n);
    
    for (size_t i = 0; i < n; i++) {
        size_t j = 0;
        size_t temp = i;
        
        for (int b = 0; b < bits; b++) {
            j = (j << 1) | (temp & 1);
            temp >>= 1;
        }
        
        if (i < j) {
            BigInt tmp;
            big_init(&tmp);
            big_copy(&tmp, &array[i]);
            big_copy(&array[i], &array[j]);
            big_copy(&array[j], &tmp);
            big_free(&tmp);
        }
    }
}

/* ============================================================================
 * PRIMITIVE ROOT FINDING
 * ============================================================================ */

/**
 * Find primitive n-th root of unity modulo prime p
 * Uses EXISTING big_powmod() and big_gcd()
 */
int ntt_find_primitive_root(BigInt* root, size_t n, const BigInt* p) {
    if (!root || !p || n == 0) return 0;
    
    // Special case for p = 65537 (known generator is 3)
    BigInt p_val;
    big_init(&p_val);
    big_from_int(&p_val, 65537);
    
    if (big_cmp(p, &p_val) == 0) {
        BigInt p_minus_1, n_bigint, quotient, remainder, generator;
        big_init(&p_minus_1);
        big_init(&n_bigint);
        big_init(&quotient);
        big_init(&remainder);
        big_init(&generator);
        
        BigInt one;
    char* p_debug = big_to_string(p);
    free(p_debug);
        big_init(&one);
        big_from_int(&one, 1);
        
        big_copy(&p_minus_1, p);
        big_sub(p, &one, &p_minus_1);  // p_minus_1 = p - 1
        big_from_int(&n_bigint, n);
        
        big_div(&p_minus_1, &n_bigint, &quotient, &remainder);
        
        if (big_is_zero(&remainder)) {
            // Use generator 3 for p = 65537
            big_from_int(&generator, 3);
            // Use EXISTING big_powmod()
            big_powmod(&generator, &quotient, p, root);
            
            big_free(&p_minus_1);
            big_free(&n_bigint);
            big_free(&quotient);
            big_free(&remainder);
            big_free(&generator);
            big_free(&one);
            big_free(&p_val);
            return 1;
        }
        
        big_free(&p_minus_1);
        big_free(&n_bigint);
        big_free(&quotient);
        big_free(&remainder);
        big_free(&generator);
        big_free(&one);
    }
    big_free(&p_val);
    
    // General case: try small generators
    BigInt p_minus_1, n_bigint, quotient, remainder, one;
    big_init(&p_minus_1);
    big_init(&n_bigint);
    big_init(&quotient);
    big_init(&remainder);
    big_init(&one);
    
    big_from_int(&one, 1);
    big_copy(&p_minus_1, p);
    big_sub(p, &one, &p_minus_1);  // p_minus_1 = p - 1
    big_from_int(&n_bigint, n);
    
    big_div(&p_minus_1, &n_bigint, &quotient, &remainder);
    
    if (!big_is_zero(&remainder)) {
        big_free(&p_minus_1);
        big_free(&n_bigint);
        big_free(&quotient);
        big_free(&remainder);
        big_free(&one);
        return 0;
    }
    
    // Try small values as generators
    for (uint64_t g = 2; g < 100; g++) {
        BigInt generator, candidate, test;
        big_init(&generator);
        big_init(&candidate);
        big_init(&test);
        
        big_from_int(&generator, g);
        
        // Use EXISTING big_powmod()
        big_powmod(&generator, &quotient, p, &candidate);
        big_powmod(&candidate, &n_bigint, p, &test);
        
        if (big_cmp(&test, &one) == 0) {
            // Verify it's primitive
            int is_primitive = 1;
            for (size_t k = 1; k < n; k++) {
                if (n % k == 0 && k != n) {
                    BigInt k_bigint, test_k;
                    big_init(&k_bigint);
                    big_init(&test_k);
                    
                    big_from_int(&k_bigint, k);
                    big_powmod(&candidate, &k_bigint, p, &test_k);
                    
                    if (big_cmp(&test_k, &one) == 0) {
                        is_primitive = 0;
                        big_free(&k_bigint);
                        big_free(&test_k);
                        break;
                    }
                    
                    big_free(&k_bigint);
                    big_free(&test_k);
                }
            }
            
            if (is_primitive) {
                big_copy(root, &candidate);
                big_free(&generator);
                big_free(&candidate);
                big_free(&test);
                big_free(&p_minus_1);
                big_free(&n_bigint);
                big_free(&quotient);
                big_free(&remainder);
                big_free(&one);
                return 1;
            }
        }
        
        big_free(&generator);
        big_free(&candidate);
        big_free(&test);
    }
    
    big_free(&p_minus_1);
    big_free(&n_bigint);
    big_free(&quotient);
    big_free(&remainder);
    big_free(&one);
    
    return 0;
}

/**
 * Find suitable NTT prime
 * Uses EXISTING big_is_prime_miller_rabin()
 */
int ntt_find_prime(BigInt* prime, size_t n, int bits) {
    if (!prime || n == 0 || bits < 16) return 0;
    
    // For small n, use known NTT-friendly prime
    if (n <= 16 && bits <= 64) {
        big_from_int(prime, 65537);  // 2^16 + 1
        return 1;
    }
    
    // For larger n, find prime of form k*2^m + 1
    int m = ntt_log2(ntt_next_power_of_2(n));
    
    BigInt two_pow_m, k, candidate, one, two;
    big_init(&two_pow_m);
    big_init(&k);
    big_init(&candidate);
    big_init(&one);
    big_init(&two);
    
    big_from_int(&one, 1);
    big_from_int(&two, 2);
    big_from_int(&two_pow_m, 1);
    
    // Compute 2^m
    for (int i = 0; i < m; i++) {
        BigInt temp;
        big_init(&temp);
        big_mul(&two_pow_m, &two, &temp);
        big_copy(&two_pow_m, &temp);
        big_free(&temp);
    }
    
    // Try k values until we find a prime
    for (uint64_t k_val = 1; k_val < 100000; k_val++) {
        big_from_int(&k, k_val);
        
        // candidate = k * 2^m + 1
        big_mul(&k, &two_pow_m, &candidate);
        big_add(&candidate, &one, &candidate);
        
        // Use EXISTING big_is_prime_miller_rabin()
        if (big_is_prime_miller_rabin(&candidate)) {
            big_copy(prime, &candidate);
            big_free(&two_pow_m);
            big_free(&k);
            big_free(&candidate);
            big_free(&one);
            big_free(&two);
            return 1;
        }
    }
    
    big_free(&two_pow_m);
    big_free(&k);
    big_free(&candidate);
    big_free(&one);
    big_free(&two);
    
    return 0;
}

/* ============================================================================
 * NTT CONTEXT
 * ============================================================================ */

int ntt_init(NTTContext* ctx, size_t n) {
    if (!ctx || !ntt_is_power_of_2(n)) return 0;
    
    ctx->n = n;
    ctx->initialized = 0;
    
    big_init(&ctx->prime);
    big_init(&ctx->root);
    
    // Find suitable prime
    if (!ntt_find_prime(&ctx->prime, n, 64)) {
        fprintf(stderr, "ntt_init: failed to find suitable prime\n");
        return 0;
    }
    
    // Find primitive n-th root of unity
    if (!ntt_find_primitive_root(&ctx->root, n, &ctx->prime)) {
        fprintf(stderr, "ntt_init: failed to find primitive root\n");
        big_free(&ctx->prime);
        big_free(&ctx->root);
        return 0;
    }
    
    // Precompute roots
    ctx->roots_forward = malloc(n * sizeof(BigInt));
    ctx->roots_inverse = malloc(n * sizeof(BigInt));
    
    if (!ctx->roots_forward || !ctx->roots_inverse) {
        big_free(&ctx->prime);
        big_free(&ctx->root);
        if (ctx->roots_forward) free(ctx->roots_forward);
        if (ctx->roots_inverse) free(ctx->roots_inverse);
        return 0;
    }
    
    // Compute powers of root using EXISTING big_powmod()
    for (size_t i = 0; i < n; i++) {
        big_init(&ctx->roots_forward[i]);
        
        if (i == 0) {
            big_from_int(&ctx->roots_forward[i], 1);
        } else {
            BigInt i_bigint;
            big_init(&i_bigint);
            big_from_int(&i_bigint, i);
            big_powmod(&ctx->root, &i_bigint, &ctx->prime, &ctx->roots_forward[i]);
            big_free(&i_bigint);
        }
    }
    
    // Compute inverse root using EXISTING big_mod_inverse()
    BigInt root_inv;
    big_init(&root_inv);
    if (!big_mod_inverse(&root_inv, &ctx->root, &ctx->prime)) {
        fprintf(stderr, "ntt_init: failed to compute inverse root\n");
        for (size_t i = 0; i < n; i++) {
            big_free(&ctx->roots_forward[i]);
        }
        free(ctx->roots_forward);
        free(ctx->roots_inverse);
        big_free(&ctx->prime);
        big_free(&ctx->root);
        big_free(&root_inv);
        return 0;
    }
    
    // Compute powers of inverse root
    for (size_t i = 0; i < n; i++) {
        big_init(&ctx->roots_inverse[i]);
        
        if (i == 0) {
            big_from_int(&ctx->roots_inverse[i], 1);
        } else {
            BigInt i_bigint;
            big_init(&i_bigint);
            big_from_int(&i_bigint, i);
            big_powmod(&root_inv, &i_bigint, &ctx->prime, &ctx->roots_inverse[i]);
            big_free(&i_bigint);
        }
    }
    
    big_free(&root_inv);
    ctx->initialized = 1;
    return 1;
}

void ntt_free(NTTContext* ctx) {
    if (!ctx || !ctx->initialized) return;
    
    big_free(&ctx->prime);
    big_free(&ctx->root);
    
    if (ctx->roots_forward) {
        for (size_t i = 0; i < ctx->n; i++) {
            big_free(&ctx->roots_forward[i]);
        }
        free(ctx->roots_forward);
    }
    
    if (ctx->roots_inverse) {
        for (size_t i = 0; i < ctx->n; i++) {
            big_free(&ctx->roots_inverse[i]);
        }
        free(ctx->roots_inverse);
    }
    
    ctx->initialized = 0;
}

/* ============================================================================
 * NTT TRANSFORMS - Using EXISTING big_mul(), big_add(), big_sub(), big_mod()
 * ============================================================================ */

void ntt_forward(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) return;
    
    // Copy input to output
    for (size_t i = 0; i < n; i++) {
        big_init(&output[i]);
        big_copy(&output[i], &input[i]);
    }
    
    // Bit-reverse permutation
    ntt_bit_reverse(output, n);
    
    // Cooley-Tukey butterfly operations
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                BigInt u, v, temp, rem;
                big_init(&u);
                big_init(&v);
                big_init(&temp);
                big_init(&rem);
                
                big_copy(&u, &output[i + j]);
                
                // v = output[i + j + half_len] * root (using EXISTING big_mul and big_mod)
                big_mul(&output[i + j + half_len], &ctx->roots_forward[root_idx], &temp);
                big_mod(&temp, &ctx->prime, &v);
                
                // output[i + j] = (u + v) mod prime (using EXISTING big_add and big_mod)
                big_add(&u, &v, &temp);
                big_mod(&temp, &ctx->prime, &output[i + j]);
                
                // output[i + j + half_len] = (u - v) mod prime (using EXISTING big_sub and big_mod)
                big_sub(&u, &v, &temp);
                if (temp.negative) {
                    big_add(&temp, &ctx->prime, &temp);
                }
                big_mod(&temp, &ctx->prime, &output[i + j + half_len]);
                
                big_free(&u);
                big_free(&v);
                big_free(&temp);
                big_free(&rem);
            }
        }
    }
}

void ntt_inverse(const NTTContext* ctx, BigInt* output, const BigInt* input, size_t n) {
    if (!ctx || !ctx->initialized || !output || !input || n != ctx->n) return;
    
    // Copy input to output
    for (size_t i = 0; i < n; i++) {
        big_init(&output[i]);
        big_copy(&output[i], &input[i]);
    }
    
    // Bit-reverse permutation
    ntt_bit_reverse(output, n);
    
    // Cooley-Tukey butterfly operations with inverse roots
    for (size_t len = 2; len <= n; len *= 2) {
        size_t half_len = len / 2;
        size_t step = n / len;
        
        for (size_t i = 0; i < n; i += len) {
            for (size_t j = 0; j < half_len; j++) {
                size_t root_idx = j * step;
                
                BigInt u, v, temp, rem;
                big_init(&u);
                big_init(&v);
                big_init(&temp);
                big_init(&rem);
                
                big_copy(&u, &output[i + j]);
                
                // v = output[i + j + half_len] * inverse_root
                big_mul(&output[i + j + half_len], &ctx->roots_inverse[root_idx], &temp);
                big_mod(&temp, &ctx->prime, &v);
                
                // output[i + j] = (u + v) mod prime
                big_add(&u, &v, &temp);
                big_mod(&temp, &ctx->prime, &output[i + j]);
                
                // output[i + j + half_len] = (u - v) mod prime
                big_sub(&u, &v, &temp);
                if (temp.negative) {
                    big_add(&temp, &ctx->prime, &temp);
                }
                big_mod(&temp, &ctx->prime, &output[i + j + half_len]);
                
                big_free(&u);
                big_free(&v);
                big_free(&temp);
                big_free(&rem);
            }
        }
    }
    
    // Scale by 1/n using EXISTING big_mod_inverse()
    BigInt n_bigint, n_inv;
    big_init(&n_bigint);
    big_init(&n_inv);
    
    big_from_int(&n_bigint, n);
    if (big_mod_inverse(&n_inv, &n_bigint, &ctx->prime)) {
        for (size_t i = 0; i < n; i++) {
            BigInt temp;
            big_init(&temp);
            
            big_mul(&output[i], &n_inv, &temp);
            big_mod(&temp, &ctx->prime, &output[i]);
            
            big_free(&temp);
        }
    }
    
    big_free(&n_bigint);
    big_free(&n_inv);
}

/* ============================================================================
 * NTT-BASED MULTIPLICATION - Using EXISTING big_mul()
 * ============================================================================ */

int big_ntt_multiply(BigInt* result, const BigInt* a, const BigInt* b) {
    if (!result || !a || !b) return 0;
    
    // For small numbers, use EXISTING big_mul()
    if (a->len < 32 || b->len < 32) {
        big_mul(a, b, result);
        return 1;
    }
    
    // For large numbers, NTT may be faster
    // (Implementation would go here - this is a placeholder)
    // For now, just use existing big_mul()
    big_mul(a, b, result);
    return 1;
}