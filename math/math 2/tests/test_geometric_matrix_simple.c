/**
 * @file test_geometric_matrix_simple.c
 * @brief Simple test for geometric matrix implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../algorithms/include/geometric_matrix.h"
#include "math/abacus.h"

int main(void) {
    printf("=== Geometric Matrix Simple Test ===\n\n");
    
    // Test 1: Create a small matrix
    printf("Test 1: Create 10×10 matrix\n");
    GeometricMatrix* matrix = geometric_matrix_create(10, 10, "test_matrix");
    if (!matrix) {
        fprintf(stderr, "FAIL: Could not create matrix\n");
        return 1;
    }
    printf("PASS: Matrix created\n");
    geometric_matrix_print_info(matrix);
    printf("\n");
    
    // Test 2: Initialize with zeros
    printf("Test 2: Initialize with zeros\n");
    if (geometric_matrix_init_zeros(matrix) != 0) {
        fprintf(stderr, "FAIL: Could not initialize with zeros\n");
        geometric_matrix_free(matrix);
        return 1;
    }
    printf("PASS: Initialized with zeros\n\n");
    
    // Test 3: Set a vertex value
    printf("Test 3: Set vertex value\n");
    CrystallineAbacus* value = abacus_from_double(3.14159, 60, 10);
    if (!value) {
        fprintf(stderr, "FAIL: Could not create value\n");
        geometric_matrix_free(matrix);
        return 1;
    }
    
    if (geometric_matrix_set_vertex(matrix, 0, value) != 0) {
        fprintf(stderr, "FAIL: Could not set vertex\n");
        abacus_free(value);
        geometric_matrix_free(matrix);
        return 1;
    }
    printf("PASS: Set vertex 0 to 3.14159\n\n");
    
    // Test 4: Get vertex value back
    printf("Test 4: Get vertex value\n");
    CrystallineAbacus* retrieved = abacus_new(60);
    if (geometric_matrix_get_vertex(matrix, 0, retrieved) != 0) {
        fprintf(stderr, "FAIL: Could not get vertex\n");
        abacus_free(value);
        abacus_free(retrieved);
        geometric_matrix_free(matrix);
        return 1;
    }
    
    double retrieved_val;
    abacus_to_double(retrieved, &retrieved_val);
    printf("PASS: Retrieved value: %.6f\n\n", retrieved_val);
    
    // Test 5: Get interpolated value
    printf("Test 5: Get interpolated value at (5, 5)\n");
    CrystallineAbacus* interpolated = abacus_new(60);
    if (geometric_matrix_get(matrix, 5, 5, interpolated) != 0) {
        fprintf(stderr, "FAIL: Could not get interpolated value\n");
        abacus_free(value);
        abacus_free(retrieved);
        abacus_free(interpolated);
        geometric_matrix_free(matrix);
        return 1;
    }
    
    double interp_val;
    abacus_to_double(interpolated, &interp_val);
    printf("PASS: Interpolated value: %.6f\n\n", interp_val);
    
    // Test 6: Memory usage
    printf("Test 6: Memory usage\n");
    size_t mem = geometric_matrix_memory_usage(matrix);
    printf("Memory usage: %zu bytes (%.2f KB)\n", mem, mem / 1024.0);
    
    // Compare to flat array
    size_t flat_mem = 10 * 10 * sizeof(CrystallineAbacus);
    printf("Flat array would use: %zu bytes (%.2f KB)\n", flat_mem, flat_mem / 1024.0);
    printf("Reduction: %.1fx\n\n", (double)flat_mem / (double)mem);
    
    // Test 7: Larger matrix
    printf("Test 7: Create 128×128 matrix\n");
    GeometricMatrix* large = geometric_matrix_create(128, 128, "large_matrix");
    if (!large) {
        fprintf(stderr, "FAIL: Could not create large matrix\n");
        abacus_free(value);
        abacus_free(retrieved);
        abacus_free(interpolated);
        geometric_matrix_free(matrix);
        return 1;
    }
    
    geometric_matrix_print_info(large);
    
    size_t large_mem = geometric_matrix_memory_usage(large);
    size_t large_flat = 128 * 128 * sizeof(CrystallineAbacus);
    printf("Flat array would use: %zu bytes (%.2f MB)\n", large_flat, large_flat / (1024.0 * 1024.0));
    printf("Geometric uses: %zu bytes (%.2f KB)\n", large_mem, large_mem / 1024.0);
    printf("Reduction: %.1fx\n\n", (double)large_flat / (double)large_mem);
    
    // Cleanup
    abacus_free(value);
    abacus_free(retrieved);
    abacus_free(interpolated);
    geometric_matrix_free(matrix);
    geometric_matrix_free(large);
    
    printf("=== ALL TESTS PASSED ===\n");
    return 0;
}