/**
 * @file symbolic_field_theory.c
 * @brief Implementation of Symbolic Field Theory (SFT) for prime discovery
 * 
 * PHASE 1 WEEK 2: Verified clean - no OLD library dependencies
 * - No prime_* function calls (only variable names: prime_count, prime_simple, prime_map)
 * - Removed prime_math_custom.h include (not needed)
 * - Uses only standard C library
 * - Ready for Phase 2 integration with clock lattice
 */

#include "symbolic_field_theory.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Simple primality test using trial division
 * 
 * Tests if a number is prime by checking divisibility up to sqrt(n).
 * Sufficient for the ranges we're working with in SFT.
 */
static bool is_prime_simple(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    // Check divisibility by numbers of form 6k ± 1 up to sqrt(n)
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Simple factorization to count distinct prime factors
 * 
 * This is a lightweight factorization that only counts distinct factors,
 * not their multiplicities. Optimized for ω(x) calculation.
 */
static uint32_t count_distinct_factors(uint64_t n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    
    uint32_t count = 0;
    
    // Check for factor of 2
    if (n % 2 == 0) {
        count++;
        while (n % 2 == 0) n /= 2;
    }
    
    // Check odd factors up to sqrt(n)
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            count++;
            while (n % i == 0) n /= i;
        }
    }
    
    // If n > 1, then it's a prime factor
    if (n > 1) count++;
    
    return count;
}

uint32_t sft_omega(uint64_t x) {
    return count_distinct_factors(x);
}

int64_t sft_psi_hybrid(uint64_t x) {
    // ψ(x) = (x mod 6) - ω(x)
    int64_t mod6 = (int64_t)(x % 6);
    int64_t omega = (int64_t)sft_omega(x);
    return mod6 - omega;
}

uint64_t sft_kappa(uint64_t x, int64_t psi) {
    // κ(x) = ⌊(x - ψ)² / x⌋
    if (x == 0) return 0;
    
    int64_t diff = (int64_t)x - psi;
    
    // Handle negative differences
    uint64_t diff_sq;
    if (diff < 0) {
        diff_sq = (uint64_t)((-diff) * (-diff));
    } else {
        diff_sq = (uint64_t)(diff * diff);
    }
    
    return diff_sq / x;
}

size_t sft_find_collapse_zones(uint64_t start, uint64_t end,
                                CollapseZone* zones, size_t max_zones) {
    if (start >= end || zones == NULL || max_zones == 0) {
        return 0;
    }
    
    size_t zone_count = 0;
    
    // Calculate initial values
    int64_t prev_psi = sft_psi_hybrid(start);
    uint64_t prev_kappa = sft_kappa(start, prev_psi);
    
    int64_t curr_psi = sft_psi_hybrid(start + 1);
    uint64_t curr_kappa = sft_kappa(start + 1, curr_psi);
    
    bool in_zone = false;
    uint64_t zone_start = 0;
    uint64_t min_kappa = UINT64_MAX;
    uint64_t min_candidate = 0;
    
    // Scan range looking for local minima
    for (uint64_t x = start + 2; x <= end && zone_count < max_zones; x++) {
        int64_t next_psi = sft_psi_hybrid(x);
        uint64_t next_kappa = sft_kappa(x, next_psi);
        
        // Check for local minimum (curr < prev AND curr < next)
        if (curr_kappa < prev_kappa && curr_kappa < next_kappa) {
            if (!in_zone) {
                // Start new collapse zone
                zone_start = x - 1;
                min_kappa = curr_kappa;
                min_candidate = x - 1;
                in_zone = true;
            } else {
                // Update minimum in current zone
                if (curr_kappa < min_kappa) {
                    min_kappa = curr_kappa;
                    min_candidate = x - 1;
                }
            }
        } else if (in_zone && curr_kappa > prev_kappa) {
            // End of collapse zone (kappa increasing again)
            zones[zone_count].start = zone_start;
            zones[zone_count].end = x - 1;
            zones[zone_count].min_kappa = min_kappa;
            zones[zone_count].candidate = min_candidate;
            zone_count++;
            
            in_zone = false;
            min_kappa = UINT64_MAX;
        }
        
        // Shift values
        prev_psi = curr_psi;
        prev_kappa = curr_kappa;
        curr_psi = next_psi;
        curr_kappa = next_kappa;
    }
    
    // Handle zone that extends to end of range
    if (in_zone && zone_count < max_zones) {
        zones[zone_count].start = zone_start;
        zones[zone_count].end = end;
        zones[zone_count].min_kappa = min_kappa;
        zones[zone_count].candidate = min_candidate;
        zone_count++;
    }
    
    return zone_count;
}

size_t sft_deterministic_prime_map(uint64_t start, uint64_t end,
                                    uint64_t* primes, size_t max_primes) {
    if (start >= end || primes == NULL || max_primes == 0) {
        return 0;
    }
    
    // Allocate temporary storage for collapse zones
    // Estimate: roughly 1 zone per 10 numbers
    size_t max_zones = (end - start) / 10 + 100;
    if (max_zones > 10000) max_zones = 10000; // Cap at reasonable size
    
    CollapseZone* zones = (CollapseZone*)malloc(max_zones * sizeof(CollapseZone));
    if (zones == NULL) {
        return 0;
    }
    
    // Find collapse zones
    size_t zone_count = sft_find_collapse_zones(start, end, zones, max_zones);
    
    // Test candidates at zone minima for primality
    size_t prime_count = 0;
    for (size_t i = 0; i < zone_count && prime_count < max_primes; i++) {
        uint64_t candidate = zones[i].candidate;
        
        // Skip if candidate is out of range
        if (candidate < start || candidate > end) {
            continue;
        }
        
        // Skip even numbers (except 2)
        if (candidate > 2 && candidate % 2 == 0) {
            continue;
        }
        
        // Test primality using simple trial division
        if (is_prime_simple(candidate)) {
            primes[prime_count++] = candidate;
        }
        
        // Also test nearby numbers in the collapse zone
        // (sometimes the minimum is off by 1-2)
        for (int64_t offset = -2; offset <= 2; offset++) {
            if (offset == 0) continue; // Already tested
            
            int64_t test = (int64_t)candidate + offset;
            if (test < (int64_t)start || test > (int64_t)end) continue;
            if (test <= 1) continue;
            if (test > 2 && test % 2 == 0) continue;
            
            if (prime_count >= max_primes) break;
            
            if (is_prime_simple((uint64_t)test)) {
                primes[prime_count++] = (uint64_t)test;
            }
        }
    }
    
    free(zones);
    
    // Sort primes (they may be out of order due to offset testing)
    // Simple insertion sort (efficient for small arrays)
    for (size_t i = 1; i < prime_count; i++) {
        uint64_t key = primes[i];
        size_t j = i;
        while (j > 0 && primes[j - 1] > key) {
            primes[j] = primes[j - 1];
            j--;
        }
        primes[j] = key;
    }
    
    // Remove duplicates
    if (prime_count > 0) {
        size_t write = 1;
        for (size_t read = 1; read < prime_count; read++) {
            if (primes[read] != primes[read - 1]) {
                primes[write++] = primes[read];
            }
        }
        prime_count = write;
    }
    
    return prime_count;
}

bool sft_is_in_collapse_zone(uint64_t x, uint32_t window_size) {
    if (window_size == 0) window_size = 5; // Default window
    
    // Calculate kappa for x and surrounding points
    int64_t psi_x = sft_psi_hybrid(x);
    uint64_t kappa_x = sft_kappa(x, psi_x);
    
    // Check if x is a local minimum within window
    uint32_t lower_count = 0;
    uint32_t total_count = 0;
    
    for (uint32_t offset = 1; offset <= window_size; offset++) {
        // Check left
        if (x >= offset) {
            uint64_t left = x - offset;
            int64_t psi_left = sft_psi_hybrid(left);
            uint64_t kappa_left = sft_kappa(left, psi_left);
            if (kappa_x < kappa_left) lower_count++;
            total_count++;
        }
        
        // Check right
        uint64_t right = x + offset;
        int64_t psi_right = sft_psi_hybrid(right);
        uint64_t kappa_right = sft_kappa(right, psi_right);
        if (kappa_x < kappa_right) lower_count++;
        total_count++;
    }
    
    // Consider it a collapse zone if kappa_x is lower than most neighbors
    return (lower_count * 2 > total_count);
}

int64_t sft_kappa_gradient(uint64_t x) {
    if (x == 0) return 0;
    
    // Calculate κ(x) and κ(x+1)
    int64_t psi_x = sft_psi_hybrid(x);
    uint64_t kappa_x = sft_kappa(x, psi_x);
    
    int64_t psi_next = sft_psi_hybrid(x + 1);
    uint64_t kappa_next = sft_kappa(x + 1, psi_next);
    
    // Gradient = κ(x+1) - κ(x)
    return (int64_t)kappa_next - (int64_t)kappa_x;
}
