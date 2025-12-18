#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/sha.h>

/**
 * ECDSA Sample Generator
 * 
 * Generates real ECDSA signatures with known k values for testing.
 * Saves samples to files for reproducible testing.
 * 
 * RULE 1 COMPLIANT: Uses OpenSSL ONLY for EC/BIGNUM operations (allowed)
 */

typedef struct {
    char* k_hex;           // Private key (nonce) in hex
    char* r_hex;           // Signature r component in hex
    char* s_hex;           // Signature s component in hex
    char* pubkey_x_hex;    // Public key X coordinate in hex
    char* pubkey_y_hex;    // Public key Y coordinate in hex
    char* message_hex;     // Message hash in hex
    int bit_length;        // Bit length of k
    int curve_nid;         // OpenSSL curve NID
} ECDSASample;

/**
 * Generate random message hash
 */
void generate_random_hash(unsigned char* hash, size_t len) {
    for (size_t i = 0; i < len; i++) {
        hash[i] = (unsigned char)(rand() % 256);
    }
}

/**
 * Generate ECDSA sample with specific bit length
 */
ECDSASample* generate_ecdsa_sample(int bit_length, int curve_nid) {
    ECDSASample* sample = (ECDSASample*)calloc(1, sizeof(ECDSASample));
    if (!sample) return NULL;
    
    sample->bit_length = bit_length;
    sample->curve_nid = curve_nid;
    
    // Create EC_KEY
    EC_KEY* key = EC_KEY_new_by_curve_name(curve_nid);
    if (!key) {
        free(sample);
        return NULL;
    }
    
    // Generate key pair
    if (!EC_KEY_generate_key(key)) {
        EC_KEY_free(key);
        free(sample);
        return NULL;
    }
    
    // Get private key (this is our "k" for testing)
    const BIGNUM* priv_key = EC_KEY_get0_private_key(key);
    
    // For smaller bit lengths, create a smaller k
    BIGNUM* k = BN_new();
    if (bit_length < 256) {
        // Create k with specific bit length
        BN_rand(k, bit_length, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        
        // Set as private key
        EC_KEY_set_private_key(key, k);
        
        // Regenerate public key from new private key
        const EC_GROUP* group = EC_KEY_get0_group(key);
        EC_POINT* pub_key = EC_POINT_new(group);
        BN_CTX* ctx = BN_CTX_new();
        
        EC_POINT_mul(group, pub_key, k, NULL, NULL, ctx);
        EC_KEY_set_public_key(key, pub_key);
        
        EC_POINT_free(pub_key);
        BN_CTX_free(ctx);
        
        priv_key = k;
    }
    
    // Store k in hex
    sample->k_hex = BN_bn2hex(priv_key);
    
    // Generate random message hash
    unsigned char hash[32];
    generate_random_hash(hash, 32);
    
    // Store message hash
    sample->message_hex = (char*)malloc(65);
    for (int i = 0; i < 32; i++) {
        sprintf(sample->message_hex + (i * 2), "%02x", hash[i]);
    }
    sample->message_hex[64] = '\0';
    
    // Sign the message
    ECDSA_SIG* sig = ECDSA_do_sign(hash, 32, key);
    if (!sig) {
        EC_KEY_free(key);
        if (bit_length < 256) BN_free(k);
        free(sample->k_hex);
        free(sample->message_hex);
        free(sample);
        return NULL;
    }
    
    // Get r and s
    const BIGNUM* r = NULL;
    const BIGNUM* s = NULL;
    ECDSA_SIG_get0(sig, &r, &s);
    
    // Store r and s in hex
    sample->r_hex = BN_bn2hex(r);
    sample->s_hex = BN_bn2hex(s);
    
    // Get public key
    const EC_POINT* pub_key = EC_KEY_get0_public_key(key);
    const EC_GROUP* group = EC_KEY_get0_group(key);
    
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    BN_CTX* ctx = BN_CTX_new();
    
    EC_POINT_get_affine_coordinates(group, pub_key, x, y, ctx);
    
    sample->pubkey_x_hex = BN_bn2hex(x);
    sample->pubkey_y_hex = BN_bn2hex(y);
    
    // Cleanup
    BN_free(x);
    BN_free(y);
    BN_CTX_free(ctx);
    ECDSA_SIG_free(sig);
    EC_KEY_free(key);
    if (bit_length < 256) BN_free(k);
    
    return sample;
}

/**
 * Free ECDSA sample
 */
void free_ecdsa_sample(ECDSASample* sample) {
    if (!sample) return;
    
    if (sample->k_hex) OPENSSL_free(sample->k_hex);
    if (sample->r_hex) OPENSSL_free(sample->r_hex);
    if (sample->s_hex) OPENSSL_free(sample->s_hex);
    if (sample->pubkey_x_hex) OPENSSL_free(sample->pubkey_x_hex);
    if (sample->pubkey_y_hex) OPENSSL_free(sample->pubkey_y_hex);
    if (sample->message_hex) free(sample->message_hex);
    
    free(sample);
}

/**
 * Save sample to file
 */
int save_sample_to_file(ECDSASample* sample, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    
    fprintf(f, "# ECDSA Sample - %d-bit k\n", sample->bit_length);
    fprintf(f, "# Generated: %s", ctime(&(time_t){time(NULL)}));
    fprintf(f, "# Curve: %s\n", 
            sample->curve_nid == NID_secp256k1 ? "secp256k1" : "secp192k1");
    fprintf(f, "\n");
    
    fprintf(f, "k=%s\n", sample->k_hex);
    fprintf(f, "r=%s\n", sample->r_hex);
    fprintf(f, "s=%s\n", sample->s_hex);
    fprintf(f, "pubkey_x=%s\n", sample->pubkey_x_hex);
    fprintf(f, "pubkey_y=%s\n", sample->pubkey_y_hex);
    fprintf(f, "message=%s\n", sample->message_hex);
    fprintf(f, "bit_length=%d\n", sample->bit_length);
    fprintf(f, "curve_nid=%d\n", sample->curve_nid);
    
    fclose(f);
    return 1;
}

/**
 * Generate samples for a specific bit length
 */
void generate_samples_for_bit_length(int bit_length, int num_samples, int curve_nid, const char* output_dir) {
    printf("Generating %d samples for %d-bit k...\n", num_samples, bit_length);
    
    for (int i = 0; i < num_samples; i++) {
        // Generate sample
        ECDSASample* sample = generate_ecdsa_sample(bit_length, curve_nid);
        if (!sample) {
            printf("  Failed to generate sample %d\n", i + 1);
            continue;
        }
        
        // Create filename
        char filename[256];
        const char* curve_name = (curve_nid == NID_secp256k1) ? "secp256k1" : "secp192k1";
        snprintf(filename, sizeof(filename), "%s/%s_%dbit_sample_%03d.txt",
                 output_dir, curve_name, bit_length, i + 1);
        
        // Save to file
        if (save_sample_to_file(sample, filename)) {
            printf("  ✓ Saved: %s (k=%.*s...)\n", filename, 16, sample->k_hex);
        } else {
            printf("  ✗ Failed to save: %s\n", filename);
        }
        
        // Cleanup
        free_ecdsa_sample(sample);
    }
    
    printf("\n");
}

/**
 * Main function
 */
int main(int argc, char** argv) {
    printf("\n========================================\n");
    printf("ECDSA SAMPLE GENERATOR\n");
    printf("========================================\n\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    // Output directory
    const char* output_dir = (argc > 1) ? argv[1] : "samples";
    
    printf("Output directory: %s\n\n", output_dir);
    
    // Bit lengths to generate
    int bit_lengths[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256};
    int num_bit_lengths = sizeof(bit_lengths) / sizeof(bit_lengths[0]);
    
    // Number of samples per bit length
    int samples_per_length = 10;
    
    // Generate for secp256k1 (Bitcoin curve)
    printf("=== Generating secp256k1 samples ===\n\n");
    for (int i = 0; i < num_bit_lengths; i++) {
        generate_samples_for_bit_length(bit_lengths[i], samples_per_length, NID_secp256k1, output_dir);
    }
    
    // Generate for secp192k1 (smaller curve for testing)
    printf("=== Generating secp192k1 samples ===\n\n");
    for (int i = 0; i < num_bit_lengths; i++) {
        if (bit_lengths[i] <= 192) {  // Only generate for valid bit lengths
            generate_samples_for_bit_length(bit_lengths[i], samples_per_length, NID_secp192k1, output_dir);
        }
    }
    
    printf("========================================\n");
    printf("GENERATION COMPLETE!\n");
    printf("========================================\n\n");
    
    // Summary
    int total_secp256k1 = num_bit_lengths * samples_per_length;
    int total_secp192k1 = 0;
    for (int i = 0; i < num_bit_lengths; i++) {
        if (bit_lengths[i] <= 192) total_secp192k1 += samples_per_length;
    }
    
    printf("Total samples generated:\n");
    printf("  secp256k1: %d samples\n", total_secp256k1);
    printf("  secp192k1: %d samples\n", total_secp192k1);
    printf("  Total: %d samples\n", total_secp256k1 + total_secp192k1);
    printf("\n");
    
    printf("Bit lengths: ");
    for (int i = 0; i < num_bit_lengths; i++) {
        printf("%d", bit_lengths[i]);
        if (i < num_bit_lengths - 1) printf(", ");
    }
    printf("\n");
    
    printf("Samples per bit length: %d\n", samples_per_length);
    printf("\n");
    
    return 0;
}