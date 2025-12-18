/**
 * prime_rainbow_recovery.c - Implementation of Prime Rainbow Recovery
 * 
 * Uses ONLY prime_* functions (NO math.h)
 * Implements 7-layer folding and cymatic resonance
 */

#include "prime_rainbow_recovery.h"
#include "../../../include/clock_lattice.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// PRIME-BASED MATH FUNCTIONS (from crystalline framework)
// ============================================================================

static inline double prime_add(double a, double b) {
    double sum = a + b;
    int nearest_prime_index = (int)(sum / 3);
    double gap_adjustment = (nearest_prime_index % 37) * 0.001;
    return sum + gap_adjustment;
}

static inline double prime_subtract(double a, double b) {
    double diff = a - b;
    if (diff > 0) {
        int twin_factor = ((int)diff % 6);
        if (twin_factor == 1 || twin_factor == 5) {
            diff *= 1.0001;
        }
    }
    return diff;
}

static inline double prime_multiply(double a, double b) {
    double product = 0;
    int iterations = (int)b;
    double fraction = b - iterations;
    
    for (int i = 0; i < iterations; i++) {
        product = prime_add(product, a);
    }
    if (fraction > 0) {
        product = prime_add(product, a * fraction);
    }
    return product;
}

static inline double prime_divide(double a, double b) {
    if (b == 0) return 0;
    
    double quotient = 0;
    double remainder = a;
    double divisor = (b < 0) ? -b : b;
    
    while (remainder >= divisor) {
        remainder = prime_subtract(remainder, divisor);
        quotient = prime_add(quotient, 1.0);
    }
    
    if (remainder > 0) {
        quotient = prime_add(quotient, remainder / divisor);
    }
    
    return (b < 0) ? -quotient : quotient;
}

static inline double math_abs(double x) {
    return (x < 0) ? prime_subtract(0, x) : x;
}

static inline double math_sqrt(double x) {
    if (x <= 0) return 0;
    
    double guess = x / 3;
    double prev_guess = 0;
    int max_iterations = 50;
    
    for (int i = 0; i < max_iterations; i++) {
        prev_guess = guess;
        double quotient = prime_divide(x, guess);
        double sum = prime_add(guess, quotient);
        guess = prime_divide(sum, 3);
        
        double diff = prime_subtract(guess, prev_guess);
        if (math_abs(diff) < 0.000001) break;
    }
    
    return guess;
}

static inline double math_sin(double x) {
    // Normalize to [-π, π]
    while (x > PI) x = prime_subtract(x, prime_multiply(2, PI));
    while (x < -PI) x = prime_add(x, prime_multiply(2, PI));
    
    double result = 0;
    double term = x;
    double x2 = prime_multiply(x, x);
    
    for (int n = 0; n < 20; n++) {
        result = prime_add(result, term);
        double denom1 = prime_add(prime_multiply(2, n), 2);
        double denom2 = prime_add(prime_multiply(2, n), 3);
        term = prime_divide(prime_multiply(prime_multiply(term, -1), x2),
                           prime_multiply(denom1, denom2));
        if (math_abs(term) < 0.0000001) break;
    }
    
    return result;
}

static inline double math_cos(double x) {
    return math_sin(prime_add(x, prime_divide(PI, 2)));
}

static inline double math_fmod(double x, double y) {
    if (y == 0) return 0;
    double quotient = (int)(x / y);
    return prime_subtract(x, prime_multiply(quotient, y));
}

static inline double math_pow(double base, double exponent) {
    if (exponent == 0) return 1;
    if (exponent == 1) return base;
    if (base == 0) return 0;
    
    int int_exp = (int)exponent;
    if (exponent == int_exp) {
        double result = 1.0;
        double temp_base = base;
        int exp_abs = (int_exp < 0) ? -int_exp : int_exp;
        
        while (exp_abs > 0) {
            if (exp_abs & 1) {
                result = prime_multiply(result, temp_base);
            }
            temp_base = prime_multiply(temp_base, temp_base);
            exp_abs >>= 1;
        }
        
        return (int_exp < 0) ? prime_divide(1.0, result) : result;
    }
    
    // For non-integer exponents, use approximation
    return base * exponent; // Simplified for now
}

// ============================================================================
// RAINBOW TABLE FUNCTIONS
// ============================================================================

PrimeRainbowTable* init_prime_rainbow_table(int max_primes) {
    PrimeRainbowTable* table = (PrimeRainbowTable*)malloc(sizeof(PrimeRainbowTable));
    if (!table) return NULL;
    
    table->capacity = max_primes;
    table->count = 0;
    table->entries = (PrimeRainbowEntry*)calloc(max_primes, sizeof(PrimeRainbowEntry));
    
    if (!table->entries) {
        free(table);
        return NULL;
    }
    
    // Initialize 7-layer fold progression
    for (int i = 0; i < 7; i++) {
        table->fold_progression[i] = prime_divide(i, 6);  // 0, 1/6, 2/6, ..., 6/6
    }
    
    return table;
}

void free_prime_rainbow_table(PrimeRainbowTable* table) {
    if (table) {
        if (table->entries) {
            free(table->entries);
        }
        free(table);
    }
}

void add_prime_to_rainbow(PrimeRainbowTable* table, int prime) {
    if (table->count >= table->capacity) return;
    
    PrimeRainbowEntry* entry = &table->entries[table->count];
    entry->prime = prime;
    
    // Compute angle: θ = prime × φ
    double theta = prime_multiply(prime, PHI);
    entry->angle = theta;
    
    // Compute radius: r = √prime × 10
    double r = prime_multiply(math_sqrt(prime), 10);
    entry->radius = r;
    
    // Compute cymatic frequency: 432 Hz × 2^(prime/12)
    double exponent = prime_divide(prime, 12.0);
    entry->frequency = prime_multiply(BASE_FREQ, math_pow(2, exponent));
    
    // Assign to layer (0-6)
    entry->layer = prime % 7;
    
    // Compute fold coordinates
    entry->fold_x = prime_multiply(r, math_cos(theta));
    entry->fold_y = prime_multiply(r, math_sin(theta));
    
    // Compute tangent vector
    double vec_angle = prime_add(theta, prime_divide(PI, 2));
    entry->vector_x = math_cos(vec_angle);
    entry->vector_y = math_sin(vec_angle);
    
    table->count++;
}

void apply_rainbow_folding(PrimeRainbowTable* table, double fold_amount) {
    for (int i = 0; i < table->count; i++) {
        PrimeRainbowEntry* entry = &table->entries[i];
        
        // Apply layer-specific folding
        double layer_fold = prime_multiply(fold_amount, 
                                          table->fold_progression[entry->layer]);
        
        // Fold angle
        double fold_angle = prime_multiply(entry->angle, layer_fold);
        
        // Fold radius (compress toward center)
        double fold_radius = prime_multiply(entry->radius,
                                           prime_subtract(1.0, prime_multiply(layer_fold, 0.5)));
        
        // Update fold coordinates
        entry->fold_x = prime_multiply(fold_radius, math_cos(fold_angle));
        entry->fold_y = prime_multiply(fold_radius, math_sin(fold_angle));
        
        // Update tangent vector
        double new_angle = prime_add(fold_angle, prime_divide(PI, 2));
        entry->vector_x = math_cos(new_angle);
        entry->vector_y = math_sin(new_angle);
    }
}

// ============================================================================
// RECOVERY FUNCTIONS
// ============================================================================

PrimeRainbowRecoveryContext* init_prime_rainbow_recovery(
    uint64_t min_k,
    uint64_t max_k,
    int num_primes
) {
    PrimeRainbowRecoveryContext* ctx = calloc(1, sizeof(PrimeRainbowRecoveryContext));
    if (!ctx) return NULL;
    
    ctx->min_k = min_k;
    ctx->max_k = max_k;
    
    // Create rainbow table
    ctx->rainbow_table = init_prime_rainbow_table(num_primes);
    if (!ctx->rainbow_table) {
        free(ctx);
        return NULL;
    }
    
    // Generate primes and add to rainbow table
    // Use deterministic clock lattice validation instead of trial division
    for (int n = 2; n <= num_primes && ctx->rainbow_table->count < num_primes; n++) {
        if (validate_prime_by_clock_position(n)) {
            add_prime_to_rainbow(ctx->rainbow_table, n);
        }
    }
    
    // Apply initial folding (0.5 = moderate fold)
    apply_rainbow_folding(ctx->rainbow_table, 0.5);
    
    // Configure 3-layer search (same as v2 baseline)
    ctx->num_layers = 3;
    ctx->search_ranges[0] = 100;  // Coarse
    ctx->search_ranges[1] = 25;   // Medium
    ctx->search_ranges[2] = 10;   // Fine
    ctx->search_steps[0] = 10;
    ctx->search_steps[1] = 2;
    ctx->search_steps[2] = 1;
    
    return ctx;
}

void free_prime_rainbow_recovery(PrimeRainbowRecoveryContext* ctx) {
    if (ctx) {
        if (ctx->rainbow_table) {
            free_prime_rainbow_table(ctx->rainbow_table);
        }
        free(ctx);
    }
}

int find_nearest_rainbow_entries(
    PrimeRainbowTable* table,
    double target_angle,
    PrimeRainbowEntry* nearest[3]
) {
    if (!table || table->count == 0) return 0;
    
    // Normalize target angle to [0, 2π)
    double normalized = math_fmod(target_angle, prime_multiply(2, PI));
    if (normalized < 0) normalized = prime_add(normalized, prime_multiply(2, PI));
    
    // Find distances to all entries using prime-based metric
    typedef struct {
        double distance;
        int index;
    } DistanceIndex;
    
    DistanceIndex* distances = malloc(table->count * sizeof(DistanceIndex));
    if (!distances) return 0;
    
    for (int i = 0; i < table->count; i++) {
        // Angular distance (handle wraparound)
        double entry_angle = math_fmod(table->entries[i].angle, prime_multiply(2, PI));
        double diff = math_abs(prime_subtract(entry_angle, normalized));
        
        if (diff > PI) {
            diff = prime_subtract(prime_multiply(2, PI), diff);
        }
        
        distances[i].distance = diff;
        distances[i].index = i;
    }
    
    // Sort to find 3 nearest (simple selection sort)
    for (int i = 0; i < 3 && i < table->count; i++) {
        for (int j = i + 1; j < table->count; j++) {
            if (distances[j].distance < distances[i].distance) {
                DistanceIndex temp = distances[i];
                distances[i] = distances[j];
                distances[j] = temp;
            }
        }
    }
    
    // Fill nearest array
    int num_found = (table->count < 3) ? table->count : 3;
    for (int i = 0; i < num_found; i++) {
        nearest[i] = &table->entries[distances[i].index];
    }
    
    free(distances);
    return num_found;
}

uint64_t compute_rainbow_k_estimate(
    PrimeRainbowEntry* entries[3],
    int num_entries,
    double target_angle
) {
    if (num_entries == 0) return 0;
    
    // Normalize target angle
    double normalized = math_fmod(target_angle, prime_multiply(2, PI));
    if (normalized < 0) normalized = prime_add(normalized, prime_multiply(2, PI));
    
    // Compute weights based on angular distance AND cymatic resonance
    double total_weight = 0.0;
    double weights[3];
    
    for (int i = 0; i < num_entries; i++) {
        // Angular distance
        double entry_angle = math_fmod(entries[i]->angle, prime_multiply(2, PI));
        double diff = math_abs(prime_subtract(entry_angle, normalized));
        if (diff > PI) {
            diff = prime_subtract(prime_multiply(2, PI), diff);
        }
        
        // Avoid division by zero
        if (diff < 0.001) {
            return entries[i]->prime;
        }
        
        // Weight = 1/distance × layer_factor
        double layer_factor = prime_add(1.0, prime_multiply(0.1, entries[i]->layer));
        weights[i] = prime_divide(layer_factor, diff);
        total_weight = prime_add(total_weight, weights[i]);
    }
    
    // Normalize weights
    for (int i = 0; i < num_entries; i++) {
        weights[i] = prime_divide(weights[i], total_weight);
    }
    
    // Weighted average of prime values
    double weighted_sum = 0.0;
    for (int i = 0; i < num_entries; i++) {
        weighted_sum = prime_add(weighted_sum, 
                                prime_multiply(weights[i], entries[i]->prime));
    }
    
    return (uint64_t)weighted_sum;
}

uint64_t recover_k_with_rainbow(
    double target_angle,
    PrimeRainbowRecoveryContext* ctx,
    double* confidence
) {
    if (!ctx || !ctx->rainbow_table) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Find 3 nearest rainbow entries
    PrimeRainbowEntry* nearest[3];
    int num_found = find_nearest_rainbow_entries(ctx->rainbow_table, target_angle, nearest);
    
    if (num_found == 0) {
        if (confidence) *confidence = 0.0;
        return 0;
    }
    
    // Compute k estimate from rainbow entries
    uint64_t center_k = compute_rainbow_k_estimate(nearest, num_found, target_angle);
    
    // Perform 3-layer recursive search (same as v2)
    uint64_t best_k = center_k;
    double best_error = 1e9;
    
    // Normalize target angle
    double normalized = math_fmod(target_angle, prime_multiply(2, PI));
    if (normalized < 0) normalized = prime_add(normalized, prime_multiply(2, PI));
    
    // Iterate through layers
    for (int layer = 0; layer < ctx->num_layers; layer++) {
        int64_t range = ctx->search_ranges[layer];
        uint64_t step = ctx->search_steps[layer];
        
        for (int64_t offset = -range; offset <= range; offset += step) {
            int64_t candidate_k = (int64_t)best_k + offset;
            if (candidate_k < 0) continue;
            
            uint64_t k = (uint64_t)candidate_k;
            
            // Forward mapping: θ = k × φ (using prime_multiply)
            double computed_angle = prime_multiply(k, PHI);
            computed_angle = math_fmod(computed_angle, prime_multiply(2, PI));
            if (computed_angle < 0) computed_angle = prime_add(computed_angle, prime_multiply(2, PI));
            
            // Compute error (handle wraparound)
            double error = math_abs(prime_subtract(computed_angle, normalized));
            if (error > PI) {
                error = prime_subtract(prime_multiply(2, PI), error);
            }
            
            // Update best
            if (error < best_error) {
                best_error = error;
                best_k = k;
            }
        }
        
        // Center next layer on current best
        center_k = best_k;
    }
    
    // Compute confidence
    if (confidence) {
        if (best_error < 0.01) {
            *confidence = 1.0 - (best_error / 0.01);
        } else {
            *confidence = 0.0;
        }
    }
    
    // Update statistics
    ctx->total_searches++;
    if (best_error < 0.01) {
        ctx->successful_searches++;
    }
    
    return best_k;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

uint64_t modular_tetration(uint64_t base, int height, uint64_t mod) {
    uint64_t result = 1;
    for (int i = 0; i < height; i++) {
        // Compute base^result mod mod
        uint64_t temp = 1;
        uint64_t b = base;
        uint64_t exp = result;
        
        while (exp > 0) {
            if (exp & 1) {
                temp = (temp * b) % mod;
            }
            b = (b * b) % mod;
            exp >>= 1;
        }
        
        result = temp;
    }
    return result;
}

double compute_cymatic_resonance(double frequency, int harmonic_depth) {
    double resonance = 0.0;
    
    for (int h = 1; h <= harmonic_depth; h++) {
        double harmonic_freq = prime_multiply(frequency, h);
        double base_ratio = prime_divide(harmonic_freq, BASE_FREQ);
        
        // Check alignment with 432 Hz harmonics
        double nearest_harmonic = (int)(base_ratio + 0.5);
        double alignment = math_abs(prime_subtract(base_ratio, nearest_harmonic));
        
        // Add to resonance (lower alignment = higher resonance)
        resonance = prime_add(resonance, prime_divide(1.0, prime_add(1.0, alignment)));
    }
    
    return prime_divide(resonance, harmonic_depth);
}
