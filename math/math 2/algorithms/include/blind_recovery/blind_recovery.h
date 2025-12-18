/**
 * @file blind_recovery.h
 * @brief Universal Blind Recovery Algorithm (OBJECTIVE 28)
 * 
 * Revolutionary algorithm for detecting, analyzing, and stabilizing
 * oscillating multi-dimensional structures.
 * 
 * KEY INNOVATION: Uses oscillations as information rather than noise
 * 
 * ARCHITECTURE:
 * - Layer 1: Detection & Mapping (oscillation detection, structural analysis)
 * - Layer 2: Anchor-Based Triangulation (stable reference points)
 * - Layer 3: Iterative Search with SFT (efficient candidate generation)
 * - Layer 4: Recursive Stabilization (multi-scale convergence)
 * - Layer 5: Dynamic Model Expansion (self-similar structures)
 * - Layer 6: Hyper-Dimensional Analysis (4D+ support)
 * 
 * PERFORMANCE TARGETS:
 * - 5% corruption: 99.9% recovery in <1s
 * - 10% corruption: 99.5% recovery in <5s
 * - 25% corruption: 95.0% recovery in <60s
 * 
 * See OBJECTIVE_28_DETAILED_SPEC.md for complete specification
 */

#ifndef BLIND_RECOVERY_H
#define BLIND_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// LAYER 1: DETECTION & MAPPING
// ============================================================================

/**
 * Oscillation signature for a single dimension
 */
typedef struct {
    uint32_t dimension;           // Which dimension is oscillating
    double frequency;             // Oscillation frequency (Hz)
    double amplitude;             // Oscillation amplitude
    double phase;                 // Phase offset (radians)
    uint64_t period;              // Period in iterations
    bool is_stable;               // Stable vs unstable oscillation
    double convergence_rate;      // Rate of convergence (if converging)
} OscillationSignature;

/**
 * Complete oscillation map across all dimensions
 */
typedef struct {
    uint32_t num_dimensions;
    OscillationSignature* signatures;  // [num_dimensions]
    double* cross_correlations;        // [num_dimensions × num_dimensions]
    bool is_converging;
    uint64_t iterations_to_convergence;
} OscillationMap;

/**
 * Structural map of geometric properties
 */
typedef struct {
    uint32_t num_vertices;
    uint32_t num_edges;
    uint32_t num_faces;
    uint64_t** coprime_matrix;         // GCD relationships [n×n]
    double* dimensional_offsets;       // Offset from ideal geometry
    bool* corruption_mask;             // Which elements are corrupted
    double corruption_percentage;
} StructuralMap;

/**
 * Detect oscillations in structure using FFT
 * 
 * @param structure_data Raw structure data
 * @param num_dimensions Number of dimensions
 * @param num_samples Number of samples to analyze
 * @param sampling_rate Sampling rate (Hz)
 * @return Oscillation map (caller must free)
 */
OscillationMap* detect_oscillations(
    const double* structure_data,
    uint32_t num_dimensions,
    uint32_t num_samples,
    double sampling_rate
);

/**
 * Map geometric structure properties
 * 
 * @param num_vertices Number of vertices (V)
 * @param num_edges Number of edges (E)
 * @param num_faces Number of faces (F)
 * @param vertex_positions Vertex positions [num_vertices × 3]
 * @return Structural map (caller must free)
 */
StructuralMap* map_structure(
    uint32_t num_vertices,
    uint32_t num_edges,
    uint32_t num_faces,
    const double* vertex_positions
);

/**
 * Compute coprime relationships (GCD matrix)
 * 
 * @param dimension_sizes Size of each dimension
 * @param num_dimensions Number of dimensions
 * @return GCD matrix [num_dimensions × num_dimensions] (caller must free)
 */
uint64_t** compute_coprime_matrix(
    const uint32_t* dimension_sizes,
    uint32_t num_dimensions
);

/**
 * Detect corrupted elements in structure
 * 
 * @param structure_data Raw structure data
 * @param num_elements Number of elements
 * @param threshold Corruption threshold
 * @return Corruption mask (caller must free)
 */
bool* detect_corruption(
    const double* structure_data,
    uint32_t num_elements,
    double threshold
);

/**
 * Free oscillation map
 */
void free_oscillation_map(OscillationMap* map);

/**
 * Free structural map
 */
void free_structural_map(StructuralMap* map);

/**
 * Free coprime matrix
 */
void free_coprime_matrix(uint64_t** matrix, uint32_t num_dimensions);

// ============================================================================
// LAYER 2: ANCHOR-BASED TRIANGULATION
// ============================================================================

/**
 * Anchor point for triangulation
 */
typedef struct {
    uint32_t vertex_id;            // Vertex ID in original structure
    double* position;              // [3] - 3D coordinates (dynamically allocated)
    double confidence;             // 0.0 to 1.0
    bool is_corrupted;             // Whether anchor is corrupted
    uint32_t num_neighbors;        // Number of neighbors
    uint32_t* neighbor_ids;        // Neighbor vertex IDs
} AnchorPoint;

/**
 * Complete anchor system
 */
typedef struct {
    uint32_t num_anchors;
    AnchorPoint* anchors;
    double* triangulation_matrix;  // [num_anchors × num_anchors]
    double global_confidence;
} AnchorSystem;

/**
 * Recovery quality metrics
 */
typedef struct {
    uint32_t total_vertices;
    uint32_t corrupted_vertices;
    uint32_t recovered_vertices;
    double avg_confidence;
    double min_confidence;
    double max_confidence;
    double avg_corrupted_confidence;
    double recovery_rate;
    double corruption_percentage;
} RecoveryMetrics;

/**
 * Create anchor system with initial allocation
 */
AnchorSystem* create_anchor_system(uint32_t max_anchors);

/**
 * Select optimal anchor points using greedy maximum separation
 */
AnchorSystem* select_anchors(
    const StructuralMap* structure,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices
);

/**
 * Validate anchor system quality
 */
bool validate_anchor_system(const AnchorSystem* system);

/**
 * Recover single vertex using least-squares triangulation
 */
bool recover_vertex_triangulation(
    const AnchorSystem* anchors,
    const double* expected_distances,
    double* recovered_position,
    double* confidence
);

/**
 * Recover all corrupted vertices using triangulation
 */
bool recover_all_vertices(
    const AnchorSystem* anchors,
    const StructuralMap* structure,
    double* vertex_positions,
    double* confidence_scores,
    uint32_t num_vertices
);

/**
 * Compute triangulation quality metrics
 */
void compute_triangulation_metrics(
    const AnchorSystem* anchors,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices,
    double* avg_confidence,
    double* min_confidence,
    double* max_confidence
);

/**
 * Detect if anchor may be corrupted
 */
bool detect_anchor_corruption(
    const AnchorSystem* system,
    uint32_t anchor_idx,
    const double* vertex_positions,
    uint32_t num_vertices
);

/**
 * Refine anchor position using neighboring vertices
 */
bool refine_anchor_position(
    AnchorPoint* anchor,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices
);

/**
 * Update anchor confidence scores based on recovery results
 */
void update_anchor_confidence(
    AnchorSystem* system,
    const double* vertex_positions,
    const double* confidence_scores,
    uint32_t num_vertices
);

/**
 * Replace corrupted anchor with new candidate
 */
bool replace_corrupted_anchor(
    AnchorSystem* system,
    uint32_t anchor_idx,
    const double* vertex_positions,
    const double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices
);

/**
 * Perform iterative anchor adjustment
 */
uint32_t adjust_anchors_iterative(
    AnchorSystem* system,
    double* vertex_positions,
    double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices,
    uint32_t max_iterations
);

/**
 * Compute vertex confidence score
 */
double compute_vertex_confidence(
    const double* vertex_position,
    const AnchorSystem* anchors,
    const double* expected_distances
);

/**
 * Compute confidence scores for all vertices
 */
void compute_all_confidence_scores(
    const double* vertex_positions,
    const AnchorSystem* anchors,
    const StructuralMap* structure,
    double* confidence_scores,
    uint32_t num_vertices
);

/**
 * Compute global recovery metrics
 */
void compute_recovery_metrics(
    const double* confidence_scores,
    const bool* corruption_mask,
    uint32_t num_vertices,
    RecoveryMetrics* metrics
);

/**
 * Validate recovery quality against thresholds
 */
bool validate_recovery_quality(
    const RecoveryMetrics* metrics,
    double min_recovery_rate,
    double min_avg_confidence
);

/**
 * Compute structural confidence based on Euler's formula
 */
double compute_structural_confidence(
    const StructuralMap* structure,
    uint32_t vertex_id
);

/**
 * Update confidence scores iteratively
 */
void update_confidence_iterative(
    double* confidence_scores,
    const double* previous_scores,
    uint32_t num_vertices,
    double learning_rate
);

/**
 * Free anchor system
 */
void free_anchor_system(AnchorSystem* system);

// ============================================================================
// LAYER 3: ITERATIVE SEARCH WITH SFT
// ============================================================================

/**
 * Search candidate
 */
typedef struct {
    uint64_t candidate;            // Candidate value
    uint32_t dimension;            // Which dimension
    double sft_score;              // SFT heuristic score
    bool is_prime;                 // Primality (if applicable)
    uint32_t clock_position;       // Position on clock lattice
} SearchCandidate;

/**
 * Iterative search state
 */
typedef struct {
    uint32_t max_iterations;
    double convergence_threshold;
    uint64_t nonce;                // Random seed for search
    SearchCandidate* candidates;   // Candidate pool
    uint32_t num_candidates;
} IterativeSearch;

/**
 * Generate candidates using SFT
 * 
 * @param nonce Random seed
 * @param dimension Target dimension
 * @param num_candidates Number of candidates to generate
 * @param structure Structural map
 * @return Array of candidates (caller must free)
 */
SearchCandidate* generate_candidates(
    uint64_t nonce,
    uint32_t dimension,
    uint32_t num_candidates,
    const StructuralMap* structure
);

/**
 * Score candidate fitness
 * 
 * @param candidate Search candidate
 * @param structure Structural map
 * @param oscillations Oscillation map
 * @return Fitness score (higher is better)
 */
double score_candidate(
    const SearchCandidate* candidate,
    const StructuralMap* structure,
    const OscillationMap* oscillations
);

/**
 * Iterative refinement
 * 
 * @param structure Structural map
 * @param search Iterative search state
 * @param convergence_threshold Convergence threshold
 * @return true if converged, false otherwise
 */
bool refine_structure(
    StructuralMap* structure,
    IterativeSearch* search,
    double convergence_threshold
);

/**
 * Free iterative search
 */
void free_iterative_search(IterativeSearch* search);

/**
 * Generate candidates for all dimensions
 */
SearchCandidate** generate_candidates_all_dimensions(
    uint64_t nonce,
    uint32_t num_candidates_per_dim,
    const StructuralMap* structure,
    uint32_t* total_candidates
);

/**
 * Free candidate array
 */
void free_candidates(SearchCandidate* candidates);

/**
 * Free all candidates
 */
void free_all_candidates(SearchCandidate** candidates, uint32_t num_dimensions);

/**
 * Score and sort candidates by fitness
 */
void score_and_sort_candidates(
    SearchCandidate* candidates,
    uint32_t num_candidates,
    const StructuralMap* structure,
    const OscillationMap* oscillations
);

/**
 * Select top N candidates
 */
SearchCandidate* select_top_candidates(
    const SearchCandidate* candidates,
    uint32_t num_candidates,
    uint32_t num_to_select
);

/**
 * Compute average fitness of candidates
 */
double compute_average_fitness(
    const SearchCandidate* candidates,
    uint32_t num_candidates
);

/**
 * Find best candidate
 */
const SearchCandidate* find_best_candidate(
    const SearchCandidate* candidates,
    uint32_t num_candidates
);

/**
 * Create iterative search state
 */
IterativeSearch* create_iterative_search(
    uint32_t max_iterations,
    double convergence_threshold,
    uint64_t nonce
);

/**
 * Initialize search with candidates
 */
bool initialize_search_candidates(
    IterativeSearch* search,
    SearchCandidate* candidates,
    uint32_t num_candidates
);

/**
 * Perform multi-iteration refinement
 */
bool refine_structure_multi_pass(
    StructuralMap* structure,
    const OscillationMap* oscillations,
    uint32_t num_passes,
    uint32_t candidates_per_pass,
    double convergence_threshold
);

/**
 * Check if structure has converged
 */
bool is_structure_converged(
    const StructuralMap* structure,
    double threshold
);

/**
 * Convergence history (opaque type)
 */
typedef struct ConvergenceHistory ConvergenceHistory;

/**
 * Create convergence history
 */
ConvergenceHistory* create_convergence_history(uint32_t size);

/**
 * Add to convergence history
 */
void add_to_convergence_history(ConvergenceHistory* history, double value);

/**
 * Check convergence with history
 */
bool is_converged_with_history(
    const ConvergenceHistory* history,
    double threshold,
    double variance_threshold
);

/**
 * Free convergence history
 */
void free_convergence_history(ConvergenceHistory* history);

/**
 * Detect early convergence
 */
bool detect_early_convergence(
    const OscillationMap* oscillations,
    const StructuralMap* structure,
    ConvergenceHistory* history,
    double threshold
);

// ============================================================================
// LAYER 4: RECURSIVE STABILIZATION
// ============================================================================

/**
 * Scale level for multi-scale analysis
 */
typedef struct {
    uint32_t scale_level;          // 0 = finest, N = coarsest
    double resolution;             // Spatial resolution at this scale
    OscillationMap* oscillations;  // Oscillations at this scale
    bool is_stable;
} ScaleLevel;

/**
 * Multi-scale analysis
 */
typedef struct {
    uint32_t num_scales;
    ScaleLevel* scales;            // [num_scales]
    uint32_t current_scale;        // Which scale we're working on
    bool all_scales_stable;
} MultiScaleAnalysis;

/**
 * Analyze structure at multiple scales
 * 
 * @param structure Structural map
 * @param num_scales Number of scales to analyze
 * @return Multi-scale analysis (caller must free)
 */
MultiScaleAnalysis* analyze_multi_scale(
    const StructuralMap* structure,
    uint32_t num_scales
);

/**
 * Recursive stabilization
 * 
 * @param structure Structural map
 * @param analysis Multi-scale analysis
 * @param max_depth Maximum recursion depth
 * @return true if stabilized, false otherwise
 */
bool stabilize_recursive(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis,
    uint32_t max_depth
);

/**
 * Detect convergence
 * 
 * @param oscillations Oscillation map
 * @param threshold Convergence threshold
 * @return true if converged, false otherwise
 */
bool is_converged(
    const OscillationMap* oscillations,
    double threshold
);

/**
 * Free multi-scale analysis
 */
void free_multi_scale_analysis(MultiScaleAnalysis* analysis);

/**
 * Get scale level by index
 */
const ScaleLevel* get_scale_level(
    const MultiScaleAnalysis* analysis,
    uint32_t scale_index
);

/**
 * Check if scale is stable
 */
bool is_scale_stable(
    const MultiScaleAnalysis* analysis,
    uint32_t scale_index
);

/**
 * Get coarsest stable scale
 */
uint32_t get_coarsest_stable_scale(const MultiScaleAnalysis* analysis);

/**
 * Get finest unstable scale
 */
uint32_t get_finest_unstable_scale(const MultiScaleAnalysis* analysis);

/**
 * Update scale stability
 */
void update_scale_stability(
    MultiScaleAnalysis* analysis,
    uint32_t scale_index,
    bool is_stable
);

/**
 * Stabilize structure at single scale
 */
bool stabilize_at_scale(
    StructuralMap* structure,
    ScaleLevel* scale,
    uint32_t max_iterations
);

/**
 * Stabilize iteratively across scales
 */
bool stabilize_iterative(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis,
    uint32_t max_iterations
);

/**
 * Adaptive stabilization (chooses best strategy)
 */
bool stabilize_adaptive(
    StructuralMap* structure,
    MultiScaleAnalysis* analysis
);

/**
 * Compute stabilization quality
 */
double compute_stabilization_quality(const MultiScaleAnalysis* analysis);

// ============================================================================
// LAYER 5: DYNAMIC MODEL EXPANSION (Removed - see Phase 5 below)
// ============================================================================

// ============================================================================
// LAYER 6: HYPER-DIMENSIONAL ANALYSIS
// ============================================================================

/**
 * Hyper-dimensional structure (>3D)
 */
typedef struct {
    uint32_t num_dimensions;       // Could be 4, 5, 6, ... dimensions
    uint32_t* dimension_sizes;     // Size of each dimension
    uint64_t** coprime_matrix;     // GCD relationships [n×n]
    double* offset_vectors;        // Offset in each dimension
    bool* dimension_stability;     // Is each dimension stable?
} HyperDimensionalStructure;

/**
 * Scalar analysis
 */
typedef struct {
    double scalar_value;           // Scaling factor
    OscillationMap* oscillations;  // Oscillations at this scale
    double* variance_per_axis;     // Variance along each axis
    double* variance_per_vertex;   // Variance at each vertex
    bool is_stable;
} ScalarAnalysis;

/**
 * Multi-scalar analysis
 */
typedef struct {
    uint32_t num_scalars;
    ScalarAnalysis* analyses;      // [num_scalars]
    double* cross_scalar_correlations;
} MultiScalarAnalysis;

/**
 * Analyze hyper-dimensional structure
 * 
 * @param num_dimensions Number of dimensions
 * @param dimension_sizes Size of each dimension
 * @param structure_data Structure data
 * @return Hyper-dimensional structure (caller must free)
 */
HyperDimensionalStructure* analyze_hyperdimensional(
    uint32_t num_dimensions,
    const uint32_t* dimension_sizes,
    const double* structure_data
);

/**
 * Multi-scalar analysis
 * 
 * @param structure Hyper-dimensional structure
 * @param scalar_values Array of scalar values
 * @param num_scalars Number of scalars
 * @return Multi-scalar analysis (caller must free)
 */
MultiScalarAnalysis* analyze_multi_scalar(
    const HyperDimensionalStructure* structure,
    const double* scalar_values,
    uint32_t num_scalars
);



/**
 * Free hyper-dimensional structure
 */
void free_hyperdimensional_structure(HyperDimensionalStructure* structure);

/**
 * Free multi-scalar analysis
 */
void free_multi_scalar_analysis(MultiScalarAnalysis* analysis);

/**
 * Check if hyper-dimensional structure is valid
 */
bool is_hyperdimensional_valid(const HyperDimensionalStructure* structure);

/**
 * Get dimension size
 */
uint32_t get_dimension_size(
    const HyperDimensionalStructure* structure,
    uint32_t dimension
);

/**
 * Check if dimension is stable
 */
bool is_dimension_stable(
    const HyperDimensionalStructure* structure,
    uint32_t dimension
);

/**
 * Get GCD between two dimensions
 */
uint64_t get_dimension_gcd(
    const HyperDimensionalStructure* structure,
    uint32_t dim1,
    uint32_t dim2
);

/**
 * Get total elements in structure
 */
uint64_t get_total_elements(const HyperDimensionalStructure* structure);

/**
 * Check if all dimensions are stable
 */
bool are_all_dimensions_stable(const HyperDimensionalStructure* structure);

/**
 * Get scalar analysis at index
 */
const ScalarAnalysis* get_scalar_analysis(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar_index
);

/**
 * Get cross-scalar correlation
 */
double get_cross_scalar_correlation(
    const MultiScalarAnalysis* analysis,
    uint32_t scalar1,
    uint32_t scalar2
);

/**
 * Check if all scalars are stable
 */
bool are_all_scalars_stable(const MultiScalarAnalysis* analysis);

/**
 * Compute average variance across scalars
 */
double compute_average_variance(
    const MultiScalarAnalysis* analysis,
    uint32_t dimension
);

/**
 * Find most stable scalar
 */
uint32_t find_most_stable_scalar(const MultiScalarAnalysis* analysis);

/**
 * Compute multi-scalar consistency score
 */
double compute_multi_scalar_consistency(const MultiScalarAnalysis* analysis);

/**
 * Compute variance for structure
 */
void compute_variance(
    const HyperDimensionalStructure* structure,
    const double* structure_data,
    double* variance_per_axis,
    double* variance_per_vertex
);

/**
 * Compute cross-correlation matrix
 */
double* compute_cross_correlation_matrix(
    const HyperDimensionalStructure* structure,
    const double* structure_data
);

/**
 * Get correlation between dimensions
 */
double get_dimension_correlation(
    const double* correlation_matrix,
    uint32_t num_dimensions,
    uint32_t dim1,
    uint32_t dim2
);

/**
 * Compute average correlation
 */
double compute_average_correlation(
    const double* correlation_matrix,
    uint32_t num_dimensions
);

/**
 * Compute correlation strength
 */
double compute_correlation_strength(
    const double* correlation_matrix,
    uint32_t num_dimensions
);

/**
 * Validate cross-dimensional consistency
 */
bool validate_cross_dimensional_consistency(
    const double* correlation_matrix,
    uint32_t num_dimensions
);

/**
 * Detect corruption by variance
 */
bool detect_corruption_by_variance(
    const HyperDimensionalStructure* structure,
    const double* structure_data,
    double threshold,
    uint64_t* num_corrupted
);

// ============================================================================
// HIGH-LEVEL API
// ============================================================================

/**
 * Recovery statistics
 */
typedef struct {
    int total_corruptions;
    int successful_recoveries;
    int failed_recoveries;
    double recovery_rate;
    uint64_t recovery_time_ns;
} RecoveryStatistics;

/**
 * Complete blind recovery (all layers)
 * 
 * @param structure_data Raw structure data
 * @param num_elements Number of elements
 * @param num_dimensions Number of dimensions
 * @param corruption_threshold Corruption detection threshold
 * @return Recovery statistics
 */
RecoveryStatistics blind_recovery_complete(
    double* structure_data,
    uint32_t num_elements,
    uint32_t num_dimensions,
    double corruption_threshold
);

/**
 * Universal Blind Recovery - FIXED VERSION 2
 * 
 * Improvements over v1:
 * - Tracks actual vertex position evolution across passes
 * - Uses adaptive threshold for recovery metrics
 * - Improved tetration attractor computation
 * - Better oscillation detection from real data
 */
RecoveryStatistics blind_recovery_complete_v2(
    double* structure_data,
    uint32_t num_elements,
    uint32_t num_dimensions,
    double corruption_threshold
);

// ============================================================================
// PHASE 5: DYNAMIC MODEL EXPANSION
// ============================================================================

/**
 * Platonic solid types
 */
typedef enum {
    PLATONIC_TETRAHEDRON = 0,
    PLATONIC_CUBE = 1,
    PLATONIC_OCTAHEDRON = 2,
    PLATONIC_DODECAHEDRON = 3,
    PLATONIC_ICOSAHEDRON = 4
} PlatonicSolidType;

/**
 * Expansion metrics
 */
typedef struct {
    double vertex_ratio;
    double edge_ratio;
    double face_ratio;
    double corruption_reduction;
    bool euler_maintained;
    double quality_score;
} ExpansionMetrics;

/**
 * Self-similar hierarchy
 */
typedef struct {
    uint32_t num_levels;
    StructuralMap** levels;
} SelfSimilarHierarchy;

/**
 * Hierarchy metrics
 */
typedef struct {
    uint32_t num_levels;
    double self_similarity_score;
    bool all_levels_valid;
    double average_corruption;
    uint32_t total_vertices;
} HierarchyMetrics;

/**
 * Expand model to target Platonic solid
 * 
 * @param source Source structure
 * @param target_solid Target Platonic solid type
 * @return Expanded structure (caller must free)
 */
StructuralMap* expand_to_platonic_solid(
    const StructuralMap* source,
    PlatonicSolidType target_solid
);

/**
 * Expand model by level
 * 
 * @param source Source structure
 * @param expansion_level Expansion level (1, 2, 3, ...)
 * @return Expanded structure (caller must free)
 */
StructuralMap* expand_model_by_level(
    const StructuralMap* source,
    uint32_t expansion_level
);

/**
 * Check if expansion is valid
 * 
 * @param expanded Expanded structure
 * @return true if valid (Euler's formula maintained)
 */
bool is_expansion_valid(const StructuralMap* expanded);

/**
 * Get expansion factor between Platonic solids
 * 
 * @param source Source solid type
 * @param target Target solid type
 * @return Expansion factor
 */
double get_expansion_factor(
    PlatonicSolidType source,
    PlatonicSolidType target
);

/**
 * Get available expansions for a Platonic solid
 * 
 * @param source Source solid type
 * @param targets Output array of target solid types
 * @param max_targets Maximum number of targets
 * @return Number of available expansions
 */
int get_available_expansions(
    PlatonicSolidType source,
    PlatonicSolidType* targets,
    int max_targets
);

/**
 * Compute expansion metrics
 * 
 * @param source Source structure
 * @param expanded Expanded structure
 * @param metrics Output metrics
 */
void compute_expansion_metrics(
    const StructuralMap* source,
    const StructuralMap* expanded,
    ExpansionMetrics* metrics
);

/**
 * Free expanded model
 * 
 * @param expanded Expanded structure
 */
void free_expanded_model(StructuralMap* expanded);

/**
 * Generate self-similar hierarchy
 * 
 * @param base Base structure
 * @param num_levels Number of levels in hierarchy
 * @return Self-similar hierarchy (caller must free)
 */
SelfSimilarHierarchy* generate_self_similar_hierarchy(
    const StructuralMap* base,
    uint32_t num_levels
);

/**
 * Validate self-similar hierarchy
 * 
 * @param hierarchy Hierarchy to validate
 * @return true if valid
 */
bool validate_self_similar_hierarchy(const SelfSimilarHierarchy* hierarchy);

/**
 * Compute self-similarity score
 * 
 * @param hierarchy Hierarchy
 * @return Self-similarity score [0, 1]
 */
double compute_self_similarity_score(const SelfSimilarHierarchy* hierarchy);

/**
 * Get level from hierarchy
 * 
 * @param hierarchy Hierarchy
 * @param level Level index
 * @return Structure at level (NULL if invalid)
 */
const StructuralMap* get_hierarchy_level(
    const SelfSimilarHierarchy* hierarchy,
    uint32_t level
);

/**
 * Free self-similar hierarchy
 * 
 * @param hierarchy Hierarchy to free
 */
void free_self_similar_hierarchy(SelfSimilarHierarchy* hierarchy);

/**
 * Apply recovery using hierarchy
 * 
 * @param hierarchy Hierarchy
 * @param target_level Target level to recover
 * @return true if successful
 */
bool recover_using_hierarchy(
    SelfSimilarHierarchy* hierarchy,
    uint32_t target_level
);

/**
 * Compute hierarchy metrics
 * 
 * @param hierarchy Hierarchy
 * @param metrics Output metrics
 */
void compute_hierarchy_metrics(
    const SelfSimilarHierarchy* hierarchy,
    HierarchyMetrics* metrics
);

#ifdef __cplusplus
}
#endif

#endif /* BLIND_RECOVERY_H */
