/**
 * @file recursive_search.c
 * @brief Recursive Search Implementation
 * 
 * Implements oscillation-guided recursive search using pure crystalline math.
 */

#include "../include/recursive_search.h"
#include "../include/ecdlp_integration.h"
#include "../include/oscillation_detection.h"
#include "../include/tetration_attractors.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============================================================================
// SEARCH STATE MANAGEMENT
// ============================================================================

SearchState* create_search_state(const SearchParameters* params) {
    if (!params) return NULL;
    
    SearchState* state = (SearchState*)malloc(sizeof(SearchState));
    if (!state) return NULL;
    
    state->depth = params->initial_depth;
    state->max_depth = params->max_depth;
    state->iterations = 0;
    state->max_iterations = params->max_iterations;
    state->best_score = 0.0;
    state->best_k = BN_new();
    state->converged = false;
    state->convergence_threshold = params->convergence_threshold;
    
    if (!state->best_k) {
        free(state);
        return NULL;
    }
    
    BN_zero(state->best_k);
    
    return state;
}

void free_search_state(SearchState* state) {
    if (!state) return;
    if (state->best_k) BN_free(state->best_k);
    free(state);
}

bool update_search_state(SearchState* state, const BIGNUM* k_candidate, double score) {
    if (!state || !k_candidate) return false;
    
    state->iterations++;
    
    // Update best if this is better
    if (score > state->best_score) {
        state->best_score = score;
        BN_copy(state->best_k, k_candidate);
    }
    
    // Check termination conditions
    if (state->iterations >= state->max_iterations) {
        return false;  // Stop: max iterations reached
    }
    
    if (state->depth >= state->max_depth) {
        return false;  // Stop: max depth reached
    }
    
    // Check convergence
    if (check_convergence(state)) {
        state->converged = true;
        return false;  // Stop: converged
    }
    
    return true;  // Continue
}

bool check_convergence(const SearchState* state) {
    if (!state) return false;
    
    // Converged if best score exceeds threshold
    return (state->best_score >= state->convergence_threshold);
}

// ============================================================================
// OSCILLATION-GUIDED CANDIDATE GENERATION
// ============================================================================

int generate_oscillation_guided_candidates(
    const ECDLPInstance* instance,
    const OscillationMap* oscillation_map,
    const ECLatticeEmbedding* Q_embedding,
    const BIGNUM* current_k,
    BIGNUM** candidates,
    int max_candidates
) {
    if (!instance || !oscillation_map || !Q_embedding || !candidates || max_candidates <= 0) {
        return 0;
    }
    
    int count = 0;
    
    // Find dimensions with strongest oscillations
    uint32_t strongest_dim = 0;
    double max_amplitude = 0.0;
    
    for (uint32_t i = 0; i < oscillation_map->num_dimensions; i++) {
        if (oscillation_map->signatures[i].amplitude > max_amplitude) {
            max_amplitude = oscillation_map->signatures[i].amplitude;
            strongest_dim = i;
        }
    }
    
    // Use frequency and phase to predict next k values
    double frequency = oscillation_map->signatures[strongest_dim].frequency;
    double phase = oscillation_map->signatures[strongest_dim].phase;
    double amplitude = oscillation_map->signatures[strongest_dim].amplitude;
    
    // Predict period
    uint64_t period = (frequency > 0.0) ? (uint64_t)(1.0 / frequency) : 1;
    if (period == 0) period = 1;
    
    // Generate candidates around predicted values
    uint64_t current_k_val = current_k ? BN_get_word(current_k) : 1;
    
    for (int i = 0; i < max_candidates && count < max_candidates; i++) {
        // Predict next k based on period and phase
        uint64_t predicted_k = current_k_val + period * (i + 1);
        
        // Add phase offset
        int64_t phase_offset = (int64_t)(phase * period / (2.0 * 3.14159265358979323846));
        predicted_k += phase_offset;
        
        // Add amplitude-based variation
        int64_t amplitude_variation = (int64_t)(amplitude * period * 0.1);
        
        // Generate multiple candidates around prediction
        for (int j = -2; j <= 2 && count < max_candidates; j++) {
            uint64_t k_val = predicted_k + j * amplitude_variation;
            
            if (k_val > 0) {
                BIGNUM* candidate = BN_new();
                if (candidate) {
                    BN_set_word(candidate, k_val);
                    
                    // Ensure candidate < order
                    if (BN_cmp(candidate, instance->order) >= 0) {
                        BN_mod(candidate, candidate, instance->order, NULL);
                    }
                    
                    candidates[count++] = candidate;
                }
            }
        }
    }
    
    return count;
}

int generate_attractor_guided_candidates(
    const ECDLPInstance* instance,
    const AttractorSystem* attractors,
    const ECLatticeEmbedding* Q_embedding,
    BIGNUM** candidates,
    int max_candidates
) {
    if (!instance || !attractors || !Q_embedding || !candidates || max_candidates <= 0) {
        return 0;
    }
    
    int count = 0;
    
    // Generate candidates from each attractor
    for (uint32_t i = 0; i < attractors->num_attractors && count < max_candidates; i++) {
        uint64_t attractor_value = attractors->attractors[i].value;
        
        // Generate candidates around attractor
        for (int j = -5; j <= 5 && count < max_candidates; j++) {
            uint64_t k_val = attractor_value + j * 10;
            
            if (k_val > 0) {
                BIGNUM* candidate = BN_new();
                if (candidate) {
                    BN_set_word(candidate, k_val);
                    
                    // Ensure candidate < order
                    if (BN_cmp(candidate, instance->order) >= 0) {
                        BN_mod(candidate, candidate, instance->order, NULL);
                    }
                    
                    candidates[count++] = candidate;
                }
            }
        }
    }
    
    return count;
}

int generate_hybrid_candidates(
    const ECDLPInstance* instance,
    const OscillationMap* oscillation_map,
    const AttractorSystem* attractors,
    const ECLatticeEmbedding* Q_embedding,
    const BIGNUM* current_k,
    const SearchParameters* params,
    BIGNUM** candidates,
    int max_candidates
) {
    if (!instance || !params || !candidates || max_candidates <= 0) {
        return 0;
    }
    
    int count = 0;
    
    // Split candidates between oscillation and attractor guidance
    int osc_count = (int)(max_candidates * params->oscillation_weight);
    int attr_count = max_candidates - osc_count;
    
    // Generate oscillation-guided candidates
    if (oscillation_map && osc_count > 0) {
        int generated = generate_oscillation_guided_candidates(
            instance, oscillation_map, Q_embedding, current_k,
            candidates, osc_count
        );
        count += generated;
    }
    
    // Generate attractor-guided candidates
    if (attractors && attr_count > 0) {
        int generated = generate_attractor_guided_candidates(
            instance, attractors, Q_embedding,
            candidates + count, attr_count
        );
        count += generated;
    }
    
    return count;
}

// ============================================================================
// CANDIDATE SCORING
// ============================================================================

double score_k_candidate(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding,
    const OscillationMap* oscillation_map,
    const AttractorSystem* attractors,
    const SearchParameters* params
) {
    if (!instance || !k_candidate || !Q_embedding || !params) {
        return 0.0;
    }
    
    double total_score = 0.0;
    double total_weight = 0.0;
    
    // Score by lattice distance (always included)
    double lattice_score = score_by_lattice_distance(instance, k_candidate, Q_embedding);
    total_score += lattice_score * 0.5;  // 50% weight
    total_weight += 0.5;
    
    // Score by oscillation fit (if available)
    if (oscillation_map && params->oscillation_weight > 0.0) {
        double osc_score = score_by_oscillation_fit(instance, k_candidate, oscillation_map);
        total_score += osc_score * params->oscillation_weight * 0.3;  // 30% weight
        total_weight += params->oscillation_weight * 0.3;
    }
    
    // Score by attractor proximity (if available)
    if (attractors && params->attractor_weight > 0.0) {
        double attr_score = score_by_attractor_proximity(k_candidate, attractors);
        total_score += attr_score * params->attractor_weight * 0.2;  // 20% weight
        total_weight += params->attractor_weight * 0.2;
    }
    
    // Normalize
    if (total_weight > 0.0) {
        total_score /= total_weight;
    }
    
    return total_score;
}

double score_by_lattice_distance(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding
) {
    if (!instance || !k_candidate || !Q_embedding) {
        return 0.0;
    }
    
    // Compute k_candidate * G
    EC_POINT* test_point = EC_POINT_new(instance->group);
    if (!test_point) return 0.0;
    
    if (!EC_POINT_mul(instance->group, test_point, k_candidate, NULL, NULL, NULL)) {
        EC_POINT_free(test_point);
        return 0.0;
    }
    
    // Get coordinates and embed
    ECPointCoords* coords = ec_get_point_coords(instance->group, test_point);
    EC_POINT_free(test_point);
    
    if (!coords) return 0.0;
    
    ECLatticeEmbedding test_embedding;
    if (!lattice_embed_ec_point(coords, &test_embedding)) {
        ec_free_point_coords(coords);
        return 0.0;
    }
    ec_free_point_coords(coords);
    
    // Compute distance
    double distance = lattice_distance(&test_embedding, Q_embedding);
    
    // Convert to score (inverse distance, normalized)
    double score = 1.0 / (1.0 + distance);
    
    return score;
}

double score_by_oscillation_fit(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const OscillationMap* oscillation_map
) {
    if (!instance || !k_candidate || !oscillation_map) {
        return 0.0;
    }
    
    // Simple heuristic: score based on how well k fits the oscillation period
    uint64_t k_val = BN_get_word(k_candidate);
    
    double total_fit = 0.0;
    int count = 0;
    
    for (uint32_t i = 0; i < oscillation_map->num_dimensions; i++) {
        if (oscillation_map->signatures[i].period > 0) {
            uint64_t period = oscillation_map->signatures[i].period;
            uint64_t remainder = k_val % period;
            
            // Fit is better when k is close to a multiple of period
            double fit = 1.0 - (double)remainder / (double)period;
            if (fit < 0.5) fit = 1.0 - fit;  // Symmetric around 0.5
            
            total_fit += fit;
            count++;
        }
    }
    
    if (count > 0) {
        return total_fit / count;
    }
    
    return 0.5;  // Neutral score
}

double score_by_attractor_proximity(
    const BIGNUM* k_candidate,
    const AttractorSystem* attractors
) {
    if (!k_candidate || !attractors) {
        return 0.0;
    }
    
    uint64_t k_val = BN_get_word(k_candidate);
    
    // Find nearest attractor
    uint64_t min_distance = UINT64_MAX;
    
    for (uint32_t i = 0; i < attractors->num_attractors; i++) {
        uint64_t attractor_val = attractors->attractors[i].value;
        uint64_t distance = (k_val > attractor_val) ? 
                           (k_val - attractor_val) : 
                           (attractor_val - k_val);
        
        if (distance < min_distance) {
            min_distance = distance;
        }
    }
    
    // Convert to score (inverse distance, normalized)
    double score = 1.0 / (1.0 + (double)min_distance / 1000.0);
    
    return score;
}

// ============================================================================
// RECURSIVE SEARCH
// ============================================================================

BIGNUM* recursive_search(
    const ECDLPInstance* instance,
    const ECLatticeEmbedding* Q_embedding,
    const SearchParameters* params,
    SearchState* state,
    const BIGNUM* current_k
) {
    if (!instance || !Q_embedding || !params || !state) {
        return NULL;
    }
    
    // Check termination conditions
    if (state->depth >= state->max_depth || 
        state->iterations >= state->max_iterations ||
        state->converged) {
        return state->best_k ? BN_dup(state->best_k) : NULL;
    }
    
    // Sample trajectory for oscillation detection
    uint32_t num_samples = 64;
    uint64_t k_start = current_k ? BN_get_word(current_k) : 1;
    
    ECTrajectory* trajectory = sample_ec_trajectory(instance, k_start, 1, num_samples);
    if (!trajectory) {
        return state->best_k ? BN_dup(state->best_k) : NULL;
    }
    
    // Detect oscillations
    OscillationMap* osc_map = detect_oscillations(trajectory);
    free_ec_trajectory(trajectory);
    
    if (!osc_map) {
        return state->best_k ? BN_dup(state->best_k) : NULL;
    }
    
    // Create attractor system
    AttractorSystem* attractors = create_attractor_system(1000000, 0.95);
    if (attractors) {
        compute_all_attractors(attractors);
    }
    
    // Generate candidates based on strategy
    BIGNUM* candidates[100];
    int num_candidates = 0;
    
    switch (params->strategy) {
        case STRATEGY_OSCILLATION_GUIDED:
            num_candidates = generate_oscillation_guided_candidates(
                instance, osc_map, Q_embedding, current_k,
                candidates, 100
            );
            break;
            
        case STRATEGY_ATTRACTOR_GUIDED:
            if (attractors) {
                num_candidates = generate_attractor_guided_candidates(
                    instance, attractors, Q_embedding,
                    candidates, 100
                );
            }
            break;
            
        case STRATEGY_HYBRID:
            num_candidates = generate_hybrid_candidates(
                instance, osc_map, attractors, Q_embedding, current_k, params,
                candidates, 100
            );
            break;
            
        default:
            // Random strategy - not implemented
            break;
    }
    
    // Score and select best candidate
    BIGNUM* best_candidate = NULL;
    double best_score = 0.0;
    
    for (int i = 0; i < num_candidates; i++) {
        double score = score_k_candidate(
            instance, candidates[i], Q_embedding,
            osc_map, attractors, params
        );
        
        if (score > best_score) {
            best_score = score;
            if (best_candidate) BN_free(best_candidate);
            best_candidate = BN_dup(candidates[i]);
        }
        
        // Update state
        update_search_state(state, candidates[i], score);
        
        // Free candidate
        BN_free(candidates[i]);
    }
    
    // Adaptive depth adjustment
    if (params->enable_adaptive_depth) {
        uint32_t new_depth = adjust_depth(state, osc_map);
        state->depth = new_depth;
    }
    
    // Cleanup
    free_oscillation_map(osc_map);
    if (attractors) free_attractor_system(attractors);
    
    // Recurse if not converged and depth allows
    if (!state->converged && state->depth < state->max_depth && best_candidate) {
        state->depth++;
        BIGNUM* recursive_result = recursive_search(
            instance, Q_embedding, params, state, best_candidate
        );
        
        if (recursive_result) {
            BN_free(best_candidate);
            return recursive_result;
        }
    }
    
    return best_candidate;
}

uint32_t adjust_depth(SearchState* state, const OscillationMap* oscillation_map) {
    if (!state || !oscillation_map) {
        return state->depth;
    }
    
    // Count unstable dimensions
    int unstable_count = 0;
    int converging_count = 0;
    
    for (uint32_t i = 0; i < oscillation_map->num_dimensions; i++) {
        if (!oscillation_map->signatures[i].is_stable) {
            if (oscillation_map->signatures[i].convergence_rate > 0.0) {
                converging_count++;
            } else {
                unstable_count++;
            }
        }
    }
    
    uint32_t new_depth = state->depth;
    
    // Increase depth if many unstable dimensions
    if (unstable_count > 5) {
        new_depth = (state->depth < state->max_depth) ? state->depth + 1 : state->depth;
    }
    // Decrease depth if many converging dimensions
    else if (converging_count > 10) {
        new_depth = (state->depth > 1) ? state->depth - 1 : state->depth;
    }
    
    return new_depth;
}

bool backtrack(SearchState* state) {
    if (!state || state->depth == 0) {
        return false;
    }
    
    state->depth--;
    return true;
}

// ============================================================================
// HIGH-LEVEL API
// ============================================================================

SearchResult* recover_k_recursive(
    const ECDLPInstance* instance,
    const SearchParameters* params
) {
    if (!instance || !params) {
        return NULL;
    }
    
    SearchResult* result = (SearchResult*)malloc(sizeof(SearchResult));
    if (!result) return NULL;
    
    // Initialize result
    result->recovered_k = NULL;
    result->success = false;
    result->final_depth = 0;
    result->total_iterations = 0;
    result->final_score = 0.0;
    result->elapsed_time = 0.0;
    result->num_backtracks = 0;
    result->num_depth_adjustments = 0;
    
    // Start timer
    clock_t start_time = clock();
    
    // Get Q embedding
    ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
    if (!Q_coords) {
        free(result);
        return NULL;
    }
    
    ECLatticeEmbedding Q_embedding;
    if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
        ec_free_point_coords(Q_coords);
        free(result);
        return NULL;
    }
    ec_free_point_coords(Q_coords);
    
    // Create search state
    SearchState* state = create_search_state(params);
    if (!state) {
        free(result);
        return NULL;
    }
    
    // Perform recursive search
    BIGNUM* start_k = BN_new();
    BN_one(start_k);
    
    BIGNUM* recovered = recursive_search(instance, &Q_embedding, params, state, start_k);
    BN_free(start_k);
    
    // Fill result
    result->recovered_k = recovered;
    result->success = (recovered != NULL) && ecdlp_verify_solution(instance, recovered);
    result->final_depth = state->depth;
    result->total_iterations = state->iterations;
    result->final_score = state->best_score;
    result->num_backtracks = 0;  // TODO: Track backtracks
    result->num_depth_adjustments = 0;  // TODO: Track adjustments
    
    // Stop timer
    clock_t end_time = clock();
    result->elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    free_search_state(state);
    
    return result;
}

void free_search_result(SearchResult* result) {
    if (!result) return;
    if (result->recovered_k) BN_free(result->recovered_k);
    free(result);
}

void init_default_search_parameters(SearchParameters* params) {
    if (!params) return;
    
    params->strategy = STRATEGY_HYBRID;
    params->initial_depth = 1;
    params->max_depth = 5;
    params->max_iterations = 10000;
    params->convergence_threshold = 0.95;
    params->candidates_per_iteration = 100;
    params->oscillation_weight = 0.6;
    params->attractor_weight = 0.4;
    params->enable_backtracking = true;
    params->enable_adaptive_depth = true;
}

void print_search_result(const SearchResult* result) {
    if (!result) return;
    
    printf("Search Result:\n");
    printf("  Success: %s\n", result->success ? "YES" : "NO");
    if (result->recovered_k) {
        printf("  Recovered k: %s\n", BN_bn2hex(result->recovered_k));
    }
    printf("  Final depth: %u\n", result->final_depth);
    printf("  Total iterations: %lu\n", result->total_iterations);
    printf("  Final score: %.6f\n", result->final_score);
    printf("  Elapsed time: %.3f seconds\n", result->elapsed_time);
    printf("  Backtracks: %u\n", result->num_backtracks);
    printf("  Depth adjustments: %u\n", result->num_depth_adjustments);
}

void print_search_parameters(const SearchParameters* params) {
    if (!params) return;
    
    printf("Search Parameters:\n");
    printf("  Strategy: ");
    switch (params->strategy) {
        case STRATEGY_RANDOM: printf("RANDOM\n"); break;
        case STRATEGY_ATTRACTOR_GUIDED: printf("ATTRACTOR_GUIDED\n"); break;
        case STRATEGY_OSCILLATION_GUIDED: printf("OSCILLATION_GUIDED\n"); break;
        case STRATEGY_HYBRID: printf("HYBRID\n"); break;
    }
    printf("  Initial depth: %u\n", params->initial_depth);
    printf("  Max depth: %u\n", params->max_depth);
    printf("  Max iterations: %lu\n", params->max_iterations);
    printf("  Convergence threshold: %.6f\n", params->convergence_threshold);
    printf("  Candidates per iteration: %u\n", params->candidates_per_iteration);
    printf("  Oscillation weight: %.2f\n", params->oscillation_weight);
    printf("  Attractor weight: %.2f\n", params->attractor_weight);
    printf("  Backtracking: %s\n", params->enable_backtracking ? "ENABLED" : "DISABLED");
    printf("  Adaptive depth: %s\n", params->enable_adaptive_depth ? "ENABLED" : "DISABLED");
}