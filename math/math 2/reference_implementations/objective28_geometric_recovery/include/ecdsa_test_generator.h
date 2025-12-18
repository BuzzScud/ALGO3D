#ifndef ECDSA_TEST_GENERATOR_H
#define ECDSA_TEST_GENERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>

/**
 * ECDSA Test Case
 * 
 * Represents a single ECDSA signature with known k value for testing
 */
typedef struct {
    EC_KEY* key;
    BIGNUM* private_key;  // Known k for testing
    EC_POINT* public_key;
    
    // Message and signature
    unsigned char message_hash[32];
    ECDSA_SIG* signature;
    BIGNUM* r;
    BIGNUM* s;
    
    // Recovery results
    BIGNUM* recovered_k;
    bool recovery_success;
    double recovery_time;
    
} ECDSATestCase;

/**
 * ECDSA Test Suite
 * 
 * Collection of test cases with statistics
 */
typedef struct {
    uint32_t num_tests;
    ECDSATestCase** tests;
    
    // Statistics
    uint32_t num_successes;
    double avg_recovery_time;
    double success_rate;
} ECDSATestSuite;

/**
 * Create ECDSA test case with known k
 */
ECDSATestCase* create_ecdsa_test_case(int curve_nid, const BIGNUM* known_k);

/**
 * Create ECDSA test case with random k of specified bit length
 */
ECDSATestCase* create_random_ecdsa_test_case(int curve_nid, uint32_t bit_length);

/**
 * Verify ECDSA signature
 */
bool verify_ecdsa_signature(const ECDSATestCase* test);

/**
 * Print test case information
 */
void print_ecdsa_test_case(const ECDSATestCase* test);

/**
 * Free ECDSA test case
 */
void free_ecdsa_test_case(ECDSATestCase* test);

/**
 * Create test suite with multiple bit lengths
 */
ECDSATestSuite* create_ecdsa_test_suite(int curve_nid, const uint32_t* bit_lengths, 
                                        const uint32_t* tests_per_length, 
                                        uint32_t num_lengths);

/**
 * Compute test suite statistics
 */
void compute_test_suite_statistics(ECDSATestSuite* suite);

/**
 * Print test suite summary
 */
void print_test_suite_summary(const ECDSATestSuite* suite);

/**
 * Free test suite
 */
void free_ecdsa_test_suite(ECDSATestSuite* suite);

#endif // ECDSA_TEST_GENERATOR_H