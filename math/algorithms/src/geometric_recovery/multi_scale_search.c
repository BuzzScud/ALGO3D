/**
 * @file multi_scale_search.c
 * @brief Multi-Scale Fractal Search for Geometric Recovery
 * 
 * UNIVERSAL GEOMETRIC MATHEMATICS - Works with ANY system!
 */

#include "geometric_recovery/multi_scale_search.h"
#include "geometric_recovery/fractal_bounds.h"
#include "geometric_recovery/tetration_attractors.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Multi-scale search context
 */
struct MultiScaleSearchContext {
    uint64_t n;                         // System size
    uint32_t max_scales;                // Maximum scales
    TetrationSystem* attractors;        // Tetration attractors
    ScaleLevel* levels;                 // Scale levels
    uint32_t num_levels_used;           // Number of levels actually used
    uint64_t best_candidate;            // Best candidate overall
    double best_score;                  // Best score overall
};

/**
 * Create multi-scale search context
 */
MultiScaleSearchContext* multi_scale_search_create(
    uint64_t n,
    uint32_t max_scales,
    TetrationSystem* attractors
) {
    if (n == 0 || max_scales == 0) {
        return NULL;
    }
    
    MultiScaleSearchContext* ctx = (MultiScaleSearchContext*)calloc(
        1, sizeof(MultiScaleSearchContext)
    );
    if (!ctx) {
        return NULL;
    }
    
    ctx->n = n;
    ctx->max_scales = max_scales;
    ctx->attractors = attractors;
    ctx->num_levels_used = 0;
    ctx->best_candidate = 0;
    ctx->best_score = 0.0;
    
    // Allocate scale levels
    ctx->levels = (ScaleLevel*)calloc(max_scales, sizeof(ScaleLevel));
    if (!ctx->levels) {
        free(ctx);
        return NULL;
    }
    
    return ctx;
}

/**
 * Score candidate by distance to target
 */
static double score_multi_scale_candidate(
    uint64_t candidate,
    uint64_t target,
    uint64_t n
) {
    if (n == 0) return 0.0;
    
    uint64_t diff = (candidate > target) ? (candidate - target) : (target - candidate);
    double normalized_diff = (double)diff / (double)n;
    
    // Exponential decay - higher score for closer candidates
    return math_exp(-10.0 * normalized_diff);
}

/**
 * Perform multi-scale search
 */
bool multi_scale_search(
    MultiScaleSearchContext* ctx,
    const double start_position[MULTI_SCALE_MAX_DIMENSIONS],
    uint64_t target,
    const FractalPartition* bounds,
    uint64_t* result_out
) {
    if (!ctx || !start_position || !result_out) {
        return false;
    }
    
    ctx->best_score = 0.0;
    ctx->best_candidate = 0;
    
    // Search at multiple scales
    for (uint32_t scale = 0; scale < ctx->max_scales; scale++) {
        ScaleLevel* level = &ctx->levels[scale];
        level->scale_index = scale;
        level->scale_factor = math_pow(0.5, (double)scale);  // 1.0, 0.5, 0.25, ...
        
        // Number of candidates decreases with scale
        level->num_candidates = 100 / (scale + 1);
        level->candidates = (uint64_t*)calloc(level->num_candidates, sizeof(uint64_t));
        level->scores = (double*)calloc(level->num_candidates, sizeof(double));
        
        if (!level->candidates || !level->scores) {
            return false;
        }
        
        level->best_score = 0.0;
        level->best_candidate = 0;
        
        // Generate candidates at this scale
        for (uint32_t step = 0; step < level->num_candidates; step++) {
            double t = (double)step / (double)level->num_candidates;
            
            // Generate position at this scale
            double position[MULTI_SCALE_MAX_DIMENSIONS];
            for (uint32_t d = 0; d < MULTI_SCALE_MAX_DIMENSIONS; d++) {
                position[d] = start_position[d] + level->scale_factor * (t - 0.5);
            }
            
            // Apply fractal bounds if provided
            if (bounds) {
                apply_fractal_bounds(bounds, position, MULTI_SCALE_MAX_DIMENSIONS);
            }
            
            // Convert position to candidate value
            // Use simple mapping: sum of position components scaled to [0, n)
            double sum = 0.0;
            for (uint32_t d = 0; d < MULTI_SCALE_MAX_DIMENSIONS; d++) {
                sum += position[d];
            }
            
            // Normalize to [0, 1) and scale to [0, n)
            double normalized = math_fmod(sum / MULTI_SCALE_MAX_DIMENSIONS + 0.5, 1.0);
            if (normalized < 0.0) normalized += 1.0;
            
            uint64_t candidate = (uint64_t)(normalized * ctx->n);
            if (candidate >= ctx->n) candidate = ctx->n - 1;
            
            // Score candidate
            double score = score_multi_scale_candidate(candidate, target, ctx->n);
            
            level->candidates[step] = candidate;
            level->scores[step] = score;
            
            // Update best for this level
            if (score > level->best_score) {
                level->best_score = score;
                level->best_candidate = candidate;
            }
            
            // Update overall best
            if (score > ctx->best_score) {
                ctx->best_score = score;
                ctx->best_candidate = candidate;
            }
        }
        
        ctx->num_levels_used++;
        
        // Early termination if score is very high
        if (ctx->best_score > 0.99) {
            break;
        }
    }
    
    *result_out = ctx->best_candidate;
    return true;
}

/**
 * Get scale level results
 */
const ScaleLevel* multi_scale_get_level(
    const MultiScaleSearchContext* ctx,
    uint32_t scale_index
) {
    if (!ctx || scale_index >= ctx->num_levels_used) {
        return NULL;
    }
    
    return &ctx->levels[scale_index];
}

/**
 * Get number of scales used
 */
uint32_t multi_scale_get_num_scales(const MultiScaleSearchContext* ctx) {
    return ctx ? ctx->num_levels_used : 0;
}

/**
 * Get best candidate
 */
bool multi_scale_get_best(
    const MultiScaleSearchContext* ctx,
    uint64_t* candidate_out,
    double* score_out
) {
    if (!ctx || !candidate_out) {
        return false;
    }
    
    *candidate_out = ctx->best_candidate;
    if (score_out) {
        *score_out = ctx->best_score;
    }
    
    return true;
}

/**
 * Print multi-scale results
 */
void print_multi_scale_results(const MultiScaleSearchContext* ctx) {
    if (!ctx) return;
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Multi-Scale Search Results                                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("  Scales Used: %u / %u\n", ctx->num_levels_used, ctx->max_scales);
    printf("  Best Candidate: %lu\n", ctx->best_candidate);
    printf("  Best Score: %.6f\n\n", ctx->best_score);
    
    printf("  Results by Scale:\n");
    for (uint32_t i = 0; i < ctx->num_levels_used; i++) {
        const ScaleLevel* level = &ctx->levels[i];
        printf("    Scale %u (factor %.3f): %u candidates, best score %.6f\n",
               level->scale_index, level->scale_factor, 
               level->num_candidates, level->best_score);
    }
    
    printf("\n");
}

/**
 * Destroy multi-scale search context
 */
void multi_scale_search_destroy(MultiScaleSearchContext* ctx) {
    if (ctx) {
        if (ctx->levels) {
            for (uint32_t i = 0; i < ctx->num_levels_used; i++) {
                if (ctx->levels[i].candidates) {
                    free(ctx->levels[i].candidates);
                }
                if (ctx->levels[i].scores) {
                    free(ctx->levels[i].scores);
                }
            }
            free(ctx->levels);
        }
        free(ctx);
    }
}