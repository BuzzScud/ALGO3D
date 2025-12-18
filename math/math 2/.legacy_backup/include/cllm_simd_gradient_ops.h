/**
 * SIMD-Optimized Gradient Operations Header
 * 
 * Provides vectorized gradient accumulation with automatic
 * CPU feature detection and fallback to scalar operations.
 */

#ifndef CLLM_SIMD_GRADIENT_OPS_H
#define CLLM_SIMD_GRADIENT_OPS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Accumulate gradients: dest += src
 * 
 * Automatically uses AVX2/AVX-512 if available, falls back to scalar.
 * 
 * @param dest Destination gradient buffer (accumulated)
 * @param src Source gradient buffer (to add)
 * @param size Number of float elements
 */
void cllm_simd_accumulate_gradients(float* restrict dest,
                                    const float* restrict src,
                                    size_t size);

/**
 * Scale gradients: gradients *= scale
 * 
 * Automatically uses SIMD if available.
 * 
 * @param gradients Gradient buffer to scale
 * @param scale Scaling factor
 * @param size Number of float elements
 */
void cllm_simd_scale_gradients(float* restrict gradients,
                               float scale,
                               size_t size);

/**
 * Zero gradients: gradients = 0
 * 
 * Automatically uses SIMD if available.
 * 
 * @param gradients Gradient buffer to zero
 * @param size Number of float elements
 */
void cllm_simd_zero_gradients(float* restrict gradients, size_t size);

/**
 * Get CPU SIMD feature information
 * 
 * @return String describing available SIMD features ("AVX-512", "AVX2", or "Scalar")
 */
const char* cllm_simd_get_features(void);

/**
 * Accumulate gradients for a specific segment (lock-free)
 * 
 * Used in kissing spheres architecture where each sphere owns a segment.
 * No locking needed as segments don't overlap.
 * 
 * @param dest Destination gradient buffer
 * @param src Source gradient buffer
 * @param segment_start Start index of segment
 * @param segment_end End index of segment (exclusive)
 */
void cllm_simd_accumulate_segment(float* restrict dest,
                                  const float* restrict src,
                                  size_t segment_start,
                                  size_t segment_end);

/**
 * Atomic accumulation for boundary points
 * 
 * Used when segments overlap at boundaries in kissing spheres architecture.
 * Thread-safe atomic operation.
 * 
 * @param dest Destination gradient buffer
 * @param value Value to add
 * @param index Index of boundary point
 */
void cllm_simd_accumulate_boundary(float* restrict dest,
                                   float value,
                                   size_t index);

#ifdef __cplusplus
}
#endif

#endif // CLLM_SIMD_GRADIENT_OPS_H