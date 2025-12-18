#ifndef PRIME_FACTOR_EXTRACTION_H
#define PRIME_FACTOR_EXTRACTION_H

#include <stdint.h>
#include <stdbool.h>
#include "multi_torus_tracker.h"

/**
 * Prime Factor Extraction
 * 
 * Extracts prime factors p and q from the 20-torus structure.
 * 
 * Key Insight: The 20-torus structure represents complete pq factorization:
 * - Torus 1 and 2: p and q (the two coprime primes)
 * - Remaining tori: Higher-order factors (p², q², pq, etc.)
 * 
 * We identify p and q by finding the two tori with coprime periods.
 */

typedef struct {
    // Input
    uint64_t n;                    // The number to factor (n = p × q)
    
    // Extracted factors
    uint64_t p;                    // First prime factor
    uint64_t q;                    // Second prime factor
    
    // Torus information
    int p_torus_index;             // Index of p-torus in tracker
    int q_torus_index;             // Index of q-torus in tracker
    
    double p_torus_center;         // Center k of p-torus
    double p_torus_amplitude;      // Amplitude of p-torus
    int p_torus_period;            // Period of p-torus
    
    double q_torus_center;         // Center k of q-torus
    double q_torus_amplitude;      // Amplitude of q-torus
    int q_torus_period;            // Period of q-torus
    
    // Verification
    bool extraction_successful;    // Was extraction successful?
    bool verification_passed;      // Does p × q = n?
    double confidence;             // Confidence in extraction (0-1)
} PrimeFactorResult;

/**
 * Extract p and q from 20-torus structure
 * 
 * Analyzes the torus structure to identify the two tori with coprime periods,
 * which correspond to the prime factors p and q.
 * 
 * @param tracker The multi-torus tracker with identified tori
 * @param n The number to factor (n = p × q)
 * @return Prime factor result (must be freed with free_prime_factor_result)
 */
PrimeFactorResult* extract_prime_factors_from_torus(
    const MultiTorusTracker* tracker,
    uint64_t n
);

/**
 * Free prime factor result
 */
void free_prime_factor_result(PrimeFactorResult* result);

/**
 * Print prime factor extraction results
 */
void print_prime_factor_result(const PrimeFactorResult* result);

/**
 * Export prime factor result to file
 */
void export_prime_factor_result(
    const PrimeFactorResult* result,
    const char* filename
);

#endif // PRIME_FACTOR_EXTRACTION_H
