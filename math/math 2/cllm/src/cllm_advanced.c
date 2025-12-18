#include "math/types.h"
/**
 * Advanced Crystalline Features
 * 
 * Leverages full crystalline lattice structure:
 * 1. CVP (Closest Vector Problem) for token lookup
 * 2. SVP (Shortest Vector Problem) for optimal embeddings
 * 3. Prime factorization caching for faster GCD
 * 4. Ulam spiral spatial indexing for cache optimization
 */

#include "cllm_training.h"
#include "cllm_pure_crystalline.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/types.h"  // For MATH_INFINITY

#ifndef MATH_INFINITY
#endif

// Prime factorization cache entry
typedef struct {
    uint32_t number;
    uint32_t* factors;
    int num_factors;
} PrimeFactorization;

// Prime factorization cache
typedef struct {
    PrimeFactorization* cache;
    int capacity;
    int size;
} PrimeFactorCache;

// Spatial index for Ulam spiral
typedef struct {
    uint32_t token_id;
    float x, y, z;  // Ulam spiral coordinates
} SpatialToken;

typedef struct {
    SpatialToken* tokens;
    int num_tokens;
    int* spatial_index;  // Sorted by spatial proximity
} UlamSpatialIndex;

/**
 * Create prime factor cache
 */
static PrimeFactorCache* create_prime_factor_cache(int capacity) {
    PrimeFactorCache* cache = (PrimeFactorCache*)malloc(sizeof(PrimeFactorCache));
    cache->cache = (PrimeFactorization*)calloc(capacity, sizeof(PrimeFactorization));
    cache->capacity = capacity;
    cache->size = 0;
    return cache;
}

/**
 * Free prime factor cache
 */
static void free_prime_factor_cache(PrimeFactorCache* cache) {
    if (!cache) return;
    for (int i = 0; i < cache->size; i++) {
        free(cache->cache[i].factors);
    }
    free(cache->cache);
    free(cache);
}

/**
 * Factorize number into primes
 */
static void factorize_number(uint32_t n, uint32_t** factors, int* num_factors) {
    if (n <= 1) {
        *factors = NULL;
        *num_factors = 0;
        return;
    }
    
    // Allocate max possible factors
    uint32_t* temp = (uint32_t*)malloc(32 * sizeof(uint32_t));
    int count = 0;
    
    // Factor out 2s
    while (n % 2 == 0) {
        temp[count++] = 2;
        n /= 2;
    }
    
    // Factor out odd primes
    for (uint32_t i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            temp[count++] = i;
            n /= i;
        }
    }
    
    // If n is still > 1, it's a prime
    if (n > 1) {
        temp[count++] = n;
    }
    
    // Allocate exact size
    *factors = (uint32_t*)malloc(count * sizeof(uint32_t));
    memcpy(*factors, temp, count * sizeof(uint32_t));
    *num_factors = count;
    free(temp);
}

/**
 * Get or compute prime factorization (with caching)
 */
static PrimeFactorization* get_prime_factorization(PrimeFactorCache* cache, uint32_t n) {
    // Check cache
    for (int i = 0; i < cache->size; i++) {
        if (cache->cache[i].number == n) {
            return &cache->cache[i];
        }
    }
    
    // Not in cache, compute it
    if (cache->size >= cache->capacity) {
        // Cache full, replace oldest entry
        free(cache->cache[0].factors);
        memmove(&cache->cache[0], &cache->cache[1], 
                (cache->size - 1) * sizeof(PrimeFactorization));
        cache->size--;
    }
    
    PrimeFactorization* entry = &cache->cache[cache->size++];
    entry->number = n;
    factorize_number(n, &entry->factors, &entry->num_factors);
    
    return entry;
}

/**
 * Fast GCD using cached prime factorizations
 */
static uint32_t fast_gcd_cached(PrimeFactorCache* cache, uint32_t a, uint32_t b) {
    if (a == 0) return b;
    if (b == 0) return a;
    
    // Get factorizations
    PrimeFactorization* fa = get_prime_factorization(cache, a);
    PrimeFactorization* fb = get_prime_factorization(cache, b);
    
    // Find common factors
    uint32_t gcd = 1;
    int i = 0, j = 0;
    
    while (i < fa->num_factors && j < fb->num_factors) {
        if (fa->factors[i] == fb->factors[j]) {
            gcd *= fa->factors[i];
            i++;
            j++;
        } else if (fa->factors[i] < fb->factors[j]) {
            i++;
        } else {
            j++;
        }
    }
    
    return gcd;
}

/**
 * Compute Ulam spiral position
 */
static void compute_ulam_position(uint32_t token_id, float* x, float* y, float* z) {
    if (token_id == 0) {
        *x = *y = *z = 0.0;
        return;
    }
    
    float golden_angle = 2.39996322972865332f;  // 2π/φ²
    float radius = math_sqrt((double)(float)token_id);
    float angle = (float)token_id * golden_angle;
    
    *x = radius * math_cos((double)angle);
    *y = radius * math_sin((double)angle);
    *z = math_log((double)(float)token_id + 1.0);
}

/**
 * Create Ulam spatial index
 */
static UlamSpatialIndex* create_ulam_spatial_index(uint32_t vocab_size) {
    UlamSpatialIndex* index = (UlamSpatialIndex*)malloc(sizeof(UlamSpatialIndex));
    index->num_tokens = vocab_size;
    index->tokens = (SpatialToken*)malloc(vocab_size * sizeof(SpatialToken));
    index->spatial_index = (int*)malloc(vocab_size * sizeof(int));
    
    // Compute positions
    for (uint32_t i = 0; i < vocab_size; i++) {
        index->tokens[i].token_id = i;
        compute_ulam_position(i, &index->tokens[i].x, 
                            &index->tokens[i].y, &index->tokens[i].z);
        index->spatial_index[i] = i;
    }
    
    return index;
}

/**
 * Free Ulam spatial index
 */
static void free_ulam_spatial_index(UlamSpatialIndex* index) {
    if (!index) return;
    free(index->tokens);
    free(index->spatial_index);
    free(index);
}

/**
 * Find spatially close tokens (for cache optimization)
 */
static int* find_nearby_tokens(UlamSpatialIndex* index, uint32_t token_id, 
                               int k, float* distances) {
    if (token_id >= (uint32_t)index->num_tokens) return NULL;
    
    SpatialToken* target = &index->tokens[token_id];
    int* nearby = (int*)malloc(k * sizeof(int));
    
    // Compute distances to all tokens
    float* all_distances = (float*)malloc(index->num_tokens * sizeof(float));
    for (int i = 0; i < index->num_tokens; i++) {
        float dx = index->tokens[i].x - target->x;
        float dy = index->tokens[i].y - target->y;
        float dz = index->tokens[i].z - target->z;
        all_distances[i] = math_sqrt((double)dx*dx + dy*dy + dz*dz);
    }
    
    // Find k nearest (simple selection, could use heap)
    for (int i = 0; i < k; i++) {
        int min_idx = 0;
        float min_dist = MATH_INFINITY;
        for (int j = 0; j < index->num_tokens; j++) {
            if (all_distances[j] < min_dist) {
                // Check if already selected
                int already_selected = 0;
                for (int m = 0; m < i; m++) {
                    if (nearby[m] == j) {
                        already_selected = 1;
                        break;
                    }
                }
                if (!already_selected) {
                    min_dist = all_distances[j];
                    min_idx = j;
                }
            }
        }
        nearby[i] = min_idx;
        if (distances) distances[i] = min_dist;
    }
    
    free(all_distances);
    return nearby;
}

/**
 * CVP (Closest Vector Problem) for token lookup
 * 
 * Given an embedding vector, find the closest token in the lattice.
 * This is more accurate than simple dot product similarity.
 */
uint32_t cvp_find_closest_token(CLLMModel* model, const float* query_embedding) {
    if (!model || !query_embedding) return 0;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    
    // Get embedding helper function
    extern double* cllm_get_embedding_ptr(CLLMModel* model, uint32_t token_id);
    
    uint32_t closest_token = 0;
    float min_distance = MATH_INFINITY;
    
    // Find token with minimum Euclidean distance
    for (uint32_t v = 0; v < vocab_size; v++) {
        // Get embedding from thread storage
        double* token_embedding = cllm_get_embedding_ptr(model, v);
        if (!token_embedding) continue;
        
        float distance = 0.0;
        for (uint32_t d = 0; d < embed_dim; d++) {
            float diff = query_embedding[d] - token_embedding[d];
            distance += diff * diff;
        }
        
        if (distance < min_distance) {
            min_distance = distance;
            closest_token = v;
        }
    }
    
    return closest_token;
}

/**
 * SVP (Shortest Vector Problem) for optimal embeddings
 * 
 * Find the shortest non-zero vector in the embedding lattice.
 * This can be used to optimize embedding initialization.
 */
float* svp_find_shortest_vector(CLLMModel* model) {
    if (!model) return NULL;
    
    uint32_t vocab_size = model->vocab_size;
    uint32_t embed_dim = model->embedding_dim;
    
    // Get embedding helper function
    extern double* cllm_get_embedding_ptr(CLLMModel* model, uint32_t token_id);
    
    float* shortest = (float*)malloc(embed_dim * sizeof(float));
    float min_length = MATH_INFINITY;
    
    // Find embedding with minimum length
    for (uint32_t v = 0; v < vocab_size; v++) {
        // Get embedding from thread storage
        double* token_embedding = cllm_get_embedding_ptr(model, v);
        if (!token_embedding) continue;
        
        float length = 0.0;
        for (uint32_t d = 0; d < embed_dim; d++) {
            float val = token_embedding[d];
            length += val * val;
        }
        length = math_sqrt((double)length);
        
        if (length > 1e-6f && length < min_length) {
            min_length = length;
            for (uint32_t d = 0; d < embed_dim; d++) {
                shortest[d] = (float)token_embedding[d];
            }
        }
    }
    
    return shortest;
}

/**
 * Advanced crystalline training state
 */
typedef struct {
    PrimeFactorCache* factor_cache;
    UlamSpatialIndex* spatial_index;
    int use_cvp;
    int use_cached_gcd;
} CrystallineAdvancedState;

/**
 * Create advanced crystalline state
 */
CrystallineAdvancedState* crystalline_advanced_create(CLLMModel* model) {
    CrystallineAdvancedState* state = (CrystallineAdvancedState*)malloc(sizeof(CrystallineAdvancedState));
    
    // Create prime factor cache (cache up to 10000 factorizations)
    state->factor_cache = create_prime_factor_cache(10000);
    
    // Create spatial index
    state->spatial_index = create_ulam_spatial_index(model->vocab_size);
    
    state->use_cvp = 1;
    state->use_cached_gcd = 1;
    
    printf("Advanced crystalline features initialized\n");
    printf("- Prime factor cache: 10000 entries\n");
    printf("- Ulam spatial index: %lu tokens\n", (unsigned long)model->vocab_size);
    
    return state;
}

/**
 * Free advanced crystalline state
 */
void crystalline_advanced_free(CrystallineAdvancedState* state) {
    if (!state) return;
    free_prime_factor_cache(state->factor_cache);
    free_ulam_spatial_index(state->spatial_index);
    free(state);
}

/**
 * Compute similarity using advanced features
 */
float crystalline_advanced_similarity(CrystallineAdvancedState* state, 
                                     uint32_t token1, uint32_t token2) {
    if (!state || token1 == 0 || token2 == 0) return 0.0;
    
    // Use cached GCD if enabled
    if (state->use_cached_gcd) {
        uint32_t shared = fast_gcd_cached(state->factor_cache, token1, token2);
        uint32_t max_val = token1 > token2 ? token1 : token2;
        return (float)shared / (float)max_val;
    }
    
    // Fallback to standard GCD
    uint32_t a = token1, b = token2;
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    uint32_t max_val = token1 > token2 ? token1 : token2;
    return (float)a / (float)max_val;
}

/**
 * Prefetch spatially nearby tokens (cache optimization)
 */
void crystalline_prefetch_nearby(CrystallineAdvancedState* state, 
                                CLLMModel* model, uint32_t token_id, int k) {
    if (!state || !model || token_id >= model->vocab_size) return;
    
    // Find k nearest tokens
    int* nearby = find_nearby_tokens(state->spatial_index, token_id, k, NULL);
    
    // Get embedding helper function
    extern double* cllm_get_embedding_ptr(CLLMModel* model, uint32_t token_id);
    
    // Prefetch their embeddings (hint to CPU cache)
    for (int i = 0; i < k; i++) {
        uint32_t nearby_token = nearby[i];
        if (nearby_token < model->vocab_size) {
            double* embedding = cllm_get_embedding_ptr(model, nearby_token);
            if (embedding) {
                __builtin_prefetch(embedding, 0, 3);  // Prefetch for read, high temporal locality
            }
        }
    }
    
    free(nearby);
}