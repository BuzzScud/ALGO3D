/*
 * JSON Loader Implementation
 * Uses jansson library for JSON parsing
 * Supports crypto data (Bitcoin transactions, ECDSA signatures)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <jansson.h>

// Forward declarations for recovery_common types
typedef enum {
    DATA_TYPE_GEOMETRIC,
    DATA_TYPE_SIGNAL,
    DATA_TYPE_IMAGE,
    DATA_TYPE_NETWORK,
    DATA_TYPE_CRYPTO,
    DATA_TYPE_SCIENTIFIC,
    DATA_TYPE_ML,
    DATA_TYPE_UNKNOWN
} DataType;

typedef struct {
    uint8_t** hashes;
    size_t num_hashes;
    size_t hash_size;
    void* structure;
    void* metadata;
} CryptoData;

typedef struct {
    union {
        CryptoData crypto;
        // Other data types...
    };
} GenericData;

// Helper function to parse hex string to bytes
static int hex_to_bytes(const char* hex_str, uint8_t* bytes, size_t max_len) {
    if (!hex_str || !bytes) return -1;
    
    size_t hex_len = strlen(hex_str);
    if (hex_len % 2 != 0) return -1;  // Must be even length
    
    // Skip "0x" prefix if present
    if (hex_len >= 2 && hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        hex_str += 2;
        hex_len -= 2;
    }
    
    size_t byte_len = hex_len / 2;
    if (byte_len > max_len) return -1;
    
    for (size_t i = 0; i < byte_len; i++) {
        unsigned int byte;
        if (sscanf(hex_str + (i * 2), "%2x", &byte) != 1) {
            return -1;
        }
        bytes[i] = (uint8_t)byte;
    }
    
    return byte_len;
}

// Bitcoin transaction structure
typedef struct {
    char* txid;
    uint8_t* signature_r;
    size_t r_len;
    uint8_t* signature_s;
    size_t s_len;
    uint8_t* public_key_x;
    size_t pubkey_x_len;
    uint8_t* public_key_y;
    size_t pubkey_y_len;
    uint8_t* message_hash;
    size_t hash_len;
    char* curve;
} BitcoinTransaction;

/**
 * Load Bitcoin transaction from JSON
 */
bool load_bitcoin_transaction_json(FILE* file, BitcoinTransaction* tx) {
    if (!file || !tx) return false;
    
    // Read entire file into buffer
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) return false;
    
    size_t read_size = fread(buffer, 1, file_size, file);
    buffer[read_size] = '\0';
    
    // Parse JSON
    json_error_t error;
    json_t* root = json_loads(buffer, 0, &error);
    free(buffer);
    
    if (!root) {
        fprintf(stderr, "JSON parse error: %s\n", error.text);
        return false;
    }
    
    // Extract txid
    json_t* txid_obj = json_object_get(root, "txid");
    if (txid_obj && json_is_string(txid_obj)) {
        tx->txid = strdup(json_string_value(txid_obj));
    }
    
    // Extract signature
    json_t* sig_obj = json_object_get(root, "signature");
    if (sig_obj && json_is_object(sig_obj)) {
        json_t* r_obj = json_object_get(sig_obj, "r");
        json_t* s_obj = json_object_get(sig_obj, "s");
        
        if (r_obj && json_is_string(r_obj)) {
            const char* r_hex = json_string_value(r_obj);
            tx->signature_r = (uint8_t*)malloc(64);  // Max 256-bit
            tx->r_len = hex_to_bytes(r_hex, tx->signature_r, 64);
        }
        
        if (s_obj && json_is_string(s_obj)) {
            const char* s_hex = json_string_value(s_obj);
            tx->signature_s = (uint8_t*)malloc(64);
            tx->s_len = hex_to_bytes(s_hex, tx->signature_s, 64);
        }
    }
    
    // Extract public key
    json_t* pubkey_obj = json_object_get(root, "public_key");
    if (pubkey_obj && json_is_object(pubkey_obj)) {
        json_t* x_obj = json_object_get(pubkey_obj, "x");
        json_t* y_obj = json_object_get(pubkey_obj, "y");
        
        if (x_obj && json_is_string(x_obj)) {
            const char* x_hex = json_string_value(x_obj);
            tx->public_key_x = (uint8_t*)malloc(64);
            tx->pubkey_x_len = hex_to_bytes(x_hex, tx->public_key_x, 64);
        }
        
        if (y_obj && json_is_string(y_obj)) {
            const char* y_hex = json_string_value(y_obj);
            tx->public_key_y = (uint8_t*)malloc(64);
            tx->pubkey_y_len = hex_to_bytes(y_hex, tx->public_key_y, 64);
        }
    }
    
    // Extract message hash
    json_t* hash_obj = json_object_get(root, "message_hash");
    if (hash_obj && json_is_string(hash_obj)) {
        const char* hash_hex = json_string_value(hash_obj);
        tx->message_hash = (uint8_t*)malloc(64);
        tx->hash_len = hex_to_bytes(hash_hex, tx->message_hash, 64);
    }
    
    // Extract curve
    json_t* curve_obj = json_object_get(root, "curve");
    if (curve_obj && json_is_string(curve_obj)) {
        tx->curve = strdup(json_string_value(curve_obj));
    }
    
    json_decref(root);
    return true;
}

/**
 * Load generic JSON crypto data
 */
bool load_json_crypto(FILE* file, GenericData* data, DataType* type) {
    if (!file || !data || !type) return false;
    
    BitcoinTransaction tx = {0};
    if (!load_bitcoin_transaction_json(file, &tx)) {
        return false;
    }
    
    // Convert to CryptoData structure
    *type = DATA_TYPE_CRYPTO;
    
    // Allocate hash array (for signature components)
    data->crypto.num_hashes = 4;  // r, s, pubkey_x, pubkey_y
    data->crypto.hash_size = 32;  // 256-bit
    data->crypto.hashes = (uint8_t**)calloc(4, sizeof(uint8_t*));
    
    if (tx.signature_r && tx.r_len > 0) {
        data->crypto.hashes[0] = (uint8_t*)malloc(32);
        memcpy(data->crypto.hashes[0], tx.signature_r, tx.r_len < 32 ? tx.r_len : 32);
    }
    
    if (tx.signature_s && tx.s_len > 0) {
        data->crypto.hashes[1] = (uint8_t*)malloc(32);
        memcpy(data->crypto.hashes[1], tx.signature_s, tx.s_len < 32 ? tx.s_len : 32);
    }
    
    if (tx.public_key_x && tx.pubkey_x_len > 0) {
        data->crypto.hashes[2] = (uint8_t*)malloc(32);
        memcpy(data->crypto.hashes[2], tx.public_key_x, tx.pubkey_x_len < 32 ? tx.pubkey_x_len : 32);
    }
    
    if (tx.public_key_y && tx.pubkey_y_len > 0) {
        data->crypto.hashes[3] = (uint8_t*)malloc(32);
        memcpy(data->crypto.hashes[3], tx.public_key_y, tx.pubkey_y_len < 32 ? tx.pubkey_y_len : 32);
    }
    
    // Store metadata
    data->crypto.metadata = malloc(sizeof(BitcoinTransaction));
    memcpy(data->crypto.metadata, &tx, sizeof(BitcoinTransaction));
    
    return true;
}

/**
 * Save crypto data as JSON
 */
bool save_json_crypto(FILE* file, CryptoData* data) {
    if (!file || !data) return false;
    
    json_t* root = json_object();
    
    // Create signature object
    json_t* signature = json_object();
    if (data->num_hashes >= 2 && data->hashes[0] && data->hashes[1]) {
        char r_hex[128] = {0};
        char s_hex[128] = {0};
        
        for (size_t i = 0; i < data->hash_size && i < 32; i++) {
            sprintf(r_hex + (i * 2), "%02x", data->hashes[0][i]);
            sprintf(s_hex + (i * 2), "%02x", data->hashes[1][i]);
        }
        
        json_object_set_new(signature, "r", json_string(r_hex));
        json_object_set_new(signature, "s", json_string(s_hex));
    }
    json_object_set_new(root, "signature", signature);
    
    // Create public key object
    json_t* pubkey = json_object();
    if (data->num_hashes >= 4 && data->hashes[2] && data->hashes[3]) {
        char x_hex[128] = {0};
        char y_hex[128] = {0};
        
        for (size_t i = 0; i < data->hash_size && i < 32; i++) {
            sprintf(x_hex + (i * 2), "%02x", data->hashes[2][i]);
            sprintf(y_hex + (i * 2), "%02x", data->hashes[3][i]);
        }
        
        json_object_set_new(pubkey, "x", json_string(x_hex));
        json_object_set_new(pubkey, "y", json_string(y_hex));
    }
    json_object_set_new(root, "public_key", pubkey);
    
    // Write JSON to file
    char* json_str = json_dumps(root, JSON_INDENT(2));
    if (json_str) {
        fprintf(file, "%s\n", json_str);
        free(json_str);
    }
    
    json_decref(root);
    return true;
}

/**
 * Free Bitcoin transaction
 */
void free_bitcoin_transaction(BitcoinTransaction* tx) {
    if (!tx) return;
    
    free(tx->txid);
    free(tx->signature_r);
    free(tx->signature_s);
    free(tx->public_key_x);
    free(tx->public_key_y);
    free(tx->message_hash);
    free(tx->curve);
}
