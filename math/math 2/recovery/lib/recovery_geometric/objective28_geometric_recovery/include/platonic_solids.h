#ifndef PLATONIC_SOLIDS_H
#define PLATONIC_SOLIDS_H

#include <stdint.h>
#include <stdbool.h>
#include "clock_recovery.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Platonic solid types
 */
typedef enum {
    SOLID_TETRAHEDRON = 0,  // 4 vertices, 6 edges, 4 faces
    SOLID_CUBE = 1,          // 8 vertices, 12 edges, 6 faces
    SOLID_OCTAHEDRON = 2,    // 6 vertices, 12 edges, 8 faces
    SOLID_DODECAHEDRON = 3,  // 20 vertices, 30 edges, 12 faces
    SOLID_ICOSAHEDRON = 4,   // 12 vertices, 30 edges, 20 faces
    NUM_PLATONIC_SOLIDS = 5
} PlatonicSolidType;

/**
 * Platonic solid structure
 */
typedef struct {
    PlatonicSolidType type;
    const char* name;
    uint32_t num_vertices;
    uint32_t num_edges;
    uint32_t num_faces;
    double* vertices;           // [num_vertices * 13] - 13D coordinates
    uint32_t* edges;            // [num_edges * 2] - vertex pairs
    ClockPosition* clock_positions;  // Clock position for each vertex
} PlatonicSolid;

/**
 * Shared vertex structure
 * Represents a point where multiple Platonic solids intersect
 */
typedef struct {
    double position[13];        // 13D coordinates
    uint32_t solid_ids[5];      // Which solids share this vertex
    uint32_t vertex_ids[5];     // Vertex ID in each solid
    uint32_t num_solids;        // How many solids share this vertex
    ClockPosition clock;        // Clock position
    double stability;           // Stability score (more solids = more stable)
} SharedVertex;

/**
 * Platonic solid overlay structure
 * Contains all 5 solids and their shared vertices
 */
typedef struct {
    PlatonicSolid solids[NUM_PLATONIC_SOLIDS];
    SharedVertex* shared_vertices;
    uint32_t num_shared_vertices;
    double tolerance;           // Distance tolerance for vertex matching
} PlatonicOverlay;

// ============================================================================
// PLATONIC SOLID GENERATION
// ============================================================================

/**
 * Create a Platonic solid in 13D space
 * 
 * @param type Solid type
 * @return Allocated Platonic solid (must be freed with free_platonic_solid)
 */
PlatonicSolid* create_platonic_solid(PlatonicSolidType type);

/**
 * Free a Platonic solid
 */
void free_platonic_solid(PlatonicSolid* solid);

/**
 * Generate vertices for a Platonic solid in 13D space
 * Uses clock ring mapping and golden ratio relationships
 * 
 * @param solid Platonic solid to populate
 */
void generate_platonic_vertices_13d(PlatonicSolid* solid);

/**
 * Map Platonic solid vertices to clock positions
 * 
 * @param solid Platonic solid
 */
void map_solid_to_clock(PlatonicSolid* solid);

// ============================================================================
// PLATONIC SOLID OVERLAY
// ============================================================================

/**
 * Create overlay of all 5 Platonic solids
 * 
 * @param tolerance Distance tolerance for vertex matching (default: 0.01)
 * @return Allocated overlay (must be freed with free_platonic_overlay)
 */
PlatonicOverlay* create_platonic_overlay(double tolerance);

/**
 * Free Platonic overlay
 */
void free_platonic_overlay(PlatonicOverlay* overlay);

/**
 * Find shared vertices among all Platonic solids
 * 
 * @param overlay Platonic overlay
 */
void find_shared_vertices(PlatonicOverlay* overlay);

/**
 * Compute stability score for a shared vertex
 * More solids sharing = higher stability
 * 
 * @param vertex Shared vertex
 * @return Stability score [0, 1]
 */
double compute_vertex_stability(const SharedVertex* vertex);

// ============================================================================
// OPTIMAL ANCHOR SELECTION
// ============================================================================

/**
 * Select optimal anchors from shared vertices
 * Prioritizes vertices shared by multiple solids
 * 
 * @param overlay Platonic overlay
 * @param num_anchors Number of anchors to select
 * @return Array of selected shared vertices (must be freed)
 */
SharedVertex** select_optimal_anchors(PlatonicOverlay* overlay, uint32_t num_anchors);

/**
 * Find nearest shared vertices to a target clock position
 * 
 * @param overlay Platonic overlay
 * @param target Target clock position
 * @param k Number of nearest vertices to find
 * @return Array of k nearest shared vertices (must be freed)
 */
SharedVertex** find_nearest_shared_vertices(PlatonicOverlay* overlay, 
                                            ClockPosition target, 
                                            uint32_t k);

// ============================================================================
// GEOMETRIC PROPERTIES
// ============================================================================

/**
 * Verify Euler's formula for a Platonic solid
 * V - E + F = 2
 * 
 * @param solid Platonic solid
 * @return true if Euler's formula holds
 */
bool verify_euler_formula(const PlatonicSolid* solid);

/**
 * Get dual solid type
 * Tetrahedron ↔ Tetrahedron
 * Cube ↔ Octahedron
 * Dodecahedron ↔ Icosahedron
 * 
 * @param type Solid type
 * @return Dual solid type
 */
PlatonicSolidType get_dual_solid(PlatonicSolidType type);

/**
 * Compute distance between two vertices in 13D space
 * 
 * @param v1 First vertex (13D coordinates)
 * @param v2 Second vertex (13D coordinates)
 * @return Euclidean distance
 */
double compute_13d_distance(const double* v1, const double* v2);

/**
 * Check if two vertices are the same (within tolerance)
 * 
 * @param v1 First vertex
 * @param v2 Second vertex
 * @param tolerance Distance tolerance
 * @return true if vertices match
 */
bool vertices_match(const double* v1, const double* v2, double tolerance);

// ============================================================================
// STATISTICS & ANALYSIS
// ============================================================================

/**
 * Get statistics for Platonic overlay
 */
typedef struct {
    uint32_t total_vertices;        // Total vertices across all solids
    uint32_t unique_vertices;       // Unique vertex positions
    uint32_t shared_vertices;       // Vertices shared by 2+ solids
    uint32_t highly_stable;         // Vertices shared by 3+ solids
    double avg_stability;           // Average stability score
    uint32_t ring_distribution[4];  // Vertices per ring
} OverlayStatistics;

/**
 * Compute statistics for Platonic overlay
 * 
 * @param overlay Platonic overlay
 * @return Statistics structure
 */
OverlayStatistics compute_overlay_statistics(const PlatonicOverlay* overlay);

/**
 * Print Platonic overlay statistics
 * 
 * @param stats Statistics structure
 */
void print_overlay_statistics(const OverlayStatistics* stats);

#ifdef __cplusplus
}
#endif

#endif // PLATONIC_SOLIDS_H
