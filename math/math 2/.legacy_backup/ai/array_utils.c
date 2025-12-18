/*
 * Crystalline Abacus Array Utilities
 * Helper functions for allocating and managing CrystallineAbacus arrays
 * 
 * MIGRATED FROM: BigFixed array utilities
 * NEW API: Uses CrystallineAbacus from NEW math library
 * 
 * NO BACKWARD COMPATIBILITY - Pure NEW design
 */

#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "math/abacus.h"
#include "math/types.h"
#include "math/clock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Create an array of CrystallineAbacus values
 * Returns: Array of CrystallineAbacus* pointers, each initialized to zero
 */
CrystallineAbacus** abacus_array_create(size_t size, uint32_t base, ClockContext* ctx) {
    if (size == 0) return NULL;
    (void)ctx; // Unused in NEW API
    
    CrystallineAbacus** array = (CrystallineAbacus**)malloc(size * sizeof(CrystallineAbacus*));
    if (!array) return NULL;
    
    // Create each Abacus element
    for (size_t i = 0; i < size; i++) {
        array[i] = abacus_from_uint64(0, base);
        if (!array[i]) {
            // Allocation failed - clean up and return NULL
            for (size_t j = 0; j < i; j++) {
                abacus_free(array[j]);
            }
            free(array);
            return NULL;
        }
    }
    
    return array;
}

/*
 * Free an array of CrystallineAbacus values
 */
void abacus_array_free(CrystallineAbacus** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            abacus_free(array[i]);
        }
    }
    free(array);
}

/*
 * Set all elements of a CrystallineAbacus array to zero
 */
void abacus_array_zero(CrystallineAbacus** array, size_t size) {
    if (!array) return;
    
    for (size_t i = 0; i < size; i++) {
        if (array[i]) {
            // Free old value and create new zero
            uint32_t base = array[i]->base;
            abacus_free(array[i]);
            array[i] = abacus_from_uint64(0, base);
        }
    }
}

/*
 * Copy a CrystallineAbacus array
 */
void abacus_array_copy(CrystallineAbacus** dest, CrystallineAbacus** src, size_t size) {
    if (!dest || !src) {
        fprintf(stderr, "ERROR: abacus_array_copy called with NULL pointer (dest=%p, src=%p)\n", 
                (void*)dest, (void*)src);
        return;
    }
    
    for (size_t i = 0; i < size; i++) {
        if (!dest[i] || !src[i]) {
            // Skip NULL entries but don't crash
            if (i < 10) {  // Only log first 10 to avoid spam
                fprintf(stderr, "WARNING: abacus_array_copy skipping NULL at index %zu (dest[i]=%p, src[i]=%p)\n",
                        i, (void*)dest[i], (void*)src[i]);
            }
            continue;
        }
        // Free old destination and copy source
        abacus_free(dest[i]);
        dest[i] = abacus_copy(src[i]);
    }
}

/*
 * Convert float array to CrystallineAbacus array
 */
void abacus_array_from_float(CrystallineAbacus** dest, const float* src, size_t size, 
                             uint32_t base, int32_t precision, ClockContext* ctx) {
    if (!dest || !src) return;
    (void)ctx; // Unused in NEW API
    
    for (size_t i = 0; i < size; i++) {
        if (dest[i]) {
            abacus_free(dest[i]);
        }
        dest[i] = abacus_from_double((double)src[i], base, precision);
        if (!dest[i]) {
            fprintf(stderr, "WARNING: abacus_array_from_float failed at index %zu\n", i);
        }
    }
}

/*
 * Convert CrystallineAbacus array to float array
 */
void abacus_array_to_float(float* dest, CrystallineAbacus** src, size_t size) {
    if (!dest || !src) return;
    
    for (size_t i = 0; i < size; i++) {
        if (src[i]) {
            // Convert Abacus to double, then to float
            // This is a lossy conversion but necessary for compatibility
            double value;
            MathError err = abacus_to_double(src[i], &value);
            if (err == MATH_SUCCESS) {
                dest[i] = (float)value;
            } else {
                dest[i] = 0.0f;
                fprintf(stderr, "WARNING: abacus_array_to_float failed at index %zu\n", i);
            }
        } else {
            dest[i] = 0.0f;
        }
    }
}