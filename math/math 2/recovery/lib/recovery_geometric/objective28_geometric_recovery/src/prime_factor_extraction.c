/**
 * prime_factor_extraction.c - Extract p and q from 20-Torus Structure
 * 
 * This module implements Phase 2 of the geometric recovery algorithm:
 * extracting prime factors p and q from the identified torus structure.
 * 
 * Key Insight: The 20-torus structure represents complete pq factorization:
 * - Torus 1 and 2: p and q (the two coprime primes)
 * - Remaining tori: Higher-order factors (p², q², pq, etc.)
 * 
 * We identify p and q by finding the two tori with coprime periods.
 */

#include "../include/prime_factor_extraction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

/**
 * Compute GCD using Euclidean algorithm
 */
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Check if two numbers are coprime (GCD = 1)
 */
static bool are_coprime(uint64_t a, uint64_t b) {
    return gcd(a, b) == 1;
}

/**
 * Analyze torus to extract potential prime factor
 * 
 * The period of a torus corresponds to a factor of n.
 * For prime factors p and q, their periods should be coprime.
 */
static uint64_t extract_factor_from_torus(const TorusDescriptor* torus) {
    if (!torus) return 0;
    
    // The period directly corresponds to a factor
    // For p=2, q=5, n=10, we see periods of 2 and 5
    return (uint64_t)torus->period;
}

/**
 * Find the two tori with coprime periods
 * 
 * These are the p-torus and q-torus.
 */
static bool find_coprime_tori(
    const MultiTorusTracker* tracker,
    int* p_torus_idx,
    int* q_torus_idx
) {
    if (!tracker || tracker->num_tori < 2) return false;
    
    // Check all pairs of tori
    for (int i = 0; i < tracker->num_tori; i++) {
        for (int j = i + 1; j < tracker->num_tori; j++) {
            const TorusDescriptor* torus_i = get_torus(tracker, i);
            const TorusDescriptor* torus_j = get_torus(tracker, j);
            
            if (!torus_i || !torus_j) continue;
            
            uint64_t period_i = (uint64_t)torus_i->period;
            uint64_t period_j = (uint64_t)torus_j->period;
            
            // Check if periods are coprime
            if (are_coprime(period_i, period_j)) {
                *p_torus_idx = i;
                *q_torus_idx = j;
                return true;
            }
        }
    }
    
    return false;
}

/**
 * Extract p and q from 20-torus structure
 */
PrimeFactorResult* extract_prime_factors_from_torus(
    const MultiTorusTracker* tracker,
    uint64_t n
) {
    if (!tracker) return NULL;
    
    PrimeFactorResult* result = (PrimeFactorResult*)calloc(1, sizeof(PrimeFactorResult));
    if (!result) return NULL;
    
    result->n = n;
    result->extraction_successful = false;
    
    // Find the two tori with coprime periods
    int p_torus_idx = -1;
    int q_torus_idx = -1;
    
    if (!find_coprime_tori(tracker, &p_torus_idx, &q_torus_idx)) {
        fprintf(stderr, "Failed to find coprime tori\n");
        return result;
    }
    
    // Extract p and q from the coprime tori
    const TorusDescriptor* p_torus = get_torus(tracker, p_torus_idx);
    const TorusDescriptor* q_torus = get_torus(tracker, q_torus_idx);
    
    if (!p_torus || !q_torus) {
        fprintf(stderr, "Failed to get torus descriptors\n");
        return result;
    }
    
    result->p = extract_factor_from_torus(p_torus);
    result->q = extract_factor_from_torus(q_torus);
    
    // Ensure p <= q by convention
    if (result->p > result->q) {
        uint64_t temp = result->p;
        result->p = result->q;
        result->q = temp;
        
        int temp_idx = p_torus_idx;
        p_torus_idx = q_torus_idx;
        q_torus_idx = temp_idx;
    }
    
    // Store torus indices
    result->p_torus_index = p_torus_idx;
    result->q_torus_index = q_torus_idx;
    
    // Verify p × q = n
    uint64_t product = result->p * result->q;
    result->verification_passed = (product == n);
    
    if (result->verification_passed) {
        result->extraction_successful = true;
        
        // Store torus parameters
        result->p_torus_center = p_torus->center_k;
        result->p_torus_amplitude = p_torus->amplitude;
        result->p_torus_period = p_torus->period;
        
        result->q_torus_center = q_torus->center_k;
        result->q_torus_amplitude = q_torus->amplitude;
        result->q_torus_period = q_torus->period;
        
        // Compute confidence based on torus confidence
        result->confidence = (p_torus->confidence + q_torus->confidence) / 2.0;
    } else {
        fprintf(stderr, "Verification failed: p=%lu, q=%lu, p*q=%lu, n=%lu\n",
                result->p, result->q, product, n);
    }
    
    return result;
}

/**
 * Free prime factor result
 */
void free_prime_factor_result(PrimeFactorResult* result) {
    if (result) {
        free(result);
    }
}

/**
 * Print prime factor extraction results
 */
void print_prime_factor_result(const PrimeFactorResult* result) {
    if (!result) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Prime Factor Extraction Results                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  n = %lu\n\n", result->n);
    
    if (result->extraction_successful) {
        printf("  ✓ Extraction Successful\n\n");
        printf("  Prime Factors:\n");
        printf("    p = %lu (Torus %d)\n", result->p, result->p_torus_index + 1);
        printf("    q = %lu (Torus %d)\n", result->q, result->q_torus_index + 1);
        printf("\n");
        
        printf("  Verification:\n");
        printf("    p × q = %lu\n", result->p * result->q);
        printf("    n     = %lu\n", result->n);
        printf("    Match: %s\n", result->verification_passed ? "✓ YES" : "✗ NO");
        printf("\n");
        
        printf("  p-Torus Parameters:\n");
        printf("    Center:    %.2f\n", result->p_torus_center);
        printf("    Amplitude: %.2f\n", result->p_torus_amplitude);
        printf("    Period:    %d\n", result->p_torus_period);
        printf("\n");
        
        printf("  q-Torus Parameters:\n");
        printf("    Center:    %.2f\n", result->q_torus_center);
        printf("    Amplitude: %.2f\n", result->q_torus_amplitude);
        printf("    Period:    %d\n", result->q_torus_period);
        printf("\n");
        
        printf("  Confidence: %.2f%%\n", result->confidence * 100.0);
    } else {
        printf("  ✗ Extraction Failed\n");
        if (result->p > 0 && result->q > 0) {
            printf("    Extracted: p=%lu, q=%lu\n", result->p, result->q);
            printf("    Product:   %lu\n", result->p * result->q);
            printf("    Expected:  %lu\n", result->n);
            printf("    Reason: Verification failed (p × q ≠ n)\n");
        } else {
            printf("    Reason: Could not find coprime tori\n");
        }
    }
    
    printf("\n");
}

/**
 * Export prime factor result to file
 */
void export_prime_factor_result(
    const PrimeFactorResult* result,
    const char* filename
) {
    if (!result || !filename) return;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Failed to open %s for writing\n", filename);
        return;
    }
    
    fprintf(f, "# Prime Factor Extraction Results\n");
    fprintf(f, "n,%lu\n", result->n);
    fprintf(f, "extraction_successful,%d\n", result->extraction_successful);
    fprintf(f, "verification_passed,%d\n", result->verification_passed);
    fprintf(f, "\n");
    
    if (result->extraction_successful) {
        fprintf(f, "p,%lu\n", result->p);
        fprintf(f, "q,%lu\n", result->q);
        fprintf(f, "p_times_q,%lu\n", result->p * result->q);
        fprintf(f, "\n");
        
        fprintf(f, "p_torus_index,%d\n", result->p_torus_index);
        fprintf(f, "p_torus_center,%.6f\n", result->p_torus_center);
        fprintf(f, "p_torus_amplitude,%.6f\n", result->p_torus_amplitude);
        fprintf(f, "p_torus_period,%d\n", result->p_torus_period);
        fprintf(f, "\n");
        
        fprintf(f, "q_torus_index,%d\n", result->q_torus_index);
        fprintf(f, "q_torus_center,%.6f\n", result->q_torus_center);
        fprintf(f, "q_torus_amplitude,%.6f\n", result->q_torus_amplitude);
        fprintf(f, "q_torus_period,%d\n", result->q_torus_period);
        fprintf(f, "\n");
        
        fprintf(f, "confidence,%.6f\n", result->confidence);
    }
    
    fclose(f);
    
    printf("  ✓ Exported results to %s\n", filename);
}
