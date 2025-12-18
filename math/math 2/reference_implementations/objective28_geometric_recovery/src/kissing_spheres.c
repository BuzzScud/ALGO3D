#include "../include/kissing_spheres.h"
#include "../include/crystal_abacus.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Prime math functions (crystalline library)
extern double math_sqrt(double x);
extern double math_pow(double x, double y);
extern float math_abs(float x);

// Wrapper for double precision fabs
static inline double math_abs(double x) {
    return (x < 0.0) ? -x : x;
}

// Icosahedral vertex positions (normalized)
// 12 vertices of icosahedron inscribed in unit sphere
static const double ICOSAHEDRON_VERTICES[12][3] = {
    { 0.0,  1.0,  PHI},
    { 0.0,  1.0, -PHI},
    { 0.0, -1.0,  PHI},
    { 0.0, -1.0, -PHI},
    { 1.0,  PHI,  0.0},
    { 1.0, -PHI,  0.0},
    {-1.0,  PHI,  0.0},
    {-1.0, -PHI,  0.0},
    { PHI,  0.0,  1.0},
    { PHI,  0.0, -1.0},
    {-PHI,  0.0,  1.0},
    {-PHI,  0.0, -1.0}
};

double compute_sphere_radius(uint64_t k, uint32_t depth) {
    // Base radius scaled by golden ratio and depth
    double base_radius = 150.0;
    double scale = math_pow(PHI, -(double)depth);  // Smaller at deeper levels
    return base_radius * scale * (1.0 + (double)(k % 100) / 100.0);
}

uint64_t compute_neighbor_k(uint64_t center, uint32_t neighbor_idx, uint32_t depth) {
    // Compute neighbor k value using icosahedral geometry
    // Each neighbor is offset by a prime-based distance
    
    if (neighbor_idx >= NUM_KISSING_NEIGHBORS) return center;
    
    // Use small primes for offsets, scaled by depth
    uint64_t offset = SMALL_PRIMES[neighbor_idx] * (depth + 1);
    
    // Apply golden ratio scaling
    offset = (uint64_t)(offset * math_pow(PHI, (double)depth));
    
    // Alternate between adding and subtracting
    if (neighbor_idx % 2 == 0) {
        return center + offset;
    } else {
        return (center > offset) ? (center - offset) : offset;
    }
}

double* compute_3d_position(uint64_t k, uint32_t depth) {
    double* pos = malloc(3 * sizeof(double));
    if (!pos) return NULL;
    
    // Map k to 3D position using icosahedral symmetry
    // Use k modulo 12 to select base vertex
    uint32_t vertex_idx = k % NUM_KISSING_NEIGHBORS;
    
    // Scale by depth and k value
    double scale = math_pow(PHI, (double)depth) * (1.0 + (double)k / 1000.0);
    
    for (int i = 0; i < 3; i++) {
        pos[i] = ICOSAHEDRON_VERTICES[vertex_idx][i] * scale;
    }
    
    return pos;
}

KissingSphere* create_kissing_sphere_hierarchy(uint64_t center, uint32_t depth) {
    if (depth == 0) return NULL;
    
    KissingSphere* sphere = malloc(sizeof(KissingSphere));
    if (!sphere) return NULL;
    
    sphere->center = center;
    sphere->radius = compute_sphere_radius(center, depth);
    sphere->depth = depth;
    sphere->is_anchor = false;
    sphere->confidence = 0.0;
    
    // Compute 3D position
    sphere->position_3d = compute_3d_position(center, depth);
    if (!sphere->position_3d) {
        free(sphere);
        return NULL;
    }
    
    // Create 12 kissing neighbors (icosahedral packing)
    sphere->num_neighbors = NUM_KISSING_NEIGHBORS;
    sphere->neighbors = malloc(NUM_KISSING_NEIGHBORS * sizeof(KissingSphere*));
    if (!sphere->neighbors) {
        free(sphere->position_3d);
        free(sphere);
        return NULL;
    }
    
    // Recursively create neighbors
    for (uint32_t i = 0; i < NUM_KISSING_NEIGHBORS; i++) {
        uint64_t neighbor_k = compute_neighbor_k(center, i, depth);
        
        // Recurse to depth - 1
        sphere->neighbors[i] = create_kissing_sphere_hierarchy(neighbor_k, depth - 1);
        
        // If recursion failed, set to NULL
        if (!sphere->neighbors[i]) {
            // Clean up already created neighbors
            for (uint32_t j = 0; j < i; j++) {
                free_kissing_sphere(sphere->neighbors[j]);
            }
            free(sphere->neighbors);
            free(sphere->position_3d);
            free(sphere);
            return NULL;
        }
    }
    
    return sphere;
}

void free_kissing_sphere(KissingSphere* sphere) {
    if (!sphere) return;
    
    // Recursively free neighbors
    if (sphere->neighbors) {
        for (uint32_t i = 0; i < sphere->num_neighbors; i++) {
            free_kissing_sphere(sphere->neighbors[i]);
        }
        free(sphere->neighbors);
    }
    
    if (sphere->position_3d) free(sphere->position_3d);
    free(sphere);
}

double sphere_distance(const KissingSphere* a, const KissingSphere* b) {
    if (!a || !b || !a->position_3d || !b->position_3d) return 1e308;  // Large value instead of INFINITY
    
    double dx = a->position_3d[0] - b->position_3d[0];
    double dy = a->position_3d[1] - b->position_3d[1];
    double dz = a->position_3d[2] - b->position_3d[2];
    
    return math_sqrt(dx*dx + dy*dy + dz*dz);
}

bool spheres_kissing(const KissingSphere* a, const KissingSphere* b) {
    if (!a || !b) return false;
    
    double dist = sphere_distance(a, b);
    double sum_radii = a->radius + b->radius;
    
    // Kissing if distance ≈ sum of radii (within epsilon)
    return math_abs(dist - sum_radii) < EPSILON;
}

KissingSphere* find_nearest_sphere(KissingSphere* root, uint64_t target) {
    if (!root) return NULL;
    
    KissingSphere* nearest = root;
    int64_t diff = (int64_t)root->center - (int64_t)target;
    uint64_t min_dist = (uint64_t)math_abs((double)diff);
    
    // Check all neighbors recursively
    if (root->neighbors) {
        for (uint32_t i = 0; i < root->num_neighbors; i++) {
            KissingSphere* candidate = find_nearest_sphere(root->neighbors[i], target);
            if (candidate) {
                int64_t diff2 = (int64_t)candidate->center - (int64_t)target;
                uint64_t dist = (uint64_t)math_abs((double)diff2);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = candidate;
                }
            }
        }
    }
    
    return nearest;
}

uint32_t find_n_nearest_spheres(KissingSphere* root, uint64_t target,
                                uint32_t n, KissingSphere** out_spheres) {
    if (!root || !out_spheres || n == 0) return 0;
    
    // Collect all spheres first
    uint32_t total = count_spheres(root);
    KissingSphere** all_spheres = malloc(total * sizeof(KissingSphere*));
    if (!all_spheres) return 0;
    
    // Traverse and collect
    typedef struct {
        KissingSphere** arr;
        uint32_t* collected;
    } CollectData;
    
    uint32_t collected = 0;
    CollectData cdata = {all_spheres, &collected};
    
    bool collect_sphere(KissingSphere* s, void* data) {
        CollectData* cd = (CollectData*)data;
        cd->arr[(*cd->collected)++] = s;
        return true;
    }
    
    traverse_hierarchy(root, collect_sphere, &cdata);
    
    // Sort by distance to target
    for (uint32_t i = 0; i < collected; i++) {
        for (uint32_t j = i + 1; j < collected; j++) {
            int64_t diff_i = (int64_t)all_spheres[i]->center - (int64_t)target;
            int64_t diff_j = (int64_t)all_spheres[j]->center - (int64_t)target;
            uint64_t dist_i = (uint64_t)math_abs((double)diff_i);
            uint64_t dist_j = (uint64_t)math_abs((double)diff_j);
            
            if (dist_j < dist_i) {
                KissingSphere* temp = all_spheres[i];
                all_spheres[i] = all_spheres[j];
                all_spheres[j] = temp;
            }
        }
    }
    
    // Copy top N
    uint32_t num_found = (n < collected) ? n : collected;
    for (uint32_t i = 0; i < num_found; i++) {
        out_spheres[i] = all_spheres[i];
    }
    
    free(all_spheres);
    return num_found;
}

void mark_as_anchor(KissingSphere* sphere, double confidence) {
    if (!sphere) return;
    sphere->is_anchor = true;
    sphere->confidence = confidence;
}

uint32_t get_all_anchors(KissingSphere* root, KissingSphere** out_anchors,
                         uint32_t max_anchors) {
    if (!root || !out_anchors || max_anchors == 0) return 0;
    
    typedef struct {
        KissingSphere** out;
        uint32_t* count;
        uint32_t max;
    } AnchorData;
    
    uint32_t num_anchors = 0;
    AnchorData adata = {out_anchors, &num_anchors, max_anchors};
    
    bool collect_anchor(KissingSphere* s, void* data) {
        AnchorData* ad = (AnchorData*)data;
        if (s->is_anchor && (*ad->count) < ad->max) {
            ad->out[(*ad->count)++] = s;
        }
        return true;
    }
    
    traverse_hierarchy(root, collect_anchor, &adata);
    
    return num_anchors;
}

void traverse_hierarchy(KissingSphere* root,
                       bool (*func)(KissingSphere*, void*),
                       void* user_data) {
    if (!root || !func) return;
    
    // Apply function to current sphere
    if (!func(root, user_data)) return;  // Stop if function returns false
    
    // Recursively traverse neighbors
    if (root->neighbors) {
        for (uint32_t i = 0; i < root->num_neighbors; i++) {
            traverse_hierarchy(root->neighbors[i], func, user_data);
        }
    }
}

uint32_t count_spheres(KissingSphere* root) {
    if (!root) return 0;
    
    uint32_t count = 1;  // Count self
    
    // Recursively count neighbors
    if (root->neighbors) {
        for (uint32_t i = 0; i < root->num_neighbors; i++) {
            count += count_spheres(root->neighbors[i]);
        }
    }
    
    return count;
}

void print_hierarchy(KissingSphere* root, uint32_t indent) {
    if (!root) return;
    
    for (uint32_t i = 0; i < indent; i++) printf("  ");
    
    printf("Sphere: center=%lu, radius=%.2f, depth=%u, anchor=%s, confidence=%.3f\n",
           root->center, root->radius, root->depth,
           root->is_anchor ? "YES" : "NO", root->confidence);
    
    // Print neighbors
    if (root->neighbors) {
        for (uint32_t i = 0; i < root->num_neighbors; i++) {
            print_hierarchy(root->neighbors[i], indent + 1);
        }
    }
}