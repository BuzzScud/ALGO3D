#ifndef ECDSA_SAMPLE_LOADER_H
#define ECDSA_SAMPLE_LOADER_H

#include <stdint.h>
#include <stdbool.h>
#include <openssl/bn.h>
#include <openssl/ec.h>

/**
 * ECDSA Sample Loader
 * 
 * Loads pre-generated ECDSA samples from files for testing.
 * Provides ground truth k values for validation.
 */

typedef struct {
    BIGNUM* k;              // Private key (nonce) - GROUND TRUTH
    BIGNUM* r;              // Signature r component
    BIGNUM* s;              // Signature s component
    BIGNUM* pubkey_x;       // Public key X coordinate
    BIGNUM* pubkey_y;       // Public key Y coordinate
    unsigned char message[32];  // Message hash
    int bit_length;         // Bit length of k
    int curve_nid;          // OpenSSL curve NID
    char* filename;         // Source filename
} ECDSASample;

/**
 * Load ECDSA sample from file
 */
ECDSASample* load_ecdsa_sample(const char* filename);

/**
 * Load all samples from directory
 */
ECDSASample** load_all_samples(const char* directory, int* num_samples);

/**
 * Load samples for specific bit length
 */
ECDSASample** load_samples_by_bit_length(
    const char* directory,
    int bit_length,
    int* num_samples
);

/**
 * Load samples for specific curve
 */
ECDSASample** load_samples_by_curve(
    const char* directory,
    int curve_nid,
    int* num_samples
);

/**
 * Free ECDSA sample
 */
void free_ecdsa_sample(ECDSASample* sample);

/**
 * Free array of samples
 */
void free_ecdsa_samples(ECDSASample** samples, int num_samples);

/**
 * Print sample info
 */
void print_ecdsa_sample(const ECDSASample* sample);

/**
 * Verify sample signature
 */
bool verify_ecdsa_sample(const ECDSASample* sample);

/**
 * Get sample statistics
 */
void print_sample_statistics(ECDSASample** samples, int num_samples);

#endif // ECDSA_SAMPLE_LOADER_H