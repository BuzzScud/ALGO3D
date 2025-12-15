/**
 * @file kissing_spheres.c
 * @brief Kissing Sphere Operations for Geometric Recovery
 * 
 * Implements kissing sphere packing and contact analysis.
 * Uses NEW math library exclusively - NO legacy dependencies.
 * 
 * Thesis Reference: Chapter 14 - Kissing Spheres and Optimal Packing
 * 
 * Algorithm:
 * - Kissing sphere packing (12 neighbors in 3D)
 * - Sphere contact analysis
 * - Integration with clock lattice
 * - Optimal packing verification
 */

#include "math/types.h"
#include "math/arithmetic.h"
#include "math/transcendental.h"
#include "math/clock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define KISSING_NUMBER_3D 12
#define MAX_SPHERE_CONTACTS 12

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Sphere structure
 */
typedef struct {
    double center[3];           // Center position
    double radius;              // Radius
    uint32_t id;                // Sphere ID
    ClockPosition clock_pos;    // Clock lattice position
} Sphere;

/**
 * @brief Sphere contact
 */
typedef struct {
    uint32_t sphere1_id;        // First sphere ID
    uint32_t sphere2_id;        // Second sphere ID
    double contact_point[3];    // Contact point
    double contact_angle;       // Contact angle
} SphereContact;

/**
 * @brief Kissing sphere system
 */
typedef struct {
    Sphere* spheres;            // Array of spheres
    uint32_t num_spheres;       // Number of spheres
    SphereContact* contacts;    // Array of contacts
    uint32_t num_contacts;      // Number of contacts
    bool is_optimal_packing;    // Is this optimal packing?
} KissingSystem;

/* ============================================================================
 * INTERNAL FUNCTIONS
 * ============================================================================ */

/**
 * @brief Compute distance between two sphere centers
 */
static double compute_sphere_distance(const Sphere* s1, const Sphere* s2) {
    double dx = s1->center[0] - s2->center[0];
    double dy = s1->center[1] - s2->center[1];
    double dz = s1->center[2] - s2->center[2];
    return math_sqrt(dx*dx + dy*dy + dz*dz);
}

/**
 * @brief Check if two spheres are kissing (touching)
 */
static bool spheres_are_kissing(const Sphere* s1, const Sphere* s2) {
    double dist = compute_sphere_distance(s1, s2);
    double sum_radii = s1->radius + s2->radius;
    
    // Kissing if distance equals sum of radii (within tolerance)
    return math_abs(dist - sum_radii) < 1e-6;
}

/**
 * @brief Compute contact point between two kissing spheres
 */
static void compute_contact_point(
    const Sphere* s1,
    const Sphere* s2,
    double contact[3]
) {
    double dist = compute_sphere_distance(s1, s2);
    
    // Contact point is on line between centers
    // Distance from s1 center = s1->radius
    double t = s1->radius / dist;
    
    contact[0] = s1->center[0] + t * (s2->center[0] - s1->center[0]);
    contact[1] = s1->center[1] + t * (s2->center[1] - s1->center[1]);
    contact[2] = s1->center[2] + t * (s2->center[2] - s1->center[2]);
}

/**
 * @brief Generate optimal kissing sphere configuration
 * 
 * Places 12 spheres around a central sphere in optimal packing
 */
static void generate_optimal_packing(Sphere* spheres, double radius) {
    // Central sphere at origin
    spheres[0].center[0] = 0.0;
    spheres[0].center[1] = 0.0;
    spheres[0].center[2] = 0.0;
    spheres[0].radius = radius;
    spheres[0].id = 0;
    
    // 12 surrounding spheres at vertices of icosahedron
    // Golden ratio for icosahedron vertices
    double phi = MATH_PHI;
    double a = 1.0;
    double b = phi;
    
    // Normalize to place on sphere of radius 2*radius
    double scale = 2.0 * radius / math_sqrt(a*a + b*b);
    a *= scale;
    b *= scale;
    
    // 12 vertices of icosahedron
    double vertices[12][3] = {
        {0, a, b}, {0, a, -b}, {0, -a, b}, {0, -a, -b},
        {a, b, 0}, {a, -b, 0}, {-a, b, 0}, {-a, -b, 0},
        {b, 0, a}, {b, 0, -a}, {-b, 0, a}, {-b, 0, -a}
    };
    
    for (uint32_t i = 0; i < 12; i++) {
        spheres[i+1].center[0] = vertices[i][0];
        spheres[i+1].center[1] = vertices[i][1];
        spheres[i+1].center[2] = vertices[i][2];
        spheres[i+1].radius = radius;
        spheres[i+1].id = i + 1;
    }
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

/**
 * @brief Create kissing sphere system with optimal packing
 */
KissingSystem* kissing_create_optimal_system(double radius) {
    if (radius <= 0.0) return NULL;
    
    KissingSystem* system = (KissingSystem*)calloc(1, sizeof(KissingSystem));
    if (!system) return NULL;
    
    // Allocate 13 spheres (1 central + 12 kissing)
    system->num_spheres = 13;
    system->spheres = (Sphere*)calloc(13, sizeof(Sphere));
    if (!system->spheres) {
        free(system);
        return NULL;
    }
    
    // Generate optimal packing
    generate_optimal_packing(system->spheres, radius);
    
    // Allocate contacts (12 contacts for central sphere)
    system->contacts = (SphereContact*)calloc(MAX_SPHERE_CONTACTS, sizeof(SphereContact));
    if (!system->contacts) {
        free(system->spheres);
        free(system);
        return NULL;
    }
    
    // Detect contacts
    uint32_t contact_idx = 0;
    for (uint32_t i = 0; i < system->num_spheres; i++) {
        for (uint32_t j = i + 1; j < system->num_spheres; j++) {
            if (spheres_are_kissing(&system->spheres[i], &system->spheres[j])) {
                SphereContact* contact = &system->contacts[contact_idx++];
                contact->sphere1_id = system->spheres[i].id;
                contact->sphere2_id = system->spheres[j].id;
                compute_contact_point(&system->spheres[i], &system->spheres[j], 
                                     contact->contact_point);
                contact->contact_angle = 0.0; // TODO: compute angle
            }
        }
    }
    
    system->num_contacts = contact_idx;
    system->is_optimal_packing = (contact_idx == 12);
    
    return system;
}

/**
 * @brief Free kissing system
 */
void kissing_free_system(KissingSystem* system) {
    if (!system) return;
    
    if (system->spheres) free(system->spheres);
    if (system->contacts) free(system->contacts);
    free(system);
}

/**
 * @brief Get number of contacts for a sphere
 */
uint32_t kissing_get_num_contacts(const KissingSystem* system, uint32_t sphere_id) {
    if (!system) return 0;
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < system->num_contacts; i++) {
        if (system->contacts[i].sphere1_id == sphere_id ||
            system->contacts[i].sphere2_id == sphere_id) {
            count++;
        }
    }
    return count;
}

/**
 * @brief Verify optimal packing
 */
bool kissing_verify_optimal_packing(const KissingSystem* system) {
    if (!system) return false;
    
    // Optimal packing in 3D: central sphere has exactly 12 contacts
    uint32_t central_contacts = kissing_get_num_contacts(system, 0);
    return (central_contacts == KISSING_NUMBER_3D);
}

/**
 * @brief Print kissing system statistics
 */
void kissing_print_stats(const KissingSystem* system) {
    if (!system) return;
    
    printf("=== Kissing Sphere System ===\n");
    printf("Number of spheres: %u\n", system->num_spheres);
    printf("Number of contacts: %u\n", system->num_contacts);
    printf("Optimal packing: %s\n", system->is_optimal_packing ? "YES" : "NO");
    
    if (system->num_spheres > 0) {
        printf("Central sphere contacts: %u\n", kissing_get_num_contacts(system, 0));
        printf("Expected for optimal: %d\n", KISSING_NUMBER_3D);
    }
    
    printf("=============================\n");
}