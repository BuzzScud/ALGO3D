/**
 * @file geometric_number_babylon.c
 * @brief Pure Babylonian 4-Ring Number System
 * 
 * Implements the EXACT Babylonian clock lattice structure:
 * - Ring 0: 12 positions (hours)
 * - Ring 1: 60 positions (minutes)
 * - Ring 2: 60 positions (seconds)
 * - Ring 3: 100 positions (milliseconds)
 * 
 * Total precision: 12 × 60 × 60 × 100 = 4,320,000
 * 
 * All operations are PURE O(1) - exactly 4 ring operations.
 */

#include "math/geometric_number.h"
#include <stdlib.h>

/* Ring sizes (Babylonian structure) */
#define RING0_SIZE 12   /* Hours */
#define RING1_SIZE 60   /* Minutes */
#define RING2_SIZE 60   /* Seconds */
#define RING3_SIZE 100  /* Milliseconds */

/* Maximum value that fits in Babylonian system */
#define BABYLON_MAX (RING0_SIZE * RING1_SIZE * RING2_SIZE * RING3_SIZE)

/* ============================================================================
 * BABYLONIAN NUMBER IMPLEMENTATION
 * ============================================================================
 */

BabylonianNumber babylon_from_int(int32_t value) {
    BabylonianNumber result = {0};
    
    result.negative = (value < 0);
    uint32_t abs_value = (uint32_t)(result.negative ? -value : value);
    
    /* Check bounds */
    if (abs_value >= BABYLON_MAX) {
        /* Overflow: wrap around (modular arithmetic) */
        abs_value = abs_value % BABYLON_MAX;
    }
    
    /* Decompose into 4 rings (pure geometric decomposition) */
    result.ring3 = abs_value % RING3_SIZE;
    abs_value /= RING3_SIZE;
    
    result.ring2 = abs_value % RING2_SIZE;
    abs_value /= RING2_SIZE;
    
    result.ring1 = abs_value % RING1_SIZE;
    abs_value /= RING1_SIZE;
    
    result.ring0 = abs_value % RING0_SIZE;
    
    return result;
}

int32_t babylon_to_int(const BabylonianNumber* num) {
    if (!num) return 0;
    
    /* Reconstruct value from rings */
    int32_t value = 0;
    
    value += num->ring0;
    value *= RING1_SIZE;
    
    value += num->ring1;
    value *= RING2_SIZE;
    
    value += num->ring2;
    value *= RING3_SIZE;
    
    value += num->ring3;
    
    return num->negative ? -value : value;
}

/**
 * @brief Add two Babylonian numbers (PURE O(1))
 * 
 * Algorithm:
 *   Add each ring with carry propagation
 *   Exactly 4 operations, always
 * 
 * This is the PUREST form of O(1) geometric addition.
 */
MathError babylon_add(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Handle different signs */
    if (a->negative != b->negative) {
        /* Different signs: use subtraction */
        BabylonianNumber neg_b = *b;
        neg_b.negative = !neg_b.negative;
        return babylon_sub(result, a, &neg_b);
    }
    
    /* Same sign: pure ring addition with carry */
    
    uint16_t sum;
    uint8_t carry = 0;
    
    /* Ring 3 (milliseconds): 0-99 */
    sum = (uint16_t)a->ring3 + (uint16_t)b->ring3 + carry;
    result->ring3 = sum % RING3_SIZE;
    carry = sum / RING3_SIZE;
    
    /* Ring 2 (seconds): 0-59 */
    sum = (uint16_t)a->ring2 + (uint16_t)b->ring2 + carry;
    result->ring2 = sum % RING2_SIZE;
    carry = sum / RING2_SIZE;
    
    /* Ring 1 (minutes): 0-59 */
    sum = (uint16_t)a->ring1 + (uint16_t)b->ring1 + carry;
    result->ring1 = sum % RING1_SIZE;
    carry = sum / RING1_SIZE;
    
    /* Ring 0 (hours): 0-11 */
    sum = (uint16_t)a->ring0 + (uint16_t)b->ring0 + carry;
    result->ring0 = sum % RING0_SIZE;
    /* Overflow carry is discarded (modular arithmetic) */
    
    result->negative = a->negative;
    
    return MATH_SUCCESS;
}

/**
 * @brief Subtract two Babylonian numbers (PURE O(1))
 */
MathError babylon_sub(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Handle different signs */
    if (a->negative != b->negative) {
        /* Different signs: use addition */
        BabylonianNumber neg_b = *b;
        neg_b.negative = !neg_b.negative;
        return babylon_add(result, a, &neg_b);
    }
    
    /* Same sign: determine which is larger */
    int32_t val_a = babylon_to_int(a);
    int32_t val_b = babylon_to_int(b);
    
    if (val_a < 0) val_a = -val_a;
    if (val_b < 0) val_b = -val_b;
    
    bool a_larger = (val_a >= val_b);
    
    const BabylonianNumber* larger = a_larger ? a : b;
    const BabylonianNumber* smaller = a_larger ? b : a;
    
    /* Pure ring subtraction with borrow */
    int16_t diff;
    uint8_t borrow = 0;
    
    /* Ring 3 (milliseconds) */
    diff = (int16_t)larger->ring3 - (int16_t)smaller->ring3 - borrow;
    if (diff < 0) {
        diff += RING3_SIZE;
        borrow = 1;
    } else {
        borrow = 0;
    }
    result->ring3 = (uint8_t)diff;
    
    /* Ring 2 (seconds) */
    diff = (int16_t)larger->ring2 - (int16_t)smaller->ring2 - borrow;
    if (diff < 0) {
        diff += RING2_SIZE;
        borrow = 1;
    } else {
        borrow = 0;
    }
    result->ring2 = (uint8_t)diff;
    
    /* Ring 1 (minutes) */
    diff = (int16_t)larger->ring1 - (int16_t)smaller->ring1 - borrow;
    if (diff < 0) {
        diff += RING1_SIZE;
        borrow = 1;
    } else {
        borrow = 0;
    }
    result->ring1 = (uint8_t)diff;
    
    /* Ring 0 (hours) */
    diff = (int16_t)larger->ring0 - (int16_t)smaller->ring0 - borrow;
    if (diff < 0) {
        diff += RING0_SIZE;
        /* Underflow: result is negative */
    }
    result->ring0 = (uint8_t)diff;
    
    /* Set sign */
    result->negative = a_larger ? a->negative : !a->negative;
    
    return MATH_SUCCESS;
}

/**
 * @brief Multiply two Babylonian numbers (O(1))
 * 
 * Converts to integer, multiplies, converts back.
 * Still O(1) because Babylonian numbers have fixed size.
 */
MathError babylon_mul(BabylonianNumber* result,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b) {
    if (!result || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Convert to integers */
    int32_t val_a = babylon_to_int(a);
    int32_t val_b = babylon_to_int(b);
    
    /* Multiply */
    int64_t product = (int64_t)val_a * (int64_t)val_b;
    
    /* Handle overflow (modular arithmetic) */
    if (product >= BABYLON_MAX || product <= -BABYLON_MAX) {
        product = product % BABYLON_MAX;
    }
    
    /* Convert back */
    *result = babylon_from_int((int32_t)product);
    
    return MATH_SUCCESS;
}

/**
 * @brief Divide two Babylonian numbers (O(1))
 */
MathError babylon_div(BabylonianNumber* quotient,
                      BabylonianNumber* remainder,
                      const BabylonianNumber* a,
                      const BabylonianNumber* b) {
    if (!quotient || !a || !b) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    /* Convert to integers */
    int32_t val_a = babylon_to_int(a);
    int32_t val_b = babylon_to_int(b);
    
    /* Check for division by zero */
    if (val_b == 0) {
        return MATH_ERROR_DIVISION_BY_ZERO;
    }
    
    /* Divide */
    int32_t q = val_a / val_b;
    int32_t r = val_a % val_b;
    
    /* Convert back */
    *quotient = babylon_from_int(q);
    
    if (remainder) {
        *remainder = babylon_from_int(r);
    }
    
    return MATH_SUCCESS;
}