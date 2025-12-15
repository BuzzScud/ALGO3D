/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file coprime_analysis.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Coprime relationship analysis (OBJECTIVE 28 - Layer 1)
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Computes GCD matrix to analyze coprime relationships between dimensions.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * These relationships reveal structural constraints and enable validation.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery/blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Compute GCD using Euclidean algorithm
 * 
 * @param a First number
 * @param b Second number
 * @return GCD(a, b)
 */
static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Compute coprime relationships (GCD matrix)
 */
uint64_t** compute_coprime_matrix(
    const uint32_t* dimension_sizes,
    uint32_t num_dimensions
) {
    if (!dimension_sizes || num_dimensions == 0) {
        return NULL;
    }
    
    printf("🔢 Computing coprime relationships...\n");
    printf("  Dimensions: %u\n", num_dimensions);
    
    // Allocate matrix
    uint64_t** matrix = malloc(num_dimensions * sizeof(uint64_t*));
    if (!matrix) return NULL;
    
    for (uint32_t i = 0; i < num_dimensions; i++) {
        matrix[i] = malloc(num_dimensions * sizeof(uint64_t));
        if (!matrix[i]) {
            free_coprime_matrix(matrix, i);
            return NULL;
        }
    }
    
    // Compute GCD for each pair
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = 0; j < num_dimensions; j++) {
            if (i == j) {
                matrix[i][j] = dimension_sizes[i];
            } else {
                matrix[i][j] = gcd(dimension_sizes[i], dimension_sizes[j]);
            }
        }
    }
    
    // Print matrix
    printf("  GCD Matrix:\n");
    for (uint32_t i = 0; i < num_dimensions; i++) {
        printf("    ");
        for (uint32_t j = 0; j < num_dimensions; j++) {
            printf("%4lu ", (unsigned long)matrix[i][j]);
        }
        printf("\n");
    }
    
    // Analyze coprime relationships
    uint32_t coprime_pairs = 0;
    for (uint32_t i = 0; i < num_dimensions; i++) {
        for (uint32_t j = i + 1; j < num_dimensions; j++) {
            if (matrix[i][j] == 1) {
                coprime_pairs++;
                printf("  Coprime pair: dim %u (%u) and dim %u (%u)\n",
                       i, dimension_sizes[i], j, dimension_sizes[j]);
            }
        }
    }
    
    printf("  ✓ Found %u coprime pairs\n", coprime_pairs);
    
    return matrix;
}

/**
 * Free coprime matrix
 */
void free_coprime_matrix(uint64_t** matrix, uint32_t num_dimensions) {
    if (!matrix) return;
    
    for (uint32_t i = 0; i < num_dimensions; i++) {
        if (matrix[i]) {
            free(matrix[i]);
        }
    }
    
    free(matrix);
}
