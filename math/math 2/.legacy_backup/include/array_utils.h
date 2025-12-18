#ifndef BIGFIXED_ARRAY_UTILS_H
#define BIGFIXED_ARRAY_UTILS_H

#include "abacus_legacy.h"
#include <stddef.h>

/*
 * BigFixed Array Utilities
 * Helper functions for allocating and managing BigFixed arrays
 */

/* Create an array of BigFixed values */
BigFixed** bigfixed_array_create(size_t size, int precision);

/* Free an array of BigFixed values */
void bigfixed_array_free(BigFixed** array, size_t size);

/* Set all elements to zero */
void bigfixed_array_zero(BigFixed** array, size_t size);

/* Copy array */
void bigfixed_array_copy(BigFixed** dest, BigFixed** src, size_t size);

/* Convert between float and BigFixed arrays */
void bigfixed_array_from_float(BigFixed** dest, const float* src, size_t size);
void bigfixed_array_to_float(float* dest, BigFixed** src, size_t size);

#endif /* BIGFIXED_ARRAY_UTILS_H */