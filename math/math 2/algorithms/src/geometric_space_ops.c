/**
 * @file geometric_space_ops.c
 * @brief Implementation of geometric space operations
 */

#include "geometric_space_ops.h"
#include "math/abacus.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * BOUNDARY DETECTION IMPLEMENTATION
 * ============================================================================
 */

/**
 * @brief Threshold for boundary detection (10% of scale)
 */
#define BOUNDARY_THRESHOLD 0.1

/**
 * @brief Get magnitude of a value as double (for comparison)
 */
static double get_magnitude_double(const CrystallineAbacus* value) {
    double result = 0.0;
    if (abacus_to_double(value, &result) != MATH_SUCCESS) {
        return 0.0;
    }
    return math_abs(result);
}

MathError geometric_detect_boundary(
    const Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
) {
    if (!abacus88d || !value || !boundary) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (layer >= ABACUS88D_NUM_LAYERS || dimension >= ABACUS88D_DIMS_PER_LAYER) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Initialize boundary info
    memset(boundary, 0, sizeof(BoundaryInfo));
    boundary->current_layer = layer;
    boundary->current_dimension = dimension;
    
    // Get value magnitude
    double magnitude = get_magnitude_double(value);
    
    // Check layer boundaries
    uint64_t current_scale = abacus88d->layers[layer].magnitude_scale;
    uint64_t lower_bound = current_scale;
    uint64_t upper_bound = (layer < ABACUS88D_NUM_LAYERS - 1) ? 
                           abacus88d->layers[layer + 1].magnitude_scale : 
                           UINT64_MAX;
    
    // Check if near lower layer boundary
    if (layer > 0 && magnitude < lower_bound * (1.0 + BOUNDARY_THRESHOLD)) {
        boundary->near_layer_boundary = true;
        boundary->target_layer = layer - 1;
        boundary->layer_distance = (magnitude - lower_bound) / (lower_bound * BOUNDARY_THRESHOLD);
        boundary->requires_handoff = true;
        boundary->handoff_urgency = 1.0 - boundary->layer_distance;
    }
    
    // Check if near upper layer boundary
    if (layer < ABACUS88D_NUM_LAYERS - 1 && magnitude > upper_bound * (1.0 - BOUNDARY_THRESHOLD)) {
        boundary->near_layer_boundary = true;
        boundary->target_layer = layer + 1;
        boundary->layer_distance = (upper_bound - magnitude) / (upper_bound * BOUNDARY_THRESHOLD);
        boundary->requires_handoff = true;
        boundary->handoff_urgency = 1.0 - boundary->layer_distance;
    }
    
    // Check dimension boundaries (clock positions)
    // Dimensions are adjacent on the clock: 0-1-2-...-10-0 (wraps)
    boundary->near_dimension_boundary = false;
    
    // For now, dimension boundaries are less critical than layer boundaries
    // They would be detected based on angular position on the clock
    
    // Check if at Platonic solid vertex
    // Vertices are shared between multiple dimensions
    boundary->at_vertex = false;
    
    // Get shared vertices for this dimension
    MathError err = geometric_find_shared_vertices(
        abacus88d, layer, dimension,
        boundary->shared_dimensions,
        &boundary->num_shared_dimensions
    );
    
    if (err == MATH_SUCCESS && boundary->num_shared_dimensions > 0) {
        boundary->at_vertex = true;
        boundary->vertex_id = dimension; // Simplified - actual vertex ID would be more complex
    }
    
    return MATH_SUCCESS;
}

bool geometric_should_transition_layer(
    const CrystallineAbacus* value,
    uint8_t current_layer,
    uint8_t* target_layer
) {
    if (!value || !target_layer) {
        return false;
    }
    
    // Get value magnitude
    double magnitude = get_magnitude_double(value);
    
    // Determine appropriate layer based on magnitude
    // Layer 0: < 1000
    // Layer 1: 1000 - 1M
    // Layer 2: 1M - 1B
    // Layer 3: 1B - 1T
    // Layer 4: 1T - 1Q
    // etc.
    
    uint8_t suggested_layer = 0;
    if (magnitude >= 1e18) suggested_layer = 7;
    else if (magnitude >= 1e15) suggested_layer = 6;
    else if (magnitude >= 1e12) suggested_layer = 5;
    else if (magnitude >= 1e9) suggested_layer = 4;
    else if (magnitude >= 1e6) suggested_layer = 3;
    else if (magnitude >= 1e3) suggested_layer = 1;  // Layer 1 starts at 1000
    else suggested_layer = 0;  // Layer 0 is < 1000
    
    *target_layer = suggested_layer;
    return (suggested_layer != current_layer);
}

MathError geometric_find_shared_vertices(
    const Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    uint8_t* shared_dims,
    uint8_t* num_shared
) {
    if (!abacus88d || !shared_dims || !num_shared) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (layer >= ABACUS88D_NUM_LAYERS || dimension >= ABACUS88D_DIMS_PER_LAYER) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Get the Platonic solid for this layer
    const PlatonicSolid* solid = abacus88d->layers[layer].frame;
    if (!solid) {
        *num_shared = 0;
        return MATH_SUCCESS;
    }
    
    // Find vertices shared with this dimension
    // This is a simplified implementation - actual implementation would
    // analyze the Platonic solid's edge and face structure
    
    *num_shared = 0;
    
    // Adjacent dimensions on the clock share vertices
    // Dimension N shares vertices with (N-1) mod 11 and (N+1) mod 11
    shared_dims[(*num_shared)++] = (dimension + ABACUS88D_DIMS_PER_LAYER - 1) % ABACUS88D_DIMS_PER_LAYER;
    shared_dims[(*num_shared)++] = (dimension + 1) % ABACUS88D_DIMS_PER_LAYER;
    
    return MATH_SUCCESS;
}

/* ============================================================================
 * COORDINATE TRANSFORMATION IMPLEMENTATION
 * ============================================================================
 */

MathError geometric_calculate_transform(
    uint8_t source_layer,
    uint8_t target_layer,
    TransformMatrix* transform
) {
    if (!transform) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (source_layer >= ABACUS88D_NUM_LAYERS || target_layer >= ABACUS88D_NUM_LAYERS) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Initialize transform
    memset(transform, 0, sizeof(TransformMatrix));
    transform->source_layer = source_layer;
    transform->target_layer = target_layer;
    
    // Calculate scale factor between layers
    uint64_t source_scale = LAYER_SCALES[source_layer];
    uint64_t target_scale = LAYER_SCALES[target_layer];
    transform->scale_factor = (double)target_scale / (double)source_scale;
    
    // Initialize as identity matrix
    for (int i = 0; i < 4; i++) {
        transform->matrix[i][i] = 1.0;
    }
    
    // Apply scale factor to transformation matrix
    for (int i = 0; i < 3; i++) {
        transform->matrix[i][i] = transform->scale_factor;
    }
    
    // Additional transformations would be added here based on
    // the specific Platonic solid geometries involved
    
    return MATH_SUCCESS;
}

MathError geometric_apply_transform(
    const CrystallineAbacus* value,
    const TransformMatrix* transform,
    CrystallineAbacus* result
) {
    if (!value || !transform || !result) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // For now, just apply the scale factor
    // Full implementation would apply the complete transformation matrix
    
    // Create scale factor as abacus
    CrystallineAbacus* scale = abacus_from_double(transform->scale_factor, value->base, 6);
    if (!scale) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    // Multiply value by scale factor
    MathError err = abacus_mul(result, value, scale);
    abacus_free(scale);
    
    return err;
}

MathError geometric_transform_dimension(
    const CrystallineAbacus* value,
    uint8_t source_dim,
    uint8_t target_dim,
    CrystallineAbacus* result
) {
    if (!value || !result) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    if (source_dim >= ABACUS88D_DIMS_PER_LAYER || target_dim >= ABACUS88D_DIMS_PER_LAYER) {
        return MATH_ERROR_INVALID_ARG;
    }
    
    // Calculate angular difference on clock
    // TODO: Apply angular transformation based on dimension difference
    (void)source_dim; (void)target_dim; // Suppress unused warning until transformation is implemented
    
    // For now, just copy the value
    // Full implementation would apply angular transformation
    CrystallineAbacus* temp = abacus_copy(value);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    MathError err = abacus_add(result, temp, abacus_new(value->base));
    abacus_free(temp);
    
    return err;
}

/* ============================================================================
 * HANDOFF OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

MathError geometric_prepare_handoff(
    const BoundaryInfo* boundary,
    HandoffContext* context
) {
    if (!boundary || !context) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Initialize context
    memset(context, 0, sizeof(HandoffContext));
    
    context->source_layer = boundary->current_layer;
    context->source_dimension = boundary->current_dimension;
    
    if (boundary->near_layer_boundary) {
        context->target_layer = boundary->target_layer;
        context->target_dimension = boundary->current_dimension; // Same dimension, different layer
        context->requires_scaling = true;
        
        // Calculate transformation
        return geometric_calculate_transform(
            context->source_layer,
            context->target_layer,
            &context->transform
        );
    }
    
    if (boundary->near_dimension_boundary) {
        context->target_layer = boundary->current_layer; // Same layer, different dimension
        context->target_dimension = boundary->target_dimension;
        context->requires_scaling = false;
    }
    
    if (boundary->at_vertex) {
        context->at_shared_vertex = true;
        context->vertex_id = boundary->vertex_id;
    }
    
    return MATH_SUCCESS;
}

MathError geometric_execute_handoff(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* context,
    const CrystallineAbacus* value
) {
    if (!source || !target || !context || !value) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Transform the value
    CrystallineAbacus* transformed = abacus_new(value->base);
    if (!transformed) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    MathError err;
    
    if (context->requires_scaling) {
        err = geometric_apply_transform(value, &context->transform, transformed);
    } else {
        err = geometric_transform_dimension(
            value,
            context->source_dimension,
            context->target_dimension,
            transformed
        );
    }
    
    if (err != MATH_SUCCESS) {
        abacus_free(transformed);
        return err;
    }
    
    // Set the value in the target location
    err = abacus88d_set(
        target,
        context->target_layer,
        context->target_dimension,
        transformed
    );
    
    abacus_free(transformed);
    return err;
}

MathError geometric_calculate_handoff_value(
    const CrystallineAbacus* source_value,
    const BoundaryInfo* boundary,
    CrystallineAbacus* handoff_value
) {
    if (!source_value || !boundary || !handoff_value) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // For now, just copy the value
    // Full implementation would interpolate based on boundary distance
    CrystallineAbacus* temp = abacus_copy(source_value);
    if (!temp) {
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    MathError err = abacus_add(handoff_value, temp, abacus_new(source_value->base));
    abacus_free(temp);
    
    return err;
}

/* ============================================================================
 * WORK DISTRIBUTION IMPLEMENTATION
 * ============================================================================
 */

MathError geometric_distribute_work(
    const Abacus88D* abacus88d,
    const WorkItem* items,
    uint32_t num_items,
    uint32_t num_workers,
    WorkDistribution* distribution
) {
    if (!abacus88d || !items || !distribution || num_workers == 0) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Allocate distribution structure
    distribution->num_workers = num_workers;
    distribution->worker_loads = calloc(num_workers, sizeof(uint32_t));
    distribution->worker_items = calloc(num_workers, sizeof(WorkItem*));
    distribution->items_per_worker = calloc(num_workers, sizeof(uint32_t));
    
    if (!distribution->worker_loads || !distribution->worker_items || 
        !distribution->items_per_worker) {
        geometric_free_distribution(distribution);
        return MATH_ERROR_OUT_OF_MEMORY;
    }
    
    // Allocate space for items per worker
    for (uint32_t i = 0; i < num_workers; i++) {
        distribution->worker_items[i] = calloc(num_items, sizeof(WorkItem));
        if (!distribution->worker_items[i]) {
            geometric_free_distribution(distribution);
            return MATH_ERROR_OUT_OF_MEMORY;
        }
    }
    
    // Simple round-robin distribution for now
    // Full implementation would consider:
    // - Work item priorities
    // - Geometric locality (items in same layer/dimension)
    // - Load balancing
    // - Boundary minimization
    
    for (uint32_t i = 0; i < num_items; i++) {
        uint32_t worker = i % num_workers;
        uint32_t idx = distribution->items_per_worker[worker];
        
        distribution->worker_items[worker][idx] = items[i];
        distribution->worker_loads[worker] += items[i].work_size;
        distribution->items_per_worker[worker]++;
    }
    
    return MATH_SUCCESS;
}

void geometric_free_distribution(WorkDistribution* distribution) {
    if (!distribution) return;
    
    if (distribution->worker_items) {
        for (uint32_t i = 0; i < distribution->num_workers; i++) {
            free(distribution->worker_items[i]);
        }
        free(distribution->worker_items);
    }
    
    free(distribution->worker_loads);
    free(distribution->items_per_worker);
    
    memset(distribution, 0, sizeof(WorkDistribution));
}

/* ============================================================================
 * THREAD-SAFE OPERATIONS
 * ============================================================================
 */

MathError geometric_detect_boundary_threadsafe(
    Abacus88D* abacus88d,
    uint8_t layer,
    uint8_t dimension,
    const CrystallineAbacus* value,
    BoundaryInfo* boundary
) {
    if (!abacus88d || !value || !boundary) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Lock the layer for thread-safe access
    pthread_mutex_lock(&abacus88d->layers[layer].layer_lock);
    
    // Perform boundary detection
    MathError err = geometric_detect_boundary(abacus88d, layer, dimension, value, boundary);
    
    // Unlock
    pthread_mutex_unlock(&abacus88d->layers[layer].layer_lock);
    
    return err;
}

MathError geometric_apply_transform_threadsafe(
    Abacus88D* abacus88d,
    const TransformMatrix* transform,
    const CrystallineAbacus* input,
    CrystallineAbacus* output
) {
    if (!abacus88d || !transform || !input || !output) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Lock both source and target layers
    uint8_t source_layer = transform->source_layer;
    uint8_t target_layer = transform->target_layer;
    
    // Lock in consistent order to avoid deadlock
    if (source_layer < target_layer) {
        pthread_mutex_lock(&abacus88d->layers[source_layer].layer_lock);
        pthread_mutex_lock(&abacus88d->layers[target_layer].layer_lock);
    } else if (source_layer > target_layer) {
        pthread_mutex_lock(&abacus88d->layers[target_layer].layer_lock);
        pthread_mutex_lock(&abacus88d->layers[source_layer].layer_lock);
    } else {
        // Same layer, only lock once
        pthread_mutex_lock(&abacus88d->layers[source_layer].layer_lock);
    }
    
    // Perform transformation
    MathError err = geometric_apply_transform(input, transform, output);
    
    // Unlock in reverse order
    if (source_layer < target_layer) {
        pthread_mutex_unlock(&abacus88d->layers[target_layer].layer_lock);
        pthread_mutex_unlock(&abacus88d->layers[source_layer].layer_lock);
    } else if (source_layer > target_layer) {
        pthread_mutex_unlock(&abacus88d->layers[source_layer].layer_lock);
        pthread_mutex_unlock(&abacus88d->layers[target_layer].layer_lock);
    } else {
        pthread_mutex_unlock(&abacus88d->layers[source_layer].layer_lock);
    }
    
    return err;
}

MathError geometric_execute_handoff_threadsafe(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* context,
    const CrystallineAbacus* value
) {
    if (!source || !target || !context || !value) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Use global lock for cross-space handoffs
    if (source != target) {
        // Lock both spaces in consistent order (by pointer address)
        if (source < target) {
            pthread_mutex_lock(&source->global_lock);
            pthread_mutex_lock(&target->global_lock);
        } else {
            pthread_mutex_lock(&target->global_lock);
            pthread_mutex_lock(&source->global_lock);
        }
    } else {
        // Same space, use global lock
        pthread_mutex_lock(&source->global_lock);
    }
    
    // Perform handoff
    MathError err = geometric_execute_handoff(source, target, context, value);
    
    // Unlock
    if (source != target) {
        if (source < target) {
            pthread_mutex_unlock(&target->global_lock);
            pthread_mutex_unlock(&source->global_lock);
        } else {
            pthread_mutex_unlock(&source->global_lock);
            pthread_mutex_unlock(&target->global_lock);
        }
    } else {
        pthread_mutex_unlock(&source->global_lock);
    }
    
    return err;
}

MathError geometric_detect_boundaries_batch_threadsafe(
    Abacus88D* abacus88d,
    const uint8_t* layers,
    const uint8_t* dimensions,
    const CrystallineAbacus** values,
    BoundaryInfo* boundaries,
    uint32_t count
) {
    if (!abacus88d || !layers || !dimensions || !values || !boundaries) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Use global lock for batch operations to ensure consistency
    pthread_mutex_lock(&abacus88d->global_lock);
    
    MathError err = MATH_SUCCESS;
    for (uint32_t i = 0; i < count; i++) {
        err = geometric_detect_boundary(abacus88d, layers[i], dimensions[i], 
                                       values[i], &boundaries[i]);
        if (err != MATH_SUCCESS) {
            break;
        }
    }
    
    pthread_mutex_unlock(&abacus88d->global_lock);
    
    return err;
}

MathError geometric_execute_handoffs_batch_threadsafe(
    Abacus88D* source,
    Abacus88D* target,
    const HandoffContext* contexts,
    const CrystallineAbacus** values,
    uint32_t count
) {
    if (!source || !target || !contexts || !values) {
        return MATH_ERROR_NULL_POINTER;
    }
    
    // Use global locks for batch handoffs
    if (source != target) {
        if (source < target) {
            pthread_mutex_lock(&source->global_lock);
            pthread_mutex_lock(&target->global_lock);
        } else {
            pthread_mutex_lock(&target->global_lock);
            pthread_mutex_lock(&source->global_lock);
        }
    } else {
        pthread_mutex_lock(&source->global_lock);
    }
    
    MathError err = MATH_SUCCESS;
    for (uint32_t i = 0; i < count; i++) {
        err = geometric_execute_handoff(source, target, &contexts[i], values[i]);
        if (err != MATH_SUCCESS) {
            break;
        }
    }
    
    // Unlock
    if (source != target) {
        if (source < target) {
            pthread_mutex_unlock(&target->global_lock);
            pthread_mutex_unlock(&source->global_lock);
        } else {
            pthread_mutex_unlock(&source->global_lock);
            pthread_mutex_unlock(&target->global_lock);
        }
    } else {
        pthread_mutex_unlock(&source->global_lock);
    }
    
    return err;
}