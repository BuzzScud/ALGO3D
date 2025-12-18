/**
 * Test: Map All 20 Tori to Clock Lattice Positions
 * 
 * Revised Phase 4: Visualize Complete pq Factorization Structure
 * 
 * Maps all 20 tori (from Phase 2) to their corresponding clock positions
 * to understand the hierarchical structure and geometric relationships.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../../include/clock_lattice.h"
#include "../../../include/prime_float_math.h"

// For n = p × q, we have 20 tori representing:
// - Primary: p, q (2 tori)
// - Secondary: p², q², pq (3 tori)
// - Tertiary: p³, q³, p²q, pq² (4 tori)
// - Quaternary: p⁴, q⁴, p³q, p²q², pq³ (5 tori)
// - Quinary: p⁵, q⁵, p⁴q, p³q², p²q³, pq⁴ (6 tori)
// Total: 2 + 3 + 4 + 5 + 6 = 20 tori

#define MAX_TORI 20

typedef struct {
    int torus_id;
    char label[32];
    uint64_t value;  // p^i * q^j
    int p_power;
    int q_power;
    int ring;
    int position;
    double angle;
    double radius;
} TorusClockMapping;

/**
 * Calculate p^i * q^j
 */
uint64_t power_multiply(uint64_t p, int p_power, uint64_t q, int q_power) {
    uint64_t result = 1;
    for (int i = 0; i < p_power; i++) {
        result *= p;
    }
    for (int i = 0; i < q_power; i++) {
        result *= q;
    }
    return result;
}

/**
 * Get prime index for a value
 */
int get_prime_index_for_value(uint64_t value) {
    // First 20 primes
    uint64_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
                         31, 37, 41, 43, 47, 53, 59, 61, 67, 71};
    
    for (int i = 0; i < 20; i++) {
        if (primes[i] == value) {
            return i + 1;  // 1-based
        }
    }
    return -1;
}

/**
 * Map all 20 tori to clock positions
 */
void map_20_tori_to_clock(uint64_t p, uint64_t q, TorusClockMapping* mappings) {
    int torus_idx = 0;
    
    printf("\n=== Mapping 20 Tori to Clock Lattice ===\n");
    printf("\nFor n = %lu × %lu = %lu\n\n", p, q, p * q);
    
    // Primary tori (2)
    printf("PRIMARY TORI (2):\n");
    for (int i = 0; i <= 1; i++) {
        int p_pow = (i == 0) ? 1 : 0;
        int q_pow = (i == 0) ? 0 : 1;
        uint64_t value = power_multiply(p, p_pow, q, q_pow);
        
        mappings[torus_idx].torus_id = torus_idx + 1;
        mappings[torus_idx].p_power = p_pow;
        mappings[torus_idx].q_power = q_pow;
        mappings[torus_idx].value = value;
        
        if (p_pow == 1) {
            snprintf(mappings[torus_idx].label, 32, "p");
        } else {
            snprintf(mappings[torus_idx].label, 32, "q");
        }
        
        // Get clock position
        int prime_idx = get_prime_index_for_value(value);
        if (prime_idx > 0) {
            BabylonianClockPosition pos = map_prime_index_to_clock(prime_idx);
            mappings[torus_idx].ring = pos.ring;
            mappings[torus_idx].position = pos.position;
            mappings[torus_idx].angle = pos.angle;
            mappings[torus_idx].radius = pos.radius;
            
            printf("  Torus %2d: %s = %lu (Prime #%d) -> Ring %d, Pos %d, Angle %.2f°\n",
                   torus_idx + 1, mappings[torus_idx].label, value, prime_idx,
                   pos.ring, pos.position, pos.angle * 180.0 / M_PI);
        } else {
            printf("  Torus %2d: %s = %lu (not in prime table)\n",
                   torus_idx + 1, mappings[torus_idx].label, value);
            mappings[torus_idx].ring = -1;
        }
        
        torus_idx++;
    }
    
    // Secondary tori (3)
    printf("\nSECONDARY TORI (3):\n");
    for (int i = 0; i < 3; i++) {
        int p_pow = (i == 0) ? 2 : (i == 1) ? 0 : 1;
        int q_pow = (i == 0) ? 0 : (i == 1) ? 2 : 1;
        uint64_t value = power_multiply(p, p_pow, q, q_pow);
        
        mappings[torus_idx].torus_id = torus_idx + 1;
        mappings[torus_idx].p_power = p_pow;
        mappings[torus_idx].q_power = q_pow;
        mappings[torus_idx].value = value;
        
        if (p_pow == 2 && q_pow == 0) {
            snprintf(mappings[torus_idx].label, 32, "p²");
        } else if (p_pow == 0 && q_pow == 2) {
            snprintf(mappings[torus_idx].label, 32, "q²");
        } else {
            snprintf(mappings[torus_idx].label, 32, "pq");
        }
        
        printf("  Torus %2d: %s = %lu\n",
               torus_idx + 1, mappings[torus_idx].label, value);
        mappings[torus_idx].ring = -1;  // Not prime, so no clock position
        
        torus_idx++;
    }
    
    // Tertiary tori (4)
    printf("\nTERTIARY TORI (4):\n");
    for (int i = 0; i < 4; i++) {
        int p_pow = (i == 0) ? 3 : (i == 1) ? 0 : (i == 2) ? 2 : 1;
        int q_pow = (i == 0) ? 0 : (i == 1) ? 3 : (i == 2) ? 1 : 2;
        uint64_t value = power_multiply(p, p_pow, q, q_pow);
        
        mappings[torus_idx].torus_id = torus_idx + 1;
        mappings[torus_idx].p_power = p_pow;
        mappings[torus_idx].q_power = q_pow;
        mappings[torus_idx].value = value;
        
        if (p_pow == 3 && q_pow == 0) {
            snprintf(mappings[torus_idx].label, 32, "p³");
        } else if (p_pow == 0 && q_pow == 3) {
            snprintf(mappings[torus_idx].label, 32, "q³");
        } else if (p_pow == 2 && q_pow == 1) {
            snprintf(mappings[torus_idx].label, 32, "p²q");
        } else {
            snprintf(mappings[torus_idx].label, 32, "pq²");
        }
        
        printf("  Torus %2d: %s = %lu\n",
               torus_idx + 1, mappings[torus_idx].label, value);
        mappings[torus_idx].ring = -1;
        
        torus_idx++;
    }
    
    // Quaternary tori (5)
    printf("\nQUATERNARY TORI (5):\n");
    for (int i = 0; i < 5; i++) {
        int p_pow = (i == 0) ? 4 : (i == 1) ? 0 : (i == 2) ? 3 : (i == 3) ? 2 : 1;
        int q_pow = (i == 0) ? 0 : (i == 1) ? 4 : (i == 2) ? 1 : (i == 3) ? 2 : 3;
        uint64_t value = power_multiply(p, p_pow, q, q_pow);
        
        mappings[torus_idx].torus_id = torus_idx + 1;
        mappings[torus_idx].p_power = p_pow;
        mappings[torus_idx].q_power = q_pow;
        mappings[torus_idx].value = value;
        
        if (p_pow == 4 && q_pow == 0) {
            snprintf(mappings[torus_idx].label, 32, "p⁴");
        } else if (p_pow == 0 && q_pow == 4) {
            snprintf(mappings[torus_idx].label, 32, "q⁴");
        } else if (p_pow == 3 && q_pow == 1) {
            snprintf(mappings[torus_idx].label, 32, "p³q");
        } else if (p_pow == 2 && q_pow == 2) {
            snprintf(mappings[torus_idx].label, 32, "p²q²");
        } else {
            snprintf(mappings[torus_idx].label, 32, "pq³");
        }
        
        printf("  Torus %2d: %s = %lu\n",
               torus_idx + 1, mappings[torus_idx].label, value);
        mappings[torus_idx].ring = -1;
        
        torus_idx++;
    }
    
    // Quinary tori (6)
    printf("\nQUINARY TORI (6):\n");
    for (int i = 0; i < 6; i++) {
        int p_pow = (i == 0) ? 5 : (i == 1) ? 0 : (i == 2) ? 4 : (i == 3) ? 3 : (i == 4) ? 2 : 1;
        int q_pow = (i == 0) ? 0 : (i == 1) ? 5 : (i == 2) ? 1 : (i == 3) ? 2 : (i == 4) ? 3 : 4;
        uint64_t value = power_multiply(p, p_pow, q, q_pow);
        
        mappings[torus_idx].torus_id = torus_idx + 1;
        mappings[torus_idx].p_power = p_pow;
        mappings[torus_idx].q_power = q_pow;
        mappings[torus_idx].value = value;
        
        if (p_pow == 5 && q_pow == 0) {
            snprintf(mappings[torus_idx].label, 32, "p⁵");
        } else if (p_pow == 0 && q_pow == 5) {
            snprintf(mappings[torus_idx].label, 32, "q⁵");
        } else if (p_pow == 4 && q_pow == 1) {
            snprintf(mappings[torus_idx].label, 32, "p⁴q");
        } else if (p_pow == 3 && q_pow == 2) {
            snprintf(mappings[torus_idx].label, 32, "p³q²");
        } else if (p_pow == 2 && q_pow == 3) {
            snprintf(mappings[torus_idx].label, 32, "p²q³");
        } else {
            snprintf(mappings[torus_idx].label, 32, "pq⁴");
        }
        
        printf("  Torus %2d: %s = %lu\n",
               torus_idx + 1, mappings[torus_idx].label, value);
        mappings[torus_idx].ring = -1;
        
        torus_idx++;
    }
}

/**
 * Analyze hierarchical structure
 */
void analyze_hierarchical_structure(TorusClockMapping* mappings) {
    printf("\n\n=== Hierarchical Structure Analysis ===\n");
    
    printf("\nLevel Distribution:\n");
    printf("  Primary (p, q):                    2 tori\n");
    printf("  Secondary (p², q², pq):            3 tori\n");
    printf("  Tertiary (p³, q³, p²q, pq²):       4 tori\n");
    printf("  Quaternary (p⁴, q⁴, ...):          5 tori\n");
    printf("  Quinary (p⁵, q⁵, ...):             6 tori\n");
    printf("  Total:                             20 tori\n");
    
    printf("\nPattern: Each level adds one more torus\n");
    printf("  Level i has (i+1) tori\n");
    printf("  Sum = 2 + 3 + 4 + 5 + 6 = 20\n");
    
    printf("\nPrime Tori (mappable to clock):\n");
    int prime_count = 0;
    for (int i = 0; i < MAX_TORI; i++) {
        if (mappings[i].ring >= 0) {
            printf("  Torus %2d: %s = %lu -> Ring %d, Position %d\n",
                   mappings[i].torus_id, mappings[i].label, mappings[i].value,
                   mappings[i].ring, mappings[i].position);
            prime_count++;
        }
    }
    printf("  Total prime tori: %d/%d\n", prime_count, MAX_TORI);
    
    printf("\nComposite Tori (not directly mappable):\n");
    int composite_count = 0;
    for (int i = 0; i < MAX_TORI; i++) {
        if (mappings[i].ring < 0) {
            printf("  Torus %2d: %s = %lu (composite)\n",
                   mappings[i].torus_id, mappings[i].label, mappings[i].value);
            composite_count++;
        }
    }
    printf("  Total composite tori: %d/%d\n", composite_count, MAX_TORI);
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                                                            ║\n");
    printf("║  OBJECTIVE 28: Map 20 Tori to Clock Lattice               ║\n");
    printf("║                                                            ║\n");
    printf("║  Revised Phase 4: Visualize Complete Factorization        ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    // Test with p=2, q=5 (our case)
    uint64_t p = 2;
    uint64_t q = 5;
    
    TorusClockMapping mappings[MAX_TORI];
    memset(mappings, 0, sizeof(mappings));
    
    // Map all 20 tori
    map_20_tori_to_clock(p, q, mappings);
    
    // Analyze structure
    analyze_hierarchical_structure(mappings);
    
    printf("\n\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Summary                                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  Key Findings:\n");
    printf("  1. 20 tori form a hierarchical structure (2+3+4+5+6)\n");
    printf("  2. Only primary tori (p, q) map to clock positions\n");
    printf("  3. Composite tori (p²q, etc.) are products, not primes\n");
    printf("  4. Structure reveals complete pq factorization\n");
    printf("\n");
    printf("  Implications:\n");
    printf("  - Clock lattice shows prime factors (p, q)\n");
    printf("  - Torus structure shows all power combinations\n");
    printf("  - Hierarchical levels correspond to power sums\n");
    printf("  - This structure is intrinsic to n = p × q\n");
    printf("\n");
    printf("  Next Steps:\n");
    printf("  - Create trainable micro-model capturing this structure\n");
    printf("  - Test on real ECDSA samples\n");
    printf("  - Measure final performance\n");
    printf("\n");
    
    return 0;
}