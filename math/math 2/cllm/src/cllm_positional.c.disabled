/*
 * CLLM Positional Encoding
 * Implements multiple positional encoding schemes based on prime lattice structure
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/prime_float_math.h"

#define PI 3.14159265358979323846
// Use PHI from prime_types.h
#define SYMMETRY_ORDER 12

/**
 * Generate spiral-based positional encoding
 * Uses Archimedean spiral with golden angle
 * 
 * @param pos_enc Positional encoding structure
 */
void cllm_generate_spiral_encoding(PositionalEncoding* pos_enc) {
    if (!pos_enc || !pos_enc->spiral_positions) return;
    
    uint32_t max_length = pos_enc->max_length;
    uint32_t embedding_dim = pos_enc->embedding_dim;
    
    printf("Generating spiral positional encoding...\n");
    
    // Golden angle for optimal spiral packing
    double golden_angle = 2.0 * PI / (PHI * PHI);
    
    for (uint32_t pos = 0; pos < max_length; pos++) {
        double* encoding = &pos_enc->spiral_positions[pos * embedding_dim];
        
        // Spiral parameters
        double angle = golden_angle * (double)pos;
        double radius = prime_sqrt((double)pos);
        
        // Generate encoding using spiral coordinates
        for (uint32_t i = 0; i < embedding_dim; i++) {
            double freq = (double)(i / 2 + 1);
            
            if (i % 2 == 0) {
                // Even dimensions: use cosine with spiral angle
                encoding[i] = prime_cos(freq * angle) * (1.0 + 0.1 * radius);
            } else {
                // Odd dimensions: use sine with spiral angle
                encoding[i] = prime_sin(freq * angle) * (1.0 + 0.1 * radius);
            }
        }
        
        // Normalize
        double norm = 0.0;
        for (uint32_t i = 0; i < embedding_dim; i++) {
            norm += encoding[i] * encoding[i];
        }
        norm = prime_sqrt(norm);
        
        if (norm > 1e-8f) {
            for (uint32_t i = 0; i < embedding_dim; i++) {
                encoding[i] /= norm;
            }
        }
    }
    
    printf("Spiral encoding complete!\n");
}

/**
 * Generate clock-based positional encoding
 * Uses 12-fold symmetry (clock positions)
 * 
 * @param pos_enc Positional encoding structure
 */
void cllm_generate_clock_encoding(PositionalEncoding* pos_enc) {
    if (!pos_enc || !pos_enc->clock_positions) return;
    
    uint32_t max_length = pos_enc->max_length;
    uint32_t embedding_dim = pos_enc->embedding_dim;
    
    printf("Generating clock positional encoding...\n");
    
    for (uint32_t pos = 0; pos < max_length; pos++) {
        double* encoding = &pos_enc->clock_positions[pos * embedding_dim];
        
        // Map position to clock position (0-11)
        uint32_t clock_pos = pos % SYMMETRY_ORDER;
        double clock_angle = 2.0 * PI * (double)clock_pos / (double)SYMMETRY_ORDER;
        
        // Radial component based on position
        double radius = prime_log((double)pos + 1.0);
        
        // Generate encoding
        for (uint32_t i = 0; i < embedding_dim; i++) {
            double freq = (double)(i / 2 + 1);
            
            if (i % 2 == 0) {
                // Even: radial modulation with clock angle
                encoding[i] = prime_cos(freq * clock_angle) * (1.0 + 0.2 * radius);
            } else {
                // Odd: tangential component
                encoding[i] = prime_sin(freq * clock_angle) * (1.0 + 0.2 * radius);
            }
        }
        
        // Add harmonic components for richer representation
        for (uint32_t i = 0; i < embedding_dim / 4; i++) {
            uint32_t idx = i * 4;
            if (idx + 3 < embedding_dim) {
                double harmonic = 2.0 * PI * (double)pos / (double)max_length;
                encoding[idx] += 0.1f * prime_cos(harmonic * (float)(i + 1));
                encoding[idx + 1] += 0.1f * prime_sin(harmonic * (float)(i + 1));
            }
        }
        
        // Normalize
        double norm = 0.0;
        for (uint32_t i = 0; i < embedding_dim; i++) {
            norm += encoding[i] * encoding[i];
        }
        norm = prime_sqrt(norm);
        
        if (norm > 1e-8f) {
            for (uint32_t i = 0; i < embedding_dim; i++) {
                encoding[i] /= norm;
            }
        }
    }
    
    printf("Clock encoding complete!\n");
}

/**
 * Generate prime-based positional encoding
 * Uses prime number sequence for positions
 * 
 * @param pos_enc Positional encoding structure
 */
void cllm_generate_prime_encoding(PositionalEncoding* pos_enc) {
    if (!pos_enc || !pos_enc->prime_positions) return;
    
    uint32_t max_length = pos_enc->max_length;
    uint32_t embedding_dim = pos_enc->embedding_dim;
    
    printf("Generating prime positional encoding...\n");
    
    // Generate prime sequence
    uint64_t* primes = (uint64_t*)malloc(max_length * sizeof(uint64_t));
    if (!primes) return;
    
    // Simple prime generation
    primes[0] = 2;
    uint32_t count = 1;
    uint64_t candidate = 3;
    
    while (count < max_length) {
        int is_prime = 1;
        uint64_t sqrt_c = (uint64_t)prime_sqrt((double)candidate);
        
        for (uint32_t i = 0; i < count && primes[i] <= sqrt_c; i++) {
            if (candidate % primes[i] == 0) {
                is_prime = 0;
                break;
            }
        }
        
        if (is_prime) {
            primes[count++] = candidate;
        }
        candidate += 2;
    }
    
    // Generate encodings based on primes
    for (uint32_t pos = 0; pos < max_length; pos++) {
        double* encoding = &pos_enc->prime_positions[pos * embedding_dim];
        uint64_t prime = primes[pos];
        
        // Use prime factorization structure
        double log_prime = prime_log((double)prime);
        
        for (uint32_t i = 0; i < embedding_dim; i++) {
            float freq = (float)(i + 1);
            
            // Combine multiple prime-based features
            double phase = 2.0 * PI * (double)(prime % 1000) / 1000.0;
            double scale = log_prime / prime_log((double)primes[max_length - 1]);
            
            if (i % 2 == 0) {
                encoding[i] = prime_cos(freq * phase) * (0.5f + 0.5f * scale);
            } else {
                encoding[i] = prime_sin(freq * phase) * (0.5f + 0.5f * scale);
            }
            
            // Add prime modulo pattern
            if (i % 3 == 0) {
                encoding[i] += 0.1 * prime_cos(2.0 * PI * (double)(prime % SYMMETRY_ORDER) / (double)SYMMETRY_ORDER);
            }
        }
        
        // Normalize
        double norm = 0.0;
        for (uint32_t i = 0; i < embedding_dim; i++) {
            norm += encoding[i] * encoding[i];
        }
        norm = prime_sqrt(norm);
        
        if (norm > 1e-8f) {
            for (uint32_t i = 0; i < embedding_dim; i++) {
                encoding[i] /= norm;
            }
        }
    }
    
    free(primes);
    printf("Prime encoding complete!\n");
}

/**
 * Initialize learned positional encoding
 * Starts with sinusoidal encoding that can be fine-tuned
 * 
 * @param pos_enc Positional encoding structure
 */
void cllm_initialize_learned_encoding(PositionalEncoding* pos_enc) {
    if (!pos_enc || !pos_enc->learned_positions) return;
    
    uint32_t max_length = pos_enc->max_length;
    uint32_t embedding_dim = pos_enc->embedding_dim;
    
    printf("Initializing learned positional encoding...\n");
    
    // Initialize with standard sinusoidal encoding (Vaswani et al., 2017)
    for (uint32_t pos = 0; pos < max_length; pos++) {
        double* encoding = &pos_enc->learned_positions[pos * embedding_dim];
        
        for (uint32_t i = 0; i < embedding_dim; i++) {
            double freq = 1.0 / prime_pow(10000.0, (double)(i / 2 * 2) / (double)embedding_dim);
            
            if (i % 2 == 0) {
                encoding[i] = prime_sin((float)pos * freq);
            } else {
                encoding[i] = prime_cos((float)pos * freq);
            }
        }
    }
    
    printf("Learned encoding initialized!\n");
}

/**
 * Apply complete positional encoding (combines all schemes)
 * 
 * @param inf Inference engine state
 * @param embedding Input/output embedding [embedding_dim]
 * @param position Position in sequence
 */
void cllm_apply_positional_encoding_complete(CLLMInference* inf, float* embedding, int position) {
    if (!inf || !embedding || position < 0) return;
    
    PositionalEncoding* pos_enc = &inf->model->pos_encoding;
    uint32_t embedding_dim = inf->model->embeddings.embedding_dim;
    
    if (position >= (int)pos_enc->max_length) {
        // Position beyond max_length, use modulo
        position = position % pos_enc->max_length;
    }
    
    // Combine all positional encoding schemes
    double* spiral = &pos_enc->spiral_positions[position * embedding_dim];
    double* clock = &pos_enc->clock_positions[position * embedding_dim];
    double* prime = &pos_enc->prime_positions[position * embedding_dim];
    double* learned = &pos_enc->learned_positions[position * embedding_dim];
    
    // Weighted combination
    double w_spiral = 0.25;
    double w_clock = 0.25;
    double w_prime = 0.25;
    double w_learned = 0.25;
    
    for (uint32_t i = 0; i < embedding_dim; i++) {
        double pos_encoding = w_spiral * spiral[i] +
                           w_clock * clock[i] +
                           w_prime * prime[i] +
                           w_learned * learned[i];
        
        embedding[i] += pos_encoding;
    }
}

/**
 * Generate all positional encodings for a model
 * 
 * @param model CLLM model
 */
void cllm_generate_all_positional_encodings(CLLMModel* model) {
    if (!model) return;
    
    PositionalEncoding* pos_enc = &model->pos_encoding;
    
    printf("\n=== Generating Positional Encodings ===\n");
    printf("Max length: %u\n", pos_enc->max_length);
    printf("Embedding dim: %u\n", pos_enc->embedding_dim);
    
    // Allocate memory if not already allocated
    // size_t encoding_size = pos_enc->max_length * pos_enc->embedding_dim * sizeof(float);  // Unused
    
    if (!pos_enc->spiral_positions) {
        pos_enc->spiral_positions = (double*)calloc(pos_enc->max_length * pos_enc->embedding_dim, sizeof(double));
    }
    if (!pos_enc->clock_positions) {
        pos_enc->clock_positions = (double*)calloc(pos_enc->max_length * pos_enc->embedding_dim, sizeof(double));
    }
    if (!pos_enc->prime_positions) {
        pos_enc->prime_positions = (double*)calloc(pos_enc->max_length * pos_enc->embedding_dim, sizeof(double));
    }
    if (!pos_enc->learned_positions) {
        pos_enc->learned_positions = (double*)calloc(pos_enc->max_length * pos_enc->embedding_dim, sizeof(double));
    }
    
    // Generate each encoding scheme
    cllm_generate_spiral_encoding(pos_enc);
    cllm_generate_clock_encoding(pos_enc);
    cllm_generate_prime_encoding(pos_enc);
    cllm_initialize_learned_encoding(pos_enc);
    
    printf("=== All positional encodings generated! ===\n\n");
}

/**
 * Free positional encoding memory
 * 
 * @param pos_enc Positional encoding structure
 */
void cllm_free_positional_encoding(PositionalEncoding* pos_enc) {
    if (!pos_enc) return;
    
    if (pos_enc->spiral_positions) {
        free(pos_enc->spiral_positions);
        pos_enc->spiral_positions = NULL;
    }
    if (pos_enc->clock_positions) {
        free(pos_enc->clock_positions);
        pos_enc->clock_positions = NULL;
    }
    if (pos_enc->prime_positions) {
        free(pos_enc->prime_positions);
        pos_enc->prime_positions = NULL;
    }
    if (pos_enc->learned_positions) {
        free(pos_enc->learned_positions);
        pos_enc->learned_positions = NULL;
    }
}