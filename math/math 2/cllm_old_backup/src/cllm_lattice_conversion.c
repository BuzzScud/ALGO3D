/*
 * CLLM Lattice Conversion Utilities
 */

#include "../../include/cllm.h"
#include "../../include/bigfixed_core.h"
#include "../../include/bigint_core.h"
#include "../../include/prime_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/transcendental.h"

void cllm_float_to_bigfixed(BigFixed* output, float* input, int n, int precision) {
    (void)precision; /* Unused parameter - kept for API compatibility */
    if (!output || !input || n <= 0) return;
    
    for (int i = 0; i < n; i++) {
        big_fixed_from_double(&output[i], (double)input[i]);
    }
}

void cllm_bigfixed_to_float(float* output, BigFixed* input, int n) {
    if (!output || !input || n <= 0) return;
    
    for (int i = 0; i < n; i++) {
        output[i] = (float)big_fixed_to_double(&input[i]);
    }
}

void cllm_embeddings_to_basis(BigFixed** basis, float* embeddings, 
                              int n, int dim, int precision) {
    if (!basis || !embeddings || n <= 0 || dim <= 0) return;
    
    for (int i = 0; i < n; i++) {
        cllm_float_to_bigfixed(basis[i], &embeddings[i * dim], dim, precision);
    }
}

void cllm_basis_to_embeddings(float* embeddings, BigFixed** basis,
                              int n, int dim) {
    if (!embeddings || !basis || n <= 0 || dim <= 0) return;
    
    for (int i = 0; i < n; i++) {
        cllm_bigfixed_to_float(&embeddings[i * dim], basis[i], dim);
    }
}

BigFixed** cllm_alloc_bigfixed_basis(int n, int dim) {
    if (n <= 0 || dim <= 0) return NULL;
    
    BigFixed** basis = (BigFixed**)malloc(n * sizeof(BigFixed*));
    if (!basis) return NULL;
    
    for (int i = 0; i < n; i++) {
        basis[i] = (BigFixed*)malloc(dim * sizeof(BigFixed));
        if (!basis[i]) {
            for (int j = 0; j < i; j++) {
                free(basis[j]);
            }
            free(basis);
            return NULL;
        }
        
        for (int d = 0; d < dim; d++) {
            big_fixed_from_int(&basis[i][d], 0);
        }
    }
    
    return basis;
}

void cllm_free_bigfixed_basis(BigFixed** basis, int n) {
    if (!basis) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            free(basis[i]);
        }
    }
    free(basis);
}

void cllm_embedding_to_bigfixed(BigFixed* output, float* embedding,
                                int dim, int precision) {
    cllm_float_to_bigfixed(output, embedding, dim, precision);
}

void cllm_bigfixed_to_embedding(float* embedding, BigFixed* vector, int dim) {
    cllm_bigfixed_to_float(embedding, vector, dim);
}

float cllm_test_conversion_accuracy(float* input, int n, int precision) {
    if (!input || n <= 0) return -1.0;
    
    BigFixed* bigfixed = (BigFixed*)malloc(n * sizeof(BigFixed));
    float* output = (float*)malloc(n * sizeof(float));
    
    if (!bigfixed || !output) {
        if (bigfixed) free(bigfixed);
        if (output) free(output);
        return -1.0;
    }
    
    cllm_float_to_bigfixed(bigfixed, input, n, precision);
    cllm_bigfixed_to_float(output, bigfixed, n);
    
    float max_error = 0.0;
    for (int i = 0; i < n; i++) {
        float error = math_abs(output[i] - input[i]);
        if (error > max_error) {
            max_error = error;
        }
    }
    
    free(bigfixed);
    free(output);
    
    return max_error;
}

void cllm_print_conversion_stats(float* input, int n, int precision) {
    if (!input || n <= 0) return;
    
    float max_error = cllm_test_conversion_accuracy(input, n, precision);
    
    printf("Conversion Statistics:\n");
    printf("  Elements: %d\n", n);
    printf("  Precision: %d bits\n", precision);
    printf("  Max error: %.10e\n", max_error);
    
    if (max_error < 1e-6f) {
        printf("  Status: EXCELLENT (error < 1e-6)\n");
    } else if (max_error < 1e-4f) {
        printf("  Status: GOOD (error < 1e-4)\n");
    } else if (max_error < 1e-2f) {
        printf("  Status: ACCEPTABLE (error < 1e-2)\n");
    } else {
        printf("  Status: WARNING (error >= 1e-2)\n");
    }
}
