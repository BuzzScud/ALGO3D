/**
 * @file space_88d.c
 * @brief Implementation of 88-Dimensional Hierarchical Space
 * 
 * Based on validated prototype (105/105 tests passing)
 * Adapted for integration with CLLM threading architecture
 */

#include "ai/space_88d.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

#define FUNDAMENTAL_FREQUENCY 432.0  // Hz (base frequency)

/* Layer-to-solid mapping (from thesis Section 13.5.2) */
static const PlatonicSolidType LAYER_SOLIDS[SPACE88D_NUM_LAYERS] = {
    PLATONIC_TETRAHEDRON,    // Layer 0: 10^0
    PLATONIC_CUBE,           // Layer 1: 10^3
    PLATONIC_OCTAHEDRON,     // Layer 2: 10^6
    PLATONIC_DODECAHEDRON,   // Layer 3: 10^9
    PLATONIC_ICOSAHEDRON,    // Layer 4: 10^12
    PLATONIC_CUBE,           // Layer 5: 10^15 (rotated)
    PLATONIC_OCTAHEDRON,     // Layer 6: 10^18 (rotated)
    PLATONIC_DODECAHEDRON    // Layer 7: 10^21 (rotated)
};

/* ============================================================================
 * SPACE CREATION & DESTRUCTION
 * ============================================================================
 */

Space88D* space88d_create(uint32_t base, int32_t precision) {
    Space88D* space = (Space88D*)malloc(sizeof(Space88D));
    if (!space) {
        return NULL;
    }
    
    // Initialize all fields
    memset(space, 0, sizeof(Space88D));
    space->base = base;
    space->precision = precision;
    space->active_layer = 0;
    
    // Create abacus for each position
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        for (int dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
            space->layers[layer][dim] = abacus_new(base);
            if (!space->layers[layer][dim]) {
                // Cleanup on failure
                space88d_free(space);
                return NULL;
            }
        }
        
        // Initialize per-layer mutex
        pthread_mutex_init(&space->layer_locks[layer], NULL);
    }
    
    // Initialize coordinate frames
    if (space88d_init_frames(space) != 0) {
        space88d_free(space);
        return NULL;
    }
    
    return space;
}

void space88d_free(Space88D* space) {
    if (!space) return;
    
    // Free all abacus values
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        for (int dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
            if (space->layers[layer][dim]) {
                abacus_free(space->layers[layer][dim]);
                space->layers[layer][dim] = NULL;
            }
        }
        
        // Destroy mutex
        pthread_mutex_destroy(&space->layer_locks[layer]);
    }
    
    free(space);
}

int space88d_init_frames(Space88D* space) {
    if (!space) return -1;
    
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        CoordinateFrame88D* frame = &space->frames[layer];
        
        frame->solid = LAYER_SOLIDS[layer];
        frame->layer = layer;
        frame->num_vertices = space88d_get_num_vertices(frame->solid);
        frame->num_edges = space88d_get_num_edges(frame->solid);
        frame->num_faces = space88d_get_num_faces(frame->solid);
        frame->magnitude_scale = space88d_get_magnitude_scale(layer);
        frame->frequency = space88d_get_frequency(layer);
    }
    
    return 0;
}

/* ============================================================================
 * LAYER OPERATIONS
 * ============================================================================
 */

uint8_t space88d_get_layer_for_value(double value) {
    if (value == 0.0) return 0;
    
    double abs_value = math_abs(value);
    
    // Determine layer based on magnitude
    // Layer n handles values in range [10^(3n), 10^(3(n+1)))
    if (abs_value < 1e3) return 0;
    if (abs_value < 1e6) return 1;
    if (abs_value < 1e9) return 2;
    if (abs_value < 1e12) return 3;
    if (abs_value < 1e15) return 4;
    if (abs_value < 1e18) return 5;
    if (abs_value < 1e21) return 6;
    return 7;
}

double space88d_get_magnitude_scale(uint8_t layer) {
    if (layer >= SPACE88D_NUM_LAYERS) return 0.0;
    
    // Magnitude scale: 10^(3*layer)
    double scale = 1.0;
    for (int i = 0; i < layer; i++) {
        scale *= 1000.0;  // 10^3
    }
    return scale;
}

double space88d_get_frequency(uint8_t layer) {
    if (layer >= SPACE88D_NUM_LAYERS) return 0.0;
    
    // Frequency: 432 × 12^layer Hz
    double freq = FUNDAMENTAL_FREQUENCY;
    for (int i = 0; i < layer; i++) {
        freq *= 12.0;
    }
    return freq;
}

/* ============================================================================
 * VALUE OPERATIONS
 * ============================================================================
 */

int space88d_set_value(Space88D* space, uint8_t layer, uint8_t dimension,
                       const CrystallineAbacus* value) {
    if (!space || !value) return -1;
    if (layer >= SPACE88D_NUM_LAYERS) return -1;
    if (dimension >= SPACE88D_DIMS_PER_LAYER) return -1;
    
    // Lock the layer
    pthread_mutex_lock(&space->layer_locks[layer]);
    
    // Copy value
    CrystallineAbacus* target = space->layers[layer][dimension];
    if (!target) {
        pthread_mutex_unlock(&space->layer_locks[layer]);
        return -1;
    }
    
    // Copy value using abacus operations
    // First, free the existing target
    abacus_free(target);
    
    // Create a copy of the source value
    space->layers[layer][dimension] = abacus_copy(value);
    
    pthread_mutex_unlock(&space->layer_locks[layer]);
    
    return (space->layers[layer][dimension] != NULL) ? 0 : -1;
}

const CrystallineAbacus* space88d_get_value(const Space88D* space,
                                            uint8_t layer, uint8_t dimension) {
    if (!space) return NULL;
    if (layer >= SPACE88D_NUM_LAYERS) return NULL;
    if (dimension >= SPACE88D_DIMS_PER_LAYER) return NULL;
    
    return space->layers[layer][dimension];
}

int space88d_copy_value(Space88D* space,
                        uint8_t src_layer, uint8_t src_dim,
                        uint8_t dst_layer, uint8_t dst_dim) {
    if (!space) return -1;
    if (src_layer >= SPACE88D_NUM_LAYERS || dst_layer >= SPACE88D_NUM_LAYERS) return -1;
    if (src_dim >= SPACE88D_DIMS_PER_LAYER || dst_dim >= SPACE88D_DIMS_PER_LAYER) return -1;
    
    const CrystallineAbacus* src = space->layers[src_layer][src_dim];
    
    if (!src) return -1;
    
    // Free existing destination
    if (space->layers[dst_layer][dst_dim]) {
        abacus_free(space->layers[dst_layer][dst_dim]);
    }
    
    // Create copy
    space->layers[dst_layer][dst_dim] = abacus_copy(src);
    
    return (space->layers[dst_layer][dst_dim] != NULL) ? 0 : -1;
}

/* ============================================================================
 * ARITHMETIC OPERATIONS
 * ============================================================================
 */

int space88d_add(Space88D* result, const Space88D* a, const Space88D* b) {
    if (!result || !a || !b) return -1;
    
    // Add layer by layer, dimension by dimension
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        pthread_mutex_lock(&result->layer_locks[layer]);
        
        for (int dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
            CrystallineAbacus* r = result->layers[layer][dim];
            const CrystallineAbacus* av = a->layers[layer][dim];
            const CrystallineAbacus* bv = b->layers[layer][dim];
            
            if (!r || !av || !bv) {
                pthread_mutex_unlock(&result->layer_locks[layer]);
                return -1;
            }
            
            // Perform exact addition using abacus
            MathError err = abacus_add(r, av, bv);
            if (err != MATH_SUCCESS) {
                pthread_mutex_unlock(&result->layer_locks[layer]);
                return -1;
            }
        }
        
        pthread_mutex_unlock(&result->layer_locks[layer]);
    }
    
    return 0;
}

int space88d_mul(Space88D* result, const Space88D* a, const Space88D* b) {
    if (!result || !a || !b) return -1;
    
    // Multiply layer by layer, dimension by dimension
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        pthread_mutex_lock(&result->layer_locks[layer]);
        
        for (int dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
            CrystallineAbacus* r = result->layers[layer][dim];
            const CrystallineAbacus* av = a->layers[layer][dim];
            const CrystallineAbacus* bv = b->layers[layer][dim];
            
            if (!r || !av || !bv) {
                pthread_mutex_unlock(&result->layer_locks[layer]);
                return -1;
            }
            
            // Perform exact multiplication using abacus
            MathError err = abacus_mul(r, av, bv);
            if (err != MATH_SUCCESS) {
                pthread_mutex_unlock(&result->layer_locks[layer]);
                return -1;
            }
        }
        
        pthread_mutex_unlock(&result->layer_locks[layer]);
    }
    
    return 0;
}

int space88d_normalize_to_layer(CrystallineAbacus* result,
                                const CrystallineAbacus* value,
                                uint8_t src_layer, uint8_t dst_layer) {
    if (!result || !value) return -1;
    if (src_layer >= SPACE88D_NUM_LAYERS || dst_layer >= SPACE88D_NUM_LAYERS) return -1;
    
    // Copy value first
    CrystallineAbacus* temp = abacus_copy(value);
    if (!temp) return -1;
    
    // Copy temp to result
    abacus_free(result);
    result = temp;
    
    MathError err = MATH_SUCCESS;
    
    if (src_layer == dst_layer) {
        return 0;  // Already at target layer
    }
    
    // Calculate scale factor: 12^(dst_layer - src_layer)
    int layer_diff = dst_layer - src_layer;
    
    // Create scale factor as abacus
    CrystallineAbacus* scale = abacus_from_uint64(12, value->base);
    if (!scale) return -1;
    
    // Compute 12^|layer_diff|
    CrystallineAbacus* power = abacus_new(value->base);
    if (!power) {
        abacus_free(scale);
        return -1;
    }
    
    // Start with 1
    CrystallineAbacus* one = abacus_from_uint64(1, value->base);
    if (!one) {
        abacus_free(scale);
        abacus_free(power);
        return -1;
    }
    // power starts as 1
    // No need to copy, power is already initialized to one
    abacus_free(one);
    
    // Multiply by 12, |layer_diff| times
    for (int i = 0; i < abs(layer_diff); i++) {
        CrystallineAbacus* temp = abacus_new(value->base);
        if (!temp) {
            abacus_free(scale);
            abacus_free(power);
            return -1;
        }
        
        err = abacus_mul(temp, power, scale);
        if (err != MATH_SUCCESS) {
            abacus_free(temp);
            abacus_free(scale);
            abacus_free(power);
            return -1;
        }
        
        // Replace power with temp
        abacus_free(power);
        power = temp;
    }
    
    // Apply scaling
    if (layer_diff > 0) {
        // Scaling up: multiply
        CrystallineAbacus* temp = abacus_new(value->base);
        if (!temp) {
            abacus_free(scale);
            abacus_free(power);
            return -1;
        }
        
        err = abacus_mul(temp, result, power);
        if (err == MATH_SUCCESS) {
            // Replace result with temp
            abacus_free(result);
            result = temp;
        } else {
            abacus_free(temp);
        }
    } else {
        // Scaling down: divide
        CrystallineAbacus* quotient = abacus_new(value->base);
        CrystallineAbacus* remainder = abacus_new(value->base);
        if (!quotient || !remainder) {
            abacus_free(scale);
            abacus_free(power);
            if (quotient) abacus_free(quotient);
            if (remainder) abacus_free(remainder);
            return -1;
        }
        
        err = abacus_div(quotient, remainder, result, power);
        if (err == MATH_SUCCESS) {
            // Replace result with quotient
            abacus_free(result);
            result = quotient;
            abacus_free(remainder);
        } else {
            abacus_free(quotient);
            abacus_free(remainder);
        }
    }
    
    abacus_free(scale);
    abacus_free(power);
    
    return (err == MATH_SUCCESS) ? 0 : -1;
}

/* ============================================================================
 * COORDINATE FRAME OPERATIONS
 * ============================================================================
 */

PlatonicSolidType space88d_get_solid_for_layer(uint8_t layer) {
    if (layer >= SPACE88D_NUM_LAYERS) return PLATONIC_TETRAHEDRON;
    return LAYER_SOLIDS[layer];
}

PlatonicSolidType space88d_get_dual_solid(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON:  return PLATONIC_TETRAHEDRON;  // Self-dual
        case PLATONIC_CUBE:         return PLATONIC_OCTAHEDRON;
        case PLATONIC_OCTAHEDRON:   return PLATONIC_CUBE;
        case PLATONIC_DODECAHEDRON: return PLATONIC_ICOSAHEDRON;
        case PLATONIC_ICOSAHEDRON:  return PLATONIC_DODECAHEDRON;
        default:                    return PLATONIC_TETRAHEDRON;
    }
}

bool space88d_is_self_dual(PlatonicSolidType solid) {
    return (solid == PLATONIC_TETRAHEDRON);
}

uint8_t space88d_get_num_vertices(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON:  return 4;
        case PLATONIC_CUBE:         return 8;
        case PLATONIC_OCTAHEDRON:   return 6;
        case PLATONIC_DODECAHEDRON: return 20;
        case PLATONIC_ICOSAHEDRON:  return 12;
        default:                    return 0;
    }
}

uint8_t space88d_get_num_edges(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON:  return 6;
        case PLATONIC_CUBE:         return 12;
        case PLATONIC_OCTAHEDRON:   return 12;
        case PLATONIC_DODECAHEDRON: return 30;
        case PLATONIC_ICOSAHEDRON:  return 30;
        default:                    return 0;
    }
}

uint8_t space88d_get_num_faces(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON:  return 4;
        case PLATONIC_CUBE:         return 6;
        case PLATONIC_OCTAHEDRON:   return 8;
        case PLATONIC_DODECAHEDRON: return 12;
        case PLATONIC_ICOSAHEDRON:  return 20;
        default:                    return 0;
    }
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

const char* space88d_get_solid_name(PlatonicSolidType solid) {
    switch (solid) {
        case PLATONIC_TETRAHEDRON:  return "Tetrahedron";
        case PLATONIC_CUBE:         return "Cube";
        case PLATONIC_OCTAHEDRON:   return "Octahedron";
        case PLATONIC_DODECAHEDRON: return "Dodecahedron";
        case PLATONIC_ICOSAHEDRON:  return "Icosahedron";
        default:                    return "Unknown";
    }
}

bool space88d_verify_euler(PlatonicSolidType solid) {
    uint8_t v = space88d_get_num_vertices(solid);
    uint8_t e = space88d_get_num_edges(solid);
    uint8_t f = space88d_get_num_faces(solid);
    
    // Euler's formula: V - E + F = 2
    return (v - e + f == 2);
}

void space88d_print(const Space88D* space) {
    if (!space) return;
    
    printf("=== 88D Space ===\n");
    printf("Base: %u\n", space->base);
    printf("Precision: %d\n", space->precision);
    printf("Active Layer: %u\n", space->active_layer);
    printf("\n");
    
    for (int layer = 0; layer < SPACE88D_NUM_LAYERS; layer++) {
        const CoordinateFrame88D* frame = &space->frames[layer];
        printf("Layer %d: %s\n", layer, space88d_get_solid_name(frame->solid));
        printf("  Magnitude: %.0e\n", frame->magnitude_scale);
        printf("  Frequency: %.2f Hz\n", frame->frequency);
        printf("  V=%u, E=%u, F=%u\n", 
               frame->num_vertices, frame->num_edges, frame->num_faces);
        
        // Print non-zero values
        int non_zero = 0;
        for (int dim = 0; dim < SPACE88D_DIMS_PER_LAYER; dim++) {
            const CrystallineAbacus* val = space->layers[layer][dim];
            if (val && !abacus_is_zero(val)) {
                non_zero++;
            }
        }
        printf("  Non-zero dimensions: %d/%d\n", non_zero, SPACE88D_DIMS_PER_LAYER);
        printf("\n");
    }
}