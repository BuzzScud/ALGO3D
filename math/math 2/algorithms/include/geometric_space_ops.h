/**
 * @file geometric_space_ops.h
 * @brief Geometric space operations for 88D computational systems
 * 
 * Provides algorithms for boundary detection, coordinate transformation,
 * and handoff operations in multi-dimensional geometric spaces. These are
 * general-purpose algorithms that can be used by any system requiring
 * geometric computation across multiple dimensions and layers.
 * 
 * Key Concepts:
 * - Boundary Detection: Identify when values approach layer/dimension boundaries
 * - Coordinate Transformation: Convert between different geometric frames
 * - Handoff: Transfer values between layers/dimensions with proper scaling
 * - Work Distribution: Distribute computation across dimensions
 */

#ifndef ALGORITHMS_GEOMETRIC_SPACE_OPS_H
#define ALGORITHMS_GEOMETRIC_SPACE_OPS_H

#include "abacus88d.h"
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BOUNDARY DETECTION
 * ============================================================================
 */

/**
 * @brief Boundary information structure
 * 
 * Contains information about a position's proximity to geometric boundaries
 * in the 88D space. Boundaries occur at:
 * - Layer transitions (magnitude scales)
 * - Dimension boundaries (clock positions)
 * - Platonic solid vertices (shared points)
 */
typedef struct {
    uint8_t current_layer;          /**< Current layer (0-7) */
    uint8_t current_dimension;      /**< Current dimension (0-10) */
    
    bool near_layer_boundary;       /**< True if near layer transition */
    uint8_t target_layer;           /**< Target layer if crossing boundary */
    double layer_distance;          /**< Distance to layer boundary (0.0-1.0) */
    
    bool near_dimension_boundary;   /**< True if near dimension boundary */
    uint8_t target_dimension;       /**< Target dimension if crossing */
    double dimension_distance;      /**< Distance to dimension boundary (0.0-1.0) */
    
    bool at_vertex;                 /**< True if at Platonic solid vertex */
    uint8_t vertex_id;              /**< Vertex ID if at vertex */
    uint8_t num_shared_dimensions;  /**< Number of dimensions sharing this vertex */
    uint8_t shared_dimensions[12];  /**< List of dimensions sharing vertex */
    
    bool requires_handoff;          /**< True if handoff is needed */
    double handoff_urgency;         /**< Urgency of handoff (0.0-1.0) */
} BoundaryInfo;

/**
 * @brief Detect boundaries for a value in 88D space
 * 
 * Analyzes a value's position in the 88D space to determine if it's
 * approaching any boundaries (layer, dimension, or vertex).
 * 
 * @param abacus88d The 88D space
 * @param layer Current layer
 * @param dimension Current dimension
 * @param value The value to analyze
 * @param boundary Output boundary information
 * @return MATH_SUCCESS or error code
 */
MathError geometric_detect_boundary(
    const Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
);

/**
 * @brief Check if value should transition to different layer
 * 
 * Determines if a value's magnitude requires it to move to a different
 * layer for optimal representation.
 * 
 * @param value The value to check
 * @param current_layer Current layer
 * @param target_layer Output: suggested target layer
 * @return true if transition recommended, false otherwise
 */
bool geometric_should_transition_layer(
    const CrystallineAbacus* value,
    uint8_t current_layer,
    uint8_t* target_layer
);

/**
 * @brief Find shared vertices between dimensions
 * 
 * Identifies vertices that are shared between multiple dimensions
 * based on the Platonic solid geometry of each layer.
 * 
 * @param abacus88d The 88D space
 * @param layer Layer to analyze
 * @param dimension Dimension to analyze
 * @param shared_dims Output: array of shared dimensions
 * @param num_shared Output: number of shared dimensions
 * @return MATH_SUCCESS or error code
 */
MathError geometric_find_shared_vertices(
    const Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    uint8_t* shared_dims,
    uint8_t* num_shared
);

/* ============================================================================
 * COORDINATE TRANSFORMATION
 * ============================================================================
 */

/**
 * @brief Transformation matrix for coordinate conversion
 * 
 * Represents the transformation needed to convert coordinates from
 * one geometric frame to another (e.g., Tetrahedron to Cube).
 */
typedef struct {
    double matrix[4][4];            /**< 4x4 transformation matrix */
    uint8_t source_layer;           /**< Source layer */
    uint8_t target_layer;           /**< Target layer */
    double scale_factor;            /**< Magnitude scale factor */
} TransformMatrix;

/**
 * @brief Calculate transformation between layers
 * 
 * Computes the transformation matrix needed to convert coordinates
 * from one layer's Platonic solid frame to another's.
 * 
 * @param source_layer Source layer (0-7)
 * @param target_layer Target layer (0-7)
 * @param transform Output transformation matrix
 * @return MATH_SUCCESS or error code
 */
MathError geometric_calculate_transform(
    uint8_t source_layer,
    uint8_t target_layer,
    TransformMatrix* transform
);

/**
 * @brief Apply transformation to a value
 * 
 * Applies a coordinate transformation to convert a value from one
 * geometric frame to another, including magnitude scaling.
 * 
 * @param value Input value
 * @param transform Transformation to apply
 * @param result Output transformed value
 * @return MATH_SUCCESS or error code
 */
MathError geometric_apply_transform(
    const CrystallineAbacus* value,
    const TransformMatrix* transform,
    CrystallineAbacus* result
);

/**
 * @brief Transform coordinates between dimensions
 * 
 * Converts a value from one dimension to another within the same layer,
 * accounting for the angular relationships on the clock lattice.
 * 
 * @param value Input value
 * @param source_dim Source dimension (0-10)
 * @param target_dim Target dimension (0-10)
 * @param result Output transformed value
 * @return MATH_SUCCESS or error code
 */
MathError geometric_transform_dimension(
    const CrystallineAbacus* value,
    uint8_t source_dim,
    uint8_t target_dim,
    CrystallineAbacus* result
);

/* ============================================================================
 * HANDOFF OPERATIONS
 * ============================================================================
 */

/**
 * @brief Handoff context for value transfer
 * 
 * Contains all information needed to transfer a value from one
 * location in 88D space to another.
 */
typedef struct {
    uint8_t source_layer;           /**< Source layer */
    uint8_t source_dimension;       /**< Source dimension */
    uint8_t target_layer;           /**< Target layer */
    uint8_t target_dimension;       /**< Target dimension */
    
    TransformMatrix transform;      /**< Transformation to apply */
    bool requires_scaling;          /**< True if magnitude scaling needed */
    double scale_factor;            /**< Scale factor if needed */
    
    bool at_shared_vertex;          /**< True if handoff at shared vertex */
    uint8_t vertex_id;              /**< Vertex ID if at shared vertex */
} HandoffContext;

/**
 * @brief Prepare handoff context
 * 
 * Analyzes boundary information and prepares the context needed
 * for a value handoff operation.
 * 
 * @param boundary Boundary information
 * @param context Output handoff context
 * @return MATH_SUCCESS or error code
 */
MathError geometric_prepare_handoff(
    const BoundaryInfo* boundary,
    HandoffContext* context
);

/**
 * @brief Execute handoff operation
 * 
 * Transfers a value from one location in 88D space to another,
 * applying necessary transformations and scaling.
 * 
 * @param source Source 88D space
 * @param target Target 88D space (can be same as source)
 * @param context Handoff context
 * @param value Value to transfer
 * @return MATH_SUCCESS or error code
 */
MathError geometric_execute_handoff(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* context,
    const CrystallineAbacus* value
);

/**
 * @brief Calculate handoff value with interpolation
 * 
 * When a value is at a boundary, calculates the interpolated value
 * that should be handed off to the neighboring region.
 * 
 * @param source_value Value at source
 * @param boundary Boundary information
 * @param handoff_value Output interpolated value
 * @return MATH_SUCCESS or error code
 */
MathError geometric_calculate_handoff_value(
    const CrystallineAbacus* source_value,
    const BoundaryInfo* boundary,
    CrystallineAbacus* handoff_value
);

/* ============================================================================
 * WORK DISTRIBUTION
 * ============================================================================
 */

/**
 * @brief Work item for distribution
 * 
 * Represents a unit of work to be distributed across dimensions.
 */
typedef struct {
    uint8_t layer;                  /**< Layer for this work */
    uint8_t dimension;              /**< Dimension for this work */
    uint32_t work_size;             /**< Size of work (arbitrary units) */
    uint32_t priority;              /**< Priority (higher = more urgent) */
    void* data;                     /**< Application-specific data */
} WorkItem;

/**
 * @brief Work distribution result
 * 
 * Contains the result of work distribution across dimensions.
 */
typedef struct {
    uint32_t num_workers;           /**< Number of workers */
    uint32_t* worker_loads;         /**< Load per worker */
    WorkItem** worker_items;        /**< Items assigned to each worker */
    uint32_t* items_per_worker;     /**< Number of items per worker */
} WorkDistribution;

/**
 * @brief Calculate optimal work distribution
 * 
 * Distributes work items across dimensions/workers to balance load
 * and minimize boundary crossings.
 * 
 * @param abacus88d The 88D space
 * @param items Array of work items
 * @param num_items Number of work items
 * @param num_workers Number of workers
 * @param distribution Output distribution
 * @return MATH_SUCCESS or error code
 */
MathError geometric_distribute_work(
    const Abacus88D* abacus88d,
    const WorkItem* items,
    uint32_t num_items,
    uint32_t num_workers,
    WorkDistribution* distribution
);

/**
 * @brief Free work distribution
 * 
 * Frees memory allocated for work distribution.
 * 
 * @param distribution Distribution to free
 */
void geometric_free_distribution(WorkDistribution* distribution);

/* ============================================================================
 * THREAD-SAFE OPERATIONS
 * ============================================================================
 */

/**
 * @brief Thread-safe boundary detection
 * 
 * Same as geometric_detect_boundary but with proper locking for
 * concurrent access to the 88D space.
 * 
 * @param abacus88d The 88D space
 * @param layer Current layer
 * @param dimension Current dimension
 * @param value The value to analyze
 * @param boundary Output boundary information
 * @return MATH_SUCCESS or error code
 */
MathError geometric_detect_boundary_threadsafe(
    Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
);

/**
 * @brief Thread-safe coordinate transformation
 * 
 * Same as geometric_apply_transform but with proper locking.
 * 
 * @param abacus88d The 88D space
 * @param transform Transformation to apply
 * @param input Input value
 * @param output Output transformed value
 * @return MATH_SUCCESS or error code
 */
MathError geometric_apply_transform_threadsafe(
    Abacus88D* abacus88d,
    const TransformMatrix* transform,
    const CrystallineAbacus* input,
    CrystallineAbacus* output
);

/**
 * @brief Thread-safe handoff execution
 * 
 * Same as geometric_execute_handoff but with proper synchronization
 * to ensure atomic transfer between layers/dimensions.
 * 
 * @param source Source 88D space
 * @param target Target 88D space (can be same as source)
 * @param context Handoff context
 * @param value Value to transfer
 * @return MATH_SUCCESS or error code
 */
MathError geometric_execute_handoff_threadsafe(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* context,
    const CrystallineAbacus* value
);

/**
 * @brief Thread-safe batch boundary detection
 * 
 * Detects boundaries for multiple values concurrently with proper locking.
 * 
 * @param abacus88d The 88D space
 * @param layers Array of layers
 * @param dimensions Array of dimensions
 * @param values Array of values
 * @param boundaries Output array of boundary information
 * @param count Number of items
 * @return MATH_SUCCESS or error code
 */
MathError geometric_detect_boundaries_batch_threadsafe(
    Abacus88D* abacus88d,
    const uint8_t* layers,
    const uint8_t* dimensions,
    const CrystallineAbacus** values,
    BoundaryInfo* boundaries,
    uint32_t count
);

/**
 * @brief Thread-safe batch handoff execution
 * 
 * Executes multiple handoffs atomically with proper synchronization.
 * 
 * @param source Source 88D space
 * @param target Target 88D space
 * @param contexts Array of handoff contexts
 * @param values Array of values
 * @param count Number of handoffs
 * @return MATH_SUCCESS or error code
 */
MathError geometric_execute_handoffs_batch_threadsafe(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* contexts,
    const CrystallineAbacus** values,
    uint32_t count
);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_GEOMETRIC_SPACE_OPS_H */