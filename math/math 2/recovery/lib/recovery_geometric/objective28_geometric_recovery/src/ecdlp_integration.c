/**
 * @file ecdlp_integration.c
 * @brief ECDLP Integration Implementation
 * 
 * Uses OpenSSL for EC operations, crystalline math for everything else.
 */

#include "../include/ecdlp_integration.h"
#include "../include/crystal_abacus.h"
#include "../include/tetration_attractors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// First 15 primes for lattice embedding
static const uint64_t FIRST_15_PRIMES[15] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47
};

// ============================================================================
// ECDLP INSTANCE MANAGEMENT
// ============================================================================

ECDLPInstance* ecdlp_create_instance(int nid) {
    ECDLPInstance* instance = (ECDLPInstance*)malloc(sizeof(ECDLPInstance));
    if (!instance) return NULL;
    
    // Create EC group
    instance->group = EC_GROUP_new_by_curve_name(nid);
    if (!instance->group) {
        free(instance);
        return NULL;
    }
    
    instance->nid = nid;
    instance->curve_name = strdup(get_curve_name(nid));
    
    // Get generator
    instance->G = EC_POINT_dup(EC_GROUP_get0_generator(instance->group), instance->group);
    if (!instance->G) {
        EC_GROUP_free(instance->group);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Get group order
    instance->order = BN_new();
    if (!EC_GROUP_get_order(instance->group, instance->order, NULL)) {
        EC_POINT_free(instance->G);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Generate random k
    instance->k = BN_new();
    if (!BN_rand_range(instance->k, instance->order)) {
        EC_POINT_free(instance->G);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        BN_free(instance->k);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Compute Q = k*G
    instance->Q = EC_POINT_new(instance->group);
    if (!EC_POINT_mul(instance->group, instance->Q, instance->k, NULL, NULL, NULL)) {
        EC_POINT_free(instance->G);
        EC_POINT_free(instance->Q);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        BN_free(instance->k);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    return instance;
}

ECDLPInstance* ecdlp_create_instance_with_k(int nid, const BIGNUM* k_value) {
    ECDLPInstance* instance = (ECDLPInstance*)malloc(sizeof(ECDLPInstance));
    if (!instance) return NULL;
    
    // Create EC group
    instance->group = EC_GROUP_new_by_curve_name(nid);
    if (!instance->group) {
        free(instance);
        return NULL;
    }
    
    instance->nid = nid;
    instance->curve_name = strdup(get_curve_name(nid));
    
    // Get generator
    instance->G = EC_POINT_dup(EC_GROUP_get0_generator(instance->group), instance->group);
    if (!instance->G) {
        EC_GROUP_free(instance->group);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Get group order
    instance->order = BN_new();
    if (!EC_GROUP_get_order(instance->group, instance->order, NULL)) {
        EC_POINT_free(instance->G);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Copy k
    instance->k = BN_dup(k_value);
    if (!instance->k) {
        EC_POINT_free(instance->G);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    // Compute Q = k*G
    instance->Q = EC_POINT_new(instance->group);
    if (!EC_POINT_mul(instance->group, instance->Q, instance->k, NULL, NULL, NULL)) {
        EC_POINT_free(instance->G);
        EC_POINT_free(instance->Q);
        EC_GROUP_free(instance->group);
        BN_free(instance->order);
        BN_free(instance->k);
        free(instance->curve_name);
        free(instance);
        return NULL;
    }
    
    return instance;
}

void ecdlp_free_instance(ECDLPInstance* instance) {
    if (!instance) return;
    
    if (instance->G) EC_POINT_free(instance->G);
    if (instance->Q) EC_POINT_free(instance->Q);
    if (instance->group) EC_GROUP_free(instance->group);
    if (instance->k) BN_free(instance->k);
    if (instance->order) BN_free(instance->order);
    if (instance->curve_name) free(instance->curve_name);
    
    free(instance);
}

bool ecdlp_verify_solution(const ECDLPInstance* instance, const BIGNUM* k_candidate) {
    if (!instance || !k_candidate) return false;
    
    // Compute k_candidate * G
    EC_POINT* test_Q = EC_POINT_new(instance->group);
    if (!test_Q) return false;
    
    if (!EC_POINT_mul(instance->group, test_Q, k_candidate, NULL, NULL, NULL)) {
        EC_POINT_free(test_Q);
        return false;
    }
    
    // Compare with Q
    int result = EC_POINT_cmp(instance->group, test_Q, instance->Q, NULL);
    EC_POINT_free(test_Q);
    
    return (result == 0);
}

// ============================================================================
// EC POINT OPERATIONS
// ============================================================================

ECPointCoords* ec_get_point_coords(const EC_GROUP* group, const EC_POINT* point) {
    if (!group || !point) return NULL;
    
    // Check if point is at infinity
    if (EC_POINT_is_at_infinity(group, point)) {
        ECPointCoords* coords = (ECPointCoords*)malloc(sizeof(ECPointCoords));
        if (!coords) return NULL;
        coords->x = NULL;
        coords->y = NULL;
        coords->is_infinity = true;
        return coords;
    }
    
    ECPointCoords* coords = (ECPointCoords*)malloc(sizeof(ECPointCoords));
    if (!coords) return NULL;
    
    coords->x = BN_new();
    coords->y = BN_new();
    coords->is_infinity = false;
    
    if (!coords->x || !coords->y) {
        if (coords->x) BN_free(coords->x);
        if (coords->y) BN_free(coords->y);
        free(coords);
        return NULL;
    }
    
    if (!EC_POINT_get_affine_coordinates(group, point, coords->x, coords->y, NULL)) {
        BN_free(coords->x);
        BN_free(coords->y);
        free(coords);
        return NULL;
    }
    
    return coords;
}

void ec_free_point_coords(ECPointCoords* coords) {
    if (!coords) return;
    if (coords->x) BN_free(coords->x);
    if (coords->y) BN_free(coords->y);
    free(coords);
}

bool ec_scalar_mul(const EC_GROUP* group, EC_POINT* result, 
                   const BIGNUM* k, const EC_POINT* point) {
    if (!group || !result || !k || !point) return false;
    return EC_POINT_mul(group, result, NULL, point, k, NULL) == 1;
}

bool ec_point_add(const EC_GROUP* group, EC_POINT* result,
                  const EC_POINT* p1, const EC_POINT* p2) {
    if (!group || !result || !p1 || !p2) return false;
    return EC_POINT_add(group, result, p1, p2, NULL) == 1;
}

// ============================================================================
// LATTICE EMBEDDING (CRYSTALLINE MATH)
// ============================================================================

bool lattice_embed_ec_point(const ECPointCoords* coords, ECLatticeEmbedding* embedding) {
    if (!coords || !embedding) return false;
    
    if (coords->is_infinity) {
        // Point at infinity maps to zero vector
        for (int i = 0; i < 15; i++) {
            embedding->coords[i] = 0.0;
            embedding->primes[i] = FIRST_15_PRIMES[i];
        }
        embedding->magnitude = 0.0;
        embedding->angle = 0.0;
        return true;
    }
    
    // Convert BIGNUM to bytes
    int x_len = BN_num_bytes(coords->x);
    int y_len = BN_num_bytes(coords->y);
    
    unsigned char* x_bytes = (unsigned char*)malloc(x_len);
    unsigned char* y_bytes = (unsigned char*)malloc(y_len);
    
    if (!x_bytes || !y_bytes) {
        if (x_bytes) free(x_bytes);
        if (y_bytes) free(y_bytes);
        return false;
    }
    
    BN_bn2bin(coords->x, x_bytes);
    BN_bn2bin(coords->y, y_bytes);
    
    // Embed into 15D lattice using first 15 primes
    for (int i = 0; i < 15; i++) {
        uint64_t prime = FIRST_15_PRIMES[i];
        embedding->primes[i] = prime;
        
        // Compute (x mod prime) and (y mod prime)
        uint64_t x_mod = 0;
        uint64_t y_mod = 0;
        
        for (int j = 0; j < x_len; j++) {
            x_mod = (x_mod * 256 + x_bytes[j]) % prime;
        }
        
        for (int j = 0; j < y_len; j++) {
            y_mod = (y_mod * 256 + y_bytes[j]) % prime;
        }
        
        // Normalize: coords[i] = (x_mod + y_mod) / (2 * prime)
        embedding->coords[i] = (double)(x_mod + y_mod) / (2.0 * (double)prime);
    }
    
    free(x_bytes);
    free(y_bytes);
    
    // Compute magnitude using prime_sqrt
    double sum_squares = 0.0;
    for (int i = 0; i < 15; i++) {
        sum_squares += embedding->coords[i] * embedding->coords[i];
    }
    embedding->magnitude = math_sqrt(sum_squares);
    
    // Compute angle using prime_atan2
    if (embedding->magnitude > 1e-10) {
        embedding->angle = prime_atan2(embedding->coords[1], embedding->coords[0]);
    } else {
        embedding->angle = 0.0;
    }
    
    return true;
}

double lattice_distance(const ECLatticeEmbedding* emb1, const ECLatticeEmbedding* emb2) {
    if (!emb1 || !emb2) return -1.0;
    
    double sum_squares = 0.0;
    for (int i = 0; i < 15; i++) {
        double diff = emb1->coords[i] - emb2->coords[i];
        sum_squares += diff * diff;
    }
    
    return math_sqrt(sum_squares);
}

double lattice_angle(const ECLatticeEmbedding* emb1, const ECLatticeEmbedding* emb2) {
    if (!emb1 || !emb2) return -1.0;
    
    // Compute dot product
    double dot = 0.0;
    for (int i = 0; i < 15; i++) {
        dot += emb1->coords[i] * emb2->coords[i];
    }
    
    // Compute angle using prime_acos
    double mag_product = emb1->magnitude * emb2->magnitude;
    if (mag_product < 1e-10) return 0.0;
    
    double cos_angle = dot / mag_product;
    
    // Clamp to [-1, 1] to avoid numerical issues
    if (cos_angle > 1.0) cos_angle = 1.0;
    if (cos_angle < -1.0) cos_angle = -1.0;
    
    return math_acos(cos_angle);
}

// ============================================================================
// K CANDIDATE GENERATION (FROM ATTRACTORS)
// ============================================================================

int generate_k_candidates_from_attractors(
    const ECDLPInstance* instance,
    const ECLatticeEmbedding* Q_embedding,
    BIGNUM** candidates,
    int max_candidates
) {
    if (!instance || !Q_embedding || !candidates || max_candidates <= 0) return 0;
    
    // Initialize tetration attractors
    AttractorSystem* attractors = create_attractor_system(1000000, 0.95);
    if (!attractors) return 0;
    
    compute_all_attractors(attractors);
    
    int count = 0;
    
    // For each attractor, generate k candidates
    for (uint32_t i = 0; i < attractors->num_attractors && count < max_candidates; i++) {
        uint64_t attractor = attractors->attractors[i].value;
        
        // Convert attractor to BIGNUM
        BIGNUM* k_candidate = BN_new();
        if (!k_candidate) continue;
        
        BN_set_word(k_candidate, attractor);
        
        // Ensure k_candidate < order
        if (BN_cmp(k_candidate, instance->order) >= 0) {
            BN_mod(k_candidate, k_candidate, instance->order, NULL);
        }
        
        candidates[count++] = k_candidate;
        
        // Generate neighbors around attractor
        for (int j = 1; j <= 10 && count < max_candidates; j++) {
            BIGNUM* neighbor = BN_new();
            if (!neighbor) continue;
            
            BN_copy(neighbor, k_candidate);
            BN_add_word(neighbor, j);
            
            if (BN_cmp(neighbor, instance->order) >= 0) {
                BN_mod(neighbor, neighbor, instance->order, NULL);
            }
            
            candidates[count++] = neighbor;
        }
    }
    
    free_attractor_system(attractors);
    
    return count;
}

BIGNUM* refine_k_candidate(
    const ECDLPInstance* instance,
    const BIGNUM* k_candidate,
    const ECLatticeEmbedding* Q_embedding,
    int max_iterations
) {
    if (!instance || !k_candidate || !Q_embedding) return NULL;
    
    BIGNUM* refined = BN_dup(k_candidate);
    if (!refined) return NULL;
    
    double best_distance = 1e10;
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Compute refined * G
        EC_POINT* test_Q = EC_POINT_new(instance->group);
        if (!test_Q) break;
        
        if (!EC_POINT_mul(instance->group, test_Q, refined, NULL, NULL, NULL)) {
            EC_POINT_free(test_Q);
            break;
        }
        
        // Get coordinates and embed
        ECPointCoords* coords = ec_get_point_coords(instance->group, test_Q);
        EC_POINT_free(test_Q);
        
        if (!coords) break;
        
        ECLatticeEmbedding test_embedding;
        if (!lattice_embed_ec_point(coords, &test_embedding)) {
            ec_free_point_coords(coords);
            break;
        }
        ec_free_point_coords(coords);
        
        // Compute distance to target
        double distance = lattice_distance(&test_embedding, Q_embedding);
        
        if (distance < best_distance) {
            best_distance = distance;
        } else {
            // No improvement, stop
            break;
        }
        
        // Adjust refined based on lattice feedback
        // Simple strategy: increment by 1
        BN_add_word(refined, 1);
        
        if (BN_cmp(refined, instance->order) >= 0) {
            BN_mod(refined, refined, instance->order, NULL);
        }
    }
    
    return refined;
}

// ============================================================================
// TESTING & VALIDATION
// ============================================================================

int test_ecdlp_recovery_toy(int num_tests) {
    int successes = 0;
    
    printf("Testing ECDLP recovery on toy curve (secp192k1)...\n");
    
    for (int i = 0; i < num_tests; i++) {
        // Create instance
        ECDLPInstance* instance = ecdlp_create_instance(NID_secp192k1);
        if (!instance) continue;
        
        // Get Q embedding
        ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
        if (!Q_coords) {
            ecdlp_free_instance(instance);
            continue;
        }
        
        ECLatticeEmbedding Q_embedding;
        if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
            ec_free_point_coords(Q_coords);
            ecdlp_free_instance(instance);
            continue;
        }
        ec_free_point_coords(Q_coords);
        
        // Generate candidates
        BIGNUM* candidates[1000];
        int num_candidates = generate_k_candidates_from_attractors(
            instance, &Q_embedding, candidates, 1000
        );
        
        // Test each candidate
        bool found = false;
        for (int j = 0; j < num_candidates; j++) {
            if (ecdlp_verify_solution(instance, candidates[j])) {
                successes++;
                found = true;
                break;
            }
        }
        
        // Free candidates
        for (int j = 0; j < num_candidates; j++) {
            BN_free(candidates[j]);
        }
        
        ecdlp_free_instance(instance);
        
        if (found) {
            printf("  Test %d: SUCCESS\n", i + 1);
        } else {
            printf("  Test %d: FAILED\n", i + 1);
        }
    }
    
    printf("Success rate: %d/%d (%.1f%%)\n", successes, num_tests, 
           100.0 * successes / num_tests);
    
    return successes;
}

int test_ecdlp_recovery_secp256k1(int num_tests) {
    int successes = 0;
    
    printf("Testing ECDLP recovery on secp256k1...\n");
    
    for (int i = 0; i < num_tests; i++) {
        // Create instance
        ECDLPInstance* instance = ecdlp_create_instance(NID_secp256k1);
        if (!instance) continue;
        
        // Get Q embedding
        ECPointCoords* Q_coords = ec_get_point_coords(instance->group, instance->Q);
        if (!Q_coords) {
            ecdlp_free_instance(instance);
            continue;
        }
        
        ECLatticeEmbedding Q_embedding;
        if (!lattice_embed_ec_point(Q_coords, &Q_embedding)) {
            ec_free_point_coords(Q_coords);
            ecdlp_free_instance(instance);
            continue;
        }
        ec_free_point_coords(Q_coords);
        
        // Generate candidates
        BIGNUM* candidates[1000];
        int num_candidates = generate_k_candidates_from_attractors(
            instance, &Q_embedding, candidates, 1000
        );
        
        // Test each candidate
        bool found = false;
        for (int j = 0; j < num_candidates; j++) {
            if (ecdlp_verify_solution(instance, candidates[j])) {
                successes++;
                found = true;
                break;
            }
        }
        
        // Free candidates
        for (int j = 0; j < num_candidates; j++) {
            BN_free(candidates[j]);
        }
        
        ecdlp_free_instance(instance);
        
        if (found) {
            printf("  Test %d: SUCCESS\n", i + 1);
        } else {
            printf("  Test %d: FAILED\n", i + 1);
        }
    }
    
    printf("Success rate: %d/%d (%.1f%%)\n", successes, num_tests, 
           100.0 * successes / num_tests);
    
    return successes;
}

double benchmark_ecdlp_recovery(int nid, int num_tests) {
    // TODO: Implement timing
    return 0.0;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* get_curve_name(int nid) {
    switch (nid) {
        case NID_secp192k1: return "secp192k1";
        case NID_secp256k1: return "secp256k1";
        case NID_secp384r1: return "secp384r1";
        case NID_secp521r1: return "secp521r1";
        default: return "unknown";
    }
}

void print_ecdlp_instance(const ECDLPInstance* instance) {
    if (!instance) return;
    
    printf("ECDLP Instance:\n");
    printf("  Curve: %s\n", instance->curve_name);
    printf("  Order: %s\n", BN_bn2hex(instance->order));
    printf("  k: %s\n", BN_bn2hex(instance->k));
}

void print_lattice_embedding(const ECLatticeEmbedding* embedding) {
    if (!embedding) return;
    
    printf("Lattice Embedding (15D):\n");
    printf("  Magnitude: %.6f\n", embedding->magnitude);
    printf("  Angle: %.6f rad\n", embedding->angle);
    printf("  Coordinates:\n");
    for (int i = 0; i < 15; i++) {
        printf("    [%2d] p=%2lu: %.6f\n", i, embedding->primes[i], embedding->coords[i]);
    }
}
