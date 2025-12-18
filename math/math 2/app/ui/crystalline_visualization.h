/**
 * Crystalline Memory Visualization
 * 
 * Provides real-time visualization of the crystalline memory structure,
 * including 12-fold symmetry, kissing boundaries, cache placement,
 * and hierarchical relationships.
 */

#ifndef CRYSTALLINE_VISUALIZATION_H
#define CRYSTALLINE_VISUALIZATION_H

#include "../app_common.h"
#include "../../include/ai/cllm_crystalline_memory.h"
#include "../../include/ai/cllm_cache_optimization.h"

// ============================================================================
// VISUALIZATION STRUCTURES
// ============================================================================

/**
 * Crystalline Sphere Visualization Data
 * 
 * Extended visualization data for a single sphere, including
 * crystalline memory and cache information.
 */
typedef struct {
    int sphere_id;
    int symmetry_group;           // 0-11
    int hierarchy_level;          // 0 = root, 1+
    int is_control;               // 1 if control thread
    
    // Position and geometry
    double theta;                 // Angular position θ
    int x, y;                     // Screen coordinates
    int radius;                   // Display radius
    
    // Memory statistics
    uint64_t total_reads;
    uint64_t total_writes;
    uint64_t cache_hits;
    uint64_t cache_misses;
    
    // Cache placement
    int cache_line;
    int numa_node;
    int cpu_core;
    
    // Activity metrics
    int batches_processed;
    float avg_loss;
    float activity_level;         // 0.0 to 1.0
    
    // Segment visualization (12 segments per sphere)
    struct {
        uint64_t reads;
        uint64_t writes;
        float activity;           // 0.0 to 1.0
    } segments[12];
    
    // Parent-child relationships
    int parent_id;
    int child_ids[12];
    int num_children;
    
} CrystallineSphereViz;

/**
 * Kissing Boundary Visualization Data
 */
typedef struct {
    int segment_a;                // First segment (0-11)
    int segment_b;                // Second segment (0-11)
    uint64_t accesses_a;
    uint64_t accesses_b;
    uint32_t version;
    float activity_level;         // 0.0 to 1.0
} KissingBoundaryViz;

/**
 * Complete Crystalline Visualization State
 */
typedef struct {
    CrystallineSphereViz spheres[144];  // Up to 144 spheres (12^2)
    int num_active_spheres;
    
    KissingBoundaryViz boundaries[66];  // 12 choose 2 = 66 boundaries per level
    int num_active_boundaries;
    
    // Global statistics
    uint64_t total_memory_allocated;
    uint64_t total_reads;
    uint64_t total_writes;
    float cache_hit_rate;
    
    // Hierarchy visualization
    int max_hierarchy_level;
    int spheres_per_level[5];     // Count per level (0-4)
    
} CrystallineVizState;

// ============================================================================
// VISUALIZATION FUNCTIONS
// ============================================================================

/**
 * Initialize crystalline visualization state
 */
void crystalline_viz_init(CrystallineVizState* state);

/**
 * Update visualization from training system
 * 
 * Extracts current state from the training system and updates
 * the visualization data structures.
 */
void crystalline_viz_update(CrystallineVizState* viz_state, AppState* app_state);

/**
 * Draw crystalline memory visualization
 * 
 * Renders the complete crystalline memory structure including:
 * - 12-fold symmetry layout
 * - Memory segments with activity
 * - Kissing boundaries
 * - Cache placement
 * - Hierarchy relationships
 */
void draw_crystalline_visualization(SDL_Renderer* renderer, 
                                   CrystallineVizState* viz_state,
                                   SDL_Rect bounds);

/**
 * Draw individual sphere with crystalline details
 */
void draw_crystalline_sphere(SDL_Renderer* renderer,
                            CrystallineSphereViz* sphere,
                            bool show_segments,
                            bool show_cache_info);

/**
 * Draw kissing boundaries between spheres
 */
void draw_kissing_boundaries(SDL_Renderer* renderer,
                            CrystallineVizState* viz_state,
                            bool show_activity);

/**
 * Draw memory statistics overlay
 */
void draw_memory_statistics(SDL_Renderer* renderer,
                           CrystallineVizState* viz_state,
                           SDL_Rect bounds);

/**
 * Draw cache placement visualization
 */
void draw_cache_placement(SDL_Renderer* renderer,
                         CrystallineVizState* viz_state,
                         SDL_Rect bounds);

/**
 * Draw hierarchy tree
 */
void draw_hierarchy_tree(SDL_Renderer* renderer,
                        CrystallineVizState* viz_state,
                        SDL_Rect bounds);

#endif // CRYSTALLINE_VISUALIZATION_H