/**
 * @file geometric_types.h
 * @brief Core geometric data types for 88D threading architecture
 * 
 * This module defines the fundamental geometric types used throughout
 * the geometric threading system. All operations are designed to be O(1).
 * 
 * Key Principles:
 * - Everything is a geometric position
 * - Observer always at center
 * - 12-fold symmetry
 * - Self-similar at all scales
 */

#ifndef GEOMETRIC_TYPES_H
#define GEOMETRIC_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

// Include math library for CrystallineAbacus
#include "../../math/include/math/abacus.h"
#include "../../math/include/math/clock.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define GEO_NUM_CLOCK_POSITIONS 12
#define GEO_NUM_PRIME_POSITIONS 4      // {1, 5, 7, 11}
#define GEO_NUM_LAYERS 8               // 8 octaves for magnitude
#define GEO_DIMS_PER_LAYER 11          // 11 working dimensions (1-11)
#define GEO_NUM_WORKER_THREADS 12      // 12 threads at icosahedron vertices
#define GEO_DEGREES_PER_POSITION 30.0  // 360° / 12 = 30°

#define GEO_GOLDEN_RATIO 1.618033988749895  // φ (phi)

// ============================================================================
// PLATONIC SOLID TYPES
// ============================================================================

/**
 * @brief Platonic solid types for coordinate frames
 */
typedef enum {
    GEO_TETRAHEDRON = 0,    // 4 vertices, self-dual
    GEO_CUBE = 1,           // 8 vertices, dual: octahedron
    GEO_OCTAHEDRON = 2,     // 6 vertices, dual: cube
    GEO_DODECAHEDRON = 3,   // 20 vertices, dual: icosahedron
    GEO_ICOSAHEDRON = 4     // 12 vertices, dual: dodecahedron
} GeoPlatonicSolid;

// ============================================================================
// GEOMETRIC POSITION
// ============================================================================

/**
 * @brief Geometric position in the lattice
 * 
 * Represents any point in the geometric space using clock lattice coordinates.
 * All positions are relative to the observer (who is always at the center).
 */
typedef struct {
    uint8_t ring;           // Ring number (0-3 for clock lattice)
    uint8_t position;       // Position on ring (0-11)
    int64_t magnitude;      // Magnitude (which "lap" around clock)
    double angle;           // Geometric angle (0-360°)
    uint8_t layer;          // Layer in 88D structure (0-7)
    
    // Metadata
    bool is_prime_position; // True if position is {1, 5, 7, 11}
    bool is_control;        // True if position 0/12 (control position)
    
} GeometricPosition;

// ============================================================================
// PLATONIC FRAME
// ============================================================================

/**
 * @brief Coordinate frame based on platonic solid geometry
 * 
 * Each layer in the 88D structure uses a different platonic solid
 * as its coordinate frame. This enables geometric operations.
 */
typedef struct PlatonicFrame {
    GeoPlatonicSolid solid;           // Which platonic solid
    uint8_t layer;                    // Which layer (0-7)
    
    // Geometry
    uint8_t num_vertices;             // Number of vertices
    double vertices[20][3];           // Vertex coordinates (max 20 for dodecahedron)
    double center[3];                 // Center point
    double radius;                    // Circumradius
    
    // Dual solid
    GeoPlatonicSolid dual_solid;
    struct PlatonicFrame* dual_frame; // Pointer to dual frame
    
    // Euler characteristic (V - E + F = 2)
    uint8_t num_edges;
    uint8_t num_faces;
    
} PlatonicFrame;

// ============================================================================
// GEOMETRIC SPACE 88D
// ============================================================================

/**
 * @brief 88-dimensional computational space
 * 
 * Structure: 8 layers × 11 dimensions = 88 dimensions
 * Each position stores a CrystallineAbacus for exact arithmetic.
 * Observer is always at the center.
 */
typedef struct {
    // Core data: 8 layers × 11 dimensions
    CrystallineAbacus* values[GEO_NUM_LAYERS][GEO_DIMS_PER_LAYER];
    
    // Coordinate frames (one per layer)
    PlatonicFrame frames[GEO_NUM_LAYERS];
    
    // Active layer (magnitude-based selection)
    uint8_t active_layer;
    
    // Observer position (always at center)
    GeometricPosition observer;
    
    // Thread-safe locks (per layer)
    pthread_mutex_t layer_locks[GEO_NUM_LAYERS];
    
    // Metadata
    uint32_t base;                    // Abacus base (60 for Babylonian)
    int32_t precision;                // Precision for fractional values
    bool initialized;
    
    // Statistics
    uint64_t operations_count;        // Total operations performed
    uint64_t layer_switches;          // Number of layer switches
    
} GeometricSpace88D;

// ============================================================================
// GEOMETRIC THREAD
// ============================================================================

/**
 * @brief Thread positioned geometrically in space
 * 
 * Each thread has a specific geometric position (not arbitrary ID).
 * Threads are positioned at icosahedron vertices around a central control thread.
 */
typedef struct GeometricThread {
    // Identity
    uint32_t thread_id;               // Thread ID (0 = control, 1-12 = workers)
    bool is_control;                  // True if control thread
    
    // Geometric position
    GeometricPosition position;       // Position in geometric space
    double angle_from_center;         // Angle from control thread (0-360°)
    double distance_from_center;      // Geometric distance from center
    
    // Icosahedron vertex (for worker threads)
    uint8_t vertex_id;                // 0-11 (icosahedron vertex)
    double vertex_coords[3];          // 3D coordinates
    
    // 88D workspace
    GeometricSpace88D* workspace;     // Thread's 88D computational space
    
    // Neighbors (geometric proximity)
    uint8_t num_neighbors;            // Number of neighboring threads
    uint32_t neighbor_ids[GEO_NUM_WORKER_THREADS];  // IDs of neighboring threads
    double neighbor_angles[GEO_NUM_WORKER_THREADS]; // Angles to neighbors
    double neighbor_distances[GEO_NUM_WORKER_THREADS]; // Distances to neighbors
    
    // Shared memory regions (at boundaries with neighbors)
    void* shared_boundaries[GEO_NUM_WORKER_THREADS];
    
    // Thread state
    pthread_t thread_handle;
    atomic_int running;
    atomic_int work_available;
    
    // Work queue (geometric)
    GeometricPosition* work_queue;
    uint32_t work_queue_size;
    uint32_t work_queue_capacity;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    
    // Statistics
    uint64_t operations_performed;
    uint64_t work_items_processed;
    uint64_t boundary_crossings;
    
} GeometricThread;

// ============================================================================
// GEOMETRIC WORK ITEM
// ============================================================================

/**
 * @brief Work item with geometric position
 * 
 * Work is distributed based on geometric proximity to threads.
 */
typedef struct {
    GeometricPosition position;       // Where the work is located
    void* data;                       // Work data
    uint32_t priority;                // Priority (based on magnitude)
    
    // Callback
    void (*execute)(void* data, GeometricSpace88D* workspace);
    
} GeometricWorkItem;

// ============================================================================
// GEOMETRIC THREAD POOL
// ============================================================================

/**
 * @brief Thread pool with geometric configuration
 * 
 * 1 control thread at center + 12 worker threads at icosahedron vertices
 */
typedef struct {
    // Control thread (center)
    GeometricThread* control_thread;
    
    // Worker threads (12 at icosahedron vertices)
    GeometricThread* worker_threads[GEO_NUM_WORKER_THREADS];
    uint8_t num_workers;
    
    // Geometric relationships
    double worker_angles[GEO_NUM_WORKER_THREADS];      // Angles from center
    double worker_distances[GEO_NUM_WORKER_THREADS];   // Distances from center
    
    // Shared coordinate system
    GeometricPosition origin;         // Origin (control thread position)
    
    // Work distribution
    pthread_mutex_t work_mutex;
    pthread_cond_t work_available;
    
    // Synchronization
    pthread_barrier_t sync_barrier;
    pthread_mutex_t pool_mutex;
    
    // State
    atomic_int running;
    atomic_int threads_ready;
    
    // Statistics
    uint64_t total_work_items;
    uint64_t work_items_completed;
    uint64_t work_steals;
    
} GeometricThreadPool;

// ============================================================================
// GEOMETRIC BOUNDARY
// ============================================================================

/**
 * @brief Boundary between two threads
 * 
 * Boundaries are where threads share memory and coordinate calculations.
 */
typedef struct {
    uint32_t thread1_id;
    uint32_t thread2_id;
    
    GeometricPosition midpoint;       // Geometric midpoint
    double boundary_angle;            // Angle of boundary
    
    void* shared_memory;              // Shared memory region
    size_t shared_memory_size;
    
    pthread_mutex_t boundary_mutex;
    
    // Statistics
    uint64_t crossings;               // Number of boundary crossings
    uint64_t data_transfers;          // Amount of data transferred
    
} GeometricBoundary;

// ============================================================================
// GEOMETRIC OPERATION RESULT
// ============================================================================

/**
 * @brief Result of a geometric operation
 */
typedef struct {
    GeometricPosition result_position;
    CrystallineAbacus* result_value;
    bool success;
    const char* error_message;
    
    // Performance metrics
    uint64_t operations_performed;
    double time_elapsed_ns;
    
} GeometricOperationResult;

#ifdef __cplusplus
}
#endif

#endif // GEOMETRIC_TYPES_H