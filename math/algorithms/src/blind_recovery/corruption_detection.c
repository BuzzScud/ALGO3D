/**
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * @file corruption_detection.c
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * @brief Corruption detection (OBJECTIVE 28 - Layer 1)
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * 
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 * Detects corrupted elements in structure data.
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
 */
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf

#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "blind_recovery/blind_recovery.h"
#include "math/types.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/transcendental.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include "math/arithmetic.h"
#include "math/validation.h"  // For math_is_nan, math_is_inf
#include <stdlib.h>
#include <stdio.h>

/**
 * Detect corrupted elements in structure
 */
bool* detect_corruption(
    const double* structure_data,
    uint32_t num_elements,
    double threshold
) {
    if (!structure_data || num_elements == 0) {
        return NULL;
    }
    
    printf("🔍 Detecting corruption...\n");
    printf("  Elements: %u\n", num_elements);
    printf("  Threshold: %.4f\n", threshold);
    
    // Allocate corruption mask
    bool* mask = calloc(num_elements, sizeof(bool));
    if (!mask) return NULL;
    
    uint32_t corrupted_count = 0;
    
    // Detect corruption
    for (uint32_t i = 0; i < num_elements; i++) {
        double val = structure_data[i];
        
        // Check for NaN or Inf
        if (math_is_nan(val) || math_is_inf(val)) {
            mask[i] = true;
            corrupted_count++;
            continue;
        }
        
        // Check for extreme values
        if (math_abs(val) > threshold) {
            mask[i] = true;
            corrupted_count++;
        }
    }
    
    double corruption_rate = (double)corrupted_count / (double)num_elements;
    
    printf("  Corrupted elements: %u (%.1f%%)\n",
           corrupted_count, corruption_rate * 100.0);
    
    if (corruption_rate > 0.25) {
        printf("  ⚠️  High corruption rate (>25%%)\n");
    }
    
    printf("  ✓ Corruption detection complete\n");
    
    return mask;
}
