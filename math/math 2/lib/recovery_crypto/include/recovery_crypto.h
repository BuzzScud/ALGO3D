/**
 * @file recovery_crypto.h
 * @brief Cryptographic Recovery Library
 * 
 * Production-grade cryptographic recovery for:
 * - Bitcoin private key recovery (all networks)
 * - SSH key recovery (RSA, Ed25519, ECDSA)
 * - ECDSA nonce recovery
 * - Ransomware key recovery (WannaCry, etc.)
 * - Generic cryptographic key recovery
 * 
 * Uses samples-as-anchors with OBJECTIVE 28 Phase 1-6 algorithms.
 */

#ifndef RECOVERY_CRYPTO_H
#define RECOVERY_CRYPTO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version
#define RECOVERY_CRYPTO_VERSION_MAJOR 1
#define RECOVERY_CRYPTO_VERSION_MINOR 0
#define RECOVERY_CRYPTO_VERSION_PATCH 0

// Error codes
typedef enum {
    CRYPTO_OK = 0,
    CRYPTO_ERROR_INVALID_PARAM = -1,
    CRYPTO_ERROR_OUT_OF_MEMORY = -2,
    CRYPTO_ERROR_NOT_CONVERGED = -3,
    CRYPTO_ERROR_INVALID_KEY = -4,
    CRYPTO_ERROR_INVALID_SIGNATURE = -5,
    CRYPTO_ERROR_UNSUPPORTED = -6
} crypto_error_t;

// Key types
typedef enum {
    KEY_TYPE_BITCOIN_PRIVATE,       // Bitcoin private key (32 bytes)
    KEY_TYPE_BITCOIN_PUBLIC,        // Bitcoin public key (33/65 bytes)
    KEY_TYPE_SSH_RSA,               // SSH RSA key
    KEY_TYPE_SSH_ED25519,           // SSH Ed25519 key
    KEY_TYPE_SSH_ECDSA,             // SSH ECDSA key
    KEY_TYPE_ECDSA_PRIVATE,         // Generic ECDSA private key
    KEY_TYPE_ECDSA_PUBLIC,          // Generic ECDSA public key
    KEY_TYPE_AES,                   // AES key (128/192/256 bit)
    KEY_TYPE_GENERIC                // Generic binary key
} key_type_t;

// Bitcoin network types
typedef enum {
    BITCOIN_MAINNET,                // Bitcoin mainnet
    BITCOIN_TESTNET,                // Bitcoin testnet
    BITCOIN_REGTEST,                // Bitcoin regtest
    BITCOIN_SIGNET,                 // Bitcoin signet
    BITCOIN_CUSTOM                  // Custom network
} bitcoin_network_t;

// ECDSA curve types
typedef enum {
    CURVE_SECP256K1,                // Bitcoin curve
    CURVE_SECP256R1,                // NIST P-256
    CURVE_ED25519,                  // Ed25519
    CURVE_CUSTOM                    // Custom curve
} ecdsa_curve_t;

// Bitcoin key recovery context
typedef struct {
    bitcoin_network_t network;      // Network type
    uint8_t* partial_key;           // Partial private key (if known)
    size_t partial_key_len;         // Length of partial key
    uint8_t* public_key;            // Public key (if known)
    size_t public_key_len;          // Length of public key
    char* address;                  // Bitcoin address (if known)
    int max_iterations;             // Maximum iterations
    double convergence_threshold;   // Convergence threshold
    int verbose;                    // Verbose output
} bitcoin_recovery_ctx_t;

// SSH key recovery context
typedef struct {
    key_type_t key_type;            // SSH key type
    uint8_t* partial_key;           // Partial private key
    size_t partial_key_len;         // Length of partial key
    uint8_t* public_key;            // Public key (if known)
    size_t public_key_len;          // Length of public key
    int max_iterations;             // Maximum iterations
    double convergence_threshold;   // Convergence threshold
    int verbose;                    // Verbose output
} ssh_recovery_ctx_t;

// ECDSA nonce recovery context
typedef struct {
    ecdsa_curve_t curve;            // ECDSA curve
    uint8_t* signature1;            // First signature
    size_t sig1_len;                // Length of first signature
    uint8_t* signature2;            // Second signature (for nonce reuse)
    size_t sig2_len;                // Length of second signature
    uint8_t* message1;              // First message
    size_t msg1_len;                // Length of first message
    uint8_t* message2;              // Second message (for nonce reuse)
    size_t msg2_len;                // Length of second message
    uint8_t* public_key;            // Public key
    size_t pubkey_len;              // Length of public key
    int max_iterations;             // Maximum iterations
    double convergence_threshold;   // Convergence threshold
    int verbose;                    // Verbose output
} ecdsa_nonce_recovery_ctx_t;

// Ransomware recovery context
typedef struct {
    uint8_t* encrypted_data;        // Encrypted data
    size_t encrypted_len;           // Length of encrypted data
    uint8_t* known_plaintext;       // Known plaintext samples
    size_t plaintext_len;           // Length of known plaintext
    size_t* plaintext_offsets;      // Offsets of known plaintext
    int num_samples;                // Number of plaintext samples
    int key_size;                   // Key size in bits (128/192/256)
    int max_iterations;             // Maximum iterations
    double convergence_threshold;   // Convergence threshold
    int verbose;                    // Verbose output
} ransomware_recovery_ctx_t;

// Recovery result
typedef struct {
    uint8_t* recovered_key;         // Recovered key
    size_t key_len;                 // Length of recovered key
    int iterations;                 // Iterations taken
    double final_oscillation;       // Final oscillation value
    double quality_score;           // Quality score (0-1)
    bool converged;                 // Whether it converged
    double time_seconds;            // Time taken
    char* error_message;            // Error message (if any)
} crypto_recovery_result_t;

/**
 * Bitcoin Key Recovery
 */

/**
 * Recover Bitcoin private key from partial knowledge
 * 
 * @param ctx Recovery context
 * @return Recovery result (caller must free)
 */
crypto_recovery_result_t* recover_bitcoin_key(const bitcoin_recovery_ctx_t* ctx);

/**
 * Validate Bitcoin private key
 * 
 * @param key Private key (32 bytes)
 * @param network Network type
 * @return true if valid
 */
bool validate_bitcoin_key(const uint8_t* key, bitcoin_network_t network);

/**
 * Derive Bitcoin public key from private key
 * 
 * @param private_key Private key (32 bytes)
 * @param public_key Output buffer (33 bytes for compressed)
 * @param compressed Use compressed format
 * @return CRYPTO_OK on success
 */
crypto_error_t derive_bitcoin_pubkey(const uint8_t* private_key, 
                                     uint8_t* public_key, 
                                     bool compressed);

/**
 * Derive Bitcoin address from public key
 * 
 * @param public_key Public key (33 or 65 bytes)
 * @param pubkey_len Length of public key
 * @param network Network type
 * @param address Output buffer (at least 35 bytes)
 * @return CRYPTO_OK on success
 */
crypto_error_t derive_bitcoin_address(const uint8_t* public_key,
                                      size_t pubkey_len,
                                      bitcoin_network_t network,
                                      char* address);

/**
 * SSH Key Recovery
 */

/**
 * Recover SSH private key from partial knowledge
 * 
 * @param ctx Recovery context
 * @return Recovery result (caller must free)
 */
crypto_recovery_result_t* recover_ssh_key(const ssh_recovery_ctx_t* ctx);

/**
 * Validate SSH private key
 * 
 * @param key Private key data
 * @param key_len Length of key
 * @param key_type Key type
 * @return true if valid
 */
bool validate_ssh_key(const uint8_t* key, size_t key_len, key_type_t key_type);

/**
 * ECDSA Nonce Recovery
 */

/**
 * Recover ECDSA nonce from signature reuse
 * 
 * @param ctx Recovery context
 * @return Recovery result (caller must free)
 */
crypto_recovery_result_t* recover_ecdsa_nonce(const ecdsa_nonce_recovery_ctx_t* ctx);

/**
 * Recover private key from nonce reuse
 * 
 * @param nonce Recovered nonce
 * @param signature Signature
 * @param sig_len Length of signature
 * @param message Message
 * @param msg_len Length of message
 * @param curve ECDSA curve
 * @return Private key (caller must free)
 */
uint8_t* recover_privkey_from_nonce(const uint8_t* nonce,
                                    const uint8_t* signature,
                                    size_t sig_len,
                                    const uint8_t* message,
                                    size_t msg_len,
                                    ecdsa_curve_t curve);

/**
 * Ransomware Recovery
 */

/**
 * Recover ransomware encryption key from known plaintext
 * 
 * @param ctx Recovery context
 * @return Recovery result (caller must free)
 */
crypto_recovery_result_t* recover_ransomware_key(const ransomware_recovery_ctx_t* ctx);

/**
 * Decrypt data with recovered key
 * 
 * @param encrypted Encrypted data
 * @param encrypted_len Length of encrypted data
 * @param key Recovered key
 * @param key_len Length of key
 * @param decrypted Output buffer
 * @return CRYPTO_OK on success
 */
crypto_error_t decrypt_with_key(const uint8_t* encrypted,
                                size_t encrypted_len,
                                const uint8_t* key,
                                size_t key_len,
                                uint8_t* decrypted);

/**
 * Utility Functions
 */

/**
 * Free recovery result
 * 
 * @param result Result to free
 */
void crypto_free_result(crypto_recovery_result_t* result);

/**
 * Get error string
 * 
 * @param error Error code
 * @return Error string
 */
const char* crypto_error_string(crypto_error_t error);

/**
 * Get key type string
 * 
 * @param type Key type
 * @return Key type string
 */
const char* key_type_string(key_type_t type);

/**
 * Get network string
 * 
 * @param network Network type
 * @return Network string
 */
const char* bitcoin_network_string(bitcoin_network_t network);

/**
 * Get curve string
 * 
 * @param curve Curve type
 * @return Curve string
 */
const char* ecdsa_curve_string(ecdsa_curve_t curve);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_CRYPTO_H