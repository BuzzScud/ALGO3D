/**
 * @file cllm_price_projection.h
 * @brief CLLM-based Price Projection API
 * 
 * This file provides a C API for price projections using CLLM with 88D threading.
 * Threading is MANDATORY - all computation happens in parallel across 88D threads.
 * 
 * Integration with Price Projection tab:
 * - JavaScript calls PHP backend
 * - PHP calls these C functions
 * - C uses CLLM with 88D threading
 * - C uses CrystallineAbacus for arbitrary precision
 */

#ifndef CLLM_PRICE_PROJECTION_H
#define CLLM_PRICE_PROJECTION_H

#include "cllm.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONFIGURATION STRUCTURE
// ============================================================================

/**
 * Price projection configuration
 */
typedef struct {
    uint32_t depth_prime;        // Depth prime for projection
    double base;                 // Base price (last historical price)
    uint32_t steps;              // Number of projection steps
    uint32_t projection_count;   // Number of projection lines to generate
    double omega_hz;             // Omega frequency in Hz (default: 432)
    uint32_t decimals;           // Decimal precision (default: 8)
} PriceProjectionConfig;

/**
 * Price projection result
 */
typedef struct {
    double** projection_lines;   // [projection_count][steps] - projection points
    uint32_t num_lines;          // Number of projection lines
    uint32_t steps_per_line;     // Steps per line
    bool success;                // Success flag
    char* error_message;         // Error message if failed
} PriceProjectionResult;

// ============================================================================
// MAIN API FUNCTIONS
// ============================================================================

/**
 * Compute price projections using CLLM with 88D threading
 * 
 * This function:
 * 1. Creates a CLLM model (88D threading MANDATORY)
 * 2. Generates triads around depth_prime
 * 3. Computes crystalline projections for each triad
 * 4. Returns projection lines
 * 
 * CRITICAL: Threading is MANDATORY - function will abort if threading fails.
 * 
 * @param config Projection configuration
 * @param historical_prices Historical price data (for context)
 * @param num_historical Number of historical prices
 * @return Projection result (caller must free with cllm_price_projection_free_result)
 */
PriceProjectionResult* cllm_price_projection_compute(
    const PriceProjectionConfig* config,
    const double* historical_prices,
    uint32_t num_historical
);

/**
 * Free projection result
 * 
 * @param result Result to free (can be NULL)
 */
void cllm_price_projection_free_result(PriceProjectionResult* result);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Generate triads around a prime number
 * 
 * Generates projection_count triads centered around depth_prime.
 * Each triad is [p1, p2, p3] where p1, p2, p3 are primes.
 * 
 * @param depth_prime Center prime
 * @param projection_count Number of triads to generate
 * @return Array of triads [projection_count][3] (caller must free)
 */
uint32_t* cllm_generate_triads_around_prime(uint32_t depth_prime, uint32_t projection_count);

/**
 * Compute crystalline projection for a single triad
 * 
 * This implements the crystalline lattice projection algorithm using:
 * - CrystallineAbacus for arbitrary precision
 * - 88D threading for parallel computation
 * - Clock lattice operations
 * 
 * @param last_price Last historical price
 * @param depth_prime Depth prime
 * @param triad Triad [3] of primes
 * @param omega_hz Omega frequency in Hz
 * @param steps Number of projection steps
 * @param output Output buffer [steps] (caller allocates)
 * @return true on success, false on error
 */
bool cllm_compute_crystalline_projection(
    double last_price,
    uint32_t depth_prime,
    const uint32_t triad[3],
    double omega_hz,
    uint32_t steps,
    double* output
);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_PRICE_PROJECTION_H */

