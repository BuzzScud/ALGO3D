/**
 * ecdsa_test_generator.c - Real ECDSA Test Data Generator
 * 
 * Generates actual ECDSA signatures and tracks k values for testing
 * the geometric recovery algorithm with REAL cryptographic data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

typedef struct {
    EC_KEY* key;
    BIGNUM* private_key;  // This is our "known k" for testing
    EC_POINT* public_key;
    
    // Message and signature
    unsigned char message_hash[32];
    ECDSA_SIG* signature;
    BIGNUM* r;
    BIGNUM* s;
    
    // For recovery testing
    BIGNUM* recovered_k;
    bool recovery_success;
    double recovery_time;
    
} ECDSATestCase;

/**
 * Generate a random message hash
 */
void generate_random_hash(unsigned char* hash, size_t len) {
    for (size_t i = 0; i < len; i++) {
        hash[i] = rand() % 256;
    }
}

/**
 * Create ECDSA test case with known k
 */
ECDSATestCase* create_ecdsa_test_case(int curve_nid, const BIGNUM* known_k) {
    ECDSATestCase* test = calloc(1, sizeof(ECDSATestCase));
    if (!test) return NULL;
    
    // Create EC key
    test->key = EC_KEY_new_by_curve_name(curve_nid);
    if (!test->key) {
        free(test);
        return NULL;
    }
    
    const EC_GROUP* group = EC_KEY_get0_group(test->key);
    
    // Set private key (this is our known k for testing)
    test->private_key = BN_dup(known_k);
    if (!EC_KEY_set_private_key(test->key, test->private_key)) {
        BN_free(test->private_key);
        EC_KEY_free(test->key);
        free(test);
        return NULL;
    }
    
    // Compute public key: Q = k*G
    test->public_key = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    
    if (!EC_POINT_mul(group, test->public_key, test->private_key, NULL, NULL, ctx)) {
        BN_CTX_free(ctx);
        EC_POINT_free(test->public_key);
        BN_free(test->private_key);
        EC_KEY_free(test->key);
        free(test);
        return NULL;
    }
    
    if (!EC_KEY_set_public_key(test->key, test->public_key)) {
        BN_CTX_free(ctx);
        EC_POINT_free(test->public_key);
        BN_free(test->private_key);
        EC_KEY_free(test->key);
        free(test);
        return NULL;
    }
    
    BN_CTX_free(ctx);
    
    // Generate random message hash
    generate_random_hash(test->message_hash, 32);
    
    // Sign the message
    test->signature = ECDSA_do_sign(test->message_hash, 32, test->key);
    if (!test->signature) {
        EC_POINT_free(test->public_key);
        BN_free(test->private_key);
        EC_KEY_free(test->key);
        free(test);
        return NULL;
    }
    
    // Extract r and s
    test->r = BN_dup(ECDSA_SIG_get0_r(test->signature));
    test->s = BN_dup(ECDSA_SIG_get0_s(test->signature));
    
    return test;
}

/**
 * Create ECDSA test case with random k
 */
ECDSATestCase* create_random_ecdsa_test_case(int curve_nid, uint32_t bit_length) {
    // Generate random k with specified bit length
    BIGNUM* k = BN_new();
    if (!BN_rand(k, bit_length, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY)) {
        BN_free(k);
        return NULL;
    }
    
    // Ensure it's actually the right bit length (set MSB)
    BN_set_bit(k, bit_length - 1);
    
    // Create test case
    ECDSATestCase* test = create_ecdsa_test_case(curve_nid, k);
    BN_free(k);
    
    return test;
}

/**
 * Verify ECDSA signature
 */
bool verify_ecdsa_signature(const ECDSATestCase* test) {
    if (!test || !test->key || !test->signature) return false;
    
    int result = ECDSA_do_verify(test->message_hash, 32, test->signature, test->key);
    return (result == 1);
}

/**
 * Print test case information
 */
void print_ecdsa_test_case(const ECDSATestCase* test) {
    if (!test) return;
    
    printf("\n=== ECDSA Test Case ===\n");
    
    // Print private key (known k)
    char* k_hex = BN_bn2hex(test->private_key);
    printf("Private Key (k): %s\n", k_hex);
    printf("Bit Length: %d bits\n", BN_num_bits(test->private_key));
    OPENSSL_free(k_hex);
    
    // Print signature (r, s)
    char* r_hex = BN_bn2hex(test->r);
    char* s_hex = BN_bn2hex(test->s);
    printf("Signature (r): %s\n", r_hex);
    printf("Signature (s): %s\n", s_hex);
    OPENSSL_free(r_hex);
    OPENSSL_free(s_hex);
    
    // Print message hash
    printf("Message Hash: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", test->message_hash[i]);
    }
    printf("\n");
    
    // Verify signature
    bool valid = verify_ecdsa_signature(test);
    printf("Signature Valid: %s\n", valid ? "YES" : "NO");
    
    // Print recovery results if available
    if (test->recovered_k) {
        char* recovered_hex = BN_bn2hex(test->recovered_k);
        printf("\nRecovered k: %s\n", recovered_hex);
        printf("Recovery Success: %s\n", test->recovery_success ? "YES" : "NO");
        printf("Recovery Time: %.3f seconds\n", test->recovery_time);
        OPENSSL_free(recovered_hex);
        
        // Compute error
        if (test->recovery_success) {
            BIGNUM* error = BN_new();
            BN_sub(error, test->recovered_k, test->private_key);
            char* error_hex = BN_bn2hex(error);
            printf("Error: %s\n", error_hex);
            OPENSSL_free(error_hex);
            BN_free(error);
        }
    }
    
    printf("========================\n");
}

/**
 * Free ECDSA test case
 */
void free_ecdsa_test_case(ECDSATestCase* test) {
    if (!test) return;
    
    if (test->recovered_k) BN_free(test->recovered_k);
    if (test->s) BN_free(test->s);
    if (test->r) BN_free(test->r);
    if (test->signature) ECDSA_SIG_free(test->signature);
    if (test->public_key) EC_POINT_free(test->public_key);
    if (test->private_key) BN_free(test->private_key);
    if (test->key) EC_KEY_free(test->key);
    
    free(test);
}

/**
 * Generate test suite with multiple bit lengths
 */
typedef struct {
    uint32_t num_tests;
    ECDSATestCase** tests;
    
    // Statistics
    uint32_t num_successes;
    double avg_recovery_time;
    double success_rate;
} ECDSATestSuite;

ECDSATestSuite* create_ecdsa_test_suite(int curve_nid, const uint32_t* bit_lengths, 
                                        const uint32_t* tests_per_length, 
                                        uint32_t num_lengths) {
    ECDSATestSuite* suite = calloc(1, sizeof(ECDSATestSuite));
    if (!suite) return NULL;
    
    // Count total tests
    suite->num_tests = 0;
    for (uint32_t i = 0; i < num_lengths; i++) {
        suite->num_tests += tests_per_length[i];
    }
    
    // Allocate test array
    suite->tests = calloc(suite->num_tests, sizeof(ECDSATestCase*));
    if (!suite->tests) {
        free(suite);
        return NULL;
    }
    
    // Generate tests
    uint32_t test_idx = 0;
    for (uint32_t i = 0; i < num_lengths; i++) {
        uint32_t bit_length = bit_lengths[i];
        uint32_t num_tests = tests_per_length[i];
        
        printf("Generating %u tests with %u-bit k values...\n", num_tests, bit_length);
        
        for (uint32_t j = 0; j < num_tests; j++) {
            suite->tests[test_idx] = create_random_ecdsa_test_case(curve_nid, bit_length);
            if (!suite->tests[test_idx]) {
                printf("Failed to create test case %u\n", test_idx);
                // Continue with other tests
            }
            test_idx++;
        }
    }
    
    return suite;
}

/**
 * Compute test suite statistics
 */
void compute_test_suite_statistics(ECDSATestSuite* suite) {
    if (!suite) return;
    
    suite->num_successes = 0;
    double total_time = 0.0;
    uint32_t valid_tests = 0;
    
    for (uint32_t i = 0; i < suite->num_tests; i++) {
        if (!suite->tests[i]) continue;
        valid_tests++;
        
        if (suite->tests[i]->recovery_success) {
            suite->num_successes++;
        }
        
        total_time += suite->tests[i]->recovery_time;
    }
    
    suite->success_rate = (double)suite->num_successes / valid_tests;
    suite->avg_recovery_time = total_time / valid_tests;
}

/**
 * Print test suite summary
 */
void print_test_suite_summary(const ECDSATestSuite* suite) {
    if (!suite) return;
    
    printf("\n========================================\n");
    printf("ECDSA TEST SUITE SUMMARY\n");
    printf("========================================\n");
    printf("Total Tests: %u\n", suite->num_tests);
    printf("Successes: %u\n", suite->num_successes);
    printf("Success Rate: %.1f%%\n", suite->success_rate * 100.0);
    printf("Avg Recovery Time: %.3f seconds\n", suite->avg_recovery_time);
    printf("========================================\n");
}

/**
 * Free test suite
 */
void free_ecdsa_test_suite(ECDSATestSuite* suite) {
    if (!suite) return;
    
    if (suite->tests) {
        for (uint32_t i = 0; i < suite->num_tests; i++) {
            if (suite->tests[i]) {
                free_ecdsa_test_case(suite->tests[i]);
            }
        }
        free(suite->tests);
    }
    
    free(suite);
}
