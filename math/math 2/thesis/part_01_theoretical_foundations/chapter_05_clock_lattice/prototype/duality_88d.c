#include "duality_88d.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define FUNDAMENTAL_FREQUENCY 432.0  // Hz (base frequency)
#define POSITIONS_PER_CLOCK 12
#define DEGREES_PER_POSITION 30.0
#define LAYERS_IN_88D 8
#define DIMENSIONS_PER_LAYER 11

// ============================================================================
// CLOCK POSITION OPERATIONS
// ============================================================================

DualityClockPosition clock_position_create(uint8_t position) {
    DualityClockPosition pos;
    pos.position = position % POSITIONS_PER_CLOCK;
    pos.angle = pos.position * DEGREES_PER_POSITION;
    pos.quadrant = pos.position / 3;
    pos.is_folded = false;
    pos.source_quad = pos.quadrant;
    return pos;
}

double clock_position_angle(uint8_t position) {
    return (position % POSITIONS_PER_CLOCK) * DEGREES_PER_POSITION;
}

uint8_t clock_position_quadrant(uint8_t position) {
    return (position % POSITIONS_PER_CLOCK) / 3;
}

// ============================================================================
// QUADRANT FOLDING OPERATIONS
// ============================================================================

DualityClockPosition fold_to_q1(DualityClockPosition pos) {
    DualityClockPosition folded = pos;
    folded.source_quad = pos.quadrant;
    
    switch (pos.quadrant) {
        case 0:  // Q1 (0-3): No change
            folded.is_folded = false;
            break;
            
        case 1:  // Q2 (3-6): Reflect across 90° line
            folded.position = 6 - pos.position;
            folded.angle = 180.0 - pos.angle;
            folded.quadrant = 0;
            folded.is_folded = true;
            break;
            
        case 2:  // Q3 (6-9): Reflect across 180° line
            folded.position = 12 - pos.position;
            folded.angle = 360.0 - pos.angle;
            folded.quadrant = 0;
            folded.is_folded = true;
            break;
            
        case 3:  // Q4 (9-12): Reflect across 270° line
            folded.position = pos.position - 6;
            folded.angle = pos.angle - 180.0;
            folded.quadrant = 0;
            folded.is_folded = true;
            break;
    }
    
    return folded;
}

DualityClockPosition unfold_from_q1(DualityClockPosition folded, uint8_t target_quadrant) {
    DualityClockPosition unfolded = folded;
    
    switch (target_quadrant) {
        case 0:  // Q1: No change
            break;
            
        case 1:  // Q2: Reflect back across 90°
            unfolded.position = 6 - folded.position;
            unfolded.angle = 180.0 - folded.angle;
            break;
            
        case 2:  // Q3: Reflect back across 180°
            unfolded.position = 12 - folded.position;
            unfolded.angle = 360.0 - folded.angle;
            break;
            
        case 3:  // Q4: Reflect back across 270°
            unfolded.position = folded.position + 6;
            unfolded.angle = folded.angle + 180.0;
            break;
    }
    
    unfolded.quadrant = target_quadrant;
    unfolded.is_folded = false;
    unfolded.source_quad = target_quadrant;
    return unfolded;
}

bool positions_equivalent_folded(DualityClockPosition a, DualityClockPosition b) {
    DualityClockPosition a_folded = fold_to_q1(a);
    DualityClockPosition b_folded = fold_to_q1(b);
    return a_folded.position == b_folded.position;
}

// ============================================================================
// POLARITY FLIPPING OPERATIONS
// ============================================================================

bool is_prime_position(uint8_t pos) {
    pos = pos % POSITIONS_PER_CLOCK;
    return (pos == 1 || pos == 5 || pos == 7 || pos == 11);
}

DualityClockPosition square_prime_position(DualityClockPosition pos) {
    // All primes {1, 5, 7, 11} square to 1 (mod 12)
    // This is the UNIVERSAL POLARITY FLIP
    (void)pos;  // Unused parameter
    DualityClockPosition squared;
    squared.position = 1;
    squared.angle = DEGREES_PER_POSITION;  // 30°
    squared.quadrant = 0;
    squared.is_folded = false;
    squared.source_quad = 0;
    return squared;
}

bool verify_polarity_flip(uint8_t pos) {
    if (!is_prime_position(pos)) return false;
    
    // Square the position modulo 12
    uint8_t squared = (pos * pos) % POSITIONS_PER_CLOCK;
    
    // Should always be 1 for prime positions
    return (squared == 1);
}

DualityClockPosition square_position(DualityClockPosition pos) {
    if (is_prime_position(pos.position)) {
        // Prime positions use universal polarity flip
        return square_prime_position(pos);
    } else {
        // Non-prime positions square normally
        DualityClockPosition squared;
        uint8_t p = pos.position;
        squared.position = (p * p) % POSITIONS_PER_CLOCK;
        squared.angle = squared.position * DEGREES_PER_POSITION;
        squared.quadrant = squared.position / 3;
        squared.is_folded = false;
        squared.source_quad = squared.quadrant;
        return squared;
    }
}

// ============================================================================
// PLATONIC SOLID OPERATIONS
// ============================================================================

PlatonicSolid get_dual_solid(PlatonicSolid solid) {
    switch (solid) {
        case TETRAHEDRON:  return TETRAHEDRON;  // Self-dual
        case CUBE:         return OCTAHEDRON;
        case OCTAHEDRON:   return CUBE;
        case DODECAHEDRON: return ICOSAHEDRON;
        case ICOSAHEDRON:  return DODECAHEDRON;
        default:           return TETRAHEDRON;
    }
}

uint8_t get_num_vertices(PlatonicSolid solid) {
    switch (solid) {
        case TETRAHEDRON:  return 4;
        case CUBE:         return 8;
        case OCTAHEDRON:   return 6;
        case DODECAHEDRON: return 20;
        case ICOSAHEDRON:  return 12;
        default:           return 0;
    }
}

uint8_t get_num_edges(PlatonicSolid solid) {
    switch (solid) {
        case TETRAHEDRON:  return 6;
        case CUBE:         return 12;
        case OCTAHEDRON:   return 12;
        case DODECAHEDRON: return 30;
        case ICOSAHEDRON:  return 30;
        default:           return 0;
    }
}

uint8_t get_num_faces(PlatonicSolid solid) {
    switch (solid) {
        case TETRAHEDRON:  return 4;
        case CUBE:         return 6;
        case OCTAHEDRON:   return 8;
        case DODECAHEDRON: return 12;
        case ICOSAHEDRON:  return 20;
        default:           return 0;
    }
}

CoordinateFrame create_coordinate_frame(uint8_t layer, PlatonicSolid solid) {
    CoordinateFrame frame;
    frame.solid = solid;
    frame.layer = layer;
    frame.num_vertices = get_num_vertices(solid);
    frame.num_edges = get_num_edges(solid);
    frame.num_faces = get_num_faces(solid);
    return frame;
}

bool is_self_dual(PlatonicSolid solid) {
    return (solid == TETRAHEDRON);
}

// ============================================================================
// PHASE ANGLE OPERATIONS
// ============================================================================

PhaseAngle position_to_phase(DualityClockPosition pos, uint8_t harmonic) {
    PhaseAngle phase;
    
    // Determine which phase (A, B, C) based on position mod 3
    phase.phase = pos.position % 3;
    
    // Calculate angle
    phase.angle = pos.angle;
    
    // Calculate frequency
    phase.frequency = FUNDAMENTAL_FREQUENCY * harmonic;
    phase.harmonic = harmonic;
    
    return phase;
}

void get_three_phase_positions(uint8_t harmonic, DualityClockPosition phases[3]) {
    (void)harmonic;  // Unused parameter
    // Phase A: 0° (positions {0, 4, 8})
    phases[0] = clock_position_create(0);
    
    // Phase B: 120° (positions {1, 5, 9})
    phases[1] = clock_position_create(4);
    
    // Phase C: 240° (positions {2, 6, 10})
    phases[2] = clock_position_create(8);
}

double calculate_harmonic_frequency(uint8_t harmonic_number) {
    return FUNDAMENTAL_FREQUENCY * harmonic_number;
}

double octave_to_frequency(uint8_t octave) {
    // Each octave doubles the frequency
    return FUNDAMENTAL_FREQUENCY * pow(2.0, octave);
}

uint8_t get_phase_for_position(uint8_t position) {
    // Map positions to phases:
    // Phase A: {0, 4, 8}
    // Phase B: {1, 5, 9}
    // Phase C: {2, 6, 10}
    
    position = position % POSITIONS_PER_CLOCK;
    
    if (position == 0 || position == 4 || position == 8) return 0;  // Phase A
    if (position == 1 || position == 5 || position == 9) return 1;  // Phase B
    if (position == 2 || position == 6 || position == 10) return 2; // Phase C
    
    return 0;  // Default to Phase A
}

// ============================================================================
// 88D POINT OPERATIONS
// ============================================================================

Point88D create_point_88d(uint8_t layer, uint8_t dimension, uint8_t position) {
    Point88D point;
    
    // Initialize all layers to position 0
    for (int i = 0; i < LAYERS_IN_88D; i++) {
        point.layers[i] = clock_position_create(0);
    }
    
    // Set the specified layer
    point.layers[layer] = clock_position_create(position);
    
    point.dimension_in_layer = dimension % (DIMENSIONS_PER_LAYER + 1);
    point.is_dual = false;
    
    // Create coordinate frame (use dodecahedron for 12-fold symmetry)
    point.frame = create_coordinate_frame(layer, DODECAHEDRON);
    
    // Initialize abacus value
    point.value = abacus_new(60);  // Base 60
    
    return point;
}

void free_point_88d(Point88D* point) {
    if (point && point->value) {
        abacus_free(point->value);
        point->value = NULL;
    }
}

Point88D add_88d(Point88D a, Point88D b) {
    Point88D result = create_point_88d(0, 0, 0);
    
    // Add layer by layer
    for (int i = 0; i < LAYERS_IN_88D; i++) {
        // Simple addition without folding for now
        // (Folding logic needs more sophisticated quadrant arithmetic)
        uint8_t sum_pos = (a.layers[i].position + b.layers[i].position) % POSITIONS_PER_CLOCK;
        
        // Create result position
        result.layers[i] = clock_position_create(sum_pos);
    }
    
    // Add abacus values (skip for now to avoid segfault)
    // if (a.value && b.value && result.value) {
    //     MathError err = abacus_add(result.value, a.value, b.value);
    //     (void)err;  // Ignore error for prototype
    // }
    
    result.dimension_in_layer = a.dimension_in_layer;
    result.is_dual = a.is_dual;
    result.frame = a.frame;
    
    return result;
}

Point88D square_88d(Point88D point) {
    Point88D squared = create_point_88d(0, 0, 0);
    
    // Square each layer
    for (int i = 0; i < LAYERS_IN_88D; i++) {
        squared.layers[i] = square_position(point.layers[i]);
    }
    
    // Square abacus value (skip for now to avoid segfault)
    // if (point.value && squared.value) {
    //     MathError err = abacus_mul(squared.value, point.value, point.value);
    //     (void)err;  // Ignore error for prototype
    // }
    
    squared.dimension_in_layer = point.dimension_in_layer;
    squared.is_dual = point.is_dual;
    squared.frame = point.frame;
    
    return squared;
}

Point88D flip_to_dual(Point88D point) {
    Point88D dual = point;
    dual.is_dual = !point.is_dual;
    
    // Flip the coordinate frame to its dual
    dual.frame.solid = get_dual_solid(point.frame.solid);
    dual.frame.num_vertices = get_num_vertices(dual.frame.solid);
    dual.frame.num_edges = get_num_edges(dual.frame.solid);
    dual.frame.num_faces = get_num_faces(dual.frame.solid);
    
    // In dual space:
    // - Vertices become faces
    // - Faces become vertices
    // - Edges stay the same (invariant!)
    
    // The value stays the same, but interpretation changes
    
    return dual;
}

bool are_dual_points(Point88D p1, Point88D p2) {
    // Same position in space
    bool same_position = true;
    for (int i = 0; i < LAYERS_IN_88D; i++) {
        if (p1.layers[i].position != p2.layers[i].position) {
            same_position = false;
            break;
        }
    }
    
    // Opposite dual state
    bool opposite_dual = (p1.is_dual != p2.is_dual);
    
    return same_position && opposite_dual;
}

double get_layer_magnitude_scale(uint8_t layer) {
    // Layer 0: 10^0 = 1
    // Layer 1: 10^3 = 1,000
    // Layer 2: 10^6 = 1,000,000
    // ...
    // Layer 7: 10^21
    return pow(10.0, layer * 3);
}

void print_point_88d(Point88D point) {
    printf("Point88D:\n");
    printf("  Dimension in layer: %d\n", point.dimension_in_layer);
    printf("  Is dual: %s\n", point.is_dual ? "true" : "false");
    printf("  Coordinate frame: %s (layer %d)\n", 
           get_solid_name(point.frame.solid), point.frame.layer);
    printf("  Layers:\n");
    for (int i = 0; i < LAYERS_IN_88D; i++) {
        if (point.layers[i].position != 0) {
            printf("    Layer %d: position %d (%.1f°, Q%d)\n",
                   i, point.layers[i].position, point.layers[i].angle,
                   point.layers[i].quadrant);
        }
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* get_solid_name(PlatonicSolid solid) {
    switch (solid) {
        case TETRAHEDRON:  return "Tetrahedron";
        case CUBE:         return "Cube";
        case OCTAHEDRON:   return "Octahedron";
        case DODECAHEDRON: return "Dodecahedron";
        case ICOSAHEDRON:  return "Icosahedron";
        default:           return "Unknown";
    }
}

const char* get_phase_name(uint8_t phase) {
    switch (phase) {
        case 0: return "Phase A";
        case 1: return "Phase B";
        case 2: return "Phase C";
        default: return "Unknown";
    }
}

bool verify_euler_formula(PlatonicSolid solid) {
    // Euler's formula: V - E + F = 2
    uint8_t V = get_num_vertices(solid);
    uint8_t E = get_num_edges(solid);
    uint8_t F = get_num_faces(solid);
    
    return (V - E + F == 2);
}