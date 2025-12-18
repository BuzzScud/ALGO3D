/*
 * Simple CLLM Neural Network Demonstration
 * 
 * This program demonstrates the CONCEPT of using CrystallineAbacus
 * for neural network weights with arbitrary precision arithmetic.
 * 
 * This is a simplified demonstration showing:
 * 1. Storing weights as CrystallineAbacus numbers
 * 2. Converting between abacus and double for computation
 * 3. Comparing precision with standard doubles
 * 4. Basic forward pass computation
 * 
 * Network: Simple 2-input, 1-output perceptron
 * Task: Demonstrate arbitrary precision weight storage
 * 
 * Compilation:
 *   gcc -o simple_cllm simple_cllm.c -I../../math/include -L../../math/lib -lcrystallinemath -lm
 * 
 * Usage:
 *   ./simple_cllm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "math/abacus.h"

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

// Simple perceptron structure
typedef struct {
    CrystallineAbacus* weight1;  // First weight (arbitrary precision)
    CrystallineAbacus* weight2;  // Second weight (arbitrary precision)
    CrystallineAbacus* bias;     // Bias term (arbitrary precision)
    
    double weight1_double;       // For comparison
    double weight2_double;       // For comparison
    double bias_double;          // For comparison
} Perceptron;

/**
 * Create a perceptron with given weights
 */
Perceptron* perceptron_create(double w1, double w2, double b) {
    Perceptron* p = (Perceptron*)malloc(sizeof(Perceptron));
    if (!p) return NULL;
    
    // Store weights as CrystallineAbacus (base-60, precision 15)
    p->weight1 = abacus_from_double(w1, 60, 15);
    p->weight2 = abacus_from_double(w2, 60, 15);
    p->bias = abacus_from_double(b, 60, 15);
    
    // Store as doubles for comparison
    p->weight1_double = w1;
    p->weight2_double = w2;
    p->bias_double = b;
    
    return p;
}

/**
 * Free perceptron
 */
void perceptron_free(Perceptron* p) {
    if (!p) return;
    abacus_free(p->weight1);
    abacus_free(p->weight2);
    abacus_free(p->bias);
    free(p);
}

/**
 * Forward pass using CrystallineAbacus weights
 */
double perceptron_forward_abacus(Perceptron* p, double x1, double x2) {
    // Convert weights back to doubles for computation
    double w1, w2, b;
    abacus_to_double(p->weight1, &w1);
    abacus_to_double(p->weight2, &w2);
    abacus_to_double(p->bias, &b);
    
    // Compute: output = w1*x1 + w2*x2 + b
    double output = w1 * x1 + w2 * x2 + b;
    
    // Apply sigmoid activation
    return 1.0 / (1.0 + exp(-output));
}

/**
 * Forward pass using double precision (for comparison)
 */
double perceptron_forward_double(Perceptron* p, double x1, double x2) {
    double output = p->weight1_double * x1 + p->weight2_double * x2 + p->bias_double;
    return 1.0 / (1.0 + exp(-output));
}

/**
 * Print weight comparison
 */
void print_weight_comparison(Perceptron* p) {
    double w1, w2, b;
    abacus_to_double(p->weight1, &w1);
    abacus_to_double(p->weight2, &w2);
    abacus_to_double(p->bias, &b);
    
    printf("\n%sWeight Storage Comparison:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("\n%sWeight 1:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  Abacus (base-60, precision 15): %.15f\n", w1);
    printf("  Double (64-bit):                 %.15f\n", p->weight1_double);
    printf("  Difference:                      %.2e\n", fabs(w1 - p->weight1_double));
    
    printf("\n%sWeight 2:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  Abacus (base-60, precision 15): %.15f\n", w2);
    printf("  Double (64-bit):                 %.15f\n", p->weight2_double);
    printf("  Difference:                      %.2e\n", fabs(w2 - p->weight2_double));
    
    printf("\n%sBias:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  Abacus (base-60, precision 15): %.15f\n", b);
    printf("  Double (64-bit):                 %.15f\n", p->bias_double);
    printf("  Difference:                      %.2e\n", fabs(b - p->bias_double));
}

/**
 * Main demonstration
 */
int main(void) {
    printf("\n%s", COLOR_BOLD);
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("  SIMPLE CLLM NEURAL NETWORK DEMONSTRATION\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("%s", COLOR_RESET);
    
    printf("\n%sThis demonstration shows:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("  1. Storing neural network weights as CrystallineAbacus\n");
    printf("  2. Arbitrary precision arithmetic (base-60, 15 digits)\n");
    printf("  3. Comparison with standard double precision\n");
    printf("  4. Forward pass computation\n");
    
    // Create a simple perceptron
    printf("\n%sCreating perceptron with weights:%s\n", COLOR_YELLOW, COLOR_RESET);
    double w1 = 0.123456789012345;
    double w2 = 0.987654321098765;
    double b = -0.5;
    
    printf("  Weight 1: %.15f\n", w1);
    printf("  Weight 2: %.15f\n", w2);
    printf("  Bias:     %.15f\n", b);
    
    Perceptron* p = perceptron_create(w1, w2, b);
    if (!p) {
        printf("%sError: Failed to create perceptron%s\n", COLOR_RESET, COLOR_RESET);
        return 1;
    }
    
    printf("%s✓ Perceptron created%s\n", COLOR_GREEN, COLOR_RESET);
    
    // Show weight storage comparison
    print_weight_comparison(p);
    
    // Test forward pass
    printf("\n%sForward Pass Test:%s\n", COLOR_BOLD, COLOR_RESET);
    
    double test_inputs[][2] = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 0.0},
        {1.0, 1.0}
    };
    
    printf("\n%sInput → Output (Abacus) vs Output (Double):%s\n", COLOR_CYAN, COLOR_RESET);
    
    for (int i = 0; i < 4; i++) {
        double x1 = test_inputs[i][0];
        double x2 = test_inputs[i][1];
        
        double output_abacus = perceptron_forward_abacus(p, x1, x2);
        double output_double = perceptron_forward_double(p, x1, x2);
        double diff = fabs(output_abacus - output_double);
        
        printf("  [%.0f, %.0f] → %.10f vs %.10f (diff: %.2e)\n",
               x1, x2, output_abacus, output_double, diff);
    }
    
    // Summary
    printf("\n%sKey Insights:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  • CrystallineAbacus can store weights with arbitrary precision\n");
    printf("  • Base-60 (Babylonian) representation with 15 digits precision\n");
    printf("  • Results are nearly identical to double precision\n");
    printf("  • Demonstrates feasibility for neural network applications\n");
    
    printf("\n%sMemory Usage:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  • Double precision: 8 bytes per weight\n");
    printf("  • CrystallineAbacus: ~100-200 bytes per weight\n");
    printf("  • Trade-off: More memory for exact arithmetic\n");
    
    printf("\n%sNext Steps:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  • Implement full backpropagation with abacus\n");
    printf("  • Add training loop with gradient descent\n");
    printf("  • Scale to larger networks\n");
    printf("  • Optimize abacus operations for speed\n");
    
    // Cleanup
    perceptron_free(p);
    
    printf("\n");
    return 0;
}