/**
 * URL Prioritization System Implementation
 * 
 * Uses prime-based randomization and domain diversity
 * to ensure balanced crawling across different sites.
 */

#include "url_priority.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_DOMAINS 10000

// Priority calculator structure
struct URLPriority {
    PriorityFactors factors;
    DomainStats* domain_stats;
    int domain_count;
    int domain_capacity;
    uint64_t random_seed;
};

/**
 * Initialize default factors
 */
static void init_default_factors(PriorityFactors* factors) {
    factors->uncrawled_bonus = 100.0f;
    factors->domain_diversity = 50.0f;
    factors->time_decay = 0.5f;
    factors->depth_penalty = -5.0f;
    factors->prime_randomization = 10.0f;
}

/**
 * Create priority calculator
 */
URLPriority* url_priority_create(PriorityFactors* factors) {
    URLPriority* priority = (URLPriority*)calloc(1, sizeof(URLPriority));
    if (!priority) return NULL;
    
    if (factors) {
        memcpy(&priority->factors, factors, sizeof(PriorityFactors));
    } else {
        init_default_factors(&priority->factors);
    }
    
    // Allocate domain stats array
    priority->domain_capacity = 100;
    priority->domain_stats = (DomainStats*)calloc(priority->domain_capacity, sizeof(DomainStats));
    if (!priority->domain_stats) {
        free(priority);
        return NULL;
    }
    
    priority->domain_count = 0;
    priority->random_seed = (uint64_t)time(NULL);
    
    return priority;
}

/**
 * Destroy priority calculator
 */
void url_priority_destroy(URLPriority* priority) {
    if (!priority) return;
    
    if (priority->domain_stats) {
        free(priority->domain_stats);
    }
    
    free(priority);
}

/**
 * Calculate URL depth
 */
int url_priority_calculate_depth(const char* url) {
    if (!url) return 0;
    
    // Find path start (after domain)
    const char* proto_end = strstr(url, "://");
    if (!proto_end) return 0;
    
    const char* path_start = strchr(proto_end + 3, '/');
    if (!path_start) return 0;
    
    // Count slashes in path (before query string)
    int depth = 0;
    const char* query = strchr(path_start, '?');
    const char* end = query ? query : (url + strlen(url));
    
    for (const char* p = path_start; p < end; p++) {
        if (*p == '/') depth++;
    }
    
    // Subtract 1 because first slash doesn't count as depth
    return depth > 0 ? depth - 1 : 0;
}

/**
 * Get or create domain stats
 */
static DomainStats* get_or_create_domain_stats(URLPriority* priority, const char* domain) {
    if (!priority || !domain) return NULL;
    
    // Search for existing
    for (int i = 0; i < priority->domain_count; i++) {
        if (strcmp(priority->domain_stats[i].domain, domain) == 0) {
            return &priority->domain_stats[i];
        }
    }
    
    // Create new
    if (priority->domain_count >= priority->domain_capacity) {
        // Expand array
        int new_capacity = priority->domain_capacity * 2;
        if (new_capacity > MAX_DOMAINS) new_capacity = MAX_DOMAINS;
        
        DomainStats* new_stats = (DomainStats*)realloc(priority->domain_stats, 
                                                       new_capacity * sizeof(DomainStats));
        if (!new_stats) return NULL;
        
        priority->domain_stats = new_stats;
        priority->domain_capacity = new_capacity;
    }
    
    if (priority->domain_count >= MAX_DOMAINS) {
        return NULL;  // Too many domains
    }
    
    // Initialize new entry
    DomainStats* stats = &priority->domain_stats[priority->domain_count];
    strncpy(stats->domain, domain, sizeof(stats->domain) - 1);
    stats->domain[sizeof(stats->domain) - 1] = '\0';
    stats->crawl_count = 0;
    stats->last_crawled = 0;
    
    priority->domain_count++;
    return stats;
}

/**
 * Prime-based random number generator
 * 
 * Uses prime number properties for better distribution
 */
uint64_t url_priority_prime_random(URLPriority* priority, uint64_t seed) {
    if (!priority) return 0;
    
    // Use prime numbers for better randomization
    // This creates a more uniform distribution than standard rand()
    const uint64_t prime1 = 2654435761ULL;  // Large prime
    const uint64_t prime2 = 2246822519ULL;  // Another large prime
    const uint64_t prime3 = 3266489917ULL;  // Yet another large prime
    
    // Mix seed with internal state
    uint64_t x = seed ^ priority->random_seed;
    
    // Apply prime-based transformations
    x = (x ^ (x >> 33)) * prime1;
    x = (x ^ (x >> 33)) * prime2;
    x = (x ^ (x >> 33)) * prime3;
    
    // Update internal state
    priority->random_seed = x;
    
    return x;
}

/**
 * Calculate priority for URL entry
 */
int url_priority_calculate(URLPriority* priority, URLEntry* entry, int total_domains) {
    if (!priority || !entry) return 0;
    
    float score = 0.0f;
    
    // 1. Uncrawled bonus
    if (entry->crawl_count == 0) {
        score += priority->factors.uncrawled_bonus;
    }
    
    // 2. Domain diversity bonus
    DomainStats* stats = get_or_create_domain_stats(priority, entry->domain);
    if (stats && total_domains > 0) {
        // Give bonus to underrepresented domains
        float avg_crawls = (float)stats->crawl_count / total_domains;
        float diversity_score = (1.0f - avg_crawls) * priority->factors.domain_diversity;
        score += diversity_score;
    }
    
    // 3. Time decay for recently crawled
    if (entry->last_crawled > 0) {
        time_t now = time(NULL);
        time_t age = now - entry->last_crawled;
        
        // Decay: older = higher priority
        // Use exponential decay: e^(-decay * age_in_days)
        float age_days = (float)age / (24.0f * 3600.0f);
        float decay_factor = 1.0f - math_exp(-priority->factors.time_decay * age_days);
        score += decay_factor * 50.0f;  // Max 50 points from age
    } else {
        // Never crawled - full age bonus
        score += 50.0f;
    }
    
    // 4. Depth penalty
    int depth = url_priority_calculate_depth(entry->url);
    score += depth * priority->factors.depth_penalty;
    
    // 5. Prime-based randomization
    // Use URL as seed for consistent but random-looking priorities
    uint64_t url_hash = 0;
    for (const char* p = entry->url; *p; p++) {
        url_hash = url_hash * 31 + (uint64_t)(*p);
    }
    
    uint64_t random_val = url_priority_prime_random(priority, url_hash);
    float random_factor = (float)(random_val % 1000) / 1000.0f;  // 0.0 to 1.0
    score += random_factor * priority->factors.prime_randomization;
    
    // Convert to integer priority
    return (int)score;
}

/**
 * Update domain statistics
 */
void url_priority_update_domain_stats(URLPriority* priority, const char* domain) {
    if (!priority || !domain) return;
    
    DomainStats* stats = get_or_create_domain_stats(priority, domain);
    if (stats) {
        stats->crawl_count++;
        stats->last_crawled = time(NULL);
    }
}

/**
 * Get domain statistics
 */
DomainStats* url_priority_get_domain_stats(URLPriority* priority, const char* domain) {
    if (!priority || !domain) return NULL;
    
    for (int i = 0; i < priority->domain_count; i++) {
        if (strcmp(priority->domain_stats[i].domain, domain) == 0) {
            return &priority->domain_stats[i];
        }
    }
    
    return NULL;
}

/**
 * Get priority factors
 */
PriorityFactors* url_priority_get_factors(URLPriority* priority) {
    if (!priority) return NULL;
    return &priority->factors;
}

/**
 * Set priority factors
 */
int url_priority_set_factors(URLPriority* priority, PriorityFactors* factors) {
    if (!priority || !factors) return -1;
    
    memcpy(&priority->factors, factors, sizeof(PriorityFactors));
    return 0;
}

/**
 * Reset domain statistics
 */
void url_priority_reset_stats(URLPriority* priority) {
    if (!priority) return;
    
    priority->domain_count = 0;
    memset(priority->domain_stats, 0, priority->domain_capacity * sizeof(DomainStats));
}

/**
 * Get total domains tracked
 */
int url_priority_get_domain_count(URLPriority* priority) {
    if (!priority) return 0;
    return priority->domain_count;
}