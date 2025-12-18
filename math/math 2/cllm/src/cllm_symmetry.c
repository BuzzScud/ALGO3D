#include "math/types.h"
#include "math/constants.h"
/*
 * CLLM Symmetry Operations
 * Implements symmetry group transformations for lattice embeddings
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai/cllm.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"

#define SYMMETRY_ORDER 12

/**
 * Apply rotation transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param angle Rotation angle in radians
 * @param dim Embedding dimension
 */
static void apply_rotation(double* embedding, double angle, int dim) {
    if (!embedding || dim < 2) return;
    
    double cos_a = math_cos(angle);
    double sin_a = math_sin(angle);
    
    // Apply rotation in pairs of dimensions
    for (int i = 0; i < dim - 1; i += 2) {
        double x = embedding[i];
        double y = embedding[i + 1];
        
        embedding[i] = cos_a * x - sin_a * y;
        embedding[i + 1] = sin_a * x + cos_a * y;
    }
}

/**
 * Apply reflection transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param axis Reflection axis (0 = x, 1 = y, 2 = z, etc.)
 * @param dim Embedding dimension
 */
static void apply_reflection(double* embedding, int axis, int dim) {
    if (!embedding || axis < 0 || axis >= dim) return;
    
    // Reflect along specified axis
    embedding[axis] = -embedding[axis];
}

/**
 * Apply scaling transformation to embedding
 * 
 * @param embedding Input/output embedding [dim]
 * @param scale Scale factor
 * @param dim Embedding dimension
 */
static void apply_scaling(double* embedding, double scale, int dim) {
    if (!embedding || dim <= 0) return;
    
    for (int i = 0; i < dim; i++) {
        embedding[i] *= scale;
    }
}

/**
 * Compute symmetry group for a prime number
 * Maps prime to one of 12 symmetry groups
 * 
 * @param prime Prime number
 * @return Symmetry group (0 to SYMMETRY_ORDER-1)
 */
uint32_t cllm_compute_symmetry_group(uint64_t prime) {
    return (uint32_t)(prime % SYMMETRY_ORDER);
}

/**
 * Apply symmetry transformation based on group
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group (0 to SYMMETRY_ORDER-1)
 * @param dim Embedding dimension
 */
void cllm_apply_symmetry_transform(double* embedding, int symmetry_group, int dim) {
    if (!embedding || symmetry_group < 0 || symmetry_group >= SYMMETRY_ORDER || dim <= 0) {
        return;
    }
    
    // Each symmetry group applies a different transformation
    double angle = 2.0 * MATH_PI * (double)symmetry_group / (double)SYMMETRY_ORDER;
    
    switch (symmetry_group) {
        case 0:
            // Identity - no transformation
            break;
            
        case 1:
        case 5:
        case 7:
        case 11:
            // Rotation groups
            apply_rotation(embedding, angle, dim);
            break;
            
        case 2:
        case 4:
        case 8:
        case 10:
            // Rotation + reflection
            apply_rotation(embedding, angle, dim);
            apply_reflection(embedding, 0, dim);
            break;
            
        case 3:
        case 9:
            // Rotation + scaling
            apply_rotation(embedding, angle, dim);
            apply_scaling(embedding, 1.1, dim);
            break;
            
        case 6:
            // Pure reflection (mirror symmetry)
            apply_reflection(embedding, 0, dim);
            apply_reflection(embedding, 1, dim);
            break;
            
        default:
            // Fallback: simple rotation
            apply_rotation(embedding, angle, dim);
            break;
    }
}

/**
 * Apply inverse symmetry transformation
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group (0 to SYMMETRY_ORDER-1)
 * @param dim Embedding dimension
 */
void cllm_apply_inverse_symmetry_transform(double* embedding, int symmetry_group, int dim) {
    if (!embedding || symmetry_group < 0 || symmetry_group >= SYMMETRY_ORDER || dim <= 0) {
        return;
    }
    
    // Apply inverse transformation (reverse order, negative angles)
    double angle = -2.0 * MATH_PI * (double)symmetry_group / (double)SYMMETRY_ORDER;
    
    switch (symmetry_group) {
        case 0:
            // Identity - no transformation
            break;
            
        case 1:
        case 5:
        case 7:
        case 11:
            // Inverse rotation
            apply_rotation(embedding, angle, dim);
            break;
            
        case 2:
        case 4:
        case 8:
        case 10:
            // Inverse: reflection first, then inverse rotation
            apply_reflection(embedding, 0, dim);
            apply_rotation(embedding, angle, dim);
            break;
            
        case 3:
        case 9:
            // Inverse: scaling, then inverse rotation
            apply_scaling(embedding, 1.0 / 1.1, dim);
            apply_rotation(embedding, angle, dim);
            break;
            
        case 6:
            // Reflection is self-inverse
            apply_reflection(embedding, 0, dim);
            apply_reflection(embedding, 1, dim);
            break;
            
        default:
            // Fallback: inverse rotation
            apply_rotation(embedding, angle, dim);
            break;
    }
}

/**
 * Compute symmetry-invariant features
 * Extracts features that are invariant under symmetry transformations
 * 
 * @param embedding Input embedding [dim]
 * @param dim Embedding dimension
 * @param features Output features [num_features]
 * @param num_features Number of features to extract
 */
void cllm_compute_symmetry_invariants(double* embedding, int dim, 
                                     double* features, int num_features) {
    if (!embedding || !features || dim <= 0 || num_features <= 0) return;
    
    // Feature 0: L2 norm (rotation invariant)
    if (num_features > 0) {
        double norm = 0.0;
        for (int i = 0; i < dim; i++) {
            norm += embedding[i] * embedding[i];
        }
        features[0] = math_sqrt(norm);
    }
    
    // Feature 1: Sum of absolute values (reflection invariant)
    if (num_features > 1) {
        double sum = 0.0;
        for (int i = 0; i < dim; i++) {
            sum += (embedding[i] >= 0) ? embedding[i] : -embedding[i];
        }
        features[1] = sum;
    }
    
    // Feature 2: Product of signs (parity)
    if (num_features > 2) {
        int sign_product = 1;
        for (int i = 0; i < dim; i++) {
            if (embedding[i] < 0) sign_product *= -1;
        }
        features[2] = (double)sign_product;
    }
    
    // Feature 3: Maximum absolute value
    if (num_features > 3) {
        double max_abs = 0.0;
        for (int i = 0; i < dim; i++) {
            double abs_val = (embedding[i] >= 0) ? embedding[i] : -embedding[i];
            if (abs_val > max_abs) max_abs = abs_val;
        }
        features[3] = max_abs;
    }
    
    // Additional features: moments
    for (int f = 4; f < num_features && f < 8; f++) {
        double moment = 0.0;
        int power = f - 2;
        for (int i = 0; i < dim; i++) {
            double val = embedding[i];
            double powered = val;
            for (int p = 1; p < power; p++) {
                powered *= val;
            }
            moment += powered;
        }
        features[f] = moment;
    }
}

/**
 * Apply symmetry-equivariant transformation
 * Transformation that respects symmetry structure
 * 
 * @param embedding Input/output embedding [dim]
 * @param symmetry_group Symmetry group
 * @param transform_matrix Transformation matrix [dim x dim]
 * @param dim Embedding dimension
 */
void cllm_apply_equivariant_transform(double* embedding, int symmetry_group,
                                     double* transform_matrix, int dim) {
    if (!embedding || !transform_matrix || dim <= 0) return;
    
    // First apply symmetry transformation
    cllm_apply_symmetry_transform(embedding, symmetry_group, dim);
    
    // Then apply linear transformation
    double* temp = (double*)malloc(dim * sizeof(double));
    if (!temp) return;
    
    for (int i = 0; i < dim; i++) {
        double sum = 0.0;
        for (int j = 0; j < dim; j++) {
            sum += transform_matrix[i * dim + j] * embedding[j];
        }
        temp[i] = sum;
    }
    
    memcpy(embedding, temp, dim * sizeof(double));
    free(temp);
}

/**
 * Compute symmetry group compatibility
 * Measures how well two embeddings match under symmetry transformations
 * 
 * @param embedding1 First embedding [dim]
 * @param symmetry1 First symmetry group
 * @param embedding2 Second embedding [dim]
 * @param symmetry2 Second symmetry group
 * @param dim Embedding dimension
 * @return Compatibility score (0 to 1)
 */
double cllm_symmetry_compatibility(double* embedding1, int symmetry1,
                                 double* embedding2, int symmetry2, int dim) {
    if (!embedding1 || !embedding2 || dim <= 0) return 0.0;
    
    // Create transformed copies
    double* e1_transformed = (double*)malloc(dim * sizeof(double));
    double* e2_transformed = (double*)malloc(dim * sizeof(double));
    
    if (!e1_transformed || !e2_transformed) {
        if (e1_transformed) free(e1_transformed);
        if (e2_transformed) free(e2_transformed);
        return 0.0;
    }
    
    memcpy(e1_transformed, embedding1, dim * sizeof(double));
    memcpy(e2_transformed, embedding2, dim * sizeof(double));
    
    // Apply symmetry transformations
    cllm_apply_symmetry_transform(e1_transformed, symmetry1, dim);
    cllm_apply_symmetry_transform(e2_transformed, symmetry2, dim);
    
    // Compute cosine similarity
    double dot = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (int i = 0; i < dim; i++) {
        dot += e1_transformed[i] * e2_transformed[i];
        norm1 += e1_transformed[i] * e1_transformed[i];
        norm2 += e2_transformed[i] * e2_transformed[i];
    }
    
    free(e1_transformed);
    free(e2_transformed);
    
    norm1 = math_sqrt(norm1);
    norm2 = math_sqrt(norm2);
    
    if (norm1 < 1e-8 || norm2 < 1e-8) return 0.0;
    
    double similarity = dot / (norm1 * norm2);
    
    // Map to [0, 1]
    return (similarity + 1.0) / 2.0;
}

/**
 * Generate symmetry-aware attention mask
 * Creates attention mask that respects symmetry structure
 * 
 * @param symmetry_groups Array of symmetry groups [seq_len]
 * @param seq_len Sequence length
 * @param mask Output attention mask [seq_len x seq_len]
 */
void cllm_generate_symmetry_attention_mask(int* symmetry_groups, int seq_len, double* mask) {
    if (!symmetry_groups || !mask || seq_len <= 0) return;
    
    for (int i = 0; i < seq_len; i++) {
        for (int j = 0; j < seq_len; j++) {
            // Tokens in same symmetry group have higher attention weight
            if (symmetry_groups[i] == symmetry_groups[j]) {
                mask[i * seq_len + j] = 1.0;
            } else {
                // Compute group distance
                int dist = symmetry_groups[i] - symmetry_groups[j];
                if (dist < 0) dist = -dist;
                if (dist > SYMMETRY_ORDER / 2) {
                    dist = SYMMETRY_ORDER - dist;
                }
                
                // Decay based on distance
                mask[i * seq_len + j] = 1.0 / (1.0 + 0.5 * (double)dist);
            }
        }
    }
}