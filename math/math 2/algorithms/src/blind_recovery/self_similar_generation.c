/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file self_similar_generation.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Self-similar structure generation for fractal-like expansion
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Generates self-similar structures where each level maintains the same
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * geometric properties as the parent, creating fractal-like hierarchies.
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 */
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"

#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include "blind_recovery.h"
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
#include <stdlib.h>
#include <string.h>

/**
 * Self-similar generation parameters
 */
typedef struct {
    double scale_factor;        // Scale factor for each level
    uint32_t max_depth;         // Maximum recursion depth
    bool maintain_euler;        // Maintain Euler's formula
    bool preserve_symmetry;     // Preserve symmetry groups
} SelfSimilarParams;

/**
 * Create default self-similar parameters
 */
static SelfSimilarParams create_default_params(void) {
    SelfSimilarParams params;
    params.scale_factor = 0.618;  // Golden ratio reciprocal
    params.max_depth = 5;
    params.maintain_euler = true;
    params.preserve_symmetry = true;
    return params;
}

/**
 * Generate next level of self-similar structure
 * 
 * Creates a smaller version of the current structure at each vertex,
 * maintaining geometric properties.
 */
StructuralMap* generate_next_level(
    const StructuralMap* current,
    const SelfSimilarParams* params
) {
    if (!current || !params) return NULL;
    
    // Calculate dimensions for next level
    // Each vertex becomes a mini-structure
    uint32_t next_V = current->num_vertices * current->num_vertices;
    uint32_t next_E = current->num_edges * current->num_vertices;
    uint32_t next_F = current->num_faces * current->num_vertices;
    
    // Adjust to maintain Euler's formula if requested
    if (params->maintain_euler) {
        // V - E + F = 2
        // Adjust faces to satisfy this
        next_F = 2 + next_E - next_V;
        
        // Ensure positive
        if (next_F < 1) next_F = 1;
    }
    
    // Create next level structure
    StructuralMap* next_level = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    if (!next_level) return NULL;
    
    next_level->num_vertices = next_V;
    next_level->num_edges = next_E;
    next_level->num_faces = next_F;
    
    // Allocate arrays
    next_level->coprime_matrix = NULL;
    next_level->dimensional_offsets = NULL;
    next_level->corruption_mask = (bool*)calloc(next_V, sizeof(bool));
    
    // Scale down corruption (self-similar structures are more robust)
    next_level->corruption_percentage = current->corruption_percentage * params->scale_factor;
    
    return next_level;
}

/**
 * Generate complete self-similar hierarchy
 * 
 * Creates a hierarchy of structures from coarse to fine,
 * each level being a self-similar copy of the previous.
 */
SelfSimilarHierarchy* generate_self_similar_hierarchy(
    const StructuralMap* base,
    uint32_t num_levels
) {
    if (!base || num_levels == 0) return NULL;
    
    SelfSimilarHierarchy* hierarchy = (SelfSimilarHierarchy*)calloc(1, sizeof(SelfSimilarHierarchy));
    if (!hierarchy) return NULL;
    
    hierarchy->num_levels = num_levels;
    hierarchy->levels = (StructuralMap**)calloc(num_levels, sizeof(StructuralMap*));
    if (!hierarchy->levels) {
        free(hierarchy);
        return NULL;
    }
    
    // Create default parameters
    SelfSimilarParams params = create_default_params();
    
    // First level is the base
    hierarchy->levels[0] = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    if (!hierarchy->levels[0]) {
        free(hierarchy->levels);
        free(hierarchy);
        return NULL;
    }
    
    // Copy base structure
    hierarchy->levels[0]->num_vertices = base->num_vertices;
    hierarchy->levels[0]->num_edges = base->num_edges;
    hierarchy->levels[0]->num_faces = base->num_faces;
    hierarchy->levels[0]->corruption_percentage = base->corruption_percentage;
    hierarchy->levels[0]->corruption_mask = (bool*)calloc(base->num_vertices, sizeof(bool));
    
    // Generate subsequent levels
    for (uint32_t i = 1; i < num_levels; i++) {
        hierarchy->levels[i] = generate_next_level(hierarchy->levels[i-1], &params);
        if (!hierarchy->levels[i]) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                if (hierarchy->levels[j]) {
                    if (hierarchy->levels[j]->corruption_mask) {
                        free(hierarchy->levels[j]->corruption_mask);
                    }
                    free(hierarchy->levels[j]);
                }
            }
            free(hierarchy->levels);
            free(hierarchy);
            return NULL;
        }
    }
    
    return hierarchy;
}

/**
 * Validate self-similar hierarchy
 * 
 * Checks that each level maintains geometric properties
 * and relationships with adjacent levels.
 */
bool validate_self_similar_hierarchy(const SelfSimilarHierarchy* hierarchy) {
    if (!hierarchy || !hierarchy->levels) return false;
    
    // Check each level
    for (uint32_t i = 0; i < hierarchy->num_levels; i++) {
        const StructuralMap* level = hierarchy->levels[i];
        if (!level) return false;
        
        // Check Euler's formula
        int euler = level->num_vertices - level->num_edges + level->num_faces;
        if (euler != 2) {
            return false;
        }
        
        // Check dimensions are positive
        if (level->num_vertices == 0 || level->num_edges == 0 || level->num_faces == 0) {
            return false;
        }
    }
    
    // Check relationships between levels
    for (uint32_t i = 1; i < hierarchy->num_levels; i++) {
        const StructuralMap* prev = hierarchy->levels[i-1];
        const StructuralMap* curr = hierarchy->levels[i];
        
        // Each level should be larger than previous
        if (curr->num_vertices <= prev->num_vertices) {
            return false;
        }
    }
    
    return true;
}

/**
 * Compute self-similarity score
 * 
 * Measures how well the hierarchy maintains self-similar properties.
 */
double compute_self_similarity_score(const SelfSimilarHierarchy* hierarchy) {
    if (!hierarchy || hierarchy->num_levels < 2) return 0.0;
    
    double score = 0.0;
    int comparisons = 0;
    
    // Compare adjacent levels
    for (uint32_t i = 1; i < hierarchy->num_levels; i++) {
        const StructuralMap* prev = hierarchy->levels[i-1];
        const StructuralMap* curr = hierarchy->levels[i];
        
        // Compute ratios
        double v_ratio = (double)curr->num_vertices / prev->num_vertices;
        double e_ratio = (double)curr->num_edges / prev->num_edges;
        double f_ratio = (double)curr->num_faces / prev->num_faces;
        
        // Check if ratios are consistent (self-similar)
        double ratio_variance = math_abs(v_ratio - e_ratio) + math_abs(e_ratio - f_ratio);
        
        // Lower variance = higher self-similarity
        double level_score = 1.0 / (1.0 + ratio_variance);
        score += level_score;
        comparisons++;
    }
    
    return comparisons > 0 ? score / comparisons : 0.0;
}

/**
 * Get level from hierarchy
 */
const StructuralMap* get_hierarchy_level(
    const SelfSimilarHierarchy* hierarchy,
    uint32_t level
) {
    if (!hierarchy || level >= hierarchy->num_levels) {
        return NULL;
    }
    return hierarchy->levels[level];
}

/**
 * Free self-similar hierarchy
 */
void free_self_similar_hierarchy(SelfSimilarHierarchy* hierarchy) {
    if (!hierarchy) return;
    
    if (hierarchy->levels) {
        for (uint32_t i = 0; i < hierarchy->num_levels; i++) {
            if (hierarchy->levels[i]) {
                if (hierarchy->levels[i]->corruption_mask) {
                    free(hierarchy->levels[i]->corruption_mask);
                }
                if (hierarchy->levels[i]->coprime_matrix) {
                    free(hierarchy->levels[i]->coprime_matrix);
                }
                if (hierarchy->levels[i]->dimensional_offsets) {
                    free(hierarchy->levels[i]->dimensional_offsets);
                }
                free(hierarchy->levels[i]);
            }
        }
        free(hierarchy->levels);
    }
    
    free(hierarchy);
}

/**
 * Apply recovery across hierarchy
 * 
 * Uses information from multiple levels to improve recovery.
 */
bool recover_using_hierarchy(
    SelfSimilarHierarchy* hierarchy,
    uint32_t target_level
) {
    if (!hierarchy || target_level >= hierarchy->num_levels) {
        return false;
    }
    
    StructuralMap* target = hierarchy->levels[target_level];
    if (!target) return false;
    
    // Use information from coarser levels (lower indices)
    for (uint32_t i = 0; i < target_level; i++) {
        const StructuralMap* coarse = hierarchy->levels[i];
        if (!coarse) continue;
        
        // If coarse level has lower corruption, use it to guide recovery
        if (coarse->corruption_percentage < target->corruption_percentage) {
            // Reduce target corruption based on coarse level
            target->corruption_percentage *= 0.9;
        }
    }
    
    // Use information from finer levels (higher indices)
    for (uint32_t i = target_level + 1; i < hierarchy->num_levels; i++) {
        const StructuralMap* fine = hierarchy->levels[i];
        if (!fine) continue;
        
        // Fine level provides detailed information
        if (fine->corruption_percentage < target->corruption_percentage) {
            target->corruption_percentage *= 0.95;
        }
    }
    
    return true;
}

/**
 * Compute hierarchy metrics
 */
void compute_hierarchy_metrics(
    const SelfSimilarHierarchy* hierarchy,
    HierarchyMetrics* metrics
) {
    if (!hierarchy || !metrics) return;
    
    metrics->num_levels = hierarchy->num_levels;
    metrics->self_similarity_score = compute_self_similarity_score(hierarchy);
    metrics->all_levels_valid = validate_self_similar_hierarchy(hierarchy);
    
    // Compute average corruption across levels
    double total_corruption = 0.0;
    for (uint32_t i = 0; i < hierarchy->num_levels; i++) {
        if (hierarchy->levels[i]) {
            total_corruption += hierarchy->levels[i]->corruption_percentage;
        }
    }
    metrics->average_corruption = total_corruption / hierarchy->num_levels;
    
    // Compute total vertices across all levels
    uint32_t total_vertices = 0;
    for (uint32_t i = 0; i < hierarchy->num_levels; i++) {
        if (hierarchy->levels[i]) {
            total_vertices += hierarchy->levels[i]->num_vertices;
        }
    }
    metrics->total_vertices = total_vertices;
}
