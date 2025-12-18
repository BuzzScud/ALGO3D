/**
 * @file recovery_crypto.c
 * @brief Cryptographic Recovery Implementation
 * 
 * Integrates with OBJECTIVE 28 Phase 1-6 algorithms for cryptographic recovery.
 */

#include "recovery_crypto.h"
#include "recovery_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// OpenSSL for cryptographic operations (optional - stub implementations provided)
#ifdef HAVE_OPENSSL
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include "math/arithmetic.h"
#include "math/transcendental.h"
#else
// Stub implementations when OpenSSL is not available
// In production, install OpenSSL development libraries
#warning "OpenSSL not available - using stub implementations"
#endif

/**
 * Bitcoin Key Recovery
 */

crypto_recovery_result_t* recover_bitcoin_key(const bitcoin_recovery_ctx_t* ctx) {
    if (!ctx) return NULL;
    
    crypto_recovery_result_t* result = calloc(1, sizeof(crypto_recovery_result_t));
    if (!result) return NULL;
    
    clock_t start = clock();
    
    // Initialize recovery context using core library
    recovery_config_t config = recovery_default_config();
    config.max_iterations = ctx->max_iterations > 0 ? ctx->max_iterations : 10000;
    config.convergence_threshold = ctx->convergence_threshold > 0 ? ctx->convergence_threshold : 0.001;
    config.verbose = ctx->verbose;
    config.method = RECOVERY_METHOD_CRYPTO;
    
    recovery_context_t* recovery_ctx = recovery_init(&config);
    if (!recovery_ctx) {
        result->error_message = strdup("Failed to initialize recovery context");
        return result;
    }
    
    // Set Q data (public key or address as target)
    if (ctx->public_key && ctx->public_key_len > 0) {
        recovery_error_t err = recovery_set_q(recovery_ctx, ctx->public_key, ctx->public_key_len);
        if (err != RECOVERY_OK) {
            result->error_message = strdup("Failed to set Q data");
            recovery_free(recovery_ctx);
            return result;
        }
    }
    
    // Add partial key as sample if available
    if (ctx->partial_key && ctx->partial_key_len > 0) {
        recovery_error_t err = recovery_add_sample(recovery_ctx, 
                                                   ctx->partial_key,
                                                   ctx->partial_key_len,
                                                   0,  // offset
                                                   1.0); // confidence
        if (err != RECOVERY_OK) {
            result->error_message = strdup("Failed to add sample");
            recovery_free(recovery_ctx);
            return result;
        }
    }
    
    // Run recovery using OBJECTIVE 28 algorithms
    recovery_error_t err = recovery_run(recovery_ctx);
    if (err != RECOVERY_OK) {
        result->error_message = strdup("Recovery failed to converge");
        result->converged = false;
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Get recovery result
    recovery_result_t* recovery_result = recovery_get_result(recovery_ctx);
    if (!recovery_result) {
        result->error_message = strdup("Failed to get recovery result");
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Validate recovered key
    if (recovery_result->length == 32) {
        bool valid = validate_bitcoin_key(recovery_result->data, ctx->network);
        if (valid) {
            result->recovered_key = malloc(32);
            if (result->recovered_key) {
                memcpy(result->recovered_key, recovery_result->data, 32);
                result->key_len = 32;
                result->converged = true;
                result->quality_score = recovery_result->quality_score;
            }
        }
    }
    
    result->iterations = recovery_result->iterations;
    result->final_oscillation = recovery_result->final_oscillation;
    result->time_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    recovery_free_result(recovery_result);
    recovery_free(recovery_ctx);
    
    return result;
}

bool validate_bitcoin_key(const uint8_t* key, bitcoin_network_t network) {
    if (!key) return false;
    
#ifdef HAVE_OPENSSL
    // Check if key is in valid range (1 to n-1 where n is curve order)
    BIGNUM* bn_key = BN_new();
    BIGNUM* bn_order = BN_new();
    
    if (!bn_key || !bn_order) {
        if (bn_key) BN_free(bn_key);
        if (bn_order) BN_free(bn_order);
        return false;
    }
    
    BN_bin2bn(key, 32, bn_key);
    
    // Get secp256k1 curve order
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!group) {
        BN_free(bn_key);
        BN_free(bn_order);
        return false;
    }
    
    EC_GROUP_get_order(group, bn_order, NULL);
    
    // Check: 0 < key < order
    bool valid = (BN_cmp(bn_key, BN_value_one()) >= 0) && 
                 (BN_cmp(bn_key, bn_order) < 0);
    
    EC_GROUP_free(group);
    BN_free(bn_key);
    BN_free(bn_order);
    
    return valid;
#else
    // Stub implementation - basic validation
    // Check key is not all zeros
    bool all_zero = true;
    for (int i = 0; i < 32; i++) {
        if (key[i] != 0) {
            all_zero = false;
            break;
        }
    }
    return !all_zero;
#endif
}

crypto_error_t derive_bitcoin_pubkey(const uint8_t* private_key, 
                                     uint8_t* public_key, 
                                     bool compressed) {
    if (!private_key || !public_key) return CRYPTO_ERROR_INVALID_PARAM;
    
#ifdef HAVE_OPENSSL
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!key) return CRYPTO_ERROR_OUT_OF_MEMORY;
    
    BIGNUM* bn_priv = BN_bin2bn(private_key, 32, NULL);
    if (!bn_priv) {
        EC_KEY_free(key);
        return CRYPTO_ERROR_INVALID_KEY;
    }
    
    if (!EC_KEY_set_private_key(key, bn_priv)) {
        BN_free(bn_priv);
        EC_KEY_free(key);
        return CRYPTO_ERROR_INVALID_KEY;
    }
    
    // Compute public key
    const EC_GROUP* group = EC_KEY_get0_group(key);
    EC_POINT* pub_point = EC_POINT_new(group);
    if (!pub_point) {
        BN_free(bn_priv);
        EC_KEY_free(key);
        return CRYPTO_ERROR_OUT_OF_MEMORY;
    }
    
    if (!EC_POINT_mul(group, pub_point, bn_priv, NULL, NULL, NULL)) {
        EC_POINT_free(pub_point);
        BN_free(bn_priv);
        EC_KEY_free(key);
        return RECOVERY_ERROR_INTERNAL;
    }
    
    EC_KEY_set_public_key(key, pub_point);
    
    // Convert to bytes
    point_conversion_form_t form = compressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED;
    size_t len = EC_POINT_point2oct(group, pub_point, form, public_key, 
                                    compressed ? 33 : 65, NULL);
    
    EC_POINT_free(pub_point);
    BN_free(bn_priv);
    EC_KEY_free(key);
    
    if (len > 0) {
        return CRYPTO_OK;
    } else {
        return RECOVERY_ERROR_INTERNAL;
    }
#else
    // Stub implementation
    memset(public_key, 0, compressed ? 33 : 65);
    public_key[0] = compressed ? 0x02 : 0x04;
    return CRYPTO_OK;
#endif
}

crypto_error_t derive_bitcoin_address(const uint8_t* public_key,
                                      size_t pubkey_len,
                                      bitcoin_network_t network,
                                      char* address) {
    if (!public_key || !address || (pubkey_len != 33 && pubkey_len != 65)) {
        return CRYPTO_ERROR_INVALID_PARAM;
    }
    
#ifdef HAVE_OPENSSL
    // SHA256 hash
    uint8_t sha256_hash[32];
    SHA256(public_key, pubkey_len, sha256_hash);
    
    // RIPEMD160 hash
    uint8_t ripemd_hash[20];
    RIPEMD160(sha256_hash, 32, ripemd_hash);
    
    // Add version byte
    uint8_t versioned[21];
    versioned[0] = (network == BITCOIN_MAINNET) ? 0x00 : 0x6f; // 0x00 for mainnet, 0x6f for testnet
    memcpy(versioned + 1, ripemd_hash, 20);
    
    // Double SHA256 for checksum
    uint8_t checksum_hash[32];
    SHA256(versioned, 21, checksum_hash);
    SHA256(checksum_hash, 32, checksum_hash);
    
    // Append checksum
    uint8_t address_bytes[25];
    memcpy(address_bytes, versioned, 21);
    memcpy(address_bytes + 21, checksum_hash, 4);
    
    // Base58 encode (simplified - in production use proper Base58 library)
    // For now, return hex representation
    for (int i = 0; i < 25; i++) {
        sprintf(address + i * 2, "%02x", address_bytes[i]);
    }
    address[50] = '\0';
    
    return CRYPTO_OK;
#else
    // Stub implementation
    sprintf(address, "1STUB%s", (network == BITCOIN_MAINNET) ? "MAIN" : "TEST");
    return CRYPTO_OK;
#endif
}

/**
 * SSH Key Recovery
 */

crypto_recovery_result_t* recover_ssh_key(const ssh_recovery_ctx_t* ctx) {
    if (!ctx) return NULL;
    
    crypto_recovery_result_t* result = calloc(1, sizeof(crypto_recovery_result_t));
    if (!result) return NULL;
    
    clock_t start = clock();
    
    // Initialize recovery context
    recovery_config_t config = recovery_default_config();
    config.max_iterations = ctx->max_iterations > 0 ? ctx->max_iterations : 10000;
    config.convergence_threshold = ctx->convergence_threshold > 0 ? ctx->convergence_threshold : 0.001;
    config.verbose = ctx->verbose;
    config.method = RECOVERY_METHOD_CRYPTO;
    
    recovery_context_t* recovery_ctx = recovery_init(&config);
    if (!recovery_ctx) {
        result->error_message = strdup("Failed to initialize recovery context");
        return result;
    }
    
    // Set Q data (public key as target)
    if (ctx->public_key && ctx->public_key_len > 0) {
        recovery_error_t err = recovery_set_q(recovery_ctx, ctx->public_key, ctx->public_key_len);
        if (err != RECOVERY_OK) {
            result->error_message = strdup("Failed to set Q data");
            recovery_free(recovery_ctx);
            return result;
        }
    }
    
    // Add partial key as sample
    if (ctx->partial_key && ctx->partial_key_len > 0) {
        recovery_error_t err = recovery_add_sample(recovery_ctx,
                                                   ctx->partial_key,
                                                   ctx->partial_key_len,
                                                   0,  // offset
                                                   1.0); // confidence
        if (err != RECOVERY_OK) {
            result->error_message = strdup("Failed to add sample");
            recovery_free(recovery_ctx);
            return result;
        }
    }
    
    // Run recovery
    recovery_error_t err = recovery_run(recovery_ctx);
    if (err != RECOVERY_OK) {
        result->error_message = strdup("Recovery failed to converge");
        result->converged = false;
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Get result
    recovery_result_t* recovery_result = recovery_get_result(recovery_ctx);
    if (recovery_result && recovery_result->data) {
        result->recovered_key = malloc(recovery_result->length);
        if (result->recovered_key) {
            memcpy(result->recovered_key, recovery_result->data, recovery_result->length);
            result->key_len = recovery_result->length;
            result->converged = true;
            result->quality_score = recovery_result->quality_score;
        }
        
        result->iterations = recovery_result->iterations;
        result->final_oscillation = recovery_result->final_oscillation;
        
        recovery_free_result(recovery_result);
    }
    
    result->time_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    recovery_free(recovery_ctx);
    return result;
}

bool validate_ssh_key(const uint8_t* key, size_t key_len, key_type_t key_type) {
    if (!key || key_len == 0) return false;
    
    switch (key_type) {
        case KEY_TYPE_SSH_ED25519:
            return key_len == 32 || key_len == 64; // Private or keypair
        case KEY_TYPE_SSH_ECDSA:
            return key_len == 32; // secp256k1 private key
        case KEY_TYPE_SSH_RSA:
            return key_len >= 128; // RSA keys are larger
        default:
            return false;
    }
}

/**
 * ECDSA Nonce Recovery
 */

crypto_recovery_result_t* recover_ecdsa_nonce(const ecdsa_nonce_recovery_ctx_t* ctx) {
    if (!ctx) return NULL;
    
    crypto_recovery_result_t* result = calloc(1, sizeof(crypto_recovery_result_t));
    if (!result) return NULL;
    
    clock_t start = clock();
    
    // For nonce reuse attack, we can directly compute the nonce
    // k = (z1 - z2) / (s1 - s2) mod n
    // where z = hash(message), s = signature component
    
    // This is a simplified implementation
    // In production, use proper ECDSA library
    
    result->error_message = strdup("ECDSA nonce recovery not yet fully implemented");
    result->converged = false;
    result->time_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    return result;
}

uint8_t* recover_privkey_from_nonce(const uint8_t* nonce,
                                    const uint8_t* signature,
                                    size_t sig_len,
                                    const uint8_t* message,
                                    size_t msg_len,
                                    ecdsa_curve_t curve) {
    // Private key recovery from known nonce:
    // d = (s*k - z) / r mod n
    // where d = private key, k = nonce, z = hash(message), r,s = signature
    
    // Simplified implementation
    return NULL;
}

/**
 * Ransomware Recovery
 */

crypto_recovery_result_t* recover_ransomware_key(const ransomware_recovery_ctx_t* ctx) {
    if (!ctx) return NULL;
    
    crypto_recovery_result_t* result = calloc(1, sizeof(crypto_recovery_result_t));
    if (!result) return NULL;
    
    clock_t start = clock();
    
    // Initialize recovery context
    recovery_config_t config = recovery_default_config();
    config.max_iterations = ctx->max_iterations > 0 ? ctx->max_iterations : 10000;
    config.convergence_threshold = ctx->convergence_threshold > 0 ? ctx->convergence_threshold : 0.001;
    config.verbose = ctx->verbose;
    config.method = RECOVERY_METHOD_CRYPTO;
    
    recovery_context_t* recovery_ctx = recovery_init(&config);
    if (!recovery_ctx) {
        result->error_message = strdup("Failed to initialize recovery context");
        return result;
    }
    
    // Set Q data (encrypted data as target)
    if (ctx->encrypted_data && ctx->encrypted_len > 0) {
        recovery_error_t err = recovery_set_q(recovery_ctx, ctx->encrypted_data, ctx->encrypted_len);
        if (err != RECOVERY_OK) {
            result->error_message = strdup("Failed to set Q data");
            recovery_free(recovery_ctx);
            return result;
        }
    }
    
    // Add known plaintext samples
    if (ctx->known_plaintext && ctx->num_samples > 0) {
        for (int i = 0; i < ctx->num_samples; i++) {
            size_t offset = ctx->plaintext_offsets ? ctx->plaintext_offsets[i] : 0;
            recovery_error_t err = recovery_add_sample(recovery_ctx,
                                                       ctx->known_plaintext,
                                                       ctx->plaintext_len,
                                                       offset,
                                                       1.0); // confidence
            if (err != RECOVERY_OK) {
                result->error_message = strdup("Failed to add sample");
                recovery_free(recovery_ctx);
                return result;
            }
        }
    }
    
    // Run recovery
    recovery_error_t err = recovery_run(recovery_ctx);
    if (err != RECOVERY_OK) {
        result->error_message = strdup("Recovery failed to converge");
        result->converged = false;
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Get result
    recovery_result_t* recovery_result = recovery_get_result(recovery_ctx);
    if (recovery_result && recovery_result->data) {
        // Extract key from recovered data (key size based on ctx->key_size)
        size_t key_bytes = ctx->key_size / 8;
        result->recovered_key = malloc(key_bytes);
        if (result->recovered_key) {
            memcpy(result->recovered_key, recovery_result->data, key_bytes);
            result->key_len = key_bytes;
            result->converged = true;
            result->quality_score = recovery_result->quality_score;
        }
        
        result->iterations = recovery_result->iterations;
        result->final_oscillation = recovery_result->final_oscillation;
        
        recovery_free_result(recovery_result);
    }
    
    result->time_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    recovery_free(recovery_ctx);
    return result;
}

crypto_error_t decrypt_with_key(const uint8_t* encrypted,
                                size_t encrypted_len,
                                const uint8_t* key,
                                size_t key_len,
                                uint8_t* decrypted) {
    if (!encrypted || !key || !decrypted) return CRYPTO_ERROR_INVALID_PARAM;
    
#ifdef HAVE_OPENSSL
    // AES decryption
    AES_KEY aes_key;
    if (AES_set_decrypt_key(key, key_len * 8, &aes_key) != 0) {
        return CRYPTO_ERROR_INVALID_KEY;
    }
    
    // Decrypt in ECB mode (simplified - in production use proper mode)
    for (size_t i = 0; i < encrypted_len; i += 16) {
        AES_decrypt(encrypted + i, decrypted + i, &aes_key);
    }
    
    return CRYPTO_OK;
#else
    // Stub implementation - simple XOR
    for (size_t i = 0; i < encrypted_len; i++) {
        decrypted[i] = encrypted[i] ^ key[i % key_len];
    }
    return CRYPTO_OK;
#endif
}

/**
 * Utility Functions
 */

void crypto_free_result(crypto_recovery_result_t* result) {
    if (!result) return;
    
    if (result->recovered_key) {
        // Zero out key before freeing
        memset(result->recovered_key, 0, result->key_len);
        free(result->recovered_key);
    }
    
    if (result->error_message) {
        free(result->error_message);
    }
    
    free(result);
}

const char* crypto_error_string(crypto_error_t error) {
    switch (error) {
        case CRYPTO_OK: return "Success";
        case CRYPTO_ERROR_INVALID_PARAM: return "Invalid parameter";
        case CRYPTO_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case CRYPTO_ERROR_NOT_CONVERGED: return "Recovery did not converge";
        case CRYPTO_ERROR_INVALID_KEY: return "Invalid key";
        case CRYPTO_ERROR_INVALID_SIGNATURE: return "Invalid signature";
        case CRYPTO_ERROR_UNSUPPORTED: return "Unsupported operation";
        default: return "Unknown error";
    }
}

const char* key_type_string(key_type_t type) {
    switch (type) {
        case KEY_TYPE_BITCOIN_PRIVATE: return "Bitcoin Private Key";
        case KEY_TYPE_BITCOIN_PUBLIC: return "Bitcoin Public Key";
        case KEY_TYPE_SSH_RSA: return "SSH RSA";
        case KEY_TYPE_SSH_ED25519: return "SSH Ed25519";
        case KEY_TYPE_SSH_ECDSA: return "SSH ECDSA";
        case KEY_TYPE_ECDSA_PRIVATE: return "ECDSA Private Key";
        case KEY_TYPE_ECDSA_PUBLIC: return "ECDSA Public Key";
        case KEY_TYPE_AES: return "AES Key";
        case KEY_TYPE_GENERIC: return "Generic Key";
        default: return "Unknown";
    }
}

const char* bitcoin_network_string(bitcoin_network_t network) {
    switch (network) {
        case BITCOIN_MAINNET: return "Bitcoin Mainnet";
        case BITCOIN_TESTNET: return "Bitcoin Testnet";
        case BITCOIN_REGTEST: return "Bitcoin Regtest";
        case BITCOIN_SIGNET: return "Bitcoin Signet";
        case BITCOIN_CUSTOM: return "Custom Network";
        default: return "Unknown";
    }
}

const char* ecdsa_curve_string(ecdsa_curve_t curve) {
    switch (curve) {
        case CURVE_SECP256K1: return "secp256k1 (Bitcoin)";
        case CURVE_SECP256R1: return "secp256r1 (NIST P-256)";
        case CURVE_ED25519: return "Ed25519";
        case CURVE_CUSTOM: return "Custom Curve";
        default: return "Unknown";
    }
}
