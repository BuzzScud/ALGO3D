/**
 * SIMD-Optimized Gradient Operations
 * 
 * Provides vectorized gradient accumulation using AVX2/AVX-512
 * with automatic fallback to scalar operations.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

// CPU feature detection
#ifdef __x86_64__
#include <cpuid.h>

static int cpu_has_avx2 = -1;
static int cpu_has_avx512 = -1;

static void detect_cpu_features(void) {
    if (cpu_has_avx2 != -1) return;  // Already detected
    
    unsigned int eax, ebx, ecx, edx;
    
    // Check for AVX2
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        cpu_has_avx2 = (ebx & (1 << 5)) != 0;
        cpu_has_avx512 = (ebx & (1 << 16)) != 0;
    } else {
        cpu_has_avx2 = 0;
        cpu_has_avx512 = 0;
    }
}
#else
static void detect_cpu_features(void) {
    cpu_has_avx2 = 0;
    cpu_has_avx512 = 0;
}
static int cpu_has_avx2 = 0;
static int cpu_has_avx512 = 0;
#endif

// AVX2 implementation
#if defined(__AVX2__)
#include <immintrin.h>

static void simd_accumulate_gradients_avx2(float* restrict dest, 
                                           const float* restrict src, 
                                           size_t size) {
    size_t i = 0;
    
    // Process 8 floats at a time with AVX2
    for (; i + 8 <= size; i += 8) {
        __m256 dest_vec = _mm256_loadu_ps(&dest[i]);
        __m256 src_vec = _mm256_loadu_ps(&src[i]);
        __m256 result = _mm256_add_ps(dest_vec, src_vec);
        _mm256_storeu_ps(&dest[i], result);
    }
    
    // Handle remaining elements
    for (; i < size; i++) {
        dest[i] += src[i];
    }
}

static void simd_scale_gradients_avx2(float* restrict gradients,
                                      float scale,
                                      size_t size) {
    size_t i = 0;
    __m256 scale_vec = _mm256_set1_ps(scale);
    
    // Process 8 floats at a time
    for (; i + 8 <= size; i += 8) {
        __m256 grad_vec = _mm256_loadu_ps(&gradients[i]);
        __m256 result = _mm256_mul_ps(grad_vec, scale_vec);
        _mm256_storeu_ps(&gradients[i], result);
    }
    
    // Handle remaining elements
    for (; i < size; i++) {
        gradients[i] *= scale;
    }
}

static void simd_zero_gradients_avx2(float* restrict gradients, size_t size) {
    size_t i = 0;
    __m256 zero_vec = _mm256_setzero_ps();
    
    // Process 8 floats at a time
    for (; i + 8 <= size; i += 8) {
        _mm256_storeu_ps(&gradients[i], zero_vec);
    }
    
    // Handle remaining elements
    for (; i < size; i++) {
        gradients[i] = 0.0;
    }
}

#endif

// Scalar fallback implementation
static void scalar_accumulate_gradients(float* restrict dest,
                                       const float* restrict src,
                                       size_t size) {
    for (size_t i = 0; i < size; i++) {
        dest[i] += src[i];
    }
}

static void scalar_scale_gradients(float* restrict gradients,
                                   float scale,
                                   size_t size) {
    for (size_t i = 0; i < size; i++) {
        gradients[i] *= scale;
    }
}

static void scalar_zero_gradients(float* restrict gradients, size_t size) {
    memset(gradients, 0, size * sizeof(float));
}

// Public API - automatically selects best implementation

/**
 * Accumulate gradients: dest += src
 * Uses SIMD if available, falls back to scalar
 */
void cllm_simd_accumulate_gradients(float* restrict dest,
                                    const float* restrict src,
                                    size_t size) {
    detect_cpu_features();
    
#if defined(__AVX2__)
    if (cpu_has_avx2) {
        simd_accumulate_gradients_avx2(dest, src, size);
        return;
    }
#endif
    
    scalar_accumulate_gradients(dest, src, size);
}

/**
 * Scale gradients: gradients *= scale
 * Uses SIMD if available, falls back to scalar
 */
void cllm_simd_scale_gradients(float* restrict gradients,
                               float scale,
                               size_t size) {
    detect_cpu_features();
    
#if defined(__AVX2__)
    if (cpu_has_avx2) {
        simd_scale_gradients_avx2(gradients, scale, size);
        return;
    }
#endif
    
    scalar_scale_gradients(gradients, scale, size);
}

/**
 * Zero gradients: gradients = 0
 * Uses SIMD if available, falls back to scalar
 */
void cllm_simd_zero_gradients(float* restrict gradients, size_t size) {
    detect_cpu_features();
    
#if defined(__AVX2__)
    if (cpu_has_avx2) {
        simd_zero_gradients_avx2(gradients, size);
        return;
    }
#endif
    
    scalar_zero_gradients(gradients, size);
}

/**
 * Get CPU feature information
 */
const char* cllm_simd_get_features(void) {
    detect_cpu_features();
    
    if (cpu_has_avx512) return "AVX-512";
    if (cpu_has_avx2) return "AVX2";
    return "Scalar";
}

/**
 * Accumulate gradients with segment ownership (lock-free)
 * Each sphere owns a segment and can write without locks
 */
void cllm_simd_accumulate_segment(float* restrict dest,
                                  const float* restrict src,
                                  size_t segment_start,
                                  size_t segment_end) {
    size_t size = segment_end - segment_start;
    cllm_simd_accumulate_gradients(&dest[segment_start], 
                                   &src[segment_start], 
                                   size);
}

/**
 * Atomic accumulation for boundary points
 * Used when segments overlap at boundaries
 */
void cllm_simd_accumulate_boundary(float* restrict dest,
                                   float value,
                                   size_t index) {
    // Atomic float addition using compare-and-swap
    // This ensures thread-safe gradient accumulation
    
    #if defined(__GNUC__) || defined(__clang__)
    // Use GCC/Clang atomic builtins
    union {
        float f;
        uint32_t i;
    } old_val, new_val;
    
    do {
        old_val.f = dest[index];
        new_val.f = old_val.f + value;
    } while (!__sync_bool_compare_and_swap(
        (uint32_t*)&dest[index],
        old_val.i,
        new_val.i
    ));
    #else
    // Fallback: Use mutex (slower but safe)
    static pthread_mutex_t gradient_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&gradient_lock);
    dest[index] += value;
    pthread_mutex_unlock(&gradient_lock);
    #endif
}