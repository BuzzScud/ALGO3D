#include "../include/crystal_abacus.h"
#include <string.h>
#include <stdio.h>

// Prime math functions (crystalline library)
extern double math_sqrt(double x);
extern float math_abs(float x);

// Wrapper for double precision fabs
static inline double math_abs(double x) {
    return (x < 0.0) ? -x : x;
}

// First 50 small primes
const uint32_t SMALL_PRIMES[50] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229
};

// Viable residues mod 30 (coprime to 30)
const uint32_t VIABLE_RESIDUES[NUM_VIABLE_RESIDUES] = {1, 7, 11, 13, 17, 19, 23, 29};

// Base gates for kissing spheres
const uint32_t BASE_GATES[NUM_BASE_GATES] = {169, 289, 361, 529, 841}; // 13^2, 17^2, 19^2, 23^2, 29^2

// Quadratic residues mod 30 (precomputed)
static uint32_t QR_MOD30[30];
static bool QR_INITIALIZED = false;

static void init_qr_mod30(void) {
    if (QR_INITIALIZED) return;
    for (int i = 0; i < 30; i++) {
        QR_MOD30[i] = (i * i) % 30;
    }
    QR_INITIALIZED = true;
}

CrystalAbacus* abacus_create(void) {
    init_qr_mod30();
    
    CrystalAbacus* abacus = malloc(sizeof(CrystalAbacus));
    if (!abacus) return NULL;
    
    abacus->capacity = 1000;
    abacus->primes = malloc(abacus->capacity * sizeof(uint64_t));
    if (!abacus->primes) {
        free(abacus);
        return NULL;
    }
    
    abacus->num_primes = 1;
    abacus->primes[0] = 2;
    abacus->candidate = 3;
    
    abacus->seen_capacity = 1000;
    abacus->seen = malloc(abacus->seen_capacity * sizeof(LatticeEmbedding*));
    if (!abacus->seen) {
        free(abacus->primes);
        free(abacus);
        return NULL;
    }
    abacus->num_seen = 1;
    abacus->seen[0] = embed_k(2, 3);  // Embed first prime with depth 3
    
    return abacus;
}

void abacus_free(CrystalAbacus* abacus) {
    if (!abacus) return;
    
    if (abacus->primes) free(abacus->primes);
    
    if (abacus->seen) {
        for (uint32_t i = 0; i < abacus->num_seen; i++) {
            free_embedding(abacus->seen[i]);
        }
        free(abacus->seen);
    }
    
    free(abacus);
}

LatticeEmbedding* embed_k(uint64_t k, uint32_t depth) {
    if (depth == 0) return NULL;
    
    LatticeEmbedding* emb = malloc(sizeof(LatticeEmbedding));
    if (!emb) return NULL;
    
    // Base embedding: k mod first 15 primes
    for (int i = 0; i < 15; i++) {
        emb->residues[i] = k % SMALL_PRIMES[i];
    }
    emb->depth = depth;
    
    // Recursive sub-embedding
    if (depth > 1 && k >= SMALL_PRIMES[0]) {
        emb->sub = embed_k(k / SMALL_PRIMES[0], depth - 1);
    } else {
        emb->sub = NULL;
    }
    
    return emb;
}

void free_embedding(LatticeEmbedding* emb) {
    if (!emb) return;
    if (emb->sub) free_embedding(emb->sub);
    free(emb);
}

bool embeddings_equal(const LatticeEmbedding* a, const LatticeEmbedding* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    
    // Compare residues
    for (int i = 0; i < 15; i++) {
        if (a->residues[i] != b->residues[i]) return false;
    }
    
    // Compare depth
    if (a->depth != b->depth) return false;
    
    // Recursively compare sub-embeddings
    return embeddings_equal(a->sub, b->sub);
}

DynamicGate* compute_dynamic_gate(uint64_t k) {
    DynamicGate* gate = malloc(sizeof(DynamicGate));
    if (!gate) return NULL;
    
    uint64_t scale = (k / 500) + 1;
    
    gate->num_centers = NUM_BASE_GATES;
    gate->centers = malloc(NUM_BASE_GATES * sizeof(uint64_t));
    if (!gate->centers) {
        free(gate);
        return NULL;
    }
    
    // Scale centers with golden ratio
    for (uint32_t i = 0; i < NUM_BASE_GATES; i++) {
        gate->centers[i] = (uint64_t)(BASE_GATES[i] * scale * PHI);
    }
    
    gate->radius = 150.0 * scale;
    
    return gate;
}

void free_dynamic_gate(DynamicGate* gate) {
    if (!gate) return;
    if (gate->centers) free(gate->centers);
    free(gate);
}

bool near_dynamic_gate(uint64_t k, uint32_t recurse_depth) {
    if (k < 200) return true;
    if (recurse_depth == 0) return false;
    
    DynamicGate* gate = compute_dynamic_gate(k);
    if (!gate) return false;
    
    bool result = false;
    
    for (uint32_t i = 0; i < gate->num_centers; i++) {
        uint64_t center = gate->centers[i];
        double dist = math_abs((double)k - (double)center);
        
        // Kissing/tangent check
        if (dist < gate->radius + EPSILON) {
            result = true;
            break;
        }
        
        // Recurse on sub-layer (self-similar)
        if (near_dynamic_gate((uint64_t)dist, recurse_depth - 1)) {
            result = true;
            break;
        }
    }
    
    free_dynamic_gate(gate);
    return result;
}

uint32_t wheel_position(uint64_t k) {
    return k % WHEEL;
}

bool recursive_clock_check(uint64_t k, uint32_t depth) {
    if (depth == 0) return true;
    
    uint32_t pos = wheel_position(k);
    
    // Check if position is viable
    bool viable = false;
    for (int i = 0; i < NUM_VIABLE_RESIDUES; i++) {
        if (pos == VIABLE_RESIDUES[i]) {
            viable = true;
            break;
        }
    }
    
    if (!viable) return false;
    
    // Recurse with golden mod shift
    uint64_t next_mod = (uint64_t)(WHEEL * PHI);  // ~48
    return recursive_clock_check(k % next_mod, depth - 1);
}

uint32_t quadratic_residue_mod30(uint64_t k) {
    return QR_MOD30[wheel_position(k)];
}

bool vedic_small(uint64_t k) {
    return (k == 3 || k == 5 || k == 7 || k == 11);
}

bool origami_q1(uint64_t k) {
    if (k == 3 || k == 5) return true;
    return recursive_clock_check(k, 3);
}

bool pre_filter_candidate(uint64_t k) {
    if (k < 2) return false;
    
    // Hard rejects with exemptions
    for (int i = 1; i < 50; i++) {  // Skip 2
        if (k % SMALL_PRIMES[i] == 0) {
            return (k == SMALL_PRIMES[i]);
        }
    }
    
    // Geometric filters with recursion
    if (!recursive_clock_check(k, 3)) return false;
    
    uint32_t qr = quadratic_residue_mod30(k);
    if (qr != 1 && qr != 19 && k > 5) return false;  // Viable QR for mod 30
    
    if (!near_dynamic_gate(k, 3)) return false;
    
    return true;
}

uint64_t abacus_next_prime(CrystalAbacus* abacus) {
    while (true) {
        uint64_t k = abacus->candidate;
        abacus->candidate += 2;  // Skip evens
        
        // Pre-filter
        if (!pre_filter_candidate(k)) continue;
        
        // Embed with depth 3 (recursive self-similarity)
        LatticeEmbedding* sig = embed_k(k, 3);
        if (!sig) continue;
        
        // Check if already seen
        bool seen = false;
        for (uint32_t i = 0; i < abacus->num_seen; i++) {
            if (embeddings_equal(sig, abacus->seen[i])) {
                seen = true;
                break;
            }
        }
        
        if (seen) {
            free_embedding(sig);
            continue;
        }
        
        // Trial division (optimized with entropy cut for large k)
        if (k > SMALL_PRIMES[49]) {
            uint64_t sqrt_k = (uint64_t)math_sqrt((double)k);
            bool is_composite = false;
            
            for (uint32_t i = 0; i < abacus->num_primes; i++) {
                if (abacus->primes[i] > sqrt_k) break;
                if (k % abacus->primes[i] == 0) {
                    is_composite = true;
                    break;
                }
            }
            
            if (is_composite) {
                free_embedding(sig);
                continue;
            }
        }
        
        // Add to primes
        if (abacus->num_primes >= abacus->capacity) {
            abacus->capacity *= 2;
            uint64_t* new_primes = realloc(abacus->primes, 
                                          abacus->capacity * sizeof(uint64_t));
            if (!new_primes) {
                free_embedding(sig);
                return 0;
            }
            abacus->primes = new_primes;
        }
        
        abacus->primes[abacus->num_primes++] = k;
        
        // Add to seen
        if (abacus->num_seen >= abacus->seen_capacity) {
            abacus->seen_capacity *= 2;
            LatticeEmbedding** new_seen = realloc(abacus->seen,
                                                 abacus->seen_capacity * sizeof(LatticeEmbedding*));
            if (!new_seen) {
                free_embedding(sig);
                return k;
            }
            abacus->seen = new_seen;
        }
        
        abacus->seen[abacus->num_seen++] = sig;
        
        return k;
    }
}

bool abacus_is_prime(CrystalAbacus* abacus, uint64_t m) {
    if (m == 2) return true;
    if (m < 2 || m % 2 == 0) return false;
    
    if (!pre_filter_candidate(m)) return false;
    
    uint64_t sqrt_m = (uint64_t)math_sqrt((double)m);
    
    // Generate primes up to sqrt(m)
    while (abacus->num_primes > 0 && abacus->primes[abacus->num_primes - 1] < sqrt_m) {
        abacus_next_prime(abacus);
    }
    
    // Trial division
    for (uint32_t i = 0; i < abacus->num_primes; i++) {
        if (abacus->primes[i] > sqrt_m) break;
        if (m % abacus->primes[i] == 0) return false;
    }
    
    return true;
}
