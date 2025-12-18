#ifndef CLLM_FP16_H
#define CLLM_FP16_H

#include <stdint.h>
#include <stddef.h>

/**
 * FP16 Mixed Precision Training Utilities
 * 
 * Provides conversion between FP32 and FP16 for mixed precision training
 * Uses F16C instructions when available, falls back to software conversion
 */

/**
 * Convert FP32 array to FP16
 * 
 * @param fp16 Output FP16 array (uint16_t representation)
 * @param fp32 Input FP32 array
 * @param n Number of elements
 */
void fp32_to_fp16(uint16_t* fp16, const float* fp32, size_t n);

/**
 * Convert FP16 array to FP32
 * 
 * @param fp32 Output FP32 array
 * @param fp16 Input FP16 array (uint16_t representation)
 * @param n Number of elements
 */
void fp16_to_fp32(float* fp32, const uint16_t* fp16, size_t n);

/**
 * Scale FP32 array by loss scale factor
 * Used before converting gradients to FP16
 * 
 * @param data Array to scale (in-place)
 * @param n Number of elements
 * @param scale Scale factor
 */
void scale_fp32_array(float* data, size_t n, float scale);

/**
 * Check for NaN or Inf in FP32 array
 * Used to detect gradient overflow in mixed precision training
 * 
 * @param data Array to check
 * @param n Number of elements
 * @return 1 if NaN/Inf found, 0 otherwise
 */
int has_nan_or_inf(const float* data, size_t n);

#endif /* CLLM_FP16_H */