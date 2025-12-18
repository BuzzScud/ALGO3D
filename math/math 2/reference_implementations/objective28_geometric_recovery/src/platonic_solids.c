/**
 * platonic_solids.c - Platonic Solid Generation and Overlay
 * 
 * Implements all 5 Platonic solids in 13D space using:
 * - Clock ring mapping
 * - Golden ratio relationships
 * - 12-fold symmetry
 * - Shared vertex detection
 */

#include "platonic_solids.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// Golden ratio
#define PHI 1.618033988749895

// Platonic solid properties
static const struct {
    const char* name;
    uint32_t vertices;
    uint32_t edges;
    uint32_t faces;
} SOLID_PROPERTIES[NUM_PLATONIC_SOLIDS] = {
    {"Tetrahedron", 4, 6, 4},
    {"Cube", 8, 12, 6},
    {"Octahedron", 6, 12, 8},
    {"Dodecahedron", 20, 30, 12},
    {"Icosahedron", 12, 30, 20}
};

// ============================================================================
// PLATONIC SOLID GENERATION
// ============================================================================

/**
 * Create a Platonic solid
 */
PlatonicSolid* create_platonic_solid(PlatonicSolidType type) {
    if (type >= NUM_PLATONIC_SOLIDS) return NULL;
    
    PlatonicSolid* solid = (PlatonicSolid*)malloc(sizeof(PlatonicSolid));
    if (!solid) return NULL;
    
    solid->type = type;
    solid->name = SOLID_PROPERTIES[type].name;
    solid->num_vertices = SOLID_PROPERTIES[type].vertices;
    solid->num_edges = SOLID_PROPERTIES[type].edges;
    solid->num_faces = SOLID_PROPERTIES[type].faces;
    
    // Allocate vertex array (13D coordinates)
    solid->vertices = (double*)calloc(solid->num_vertices * 13, sizeof(double));
    if (!solid->vertices) {
        free(solid);
        return NULL;
    }
    
    // Allocate edge array
    solid->edges = (uint32_t*)calloc(solid->num_edges * 2, sizeof(uint32_t));
    if (!solid->edges) {
        free(solid->vertices);
        free(solid);
        return NULL;
    }
    
    // Allocate clock positions
    solid->clock_positions = (ClockPosition*)calloc(solid->num_vertices, sizeof(ClockPosition));
    if (!solid->clock_positions) {
        free(solid->edges);
        free(solid->vertices);
        free(solid);
        return NULL;
    }
    
    // Generate vertices and map to clock
    generate_platonic_vertices_13d(solid);
    map_solid_to_clock(solid);
    
    return solid;
}

/**
 * Free a Platonic solid
 */
void free_platonic_solid(PlatonicSolid* solid) {
    if (solid) {
        if (solid->vertices) free(solid->vertices);
        if (solid->edges) free(solid->edges);
        if (solid->clock_positions) free(solid->clock_positions);
        free(solid);
    }
}

/**
 * Generate vertices for Tetrahedron in 13D
 */
static void generate_tetrahedron_vertices(PlatonicSolid* solid) {
    // Tetrahedron: 4 vertices arranged in tetrahedral symmetry
    // Maps to Ring 0 (12 positions, 4×3)
    
    double* v = solid->vertices;
    
    // Use golden ratio and 12-fold symmetry
    double angle_step = 2.0 * M_PI / 12.0;  // 12-fold symmetry
    
    for (uint32_t i = 0; i < 4; i++) {
        double angle = i * 3 * angle_step;  // Every 3rd position (4×3=12)
        double radius = 0.25;  // Ring 0 radius
        
        // First 3 dimensions: circular arrangement
        v[i * 13 + 0] = radius * cos(angle);
        v[i * 13 + 1] = radius * sin(angle);
        v[i * 13 + 2] = radius * cos(angle * PHI);
        
        // Remaining dimensions: golden ratio spiral
        for (uint32_t d = 3; d < 13; d++) {
            v[i * 13 + d] = radius * cos(angle * pow(PHI, d - 2));
        }
    }
}

/**
 * Generate vertices for Cube in 13D
 */
static void generate_cube_vertices(PlatonicSolid* solid) {
    // Cube: 8 vertices
    // Maps to Ring 1 (60 positions)
    
    double* v = solid->vertices;
    double angle_step = 2.0 * M_PI / 60.0;
    
    for (uint32_t i = 0; i < 8; i++) {
        double angle = i * 7.5 * angle_step;  // Spread across ring
        double radius = 0.50;  // Ring 1 radius
        
        v[i * 13 + 0] = radius * cos(angle);
        v[i * 13 + 1] = radius * sin(angle);
        v[i * 13 + 2] = radius * cos(angle * PHI);
        
        for (uint32_t d = 3; d < 13; d++) {
            v[i * 13 + d] = radius * sin(angle * pow(PHI, d - 2));
        }
    }
}

/**
 * Generate vertices for Octahedron in 13D
 */
static void generate_octahedron_vertices(PlatonicSolid* solid) {
    // Octahedron: 6 vertices (dual of cube)
    // Maps to Ring 1 (60 positions, 6×10)
    
    double* v = solid->vertices;
    double angle_step = 2.0 * M_PI / 60.0;
    
    for (uint32_t i = 0; i < 6; i++) {
        double angle = i * 10 * angle_step;  // Every 10th position
        double radius = 0.50;  // Ring 1 radius
        
        v[i * 13 + 0] = radius * cos(angle);
        v[i * 13 + 1] = radius * sin(angle);
        v[i * 13 + 2] = radius * cos(angle / PHI);
        
        for (uint32_t d = 3; d < 13; d++) {
            v[i * 13 + d] = radius * cos(angle * pow(PHI, d - 3));
        }
    }
}

/**
 * Generate vertices for Dodecahedron in 13D
 */
static void generate_dodecahedron_vertices(PlatonicSolid* solid) {
    // Dodecahedron: 20 vertices
    // Maps to Ring 2 (60 positions, 20×3)
    
    double* v = solid->vertices;
    double angle_step = 2.0 * M_PI / 60.0;
    
    for (uint32_t i = 0; i < 20; i++) {
        double angle = i * 3 * angle_step;  // Every 3rd position
        double radius = 0.75;  // Ring 2 radius
        
        v[i * 13 + 0] = radius * cos(angle);
        v[i * 13 + 1] = radius * sin(angle);
        v[i * 13 + 2] = radius * cos(angle * PHI);
        
        for (uint32_t d = 3; d < 13; d++) {
            v[i * 13 + d] = radius * sin(angle * pow(PHI, d - 2));
        }
    }
}

/**
 * Generate vertices for Icosahedron in 13D
 */
static void generate_icosahedron_vertices(PlatonicSolid* solid) {
    // Icosahedron: 12 vertices (dual of dodecahedron)
    // Maps to Ring 0 (12 positions, 12×1)
    
    double* v = solid->vertices;
    double angle_step = 2.0 * M_PI / 12.0;
    
    for (uint32_t i = 0; i < 12; i++) {
        double angle = i * angle_step;  // Each position
        double radius = 0.25;  // Ring 0 radius
        
        v[i * 13 + 0] = radius * cos(angle);
        v[i * 13 + 1] = radius * sin(angle);
        v[i * 13 + 2] = radius * sin(angle * PHI);
        
        for (uint32_t d = 3; d < 13; d++) {
            v[i * 13 + d] = radius * cos(angle * pow(PHI, d - 1));
        }
    }
}

/**
 * Generate vertices for a Platonic solid in 13D space
 */
void generate_platonic_vertices_13d(PlatonicSolid* solid) {
    switch (solid->type) {
        case SOLID_TETRAHEDRON:
            generate_tetrahedron_vertices(solid);
            break;
        case SOLID_CUBE:
            generate_cube_vertices(solid);
            break;
        case SOLID_OCTAHEDRON:
            generate_octahedron_vertices(solid);
            break;
        case SOLID_DODECAHEDRON:
            generate_dodecahedron_vertices(solid);
            break;
        case SOLID_ICOSAHEDRON:
            generate_icosahedron_vertices(solid);
            break;
        default:
            break;
    }
}

/**
 * Map Platonic solid vertices to clock positions
 */
void map_solid_to_clock(PlatonicSolid* solid) {
    for (uint32_t i = 0; i < solid->num_vertices; i++) {
        double* vertex = &solid->vertices[i * 13];
        
        // Compute angle from first 2 dimensions
        double angle = atan2(vertex[1], vertex[0]);
        if (angle < 0) angle += 2.0 * M_PI;
        
        // Compute radius (distance from origin in first 3 dimensions)
        double radius = sqrt(vertex[0] * vertex[0] + 
                           vertex[1] * vertex[1] + 
                           vertex[2] * vertex[2]);
        
        // Map to clock position
        solid->clock_positions[i] = map_angle_to_ring(angle);
        solid->clock_positions[i].radius = radius;
    }
}

// ============================================================================
// PLATONIC SOLID OVERLAY
// ============================================================================

/**
 * Create overlay of all 5 Platonic solids
 */
PlatonicOverlay* create_platonic_overlay(double tolerance) {
    PlatonicOverlay* overlay = (PlatonicOverlay*)malloc(sizeof(PlatonicOverlay));
    if (!overlay) return NULL;
    
    overlay->tolerance = tolerance;
    overlay->shared_vertices = NULL;
    overlay->num_shared_vertices = 0;
    
    // Create all 5 Platonic solids
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        PlatonicSolid* solid = create_platonic_solid((PlatonicSolidType)i);
        if (!solid) {
            // Cleanup on failure
            for (int j = 0; j < i; j++) {
                free_platonic_solid(&overlay->solids[j]);
            }
            free(overlay);
            return NULL;
        }
        overlay->solids[i] = *solid;
        free(solid);  // Free the wrapper, keep the data
    }
    
    // Find shared vertices
    find_shared_vertices(overlay);
    
    return overlay;
}

/**
 * Free Platonic overlay
 */
void free_platonic_overlay(PlatonicOverlay* overlay) {
    if (overlay) {
        for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
            if (overlay->solids[i].vertices) free(overlay->solids[i].vertices);
            if (overlay->solids[i].edges) free(overlay->solids[i].edges);
            if (overlay->solids[i].clock_positions) free(overlay->solids[i].clock_positions);
        }
        if (overlay->shared_vertices) free(overlay->shared_vertices);
        free(overlay);
    }
}

/**
 * Find shared vertices among all Platonic solids
 */
void find_shared_vertices(PlatonicOverlay* overlay) {
    // Count total vertices
    uint32_t total_vertices = 0;
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        total_vertices += overlay->solids[i].num_vertices;
    }
    
    // Allocate maximum possible shared vertices
    SharedVertex* candidates = (SharedVertex*)calloc(total_vertices, sizeof(SharedVertex));
    if (!candidates) return;
    
    uint32_t num_shared = 0;
    
    // For each vertex in each solid
    for (int s1 = 0; s1 < NUM_PLATONIC_SOLIDS; s1++) {
        PlatonicSolid* solid1 = &overlay->solids[s1];
        
        for (uint32_t v1 = 0; v1 < solid1->num_vertices; v1++) {
            double* pos1 = &solid1->vertices[v1 * 13];
            
            // Check if this vertex already in shared list
            bool already_shared = false;
            for (uint32_t i = 0; i < num_shared; i++) {
                if (vertices_match(pos1, candidates[i].position, overlay->tolerance)) {
                    // Add this solid to the existing shared vertex
                    if (candidates[i].num_solids < 5) {
                        candidates[i].solid_ids[candidates[i].num_solids] = s1;
                        candidates[i].vertex_ids[candidates[i].num_solids] = v1;
                        candidates[i].num_solids++;
                    }
                    already_shared = true;
                    break;
                }
            }
            
            if (already_shared) continue;
            
            // Check against other solids
            SharedVertex new_shared;
            memcpy(new_shared.position, pos1, 13 * sizeof(double));
            new_shared.solid_ids[0] = s1;
            new_shared.vertex_ids[0] = v1;
            new_shared.num_solids = 1;
            new_shared.clock = solid1->clock_positions[v1];
            
            for (int s2 = s1 + 1; s2 < NUM_PLATONIC_SOLIDS; s2++) {
                PlatonicSolid* solid2 = &overlay->solids[s2];
                
                for (uint32_t v2 = 0; v2 < solid2->num_vertices; v2++) {
                    double* pos2 = &solid2->vertices[v2 * 13];
                    
                    if (vertices_match(pos1, pos2, overlay->tolerance)) {
                        if (new_shared.num_solids < 5) {
                            new_shared.solid_ids[new_shared.num_solids] = s2;
                            new_shared.vertex_ids[new_shared.num_solids] = v2;
                            new_shared.num_solids++;
                        }
                    }
                }
            }
            
            // If shared by 2+ solids, add to list
            if (new_shared.num_solids >= 2) {
                new_shared.stability = compute_vertex_stability(&new_shared);
                candidates[num_shared++] = new_shared;
            }
        }
    }
    
    // Allocate final shared vertices array
    if (num_shared > 0) {
        overlay->shared_vertices = (SharedVertex*)malloc(num_shared * sizeof(SharedVertex));
        if (overlay->shared_vertices) {
            memcpy(overlay->shared_vertices, candidates, num_shared * sizeof(SharedVertex));
            overlay->num_shared_vertices = num_shared;
        }
    }
    
    free(candidates);
}

/**
 * Compute stability score for a shared vertex
 */
double compute_vertex_stability(const SharedVertex* vertex) {
    // More solids sharing = higher stability
    // 2 solids = 0.4, 3 solids = 0.6, 4 solids = 0.8, 5 solids = 1.0
    return (double)vertex->num_solids / 5.0;
}

// ============================================================================
// OPTIMAL ANCHOR SELECTION
// ============================================================================

/**
 * Select optimal anchors from shared vertices
 */
SharedVertex** select_optimal_anchors(PlatonicOverlay* overlay, uint32_t num_anchors) {
    if (num_anchors == 0 || overlay->num_shared_vertices == 0) return NULL;
    
    // Limit to available shared vertices
    if (num_anchors > overlay->num_shared_vertices) {
        num_anchors = overlay->num_shared_vertices;
    }
    
    SharedVertex** anchors = (SharedVertex**)malloc(num_anchors * sizeof(SharedVertex*));
    if (!anchors) return NULL;
    
    // Sort by stability (simple selection sort for small arrays)
    for (uint32_t i = 0; i < num_anchors; i++) {
        uint32_t best_idx = i;
        double best_stability = 0.0;
        
        for (uint32_t j = i; j < overlay->num_shared_vertices; j++) {
            if (overlay->shared_vertices[j].stability > best_stability) {
                best_stability = overlay->shared_vertices[j].stability;
                best_idx = j;
            }
        }
        
        anchors[i] = &overlay->shared_vertices[best_idx];
        
        // Swap to maintain sorted order
        if (best_idx != i) {
            SharedVertex temp = overlay->shared_vertices[i];
            overlay->shared_vertices[i] = overlay->shared_vertices[best_idx];
            overlay->shared_vertices[best_idx] = temp;
        }
    }
    
    return anchors;
}

/**
 * Find nearest shared vertices to a target clock position
 */
SharedVertex** find_nearest_shared_vertices(PlatonicOverlay* overlay, 
                                            ClockPosition target, 
                                            uint32_t k) {
    if (k == 0 || overlay->num_shared_vertices == 0) return NULL;
    
    if (k > overlay->num_shared_vertices) {
        k = overlay->num_shared_vertices;
    }
    
    // Compute distances to all shared vertices
    double* distances = (double*)malloc(overlay->num_shared_vertices * sizeof(double));
    if (!distances) return NULL;
    
    for (uint32_t i = 0; i < overlay->num_shared_vertices; i++) {
        distances[i] = clock_distance_pi_phi(target, overlay->shared_vertices[i].clock);
    }
    
    // Find k nearest
    SharedVertex** nearest = (SharedVertex**)malloc(k * sizeof(SharedVertex*));
    if (!nearest) {
        free(distances);
        return NULL;
    }
    
    for (uint32_t i = 0; i < k; i++) {
        uint32_t min_idx = i;
        double min_dist = distances[i];
        
        for (uint32_t j = i + 1; j < overlay->num_shared_vertices; j++) {
            if (distances[j] < min_dist) {
                min_dist = distances[j];
                min_idx = j;
            }
        }
        
        nearest[i] = &overlay->shared_vertices[min_idx];
        
        // Swap
        if (min_idx != i) {
            double temp_dist = distances[i];
            distances[i] = distances[min_idx];
            distances[min_idx] = temp_dist;
            
            SharedVertex temp_vertex = overlay->shared_vertices[i];
            overlay->shared_vertices[i] = overlay->shared_vertices[min_idx];
            overlay->shared_vertices[min_idx] = temp_vertex;
        }
    }
    
    free(distances);
    return nearest;
}

// ============================================================================
// GEOMETRIC PROPERTIES
// ============================================================================

/**
 * Verify Euler's formula: V - E + F = 2
 */
bool verify_euler_formula(const PlatonicSolid* solid) {
    int euler = (int)solid->num_vertices - (int)solid->num_edges + (int)solid->num_faces;
    return (euler == 2);
}

/**
 * Get dual solid type
 */
PlatonicSolidType get_dual_solid(PlatonicSolidType type) {
    switch (type) {
        case SOLID_TETRAHEDRON: return SOLID_TETRAHEDRON;  // Self-dual
        case SOLID_CUBE: return SOLID_OCTAHEDRON;
        case SOLID_OCTAHEDRON: return SOLID_CUBE;
        case SOLID_DODECAHEDRON: return SOLID_ICOSAHEDRON;
        case SOLID_ICOSAHEDRON: return SOLID_DODECAHEDRON;
        default: return type;
    }
}

/**
 * Compute distance between two vertices in 13D space
 */
double compute_13d_distance(const double* v1, const double* v2) {
    double sum = 0.0;
    for (int i = 0; i < 13; i++) {
        double diff = v1[i] - v2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

/**
 * Check if two vertices match
 */
bool vertices_match(const double* v1, const double* v2, double tolerance) {
    return compute_13d_distance(v1, v2) < tolerance;
}

// ============================================================================
// STATISTICS & ANALYSIS
// ============================================================================

/**
 * Compute statistics for Platonic overlay
 */
OverlayStatistics compute_overlay_statistics(const PlatonicOverlay* overlay) {
    OverlayStatistics stats = {0};
    
    // Count total vertices
    for (int i = 0; i < NUM_PLATONIC_SOLIDS; i++) {
        stats.total_vertices += overlay->solids[i].num_vertices;
    }
    
    // Shared vertices
    stats.shared_vertices = overlay->num_shared_vertices;
    
    // Count highly stable (3+ solids)
    double total_stability = 0.0;
    for (uint32_t i = 0; i < overlay->num_shared_vertices; i++) {
        if (overlay->shared_vertices[i].num_solids >= 3) {
            stats.highly_stable++;
        }
        total_stability += overlay->shared_vertices[i].stability;
        
        // Ring distribution
        int ring = overlay->shared_vertices[i].clock.ring;
        if (ring >= 0 && ring < 4) {
            stats.ring_distribution[ring]++;
        }
    }
    
    stats.avg_stability = (overlay->num_shared_vertices > 0) ? 
                         total_stability / overlay->num_shared_vertices : 0.0;
    
    // Unique vertices (approximate)
    stats.unique_vertices = stats.total_vertices - stats.shared_vertices;
    
    return stats;
}

/**
 * Print Platonic overlay statistics
 */
void print_overlay_statistics(const OverlayStatistics* stats) {
    printf("\n=== Platonic Overlay Statistics ===\n");
    printf("Total vertices: %u\n", stats->total_vertices);
    printf("Unique vertices: %u\n", stats->unique_vertices);
    printf("Shared vertices: %u\n", stats->shared_vertices);
    printf("Highly stable (3+ solids): %u\n", stats->highly_stable);
    printf("Average stability: %.4f\n", stats->avg_stability);
    printf("\nRing distribution:\n");
    printf("  Ring 0: %u\n", stats->ring_distribution[0]);
    printf("  Ring 1: %u\n", stats->ring_distribution[1]);
    printf("  Ring 2: %u\n", stats->ring_distribution[2]);
    printf("  Ring 3: %u\n", stats->ring_distribution[3]);
    printf("\n");
}