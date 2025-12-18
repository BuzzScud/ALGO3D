#include "ai/cllm_space88d_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// LAYER SELECTION & MANAGEMENT
// ============================================================================

int cllm_space88d_select_layer(CLLMLatticeHierarchy* sphere, 
                               const CrystallineAbacus* value) {
    if (!sphere || !sphere->space || !value) {
        return -1;
    }
    
    // Convert value to double to determine magnitude
    double magnitude = 0.0;
    uint64_t uint_val = 0;
    
    if (abacus_to_uint64(value, &uint_val) == MATH_SUCCESS) {
        magnitude = (double)uint_val;
    }
    
    // Select layer based on magnitude
    // Layer 0: 0-999 (10^0 to 10^3)
    // Layer 1: 1000-999999 (10^3 to 10^6)
    // Layer 2: 1M-999M (10^6 to 10^9)
    // etc.
    uint8_t layer = 0;
    if (magnitude >= 1e21) layer = 7;
    else if (magnitude >= 1e18) layer = 6;
    else if (magnitude >= 1e15) layer = 5;
    else if (magnitude >= 1e12) layer = 4;
    else if (magnitude >= 1e9) layer = 3;
    else if (magnitude >= 1e6) layer = 2;
    else if (magnitude >= 1e3) layer = 1;
    else layer = 0;
    
    sphere->space->active_layer = layer;
    return layer;
}

int cllm_space88d_get_active_layer(const CLLMLatticeHierarchy* sphere) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    return sphere->space->active_layer;
}

int cllm_space88d_set_active_layer(CLLMLatticeHierarchy* sphere, uint8_t layer) {
    if (!sphere || !sphere->space || layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    sphere->space->active_layer = layer;
    return 0;
}

// ============================================================================
// LAYER-BASED ARITHMETIC
// ============================================================================

int cllm_space88d_add_to_position(CLLMLatticeHierarchy* sphere,
                                  uint8_t layer, uint8_t dimension,
                                  const CrystallineAbacus* value) {
    if (!sphere || !sphere->space || !value) {
        return -1;
    }
    
    if (layer >= SPACE88D_NUM_LAYERS || dimension >= SPACE88D_DIMS_PER_LAYER) {
        return -1;
    }
    
    // Get current value
    CrystallineAbacus* current = sphere->space->layers[layer][dimension];
    if (!current) {
        // No value yet, just set it
        return space88d_set_value(sphere->space, layer, dimension, value);
    }
    
    // Add to current value
    CrystallineAbacus* result = abacus_new(sphere->space->base);
    if (!result) {
        return -1;
    }
    
    MathError err = abacus_add(result, current, value);
    if (err != MATH_SUCCESS) {
        abacus_free(result);
        return -1;
    }
    
    // Replace current with result
    abacus_free(current);
    sphere->space->layers[layer][dimension] = result;
    
    return 0;
}

int cllm_space88d_multiply_position(CLLMLatticeHierarchy* sphere,
                                    uint8_t layer, uint8_t dimension,
                                    const CrystallineAbacus* value) {
    if (!sphere || !sphere->space || !value) {
        return -1;
    }
    
    if (layer >= SPACE88D_NUM_LAYERS || dimension >= SPACE88D_DIMS_PER_LAYER) {
        return -1;
    }
    
    CrystallineAbacus* current = sphere->space->layers[layer][dimension];
    if (!current) {
        return -1;
    }
    
    CrystallineAbacus* result = abacus_new(sphere->space->base);
    if (!result) {
        return -1;
    }
    
    MathError err = abacus_mul(result, current, value);
    if (err != MATH_SUCCESS) {
        abacus_free(result);
        return -1;
    }
    
    abacus_free(current);
    sphere->space->layers[layer][dimension] = result;
    
    return 0;
}

int cllm_space88d_scale_layer(CLLMLatticeHierarchy* sphere,
                              uint8_t layer,
                              const CrystallineAbacus* scale_factor) {
    if (!sphere || !sphere->space || !scale_factor) {
        return -1;
    }
    
    if (layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    
    // Scale each dimension in the layer
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        if (sphere->space->layers[layer][dim]) {
            if (cllm_space88d_multiply_position(sphere, layer, dim, scale_factor) != 0) {
                return -1;
            }
        }
    }
    
    return 0;
}

// ============================================================================
// CROSS-LAYER OPERATIONS
// ============================================================================

int cllm_space88d_copy_layer(CLLMLatticeHierarchy* sphere,
                             uint8_t src_layer, uint8_t dst_layer) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    
    if (src_layer >= SPACE88D_NUM_LAYERS || dst_layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    
    // Copy each dimension
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        CrystallineAbacus* src = sphere->space->layers[src_layer][dim];
        if (src) {
            CrystallineAbacus* copy = abacus_copy(src);
            if (!copy) {
                return -1;
            }
            
            // Free existing value if any
            if (sphere->space->layers[dst_layer][dim]) {
                abacus_free(sphere->space->layers[dst_layer][dim]);
            }
            
            sphere->space->layers[dst_layer][dim] = copy;
        }
    }
    
    return 0;
}

int cllm_space88d_add_layers(CLLMLatticeHierarchy* sphere,
                             uint8_t layer1, uint8_t layer2,
                             uint8_t dst_layer) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    
    if (layer1 >= SPACE88D_NUM_LAYERS || 
        layer2 >= SPACE88D_NUM_LAYERS || 
        dst_layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    
    // Add each dimension
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        CrystallineAbacus* val1 = sphere->space->layers[layer1][dim];
        CrystallineAbacus* val2 = sphere->space->layers[layer2][dim];
        
        if (val1 && val2) {
            CrystallineAbacus* result = abacus_new(sphere->space->base);
            if (!result) {
                return -1;
            }
            
            MathError err = abacus_add(result, val1, val2);
            if (err != MATH_SUCCESS) {
                abacus_free(result);
                return -1;
            }
            
            // Free existing value if any
            if (sphere->space->layers[dst_layer][dim]) {
                abacus_free(sphere->space->layers[dst_layer][dim]);
            }
            
            sphere->space->layers[dst_layer][dim] = result;
        }
    }
    
    return 0;
}

CrystallineAbacus* cllm_space88d_layer_dot_product(
    const CLLMLatticeHierarchy* sphere,
    uint8_t layer1, uint8_t layer2) {
    
    if (!sphere || !sphere->space) {
        return NULL;
    }
    
    if (layer1 >= SPACE88D_NUM_LAYERS || layer2 >= SPACE88D_NUM_LAYERS) {
        return NULL;
    }
    
    CrystallineAbacus* sum = abacus_new(sphere->space->base);
    if (!sum) {
        return NULL;
    }
    
    // Initialize sum to zero
    CrystallineAbacus* zero = abacus_from_uint64(0, sphere->space->base);
    if (!zero) {
        abacus_free(sum);
        return NULL;
    }
    
    MathError err = abacus_add(sum, sum, zero);
    abacus_free(zero);
    
    if (err != MATH_SUCCESS) {
        abacus_free(sum);
        return NULL;
    }
    
    // Compute dot product
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        CrystallineAbacus* val1 = sphere->space->layers[layer1][dim];
        CrystallineAbacus* val2 = sphere->space->layers[layer2][dim];
        
        if (val1 && val2) {
            CrystallineAbacus* product = abacus_new(sphere->space->base);
            if (!product) {
                abacus_free(sum);
                return NULL;
            }
            
            err = abacus_mul(product, val1, val2);
            if (err != MATH_SUCCESS) {
                abacus_free(product);
                abacus_free(sum);
                return NULL;
            }
            
            CrystallineAbacus* new_sum = abacus_new(sphere->space->base);
            if (!new_sum) {
                abacus_free(product);
                abacus_free(sum);
                return NULL;
            }
            
            err = abacus_add(new_sum, sum, product);
            abacus_free(product);
            
            if (err != MATH_SUCCESS) {
                abacus_free(new_sum);
                abacus_free(sum);
                return NULL;
            }
            
            abacus_free(sum);
            sum = new_sum;
        }
    }
    
    return sum;
}

int cllm_space88d_transfer_between_layers(CLLMLatticeHierarchy* sphere,
                                          uint8_t src_layer,
                                          uint8_t dst_layer,
                                          uint8_t dimension) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    
    if (src_layer >= SPACE88D_NUM_LAYERS || 
        dst_layer >= SPACE88D_NUM_LAYERS ||
        dimension >= SPACE88D_DIMS_PER_LAYER) {
        return -1;
    }
    
    CrystallineAbacus* src_val = sphere->space->layers[src_layer][dimension];
    if (!src_val) {
        return -1;
    }
    
    // Use space88d_normalize_to_layer to handle magnitude scaling
    CrystallineAbacus* scaled = abacus_new(sphere->space->base);
    if (!scaled) {
        return -1;
    }
    
    int result = space88d_normalize_to_layer(scaled, src_val, src_layer, dst_layer);
    if (result != 0) {
        abacus_free(scaled);
        return -1;
    }
    
    // Free existing value if any
    if (sphere->space->layers[dst_layer][dimension]) {
        abacus_free(sphere->space->layers[dst_layer][dimension]);
    }
    
    sphere->space->layers[dst_layer][dimension] = scaled;
    
    // Zero out source
    abacus_free(src_val);
    sphere->space->layers[src_layer][dimension] = NULL;
    
    return 0;
}

// ============================================================================
// LAYER INFORMATION
// ============================================================================

double cllm_space88d_get_layer_scale(uint8_t layer) {
    return space88d_get_magnitude_scale(layer);
}

PlatonicSolidType cllm_space88d_get_layer_solid(uint8_t layer) {
    return space88d_get_solid_for_layer(layer);
}

double cllm_space88d_get_layer_frequency(uint8_t layer) {
    return space88d_get_frequency(layer);
}

// ============================================================================
// DIMENSION OPERATIONS
// ============================================================================

const CrystallineAbacus* cllm_space88d_get_value(
    const CLLMLatticeHierarchy* sphere,
    uint8_t layer, uint8_t dimension) {
    
    return space88d_get_value(sphere->space, layer, dimension);
}

int cllm_space88d_set_value(CLLMLatticeHierarchy* sphere,
                            uint8_t layer, uint8_t dimension,
                            const CrystallineAbacus* value) {
    
    return space88d_set_value(sphere->space, layer, dimension, value);
}

int cllm_space88d_zero_dimension(CLLMLatticeHierarchy* sphere,
                                 uint8_t dimension) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    
    if (dimension >= SPACE88D_DIMS_PER_LAYER) {
        return -1;
    }
    
    CrystallineAbacus* zero = abacus_from_uint64(0, sphere->space->base);
    if (!zero) {
        return -1;
    }
    
    for (uint8_t layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        if (space88d_set_value(sphere->space, layer, dimension, zero) != 0) {
            abacus_free(zero);
            return -1;
        }
    }
    
    abacus_free(zero);
    return 0;
}

int cllm_space88d_zero_layer(CLLMLatticeHierarchy* sphere, uint8_t layer) {
    if (!sphere || !sphere->space) {
        return -1;
    }
    
    if (layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    
    CrystallineAbacus* zero = abacus_from_uint64(0, sphere->space->base);
    if (!zero) {
        return -1;
    }
    
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        if (space88d_set_value(sphere->space, layer, dim, zero) != 0) {
            abacus_free(zero);
            return -1;
        }
    }
    
    abacus_free(zero);
    return 0;
}

// ============================================================================
// DEBUGGING & DIAGNOSTICS
// ============================================================================

void cllm_space88d_print_layer(const CLLMLatticeHierarchy* sphere,
                               uint8_t layer) {
    if (!sphere || !sphere->space || layer >= SPACE88D_NUM_LAYERS) {
        return;
    }
    
    printf("Layer %u (Scale: %.0e, Solid: %s, Freq: %.2f Hz):\n",
           layer,
           cllm_space88d_get_layer_scale(layer),
           space88d_get_solid_name(cllm_space88d_get_layer_solid(layer)),
           cllm_space88d_get_layer_frequency(layer));
    
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        const CrystallineAbacus* val = sphere->space->layers[layer][dim];
        printf("  Dim %2u: ", dim);
        if (val) {
            abacus_print(val);
        } else {
            printf("(null)\n");
        }
    }
}

void cllm_space88d_print_all(const CLLMLatticeHierarchy* sphere) {
    if (!sphere || !sphere->space) {
        return;
    }
    
    printf("========================================\n");
    printf("Space88D for Sphere %d\n", sphere->sphere_id);
    printf("Active Layer: %u\n", sphere->space->active_layer);
    printf("Base: %u, Precision: %d\n", 
           sphere->space->base, sphere->space->precision);
    printf("========================================\n\n");
    
    for (uint8_t layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        cllm_space88d_print_layer(sphere, layer);
        printf("\n");
    }
}

int cllm_space88d_layer_stats(const CLLMLatticeHierarchy* sphere,
                              uint8_t layer,
                              CrystallineAbacus** min,
                              CrystallineAbacus** max,
                              CrystallineAbacus** sum) {
    if (!sphere || !sphere->space || !min || !max || !sum) {
        return -1;
    }
    
    if (layer >= SPACE88D_NUM_LAYERS) {
        return -1;
    }
    
    *min = NULL;
    *max = NULL;
    *sum = abacus_from_uint64(0, sphere->space->base);
    
    if (!*sum) {
        return -1;
    }
    
    for (uint8_t dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
        const CrystallineAbacus* val = sphere->space->layers[layer][dim];
        if (val) {
            // Update min
            if (!*min) {
                *min = abacus_copy(val);
            } else {
                if (abacus_compare(*min, val) > 0) {
                    abacus_free(*min);
                    *min = abacus_copy(val);
                }
            }
            
            // Update max
            if (!*max) {
                *max = abacus_copy(val);
            } else {
                if (abacus_compare(*max, val) < 0) {
                    abacus_free(*max);
                    *max = abacus_copy(val);
                }
            }
            
            // Update sum
            CrystallineAbacus* new_sum = abacus_new(sphere->space->base);
            if (!new_sum) {
                if (*min) abacus_free(*min);
                if (*max) abacus_free(*max);
                abacus_free(*sum);
                return -1;
            }
            
            MathError err = abacus_add(new_sum, *sum, val);
            if (err != MATH_SUCCESS) {
                abacus_free(new_sum);
                if (*min) abacus_free(*min);
                if (*max) abacus_free(*max);
                abacus_free(*sum);
                return -1;
            }
            
            abacus_free(*sum);
            *sum = new_sum;
        }
    }
    
    return 0;
}