/**
 * Number Theoretic Transform (NTT) Demonstration
 * 
 * Demonstrates NTT as described in Chapter 13 of the thesis.
 * NTT is like FFT but works in modular arithmetic, making it exact.
 * 
 * Key concepts:
 * - Modular arithmetic (no floating point errors)
 * - Primitive roots of unity
 * - Fast polynomial multiplication
 * - Connection to clock lattice (12-fold structure)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Use a prime modulus: p = 12289 = 3 * 2^12 + 1
// This allows NTT of size up to 2^12 = 4096
#define MOD 12289
#define PRIMITIVE_ROOT 11  // Primitive root of unity mod 12289

/**
 * Modular exponentiation: (base^exp) mod MOD
 */
uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    
    return result;
}

/**
 * Modular inverse using Fermat's little theorem: a^(p-1) = 1 (mod p)
 * So a^(-1) = a^(p-2) (mod p)
 */
uint64_t mod_inverse(uint64_t a, uint64_t mod) {
    return mod_pow(a, mod - 2, mod);
}

/**
 * Forward NTT (Cooley-Tukey algorithm)
 */
void ntt_forward(uint64_t *a, int n) {
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            uint64_t temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }
    
    // Cooley-Tukey decimation-in-time
    for (int len = 2; len <= n; len <<= 1) {
        // Find primitive root of unity for this level
        uint64_t w = mod_pow(PRIMITIVE_ROOT, (MOD - 1) / len, MOD);
        
        for (int i = 0; i < n; i += len) {
            uint64_t wn = 1;
            for (int j = 0; j < len / 2; j++) {
                uint64_t u = a[i + j];
                uint64_t v = (a[i + j + len / 2] * wn) % MOD;
                
                a[i + j] = (u + v) % MOD;
                a[i + j + len / 2] = (u - v + MOD) % MOD;
                
                wn = (wn * w) % MOD;
            }
        }
    }
}

/**
 * Inverse NTT
 */
void ntt_inverse(uint64_t *a, int n) {
    // Forward NTT with conjugate root
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        
        if (i < j) {
            uint64_t temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }
    
    for (int len = 2; len <= n; len <<= 1) {
        // Use inverse root
        uint64_t w = mod_pow(PRIMITIVE_ROOT, MOD - 1 - (MOD - 1) / len, MOD);
        
        for (int i = 0; i < n; i += len) {
            uint64_t wn = 1;
            for (int j = 0; j < len / 2; j++) {
                uint64_t u = a[i + j];
                uint64_t v = (a[i + j + len / 2] * wn) % MOD;
                
                a[i + j] = (u + v) % MOD;
                a[i + j + len / 2] = (u - v + MOD) % MOD;
                
                wn = (wn * w) % MOD;
            }
        }
    }
    
    // Divide by n
    uint64_t n_inv = mod_inverse(n, MOD);
    for (int i = 0; i < n; i++) {
        a[i] = (a[i] * n_inv) % MOD;
    }
}

/**
 * Polynomial multiplication using NTT
 */
void poly_multiply(uint64_t *a, uint64_t *b, uint64_t *result, int n) {
    // Forward NTT on both polynomials
    ntt_forward(a, n);
    ntt_forward(b, n);
    
    // Pointwise multiplication
    for (int i = 0; i < n; i++) {
        result[i] = (a[i] * b[i]) % MOD;
    }
    
    // Inverse NTT to get result
    ntt_inverse(result, n);
}

/**
 * Demonstrate NTT with simple example
 */
void demonstrate_ntt() {
    printf("=== NTT Demonstration ===\n\n");
    
    int n = 8;  // Must be power of 2
    
    // Example: multiply (1 + 2x + 3x^2) * (4 + 5x + 6x^2)
    // Expected result: 4 + 13x + 28x^2 + 27x^3 + 18x^4
    
    uint64_t a[8] = {1, 2, 3, 0, 0, 0, 0, 0};
    uint64_t b[8] = {4, 5, 6, 0, 0, 0, 0, 0};
    uint64_t result[8] = {0};
    
    printf("Polynomial A: 1 + 2x + 3x^2\n");
    printf("Polynomial B: 4 + 5x + 6x^2\n\n");
    
    // Make copies for NTT (it modifies input)
    uint64_t a_copy[8], b_copy[8];
    memcpy(a_copy, a, sizeof(a));
    memcpy(b_copy, b, sizeof(b));
    
    // Multiply using NTT
    poly_multiply(a_copy, b_copy, result, n);
    
    printf("Result (using NTT):\n");
    printf("Coefficients: ");
    for (int i = 0; i < n; i++) {
        if (result[i] != 0) {
            printf("%lu", result[i]);
            if (i > 0) printf("x^%d", i);
            if (i < n - 1 && result[i + 1] != 0) printf(" + ");
        }
    }
    printf("\n\n");
    
    printf("Expected: 4 + 13x + 28x^2 + 27x^3 + 18x^4\n");
    printf("Got:      %lu + %lux + %lux^2 + %lux^3 + %lux^4\n\n",
           result[0], result[1], result[2], result[3], result[4]);
}

/**
 * Demonstrate connection to clock lattice
 */
void demonstrate_clock_connection() {
    printf("=== Connection to Clock Lattice ===\n\n");
    
    printf("The 12-fold clock structure relates to NTT:\n\n");
    
    printf("1. Primitive Roots of Unity:\n");
    printf("   - In NTT, we use primitive n-th roots of unity\n");
    printf("   - In clock lattice, we have 12-fold symmetry\n");
    printf("   - Both use modular arithmetic\n\n");
    
    printf("2. Modular Arithmetic:\n");
    printf("   - NTT uses prime modulus (p = %d)\n", MOD);
    printf("   - Clock lattice uses mod 12\n");
    printf("   - Both avoid floating-point errors\n\n");
    
    printf("3. Fast Computation:\n");
    printf("   - NTT: O(n log n) polynomial multiplication\n");
    printf("   - Clock lattice: O(1) position lookup\n");
    printf("   - Both use geometric structure for efficiency\n\n");
    
    // Show 12th roots of unity mod 12289
    printf("4. 12th Roots of Unity (mod %d):\n", MOD);
    uint64_t root_12 = mod_pow(PRIMITIVE_ROOT, (MOD - 1) / 12, MOD);
    printf("   Primitive 12th root: %lu\n", root_12);
    printf("   Powers: ");
    uint64_t power = 1;
    for (int i = 0; i < 12; i++) {
        printf("%lu ", power);
        power = (power * root_12) % MOD;
    }
    printf("\n\n");
}

/**
 * Demonstrate exact arithmetic advantage
 */
void demonstrate_exact_arithmetic() {
    printf("=== Exact Arithmetic Advantage ===\n\n");
    
    printf("NTT vs FFT:\n\n");
    
    printf("FFT (Floating Point):\n");
    printf("  - Uses complex numbers\n");
    printf("  - Accumulates rounding errors\n");
    printf("  - Results are approximate\n");
    printf("  - Example: 1.0000000001 instead of 1\n\n");
    
    printf("NTT (Modular Arithmetic):\n");
    printf("  - Uses integers mod prime\n");
    printf("  - No rounding errors\n");
    printf("  - Results are exact\n");
    printf("  - Example: 1 is exactly 1\n\n");
    
    printf("This is crucial for:\n");
    printf("  - Cryptography (exact values required)\n");
    printf("  - Polynomial arithmetic (no error accumulation)\n");
    printf("  - Large computations (errors don't compound)\n\n");
}

/**
 * Performance comparison
 */
void demonstrate_performance() {
    printf("=== Performance Characteristics ===\n\n");
    
    printf("Complexity Analysis:\n\n");
    
    printf("Naive Polynomial Multiplication:\n");
    printf("  - Time: O(n^2)\n");
    printf("  - Space: O(n)\n");
    printf("  - Example: 1000 coefficients = 1,000,000 operations\n\n");
    
    printf("NTT Polynomial Multiplication:\n");
    printf("  - Time: O(n log n)\n");
    printf("  - Space: O(n)\n");
    printf("  - Example: 1000 coefficients ≈ 10,000 operations\n\n");
    
    printf("Speedup: ~100x for n=1000\n");
    printf("Speedup: ~1000x for n=10000\n\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        printf("NTT Demo - Custom mode not yet implemented\n");
        printf("Run without arguments for demonstration\n");
        return 1;
    }
    
    printf("=== Number Theoretic Transform (NTT) Demo ===\n\n");
    
    demonstrate_ntt();
    demonstrate_clock_connection();
    demonstrate_exact_arithmetic();
    demonstrate_performance();
    
    printf("=== Summary ===\n\n");
    printf("NTT provides:\n");
    printf("  ✓ Exact arithmetic (no rounding errors)\n");
    printf("  ✓ Fast computation (O(n log n))\n");
    printf("  ✓ Connection to clock lattice (12-fold symmetry)\n");
    printf("  ✓ Cryptographic applications (exact values)\n\n");
    
    return 0;
}