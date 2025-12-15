/**
 * @file search_recovery.h
 * @brief Attractor-Guided Search Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 * 
 * Implements attractor-guided search using:
 * - Candidate generation near attractors
 * - Candidate scoring by distance
 * - Beam search with pruning
 * - Early termination
 * 
 * NO crypto-specific dependencies - works with raw uint64_t data.
 */

#ifndef SEARCH_RECOVERY_H
#define SEARCH_RECOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct TetrationSystem TetrationSystem;

/**
 * Search results
 */
typedef struct {
    uint64_t* candidates;      // Array of candidate values
    double* scores;            // Scores for each candidate
    int num_candidates;        // Number of candidates
    int capacity;              // Capacity of arrays
} SearchResults;

/**
 * Search recovery context
 */
typedef struct SearchRecoveryContext SearchRecoveryContext;

/**
 * Create search recovery context
 * 
 * @param n System size
 * @param attractors Tetration attractors for guidance
 * @param max_candidates Maximum number of candidates to track
 * @return New context or NULL on error
 */
SearchRecoveryContext* search_recovery_create(
    uint64_t n,
    TetrationSystem* attractors,
    int max_candidates
);

/**
 * Generate candidates near attractors
 * 
 * @param ctx Context
 * @param target Target output value
 * @param radius Search radius around attractors
 * @param results_out Pointer to store results
 * @return true on success, false on error
 */
bool search_generate_candidates(
    SearchRecoveryContext* ctx,
    uint64_t target,
    uint64_t radius,
    SearchResults** results_out
);

/**
 * Score candidate by distance to target
 * 
 * @param candidate Candidate value
 * @param target Target value
 * @param n System size
 * @return Score (higher is better)
 */
double search_score_candidate(
    uint64_t candidate,
    uint64_t target,
    uint64_t n
);

/**
 * Perform beam search with pruning
 * 
 * @param ctx Context
 * @param target Target output value
 * @param beam_width Width of beam
 * @param max_iterations Maximum iterations
 * @param best_out Pointer to store best candidate
 * @return true on success, false on error
 */
bool beam_search(
    SearchRecoveryContext* ctx,
    uint64_t target,
    int beam_width,
    int max_iterations,
    uint64_t* best_out
);

/**
 * Search with attractors (main function)
 * 
 * @param ctx Context
 * @param target Target output value
 * @param results_out Pointer to store results
 * @return true on success, false on error
 */
bool search_with_attractors(
    SearchRecoveryContext* ctx,
    uint64_t target,
    SearchResults** results_out
);

/**
 * Get top N candidates from results
 * 
 * @param results Search results
 * @param n Number of top candidates to get
 * @param top_out Array to store top candidates
 * @return Number of candidates returned
 */
int get_top_candidates(
    const SearchResults* results,
    int n,
    uint64_t* top_out
);

/**
 * Print search results
 * 
 * @param results Results to print
 * @param num_to_print Number of results to print
 */
void print_search_results(
    const SearchResults* results,
    int num_to_print
);

/**
 * Free search results
 * 
 * @param results Results to free
 */
void free_search_results(SearchResults* results);

/**
 * Destroy search recovery context
 * 
 * @param ctx Context to destroy
 */
void search_recovery_destroy(SearchRecoveryContext* ctx);

#endif // SEARCH_RECOVERY_H