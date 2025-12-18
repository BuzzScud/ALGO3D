/**
 * @file recursive_search.h
 * @brief Recursive Search for ECDLP Recovery
 * 
 * Phase 5: Implement oscillation-guided recursive search with dynamic depth.
 * Uses oscillation patterns to guide k candidate generation and trigger recursion.
 * 
 * CRITICAL: Uses PURE crystalline mathematics (NO math.h).
 */

#ifndef RECURSIVE_SEARCH_H
#define RECURSIVE_SEARCH_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (include actual headers for type definitions)
#include "oscillation_detection.h"
#include "ecdlp_integration.h"
#include "tetration_attractors.h"

/**
 * Search state
 * 
 * Tracks the current state of the recursive search
 */
typedef struct {
    uint32_t depth;                     // Current recursion depth
    uint32_t max_depth;                 // Maximum allowed depth
    uint64_t iterations;                // Total iterations so far
    uint64_t max_iterations;            // Maximum iterations allowed
    double best_score;                  // Best score found so far
    BIGNUM* best_k;                     // Best k candidate found
    bool converged;                     // Has search converged?
    double convergence_threshold;       // Threshold for convergence
} SearchState;

/**
 * Search strategy
 * 
 * Defines the search strategy based on oscillation patterns
 */
typedef enum {
    STRATEGY_RANDOM,                    // Random search (baseline)
    STRATEGY_ATTRACTOR_GUIDED,          // Guided by tetration attractors
    STRATEGY_OSCILLATION_GUIDED,        // Guided by oscillation patterns
    STRATEGY_HYBRID                     // Combination of attractor + oscillation
} SearchStrategy;

/**
 * Search parameters
 * 
 * Configuration for recursive search
 */
typedef struct {
    SearchStrategy strategy;            // Search strategy to use
    uint32_t initial_depth;             // Starting recursion depth
    uint32_t max_depth;                 // Maximum recursion depth
    uint64_t max_iterations;            // Maximum total iterations
    double convergence_threshold;       // Convergence threshold
    uint32_t candidates_per_iteration;  // Number of candidates per iteration
    double oscillation_weight;          // Weight for oscillation guidance [0,1]
    double attractor_weight;            // Weight for attractor guidance [0,1]
    bool enable_backtracking;           // Enable backtracking on divergence
    bool enable_adaptive_depth;         // Enable dynamic depth adjustment
} SearchParameters;

/**
 * Search result
 * 
 * Result of recursive search
 */
typedef struct {
    BIGNUM* recovered_k;                // Recovered k value (if successful)
    bool success;                       // Was recovery successful?
    uint32_t final_depth;               // Final recursion depth reached
    uint64_t total_iterations;          // Total iterations performed
    double final_score;                 // Final score achieved
    double elapsed_time;                // Time taken (seconds)
    uint32_t num_backtracks;            // Number of backtracks performed
    uint32_t num_depth_adjustments;     // Number of depth adjustments
} SearchResult;

// ============================================================================
// SEARCH STATE MANAGEMENT
// ============================================================================

/**
 * Create search state
 * 
 * @param params Search parameters
 * @return Search state, or NULL on failure
 */
SearchState* create_search_state(const SearchParameters* params);

/**
 * Free search state
 * 
 * @param state Search state to free
 */
void free_search_state(SearchState* state);

/**
 * Update search state
 * 
 * Updates state after each iteration
 * 
 * @param state Search state
 * @param k_candidate Current k candidate
 * @param score Score for this candidate
 * @return true if should continue, false if should stop
 */
bool update_search_state(SearchState* state, const BIGNUM* k_candidate, double score);

/**
 * Check convergence
 * 
 * Checks if search has converged
 * 
 * @param state Search state
 * @return true if converged, false otherwise
 */
bool check_convergence(const SearchState* state);

// ============================================================================
// OSCILLATION-GUIDED CANDIDATE GENERATION
// ============================================================================

/**
 * Generate k candidates using oscillation patterns
 * 
 * Uses oscillation frequency, amplitude, and phase to predict k values
 * 
 * Algorithm:
 * 1. Analyze oscillation patterns in each dimension
 * 2. Identify dimensions with strong oscillations
 * 3. Use frequency to predict periodicity
 * 4. Use phase to predict offset
 * 5. Use amplitude to estimate search radius
 * 6. Generate candidates around predicted values
 * 7. Score candidates by oscillation fit
 * 
 * @param instance ECDLP instance
 * @param oscillation_map Current oscillation map
 * @param Q_embedding Target lattice embedding
 * @param current_k Current k value (for relative prediction)
 * @param candidates Output array for k candidates
 * @param max_candidates Maximum number of candidates
 * @return Number of candidates generated
 */
int generate_oscillation_guided_candidates(
    const ECDLPInstance* instance,
    const OscillationMap* oscillation_map,
    const ECLatticeEmbedding* Q_embedding,
    const BIGNUM* current_k,
    BIGNUM** candidates,
    int max_candidates
);

/**
 * Generate k candidates using attractors
 * 
 * Uses tetration attractors to generate candidates
 * 
 * @param instance ECDLP instance
 * @param attractors Attractor system
 * @param Q_embedding Target lattice embedding
 * @param candidates Output array for k candidates
 * @param max_candidates Maximum number of candidates
 * @return Number of candidates generated
 */
int generate_attractor_guided_candidates(
    const ECDLPInstance* instance,
    const AttractorSystem* attractors,
    const ECLatticeEmbedding* Q_embedding,
    BIGNUM** candidates,
    int max_candidates
);

/**
 * Generate hybrid candidates
 * 
 * Combines oscillation and attractor guidance
 * 
 * @param instance ECDLP instance
 * @param oscillation_map Oscillation map
 * @param attractors Attractor system
 * @param Q_embedding Target lattice embedding
 * @param current_k Current k value
 * @param params Search parameters (for weights)
 * @param candidates Output array for k candidates
 * @param max_candidates Maximum number of candidates
 * @return Number of candidates generated
 */
int generate_hybrid_candidates(
    const ECDLPInstance* instance,
    const OscillationMap* oscillation_map,
    const AttractorSystem* attractors,
    const ECLatticeEmbedding* Q_embedding,
    const BIGNUM* current_k,
    const SearchParameters* params,
    BIGNUM** candidates,
    int max_candidates
);

// ============================================================================
// CANDIDATE SCORING
// ============================================================================

/**
 * Score k candidate
 * 
 * Scores how well a k candidate fits the target
 * 
 * Scoring factors:
 * 1. Lattice distance to target
 * 2. Oscillation pattern fit
 * 3. Attractor proximity
 * 4. Convergence trend
 * 
 * @param instance ECDLP instance
 * @param k_candidate Candidate k value
 * @param Q_embedding Target lattice embedding
 * @param oscillation_map Current oscillation map (optional)
 * @param attractors Attractor system (optional)
 * @param params Search parameters (for weights)
 * @return Score [0, 1] (higher is better)
 */
double score_k_candidate(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding,
    const OscillationMap* oscillation_map,
    const AttractorSystem* attractors,
    const SearchParameters* params
);

/**
 * Score by lattice distance
 * 
 * Scores based on distance in 15D lattice
 * 
 * @param instance ECDLP instance
 * @param k_candidate Candidate k value
 * @param Q_embedding Target lattice embedding
 * @return Score [0, 1] (higher is better)
 */
double score_by_lattice_distance(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding
);

/**
 * Score by oscillation fit
 * 
 * Scores based on how well candidate fits oscillation pattern
 * 
 * @param instance ECDLP instance
 * @param k_candidate Candidate k value
 * @param oscillation_map Oscillation map
 * @return Score [0, 1] (higher is better)
 */
double score_by_oscillation_fit(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const OscillationMap* oscillation_map
);

/**
 * Score by attractor proximity
 * 
 * Scores based on proximity to tetration attractors
 * 
 * @param k_candidate Candidate k value
 * @param attractors Attractor system
 * @return Score [0, 1] (higher is better)
 */
double score_by_attractor_proximity(
    const BIGNUM* k_candidate,
    const AttractorSystem* attractors
);

// ============================================================================
// RECURSIVE SEARCH
// ============================================================================

/**
 * Recursive search
 * 
 * Main recursive search function
 * 
 * Algorithm:
 * 1. Check termination conditions (depth, iterations, convergence)
 * 2. Generate k candidates based on strategy
 * 3. Score each candidate
 * 4. Select best candidate
 * 5. If oscillations unstable -> recurse deeper
 * 6. If converging -> continue at current depth
 * 7. If diverging -> backtrack
 * 8. Update search state
 * 9. Repeat until convergence or termination
 * 
 * @param instance ECDLP instance
 * @param Q_embedding Target lattice embedding
 * @param params Search parameters
 * @param state Current search state (modified)
 * @param current_k Current k value (starting point)
 * @return Best k candidate found at this level
 */
BIGNUM* recursive_search(
    const ECDLPInstance* instance,
    const ECLatticeEmbedding* Q_embedding,
    const SearchParameters* params,
    SearchState* state,
    const BIGNUM* current_k
);

/**
 * Adaptive depth adjustment
 * 
 * Adjusts recursion depth based on oscillation stability
 * 
 * Rules:
 * - Unstable oscillations -> increase depth
 * - Stable oscillations -> maintain depth
 * - Converging oscillations -> decrease depth
 * 
 * @param state Search state
 * @param oscillation_map Current oscillation map
 * @return New depth
 */
uint32_t adjust_depth(SearchState* state, const OscillationMap* oscillation_map);

/**
 * Backtrack
 * 
 * Backtracks to previous depth when diverging
 * 
 * @param state Search state
 * @return true if backtrack successful, false if at root
 */
bool backtrack(SearchState* state);

// ============================================================================
// HIGH-LEVEL API
// ============================================================================

/**
 * Recover k using recursive search
 * 
 * High-level function that performs complete recovery
 * 
 * @param instance ECDLP instance
 * @param params Search parameters
 * @return Search result
 */
SearchResult* recover_k_recursive(
    const ECDLPInstance* instance,
    const SearchParameters* params
);

/**
 * Free search result
 * 
 * @param result Search result to free
 */
void free_search_result(SearchResult* result);

/**
 * Initialize default search parameters
 * 
 * @param params Search parameters to initialize
 */
void init_default_search_parameters(SearchParameters* params);

/**
 * Print search result
 * 
 * @param result Search result
 */
void print_search_result(const SearchResult* result);

/**
 * Print search parameters
 * 
 * @param params Search parameters
 */
void print_search_parameters(const SearchParameters* params);

#ifdef __cplusplus
}
#endif

#endif // RECURSIVE_SEARCH_H