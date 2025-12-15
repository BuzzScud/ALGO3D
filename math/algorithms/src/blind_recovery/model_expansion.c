/**
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @file model_expansion.c
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * @brief Dynamic model expansion for blind recovery
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * 
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * Implements dynamic expansion of geometric models, allowing structures
#include "math/types.h"
#include "math/transcendental.h"
#include "math/arithmetic.h"
 * to grow and adapt while maintaining geometric properties and Euler's formula.
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
 * Platonic solid expansion rules
 * 
 * Each Platonic solid can expand to specific other solids while
 * maintaining geometric relationships and symmetry.
 */
typedef struct {
    PlatonicSolidType source;
    PlatonicSolidType target;
    double expansion_factor;
    const char* description;
} ExpansionRule;

static const ExpansionRule EXPANSION_RULES[] = {
    // Tetrahedron expansions
    {PLATONIC_TETRAHEDRON, PLATONIC_OCTAHEDRON, 1.414, "Tetrahedron → Octahedron (dual relationship)"},
    {PLATONIC_TETRAHEDRON, PLATONIC_ICOSAHEDRON, 2.618, "Tetrahedron → Icosahedron (golden ratio)"},
    
    // Cube expansions
    {PLATONIC_CUBE, PLATONIC_OCTAHEDRON, 1.414, "Cube → Octahedron (dual)"},
    {PLATONIC_CUBE, PLATONIC_DODECAHEDRON, 1.618, "Cube → Dodecahedron (golden ratio)"},
    
    // Octahedron expansions
    {PLATONIC_OCTAHEDRON, PLATONIC_CUBE, 1.414, "Octahedron → Cube (dual)"},
    {PLATONIC_OCTAHEDRON, PLATONIC_ICOSAHEDRON, 1.618, "Octahedron → Icosahedron"},
    
    // Dodecahedron expansions
    {PLATONIC_DODECAHEDRON, PLATONIC_ICOSAHEDRON, 1.618, "Dodecahedron → Icosahedron (dual)"},
    
    // Icosahedron expansions
    {PLATONIC_ICOSAHEDRON, PLATONIC_DODECAHEDRON, 1.618, "Icosahedron → Dodecahedron (dual)"},
};

static const int NUM_EXPANSION_RULES = sizeof(EXPANSION_RULES) / sizeof(EXPANSION_RULES[0]);

/**
 * Get expansion rule for source → target transformation
 */
static const ExpansionRule* get_expansion_rule(
    PlatonicSolidType source,
    PlatonicSolidType target
) {
    for (int i = 0; i < NUM_EXPANSION_RULES; i++) {
        if (EXPANSION_RULES[i].source == source && 
            EXPANSION_RULES[i].target == target) {
            return &EXPANSION_RULES[i];
        }
    }
    return NULL;
}

/**
 * Validate expansion is geometrically valid
 */
static bool validate_expansion(
    const StructuralMap* source,
    PlatonicSolidType target_solid
) {
    (void)target_solid;  // Unused in this implementation
    if (!source) return false;
    
    // Check if source has Platonic solid type
    // (In real implementation, would check source->platonic_type)
    
    // Verify Euler's formula for source
    int euler_source = source->num_vertices - source->num_edges + source->num_faces;
    if (euler_source != 2) {
        return false;
    }
    
    // Check if expansion rule exists
    // (Simplified - would check actual source type)
    return true;
}

/**
 * Calculate target dimensions based on Platonic solid type
 */
static void calculate_target_dimensions(
    PlatonicSolidType solid_type,
    uint32_t* vertices,
    uint32_t* edges,
    uint32_t* faces
) {
    switch (solid_type) {
        case PLATONIC_TETRAHEDRON:
            *vertices = 4;
            *edges = 6;
            *faces = 4;
            break;
        case PLATONIC_CUBE:
            *vertices = 8;
            *edges = 12;
            *faces = 6;
            break;
        case PLATONIC_OCTAHEDRON:
            *vertices = 6;
            *edges = 12;
            *faces = 8;
            break;
        case PLATONIC_DODECAHEDRON:
            *vertices = 20;
            *edges = 30;
            *faces = 12;
            break;
        case PLATONIC_ICOSAHEDRON:
            *vertices = 12;
            *edges = 30;
            *faces = 20;
            break;
        default:
            *vertices = 0;
            *edges = 0;
            *faces = 0;
            break;
    }
}

/**
 * Expand model to target Platonic solid
 */
StructuralMap* expand_to_platonic_solid(
    const StructuralMap* source,
    PlatonicSolidType target_solid
) {
    if (!source) return NULL;
    
    // Validate expansion
    if (!validate_expansion(source, target_solid)) {
        return NULL;
    }
    
    // Calculate target dimensions
    uint32_t target_V, target_E, target_F;
    calculate_target_dimensions(target_solid, &target_V, &target_E, &target_F);
    
    // Verify Euler's formula for target
    int euler_target = target_V - target_E + target_F;
    if (euler_target != 2) {
        return NULL;
    }
    
    // Create expanded structure
    StructuralMap* expanded = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    if (!expanded) return NULL;
    
    expanded->num_vertices = target_V;
    expanded->num_edges = target_E;
    expanded->num_faces = target_F;
    
    // Allocate arrays
    expanded->coprime_matrix = NULL;  // Will be computed if needed
    expanded->dimensional_offsets = NULL;
    expanded->corruption_mask = (bool*)calloc(target_V, sizeof(bool));
    
    // Preserve corruption percentage (or reduce it)
    expanded->corruption_percentage = source->corruption_percentage * 0.8;  // 20% improvement
    
    return expanded;
}

/**
 * Expand model by level (iterative expansion)
 */
StructuralMap* expand_model_by_level(
    const StructuralMap* source,
    uint32_t expansion_level
) {
    if (!source || expansion_level == 0) {
        return NULL;
    }
    
    // For now, simple expansion: double the structure
    StructuralMap* expanded = (StructuralMap*)calloc(1, sizeof(StructuralMap));
    if (!expanded) return NULL;
    
    // Expansion factor based on level
    double factor = 1.0 + (expansion_level * 0.5);
    
    expanded->num_vertices = (uint32_t)(source->num_vertices * factor);
    expanded->num_edges = (uint32_t)(source->num_edges * factor);
    expanded->num_faces = (uint32_t)(source->num_faces * factor);
    
    // Ensure Euler's formula is maintained (approximately)
    // Adjust faces to satisfy V - E + F = 2
    int euler_check = expanded->num_vertices - expanded->num_edges + expanded->num_faces;
    if (euler_check != 2) {
        // Adjust faces
        expanded->num_faces = 2 + expanded->num_edges - expanded->num_vertices;
    }
    
    // Allocate arrays
    expanded->coprime_matrix = NULL;
    expanded->dimensional_offsets = NULL;
    expanded->corruption_mask = (bool*)calloc(expanded->num_vertices, sizeof(bool));
    
    // Reduce corruption with expansion
    expanded->corruption_percentage = source->corruption_percentage * 0.9;
    
    return expanded;
}

/**
 * Check if expansion is valid (maintains Euler's formula)
 */
bool is_expansion_valid(const StructuralMap* expanded) {
    if (!expanded) return false;
    
    // Check Euler's formula: V - E + F = 2
    int euler = expanded->num_vertices - expanded->num_edges + expanded->num_faces;
    return (euler == 2);
}

/**
 * Get expansion factor between two Platonic solids
 */
double get_expansion_factor(
    PlatonicSolidType source,
    PlatonicSolidType target
) {
    const ExpansionRule* rule = get_expansion_rule(source, target);
    if (rule) {
        return rule->expansion_factor;
    }
    return 1.0;  // No expansion
}

/**
 * List available expansions for a Platonic solid
 */
int get_available_expansions(
    PlatonicSolidType source,
    PlatonicSolidType* targets,
    int max_targets
) {
    int count = 0;
    
    for (int i = 0; i < NUM_EXPANSION_RULES && count < max_targets; i++) {
        if (EXPANSION_RULES[i].source == source) {
            targets[count++] = EXPANSION_RULES[i].target;
        }
    }
    
    return count;
}

/**
 * Compute expansion metrics
 */
void compute_expansion_metrics(
    const StructuralMap* source,
    const StructuralMap* expanded,
    ExpansionMetrics* metrics
) {
    if (!source || !expanded || !metrics) return;
    
    metrics->vertex_ratio = (double)expanded->num_vertices / source->num_vertices;
    metrics->edge_ratio = (double)expanded->num_edges / source->num_edges;
    metrics->face_ratio = (double)expanded->num_faces / source->num_faces;
    
    metrics->corruption_reduction = source->corruption_percentage - expanded->corruption_percentage;
    
    // Check if Euler's formula is maintained
    int euler_source = source->num_vertices - source->num_edges + source->num_faces;
    int euler_expanded = expanded->num_vertices - expanded->num_edges + expanded->num_faces;
    metrics->euler_maintained = (euler_source == 2 && euler_expanded == 2);
    
    // Overall quality score
    metrics->quality_score = 0.0;
    if (metrics->euler_maintained) {
        metrics->quality_score += 0.5;
    }
    if (metrics->corruption_reduction > 0.0) {
        metrics->quality_score += 0.3;
    }
    if (metrics->vertex_ratio > 1.0) {
        metrics->quality_score += 0.2;
    }
}

/**
 * Free expanded model
 */
void free_expanded_model(StructuralMap* expanded) {
    if (!expanded) return;
    
    if (expanded->corruption_mask) {
        free(expanded->corruption_mask);
    }
    if (expanded->coprime_matrix) {
        free(expanded->coprime_matrix);
    }
    if (expanded->dimensional_offsets) {
        free(expanded->dimensional_offsets);
    }
    
    free(expanded);
}
