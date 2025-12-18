/**
 * Generate lookup table for first 232 primes
 * 
 * This tool generates a C array containing the first 232 primes
 * for use in the deterministic prime generation system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Simple sieve to generate primes
static bool is_prime_simple(uint64_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (uint64_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

int main(void) {
    const uint32_t TARGET_COUNT = 232;
    uint64_t primes[TARGET_COUNT];
    uint32_t count = 0;
    
    // Generate first 232 primes
    uint64_t candidate = 2;
    while (count < TARGET_COUNT) {
        if (is_prime_simple(candidate)) {
            primes[count++] = candidate;
        }
        candidate++;
    }
    
    // Print C array
    printf("/**\n");
    printf(" * Lookup table for first 232 primes\n");
    printf(" * \n");
    printf(" * Ring 0 (Hours):        Primes 1-12\n");
    printf(" * Ring 1 (Minutes):      Primes 13-72\n");
    printf(" * Ring 2 (Seconds):      Primes 73-132\n");
    printf(" * Ring 3 (Milliseconds): Primes 133-232\n");
    printf(" */\n");
    printf("static const uint64_t PRIME_LOOKUP_TABLE[232] = {\n");
    
    // Ring 0: 12 primes
    printf("    // Ring 0: Hours (12 primes)\n");
    printf("    ");
    for (uint32_t i = 0; i < 12; i++) {
        printf("%lu", primes[i]);
        if (i < 11) printf(", ");
    }
    printf(",\n\n");
    
    // Ring 1: 60 primes
    printf("    // Ring 1: Minutes (60 primes)\n");
    for (uint32_t i = 12; i < 72; i++) {
        if ((i - 12) % 10 == 0) printf("    ");
        printf("%lu", primes[i]);
        if (i < 71) printf(", ");
        if ((i - 12) % 10 == 9) printf("\n");
    }
    printf("\n");
    
    // Ring 2: 60 primes
    printf("    // Ring 2: Seconds (60 primes)\n");
    for (uint32_t i = 72; i < 132; i++) {
        if ((i - 72) % 10 == 0) printf("    ");
        printf("%lu", primes[i]);
        if (i < 131) printf(", ");
        if ((i - 72) % 10 == 9) printf("\n");
    }
    printf("\n");
    
    // Ring 3: 100 primes
    printf("    // Ring 3: Milliseconds (100 primes)\n");
    for (uint32_t i = 132; i < 232; i++) {
        if ((i - 132) % 10 == 0) printf("    ");
        printf("%lu", primes[i]);
        if (i < 231) printf(", ");
        if ((i - 132) % 10 == 9) printf("\n");
    }
    printf("\n};\n");
    
    // Print statistics
    fprintf(stderr, "\nGenerated %u primes\n", count);
    fprintf(stderr, "First prime: %lu\n", primes[0]);
    fprintf(stderr, "Last prime (232nd): %lu\n", primes[231]);
    fprintf(stderr, "Ring 0 ends at: %lu (12th prime)\n", primes[11]);
    fprintf(stderr, "Ring 1 ends at: %lu (72nd prime)\n", primes[71]);
    fprintf(stderr, "Ring 2 ends at: %lu (132nd prime)\n", primes[131]);
    
    return 0;
}