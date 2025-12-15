/**
 * @file qk_mapping.c
 * @brief Implementation of Q→k mapping for general inference
 * 
 * PHASE 1 WEEK 2: Verified clean - no OLD library dependencies
 * - No prime_* function calls
 * - Removed prime_math_custom.h include (not needed)
 * - Uses only standard C library
 */

#include "qk_mapping.h"
#include "tensor_ops.h"
#include "math/types.h"
#include <stdlib.h>
#include <string.h>

QtoK_Mapping* qk_mapper_create(size_t Q_dim, size_t num_candidates) {
    if (Q_dim == 0 || num_candidates == 0) {
        return NULL;
    }
    
    QtoK_Mapping* mapper = (QtoK_Mapping*)calloc(1, sizeof(QtoK_Mapping));
    if (!mapper) {
        return NULL;
    }
    
    mapper->Q_dim = Q_dim;
    mapper->num_candidates = num_candidates;
    mapper->p_mod = 13; // Default prime modulus
    
    mapper->Q_vector = (double*)calloc(Q_dim, sizeof(double));
    mapper->k_candidates = (uint64_t*)calloc(num_candidates, sizeof(uint64_t));
    mapper->distances = (double*)calloc(num_candidates, sizeof(double));
    
    if (!mapper->Q_vector || !mapper->k_candidates || !mapper->distances) {
        qk_mapper_destroy(mapper);
        return NULL;
    }
    
    return mapper;
}

void qk_mapper_destroy(QtoK_Mapping* mapper) {
    if (!mapper) {
        return;
    }
    
    free(mapper->Q_vector);
    free(mapper->k_candidates);
    free(mapper->distances);
    free(mapper);
}

uint64_t qk_find_best(const QtoK_Mapping* mapper,
                      const double* Q_query) {
    if (!mapper || !Q_query) {
        return 0;
    }
    
    double min_distance = 1e308; // Large value
    uint64_t best_k = 0;
    
    for (size_t i = 0; i < mapper->num_candidates; i++) {
        // Create candidate Q vector from k
        // Map k to N-D space using modular arithmetic
        double* Q_candidate = (double*)malloc(mapper->Q_dim * sizeof(double));
        if (!Q_candidate) continue;
        
        uint64_t k = mapper->k_candidates[i];
        for (size_t d = 0; d < mapper->Q_dim; d++) {
            Q_candidate[d] = (double)(k % 1000000);
            k /= 1000000;
        }
        
        // Calculate tensor distance using tensor_ops
        double dist = tensor_distance(Q_query, Q_candidate,
                                      mapper->Q_dim, mapper->p_mod);
        
        free(Q_candidate);
        
        // Track minimum
        if (dist < min_distance) {
            min_distance = dist;
            best_k = mapper->k_candidates[i];
        }
    }
    
    return best_k;
}

size_t qk_find_top_n(const QtoK_Mapping* mapper,
                     const double* Q_query,
                     uint64_t* top_k, size_t N) {
    if (!mapper || !Q_query || !top_k || N == 0) {
        return 0;
    }
    
    // Calculate distances for all candidates
    for (size_t i = 0; i < mapper->num_candidates; i++) {
        double* Q_candidate = (double*)malloc(mapper->Q_dim * sizeof(double));
        if (!Q_candidate) {
            mapper->distances[i] = 1e308;
            continue;
        }
        
        uint64_t k = mapper->k_candidates[i];
        for (size_t d = 0; d < mapper->Q_dim; d++) {
            Q_candidate[d] = (double)(k % 1000000);
            k /= 1000000;
        }
        
        mapper->distances[i] = tensor_distance(Q_query, Q_candidate,
                                               mapper->Q_dim,
                                               mapper->p_mod);
        
        free(Q_candidate);
    }
    
    // Find top N with smallest distances (simple selection sort)
    size_t found = (N < mapper->num_candidates) ? N : mapper->num_candidates;
    
    // Create temporary arrays for sorting
    double* temp_distances = (double*)malloc(mapper->num_candidates * sizeof(double));
    uint64_t* temp_candidates = (uint64_t*)malloc(mapper->num_candidates * sizeof(uint64_t));
    
    if (!temp_distances || !temp_candidates) {
        free(temp_distances);
        free(temp_candidates);
        return 0;
    }
    
    memcpy(temp_distances, mapper->distances, mapper->num_candidates * sizeof(double));
    memcpy(temp_candidates, mapper->k_candidates, mapper->num_candidates * sizeof(uint64_t));
    
    for (size_t i = 0; i < found; i++) {
        size_t min_idx = i;
        double min_dist = temp_distances[i];
        
        for (size_t j = i + 1; j < mapper->num_candidates; j++) {
            if (temp_distances[j] < min_dist) {
                min_dist = temp_distances[j];
                min_idx = j;
            }
        }
        
        // Swap
        if (min_idx != i) {
            double temp_dist = temp_distances[i];
            temp_distances[i] = temp_distances[min_idx];
            temp_distances[min_idx] = temp_dist;
            
            uint64_t temp_k = temp_candidates[i];
            temp_candidates[i] = temp_candidates[min_idx];
            temp_candidates[min_idx] = temp_k;
        }
        
        top_k[i] = temp_candidates[i];
    }
    
    free(temp_distances);
    free(temp_candidates);
    
    return found;
}

bool qk_set_candidates(QtoK_Mapping* mapper,
                       const uint64_t* candidates,
                       size_t num_candidates) {
    if (!mapper || !candidates || num_candidates == 0) {
        return false;
    }
    
    if (num_candidates > mapper->num_candidates) {
        // Reallocate if needed
        uint64_t* new_candidates = (uint64_t*)realloc(mapper->k_candidates,
                                                       num_candidates * sizeof(uint64_t));
        double* new_distances = (double*)realloc(mapper->distances,
                                                  num_candidates * sizeof(double));
        
        if (!new_candidates || !new_distances) {
            return false;
        }
        
        mapper->k_candidates = new_candidates;
        mapper->distances = new_distances;
        mapper->num_candidates = num_candidates;
    }
    
    memcpy(mapper->k_candidates, candidates, num_candidates * sizeof(uint64_t));
    return true;
}
