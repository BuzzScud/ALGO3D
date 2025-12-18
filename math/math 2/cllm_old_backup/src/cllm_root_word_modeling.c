/*
 * CLLM Root Word Modeling
 * 
 * Implements linguistic root extraction and morphological analysis
 * using prime factorization and crystalline lattice structure.
 * 
 * Core Concept:
 * - Prime numbers represent linguistic roots
 * - Composite numbers are variations (tense, plurality, case, etc.)
 * - Factorization reveals morphological structure
 * - Coprime relationships indicate semantic distance
 * 
 * Example:
 * - "run" (prime 5) is a root
 * - "running" (5 × 2) is progressive form
 * - "runs" (5 × 3) is third person singular
 * - "ran" (5 × 7) is past tense
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm_attention.h"
#include "../include/clock_lattice.h"  // For validate_prime_by_clock_position()
#include "math/transcendental.h"
#include "math/types.h"  // For MATH_PI and MATH_PHI constants (NEW math library)
// Prime cache for fast lookup
#define PRIME_CACHE_SIZE 1000
static uint64_t prime_cache[PRIME_CACHE_SIZE];
static int prime_cache_initialized = 0;

/**
 * Initialize prime cache
 */
static void init_prime_cache(void) {
    if (prime_cache_initialized) return;
    
    prime_cache[0] = 2;
    prime_cache[1] = 3;
    
    int count = 2;
    uint64_t candidate = 5;
    
    while (count < PRIME_CACHE_SIZE) {
        int is_prime = 1;
        uint64_t sqrt_cand = (uint64_t)math_sqrt((double)candidate);
        
        for (int i = 0; i < count && prime_cache[i] <= sqrt_cand; i++) {
            if (candidate % prime_cache[i] == 0) {
                is_prime = 0;
                break;
            }
        }
        
        if (is_prime) {
            prime_cache[count++] = candidate;
        }
        
        candidate += 2;
    }
    
    prime_cache_initialized = 1;
}

// REMOVED: Local is_prime() implementation
// Internal code trusts the deterministic clock lattice structure
// Use validate_prime_by_clock_position() directly

/**
 * Get nth prime number
 */
__attribute__((unused))
static uint64_t get_nth_prime(uint32_t n) {
    init_prime_cache();
    
    if (n < PRIME_CACHE_SIZE) {
        return prime_cache[n];
    }
    
    // For larger n, compute on the fly
    uint64_t count = PRIME_CACHE_SIZE;
    uint64_t candidate = prime_cache[PRIME_CACHE_SIZE - 1] + 2;
    
    while (count <= n) {
        // Internal: Trust deterministic clock lattice
        if (validate_prime_by_clock_position(candidate)) {
            if (count == n) return candidate;
            count++;
        }
        candidate += 2;
    }
    
    return candidate;
}

/**
 * Get prime number for token
 * 
 * Mapping strategy:
 * - Small token IDs (< 1000): Direct prime mapping
 * - Larger token IDs: Hash to prime range
 * - Special tokens: Reserved primes (2, 3, 5, 7, 11)
 */
uint64_t cllm_get_token_prime(uint32_t token_id) {
    init_prime_cache();
    
    // Special tokens
    if (token_id == 0) return 2;   // PAD
    if (token_id == 1) return 3;   // UNK
    if (token_id == 2) return 5;   // BOS
    if (token_id == 3) return 7;   // EOS
    if (token_id == 4) return 11;  // MASK
    
    // Direct mapping for small IDs
    if (token_id < PRIME_CACHE_SIZE) {
        return prime_cache[token_id % PRIME_CACHE_SIZE];
    }
    
    // Hash to prime range for larger IDs
    uint32_t prime_idx = token_id % PRIME_CACHE_SIZE;
    return prime_cache[prime_idx];
}

/**
 * Compute lattice coordinates for a token
 */
void cllm_compute_token_lattice_coords(uint32_t token_id, uint64_t prime, float* coords) {
    if (!coords) return;
    
    init_prime_cache();
    
    // Find prime index
    uint32_t prime_index = 0;
    for (int i = 0; i < PRIME_CACHE_SIZE; i++) {
        if (prime_cache[i] == prime) {
            prime_index = i;
            break;
        }
    }
    
    // Ulam spiral: radius grows with square root of index
    double radius = math_sqrt((double)prime_index + 1.0);
    
    // Golden angle for optimal packing (use macros from prime_types.h)
    double golden_angle = 2.0 * MATH_PI / (MATH_PHI * MATH_PHI);
    double angle = golden_angle * (double)prime_index;
    
    // Normalize angle
    while (angle >= 2.0 * MATH_PI) {
        angle -= 2.0 * MATH_PI;
    }
    
    // Convert to 3D coordinates
    coords[0] = radius * math_cos(angle);
    coords[1] = radius * math_sin(angle);
    coords[2] = math_log((double)prime + 1.0);
    
    // Add token-specific perturbation
    double token_phase = 2.0 * MATH_PI * (double)token_id / 1000.0;
    coords[0] += 0.1f * math_cos(token_phase);
    coords[1] += 0.1f * math_sin(token_phase);
    coords[2] += 0.1f * math_sin(token_phase * MATH_PHI);
}

/**
 * Compute GCD using Euclidean algorithm
 */
static uint64_t compute_gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Compute semantic similarity using prime factorization
 */
double cllm_compute_prime_similarity(uint64_t prime1, uint64_t prime2) {
    if (prime1 == prime2) return 1.0;
    
    // Compute GCD
    uint64_t gcd = compute_gcd(prime1, prime2);
    
    // Coprime (gcd = 1): Maximally different
    if (gcd == 1) {
        return 0.0;
    }
    
    // Share factors: Similarity based on GCD
    double similarity = (double)gcd / (double)(prime1 < prime2 ? prime1 : prime2);
    return similarity;
}

/**
 * Extract root word from token
 * 
 * For composite numbers, extract the smallest prime factor (root).
 * For prime numbers, the token itself is the root.
 */
uint32_t cllm_extract_root_word(uint32_t token_id, uint64_t prime) {
    init_prime_cache();
    
    // If prime, it's already a root
    // Internal: Trust deterministic clock lattice
    if (validate_prime_by_clock_position(prime)) {
        return token_id;
    }
    
    // Find smallest prime factor
    for (int i = 0; i < PRIME_CACHE_SIZE; i++) {
        if (prime % prime_cache[i] == 0) {
            // Find token with this prime
            for (uint32_t tid = 0; tid < 10000; tid++) {
                if (cllm_get_token_prime(tid) == prime_cache[i]) {
                    return tid;
                }
            }
            break;
        }
    }
    
    return token_id;  // Fallback
}

/**
 * Compute morphological relationship between tokens
 * 
 * Returns:
 * 0 = Unrelated (coprime)
 * 1 = Related (share factors)
 * 2 = Derived (one divides other)
 * 3 = Same (identical primes)
 */
int cllm_compute_morphological_relationship(uint64_t token1_prime,
                                           uint64_t token2_prime) {
    if (token1_prime == token2_prime) {
        return 3;  // Same
    }
    
    uint64_t gcd = compute_gcd(token1_prime, token2_prime);
    
    if (gcd == 1) {
        return 0;  // Unrelated (coprime)
    }
    
    // Check if one divides the other
    if (token1_prime % token2_prime == 0 || token2_prime % token1_prime == 0) {
        return 2;  // Derived
    }
    
    return 1;  // Related (share factors)
}

/**
 * Compute hyperdimensional distance
 */
double cllm_compute_hyperdimensional_distance(const double* coords1,
                                             const double* coords2,
                                             uint64_t prime1,
                                             uint64_t prime2) {
    if (!coords1 || !coords2) return 0.0;
    
    // Euclidean distance in 3D
    double dx = coords1[0] - coords2[0];
    double dy = coords1[1] - coords2[1];
    double dz = coords1[2] - coords2[2];
    double euclidean = math_sqrt(dx*dx + dy*dy + dz*dz);
    
    // Prime distance
    uint64_t gcd = compute_gcd(prime1, prime2);
    double prime_dist = (gcd == 1) ? 1.0 : (1.0 / (double)gcd);
    
    // Combined distance
    return euclidean * prime_dist;
}

/**
 * Apply symmetry operation to attention weights
 */
void cllm_apply_symmetry_operation(float* weights, int seq_len, int symmetry_type) {
    if (!weights || seq_len <= 0) return;
    
    int operation = symmetry_type % 24;
    
    if (operation < 12) {
        // Rotation (12-fold symmetry)
        double angle = 2.0 * 3.14159265358979323846 * (double)operation / 12.0;
        
        for (int i = 0; i < seq_len; i++) {
            double phase = angle * (double)i / (double)seq_len;
            double rotation = (1.0 + math_cos(phase)) / 2.0;
            weights[i] *= rotation;
        }
    } else {
        // Reflection (mirror planes)
        int mirror_axis = operation - 12;
        (void)mirror_axis; /* Reserved for future axis-specific reflections */
        
        for (int i = 0; i < seq_len; i++) {
            int reflected_i = seq_len - 1 - i;
            if (i < reflected_i) {
                double temp = weights[i];
                weights[i] = weights[reflected_i];
                weights[reflected_i] = temp;
            }
        }
    }
}

/**
 * Compute Fourier transform of attention pattern
 * 
 * Simple DFT implementation for attention analysis
 */
void cllm_compute_attention_fourier(const float* attention_weights,
                                    float* fourier_output,
                                    int seq_len) {
    if (!attention_weights || !fourier_output || seq_len <= 0) return;
    
    // Use MATH_PI macro from prime_types.h
    
    for (int k = 0; k < seq_len; k++) {
        double real = 0.0;
        double imag = 0.0;
        
        for (int n = 0; n < seq_len; n++) {
            double angle = -2.0 * MATH_PI * (double)k * (double)n / (double)seq_len;
            real += attention_weights[n] * math_cos(angle);
            imag += attention_weights[n] * math_sin(angle);
        }
        
        // Magnitude
        fourier_output[k] = math_sqrt(real * real + imag * imag);
    }
}

/**
 * Apply Fourier-based dampening to attention
 */
void cllm_apply_fourier_dampening(float* attention_weights,
                                  int seq_len,
                                  double cutoff_freq) {
    if (!attention_weights || seq_len <= 0) return;
    
    // Compute Fourier transform
    float* fourier = (float*)malloc(seq_len * sizeof(float));
    if (!fourier) return;
    
    cllm_compute_attention_fourier(attention_weights, fourier, seq_len);
    
    // Apply low-pass filter
    for (int i = 0; i < seq_len; i++) {
        double freq = (double)i / (double)seq_len;
        if (freq > cutoff_freq) {
            fourier[i] *= math_exp(-(freq - cutoff_freq) * 10.0);
        }
    }
    
    // Inverse transform (simplified - just scale by filtered magnitudes)
    for (int i = 0; i < seq_len; i++) {
        double scale = fourier[i] / (fourier[0] + 1e-8);
        attention_weights[i] *= scale;
    }
    
    free(fourier);
}
