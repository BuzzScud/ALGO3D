/**
 * geometric_anchors.c - Pure Geometric Anchor System Implementation
 * 
 * This implements the corrected algorithm where anchors are Platonic solid
 * vertices, NOT known k values. In production, NO known k is needed!
 */

#include "geometric_anchors.h"
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// Mathematical constants
#define PHI ((1.0 + math_sqrt(5.0)) / 2.0)
#define PI M_PI
#define TWO_PI (2.0 * M_PI)

// Dimensional frequencies (from cllm_mathematical_constants.h)
const uint64_t DIMENSIONAL_FREQUENCIES[13] = {
    3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 37, 41
};

// 42° phase offset (7/60 on clock = 7 minutes)
#define PHASE_OFFSET_42_DEG (42.0 * PI / 180.0)

// ============================================================================
// PLATONIC SOLID GENERATION IN 13D
// ============================================================================

static void generate_tetrahedron_13d(GeometricAnchor* anchors, int start_idx) {
    // Tetrahedron: 4 vertices
    // Use golden ratio for 13D embedding
    
    for (int v = 0; v < 4; v++) {
        anchors[start_idx + v].solid_type = 0;  // Tetrahedron
        anchors[start_idx + v].vertex_index = v;
        
        // Generate vertex using tetrahedral angles
        double angle = v * TWO_PI / 4.0;
        
        for (int d = 0; d < 13; d++) {
            // Use dimensional frequency for this dimension
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            
            // Position = math_cos(angle * φ_d) * golden_ratio^d
            anchors[start_idx + v].position[d] = 
                math_cos(angle * phi_d) * math_pow(PHI, d % 3);
        }
    }
}

static void generate_cube_13d(GeometricAnchor* anchors, int start_idx) {
    // Cube: 8 vertices
    // Use cubic symmetry for 13D embedding
    
    for (int v = 0; v < 8; v++) {
        anchors[start_idx + v].solid_type = 1;  // Cube
        anchors[start_idx + v].vertex_index = v;
        
        // Binary representation for cube vertices
        int x = (v & 1) ? 1 : -1;
        int y = (v & 2) ? 1 : -1;
        int z = (v & 4) ? 1 : -1;
        
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            
            // Combine x,y,z with dimensional frequency
            anchors[start_idx + v].position[d] = 
                (x * math_cos(phi_d) + y * math_sin(phi_d) + z * math_cos(2.0 * phi_d)) / math_sqrt(3.0);
        }
    }
}

static void generate_octahedron_13d(GeometricAnchor* anchors, int start_idx) {
    // Octahedron: 6 vertices (dual of cube)
    
    for (int v = 0; v < 6; v++) {
        anchors[start_idx + v].solid_type = 2;  // Octahedron
        anchors[start_idx + v].vertex_index = v;
        
        // Octahedral angles (6-fold symmetry)
        double angle = v * TWO_PI / 6.0;
        
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            
            anchors[start_idx + v].position[d] = 
                math_cos(angle * phi_d) * math_pow(PHI, (d % 2));
        }
    }
}

static void generate_dodecahedron_13d(GeometricAnchor* anchors, int start_idx) {
    // Dodecahedron: 20 vertices
    // Use golden ratio extensively (dodecahedron is golden ratio solid)
    
    for (int v = 0; v < 20; v++) {
        anchors[start_idx + v].solid_type = 3;  // Dodecahedron
        anchors[start_idx + v].vertex_index = v;
        
        // Dodecahedral angles with golden ratio
        double angle = v * TWO_PI / 20.0;
        
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            
            // Use golden ratio for dodecahedron
            anchors[start_idx + v].position[d] = 
                math_cos(angle * phi_d * PHI) * math_pow(PHI, d % 5);
        }
    }
}

static void generate_icosahedron_13d(GeometricAnchor* anchors, int start_idx) {
    // Icosahedron: 12 vertices (dual of dodecahedron)
    // 12-fold symmetry (kissing spheres!)
    
    for (int v = 0; v < 12; v++) {
        anchors[start_idx + v].solid_type = 4;  // Icosahedron
        anchors[start_idx + v].vertex_index = v;
        
        // Icosahedral angles (12-fold symmetry)
        double angle = v * TWO_PI / 12.0;
        
        for (int d = 0; d < 13; d++) {
            double phi_d = (double)DIMENSIONAL_FREQUENCIES[d];
            
            // 12-fold symmetry with golden ratio
            anchors[start_idx + v].position[d] = 
                math_cos(angle * phi_d) * math_pow(PHI, d % 4);
        }
    }
}

GeometricAnchor* generate_platonic_anchors_13d(int* num_anchors) {
    // Total: 4 + 8 + 6 + 20 + 12 = 50 vertices
    GeometricAnchor* anchors = calloc(50, sizeof(GeometricAnchor));
    if (!anchors) return NULL;
    
    generate_tetrahedron_13d(anchors, 0);
    generate_cube_13d(anchors, 4);
    generate_octahedron_13d(anchors, 12);
    generate_dodecahedron_13d(anchors, 18);
    generate_icosahedron_13d(anchors, 38);
    
    *num_anchors = 50;
    return anchors;
}

// ============================================================================
// π×φ DISTANCE METRIC
// ============================================================================

double pi_phi_distance_13d(const double* p1, const double* p2) {
    double sum = 0.0;
    
    for (int d = 0; d < 13; d++) {
        double diff = p1[d] - p2[d];
        
        // Weight by dimensional frequency
        double weight = (double)DIMENSIONAL_FREQUENCIES[d];
        
        sum += weight * diff * diff;
    }
    
    // Normalize by π×φ
    return math_sqrt(sum) / (PI * PHI);
}

// ============================================================================
// SHARED VERTEX DETECTION
// ============================================================================

SharedVertex* find_shared_geometric_vertices(
    GeometricAnchor* anchors,
    int num_anchors,
    double tolerance,
    int* num_shared
) {
    // Allocate space for potential shared vertices
    SharedVertex* shared = calloc(3000, sizeof(SharedVertex));
    if (!shared) return NULL;
    
    int count = 0;
    
    // Compare all pairs of anchors
    for (int i = 0; i < num_anchors; i++) {
        for (int j = i + 1; j < num_anchors; j++) {
            // Skip if same solid type
            if (anchors[i].solid_type == anchors[j].solid_type) continue;
            
            double dist = pi_phi_distance_13d(
                anchors[i].position,
                anchors[j].position
            );
            
            if (dist < tolerance) {
                // Found shared vertex!
                // Check if already in list
                bool found = false;
                for (int k = 0; k < count; k++) {
                    double d = pi_phi_distance_13d(
                        shared[k].position,
                        anchors[i].position
                    );
                    if (d < tolerance) {
                        // Add to existing shared vertex
                        if (shared[k].num_solids < 5) {
                            shared[k].solid_types[shared[k].num_solids] = anchors[j].solid_type;
                            shared[k].num_solids++;
                        }
                        found = true;
                        break;
                    }
                }
                
                if (!found && count < 3000) {
                    // Create new shared vertex
                    memcpy(shared[count].position, anchors[i].position, 13 * sizeof(double));
                    shared[count].num_solids = 2;
                    shared[count].solid_types[0] = anchors[i].solid_type;
                    shared[count].solid_types[1] = anchors[j].solid_type;
                    shared[count].stability_score = 0.0;
                    count++;
                }
            }
        }
    }
    
    *num_shared = count;
    return shared;
}

// ============================================================================
// ANCHOR STABILITY
// ============================================================================

void compute_anchor_stability(SharedVertex* anchors, int num_anchors) {
    for (int i = 0; i < num_anchors; i++) {
        // Base stability = number of solids sharing this vertex
        anchors[i].stability_score = (double)anchors[i].num_solids / 5.0;
        
        // Bonus for being on multiple clock rings
        // (compute which ring based on radius)
        double radius = 0.0;
        for (int d = 0; d < 13; d++) {
            radius += anchors[i].position[d] * anchors[i].position[d];
        }
        radius = math_sqrt(radius);
        
        if (radius < 0.375) anchors[i].clock_ring = 0;
        else if (radius < 0.625) anchors[i].clock_ring = 1;
        else if (radius < 0.875) anchors[i].clock_ring = 2;
        else anchors[i].clock_ring = 3;
        
        // Bonus for being on inner rings (more stable)
        anchors[i].stability_score *= (1.0 + 0.1 * (3 - anchors[i].clock_ring));
    }
}

// ============================================================================
// FIND 3 NEAREST ANCHORS
// ============================================================================

void find_3_nearest_anchors(
    const double target[13],
    SharedVertex* anchors,
    int num_anchors,
    SharedVertex* nearest[3]
) {
    // Compute distances to all anchors
    double* distances = malloc(num_anchors * sizeof(double));
    int* indices = malloc(num_anchors * sizeof(int));
    
    for (int i = 0; i < num_anchors; i++) {
        distances[i] = pi_phi_distance_13d(target, anchors[i].position);
        indices[i] = i;
    }
    
    // Sort by distance (bubble sort for simplicity)
    for (int i = 0; i < num_anchors - 1; i++) {
        for (int j = 0; j < num_anchors - i - 1; j++) {
            if (distances[j] > distances[j+1]) {
                double tmp_d = distances[j];
                distances[j] = distances[j+1];
                distances[j+1] = tmp_d;
                
                int tmp_i = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = tmp_i;
            }
        }
    }
    
    // Return 3 nearest
    nearest[0] = &anchors[indices[0]];
    nearest[1] = &anchors[indices[1]];
    nearest[2] = &anchors[indices[2]];
    
    free(distances);
    free(indices);
}

// ============================================================================
// PYTHAGOREAN TRIPLE VERIFICATION
// ============================================================================

bool verify_pythagorean_triple(
    SharedVertex* v1,
    SharedVertex* v2,
    SharedVertex* v3
) {
    // Compute pairwise π×φ distances
    double d12 = pi_phi_distance_13d(v1->position, v2->position);
    double d13 = pi_phi_distance_13d(v1->position, v3->position);
    double d23 = pi_phi_distance_13d(v2->position, v3->position);
    
    // Sort distances manually
    double a, b, c;
    if (d12 <= d13 && d12 <= d23) {
        a = d12;
        b = (d13 <= d23) ? d13 : d23;
        c = (d13 <= d23) ? d23 : d13;
    } else if (d13 <= d12 && d13 <= d23) {
        a = d13;
        b = (d12 <= d23) ? d12 : d23;
        c = (d12 <= d23) ? d23 : d12;
    } else {
        a = d23;
        b = (d12 <= d13) ? d12 : d13;
        c = (d12 <= d13) ? d13 : d12;
    }
    
    // Check Pythagorean relationship: a² + b² ≈ c²
    double lhs = a*a + b*b;
    double rhs = c*c;
    double error = math_abs(lhs - rhs) / (rhs + 1e-10);
    
    return error < 0.05;  // 5% tolerance
}

bool find_pythagorean_triple_anchors(
    const double target[13],
    SharedVertex* anchors,
    int num_anchors,
    SharedVertex* triple[3]
) {
    // Find 10 nearest anchors
    double* distances = malloc(num_anchors * sizeof(double));
    int* indices = malloc(num_anchors * sizeof(int));
    
    for (int i = 0; i < num_anchors; i++) {
        distances[i] = pi_phi_distance_13d(target, anchors[i].position);
        indices[i] = i;
    }
    
    // Sort
    for (int i = 0; i < num_anchors - 1; i++) {
        for (int j = 0; j < num_anchors - i - 1; j++) {
            if (distances[j] > distances[j+1]) {
                double tmp_d = distances[j];
                distances[j] = distances[j+1];
                distances[j+1] = tmp_d;
                
                int tmp_i = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = tmp_i;
            }
        }
    }
    
    // Try combinations of top 10 to find Pythagorean triple
    for (int i = 0; i < 10 && i < num_anchors; i++) {
        for (int j = i+1; j < 10 && j < num_anchors; j++) {
            for (int k = j+1; k < 10 && k < num_anchors; k++) {
                if (verify_pythagorean_triple(
                    &anchors[indices[i]],
                    &anchors[indices[j]],
                    &anchors[indices[k]]
                )) {
                    triple[0] = &anchors[indices[i]];
                    triple[1] = &anchors[indices[j]];
                    triple[2] = &anchors[indices[k]];
                    
                    free(distances);
                    free(indices);
                    return true;
                }
            }
        }
    }
    
    free(distances);
    free(indices);
    return false;
}

// ============================================================================
// Q TO POSITION MAPPING
// ============================================================================

void hash_Q_to_13d_position(EC_POINT* Q, EC_GROUP* group, double position[13]) {
    // Get Q coordinates
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    EC_POINT_get_affine_coordinates(group, Q, x, y, NULL);
    
    // Hash to 13 dimensions using dimensional frequencies
    for (int d = 0; d < 13; d++) {
        uint64_t phi_i = DIMENSIONAL_FREQUENCIES[d];
        
        // Compute position in this dimension
        // Use both x and y coordinates
        uint64_t x_mod = BN_mod_word(x, phi_i);
        uint64_t y_mod = BN_mod_word(y, phi_i * phi_i);
        
        double x_contrib = (double)x_mod / (double)phi_i;
        double y_contrib = (double)y_mod / (double)(phi_i * phi_i);
        
        position[d] = math_fmod(x_contrib * TWO_PI + y_contrib * PI, TWO_PI);
    }
    
    BN_free(x);
    BN_free(y);
}

void map_13d_to_clock(const double position[13], int* ring, int* pos, double* angle) {
    // Compute angle using π×φ metric
    *angle = 0.0;
    for (int d = 0; d < 13; d++) {
        *angle += position[d] * DIMENSIONAL_FREQUENCIES[d];
    }
    *angle = math_fmod(*angle, TWO_PI);
    if (*angle < 0) *angle += TWO_PI;
    
    // DISABLED: 42° phase offset (was hardcoded for k=42 test case)
    // *angle += PHASE_OFFSET_42_DEG;
    // *angle = math_fmod(*angle, TWO_PI);
    
    // Compute radius
    double radius = 0.0;
    for (int d = 0; d < 13; d++) {
        radius += position[d] * position[d];
    }
    radius = math_sqrt(radius);
    
    // Determine ring
    if (radius < 0.375) *ring = 0;
    else if (radius < 0.625) *ring = 1;
    else if (radius < 0.875) *ring = 2;
    else *ring = 3;
    
    // Determine position on ring
    int ring_sizes[] = {12, 60, 60, 100};
    *pos = (int)(*angle / TWO_PI * ring_sizes[*ring]);
}

// ============================================================================
// TETRATION ATTRACTORS
// ============================================================================

TetrationAttractor compute_tetration_attractor(uint64_t base, int depth) {
    TetrationAttractor attractor;
    attractor.base = base;
    attractor.depth = depth;
    attractor.converged = false;
    
    double result = (double)base;
    double prev = 0.0;
    
    for (int i = 1; i < depth; i++) {
        prev = result;
        result = math_pow((double)base, result);
        
        // Check for convergence
        if (math_abs(result - prev) < 1e-6) {
            attractor.converged = true;
            break;
        }
        
        // Prevent overflow
        if (result > 1e15 || math_is_nan(result) || math_is_inf(result)) {
            result = prev;
            break;
        }
    }
    
    attractor.value = result;
    return attractor;
}

void compute_all_attractors(TetrationAttractor attractors[13]) {
    for (int d = 0; d < 13; d++) {
        attractors[d] = compute_tetration_attractor(DIMENSIONAL_FREQUENCIES[d], 5);
    }
}

void bias_toward_attractors(double position[13]) {
    TetrationAttractor attractors[13];
    compute_all_attractors(attractors);
    
    for (int d = 0; d < 13; d++) {
        if (attractors[d].converged) {
            // Bias toward attractor
            double attractor_angle = math_fmod(attractors[d].value, TWO_PI);
            double bias_strength = 0.05;  // 5% bias
            
            position[d] = (1.0 - bias_strength) * position[d] + 
                          bias_strength * attractor_angle;
        }
    }
}

double distance_to_nearest_attractor(
    const double position[13],
    TetrationAttractor attractors[13]
) {
    double min_dist = 1e9;
    
    for (int d = 0; d < 13; d++) {
        if (attractors[d].converged) {
            double attractor_angle = math_fmod(attractors[d].value, TWO_PI);
            double dist = math_abs(position[d] - attractor_angle);
            
            // Handle wraparound
            if (dist > PI) dist = TWO_PI - dist;
            
            if (dist < min_dist) min_dist = dist;
        }
    }
    
    return min_dist;
}

// ============================================================================
// GCD CONSTRAINTS
// ============================================================================

static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

static uint64_t gcd5(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    return gcd(gcd(gcd(gcd(a, b), c), d), e);
}

static uint64_t lcm(uint64_t a, uint64_t b) {
    return (a * b) / gcd(a, b);
}

static uint64_t lcm5(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    return lcm(lcm(lcm(lcm(a, b), c), d), e);
}

GCDConstraints compute_gcd_constraints(void) {
    GCDConstraints constraints;
    
    // Vertex counts: Tetrahedron=4, Cube=8, Octahedron=6, Dodecahedron=20, Icosahedron=12
    constraints.gcd_vertices = gcd5(4, 8, 6, 20, 12);  // = 2
    constraints.lcm_vertices = lcm5(4, 8, 6, 20, 12);  // = 120
    
    // Edge counts: 6, 12, 12, 30, 30
    constraints.gcd_edges = gcd5(6, 12, 12, 30, 30);  // = 6
    constraints.lcm_edges = lcm5(6, 12, 12, 30, 30);  // = 60
    
    // Face counts: 4, 6, 8, 12, 20
    constraints.gcd_faces = gcd5(4, 6, 8, 12, 20);  // = 2
    constraints.lcm_faces = lcm5(4, 6, 8, 12, 20);  // = 60
    
    return constraints;
}

uint64_t apply_gcd_constraints(uint64_t k_estimate, GCDConstraints gcd) {
    // k must be even (gcd_vertices = 2)
    if (k_estimate % 2 != 0) {
        k_estimate++;
    }
    
    // k mod 6 should follow edge pattern
    uint64_t mod6 = k_estimate % gcd.gcd_edges;
    if (mod6 != 0) {
        k_estimate += (gcd.gcd_edges - mod6);
    }
    
    return k_estimate;
}

bool satisfies_gcd_constraints(uint64_t k, GCDConstraints gcd) {
    // Must be even
    if (k % gcd.gcd_vertices != 0) return false;
    
    // Must satisfy edge constraint
    if (k % gcd.gcd_edges != 0) return false;
    
    return true;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

GeometricAnchorSystem* init_geometric_anchor_system(void) {
    GeometricAnchorSystem* system = calloc(1, sizeof(GeometricAnchorSystem));
    if (!system) return NULL;
    
    // Generate base anchors (50 vertices from 5 Platonic solids)
    system->base_anchors = generate_platonic_anchors_13d(&system->num_base_anchors);
    if (!system->base_anchors) {
        free(system);
        return NULL;
    }
    
    // Find shared vertices (intersections)
    system->shared_vertices = find_shared_geometric_vertices(
        system->base_anchors,
        system->num_base_anchors,
        0.01,  // tolerance
        &system->num_shared_vertices
    );
    
    if (!system->shared_vertices) {
        free(system->base_anchors);
        free(system);
        return NULL;
    }
    
    // Compute stability scores
    compute_anchor_stability(system->shared_vertices, system->num_shared_vertices);
    
    // Compute GCD constraints
    system->gcd = compute_gcd_constraints();
    
    // Compute tetration attractors
    compute_all_attractors(system->attractors);
    
    return system;
}

void free_geometric_anchor_system(GeometricAnchorSystem* system) {
    if (!system) return;
    
    free(system->base_anchors);
    free(system->shared_vertices);
    free(system->boundaries);
    free(system->intersections);
    free(system);
}
