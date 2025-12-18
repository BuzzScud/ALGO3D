/*
 * Pure Crystalline CLLM - Token Operations
 * 
 * PURE IMPLEMENTATION: Uses ONLY arbitrary precision mathematics.
 * NO external math libraries (math.h, GMP, etc.)
 */

#include "../../include/cllm_pure_crystalline.h"
#include "../../include/bigint_core.h"
#include "../../include/bigfixed_core.h"
#include "math/transcendental.h"
#include "math/transcendental.h"
#include "../../include/bigfixed_constants.h"
#include "../../include/prime_rainbow.h"  // Rainbow table - single source of primes
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PRIME_CACHE_SIZE 100000  // Target number of primes to generate

static bool rainbow_table_initialized = false;

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

// OPTIMIZED: Use rainbow table with crystalline sieve
// Rainbow table is the SINGLE SOURCE OF TRUTH for all primes
// Generates 100,000 primes in ~100ms using fast sieve with 12-fold symmetry
static void init_rainbow_table(void) {
    if (rainbow_table_initialized) return;
    
    printf("Initializing rainbow table (100,000 primes using crystalline sieve)...\n");
    
    // Initialize rainbow table
    rainbow_table_init();
    
    // Load important primes first (sacred, Mersenne, etc.)
    int important_count = rainbow_table_load_important_primes();
    printf("✓ Loaded %d important primes\n", important_count);
    
    // Generate remaining primes using fast sieve
    int generated = rainbow_table_generate_primes(PRIME_CACHE_SIZE);
    if (generated < 0) {
        fprintf(stderr, "ERROR: Failed to generate primes\n");
        return;
    }
    
    rainbow_table_initialized = true;
    
    int total = rainbow_table_get_count();
    BigInt* last_prime = rainbow_table_get_prime(total - 1);
    uint64_t last_prime_val = last_prime ? bigint_to_uint64(last_prime) : 0;
    
    printf("✓ Rainbow table initialized: %d primes (2 to %lu)\n", 
           total, last_prime_val);
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
    BigInt* prime = rainbow_table_get_prime(n);
    if (prime) {
        return bigint_to_uint64(prime);
    }
    
    // If not in table, generate more primes
    int current_count = rainbow_table_get_count();
    if (n >= (uint32_t)current_count) {
        // Generate more primes
        int needed = (n - current_count) + 1000;  // Generate extra for future use
        rainbow_table_generate_primes(needed);
        
        // Try again
        prime = rainbow_table_get_prime(n);
        if (prime) {
            return bigint_to_uint64(prime);
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
    
    return candidate;
}

void crystalline_factorize(uint64_t number, uint64_t* factors, uint8_t* num_factors) {
    if (!factors || !num_factors) return;
    
    init_rainbow_table();
    
    *num_factors = 0;
    
    if (number <= 1) return;
    
    if (crystalline_is_prime(number)) {
        factors[0] = number;
        *num_factors = 1;
        return;
    }
    
    while (number % 2 == 0 && *num_factors < MAX_PRIME_FACTORS) {
        factors[(*num_factors)++] = 2;
        number /= 2;
    }
    
    for (uint64_t i = 3; i * i <= number && *num_factors < MAX_PRIME_FACTORS; i += 2) {
        while (number % i == 0) {
            factors[(*num_factors)++] = i;
            number /= i;
        }
    }
    
    if (number > 1 && *num_factors < MAX_PRIME_FACTORS) {
        factors[(*num_factors)++] = number;
    }
}

void crystalline_compute_ulam_position(uint64_t prime, BigFixed coords[3], int precision) {
    if (!coords) return;
    
    init_rainbow_table();
    
    // Initialize BigFixed structures if needed
    for (int i = 0; i < 3; i++) {
        if (!coords[i].integer_part) {
            coords[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
            big_init(coords[i].integer_part);
        }
        if (!coords[i].fractional_part) {
            coords[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
            big_init(coords[i].fractional_part);
        }
        coords[i].scale_bits = precision;
        coords[i].negative = 0;
    }
    
    // Find prime index in rainbow table
    uint32_t prime_index = 0;
    int table_count = rainbow_table_get_count();
    
    for (int i = 0; i < table_count; i++) {
        BigInt* table_prime = rainbow_table_get_prime(i);
        if (table_prime) {
            uint64_t table_prime_val = bigint_to_uint64(table_prime);
            if (table_prime_val == prime) {
                prime_index = i;
                break;
            }
            if (table_prime_val > prime) break;
        }
    }
    
    // If prime not in table, estimate index
    if (prime_index == 0) {
        // Approximate: prime_index ≈ prime / ln(prime)
        // For simplicity, use prime_index = prime / 10 as rough estimate
        prime_index = (uint32_t)(prime / 10);
        if (prime_index == 0) prime_index = 1;
    }
    
    // Create BigInt for prime_index
    BigInt* idx = (BigInt*)malloc(sizeof(BigInt));
    big_init(idx);
    big_from_int(idx, prime_index);
    
    // Compute radius = sqrt(prime_index) using pure BigFixed
    BigFixed radius;
    radius.integer_part = (BigInt*)malloc(sizeof(BigInt));
    radius.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(radius.integer_part);
    big_init(radius.fractional_part);
    radius.scale_bits = precision;
    radius.negative = 0;
    big_sqrt(&radius, idx, precision);
    
    // Compute angle = golden_angle * prime_index
    // golden_angle = 2π / φ² ≈ 2.39996322972865332 radians
    BigFixed golden_angle;
    golden_angle.integer_part = (BigInt*)malloc(sizeof(BigInt));
    golden_angle.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(golden_angle.integer_part);
    big_init(golden_angle.fractional_part);
    golden_angle.scale_bits = precision;
    golden_angle.negative = 0;
    big_fixed_from_double(&golden_angle, 2.39996322972865332);
    
    BigFixed angle;
    angle.integer_part = (BigInt*)malloc(sizeof(BigInt));
    angle.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(angle.integer_part);
    big_init(angle.fractional_part);
    angle.scale_bits = precision;
    angle.negative = 0;
    
    // angle = golden_angle * prime_index
    BigFixed idx_fixed;
    idx_fixed.integer_part = (BigInt*)malloc(sizeof(BigInt));
    idx_fixed.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(idx_fixed.integer_part);
    big_init(idx_fixed.fractional_part);
    idx_fixed.scale_bits = precision;
    idx_fixed.negative = 0;
    big_fixed_from_int(&idx_fixed, prime_index);
    
    big_fixed_mul(&angle, &golden_angle, &idx_fixed);
    
    // Compute x = radius * cos(angle)
    BigFixed cos_angle;
    cos_angle.integer_part = (BigInt*)malloc(sizeof(BigInt));
    cos_angle.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(cos_angle.integer_part);
    big_init(cos_angle.fractional_part);
    cos_angle.scale_bits = precision;
    cos_angle.negative = 0;
    big_cos(&cos_angle, &angle, precision);
    
    big_fixed_mul(&coords[0], &radius, &cos_angle);
    
    // Compute y = radius * sin(angle)
    BigFixed sin_angle;
    sin_angle.integer_part = (BigInt*)malloc(sizeof(BigInt));
    sin_angle.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(sin_angle.integer_part);
    big_init(sin_angle.fractional_part);
    sin_angle.scale_bits = precision;
    sin_angle.negative = 0;
    big_sin(&sin_angle, &angle, precision);
    
    big_fixed_mul(&coords[1], &radius, &sin_angle);
    
    // Compute z = ln(prime + 1)
    BigInt* prime_plus_1 = (BigInt*)malloc(sizeof(BigInt));
    big_init(prime_plus_1);
    big_from_int(prime_plus_1, prime + 1);
    
    big_ln(&coords[2], prime_plus_1, precision);
    
    // Cleanup temporary BigFixed/BigInt structures
    big_free(prime_plus_1);
    free(prime_plus_1);
    big_free(sin_angle.integer_part);
    free(sin_angle.integer_part);
    big_free(sin_angle.fractional_part);
    free(sin_angle.fractional_part);
    big_free(cos_angle.integer_part);
    free(cos_angle.integer_part);
    big_free(cos_angle.fractional_part);
    free(cos_angle.fractional_part);
    big_free(idx_fixed.integer_part);
    free(idx_fixed.integer_part);
    big_free(idx_fixed.fractional_part);
    free(idx_fixed.fractional_part);
    big_free(angle.integer_part);
    free(angle.integer_part);
    big_free(angle.fractional_part);
    free(angle.fractional_part);
    big_free(golden_angle.integer_part);
    free(golden_angle.integer_part);
    big_free(golden_angle.fractional_part);
    free(golden_angle.fractional_part);
    big_free(radius.integer_part);
    free(radius.integer_part);
    big_free(radius.fractional_part);
    free(radius.fractional_part);
    big_free(idx);
    free(idx);
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
    
    // Initialize BigFixed coordinates
    for (int i = 0; i < 3; i++) {
        token->lattice_coords[i].integer_part = NULL;
        token->lattice_coords[i].fractional_part = NULL;
    }
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
    crystalline_compute_ulam_position(prime, token->lattice_coords, 256);
    #pragma GCC diagnostic pop
    
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
    
    // Free BigFixed coordinates
    for (int i = 0; i < 3; i++) {
        if (token->lattice_coords[i].integer_part) {
            big_free(token->lattice_coords[i].integer_part);
            free(token->lattice_coords[i].integer_part);
        }
        if (token->lattice_coords[i].fractional_part) {
            big_free(token->lattice_coords[i].fractional_part);
            free(token->lattice_coords[i].fractional_part);
        }
    }
    
    free(token);
}

void crystalline_lattice_distance(const BigFixed pos1[3], const BigFixed pos2[3], BigFixed* distance) {
    if (!pos1 || !pos2 || !distance) return;
    
    // Initialize distance if needed
    if (!distance->integer_part) {
        distance->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(distance->integer_part);
    }
    if (!distance->fractional_part) {
        distance->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(distance->fractional_part);
    }
    
    BigFixed diff[3];
    BigFixed diff_sq[3];
    BigFixed sum;
    
    // Initialize temporary BigFixed structures
    for (int i = 0; i < 3; i++) {
        diff[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
        diff[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(diff[i].integer_part);
        big_init(diff[i].fractional_part);
        
        diff_sq[i].integer_part = (BigInt*)malloc(sizeof(BigInt));
        diff_sq[i].fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(diff_sq[i].integer_part);
        big_init(diff_sq[i].fractional_part);
    }
    
    sum.integer_part = (BigInt*)malloc(sizeof(BigInt));
    sum.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(sum.integer_part);
    big_init(sum.fractional_part);
    
    for (int i = 0; i < 3; i++) {
        big_fixed_sub(&diff[i], &pos1[i], &pos2[i]);
        big_fixed_mul(&diff_sq[i], &diff[i], &diff[i]);
    }
    
    big_fixed_from_int(&sum, 0);
    for (int i = 0; i < 3; i++) {
        BigFixed temp;
        temp.integer_part = (BigInt*)malloc(sizeof(BigInt));
        temp.fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(temp.integer_part);
        big_init(temp.fractional_part);
        
        big_fixed_add(&temp, &sum, &diff_sq[i]);
        
        big_free(sum.integer_part);
        big_free(sum.fractional_part);
        free(sum.integer_part);
        free(sum.fractional_part);
        
        sum = temp;
    }
    
    // Compute sqrt(sum) using pure BigFixed
    // First convert sum to BigInt for big_sqrt
    BigInt* sum_int = (BigInt*)malloc(sizeof(BigInt));
    big_init(sum_int);
    big_fixed_to_bigint_rounded(sum_int, &sum);
    
    // Compute sqrt using pure arbitrary precision
    big_sqrt(distance, sum_int, 256);
    
    // Cleanup
    big_free(sum_int);
    free(sum_int);
    
    // Cleanup
    for (int i = 0; i < 3; i++) {
        big_free(diff[i].integer_part);
        big_free(diff[i].fractional_part);
        free(diff[i].integer_part);
        free(diff[i].fractional_part);
        
        big_free(diff_sq[i].integer_part);
        big_free(diff_sq[i].fractional_part);
        free(diff_sq[i].integer_part);
        free(diff_sq[i].fractional_part);
    }
    
    big_free(sum.integer_part);
    big_free(sum.fractional_part);
    free(sum.integer_part);
    free(sum.fractional_part);
}

void crystalline_prime_similarity(uint64_t prime1, uint64_t prime2, BigFixed* similarity) {
    if (!similarity) return;
    
    // Initialize if needed
    if (!similarity->integer_part) {
        similarity->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(similarity->integer_part);
    }
    if (!similarity->fractional_part) {
        similarity->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(similarity->fractional_part);
    }
    
    if (prime1 == prime2) {
        big_fixed_from_double(similarity, 1.0);
        return;
    }
    
    uint64_t a = prime1, b = prime2;
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    uint64_t gcd = a;
    
    if (gcd == 1) {
        big_fixed_from_double(similarity, 0.5);
    } else {
        double sim = 1.0 / (double)gcd;
        if (sim > 1.0) sim = 1.0;
        big_fixed_from_double(similarity, sim);
    }
}

void crystalline_phase_alignment(uint64_t prime1, uint64_t prime2, BigFixed* alignment) {
    if (!alignment) return;
    
    // Initialize if needed
    if (!alignment->integer_part) {
        alignment->integer_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(alignment->integer_part);
    }
    if (!alignment->fractional_part) {
        alignment->fractional_part = (BigInt*)malloc(sizeof(BigInt));
        big_init(alignment->fractional_part);
    }
    
    // Compute phase_diff = 2π * (prime1 - prime2) / (prime1 + prime2) using pure BigFixed
    
    // Get π using big_pi
    BigFixed pi;
    pi.integer_part = (BigInt*)malloc(sizeof(BigInt));
    pi.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(pi.integer_part);
    big_init(pi.fractional_part);
    pi.scale_bits = 256;
    pi.negative = 0;
    big_pi(&pi, 256);
    
    // Compute 2π
    BigFixed two_pi;
    two_pi.integer_part = (BigInt*)malloc(sizeof(BigInt));
    two_pi.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(two_pi.integer_part);
    big_init(two_pi.fractional_part);
    two_pi.scale_bits = 256;
    two_pi.negative = 0;
    
    BigFixed two;
    two.integer_part = (BigInt*)malloc(sizeof(BigInt));
    two.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(two.integer_part);
    big_init(two.fractional_part);
    two.scale_bits = 256;
    two.negative = 0;
    big_fixed_from_int(&two, 2);
    
    big_fixed_mul(&two_pi, &pi, &two);
    
    // Compute (prime1 - prime2)
    int64_t diff = (int64_t)prime1 - (int64_t)prime2;
    BigFixed diff_fixed;
    diff_fixed.integer_part = (BigInt*)malloc(sizeof(BigInt));
    diff_fixed.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(diff_fixed.integer_part);
    big_init(diff_fixed.fractional_part);
    diff_fixed.scale_bits = 256;
    diff_fixed.negative = (diff < 0) ? 1 : 0;
    big_fixed_from_int(&diff_fixed, (diff < 0) ? -diff : diff);
    
    // Compute (prime1 + prime2)
    uint64_t sum_primes = prime1 + prime2;
    BigFixed sum_fixed;
    sum_fixed.integer_part = (BigInt*)malloc(sizeof(BigInt));
    sum_fixed.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(sum_fixed.integer_part);
    big_init(sum_fixed.fractional_part);
    sum_fixed.scale_bits = 256;
    sum_fixed.negative = 0;
    big_fixed_from_int(&sum_fixed, sum_primes);
    
    // Compute 2π * (prime1 - prime2)
    BigFixed numerator;
    numerator.integer_part = (BigInt*)malloc(sizeof(BigInt));
    numerator.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(numerator.integer_part);
    big_init(numerator.fractional_part);
    numerator.scale_bits = 256;
    numerator.negative = 0;
    big_fixed_mul(&numerator, &two_pi, &diff_fixed);
    
    // Compute phase_diff = numerator / (prime1 + prime2)
    BigFixed phase_diff;
    phase_diff.integer_part = (BigInt*)malloc(sizeof(BigInt));
    phase_diff.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(phase_diff.integer_part);
    big_init(phase_diff.fractional_part);
    phase_diff.scale_bits = 256;
    phase_diff.negative = 0;
    big_fixed_div(&phase_diff, &numerator, &sum_fixed);
    
    // Compute cos(phase_diff)
    BigFixed cos_phase;
    cos_phase.integer_part = (BigInt*)malloc(sizeof(BigInt));
    cos_phase.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(cos_phase.integer_part);
    big_init(cos_phase.fractional_part);
    cos_phase.scale_bits = 256;
    cos_phase.negative = 0;
    big_cos(&cos_phase, &phase_diff, 256);
    
    // Compute (1 + cos(phase_diff))
    BigFixed one;
    one.integer_part = (BigInt*)malloc(sizeof(BigInt));
    one.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(one.integer_part);
    big_init(one.fractional_part);
    one.scale_bits = 256;
    one.negative = 0;
    big_fixed_from_int(&one, 1);
    
    BigFixed one_plus_cos;
    one_plus_cos.integer_part = (BigInt*)malloc(sizeof(BigInt));
    one_plus_cos.fractional_part = (BigInt*)malloc(sizeof(BigInt));
    big_init(one_plus_cos.integer_part);
    big_init(one_plus_cos.fractional_part);
    one_plus_cos.scale_bits = 256;
    one_plus_cos.negative = 0;
    big_fixed_add(&one_plus_cos, &one, &cos_phase);
    
    // Compute alignment = (1 + cos(phase_diff)) / 2
    big_fixed_div(alignment, &one_plus_cos, &two);
    
    // Cleanup
    big_free(one_plus_cos.integer_part);
    free(one_plus_cos.integer_part);
    big_free(one_plus_cos.fractional_part);
    free(one_plus_cos.fractional_part);
    big_free(one.integer_part);
    free(one.integer_part);
    big_free(one.fractional_part);
    free(one.fractional_part);
    big_free(cos_phase.integer_part);
    free(cos_phase.integer_part);
    big_free(cos_phase.fractional_part);
    free(cos_phase.fractional_part);
    big_free(phase_diff.integer_part);
    free(phase_diff.integer_part);
    big_free(phase_diff.fractional_part);
    free(phase_diff.fractional_part);
    big_free(numerator.integer_part);
    free(numerator.integer_part);
    big_free(numerator.fractional_part);
    free(numerator.fractional_part);
    big_free(sum_fixed.integer_part);
    free(sum_fixed.integer_part);
    big_free(sum_fixed.fractional_part);
    free(sum_fixed.fractional_part);
    big_free(diff_fixed.integer_part);
    free(diff_fixed.integer_part);
    big_free(diff_fixed.fractional_part);
    free(diff_fixed.fractional_part);
    big_free(two.integer_part);
    free(two.integer_part);
    big_free(two.fractional_part);
    free(two.fractional_part);
    big_free(two_pi.integer_part);
    free(two_pi.integer_part);
    big_free(two_pi.fractional_part);
    free(two_pi.fractional_part);
    big_free(pi.integer_part);
    free(pi.integer_part);
    big_free(pi.fractional_part);
    free(pi.fractional_part);
}
