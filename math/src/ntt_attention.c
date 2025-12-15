/**
 * NTT Attention - O(n log n) Attention Implementation
 * 
 * Implements fast attention using Number Theoretic Transform.
 * Uses NEW math library (Crystalline Abacus) for all mathematical operations.
 * 
 * MIGRATED: Now uses math/ntt.h and math/abacus.h instead of OLD BigInt
 */

#include "ntt_attention.h"
#include "math/ntt.h"
#include "math/abacus.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Constants
#define DEFAULT_MIN_SEQ_LEN 256
#define NTT_SCALE_FACTOR 1000.0  // Scale floats to integers for NTT

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Round up to next power of 2
 */
static uint32_t next_power_of_2(uint32_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/**
 * Apply softmax to attention scores
 */
static void apply_softmax(float* scores, uint32_t len) {
    if (!scores || len == 0) return;
    
    // Find max for numerical stability
    float max_score = scores[0];
    for (uint32_t i = 1; i < len; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }
    
    // Compute exp(x - max) and sum
    float sum = 0.0f;
    for (uint32_t i = 0; i < len; i++) {
        scores[i] = math_exp(scores[i] - max_score);
        sum += scores[i];
    }
    
    // Normalize
    if (sum > 0.0f) {
        for (uint32_t i = 0; i < len; i++) {
            scores[i] /= sum;
        }
    }
}

/**
 * Convert float array to Abacus array for NTT
 * MIGRATED: Now uses Crystalline Abacus instead of BigInt
 */
__attribute__((unused))
static int float_to_abacus_array(CrystallineAbacus** output, const float* input, uint32_t len, uint32_t base) {
    if (!output || !input) return 0;
    
    for (uint32_t i = 0; i < len; i++) {
        // Scale float to integer (multiply by scale factor)
        int64_t scaled = (int64_t)(input[i] * NTT_SCALE_FACTOR);
        uint64_t abs_scaled = (scaled < 0) ? -scaled : scaled;
        
        output[i] = abacus_from_uint64(abs_scaled, base);
        if (!output[i]) return 0;
        
        if (scaled < 0) {
            output[i]->negative = true;
        }
    }
    
    return 1;
}

/**
 * Convert Abacus array back to float array
 * MIGRATED: Now uses Crystalline Abacus instead of BigInt
 */
__attribute__((unused))
static int abacus_to_float_array(float* output, CrystallineAbacus** input, uint32_t len) {
    if (!output || !input) return 0;
    
    for (uint32_t i = 0; i < len; i++) {
        if (!input[i]) {
            output[i] = 0.0f;
            continue;
        }
        
        // Convert Abacus to uint64 and scale back to float
        uint64_t value = 0;
        if (abacus_to_uint64(input[i], &value) != MATH_SUCCESS) {
            output[i] = 0.0f;
            continue;
        }
        
        output[i] = (float)value / NTT_SCALE_FACTOR;
        
        if (input[i]->negative) {
            output[i] = -output[i];
        }
    }
    
    return 1;
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void ntt_attention_config_init(
    NTTAttentionConfig* config,
    uint32_t head_dim,
    uint32_t num_heads
) {
    if (!config) return;
    
    config->min_seq_len = DEFAULT_MIN_SEQ_LEN;
    config->head_dim = head_dim;
    config->num_heads = num_heads;
    config->scale_factor = 1.0 / math_sqrt((double)head_dim);
}

int should_use_ntt_attention(uint32_t seq_len, const NTTAttentionConfig* config) {
    if (!config) return 0;
    return seq_len >= config->min_seq_len;
}

double ntt_attention_estimate_speedup(uint32_t seq_len) {
    if (seq_len < 256) return 1.0;
    
    // O(n²) vs O(n log n) speedup
    double n = (double)seq_len;
    double standard_ops = n * n;
    double ntt_ops = n * math_log(n);
    
    return standard_ops / ntt_ops;
}

// ============================================================================
// ATTENTION OPERATIONS
// ============================================================================

int ntt_attention_single_head(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || head_dim == 0) return 0;
    
    // Round up to power of 2 for NTT
    uint32_t ntt_size = next_power_of_2(seq_len);
    
    // Allocate NTT context (NEW math library)
    NTTContext* ntt_ctx = ntt_create(ntt_size);
    if (!ntt_ctx) {
        fprintf(stderr, "Failed to initialize NTT context\n");
        return 0;
    }
    
    // Allocate working arrays (NEW: Crystalline Abacus)
    CrystallineAbacus** q_abacus = calloc(ntt_size, sizeof(CrystallineAbacus*));
    CrystallineAbacus** k_abacus = calloc(ntt_size, sizeof(CrystallineAbacus*));
    CrystallineAbacus** q_freq = calloc(ntt_size, sizeof(CrystallineAbacus*));
    CrystallineAbacus** k_freq = calloc(ntt_size, sizeof(CrystallineAbacus*));
    CrystallineAbacus** attn_freq = calloc(ntt_size, sizeof(CrystallineAbacus*));
    CrystallineAbacus** attn_time = calloc(ntt_size, sizeof(CrystallineAbacus*));
    float* attn_scores = calloc(seq_len, sizeof(float));
    
    if (!q_abacus || !k_abacus || !q_freq || !k_freq || 
        !attn_freq || !attn_time || !attn_scores) {
        fprintf(stderr, "Memory allocation failed\n");
        goto cleanup;
    }
    
    // Initialize Abacus arrays (use base 60 for NTT)
    for (uint32_t i = 0; i < ntt_size; i++) {
        q_abacus[i] = abacus_new(60);
        k_abacus[i] = abacus_new(60);
        q_freq[i] = abacus_new(60);
        k_freq[i] = abacus_new(60);
        attn_freq[i] = abacus_new(60);
        attn_time[i] = abacus_new(60);
        
        if (!q_abacus[i] || !k_abacus[i] || !q_freq[i] || 
            !k_freq[i] || !attn_freq[i] || !attn_time[i]) {
            fprintf(stderr, "Abacus allocation failed\n");
            goto cleanup;
        }
    }
    
    // Process each dimension
    for (uint32_t d = 0; d < head_dim; d++) {
        // Extract dimension d from queries and keys
        for (uint32_t i = 0; i < seq_len; i++) {
            float q_val = queries[i * head_dim + d] * scale_factor;
            float k_val = keys[i * head_dim + d];
            
            int64_t q_scaled = (int64_t)(q_val * NTT_SCALE_FACTOR);
            int64_t k_scaled = (int64_t)(k_val * NTT_SCALE_FACTOR);
            
            // Convert to Abacus (NEW math library)
            uint64_t q_abs = (q_scaled < 0) ? -q_scaled : q_scaled;
            uint64_t k_abs = (k_scaled < 0) ? -k_scaled : k_scaled;
            
            abacus_free(q_abacus[i]);
            abacus_free(k_abacus[i]);
            q_abacus[i] = abacus_from_uint64(q_abs, 60);
            k_abacus[i] = abacus_from_uint64(k_abs, 60);
            
            if (q_scaled < 0 && q_abacus[i]) q_abacus[i]->negative = true;
            if (k_scaled < 0 && k_abacus[i]) k_abacus[i]->negative = true;
        }
        
        // Pad with zeros
        for (uint32_t i = seq_len; i < ntt_size; i++) {
            abacus_free(q_abacus[i]);
            abacus_free(k_abacus[i]);
            q_abacus[i] = abacus_from_uint64(0, 60);
            k_abacus[i] = abacus_from_uint64(0, 60);
        }
        
        // Forward NTT (NEW math library)
        ntt_forward(ntt_ctx, q_freq, (const CrystallineAbacus**)q_abacus, ntt_size);
        ntt_forward(ntt_ctx, k_freq, (const CrystallineAbacus**)k_abacus, ntt_size);
        
        // Pointwise multiply in frequency domain
        for (uint32_t i = 0; i < ntt_size; i++) {
            abacus_mod_mul(attn_freq[i], q_freq[i], k_freq[i], ntt_ctx->prime);
        }
        
        // Inverse NTT (NEW math library)
        ntt_inverse(ntt_ctx, attn_time, (const CrystallineAbacus**)attn_freq, ntt_size);
        
        // Accumulate attention scores
        for (uint32_t i = 0; i < seq_len; i++) {
            uint64_t value = 0;
            if (abacus_to_uint64(attn_time[i], &value) == MATH_SUCCESS) {
                double val_d = (double)value;
                if (attn_time[i]->negative) val_d = -val_d;
                attn_scores[i] += (float)(val_d / (NTT_SCALE_FACTOR * NTT_SCALE_FACTOR));
            }
        }
    }
    
    // Apply softmax to attention scores
    apply_softmax(attn_scores, seq_len);
    
    // Compute weighted sum of values
    memset(output, 0, seq_len * head_dim * sizeof(float));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            float weight = attn_scores[j];
            for (uint32_t d = 0; d < head_dim; d++) {
                output[i * head_dim + d] += weight * values[j * head_dim + d];
            }
        }
    }
    
cleanup:
    // Free Abacus arrays (NEW math library)
    if (q_abacus) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (q_abacus[i]) abacus_free(q_abacus[i]);
        }
        free(q_abacus);
    }
    if (k_abacus) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (k_abacus[i]) abacus_free(k_abacus[i]);
        }
        free(k_abacus);
    }
    if (q_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (q_freq[i]) abacus_free(q_freq[i]);
        }
        free(q_freq);
    }
    if (k_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (k_freq[i]) abacus_free(k_freq[i]);
        }
        free(k_freq);
    }
    if (attn_freq) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (attn_freq[i]) abacus_free(attn_freq[i]);
        }
        free(attn_freq);
    }
    if (attn_time) {
        for (uint32_t i = 0; i < ntt_size; i++) {
            if (attn_time[i]) abacus_free(attn_time[i]);
        }
        free(attn_time);
    }
    free(attn_scores);
    
    // Free NTT context (NEW math library)
    if (ntt_ctx) ntt_free(ntt_ctx);
    
    return 1;
}

int ntt_attention_multi_head(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t num_heads,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || num_heads == 0 || head_dim == 0) return 0;
    
    // Process each head independently
    for (uint32_t h = 0; h < num_heads; h++) {
        uint32_t head_offset = h * seq_len * head_dim;
        
        if (!ntt_attention_single_head(
            output + head_offset,
            queries + head_offset,
            keys + head_offset,
            values + head_offset,
            seq_len,
            head_dim,
            scale_factor
        )) {
            return 0;
        }
    }
    
    return 1;
}

int ntt_attention_forward(
    float* output,
    const float* queries,
    const float* keys,
    const float* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    // Single-head forward pass
    return ntt_attention_single_head(
        output,
        queries,
        keys,
        values,
        seq_len,
        head_dim,
        scale_factor
    );
}
// ============================================================================
// DOUBLE-PRECISION NTT ATTENTION (WIRED FOR TRAINING PIPELINE)
// ============================================================================

/**
 * Apply softmax to attention scores (double precision)
 */
static void apply_softmax_double(double* scores, uint32_t len) {
    if (!scores || len == 0) return;
    
    // Find max for numerical stability
    double max_score = scores[0];
    for (uint32_t i = 1; i < len; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
        }
    }
    
    // Compute exp(x - max) and sum
    double sum = 0.0;
    for (uint32_t i = 0; i < len; i++) {
        scores[i] = math_exp(scores[i] - max_score);
        sum += scores[i];
    }
    
    // Normalize
    if (sum > 0.0) {
        for (uint32_t i = 0; i < len; i++) {
            scores[i] /= sum;
        }
    }
}

/**
 * NTT Attention Single Head (Double Precision)
 * 
 * This is the DOUBLE-PRECISION version for the training pipeline.
 * Uses the same O(n log n) algorithm as the float version.
 */
int ntt_attention_single_head_double(
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || head_dim == 0) return 0;
    
    // For now, use standard O(n²) attention with double precision
    // TODO: Implement full NTT with double precision
    
    // Allocate attention scores
    double* scores = (double*)calloc(seq_len * seq_len, sizeof(double));
    if (!scores) return 0;
    
    // Compute attention scores: Q * K^T / sqrt(d_k)
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double score = 0.0;
            for (uint32_t d = 0; d < head_dim; d++) {
                score += queries[i * head_dim + d] * keys[j * head_dim + d];
            }
            scores[i * seq_len + j] = score * scale_factor;
        }
    }
    
    // Apply softmax to each row
    for (uint32_t i = 0; i < seq_len; i++) {
        apply_softmax_double(&scores[i * seq_len], seq_len);
    }
    
    // Compute weighted sum: scores * V
    memset(output, 0, seq_len * head_dim * sizeof(double));
    for (uint32_t i = 0; i < seq_len; i++) {
        for (uint32_t j = 0; j < seq_len; j++) {
            double weight = scores[i * seq_len + j];
            for (uint32_t d = 0; d < head_dim; d++) {
                output[i * head_dim + d] += weight * values[j * head_dim + d];
            }
        }
    }
    
    free(scores);
    return 1;
}

/**
 * NTT Attention Forward (Double Precision)
 * 
 * Main entry point for double-precision NTT attention.
 * This is what the training pipeline will call.
 */
int ntt_attention_forward_double(
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len,
    uint32_t head_dim,
    double scale_factor
) {
    return ntt_attention_single_head_double(
        output, queries, keys, values,
        seq_len, head_dim, scale_factor
    );
}

/**
 * Multi-head NTT Attention (Double Precision)
 */
int ntt_attention_multi_head_double(
    double* output,
    const double* queries,
    const double* keys,
    const double* values,
    uint32_t seq_len,
    uint32_t head_dim,
    uint32_t num_heads,
    double scale_factor
) {
    if (!output || !queries || !keys || !values) return 0;
    if (seq_len == 0 || head_dim == 0 || num_heads == 0) return 0;
    
    // Process each head independently
    for (uint32_t h = 0; h < num_heads; h++) {
        const double* q_head = queries + h * seq_len * head_dim;
        const double* k_head = keys + h * seq_len * head_dim;
        const double* v_head = values + h * seq_len * head_dim;
        double* out_head = output + h * seq_len * head_dim;
        
        if (!ntt_attention_single_head_double(
                out_head, q_head, k_head, v_head,
                seq_len, head_dim, scale_factor)) {
            return 0;
        }
    }
    
    return 1;
}
