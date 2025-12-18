/**
 * @file space_88d.h
 * @brief 88-Dimensional Hierarchical Space Structure
 * 
 * Implements the 88D architecture (88 = 8 layers × 11 dimensions) for
 * organizing computation across vastly different magnitude scales while
 * maintaining exact arithmetic and geometric structure.
 * 
 * Key Concepts:
 * - 8 Octaves: Magnitude scales from 10^0 to 10^21
 * - 11 Dimensions per Layer: Clock positions {1,2,3,4,5,6,7,8,9,10,11}
 * - Position 0/12: Control position (not a working dimension)
 * - Platonic Solids: Coordinate frames for each layer
 * - CrystallineAbacus: Exact arbitrary precision at each position
 * 
 * Based on validated prototype: 105/105 tests passing (100%)
 */

#ifndef AI_SPACE_88D_H
#define AI_SPACE_88D_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "math/abacus.h"
#include "cllm.h"  // For PlatonicSolidType enum

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

#define SPACE88D_NUM_LAYERS 8
#define SPACE88D_DIMS_PER_LAYER 11
#define SPACE88D_TOTAL_DIMS 88  // 8 × 11

#define SPACE88D_CONTROL_POSITION 0  // Position 0/12 is control, not working dimension

/* ============================================================================
 * PLATONIC SOLID TYPES
 * ============================================================================
 */

// PlatonicSolidType is defined in cllm.h
// We use it directly here for coordinate frames

/**
 * @brief Coordinate frame using a Platonic solid
 */
typedef struct {
    PlatonicSolidType solid;  // Which solid to use
    uint8_t layer;            // 0-7 (which octave/layer)
    uint8_t num_vertices;     // Number of vertices
    uint8_t num_edges;        // Number of edges
    uint8_t num_faces;        // Number of faces
    double magnitude_scale;   // 10^(3*layer)
    double frequency;         // 432 × 12^layer Hz
} CoordinateFrame88D;

/* ============================================================================
 * 88D SPACE STRUCTURE
 * ============================================================================
 */

/**
 * @brief 88-dimensional hierarchical space
 * 
 * Structure:
 * - 8 layers (octaves) for magnitude scales
 * - 11 dimensions per layer (clock positions 1-11)
 * - Each position stores a CrystallineAbacus value
 * - Each layer has a Platonic solid coordinate frame
 */
typedef struct {
    // Core data: 8 layers × 11 dimensions
    CrystallineAbacus* layers[SPACE88D_NUM_LAYERS][SPACE88D_DIMS_PER_LAYER];
    
    // Active layer (for current operations)
    uint8_t active_layer;
    
    // Coordinate frames (one per layer)
    CoordinateFrame88D frames[SPACE88D_NUM_LAYERS];
    
    // Thread safety (per-layer locks)
    pthread_mutex_t layer_locks[SPACE88D_NUM_LAYERS];
    
    // Base for all abacus values (typically 60 for Babylonian)
    uint32_t base;
    
    // Precision for fractional values
    int32_t precision;
    
} Space88D;

/* ============================================================================
 * SPACE CREATION & DESTRUCTION
 * ============================================================================
 */

/**
 * @brief Create a new 88D space
 * @param base Base for abacus values (typically 60)
 * @param precision Precision for fractional values
 * @return Pointer to new space, or NULL on failure
 */
Space88D* space88d_create(uint32_t base, int32_t precision);

/**
 * @brief Free an 88D space
 * @param space Space to free
 */
void space88d_free(Space88D* space);

/**
 * @brief Initialize coordinate frames for all layers
 * @param space Space to initialize
 * @return 0 on success, -1 on failure
 */
int space88d_init_frames(Space88D* space);

/* ============================================================================
 * LAYER OPERATIONS
 * ============================================================================
 */

/**
 * @brief Get the appropriate layer for a value based on magnitude
 * @param value Value to check
 * @return Layer number (0-7)
 */
uint8_t space88d_get_layer_for_value(double value);

/**
 * @brief Get magnitude scale for a layer
 * @param layer Layer number (0-7)
 * @return Magnitude scale (10^(3*layer))
 */
double space88d_get_magnitude_scale(uint8_t layer);

/**
 * @brief Get frequency for a layer
 * @param layer Layer number (0-7)
 * @return Frequency in Hz (432 × 12^layer)
 */
double space88d_get_frequency(uint8_t layer);

/* ============================================================================
 * VALUE OPERATIONS
 * ============================================================================
 */

/**
 * @brief Set value at specific layer and dimension
 * @param space 88D space
 * @param layer Layer number (0-7)
 * @param dimension Dimension number (0-10)
 * @param value Value to set
 * @return 0 on success, -1 on failure
 */
int space88d_set_value(Space88D* space, uint8_t layer, uint8_t dimension,
                       const CrystallineAbacus* value);

/**
 * @brief Get value at specific layer and dimension
 * @param space 88D space
 * @param layer Layer number (0-7)
 * @param dimension Dimension number (0-10)
 * @return Pointer to abacus value, or NULL if not set
 */
const CrystallineAbacus* space88d_get_value(const Space88D* space, 
                                            uint8_t layer, uint8_t dimension);

/**
 * @brief Copy value from one position to another
 * @param space 88D space
 * @param src_layer Source layer
 * @param src_dim Source dimension
 * @param dst_layer Destination layer
 * @param dst_dim Destination dimension
 * @return 0 on success, -1 on failure
 */
int space88d_copy_value(Space88D* space,
                        uint8_t src_layer, uint8_t src_dim,
                        uint8_t dst_layer, uint8_t dst_dim);

/* ============================================================================
 * ARITHMETIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Add two values in 88D space
 * @param result Result space
 * @param a First operand
 * @param b Second operand
 * @return 0 on success, -1 on failure
 * 
 * Performs element-wise addition across all 88 dimensions
 */
int space88d_add(Space88D* result, const Space88D* a, const Space88D* b);

/**
 * @brief Multiply two values in 88D space
 * @param result Result space
 * @param a First operand
 * @param b Second operand
 * @return 0 on success, -1 on failure
 * 
 * Performs element-wise multiplication across all 88 dimensions
 */
int space88d_mul(Space88D* result, const Space88D* a, const Space88D* b);

/**
 * @brief Normalize value from one layer to another
 * @param result Result abacus
 * @param value Value to normalize
 * @param src_layer Source layer
 * @param dst_layer Destination layer
 * @return 0 on success, -1 on failure
 * 
 * Scales value by 12^(dst_layer - src_layer)
 */
int space88d_normalize_to_layer(CrystallineAbacus* result,
                                const CrystallineAbacus* value,
                                uint8_t src_layer, uint8_t dst_layer);

/* ============================================================================
 * COORDINATE FRAME OPERATIONS
 * ============================================================================
 */

/**
 * @brief Get Platonic solid for a layer
 * @param layer Layer number (0-7)
 * @return Platonic solid type
 */
PlatonicSolidType space88d_get_solid_for_layer(uint8_t layer);

/**
 * @brief Get dual of a Platonic solid
 * @param solid Platonic solid
 * @return Dual solid
 */
PlatonicSolidType space88d_get_dual_solid(PlatonicSolidType solid);

/**
 * @brief Check if solid is self-dual
 * @param solid Platonic solid
 * @return true if self-dual (tetrahedron)
 */
bool space88d_is_self_dual(PlatonicSolidType solid);

/**
 * @brief Get number of vertices for a solid
 * @param solid Platonic solid
 * @return Number of vertices
 */
uint8_t space88d_get_num_vertices(PlatonicSolidType solid);

/**
 * @brief Get number of edges for a solid
 * @param solid Platonic solid
 * @return Number of edges
 */
uint8_t space88d_get_num_edges(PlatonicSolidType solid);

/**
 * @brief Get number of faces for a solid
 * @param solid Platonic solid
 * @return Number of faces
 */
uint8_t space88d_get_num_faces(PlatonicSolidType solid);

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

/**
 * @brief Print space information for debugging
 * @param space 88D space
 */
void space88d_print(const Space88D* space);

/**
 * @brief Verify Euler's formula for a solid: V - E + F = 2
 * @param solid Platonic solid
 * @return true if formula holds
 */
bool space88d_verify_euler(PlatonicSolidType solid);

/**
 * @brief Get name of Platonic solid
 * @param solid Platonic solid
 * @return Name string
 */
const char* space88d_get_solid_name(PlatonicSolidType solid);

#ifdef __cplusplus
}
#endif

#endif // AI_SPACE_88D_H