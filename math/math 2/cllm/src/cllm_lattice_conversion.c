/*
 * CLLM Lattice Conversion Utilities
 * 
 * MIGRATED FROM: BigFixed conversion utilities
 * NEW API: Uses CrystallineAbacus from NEW math library
 * 
 * NO BACKWARD COMPATIBILITY - Pure NEW design
 */

// Include NEW math library FIRST to avoid type conflicts
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/abacus.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Use math_abs from NEW math library
#include "math/arithmetic.h"

void cllm_float_to_abacus(CrystallineAbacus** output, float* input, int n, 
                         uint32_t base, int32_t precision, ClockContext* ctx) {
    if (!output || !input || n <= 0) return;
    (void)ctx; // Unused in NEW API
    
    for (int i = 0; i < n; i++) {
        if (output[i]) {
            abacus_free(output[i]);
        }
        output[i] = abacus_from_double((double)input[i], base, precision);
        if (!output[i]) {
            fprintf(stderr, "WARNING: cllm_float_to_abacus failed at index %d\n", i);
        }
    }
}

void cllm_abacus_to_float(float* output, CrystallineAbacus** input, int n) {
    if (!output || !input || n <= 0) return;
    
    for (int i = 0; i < n; i++) {
        if (input[i]) {
            double value;
            MathError err = abacus_to_double(input[i], &value);
            if (err == MATH_SUCCESS) {
                output[i] = (float)value;
            } else {
                output[i] = 0.0f;
                fprintf(stderr, "WARNING: cllm_abacus_to_float failed at index %d\n", i);
            }
        } else {
            output[i] = 0.0f;
        }
    }
}

void cllm_embeddings_to_basis(CrystallineAbacus*** basis, float* embeddings, 
                              int n, int dim, uint32_t base, int32_t precision, 
                              ClockContext* ctx) {
    if (!basis || !embeddings || n <= 0 || dim <= 0) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            cllm_float_to_abacus(basis[i], &embeddings[i * dim], dim, base, precision, ctx);
        }
    }
}

void cllm_basis_to_embeddings(float* embeddings, CrystallineAbacus*** basis,
                              int n, int dim) {
    if (!embeddings || !basis || n <= 0 || dim <= 0) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            cllm_abacus_to_float(&embeddings[i * dim], basis[i], dim);
        }
    }
}

CrystallineAbacus*** cllm_alloc_abacus_basis(int n, int dim, uint32_t base, ClockContext* ctx) {
    if (n <= 0 || dim <= 0) return NULL;
    (void)ctx; // Unused in NEW API
    
    CrystallineAbacus*** basis = (CrystallineAbacus***)malloc(n * sizeof(CrystallineAbacus**));
    if (!basis) return NULL;
    
    for (int i = 0; i < n; i++) {
        basis[i] = (CrystallineAbacus**)malloc(dim * sizeof(CrystallineAbacus*));
        if (!basis[i]) {
            for (int j = 0; j < i; j++) {
                for (int d = 0; d < dim; d++) {
                    abacus_free(basis[j][d]);
                }
                free(basis[j]);
            }
            free(basis);
            return NULL;
        }
        
        for (int d = 0; d < dim; d++) {
            basis[i][d] = abacus_from_uint64(0, base);
            if (!basis[i][d]) {
                // Cleanup on failure
                for (int dd = 0; dd < d; dd++) {
                    abacus_free(basis[i][dd]);
                }
                for (int j = 0; j < i; j++) {
                    for (int dd = 0; dd < dim; dd++) {
                        abacus_free(basis[j][dd]);
                    }
                    free(basis[j]);
                }
                free(basis[i]);
                free(basis);
                return NULL;
            }
        }
    }
    
    return basis;
}

void cllm_free_abacus_basis(CrystallineAbacus*** basis, int n, int dim) {
    if (!basis) return;
    
    for (int i = 0; i < n; i++) {
        if (basis[i]) {
            for (int d = 0; d < dim; d++) {
                if (basis[i][d]) {
                    abacus_free(basis[i][d]);
                }
            }
            free(basis[i]);
        }
    }
    free(basis);
}

void cllm_embedding_to_abacus(CrystallineAbacus** output, float* embedding,
                              int dim, uint32_t base, int32_t precision, 
                              ClockContext* ctx) {
    cllm_float_to_abacus(output, embedding, dim, base, precision, ctx);
}

void cllm_abacus_to_embedding(float* embedding, CrystallineAbacus** vector, int dim) {
    cllm_abacus_to_float(embedding, vector, dim);
}

float cllm_test_conversion_accuracy(float* input, int n, uint32_t base, 
                                   int32_t precision, ClockContext* ctx) {
    if (!input || n <= 0) return -1.0f;
    (void)ctx; // Unused in NEW API
    
    CrystallineAbacus** abacus = (CrystallineAbacus**)malloc(n * sizeof(CrystallineAbacus*));
    float* output = (float*)malloc(n * sizeof(float));
    
    if (!abacus || !output) {
        if (abacus) free(abacus);
        if (output) free(output);
        return -1.0f;
    }
    
    // Create abacus array
    for (int i = 0; i < n; i++) {
        abacus[i] = abacus_from_uint64(0, base);
        if (!abacus[i]) {
            for (int j = 0; j < i; j++) {
                abacus_free(abacus[j]);
            }
            free(abacus);
            free(output);
            return -1.0f;
        }
    }
    
    cllm_float_to_abacus(abacus, input, n, base, precision, ctx);
    cllm_abacus_to_float(output, abacus, n);
    
    float max_error = 0.0f;
    for (int i = 0; i < n; i++) {
        float error_val = output[i] - input[i];
        float error = (error_val < 0) ? -error_val : error_val;
        if (error > max_error) {
            max_error = error;
        }
    }
    
    // Cleanup
    for (int i = 0; i < n; i++) {
        abacus_free(abacus[i]);
    }
    free(abacus);
    free(output);
    
    return max_error;
}

void cllm_print_conversion_stats(float* input, int n, uint32_t base, 
                                int32_t precision, ClockContext* ctx) {
    if (!input || n <= 0) return;
    
    float max_error = cllm_test_conversion_accuracy(input, n, base, precision, ctx);
    
    printf("Conversion Statistics:\n");
    printf("  Elements: %d\n", n);
    printf("  Base: %u\n", base);
    printf("  Precision: %d (10^%d)\n", precision, precision);
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