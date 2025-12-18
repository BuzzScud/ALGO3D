/**
 * Simple Crystalline Abacus Demonstration
 * 
 * Demonstrates the basic structure and operations of the CrystallineAbacus
 * as described in Chapter 6 of the thesis.
 * 
 * This is a simplified version showing the core concepts:
 * - Base-60 representation
 * - Bead structure with weight exponents
 * - Sparse representation (only non-zero beads)
 * - Basic arithmetic operations
 */

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_BEADS 20
#define BASE 60

/**
 * A single bead on the abacus
 */
typedef struct {
    int value;           // 0-59 for base-60
    int weight_exponent; // Power of 60 (e.g., 0 for 60^0, 1 for 60^1, -1 for 60^-1)
} Bead;

/**
 * The Crystalline Abacus structure
 */
typedef struct {
    Bead beads[MAX_BEADS];
    int num_beads;
    int base;
    int precision;  // Number of fractional positions
} CrystallineAbacus;

/**
 * Create a new abacus
 */
CrystallineAbacus abacus_create(int base, int precision) {
    CrystallineAbacus abacus = {0};
    abacus.base = base;
    abacus.precision = precision;
    abacus.num_beads = 0;
    return abacus;
}

/**
 * Add a bead to the abacus (sparse representation)
 */
void abacus_add_bead(CrystallineAbacus *abacus, int value, int weight_exponent) {
    if (value == 0) return; // Don't store zero beads (sparse)
    if (abacus->num_beads >= MAX_BEADS) return;
    
    abacus->beads[abacus->num_beads].value = value;
    abacus->beads[abacus->num_beads].weight_exponent = weight_exponent;
    abacus->num_beads++;
}

/**
 * Convert double to abacus representation
 */
CrystallineAbacus abacus_from_double(double value, int base, int precision) {
    CrystallineAbacus abacus = abacus_create(base, precision);
    
    // Handle integer part
    long long integer_part = (long long)fabs(value);
    int exponent = 0;
    
    while (integer_part > 0) {
        int digit = integer_part % base;
        if (digit != 0) {
            abacus_add_bead(&abacus, digit, exponent);
        }
        integer_part /= base;
        exponent++;
    }
    
    // Handle fractional part
    double fractional_part = fabs(value) - floor(fabs(value));
    exponent = -1;
    
    for (int i = 0; i < precision && fractional_part > 0; i++) {
        fractional_part *= base;
        int digit = (int)fractional_part;
        if (digit != 0) {
            abacus_add_bead(&abacus, digit, exponent);
        }
        fractional_part -= digit;
        exponent--;
    }
    
    return abacus;
}

/**
 * Convert abacus to double
 */
double abacus_to_double(CrystallineAbacus *abacus) {
    double result = 0.0;
    
    for (int i = 0; i < abacus->num_beads; i++) {
        double weight = pow(abacus->base, abacus->beads[i].weight_exponent);
        result += abacus->beads[i].value * weight;
    }
    
    return result;
}

/**
 * Print abacus in human-readable form
 */
void abacus_print(CrystallineAbacus *abacus) {
    printf("CrystallineAbacus (base=%d, precision=%d, beads=%d):\n", 
           abacus->base, abacus->precision, abacus->num_beads);
    
    if (abacus->num_beads == 0) {
        printf("  [empty - represents 0]\n");
        return;
    }
    
    // Sort beads by weight (descending)
    for (int i = 0; i < abacus->num_beads - 1; i++) {
        for (int j = i + 1; j < abacus->num_beads; j++) {
            if (abacus->beads[i].weight_exponent < abacus->beads[j].weight_exponent) {
                Bead temp = abacus->beads[i];
                abacus->beads[i] = abacus->beads[j];
                abacus->beads[j] = temp;
            }
        }
    }
    
    // Print each bead
    for (int i = 0; i < abacus->num_beads; i++) {
        Bead *bead = &abacus->beads[i];
        printf("  Bead %d: value=%2d, weight=%d^%d", 
               i, bead->value, abacus->base, bead->weight_exponent);
        
        double contribution = bead->value * pow(abacus->base, bead->weight_exponent);
        printf(" (contributes %.10f)\n", contribution);
    }
    
    printf("  Total value: %.15f\n", abacus_to_double(abacus));
}

/**
 * Add two abacus numbers
 */
CrystallineAbacus abacus_add(CrystallineAbacus *a, CrystallineAbacus *b) {
    // Convert to double, add, convert back
    // (Simplified version - full version would work directly with beads)
    double sum = abacus_to_double(a) + abacus_to_double(b);
    int precision = (a->precision > b->precision) ? a->precision : b->precision;
    return abacus_from_double(sum, a->base, precision);
}

/**
 * Multiply two abacus numbers
 */
CrystallineAbacus abacus_multiply(CrystallineAbacus *a, CrystallineAbacus *b) {
    // Convert to double, multiply, convert back
    // (Simplified version - full version would work directly with beads)
    double product = abacus_to_double(a) * abacus_to_double(b);
    int precision = a->precision + b->precision;
    return abacus_from_double(product, a->base, precision);
}

/**
 * Demonstrate sparse representation advantage
 */
void demonstrate_sparse_representation() {
    printf("=== Sparse Representation Advantage ===\n\n");
    
    // Large number with few non-zero digits
    double value = 3600.0; // 1,0,0 in base-60 (only 1 non-zero digit)
    
    printf("Value: %.0f\n", value);
    printf("In base-60: 1,0,0 (1×60² + 0×60¹ + 0×60⁰)\n\n");
    
    CrystallineAbacus abacus = abacus_from_double(value, 60, 0);
    abacus_print(&abacus);
    
    printf("\nSparse representation stores only 1 bead instead of 3!\n");
    printf("Memory saved: %d%% (stored 1 bead instead of 3)\n\n", 
           (int)((2.0/3.0) * 100));
}

/**
 * Demonstrate arbitrary precision
 */
void demonstrate_arbitrary_precision() {
    printf("=== Arbitrary Precision ===\n\n");
    
    double pi = M_PI;
    
    printf("Pi with different precisions:\n\n");
    
    for (int precision = 2; precision <= 8; precision += 2) {
        printf("Precision %d:\n", precision);
        CrystallineAbacus abacus = abacus_from_double(pi, 60, precision);
        abacus_print(&abacus);
        
        double reconstructed = abacus_to_double(&abacus);
        double error = fabs(pi - reconstructed);
        printf("  Error: %.15e\n\n", error);
    }
}

/**
 * Demonstrate arithmetic operations
 */
void demonstrate_arithmetic() {
    printf("=== Arithmetic Operations ===\n\n");
    
    // Addition
    printf("Addition: 1.5 + 2.25 = 3.75\n\n");
    
    CrystallineAbacus a = abacus_from_double(1.5, 60, 2);
    printf("a = 1.5:\n");
    abacus_print(&a);
    printf("\n");
    
    CrystallineAbacus b = abacus_from_double(2.25, 60, 2);
    printf("b = 2.25:\n");
    abacus_print(&b);
    printf("\n");
    
    CrystallineAbacus sum = abacus_add(&a, &b);
    printf("sum = a + b:\n");
    abacus_print(&sum);
    printf("\n");
    
    // Multiplication
    printf("Multiplication: 2.0 × 3.0 = 6.0\n\n");
    
    CrystallineAbacus c = abacus_from_double(2.0, 60, 2);
    printf("c = 2.0:\n");
    abacus_print(&c);
    printf("\n");
    
    CrystallineAbacus d = abacus_from_double(3.0, 60, 2);
    printf("d = 3.0:\n");
    abacus_print(&d);
    printf("\n");
    
    CrystallineAbacus product = abacus_multiply(&c, &d);
    printf("product = c × d:\n");
    abacus_print(&product);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Convert command-line argument
        double value = atof(argv[1]);
        int precision = (argc > 2) ? atoi(argv[2]) : 4;
        
        printf("Converting %.15f to CrystallineAbacus (base=60, precision=%d)\n\n", 
               value, precision);
        
        CrystallineAbacus abacus = abacus_from_double(value, 60, precision);
        abacus_print(&abacus);
    } else {
        // Run demonstrations
        printf("=== CrystallineAbacus Demonstration ===\n\n");
        
        demonstrate_sparse_representation();
        printf("\n");
        
        demonstrate_arbitrary_precision();
        printf("\n");
        
        demonstrate_arithmetic();
    }
    
    return 0;
}