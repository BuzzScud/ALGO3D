// lattice_cache.c - Implementation of Crystalline Lattice Cache
// Uses geometric prime generation and pre-calculates all lattice values

#include "lattice_cache.h"
#include "prime_math_custom.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// External functions from prime_lattice_core.c
extern uint64_t* generate_primes_geometric(uint64_t limit, uint64_t *count);
extern double O_exponent(uint64_t n, int k, const char *lambda_phon);
extern double theta_n(uint64_t n, int k, const char *lambda_phon,
                     uint16_t omega, uint64_t p, uint64_t q, bool use_ratio);
extern double L_lattice(uint64_t n, uint64_t d, int k, const char *lambda_phon,
                       uint16_t omega, uint64_t p, uint64_t q);

// ═══════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════

LatticeCache* lattice_cache_create(uint64_t max_prime) {
    printf("Initializing Crystalline Lattice Cache (max prime: %lu)...\n", max_prime);
    
    LatticeCache* cache = (LatticeCache*)calloc(1, sizeof(LatticeCache));
    if (!cache) {
        fprintf(stderr, "Failed to allocate lattice cache\n");
        return NULL;
    }
    
    // Stage 1: Set foundation (18 seed primes)
    // Note: SMALL_PRIMES has conflicting declarations in headers
    // For now, just note that geometric generation uses them internally
    cache->seed_primes = NULL;
    cache->seed_count = 18;
    
    // Stage 2: Generate primes geometrically
    uint64_t count = 0;
    uint64_t *primes = generate_primes_geometric(max_prime, &count);
    
    if (!primes || count == 0) {
        fprintf(stderr, "Failed to generate primes\n");
        free(cache);
        return NULL;
    }
    
    printf("Generated %lu primes geometrically\n", count);
    
    // Allocate cache entries
    cache->entries = (PrimeLatticeEntry*)calloc(count, sizeof(PrimeLatticeEntry));
    if (!cache->entries) {
        fprintf(stderr, "Failed to allocate cache entries\n");
        free(primes);
        free(cache);
        return NULL;
    }
    
    cache->capacity = count;
    cache->count = 0;
    cache->max_prime = 0;
    cache->auto_extend = false;
    cache->lookups = 0;
    cache->extensions = 0;
    
    // Pre-calculate lattice values for each prime
    printf("Pre-calculating lattice values...\n");
    
    for (uint64_t i = 0; i < count; i++) {
        uint64_t prime = primes[i];
        uint64_t n = i + 1;  // Prime index (1-based)
        
        // Store basic info
        cache->entries[i].prime = prime;
        cache->entries[i].n = n;
        
        // Calculate lattice parameters using library functions
        // Parameters: k=0, λ="dub", ω=432, d=12, p=3, q=4
        cache->entries[i].O = O_exponent(n, 0, "dub");
        cache->entries[i].theta = theta_n(n, 0, "dub", 432, 3, 4, false);
        cache->entries[i].L = L_lattice(n, 12, 0, "dub", 432, 3, 4);
        
        // Calculate position
        cache->entries[i].radius = math_pow(3.0, cache->entries[i].O);
        cache->entries[i].angle = cache->entries[i].theta;
        
        // Möbius twist
        cache->entries[i].mobius = (0 % 2 == 0) ? 1 : -1;  // Γ(k) with k=0
        
        cache->count++;
        cache->max_prime = prime;
        
        // Progress indicator for large caches
        if (i > 0 && i % 10000 == 0) {
            printf("  Processed %lu / %lu primes...\n", i, count);
        }
    }
    
    free(primes);
    
    // Calculate memory usage
    size_t memory_bytes = cache->count * sizeof(PrimeLatticeEntry);
    double memory_mb = memory_bytes / (1024.0 * 1024.0);
    
    printf("Lattice cache initialized:\n");
    printf("  Primes: %lu\n", cache->count);
    printf("  Max prime: %lu\n", cache->max_prime);
    printf("  Memory: %.2f MB\n", memory_mb);
    
    return cache;
}

void lattice_cache_free(LatticeCache* cache) {
    if (!cache) return;
    
    if (cache->entries) {
        free(cache->entries);
    }
    
    free(cache);
}

// ═══════════════════════════════════════════════════════════════
// LOOKUP (O(1) - Lightning Fast!)
// ═══════════════════════════════════════════════════════════════

bool lattice_cache_get_entry(LatticeCache* cache, size_t prime_index, 
                             PrimeLatticeEntry** entry) {
    if (!cache || !entry) return false;
    
    if (prime_index >= cache->count) {
        // Out of range - try auto-extend if enabled
        if (cache->auto_extend) {
            // Extend by 50% or 10000 primes, whichever is larger
            uint64_t extend_by = cache->max_prime / 2;
            if (extend_by < 10000) extend_by = 10000;
            
            uint64_t new_max = cache->max_prime + extend_by;
            printf("Auto-extending cache to %lu...\n", new_max);
            lattice_cache_extend(cache, new_max);
            
            // Check again
            if (prime_index >= cache->count) {
                return false;
            }
        } else {
            return false;
        }
    }
    
    cache->lookups++;
    *entry = &cache->entries[prime_index];
    return true;
}

bool lattice_cache_get_position(LatticeCache* cache, size_t prime_index,
                                double* radius, double* angle) {
    PrimeLatticeEntry* entry = NULL;
    if (!lattice_cache_get_entry(cache, prime_index, &entry)) {
        return false;
    }
    
    if (radius) *radius = entry->radius;
    if (angle) *angle = entry->angle;
    return true;
}

bool lattice_cache_get_prime(LatticeCache* cache, size_t prime_index,
                             uint64_t* prime) {
    PrimeLatticeEntry* entry = NULL;
    if (!lattice_cache_get_entry(cache, prime_index, &entry)) {
        return false;
    }
    
    if (prime) *prime = entry->prime;
    return true;
}

int lattice_cache_find_prime(LatticeCache* cache, uint64_t prime) {
    if (!cache) return -1;
    
    // Binary search
    int left = 0;
    int right = cache->count - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (cache->entries[mid].prime == prime) {
            return mid;
        } else if (cache->entries[mid].prime < prime) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  // Not found
}

// ═══════════════════════════════════════════════════════════════
// ON-DEMAND GENERATION
// ═══════════════════════════════════════════════════════════════

size_t lattice_cache_extend(LatticeCache* cache, uint64_t new_max) {
    if (!cache) return 0;
    if (new_max <= cache->max_prime) return 0;
    
    printf("Extending lattice cache from %lu to %lu...\n", 
           cache->max_prime, new_max);
    
    // Generate new primes
    uint64_t count = 0;
    uint64_t *primes = generate_primes_geometric(new_max, &count);
    
    if (!primes || count <= cache->count) {
        if (primes) free(primes);
        return 0;
    }
    
    size_t new_primes = count - cache->count;
    printf("Found %lu new primes\n", new_primes);
    
    // Reallocate cache
    PrimeLatticeEntry *new_entries = (PrimeLatticeEntry*)realloc(
        cache->entries, count * sizeof(PrimeLatticeEntry));
    
    if (!new_entries) {
        fprintf(stderr, "Failed to extend cache\n");
        free(primes);
        return 0;
    }
    
    cache->entries = new_entries;
    cache->capacity = count;
    
    // Calculate lattice values for new primes
    for (uint64_t i = cache->count; i < count; i++) {
        uint64_t prime = primes[i];
        uint64_t n = i + 1;
        
        cache->entries[i].prime = prime;
        cache->entries[i].n = n;
        cache->entries[i].O = O_exponent(n, 0, "dub");
        cache->entries[i].theta = theta_n(n, 0, "dub", 432, 3, 4, false);
        cache->entries[i].L = L_lattice(n, 12, 0, "dub", 432, 3, 4);
        cache->entries[i].radius = math_pow(3.0, cache->entries[i].O);
        cache->entries[i].angle = cache->entries[i].theta;
        cache->entries[i].mobius = 1;
    }
    
    cache->count = count;
    cache->max_prime = new_max;
    cache->extensions++;
    
    free(primes);
    
    return new_primes;
}

void lattice_cache_set_auto_extend(LatticeCache* cache, bool enable) {
    if (cache) {
        cache->auto_extend = enable;
        printf("Auto-extend %s\n", enable ? "enabled" : "disabled");
    }
}

void lattice_cache_get_stats(LatticeCache* cache, 
                            size_t* count, 
                            uint64_t* max_prime,
                            size_t* memory_bytes) {
    if (!cache) return;
    
    if (count) *count = cache->count;
    if (max_prime) *max_prime = cache->max_prime;
    if (memory_bytes) {
        *memory_bytes = cache->count * sizeof(PrimeLatticeEntry);
    }
}

// ═══════════════════════════════════════════════════════════════
// SPECIAL QUERIES
// ═══════════════════════════════════════════════════════════════

bool lattice_cache_is_near_boundary(LatticeCache* cache, size_t prime_index,
                                   double threshold) {
    if (!cache || prime_index >= cache->count) return false;
    
    uint64_t prime = cache->entries[prime_index].prime;
    
    // Check if near 144000 boundary
    uint64_t boundary = 144000;
    int64_t diff = (int64_t)prime - (int64_t)boundary;
    
    return (diff >= -threshold && diff <= threshold);
}

size_t lattice_cache_get_primes_in_radius_range(LatticeCache* cache,
                                                double min_radius,
                                                double max_radius,
                                                size_t** indices) {
    if (!cache || !indices) return 0;
    
    // Count matching primes
    size_t count = 0;
    for (size_t i = 0; i < cache->count; i++) {
        double r = cache->entries[i].radius;
        if (r >= min_radius && r <= max_radius) {
            count++;
        }
    }
    
    if (count == 0) {
        *indices = NULL;
        return 0;
    }
    
    // Allocate result array
    *indices = (size_t*)malloc(count * sizeof(size_t));
    if (!*indices) return 0;
    
    // Fill result array
    size_t idx = 0;
    for (size_t i = 0; i < cache->count; i++) {
        double r = cache->entries[i].radius;
        if (r >= min_radius && r <= max_radius) {
            (*indices)[idx++] = i;
        }
    }
    
    return count;
}

size_t lattice_cache_get_primes_in_angle_range(LatticeCache* cache,
                                              double min_angle,
                                              double max_angle,
                                              size_t** indices) {
    if (!cache || !indices) return 0;
    
    // Count matching primes
    size_t count = 0;
    for (size_t i = 0; i < cache->count; i++) {
        double a = cache->entries[i].angle;
        if (a >= min_angle && a <= max_angle) {
            count++;
        }
    }
    
    if (count == 0) {
        *indices = NULL;
        return 0;
    }
    
    // Allocate result array
    *indices = (size_t*)malloc(count * sizeof(size_t));
    if (!*indices) return 0;
    
    // Fill result array
    size_t idx = 0;
    for (size_t i = 0; i < cache->count; i++) {
        double a = cache->entries[i].angle;
        if (a >= min_angle && a <= max_angle) {
            (*indices)[idx++] = i;
        }
    }
    
    return count;
}
// Lookup entry by prime value (linear search)
PrimeLatticeEntry* lattice_cache_lookup(LatticeCache* cache, uint64_t prime) {
    if (!cache) return NULL;
    
    // Linear search through cache
    for (size_t i = 0; i < cache->count; i++) {
        if (cache->entries[i].prime == prime) {
            return &cache->entries[i];
        }
    }
    
    return NULL;  // Not found
}
