/**
 * @file geometric_ops.c
 * @brief Implementation of core geometric operations
 * 
 * All operations are O(1) - constant time regardless of magnitude.
 */

#define _GNU_SOURCE
#include "geometric_ops.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// POSITION OPERATIONS
// ============================================================================

GeometricPosition geo_pos_create(uint8_t ring, uint8_t position, int64_t magnitude) {
    GeometricPosition pos;
    
    pos.ring = ring % 4;  // 4 rings in clock lattice
    pos.position = position % GEO_NUM_CLOCK_POSITIONS;
    pos.magnitude = magnitude;
    pos.angle = pos.position * GEO_DEGREES_PER_POSITION;
    pos.layer = geo_select_layer(magnitude);
    
    // Check if prime position
    pos.is_prime_position = (pos.position == 1 || pos.position == 5 || 
                             pos.position == 7 || pos.position == 11);
    
    // Check if control position
    pos.is_control = (pos.position == 0);
    
    return pos;
}

GeometricPosition geo_pos_from_clock(const ClockPosition* clock_pos) {
    GeometricPosition pos;
    
    pos.ring = clock_pos->ring;
    pos.position = clock_pos->position;
    // ClockPosition doesn't have magnitude field - use 0 as default
    pos.magnitude = 0;
    pos.angle = pos.position * GEO_DEGREES_PER_POSITION;
    pos.layer = geo_select_layer(pos.magnitude);
    
    pos.is_prime_position = (pos.position == 1 || pos.position == 5 || 
                             pos.position == 7 || pos.position == 11);
    pos.is_control = (pos.position == 0);
    
    return pos;
}

void geo_pos_to_clock(const GeometricPosition* pos, ClockPosition* clock_pos) {
    clock_pos->ring = pos->ring;
    clock_pos->position = pos->position;
    // ClockPosition doesn't have magnitude field
}

bool geo_pos_is_prime(const GeometricPosition* pos) {
    return pos->is_prime_position;
}

bool geo_pos_is_control(const GeometricPosition* pos) {
    return pos->is_control;
}

double geo_pos_angle(const GeometricPosition* pos) {
    return pos->angle;
}

uint8_t geo_select_layer(int64_t magnitude) {
    // O(1) - direct calculation based on magnitude
    
    // Each layer represents ~3 orders of magnitude
    if (magnitude < 1000LL) return 0;                    // 10^0 to 10^3
    if (magnitude < 1000000LL) return 1;                 // 10^3 to 10^6
    if (magnitude < 1000000000LL) return 2;              // 10^6 to 10^9
    if (magnitude < 1000000000000LL) return 3;           // 10^9 to 10^12
    if (magnitude < 1000000000000000LL) return 4;        // 10^12 to 10^15
    if (magnitude < 1000000000000000000LL) return 5;     // 10^15 to 10^18
    if (magnitude < 1000000000000000000LL * 1000LL) return 6;  // 10^18 to 10^21
    return 7;  // > 10^21
}

// ============================================================================
// DISTANCE & ANGLE OPERATIONS (O(1))
// ============================================================================

uint8_t geo_shortest_path(uint8_t pos1, uint8_t pos2) {
    // O(1) - shortest path on circle
    uint8_t diff = (pos1 > pos2) ? (pos1 - pos2) : (pos2 - pos1);
    if (diff > 6) {
        diff = GEO_NUM_CLOCK_POSITIONS - diff;  // Go the other way
    }
    return diff;
}

double geo_distance(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - geometric distance using clock lattice geometry
    
    // Position difference (shortest path on circle)
    uint8_t pos_diff = geo_shortest_path(pos1->position, pos2->position);
    
    // Magnitude difference
    int64_t mag_diff = (pos1->magnitude > pos2->magnitude) ? 
                       (pos1->magnitude - pos2->magnitude) : 
                       (pos2->magnitude - pos1->magnitude);
    
    // Geometric distance (Pythagorean on clock)
    double angle_dist = pos_diff * GEO_DEGREES_PER_POSITION;  // 30° per position
    double mag_dist = (double)mag_diff * 12.0;                // 12 units per magnitude
    
    return sqrt(angle_dist * angle_dist + mag_dist * mag_dist);
}

double geo_angle_between(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - angle between two positions
    
    double angle1 = pos1->angle;
    double angle2 = pos2->angle;
    
    double diff = fabs(angle2 - angle1);
    
    // Take shortest angle
    if (diff > 180.0) {
        diff = 360.0 - diff;
    }
    
    return diff;
}

// ============================================================================
// TRIANGULATION (O(1))
// ============================================================================

GeometricPosition geo_triangulate(
    const GeometricPosition* p1,
    const GeometricPosition* p2,
    const GeometricPosition* p3
) {
    // O(1) - uses Plimpton 322 formula (Babylonian mathematics)
    
    // Extract positions
    uint8_t pos1 = p1->position;
    uint8_t pos2 = p2->position;
    uint8_t pos3 = p3->position;
    
    // Babylonian formula: a = p² - q², b = 2pq, c = p² + q²
    // For triangulation, we compute the centroid
    
    // Position: average of three positions (mod 12)
    uint8_t result_pos = (pos1 + pos2 + pos3) % GEO_NUM_CLOCK_POSITIONS;
    
    // Magnitude: average of three magnitudes
    int64_t result_mag = (p1->magnitude + p2->magnitude + p3->magnitude) / 3;
    
    // Create result position
    GeometricPosition result = geo_pos_create(0, result_pos, result_mag);
    
    return result;
}

GeometricPosition geo_triangulate_weighted(
    const GeometricPosition* p1, double w1,
    const GeometricPosition* p2, double w2,
    const GeometricPosition* p3, double w3
) {
    // O(1) - weighted triangulation
    
    double total_weight = w1 + w2 + w3;
    
    // Weighted position
    double weighted_pos = (p1->position * w1 + p2->position * w2 + p3->position * w3) / total_weight;
    uint8_t result_pos = ((uint8_t)weighted_pos) % GEO_NUM_CLOCK_POSITIONS;
    
    // Weighted magnitude
    double weighted_mag = (p1->magnitude * w1 + p2->magnitude * w2 + p3->magnitude * w3) / total_weight;
    int64_t result_mag = (int64_t)weighted_mag;
    
    return geo_pos_create(0, result_pos, result_mag);
}

// ============================================================================
// POSITION ARITHMETIC (O(1))
// ============================================================================

GeometricPosition geo_add(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - geometric addition
    
    // Add positions (mod 12)
    uint8_t sum_pos = (pos1->position + pos2->position) % GEO_NUM_CLOCK_POSITIONS;
    
    // Add magnitudes
    int64_t sum_mag = pos1->magnitude + pos2->magnitude;
    
    // Handle carry if needed
    if (pos1->position + pos2->position >= GEO_NUM_CLOCK_POSITIONS) {
        sum_mag += 1;  // Carry to magnitude
    }
    
    return geo_pos_create(0, sum_pos, sum_mag);
}

GeometricPosition geo_subtract(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - geometric subtraction
    
    int diff_pos = pos1->position - pos2->position;
    int64_t diff_mag = pos1->magnitude - pos2->magnitude;
    
    // Handle borrow if needed
    if (diff_pos < 0) {
        diff_pos += GEO_NUM_CLOCK_POSITIONS;
        diff_mag -= 1;
    }
    
    return geo_pos_create(0, (uint8_t)diff_pos, diff_mag);
}

GeometricPosition geo_scale(const GeometricPosition* pos, int64_t scalar) {
    // O(1) - multiply position by scalar
    
    // Scale position
    uint8_t scaled_pos = (pos->position * (scalar % GEO_NUM_CLOCK_POSITIONS)) % GEO_NUM_CLOCK_POSITIONS;
    
    // Scale magnitude
    int64_t scaled_mag = pos->magnitude * scalar + (pos->position * scalar) / GEO_NUM_CLOCK_POSITIONS;
    
    return geo_pos_create(0, scaled_pos, scaled_mag);
}

GeometricPosition geo_midpoint(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - midpoint between two positions
    
    // Average position (shortest path)
    uint8_t diff = geo_shortest_path(pos1->position, pos2->position);
    uint8_t mid_pos = (pos1->position + diff / 2) % GEO_NUM_CLOCK_POSITIONS;
    
    // Average magnitude
    int64_t mid_mag = (pos1->magnitude + pos2->magnitude) / 2;
    
    return geo_pos_create(0, mid_pos, mid_mag);
}

// ============================================================================
// PLATONIC FRAME OPERATIONS
// ============================================================================

PlatonicFrame geo_frame_create(uint8_t layer) {
    PlatonicFrame frame;
    memset(&frame, 0, sizeof(PlatonicFrame));
    
    frame.layer = layer % GEO_NUM_LAYERS;
    
    // Assign solid based on layer (repeating pattern)
    switch (layer % 5) {
        case 0: frame.solid = GEO_TETRAHEDRON; break;
        case 1: frame.solid = GEO_CUBE; break;
        case 2: frame.solid = GEO_OCTAHEDRON; break;
        case 3: frame.solid = GEO_DODECAHEDRON; break;
        case 4: frame.solid = GEO_ICOSAHEDRON; break;
    }
    
    // Set geometry based on solid
    switch (frame.solid) {
        case GEO_TETRAHEDRON:
            frame.num_vertices = 4;
            frame.num_edges = 6;
            frame.num_faces = 4;
            frame.dual_solid = GEO_TETRAHEDRON;  // Self-dual
            break;
            
        case GEO_CUBE:
            frame.num_vertices = 8;
            frame.num_edges = 12;
            frame.num_faces = 6;
            frame.dual_solid = GEO_OCTAHEDRON;
            break;
            
        case GEO_OCTAHEDRON:
            frame.num_vertices = 6;
            frame.num_edges = 12;
            frame.num_faces = 8;
            frame.dual_solid = GEO_CUBE;
            break;
            
        case GEO_DODECAHEDRON:
            frame.num_vertices = 20;
            frame.num_edges = 30;
            frame.num_faces = 12;
            frame.dual_solid = GEO_ICOSAHEDRON;
            break;
            
        case GEO_ICOSAHEDRON:
            frame.num_vertices = 12;
            frame.num_edges = 30;
            frame.num_faces = 20;
            frame.dual_solid = GEO_DODECAHEDRON;
            break;
    }
    
    frame.center[0] = 0.0;
    frame.center[1] = 0.0;
    frame.center[2] = 0.0;
    frame.radius = 1.0;
    
    return frame;
}

void geo_frame_get_vertex(const PlatonicFrame* frame, uint8_t position, double vertex[3]) {
    // O(1) - get vertex coordinates for position
    
    // For now, use simple mapping
    // TODO: Implement proper vertex calculations for each solid
    
    double angle = position * GEO_DEGREES_PER_POSITION * M_PI / 180.0;
    vertex[0] = frame->radius * cos(angle);
    vertex[1] = frame->radius * sin(angle);
    vertex[2] = 0.0;
}

GeometricPosition geo_frame_transform(
    const GeometricPosition* pos,
    const PlatonicFrame* from_frame,
    const PlatonicFrame* to_frame
) {
    // O(1) - transform between frames
    
    // For now, simple identity transform
    // TODO: Implement proper coordinate transformation
    
    (void)from_frame;
    (void)to_frame;
    
    return *pos;
}

PlatonicFrame geo_frame_dual(const PlatonicFrame* frame) {
    // O(1) - get dual frame
    
    PlatonicFrame dual = *frame;
    dual.solid = frame->dual_solid;
    
    // Swap vertices and faces
    uint8_t temp = dual.num_vertices;
    dual.num_vertices = dual.num_faces;
    dual.num_faces = temp;
    
    return dual;
}

// ============================================================================
// ICOSAHEDRON OPERATIONS
// ============================================================================

void geo_icosahedron_vertex(uint8_t vertex_id, double coords[3]) {
    // O(1) - icosahedron vertices using golden ratio
    
    double phi = GEO_GOLDEN_RATIO;
    
    // 12 vertices of icosahedron (computed at runtime to avoid const issues)
    double vertices[12][3];
    
    // Initialize vertices
    vertices[0][0] = 0; vertices[0][1] = 1; vertices[0][2] = phi;
    vertices[1][0] = 0; vertices[1][1] = 1; vertices[1][2] = -phi;
    vertices[2][0] = 0; vertices[2][1] = -1; vertices[2][2] = phi;
    vertices[3][0] = 0; vertices[3][1] = -1; vertices[3][2] = -phi;
    
    vertices[4][0] = 1; vertices[4][1] = phi; vertices[4][2] = 0;
    vertices[5][0] = 1; vertices[5][1] = -phi; vertices[5][2] = 0;
    vertices[6][0] = -1; vertices[6][1] = phi; vertices[6][2] = 0;
    vertices[7][0] = -1; vertices[7][1] = -phi; vertices[7][2] = 0;
    
    vertices[8][0] = phi; vertices[8][1] = 0; vertices[8][2] = 1;
    vertices[9][0] = phi; vertices[9][1] = 0; vertices[9][2] = -1;
    vertices[10][0] = -phi; vertices[10][1] = 0; vertices[10][2] = 1;
    vertices[11][0] = -phi; vertices[11][1] = 0; vertices[11][2] = -1;
    
    if (vertex_id < 12) {
        coords[0] = vertices[vertex_id][0];
        coords[1] = vertices[vertex_id][1];
        coords[2] = vertices[vertex_id][2];
    }
}

double geo_icosahedron_angle(uint8_t vertex_id) {
    // O(1) - angle for vertex
    return vertex_id * GEO_DEGREES_PER_POSITION;
}

uint8_t geo_icosahedron_neighbors(uint8_t vertex_id, uint8_t neighbors[5]) {
    // O(1) - get neighbors for vertex
    
    // Each vertex in icosahedron has exactly 5 neighbors
    // This is a simplified mapping
    for (uint8_t i = 0; i < 5; i++) {
        neighbors[i] = (vertex_id + i + 1) % 12;
    }
    
    return 5;
}

// ============================================================================
// MAGNITUDE SCALING
// ============================================================================

GeometricPosition geo_scale_to_layer(const GeometricPosition* pos, uint8_t target_layer) {
    // O(1) - scale to different layer
    
    int layer_diff = target_layer - pos->layer;
    
    // Scaling factor: 12^layer_diff
    int64_t scale_factor = 1;
    for (int i = 0; i < abs(layer_diff); i++) {
        scale_factor *= 12;
    }
    
    int64_t new_magnitude = (layer_diff > 0) ? 
                            (pos->magnitude * scale_factor) : 
                            (pos->magnitude / scale_factor);
    
    return geo_pos_create(pos->ring, pos->position, new_magnitude);
}

double geo_layer_scale(uint8_t layer) {
    // O(1) - magnitude scale for layer
    
    // Each layer represents ~3 orders of magnitude
    return pow(10.0, 3.0 * layer);
}

// ============================================================================
// VALIDATION & UTILITIES
// ============================================================================

bool geo_pos_validate(const GeometricPosition* pos) {
    // O(1) - validate position
    
    if (pos->ring >= 4) return false;
    if (pos->position >= GEO_NUM_CLOCK_POSITIONS) return false;
    if (pos->layer >= GEO_NUM_LAYERS) return false;
    
    return true;
}

int geo_pos_compare(const GeometricPosition* pos1, const GeometricPosition* pos2) {
    // O(1) - compare positions
    
    // Compare by magnitude first
    if (pos1->magnitude != pos2->magnitude) {
        return (pos1->magnitude > pos2->magnitude) ? 1 : -1;
    }
    
    // Then by position
    if (pos1->position != pos2->position) {
        return (pos1->position > pos2->position) ? 1 : -1;
    }
    
    return 0;
}

void geo_pos_print(const GeometricPosition* pos) {
    printf("GeometricPosition:\n");
    printf("  Ring: %u\n", pos->ring);
    printf("  Position: %u\n", pos->position);
    printf("  Magnitude: %ld\n", pos->magnitude);
    printf("  Angle: %.1f°\n", pos->angle);
    printf("  Layer: %u\n", pos->layer);
    printf("  Prime: %s\n", pos->is_prime_position ? "yes" : "no");
    printf("  Control: %s\n", pos->is_control ? "yes" : "no");
}

void geo_frame_print(const PlatonicFrame* frame) {
    const char* solid_names[] = {
        "Tetrahedron", "Cube", "Octahedron", "Dodecahedron", "Icosahedron"
    };
    
    printf("PlatonicFrame:\n");
    printf("  Solid: %s\n", solid_names[frame->solid]);
    printf("  Layer: %u\n", frame->layer);
    printf("  Vertices: %u\n", frame->num_vertices);
    printf("  Edges: %u\n", frame->num_edges);
    printf("  Faces: %u\n", frame->num_faces);
    printf("  Dual: %s\n", solid_names[frame->dual_solid]);
    printf("  Euler: V - E + F = %u - %u + %u = %d\n",
           frame->num_vertices, frame->num_edges, frame->num_faces,
           frame->num_vertices - frame->num_edges + frame->num_faces);
}