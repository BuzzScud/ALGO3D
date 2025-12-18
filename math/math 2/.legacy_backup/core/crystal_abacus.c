// crystal_abacus.c - Crystal Abacus Implementation (Crystalline Lattice Prime Generator)
// Integrated from: crystal_abacus_duplicates.c, missing_functions.c
// Part of the Prime Mathematics Library - Crystalline Lattice Architecture

#include "crystal_abacus.h"
#include "math/clock.h"
#include "math/prime.h"  // For prime_validate_by_clock()
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"

// ═══════════════════════════════════════════════════════════════════════════
// CRYSTAL ABACUS - INTEGER PRIME GENERATION
// ═══════════════════════════════════════════════════════════════════════════

CrystalAbacus* abacus_create(void) {
    CrystalAbacus* abacus = (CrystalAbacus*)malloc(sizeof(CrystalAbacus));
    if (!abacus) return NULL;
    
    abacus->primes = NULL;
    abacus->num_primes = 0;
    abacus->capacity = 0;
    abacus->candidate = 2;
    abacus->seen = NULL;
    
    return abacus;
}

void abacus_free(CrystalAbacus *abacus) {
    if (!abacus) return;
    
    if (abacus->primes) {
        free(abacus->primes);
    }
    
    if (abacus->seen) {
        // Free hash table
        for (int i = 0; i < abacus->seen->num_buckets; i++) {
            SeenNode* node = abacus->seen->buckets[i];
            while (node) {
                SeenNode* next = node->next;
                free(node);
                node = next;
            }
        }
        free(abacus->seen->buckets);
        free(abacus->seen);
    }
    
    free(abacus);
}

int abacus_next_prime(CrystalAbacus *abacus) {
    if (!abacus) return 0;
    
    // Simple trial division prime generation
    while (1) {
        int is_prime = 1;
        
        if (abacus->candidate < 2) {
            abacus->candidate = 2;
            return 2;
        }
        
        if (abacus->candidate == 2) {
            abacus->candidate = 3;
            return 2;
        }
        
        if (abacus->candidate % 2 == 0) {
            abacus->candidate++;
            continue;
        }
        
        for (int i = 3; i * i <= abacus->candidate; i += 2) {
            if (abacus->candidate % i == 0) {
                is_prime = 0;
                break;
            }
        }
        
        if (is_prime) {
            int result = abacus->candidate;
            abacus->candidate += 2;
            return result;
        }
        
        abacus->candidate += 2;
    }
}

bool abacus_is_prime(CrystalAbacus *abacus, int m) {
    (void)abacus; // Not used in this implementation
    
    if (m < 2) return false;
    if (m == 2) return true;
    if (m % 2 == 0) return false;
    
    for (int i = 3; i * i <= m; i += 2) {
        if (m % i == 0) return false;
    }
    
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════

// Note: These int versions renamed to avoid conflict with uint64_t versions in prime_lowlevel.h
int prime_max_int_local(int a, int b) {
    return (a > b) ? a : b;
}

int prime_min_int_local(int a, int b) {
    return (a < b) ? a : b;
}

/**
 * PUBLIC API: Primality test for external users
 * 
 * ARCHITECTURAL PRINCIPLE:
 * Internally, the crystalline math library trusts its own deterministic
 * clock lattice structure. This function is a wrapper that provides a
 * familiar API for external code while using our deterministic validation.
 * 
 * For internal use within crystalline/algorithms libraries, prefer direct
 * calls to prime_validate_by_clock() to express trust in our
 * geometric consistency.
 */
bool is_prime(uint64_t n) {
    // Trust our deterministic clock lattice structure
    return prime_validate_by_clock(n);
}

int* generate_n_primes(int n) {
    if (n <= 0) return NULL;
    
    int* primes = (int*)malloc(n * sizeof(int));
    if (!primes) return NULL;
    
    int count = 0;
    int candidate = 2;
    
    while (count < n) {
        // Internal use: Trust our deterministic clock lattice
        if (prime_validate_by_clock(candidate)) {
            primes[count++] = candidate;
        }
        candidate++;
    }
    
    return primes;
}

// ═══════════════════════════════════════════════════════════════════════════
// VIBRATIONAL TRANSDUCER - Crystalline Lattice Frequency Mapping
// ═══════════════════════════════════════════════════════════════════════════

double vibrational_transducer(double input, double prime_factor) {
    // Map input value to a frequency using prime-based transformation
    // This is part of the crystalline lattice architecture
    double base_freq = 432.0; // A4 tuning (crystalline resonance)
    double freq = base_freq * math_pow(2.0, input / 12.0); // Musical scale mapping
    
    // Apply prime factor modulation through golden ratio
    freq *= (1.0 + math_sin(prime_factor * PHI) * 0.1);
    
    return freq;
}

double lattice_vibrational_transducer(double input, double prime_factor, int depth) {
    (void)depth; // Depth parameter for future lattice integration
    return vibrational_transducer(input, prime_factor);
}