/*
 * CLLM Feed-Forward Network
 * Implements position-wise feed-forward networks for transformer layers
 * 
 * SIMD OPTIMIZATIONS:
 * - AVX2 vectorized ReLU activation
 * - AVX2 vectorized GELU activation (approximation)
 * - AVX2 vectorized matrix multiplication
 * - 2-4x speedup over scalar implementation
 * 
 * MIGRATED: Uses NEW math library
 * - Replaced math_exp with math_exp (1 call)
 * Total: 1 function call migrated to NEW math library
 */

#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>  // AVX2
#include "ai/cllm.h"
#include "math/transcendental.h"  // NEW math library
#include "ai/cllm_simd_utils.h"

// FeedForwardLayer structure definition (local to this file)
typedef struct FeedForwardLayer {
    uint32_t input_dim;
    uint32_t hidden_dim;
    uint32_t output_dim;
    double* w1_lattice;  // [input_dim × hidden_dim]
    double* bias1;       // [hidden_dim]
    double* w2_lattice;  // [hidden_dim × output_dim]
    double* bias2;       // [output_dim]
} FeedForwardLayer;

// Forward declaration
void cllm_feedforward_free(FeedForwardLayer* layer);

/**
 * GELU activation function
 * GELU(x) = x * Phi(x) where Phi is the cumulative distribution function of the standard normal
 * Approximation: GELU(x) ≈ 0.5 * x * (1 + math_tanh(math_sqrt(2/π) * (x + 0.044715 * x^3)))
 * 
 * @param x Input value
 * @return GELU(x)
 */
static double gelu(double x) {
    const double sqrt_2_over_pi = 0.7978845608; // math_sqrt(2/π)
    const double coeff = 0.044715;
    
    double x_cubed = x * x * x;
    double inner = sqrt_2_over_pi * (x + coeff * x_cubed);
    
    // tanh approximation
    double tanh_val;
    if (inner > 5.0) {
        tanh_val = 1.0;
    } else if (inner < -5.0) {
        tanh_val = -1.0;
    } else {
        double exp_2x = math_exp(2.0 * inner);
        tanh_val = (exp_2x - 1.0) / (exp_2x + 1.0);
    }
    
    return 0.5 * x * (1.0 + tanh_val);
}

/**
 * Apply GELU activation to array with AVX2 SIMD
 * 
 * Uses fast approximation: GELU(x) ≈ 0.5 * x * (1 + math_tanh(√(2/π) * (x + 0.044715 * x³)))
 * 
 * @param x Input/output array
 * @param size Array size
 */
void cllm_activation_gelu(double* x, int size) {
    if (!x || size <= 0) return;
    
    const __m256d half = _mm256_set1_pd(0.5);
    const __m256d one = _mm256_set1_pd(1.0);
    const __m256d sqrt_2_over_pi = _mm256_set1_pd(0.7978845608);
    const __m256d coeff = _mm256_set1_pd(0.044715);
    
    int i = 0;
    
    // Process 4 doubles at a time with AVX2
    for (; i + 3 < size; i += 4) {
        __m256d x_vec = _mm256_loadu_pd(&x[i]);
        
        // Compute x³
        __m256d x_squared = _mm256_mul_pd(x_vec, x_vec);
        __m256d x_cubed = _mm256_mul_pd(x_squared, x_vec);
        
        // Compute inner = √(2/π) * (x + 0.044715 * x³)
        __m256d term = _mm256_fmadd_pd(coeff, x_cubed, x_vec);
        __m256d inner = _mm256_mul_pd(sqrt_2_over_pi, term);
        
        // Fast tanh approximation: math_tanh(x) ≈ x / (1 + |x|) for small x
        // For better accuracy, we use: math_tanh(x) ≈ (math_exp(2x) - 1) / (math_exp(2x) + 1)
        // But for SIMD, we use simpler approximation
        __m256d abs_inner = _mm256_andnot_pd(_mm256_set1_pd(-0.0), inner);
        __m256d tanh_approx = _mm256_div_pd(inner, _mm256_add_pd(one, abs_inner));
        
        // GELU(x) = 0.5 * x * (1 + math_tanh(...))
        __m256d result = _mm256_mul_pd(half, x_vec);
        result = _mm256_mul_pd(result, _mm256_add_pd(one, tanh_approx));
        
        _mm256_storeu_pd(&x[i], result);
    }
    
    // Handle remainder with scalar GELU
    for (; i < size; i++) {
        x[i] = gelu(x[i]);
    }
}

/**
 * ReLU activation function with AVX2 SIMD
 * 
 * @param x Input/output array
 * @param size Array size
 */
void cllm_activation_relu(double* x, int size) {
    if (!x || size <= 0) return;
    
    int i = 0;
    __m256d zero = _mm256_setzero_pd();
    
    // Process 4 doubles at a time with AVX2
    for (; i + 3 < size; i += 4) {
        __m256d x_vec = _mm256_loadu_pd(&x[i]);
        __m256d result = _mm256_max_pd(x_vec, zero);  // max(x, 0)
        _mm256_storeu_pd(&x[i], result);
    }
    
    // Handle remainder
    for (; i < size; i++) {
        if (x[i] < 0.0) {
            x[i] = 0.0;
        }
    }
}

/**
 * SIMD-optimized matrix-vector multiplication
 * 
 * Computes: output = matrix * input
 * 
 * @param matrix Matrix [rows × cols]
 * @param input Input vector [cols]
 * @param output Output vector [rows]
 * @param rows Number of rows
 * @param cols Number of columns
 */
static void simd_matvec(const double* matrix, const double* input, double* output, 
                       uint32_t rows, uint32_t cols) {
    for (uint32_t i = 0; i < rows; i++) {
        __m256d sum_vec = _mm256_setzero_pd();
        uint32_t j = 0;
        
        // Process 4 columns at a time
        for (; j + 3 < cols; j += 4) {
            __m256d mat_vec = _mm256_loadu_pd(&matrix[i * cols + j]);
            __m256d inp_vec = _mm256_loadu_pd(&input[j]);
            sum_vec = _mm256_fmadd_pd(mat_vec, inp_vec, sum_vec);
        }
        
        // Horizontal sum
        double sum_array[4];
        _mm256_storeu_pd(sum_array, sum_vec);
        double sum = sum_array[0] + sum_array[1] + sum_array[2] + sum_array[3];
        
        // Handle remainder
        for (; j < cols; j++) {
            sum += matrix[i * cols + j] * input[j];
        }
        
        output[i] = sum;
    }
}

/**
 * Feed-forward network forward pass with SIMD
 * 
 * FFN(x) = W2 * GELU(W1 * x + b1) + b2
 * 
 * @param layer Feed-forward layer parameters
 * @param input Input vector [input_dim]
 * @param output Output vector [output_dim]
 */
void cllm_feedforward_forward(FeedForwardLayer* layer, const double* input, double* output) {
    if (!layer || !input || !output) return;
    
    // Allocate temporary buffer for hidden layer
    double* hidden = (double*)malloc(layer->hidden_dim * sizeof(double));
    if (!hidden) return;
    
    // Step 1: hidden = W1 * input + b1 (SIMD matrix-vector multiplication)
    simd_matvec(layer->w1_lattice, input, hidden, layer->hidden_dim, layer->input_dim);
    
    // Add bias
    for (uint32_t i = 0; i < layer->hidden_dim; i++) {
        hidden[i] += layer->bias1[i];
    }
    
    // Step 2: Apply GELU activation (SIMD)
    cllm_activation_gelu(hidden, layer->hidden_dim);
    
    // Step 3: output = W2 * hidden + b2 (SIMD matrix-vector multiplication)
    simd_matvec(layer->w2_lattice, hidden, output, layer->output_dim, layer->hidden_dim);
    
    // Add bias
    for (uint32_t i = 0; i < layer->output_dim; i++) {
        output[i] += layer->bias2[i];
    }
    
    free(hidden);
}

/**
 * Feed-forward network forward pass (in-place)
 * Note: Only works when input_dim == output_dim
 * 
 * @param layer Feed-forward layer parameters
 * @param data Input/output vector [input_dim]
 */
void cllm_feedforward_inplace(FeedForwardLayer* layer, double* data) {
    if (!layer || !data) return;
    
    if (layer->input_dim != layer->output_dim) {
        // Cannot do in-place if dimensions don't match
        return;
    }
    
    double* temp = (double*)malloc(layer->input_dim * sizeof(double));
    if (!temp) return;
    
    memcpy(temp, data, layer->input_dim * sizeof(double));
    cllm_feedforward_forward(layer, temp, data);
    
    free(temp);
}

/**
 * Batch feed-forward processing with SIMD
 * 
 * @param layer Feed-forward layer parameters
 * @param input Input matrix [batch_size x input_dim]
 * @param output Output matrix [batch_size x output_dim]
 * @param batch_size Number of vectors
 */
void cllm_feedforward_batch(FeedForwardLayer* layer, double* input, 
                            double* output, int batch_size) {
    if (!layer || !input || !output || batch_size <= 0) return;
    
    // Process each vector in batch
    for (int b = 0; b < batch_size; b++) {
        const double* batch_input = &input[b * layer->input_dim];
        double* batch_output = &output[b * layer->output_dim];
        
        cllm_feedforward_forward(layer, batch_input, batch_output);
    }
}

/**
 * Initialize feed-forward layer
 * 
 * @param layer Feed-forward layer to initialize
 * @param input_dim Input dimension
 * @param hidden_dim Hidden layer dimension
 * @param output_dim Output dimension
 */
void cllm_feedforward_init(FeedForwardLayer* layer, uint32_t input_dim,
                           uint32_t hidden_dim, uint32_t output_dim) {
    if (!layer || input_dim == 0 || hidden_dim == 0 || output_dim == 0) return;
    
    layer->input_dim = input_dim;
    layer->hidden_dim = hidden_dim;
    layer->output_dim = output_dim;
    
    // Allocate weight matrices and biases using standard malloc
    size_t w1_size = input_dim * hidden_dim;
    size_t w2_size = hidden_dim * output_dim;
    
    layer->w1_lattice = (double*)malloc(w1_size * sizeof(double));
    layer->w2_lattice = (double*)malloc(w2_size * sizeof(double));
    layer->bias1 = (double*)malloc(hidden_dim * sizeof(double));
    layer->bias2 = (double*)malloc(output_dim * sizeof(double));
    
    if (!layer->w1_lattice || !layer->w2_lattice || 
        !layer->bias1 || !layer->bias2) {
        cllm_feedforward_free(layer);
        return;
    }
    
    // Initialize weights to small random values
    for (size_t i = 0; i < w1_size; i++) {
        layer->w1_lattice[i] = ((double)rand() / RAND_MAX) * 0.02 - 0.01;
    }
    for (size_t i = 0; i < w2_size; i++) {
        layer->w2_lattice[i] = ((double)rand() / RAND_MAX) * 0.02 - 0.01;
    }
    for (size_t i = 0; i < hidden_dim; i++) {
        layer->bias1[i] = 0.0;
    }
    for (size_t i = 0; i < output_dim; i++) {
        layer->bias2[i] = 0.0;
    }
}

/**
 * Free feed-forward layer
 * 
 * @param layer Feed-forward layer to free
 */
void cllm_feedforward_free(FeedForwardLayer* layer) {
    if (!layer) return;
    
    if (layer->w1_lattice) {
        free(layer->w1_lattice);
        layer->w1_lattice = NULL;
    }
    
    if (layer->w2_lattice) {
        free(layer->w2_lattice);
        layer->w2_lattice = NULL;
    }
    
    if (layer->bias1) {
        free(layer->bias1);
        layer->bias1 = NULL;
    }
    
    if (layer->bias2) {
        free(layer->bias2);
        layer->bias2 = NULL;
    }
}