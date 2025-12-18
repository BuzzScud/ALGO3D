/**
 * Prime-Based Randomization Implementation
 * 
 * Uses prime numbers for mathematically-based crawl patterns.
 */

#include "prime_randomization.h"
#include "math/clock.h"
#include "math/prime.h"  // For prime_validate_by_clock()
#include <stdlib.h>
#include <time.h>
// #include "ai/prime_float_math.h"  // OBJECTIVE 3A: Using crystalline math only  // REMOVED: Not using any math.h functions

// REMOVED: Local is_prime() implementation
// Internal code trusts the deterministic clock lattice structure
// Use prime_validate_by_clock() directly

/**
 * Get next prime number after n
 */
uint64_t next_prime(uint64_t n) {
    if (n < 2) return 2;
    
    uint64_t candidate = n + 1;
    // Internal: Trust deterministic clock lattice
    while (!prime_validate_by_clock(candidate)) {
        candidate++;
        if (candidate > n + 1000) {
            // Safety limit
            return n;
        }
    }
    
    return candidate;
}

/**
 * Get previous prime number before n
 */
uint64_t prev_prime(uint64_t n) {
    if (n <= 2) return 2;
    
    uint64_t candidate = n - 1;
    // Internal: Trust deterministic clock lattice
    while (candidate > 1 && !prime_validate_by_clock(candidate)) {
        candidate--;
    }
    
    return candidate > 1 ? candidate : 2;
}

/**
 * Initialize prime configuration with default values
 */
void prime_config_init_default(CrawlerPrimeConfig* config) {
    if (!config) return;
    
    config->frequency_prime = 7;        // Crawl every 7 units
    config->link_selection_prime = 13;  // Select every 13th link
    config->delay_min_prime = 3;        // Min delay: 3 seconds
    config->delay_max_prime = 11;       // Max delay: 11 seconds
    config->use_prime_randomization = true;
}

/**
 * Calculate delay based on prime numbers
 */
uint64_t calculate_prime_delay(uint64_t min_prime, uint64_t max_prime, uint64_t iteration) {
    if (min_prime >= max_prime) {
        return min_prime;
    }
    
    // Use iteration to create varying delays
    // Formula: delay = min + ((iteration * min) % (max - min))
    uint64_t range = max_prime - min_prime;
    uint64_t offset = (iteration * min_prime) % range;
    uint64_t delay = min_prime + offset;
    
    // Ensure delay is prime
    // Internal: Trust deterministic clock lattice
    if (!prime_validate_by_clock(delay)) {
        delay = next_prime(delay);
        if (delay > max_prime) {
            delay = prev_prime(max_prime);
        }
    }
    
    return delay;
}

/**
 * Select link index using prime-based selection
 */
int select_prime_link_index(uint64_t selection_prime, int total_links, uint64_t iteration) {
    if (total_links <= 0) return -1;
    if (total_links == 1) return 0;
    
    // Use prime-based selection with iteration
    // Formula: index = (iteration * selection_prime) % total_links
    uint64_t index = (iteration * selection_prime) % (uint64_t)total_links;
    
    return (int)index;
}

/**
 * Validate prime configuration
 */
bool prime_config_validate(const CrawlerPrimeConfig* config) {
    if (!config) return false;
    
    // Check if primes are actually prime
    // Internal: Trust deterministic clock lattice
    if (config->use_prime_randomization) {
        if (!prime_validate_by_clock(config->frequency_prime)) return false;
        if (!prime_validate_by_clock(config->link_selection_prime)) return false;
        if (!prime_validate_by_clock(config->delay_min_prime)) return false;
        if (!prime_validate_by_clock(config->delay_max_prime)) return false;
    }
    
    // Check if min < max
    if (config->delay_min_prime >= config->delay_max_prime) return false;
    
    // Check reasonable ranges
    if (config->frequency_prime < 2 || config->frequency_prime > 1000) return false;
    if (config->link_selection_prime < 2 || config->link_selection_prime > 1000) return false;
    if (config->delay_min_prime < 1 || config->delay_min_prime > 3600) return false;
    if (config->delay_max_prime < 1 || config->delay_max_prime > 3600) return false;
    
    return true;
}