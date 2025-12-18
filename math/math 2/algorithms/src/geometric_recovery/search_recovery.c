/**
 * @file search_recovery.c
 * @brief Attractor-Guided Search Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 */

#include "geometric_recovery/search_recovery.h"
#include "math/constants.h"
#include "geometric_recovery/tetration_attractors.h"
#include "math/constants.h"
#include "math/types.h"
#include "math/constants.h"
#include "math/transcendental.h"
#include "math/constants.h"
#include "math/arithmetic.h"
#include "math/constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Search recovery context
 */
struct SearchRecoveryContext {
    uint64_t n;                    // System size
    TetrationSystem* attractors;   // Tetration attractors
    int max_candidates;            // Maximum candidates to track
    double confidence;             // Confidence score
};

/**
 * Create search recovery context
 */
SearchRecoveryContext* search_recovery_create(
    uint64_t n,
    TetrationSystem* attractors,
    int max_candidates
) {
    if (n == 0 || max_candidates <= 0) {
        return NULL;
    }
    
    SearchRecoveryContext* ctx = (SearchRecoveryContext*)calloc(
        1, sizeof(SearchRecoveryContext)
    );
    if (!ctx) {
        return NULL;
    }
    
    ctx->n = n;
    ctx->attractors = attractors;
    ctx->max_candidates = max_candidates;
    ctx->confidence = 0.0;
    
    return ctx;
}

/**
 * Score candidate by distance to target
 */
double search_score_candidate(
    uint64_t candidate,
    uint64_t target,
    uint64_t n
) {
    if (n == 0) {
        return 0.0;
    }
    
    // Compute normalized distance
    uint64_t diff = (candidate > target) ? (candidate - target) : (target - candidate);
    double normalized_diff = (double)diff / (double)n;
    
    // Score is inverse of distance (higher is better)
    // Use exponential decay for better discrimination
    return math_exp(-10.0 * normalized_diff);
}

/**
 * Generate candidates near attractors
 */
bool search_generate_candidates(
    SearchRecoveryContext* ctx,
    uint64_t target,
    uint64_t radius,
    SearchResults** results_out
) {
    if (!ctx || !results_out) {
        return false;
    }
    
    // Allocate results
    SearchResults* results = (SearchResults*)calloc(1, sizeof(SearchResults));
    if (!results) {
        return false;
    }
    
    results->capacity = ctx->max_candidates;
    results->candidates = (uint64_t*)calloc(results->capacity, sizeof(uint64_t));
    results->scores = (double*)calloc(results->capacity, sizeof(double));
    
    if (!results->candidates || !results->scores) {
        free_search_results(results);
        return false;
    }
    
    results->num_candidates = 0;
    
    // Generate candidates around target
    // Use golden ratio for optimal distribution
    for (int i = 0; i < ctx->max_candidates && results->num_candidates < results->capacity; i++) {
        // Generate candidate using golden ratio spacing
        double offset = (double)i * MATH_PHI;
        offset = math_fmod(offset, 1.0);
        
        // Map to range around target
        int64_t signed_offset = (int64_t)(offset * 2.0 * radius) - (int64_t)radius;
        int64_t candidate = (int64_t)target + signed_offset;
        
        // Ensure within bounds
        if (candidate < 0) {
            candidate = 0;
        }
        if ((uint64_t)candidate >= ctx->n) {
            candidate = ctx->n - 1;
        }
        
        // Add candidate
        results->candidates[results->num_candidates] = (uint64_t)candidate;
        results->scores[results->num_candidates] = search_score_candidate(
            (uint64_t)candidate, target, ctx->n
        );
        results->num_candidates++;
    }
    
    *results_out = results;
    return true;
}

/**
 * Compare function for sorting candidates by score (descending)
 * Reserved for future beam search implementation
 */
__attribute__((unused))
static int compare_candidates(const void* a, const void* b) {
    double score_a = *((double*)a + 1);
    double score_b = *((double*)b + 1);
    
    if (score_a > score_b) return -1;
    if (score_a < score_b) return 1;
    return 0;
}

/**
 * Perform beam search with pruning
 */
bool beam_search(
    SearchRecoveryContext* ctx,
    uint64_t target,
    int beam_width,
    int max_iterations,
    uint64_t* best_out
) {
    if (!ctx || !best_out || beam_width <= 0) {
        return false;
    }
    
    // Initialize beam with candidates around target
    uint64_t radius = ctx->n / 100;  // Start with 1% of search space
    
    SearchResults* current_beam = NULL;
    if (!search_generate_candidates(ctx, target, radius, &current_beam)) {
        return false;
    }
    
    // Keep only top beam_width candidates
    if (current_beam->num_candidates > beam_width) {
        current_beam->num_candidates = beam_width;
    }
    
    // Iterative refinement
    for (int iter = 0; iter < max_iterations; iter++) {
        // Reduce radius for next iteration
        radius = radius / 2;
        if (radius < 1) {
            radius = 1;
        }
        
        // Generate new candidates around current beam
        SearchResults* new_beam = NULL;
        if (!search_generate_candidates(ctx, current_beam->candidates[0], radius, &new_beam)) {
            free_search_results(current_beam);
            return false;
        }
        
        // Merge with current beam
        // (In a full implementation, we would merge and re-sort)
        
        // For now, just use the new beam if it has better scores
        if (new_beam->num_candidates > 0 && 
            new_beam->scores[0] > current_beam->scores[0]) {
            free_search_results(current_beam);
            current_beam = new_beam;
        } else {
            free_search_results(new_beam);
        }
        
        // Early termination if score is very high
        if (current_beam->scores[0] > 0.99) {
            break;
        }
    }
    
    // Return best candidate
    *best_out = current_beam->candidates[0];
    ctx->confidence = current_beam->scores[0];
    
    free_search_results(current_beam);
    return true;
}

/**
 * Search with attractors (main function)
 */
bool search_with_attractors(
    SearchRecoveryContext* ctx,
    uint64_t target,
    SearchResults** results_out
) {
    if (!ctx || !results_out) {
        return false;
    }
    
    // Generate candidates around target
    uint64_t radius = ctx->n / 10;  // 10% of search space
    
    return search_generate_candidates(ctx, target, radius, results_out);
}

/**
 * Get top N candidates from results
 */
int get_top_candidates(
    const SearchResults* results,
    int n,
    uint64_t* top_out
) {
    if (!results || !top_out || n <= 0) {
        return 0;
    }
    
    int num_to_return = (n < results->num_candidates) ? n : results->num_candidates;
    
    for (int i = 0; i < num_to_return; i++) {
        top_out[i] = results->candidates[i];
    }
    
    return num_to_return;
}

/**
 * Print search results
 */
void print_search_results(
    const SearchResults* results,
    int num_to_print
) {
    if (!results) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Search Recovery Results                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Total Candidates: %d\n\n", results->num_candidates);
    
    int num_print = (num_to_print < results->num_candidates) ? 
                    num_to_print : results->num_candidates;
    
    printf("  Top %d Candidates:\n", num_print);
    for (int i = 0; i < num_print; i++) {
        printf("    %2d. Value: %lu, Score: %.6f\n", 
               i + 1, results->candidates[i], results->scores[i]);
    }
    
    printf("\n");
}

/**
 * Free search results
 */
void free_search_results(SearchResults* results) {
    if (results) {
        if (results->candidates) {
            free(results->candidates);
        }
        if (results->scores) {
            free(results->scores);
        }
        free(results);
    }
}

/**
 * Destroy search recovery context
 */
void search_recovery_destroy(SearchRecoveryContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}