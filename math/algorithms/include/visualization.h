#ifndef ALGORITHMS_VISUALIZATION_H
#define ALGORITHMS_VISUALIZATION_H

/**
 * Unified Visualization Framework
 * 
 * Generic visualization framework supporting 2D, 3D, and Crystalline modes.
 * This is THE visualization system - not an optional variant.
 * 
 * The framework provides:
 * - Mathematical projection algorithms (2D, 3D, Crystalline)
 * - Statistics calculation
 * - Data export (JSON, CSV)
 * - Connection graph visualization
 * - Symmetry analysis
 * 
 * This is generic and can visualize:
 * - Neural network structures (CLLM)
 * - Graph structures
 * - Particle systems
 * - Lattice structures
 * - Any data with spatial relationships
 * 
 * The UI layer handles rendering (SDL/OpenGL), while this layer
 * handles the mathematical projections and data preparation.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CONSTANTS
// ============================================================================

#define VIZ_MAX_DIMENSIONS 3
#define VIZ_MAX_CONNECTIONS 12  // For kissing spheres

// ============================================================================
// VISUALIZATION MODES
// ============================================================================

/**
 * Visualization Mode
 * 
 * Determines how data is projected for visualization.
 */
typedef enum {
    VIZ_MODE_2D,          // 2D projection (X, Y)
    VIZ_MODE_3D,          // 3D projection (X, Y, Z)
    VIZ_MODE_CRYSTALLINE  // Crystalline structure (clock lattice)
} VisualizationMode;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Point in visualization space
 */
typedef struct {
    uint32_t id;                    // Point ID
    double position[VIZ_MAX_DIMENSIONS]; // Position (2D or 3D)
    double value;                   // Value to visualize (color/size)
    uint32_t symmetry_group;        // Symmetry group (0-11)
    
    // Connections
    uint32_t neighbors[VIZ_MAX_CONNECTIONS];
    uint32_t num_neighbors;
    
    // Metadata
    char label[64];                 // Optional label
    
} VisualizationPoint;

/**
 * Visualization Data
 * 
 * Contains all data needed for visualization.
 */
typedef struct {
    // Configuration
    VisualizationMode mode;
    uint32_t num_points;
    
    // Points
    VisualizationPoint* points;
    
    // Bounds (for scaling)
    double min_bounds[VIZ_MAX_DIMENSIONS];
    double max_bounds[VIZ_MAX_DIMENSIONS];
    
    // Statistics
    double avg_value;
    double min_value;
    double max_value;
    
    // Symmetry distribution
    uint32_t symmetry_counts[12];
    
    // Metadata
    char title[128];
    char description[256];
    
} VisualizationData;

/**
 * Visualization Statistics
 */
typedef struct {
    uint32_t num_points;
    uint32_t num_connections;
    double avg_neighbors;
    double load_balance_score;
    double symmetry_distribution[12];
    double spatial_extent[VIZ_MAX_DIMENSIONS];
} VisualizationStatistics;

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Create visualization data
 * 
 * @param mode Visualization mode
 * @param num_points Number of points
 * @return Initialized visualization data, or NULL on error
 */
VisualizationData* viz_create(VisualizationMode mode, uint32_t num_points);

/**
 * Free visualization data
 * 
 * @param data Visualization data to free
 */
void viz_free(VisualizationData* data);

// ============================================================================
// DATA POPULATION
// ============================================================================

/**
 * Set point position
 * 
 * @param data Visualization data
 * @param point_id Point ID
 * @param position Position array (size depends on mode)
 * @return 0 on success, -1 on error
 */
int viz_set_point_position(
    VisualizationData* data,
    uint32_t point_id,
    const double* position
);

/**
 * Set point value
 * 
 * @param data Visualization data
 * @param point_id Point ID
 * @param value Value to visualize
 * @return 0 on success, -1 on error
 */
int viz_set_point_value(
    VisualizationData* data,
    uint32_t point_id,
    double value
);

/**
 * Set point neighbors
 * 
 * @param data Visualization data
 * @param point_id Point ID
 * @param neighbors Array of neighbor IDs
 * @param num_neighbors Number of neighbors
 * @return 0 on success, -1 on error
 */
int viz_set_point_neighbors(
    VisualizationData* data,
    uint32_t point_id,
    const uint32_t* neighbors,
    uint32_t num_neighbors
);

/**
 * Set point label
 * 
 * @param data Visualization data
 * @param point_id Point ID
 * @param label Label string
 * @return 0 on success, -1 on error
 */
int viz_set_point_label(
    VisualizationData* data,
    uint32_t point_id,
    const char* label
);

// ============================================================================
// PROJECTION ALGORITHMS
// ============================================================================

/**
 * Project to 2D
 * 
 * Projects data to 2D space using appropriate algorithm.
 * Updates point positions in the visualization data.
 * 
 * @param data Visualization data
 * @return 0 on success, -1 on error
 */
int viz_project_2d(VisualizationData* data);

/**
 * Project to 3D
 * 
 * Projects data to 3D space using appropriate algorithm.
 * Updates point positions in the visualization data.
 * 
 * @param data Visualization data
 * @return 0 on success, -1 on error
 */
int viz_project_3d(VisualizationData* data);

/**
 * Project to Crystalline structure
 * 
 * Projects data to crystalline lattice structure using clock geometry.
 * Updates point positions based on Babylonian clock (12, 60, 60, 100).
 * 
 * @param data Visualization data
 * @return 0 on success, -1 on error
 */
int viz_project_crystalline(VisualizationData* data);

// ============================================================================
// STATISTICS
// ============================================================================

/**
 * Calculate visualization statistics
 * 
 * @param data Visualization data
 * @param stats Output statistics
 * @return 0 on success, -1 on error
 */
int viz_calculate_statistics(
    const VisualizationData* data,
    VisualizationStatistics* stats
);

/**
 * Update bounds
 * 
 * Recalculates min/max bounds based on current point positions.
 * 
 * @param data Visualization data
 * @return 0 on success, -1 on error
 */
int viz_update_bounds(VisualizationData* data);

/**
 * Calculate symmetry distribution
 * 
 * @param data Visualization data
 * @return 0 on success, -1 on error
 */
int viz_calculate_symmetry_distribution(VisualizationData* data);

// ============================================================================
// EXPORT
// ============================================================================

/**
 * Export to JSON
 * 
 * Exports visualization data to JSON format for external tools.
 * 
 * @param data Visualization data
 * @param filename Output filename
 * @return 0 on success, -1 on error
 */
int viz_export_json(const VisualizationData* data, const char* filename);

/**
 * Export to CSV
 * 
 * Exports point data to CSV format.
 * 
 * @param data Visualization data
 * @param filename Output filename
 * @return 0 on success, -1 on error
 */
int viz_export_csv(const VisualizationData* data, const char* filename);

// ============================================================================
// RENDERING DATA PREPARATION
// ============================================================================

/**
 * Get rendering data for 2D
 * 
 * Prepares data for 2D rendering (UI layer).
 * Returns arrays of positions and connections.
 * 
 * @param data Visualization data
 * @param positions Output: array of [x, y] positions
 * @param connections Output: array of connection pairs
 * @param num_connections Output: number of connections
 * @return 0 on success, -1 on error
 */
int viz_get_2d_rendering_data(
    const VisualizationData* data,
    double** positions,
    uint32_t** connections,
    uint32_t* num_connections
);

/**
 * Get rendering data for 3D
 * 
 * Prepares data for 3D rendering (UI layer).
 * Returns arrays of positions and connections.
 * 
 * @param data Visualization data
 * @param positions Output: array of [x, y, z] positions
 * @param connections Output: array of connection pairs
 * @param num_connections Output: number of connections
 * @return 0 on success, -1 on error
 */
int viz_get_3d_rendering_data(
    const VisualizationData* data,
    double** positions,
    uint32_t** connections,
    uint32_t* num_connections
);

/**
 * Get rendering data for Crystalline
 * 
 * Prepares data for crystalline structure rendering (UI layer).
 * Returns clock lattice positions and ring information.
 * 
 * @param data Visualization data
 * @param ring_positions Output: array of [ring, position] pairs
 * @param connections Output: array of connection pairs
 * @param num_connections Output: number of connections
 * @return 0 on success, -1 on error
 */
int viz_get_crystalline_rendering_data(
    const VisualizationData* data,
    double** ring_positions,
    uint32_t** connections,
    uint32_t* num_connections
);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Print visualization summary
 * 
 * @param data Visualization data
 * @param output Output file
 */
void viz_print_summary(const VisualizationData* data, FILE* output);

/**
 * Validate visualization data
 * 
 * @param data Visualization data
 * @return 1 if valid, 0 if issues found
 */
int viz_validate(const VisualizationData* data);

#ifdef __cplusplus
}
#endif

#endif /* ALGORITHMS_VISUALIZATION_H */
