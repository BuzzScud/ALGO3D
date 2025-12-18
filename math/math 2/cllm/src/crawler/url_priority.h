#ifndef URL_PRIORITY_H
#define URL_PRIORITY_H

#include "url_database.h"
#include <stdint.h>

/**
 * URL Prioritization System
 * 
 * Features:
 * - Uncrawled page priority boost
 * - Domain diversity scoring
 * - Time-based decay for recently crawled
 * - Depth penalty for deep URLs
 * - Prime-based randomization for diversity
 */

// Priority factors configuration
typedef struct {
    float uncrawled_bonus;      // Bonus for never-crawled URLs (default: 100.0)
    float domain_diversity;     // Bonus for underrepresented domains (default: 50.0)
    float time_decay;           // Decay factor for recently crawled (default: 0.5)
    float depth_penalty;        // Penalty per path level (default: -5.0)
    float prime_randomization;  // Prime-based random factor (default: 10.0)
} PriorityFactors;

// Domain statistics for diversity calculation
typedef struct {
    char domain[256];
    int crawl_count;
    time_t last_crawled;
} DomainStats;

// Priority calculator
typedef struct URLPriority URLPriority;

/**
 * Create priority calculator with factors
 * 
 * @param factors Priority factors (NULL for defaults)
 * @return Priority calculator or NULL on error
 */
URLPriority* url_priority_create(PriorityFactors* factors);

/**
 * Destroy priority calculator
 * 
 * @param priority Priority calculator
 */
void url_priority_destroy(URLPriority* priority);

/**
 * Calculate priority for URL entry
 * 
 * @param priority Priority calculator
 * @param entry URL entry
 * @param total_domains Total number of unique domains
 * @return Priority score (higher = more important)
 */
int url_priority_calculate(URLPriority* priority, URLEntry* entry, int total_domains);

/**
 * Update domain statistics
 * 
 * @param priority Priority calculator
 * @param domain Domain that was crawled
 */
void url_priority_update_domain_stats(URLPriority* priority, const char* domain);

/**
 * Get domain statistics
 * 
 * @param priority Priority calculator
 * @param domain Domain to query
 * @return Domain stats or NULL if not found
 */
DomainStats* url_priority_get_domain_stats(URLPriority* priority, const char* domain);

/**
 * Get prime-based random value
 * 
 * Uses prime number properties for better randomization
 * 
 * @param priority Priority calculator
 * @param seed Seed value (e.g., URL hash)
 * @return Random value based on prime properties
 */
uint64_t url_priority_prime_random(URLPriority* priority, uint64_t seed);

/**
 * Calculate URL depth (number of path segments)
 * 
 * @param url URL to analyze
 * @return Depth (0 for root, 1 for /page, 2 for /dir/page, etc.)
 */
int url_priority_calculate_depth(const char* url);

/**
 * Get priority factors
 * 
 * @param priority Priority calculator
 * @return Priority factors (do not free)
 */
PriorityFactors* url_priority_get_factors(URLPriority* priority);

/**
 * Set priority factors
 * 
 * @param priority Priority calculator
 * @param factors New priority factors
 * @return 0 on success, -1 on error
 */
int url_priority_set_factors(URLPriority* priority, PriorityFactors* factors);

/**
 * Reset domain statistics
 * 
 * @param priority Priority calculator
 */
void url_priority_reset_stats(URLPriority* priority);

/**
 * Get total domains tracked
 * 
 * @param priority Priority calculator
 * @return Number of unique domains
 */
int url_priority_get_domain_count(URLPriority* priority);

#endif // URL_PRIORITY_H