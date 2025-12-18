#ifndef PRIME_RANDOMIZATION_H
#define PRIME_RANDOMIZATION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Prime-Based Randomization for Crawler
 * 
 * Uses prime numbers to create mathematically-based randomization patterns
 * for crawl frequency, delays, and link selection.
 */

/**
 * Prime configuration for crawler
 */
typedef struct {
    uint64_t frequency_prime;      // Prime for crawl frequency (e.g., 7 = every 7 units)
    uint64_t link_selection_prime; // Prime for link selection (e.g., 13 = every 13th link)
    uint64_t delay_min_prime;      // Minimum delay prime (seconds)
    uint64_t delay_max_prime;      // Maximum delay prime (seconds)
    bool use_prime_randomization;  // Enable/disable prime randomization
} CrawlerPrimeConfig;

/**
 * Initialize prime randomization with default values
 * 
 * @param config Output configuration structure
 */
void prime_config_init_default(CrawlerPrimeConfig* config);

/**
 * Calculate delay based on prime numbers
 * 
 * @param min_prime Minimum delay prime
 * @param max_prime Maximum delay prime
 * @param iteration Current iteration number
 * @return Delay in seconds
 */
uint64_t calculate_prime_delay(uint64_t min_prime, uint64_t max_prime, uint64_t iteration);

/**
 * Select link index using prime-based selection
 * 
 * @param selection_prime Prime number for selection
 * @param total_links Total number of links available
 * @param iteration Current iteration number
 * @return Selected link index
 */
int select_prime_link_index(uint64_t selection_prime, int total_links, uint64_t iteration);

/**
 * Check if a number is prime
 * 
 * @param n Number to check
 * @return true if prime, false otherwise
 */
bool is_prime(uint64_t n);

/**
 * Get next prime number after n
 * 
 * @param n Starting number
 * @return Next prime number
 */
uint64_t next_prime(uint64_t n);

/**
 * Get previous prime number before n
 * 
 * @param n Starting number
 * @return Previous prime number
 */
uint64_t prev_prime(uint64_t n);

/**
 * Validate prime configuration
 * 
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
bool prime_config_validate(const CrawlerPrimeConfig* config);

#endif // PRIME_RANDOMIZATION_H