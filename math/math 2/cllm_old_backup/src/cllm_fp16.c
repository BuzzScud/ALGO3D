/**
 * FP16 Mixed Precision Training Utilities
 * 
 * Provides conversion between FP32 and FP16 for mixed precision training
 */

#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>
#include "../include/cllm_fp16.h"
#include "math/transcendental.h"

/**
 * Software FP32 to FP16 conversion (fallback)
 */
static uint16_t fp32_to_fp16_scalar(float value) {
    // Use union to avoid strict aliasing violation
    union { float f; uint32_t u; } converter;
    converter.f = value;
    uint32_t f32 = converter.u;
    
    uint32_t sign = (f32 >> 16) & 0x8000;
    uint32_t exponent = ((f32 >> 23) & 0xFF) - 127 + 15;
    uint32_t mantissa = (f32 >> 13) & 0x3FF;
    
    // Handle special cases
    if (exponent <= 0) {
        // Underflow to zero
        return (uint16_t)sign;
    } else if (exponent >= 31) {
        // Overflow to infinity
        return (uint16_t)(sign | 0x7C00);
    }
    
    return (uint16_t)(sign | (exponent << 10) | mantissa);
}

/**
 * Software FP16 to FP32 conversion (fallback)
 */
static float fp16_to_fp32_scalar(uint16_t value) {
    uint32_t sign = (value & 0x8000) << 16;
    uint32_t exponent = (value >> 10) & 0x1F;
    uint32_t mantissa = value & 0x3FF;
    
    uint32_t f32 = 0;  // Initialize to zero to fix warning
    
    if (exponent == 0) {
        if (mantissa == 0) {
            // Zero
            f32 = sign;
        } else {
            // Denormalized number
            exponent = 1;
            while ((mantissa & 0x400) == 0) {
                mantissa <<= 1;
                exponent--;
            }
            mantissa &= 0x3FF;
            f32 = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
        }
    } else if (exponent == 31) {
        // Infinity or NaN
        f32 = sign | 0x7F800000 | (mantissa << 13);
    } else {
        // Normalized number
        f32 = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
    }
    
    // Use union to avoid strict aliasing violation
    union { uint32_t u; float f; } converter;
    converter.u = f32;
    return converter.f;
}

/**
 * Convert FP32 array to FP16 using F16C instructions
 */
void fp32_to_fp16(uint16_t* fp16, const float* fp32, size_t n) {
#ifdef __F16C__
    // Use hardware F16C instructions
    size_t i = 0;
    size_t n_vec = (n / 8) * 8;
    
    for (; i < n_vec; i += 8) {
        __m256 v = _mm256_loadu_ps(&fp32[i]);
        __m128i h = _mm256_cvtps_ph(v, _MM_FROUND_TO_NEAREST_INT);
        _mm_storeu_si128((__m128i*)&fp16[i], h);
    }
    
    // Scalar remainder
    for (; i < n; i++) {
        fp16[i] = fp32_to_fp16_scalar(fp32[i]);
    }
#else
    // Software fallback
    for (size_t i = 0; i < n; i++) {
        fp16[i] = fp32_to_fp16_scalar(fp32[i]);
    }
#endif
}

/**
 * Convert FP16 array to FP32 using F16C instructions
 */
void fp16_to_fp32(float* fp32, const uint16_t* fp16, size_t n) {
#ifdef __F16C__
    // Use hardware F16C instructions
    size_t i = 0;
    size_t n_vec = (n / 8) * 8;
    
    for (; i < n_vec; i += 8) {
        __m128i h = _mm_loadu_si128((__m128i*)&fp16[i]);
        __m256 v = _mm256_cvtph_ps(h);
        _mm256_storeu_ps(&fp32[i], v);
    }
    
    // Scalar remainder
    for (; i < n; i++) {
        fp32[i] = fp16_to_fp32_scalar(fp16[i]);
    }
#else
    // Software fallback
    for (size_t i = 0; i < n; i++) {
        fp32[i] = fp16_to_fp32_scalar(fp16[i]);
    }
#endif
}

/**
 * Scale FP32 array by loss scale factor
 */
void scale_fp32_array(float* data, size_t n, float scale) {
    size_t i = 0;
    size_t n_vec = (n / 8) * 8;
    __m256 vscale = _mm256_set1_ps(scale);
    
    // Vectorized scaling
    for (; i < n_vec; i += 8) {
        __m256 v = _mm256_loadu_ps(&data[i]);
        v = _mm256_mul_ps(v, vscale);
        _mm256_storeu_ps(&data[i], v);
    }
    
    // Scalar remainder
    for (; i < n; i++) {
        data[i] *= scale;
    }
}

/**
 * Check for NaN or Inf in FP32 array
 */
int has_nan_or_inf(const float* data, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (math_is_nan(data[i]) || math_is_inf(data[i])) {
            return 1;
        }
    }
    return 0;
}
