/*
 * Pure Crystalline CLLM - Token Operations
 * 
 * MIGRATED TO NEW MATH LIBRARY: Uses Crystalline Math Library exclusively.
 * Double precision for 3D lattice coordinates (sufficient for geometric operations).
 */

#include "ai/cllm_pure_crystalline.h"
#include "math/constants.h"
#include "math/transcendental.h"  // NEW math library
#include "math/constants.h"
#include "math/types.h"           // MATH_PI constant
#include "math/constants.h"
#include "math/rainbow.h"         // NEW rainbow table
#include "math/constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PRIME_CACHE_SIZE 100000  // Target number of primes to generate

static bool rainbow_table_initialized = false;
static RainbowTable global_rainbow_table = {0};

/*
 * Pure integer square root using Newton's method
 * NO floating point operations
 */
static uint64_t isqrt(uint64_t n) {
    if (n == 0) return 0;
    if (n <= 3) return 1;
    
    // Initial guess
    uint64_t x = n;
    uint64_t y = (x + 1) / 2;
    
    // Newton's method: x_new = (x + n/x) / 2
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    
    return x;
}

// OPTIMIZED: Use NEW rainbow table with crystalline sieve
// Rainbow table is the SINGLE SOURCE OF TRUTH for all primes
// Generates 100,000 primes in ~100ms using fast sieve with 12-fold symmetry
static void init_rainbow_table(void) {
    if (rainbow_table_initialized) return;
    
    printf("Initializing rainbow table (100,000 primes using crystalline sieve)...\n");
    
    MathError err = rainbow_init(&global_rainbow_table, PRIME_CACHE_SIZE);
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to initialize rainbow table\n");
        return;
    }
    
    // Populate with primes
    err = rainbow_populate_count(&global_rainbow_table, PRIME_CACHE_SIZE);
    if (err != MATH_SUCCESS) {
        fprintf(stderr, "ERROR: Failed to populate rainbow table\n");
        return;
    }
    
    rainbow_table_initialized = true;
    
    size_t total = rainbow_size(&global_rainbow_table);
    uint64_t last_prime = 0;
    rainbow_lookup_by_index(&global_rainbow_table, total - 1, &last_prime);
    
    printf("✓ Rainbow table initialized: %zu primes (2 to %lu)\n", 
           total, last_prime);
}


bool crystalline_is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    uint64_t sqrt_n = isqrt(n);
    for (uint64_t i = 3; i <= sqrt_n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

uint64_t crystalline_get_nth_prime(uint32_t n) {
    init_rainbow_table();
    
    // Get prime from rainbow table
    uint64_t prime = 0;
    MathError err = rainbow_lookup_by_index(&global_rainbow_table, n, &prime);
    if (err == MATH_SUCCESS && prime > 0) {
        return prime;
    }
    
    // If not in table, generate more primes
    size_t current_count = rainbow_size(&global_rainbow_table);
    if (n >= current_count) {
        uint64_t needed = (n - current_count) + 1000;  // Generate extra for future use
        err = rainbow_populate_count(&global_rainbow_table, current_count + needed);
        if (err == MATH_SUCCESS) {
            // Try again
            err = rainbow_lookup_by_index(&global_rainbow_table, n, &prime);
            if (err == MATH_SUCCESS && prime > 0) {
                return prime;
            }
        }
    }
    
    // Fallback: use trial division (should rarely happen)
    fprintf(stderr, "WARNING: Rainbow table failed, using fallback for prime %u\n", n);
    uint64_t count = 0;
    uint64_t candidate = 2;
    
    while (count <= n) {
        if (crystalline_is_prime(candidate)) {
            if (count == n) return candidate;
            count++;
        }
        candidate++;
    }
    
    return 0;
}

void crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors) {
    if (!factors || !num_factors) return;
    
    *num_factors = 0;
    
    if (number <= 1) return;
    
    init_rainbow_table();
    
    uint64_t n = number;
    size_t table_count = rainbow_size(&global_rainbow_table);
    
    // Try primes from rainbow table
    for (size_t i = 0; i < table_count && n > 1; i++) {
        uint64_t prime = 0;
        MathError err = rainbow_lookup_by_index(&global_rainbow_table, i, &prime);
        if (err != MATH_SUCCESS || prime == 0) break;
        
        while (n % prime == 0) {
            if (*num_factors < MAX_PRIME_FACTORS) {
                factors[(*num_factors)++] = prime;
            }
            n /= prime;
        }
        
        if (prime * prime > n) break;
    }
    
    // If n > 1, it's a prime factor
    if (n > 1 && *num_factors < MAX_PRIME_FACTORS) {
        factors[(*num_factors)++] = n;
    }
}

void crystalline_compute_ulam_position(uint64_t prime, double coords[3], int precision) {
    if (!coords) return;
    
    (void)precision;  // Unused - kept for API compatibility
    init_rainbow_table();
    
    // Find prime index in rainbow table
    uint64_t prime_index = 0;
    MathError err = rainbow_lookup_index(&global_rainbow_table, prime, &prime_index);
    
    // If prime not in table, estimate index using prime number theorem
    if (err != MATH_SUCCESS || prime_index == 0) {
        if (prime > 2) {
            // π(n) ≈ n / ln(n)
            double ln_prime = math_log((double)prime);
            prime_index = (uint64_t)((double)prime / ln_prime);
            if (prime_index == 0) prime_index = 1;
        }
    }
    
    // Ulam spiral: Vogel's method (golden angle spiral)
    // radius = sqrt(index)
    double radius = math_sqrt((double)prime_index);
    
    // golden_angle = 2π / φ² ≈ 2.39996322972865332 radians
    const double golden_angle = 2.39996322972865332;
    
    // angle = golden_angle * index
    double angle = golden_angle * (double)prime_index;
    
    // Cartesian coordinates
    coords[0] = radius * math_cos(angle);  // x
    coords[1] = radius * math_sin(angle);  // y
    coords[2] = math_log((double)(prime + 1));  // z = ln(prime + 1)
}

CrystallineToken* crystalline_token_create(uint32_t token_id, const char* token_str, uint64_t prime) {
    CrystallineToken* token = (CrystallineToken*)calloc(1, sizeof(CrystallineToken));
    if (!token) return NULL;
    
    token->token_id = token_id;
    
    if (token_str) {
        strncpy(token->token_str, token_str, 63);
        token->token_str[63] = '\0';
    }
    
    token->prime = prime;
    token->is_root = crystalline_is_prime(prime);
    
    crystalline_factorize(prime, token->prime_factors, &token->num_factors);
    
    // Compute 3D lattice coordinates (now using double precision)
    crystalline_compute_ulam_position(prime, token->lattice_coords, 256);
    
    if (token->is_root) {
        token->root_token_id = token_id;
    } else {
        token->root_token_id = 0;
    }
    
    token->num_neighbors = 0;
    token->usage_count = 0;
    token->root_score = 1.0;
    
    return token;
}

void crystalline_token_free(CrystallineToken* token) {
    if (!token) return;
    
    // No cleanup needed for double arrays
    free(token);
}

void crystalline_lattice_distance(const double pos1[3], const double pos2[3], double* distance) {
    if (!pos1 || !pos2 || !distance) return;
    
    // Euclidean distance: sqrt((x1-x2)² + (y1-y2)² + (z1-z2)²)
    double sum_sq = 0.0;
    for (int i = 0; i < 3; i++) {
        double diff = pos1[i] - pos2[i];
        sum_sq += diff * diff;
    }
    
    *distance = math_sqrt(sum_sq);
}

void crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, double* similarity) {
    if (!similarity) return;
    
    init_rainbow_table();
    
    // Get positions for both primes
    double pos1[3], pos2[3];
    crystalline_compute_ulam_position(prime1, pos1, 256);
    crystalline_compute_ulam_position(prime2, pos2, 256);
    
    // Compute distance
    double dist;
    crystalline_lattice_distance(pos1, pos2, &dist);
    
    // Similarity = 1 / (1 + distance)
    *similarity = 1.0 / (1.0 + dist);
}

void crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, double* alignment) {
    if (!alignment) return;
    
    init_rainbow_table();
    
    // Phase alignment based on prime difference and sum
    int64_t diff = (int64_t)prime1 - (int64_t)prime2;
    uint64_t sum_primes = prime1 + prime2;
    
    // Compute phase difference: 2π * |diff| / sum
    double two_pi = 2.0 * MATH_PI;
    double abs_diff = (diff < 0) ? -(double)diff : (double)diff;
    double phase_diff = (two_pi * abs_diff) / (double)sum_primes;
    
    // Alignment = (1 + cos(phase_diff)) / 2
    // This gives 1 for aligned (diff=0), 0 for opposite (diff=sum/2)
    double cos_phase = math_cos(phase_diff);
    *alignment = (1.0 + cos_phase) / 2.0;
}
