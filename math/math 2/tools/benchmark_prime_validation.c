/**
 * Comprehensive Prime Validation Benchmark Suite
 * 
 * Tests the deterministic clock lattice prime validation system across:
 * 1. Small primes (2 - 10,000)
 * 2. Medium primes (10K - 1M)
 * 3. Large primes (1M - 1B)
 * 4. Extremely large primes (Mersenne primes, etc.)
 * 5. Composite numbers (false positive testing)
 * 
 * Validates OBJECTIVE 22 Phase 2: Pure deterministic formula discovery
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "clock_lattice.h"
#include "crystal_abacus.h"

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// Test statistics
typedef struct {
    uint64_t total_tests;
    uint64_t passed;
    uint64_t failed;
    double total_time_ns;
    double min_time_ns;
    double max_time_ns;
} TestStats;

// Known small primes for validation (first 1000 primes)
static const uint64_t KNOWN_SMALL_PRIMES[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
    239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
    331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
    421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503,
    509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
    613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701,
    709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811,
    821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911,
    919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
    1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091,
    1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181,
    1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277
};
static const size_t NUM_KNOWN_SMALL_PRIMES = sizeof(KNOWN_SMALL_PRIMES) / sizeof(KNOWN_SMALL_PRIMES[0]);

// Known Mersenne primes (2^p - 1)
typedef struct {
    uint32_t exponent;
    const char* value_str;
} MersennePrime;

static const MersennePrime KNOWN_MERSENNE_PRIMES[] = {
    {31, "2147483647"},
    {61, "2305843009213693951"},
    // Note: Larger Mersenne primes exceed uint64_t range
};
static const size_t NUM_MERSENNE_PRIMES = sizeof(KNOWN_MERSENNE_PRIMES) / sizeof(KNOWN_MERSENNE_PRIMES[0]);

// Known composite numbers that might fool weak tests
static const uint64_t KNOWN_COMPOSITES[] = {
    4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30,
    // Carmichael numbers (pseudoprimes)
    561, 1105, 1729, 2465, 2821, 6601, 8911, 10585, 15841, 29341,
    // Strong pseudoprimes
    2047, 3277, 4033, 4681, 8321,
    // Perfect squares of primes
    121, 169, 289, 361, 529, 841, 961, 1369, 1681, 1849
};
static const size_t NUM_KNOWN_COMPOSITES = sizeof(KNOWN_COMPOSITES) / sizeof(KNOWN_COMPOSITES[0]);

/**
 * Get high-resolution timestamp in nanoseconds
 */
static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Simple trial division for comparison (O(sqrt(n)))
 */
static bool trial_division_is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

/**
 * Initialize test statistics
 */
static void init_stats(TestStats* stats) {
    stats->total_tests = 0;
    stats->passed = 0;
    stats->failed = 0;
    stats->total_time_ns = 0.0;
    stats->min_time_ns = 1e20;
    stats->max_time_ns = 0.0;
}

/**
 * Update test statistics
 */
static void update_stats(TestStats* stats, bool passed, double time_ns) {
    stats->total_tests++;
    if (passed) {
        stats->passed++;
    } else {
        stats->failed++;
    }
    stats->total_time_ns += time_ns;
    if (time_ns < stats->min_time_ns) stats->min_time_ns = time_ns;
    if (time_ns > stats->max_time_ns) stats->max_time_ns = time_ns;
}

/**
 * Print test statistics
 */
static void print_stats(const char* test_name, const TestStats* stats) {
    printf("\n%s=== %s ===%s\n", COLOR_BOLD, test_name, COLOR_RESET);
    printf("Total Tests: %lu\n", stats->total_tests);
    printf("Passed: %s%lu%s\n", COLOR_GREEN, stats->passed, COLOR_RESET);
    printf("Failed: %s%lu%s\n", stats->failed > 0 ? COLOR_RED : COLOR_GREEN, stats->failed, COLOR_RESET);
    
    if (stats->total_tests > 0) {
        double avg_time_ns = stats->total_time_ns / (double)stats->total_tests;
        printf("Average Time: %.2f ns\n", avg_time_ns);
        printf("Min Time: %.2f ns\n", stats->min_time_ns);
        printf("Max Time: %.2f ns\n", stats->max_time_ns);
        printf("Success Rate: %.2f%%\n", 100.0 * stats->passed / stats->total_tests);
    }
}

/**
 * Test 1.1: Small Prime Validation (2 - 10,000)
 */
static void test_small_primes(void) {
    printf("\n%s%s[TEST 1.1] Small Prime Validation (2 - 10,000)%s%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET, COLOR_RESET);
    
    TestStats stats;
    init_stats(&stats);
    
    // Test known small primes
    printf("Testing %zu known small primes...\n", NUM_KNOWN_SMALL_PRIMES);
    for (size_t i = 0; i < NUM_KNOWN_SMALL_PRIMES; i++) {
        uint64_t prime = KNOWN_SMALL_PRIMES[i];
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(prime);
        uint64_t end = get_time_ns();
        
        bool passed = result;
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu should be prime but was rejected%s\n", 
                   COLOR_RED, prime, COLOR_RESET);
        }
    }
    
    // Test all numbers from 2 to 10,000
    printf("Testing all numbers from 2 to 10,000...\n");
    for (uint64_t n = 2; n <= 10000; n++) {
        bool expected = trial_division_is_prime(n);
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(n);
        uint64_t end = get_time_ns();
        
        bool passed = (result == expected);
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu - expected %s, got %s%s\n", 
                   COLOR_RED, n, expected ? "prime" : "composite",
                   result ? "prime" : "composite", COLOR_RESET);
        }
    }
    
    print_stats("Small Prime Validation Results", &stats);
}

/**
 * Test 1.2: Medium Prime Validation (10,000 - 1,000,000)
 */
static void test_medium_primes(void) {
    printf("\n%s%s[TEST 1.2] Medium Prime Validation (10,000 - 1,000,000)%s%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET, COLOR_RESET);
    
    TestStats stats;
    init_stats(&stats);
    
    // Test sample of 10,000 random numbers in this range
    printf("Testing 10,000 random numbers in range...\n");
    srand(time(NULL));
    
    for (int i = 0; i < 10000; i++) {
        uint64_t n = 10000 + (rand() % 990000);
        bool expected = trial_division_is_prime(n);
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(n);
        uint64_t end = get_time_ns();
        
        bool passed = (result == expected);
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu - expected %s, got %s%s\n", 
                   COLOR_RED, n, expected ? "prime" : "composite",
                   result ? "prime" : "composite", COLOR_RESET);
        }
    }
    
    print_stats("Medium Prime Validation Results", &stats);
}

/**
 * Test 1.3: Large Prime Validation (1M - 1B)
 */
static void test_large_primes(void) {
    printf("\n%s%s[TEST 1.3] Large Prime Validation (1M - 1B)%s%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET, COLOR_RESET);
    
    TestStats stats;
    init_stats(&stats);
    
    // Test sample of 1,000 random numbers in this range
    printf("Testing 1,000 random numbers in range...\n");
    
    for (int i = 0; i < 1000; i++) {
        uint64_t n = 1000000 + (rand() % 999000000);
        bool expected = trial_division_is_prime(n);
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(n);
        uint64_t end = get_time_ns();
        
        bool passed = (result == expected);
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu - expected %s, got %s%s\n", 
                   COLOR_RED, n, expected ? "prime" : "composite",
                   result ? "prime" : "composite", COLOR_RESET);
        }
    }
    
    print_stats("Large Prime Validation Results", &stats);
}

/**
 * Test 1.4: Extremely Large Prime Validation (Mersenne primes)
 */
static void test_extremely_large_primes(void) {
    printf("\n%s%s[TEST 1.4] Extremely Large Prime Validation%s%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET, COLOR_RESET);
    
    TestStats stats;
    init_stats(&stats);
    
    // Test Mersenne primes that fit in uint64_t
    printf("Testing Mersenne primes...\n");
    for (size_t i = 0; i < NUM_MERSENNE_PRIMES; i++) {
        uint64_t mersenne = (1ULL << KNOWN_MERSENNE_PRIMES[i].exponent) - 1;
        
        printf("  Testing 2^%u - 1 = %lu...\n", 
               KNOWN_MERSENNE_PRIMES[i].exponent, mersenne);
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(mersenne);
        uint64_t end = get_time_ns();
        
        bool passed = result;
        update_stats(&stats, passed, (double)(end - start));
        
        if (passed) {
            printf("%s    PASS: Correctly identified as prime (%.2f ns)%s\n", 
                   COLOR_GREEN, (double)(end - start), COLOR_RESET);
        } else {
            printf("%s    FAIL: Should be prime but was rejected%s\n", 
                   COLOR_RED, COLOR_RESET);
        }
    }
    
    // Test some known large primes
    uint64_t large_primes[] = {
        999999999989ULL,  // Largest prime < 10^12
        1000000007ULL,    // Common large prime
        2147483647ULL,    // 2^31 - 1 (Mersenne prime)
    };
    
    printf("\nTesting other large primes...\n");
    for (size_t i = 0; i < sizeof(large_primes) / sizeof(large_primes[0]); i++) {
        uint64_t prime = large_primes[i];
        
        printf("  Testing %lu...\n", prime);
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(prime);
        uint64_t end = get_time_ns();
        
        bool passed = result;
        update_stats(&stats, passed, (double)(end - start));
        
        if (passed) {
            printf("%s    PASS: Correctly identified as prime (%.2f ns)%s\n", 
                   COLOR_GREEN, (double)(end - start), COLOR_RESET);
        } else {
            printf("%s    FAIL: Should be prime but was rejected%s\n", 
                   COLOR_RED, COLOR_RESET);
        }
    }
    
    print_stats("Extremely Large Prime Validation Results", &stats);
}

/**
 * Test 1.5: Composite Number Validation (False Positive Testing)
 */
static void test_composite_numbers(void) {
    printf("\n%s%s[TEST 1.5] Composite Number Validation (False Positive Testing)%s%s\n", 
           COLOR_BOLD, COLOR_CYAN, COLOR_RESET, COLOR_RESET);
    
    TestStats stats;
    init_stats(&stats);
    
    // Test known composite numbers
    printf("Testing %zu known composite numbers...\n", NUM_KNOWN_COMPOSITES);
    for (size_t i = 0; i < NUM_KNOWN_COMPOSITES; i++) {
        uint64_t composite = KNOWN_COMPOSITES[i];
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(composite);
        uint64_t end = get_time_ns();
        
        bool passed = !result;  // Should be rejected
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu is composite but was accepted as prime%s\n", 
                   COLOR_RED, composite, COLOR_RESET);
        }
    }
    
    // Test random composite numbers
    printf("Testing 10,000 random composite numbers...\n");
    for (int i = 0; i < 10000; i++) {
        // Generate composite by multiplying two random numbers
        uint64_t a = 2 + (rand() % 1000);
        uint64_t b = 2 + (rand() % 1000);
        uint64_t composite = a * b;
        
        uint64_t start = get_time_ns();
        bool result = validate_prime_by_clock_position(composite);
        uint64_t end = get_time_ns();
        
        bool passed = !result;  // Should be rejected
        update_stats(&stats, passed, (double)(end - start));
        
        if (!passed) {
            printf("%s  FAIL: %lu = %lu * %lu is composite but was accepted as prime%s\n", 
                   COLOR_RED, composite, a, b, COLOR_RESET);
        }
    }
    
    print_stats("Composite Number Validation Results", &stats);
}

/**
 * Performance comparison: Clock lattice vs Trial division
 */
static void test_performance_comparison(void) {
    printf("\n%s%s[PERFORMANCE] Clock Lattice vs Trial Division%s%s\n", 
           COLOR_BOLD, COLOR_MAGENTA, COLOR_RESET, COLOR_RESET);
    
    uint64_t test_numbers[] = {
        1009, 10007, 100003, 1000003, 10000019, 100000007, 1000000007ULL
    };
    
    printf("\n%-15s %-20s %-20s %-10s\n", "Number", "Clock Lattice (ns)", "Trial Division (ns)", "Speedup");
    printf("%-15s %-20s %-20s %-10s\n", "------", "-----------------", "------------------", "-------");
    
    for (size_t i = 0; i < sizeof(test_numbers) / sizeof(test_numbers[0]); i++) {
        uint64_t n = test_numbers[i];
        
        // Test clock lattice
        uint64_t start_clock = get_time_ns();
        bool result_clock = validate_prime_by_clock_position(n);
        uint64_t end_clock = get_time_ns();
        double time_clock = (double)(end_clock - start_clock);
        
        // Test trial division
        uint64_t start_trial = get_time_ns();
        bool result_trial = trial_division_is_prime(n);
        uint64_t end_trial = get_time_ns();
        double time_trial = (double)(end_trial - start_trial);
        
        double speedup = time_trial / time_clock;
        
        printf("%-15lu %-20.2f %-20.2f %s%.2fx%s\n", 
               n, time_clock, time_trial, 
               speedup > 1.0 ? COLOR_GREEN : COLOR_RED,
               speedup, COLOR_RESET);
        
        if (result_clock != result_trial) {
            printf("%s  WARNING: Results differ! Clock=%s, Trial=%s%s\n",
                   COLOR_YELLOW, result_clock ? "prime" : "composite",
                   result_trial ? "prime" : "composite", COLOR_RESET);
        }
    }
}

/**
 * Main benchmark suite
 */
int main(int argc, char** argv) {
    printf("\n%s%s╔════════════════════════════════════════════════════════════╗%s\n", 
           COLOR_BOLD, COLOR_BLUE, COLOR_RESET);
    printf("%s%s║  COMPREHENSIVE PRIME VALIDATION BENCHMARK SUITE            ║%s\n", 
           COLOR_BOLD, COLOR_BLUE, COLOR_RESET);
    printf("%s%s║  Testing Deterministic Clock Lattice Prime Validation      ║%s\n", 
           COLOR_BOLD, COLOR_BLUE, COLOR_RESET);
    printf("%s%s╚════════════════════════════════════════════════════════════╝%s\n", 
           COLOR_BOLD, COLOR_BLUE, COLOR_RESET);
    
    bool run_all = (argc == 1);
    bool run_small = run_all;
    bool run_medium = run_all;
    bool run_large = run_all;
    bool run_xlarge = run_all;
    bool run_composite = run_all;
    bool run_performance = run_all;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--small") == 0) run_small = true;
        else if (strcmp(argv[i], "--medium") == 0) run_medium = true;
        else if (strcmp(argv[i], "--large") == 0) run_large = true;
        else if (strcmp(argv[i], "--xlarge") == 0) run_xlarge = true;
        else if (strcmp(argv[i], "--composite") == 0) run_composite = true;
        else if (strcmp(argv[i], "--performance") == 0) run_performance = true;
        else if (strcmp(argv[i], "--help") == 0) {
            printf("\nUsage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --small       Run small prime tests (2 - 10,000)\n");
            printf("  --medium      Run medium prime tests (10K - 1M)\n");
            printf("  --large       Run large prime tests (1M - 1B)\n");
            printf("  --xlarge      Run extremely large prime tests (Mersenne primes)\n");
            printf("  --composite   Run composite number tests (false positive testing)\n");
            printf("  --performance Run performance comparison tests\n");
            printf("  --help        Show this help message\n");
            printf("\nIf no options specified, all tests will run.\n\n");
            return 0;
        }
    }
    
    uint64_t total_start = get_time_ns();
    
    if (run_small) test_small_primes();
    if (run_medium) test_medium_primes();
    if (run_large) test_large_primes();
    if (run_xlarge) test_extremely_large_primes();
    if (run_composite) test_composite_numbers();
    if (run_performance) test_performance_comparison();
    
    uint64_t total_end = get_time_ns();
    double total_time_s = (double)(total_end - total_start) / 1e9;
    
    printf("\n%s%s╔════════════════════════════════════════════════════════════╗%s\n", 
           COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    printf("%s%s║  BENCHMARK SUITE COMPLETE                                  ║%s\n", 
           COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    printf("%s%s║  Total Time: %-44.2f║%s\n", 
           COLOR_BOLD, COLOR_GREEN, total_time_s, COLOR_RESET);
    printf("%s%s╚════════════════════════════════════════════════════════════╝%s\n\n", 
           COLOR_BOLD, COLOR_GREEN, COLOR_RESET);
    
    return 0;
}