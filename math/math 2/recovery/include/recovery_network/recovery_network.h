/**
 * @file recovery_network.h
 * @brief Network Protocol Recovery Library
 * 
 * Production-grade network protocol support for:
 * - Bitcoin (mainnet, testnet, regtest, signet, custom)
 * - Full Bitcoin protocol (P2P, RPC, mining, transactions)
 * - Altcoins (Litecoin, Dogecoin, etc.)
 * - Custom blockchain networks
 * - Network data recovery
 */

#ifndef RECOVERY_NETWORK_H
#define RECOVERY_NETWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version
#define RECOVERY_NETWORK_VERSION_MAJOR 1
#define RECOVERY_NETWORK_VERSION_MINOR 0
#define RECOVERY_NETWORK_VERSION_PATCH 0

// Error codes
typedef enum {
    NETWORK_OK = 0,
    NETWORK_ERROR_INVALID_PARAM = -1,
    NETWORK_ERROR_CONNECTION = -2,
    NETWORK_ERROR_TIMEOUT = -3,
    NETWORK_ERROR_PROTOCOL = -4,
    NETWORK_ERROR_PARSE = -5,
    NETWORK_ERROR_NOT_FOUND = -6
} network_error_t;

// Network types
typedef enum {
    NETWORK_BITCOIN_MAINNET,
    NETWORK_BITCOIN_TESTNET,
    NETWORK_BITCOIN_REGTEST,
    NETWORK_BITCOIN_SIGNET,
    NETWORK_LITECOIN_MAINNET,
    NETWORK_LITECOIN_TESTNET,
    NETWORK_DOGECOIN_MAINNET,
    NETWORK_DOGECOIN_TESTNET,
    NETWORK_CUSTOM
} network_type_t;

// Connection types
typedef enum {
    CONNECTION_RPC,                 // JSON-RPC over HTTP
    CONNECTION_P2P,                 // Peer-to-peer protocol
    CONNECTION_REST,                // REST API
    CONNECTION_WEBSOCKET            // WebSocket
} connection_type_t;

// Network configuration
typedef struct {
    network_type_t network;         // Network type
    connection_type_t connection;   // Connection type
    char* host;                     // Host (e.g., "localhost", "blockchain.info")
    int port;                       // Port (e.g., 8332 for Bitcoin RPC)
    char* username;                 // RPC username (if needed)
    char* password;                 // RPC password (if needed)
    char* api_key;                  // API key (if needed)
    int timeout_seconds;            // Connection timeout
    int max_retries;                // Maximum retries
    bool use_ssl;                   // Use SSL/TLS
    int verbose;                    // Verbose output
} network_config_t;

// Network context (opaque)
typedef struct network_context network_context_t;

// Transaction structure
typedef struct {
    char* txid;                     // Transaction ID (hex)
    uint32_t version;               // Transaction version
    uint32_t locktime;              // Lock time
    size_t num_inputs;              // Number of inputs
    size_t num_outputs;             // Number of outputs
    uint8_t* raw_data;              // Raw transaction data
    size_t raw_len;                 // Length of raw data
    bool confirmed;                 // Confirmed in blockchain
    int confirmations;              // Number of confirmations
    uint64_t block_height;          // Block height (if confirmed)
    char* block_hash;               // Block hash (if confirmed)
} network_transaction_t;

// Block structure
typedef struct {
    char* hash;                     // Block hash (hex)
    uint64_t height;                // Block height
    uint32_t version;               // Block version
    char* prev_hash;                // Previous block hash
    char* merkle_root;              // Merkle root
    uint32_t timestamp;             // Block timestamp
    uint32_t bits;                  // Difficulty bits
    uint32_t nonce;                 // Nonce
    size_t num_transactions;        // Number of transactions
    char** transaction_ids;         // Transaction IDs
    uint8_t* raw_data;              // Raw block data
    size_t raw_len;                 // Length of raw data
} network_block_t;

// Address info
typedef struct {
    char* address;                  // Address string
    uint64_t balance;               // Balance in satoshis
    uint64_t received;              // Total received
    uint64_t sent;                  // Total sent
    size_t num_transactions;        // Number of transactions
    char** transaction_ids;         // Transaction IDs
} network_address_info_t;

// Peer info
typedef struct {
    char* address;                  // Peer address
    int port;                       // Peer port
    char* version;                  // Peer version
    char* subversion;               // Peer subversion
    uint64_t services;              // Services flags
    int64_t ping_time;              // Ping time (ms)
    bool inbound;                   // Inbound connection
    uint64_t bytes_sent;            // Bytes sent
    uint64_t bytes_recv;            // Bytes received
} network_peer_info_t;

/**
 * Network Initialization
 */

/**
 * Get default network configuration
 * 
 * @param network Network type
 * @return Default configuration
 */
network_config_t network_default_config(network_type_t network);

/**
 * Initialize network context
 * 
 * @param config Network configuration
 * @return Context or NULL on error
 */
network_context_t* network_init(const network_config_t* config);

/**
 * Free network context
 * 
 * @param ctx Context to free
 */
void network_free(network_context_t* ctx);

/**
 * Test network connection
 * 
 * @param ctx Network context
 * @return true if connected
 */
bool network_test_connection(network_context_t* ctx);

/**
 * Transaction Operations
 */

/**
 * Get transaction by ID
 * 
 * @param ctx Network context
 * @param txid Transaction ID (hex)
 * @return Transaction or NULL on error (caller must free)
 */
network_transaction_t* network_get_transaction(network_context_t* ctx, const char* txid);

/**
 * Broadcast transaction
 * 
 * @param ctx Network context
 * @param raw_tx Raw transaction data
 * @param raw_len Length of raw data
 * @param txid Output buffer for transaction ID (at least 65 bytes)
 * @return NETWORK_OK on success
 */
network_error_t network_broadcast_transaction(network_context_t* ctx,
                                              const uint8_t* raw_tx,
                                              size_t raw_len,
                                              char* txid);

/**
 * Create transaction
 * 
 * @param ctx Network context
 * @param inputs Array of input transaction IDs
 * @param num_inputs Number of inputs
 * @param outputs Array of output addresses
 * @param amounts Array of output amounts (satoshis)
 * @param num_outputs Number of outputs
 * @param raw_tx Output buffer for raw transaction
 * @param raw_len Output length of raw transaction
 * @return NETWORK_OK on success
 */
network_error_t network_create_transaction(network_context_t* ctx,
                                           const char** inputs,
                                           size_t num_inputs,
                                           const char** outputs,
                                           const uint64_t* amounts,
                                           size_t num_outputs,
                                           uint8_t** raw_tx,
                                           size_t* raw_len);

/**
 * Free transaction
 * 
 * @param tx Transaction to free
 */
void network_free_transaction(network_transaction_t* tx);

/**
 * Block Operations
 */

/**
 * Get block by hash
 * 
 * @param ctx Network context
 * @param hash Block hash (hex)
 * @return Block or NULL on error (caller must free)
 */
network_block_t* network_get_block(network_context_t* ctx, const char* hash);

/**
 * Get block by height
 * 
 * @param ctx Network context
 * @param height Block height
 * @return Block or NULL on error (caller must free)
 */
network_block_t* network_get_block_by_height(network_context_t* ctx, uint64_t height);

/**
 * Get best block hash
 * 
 * @param ctx Network context
 * @param hash Output buffer for block hash (at least 65 bytes)
 * @return NETWORK_OK on success
 */
network_error_t network_get_best_block_hash(network_context_t* ctx, char* hash);

/**
 * Get blockchain height
 * 
 * @param ctx Network context
 * @param height Output for blockchain height
 * @return NETWORK_OK on success
 */
network_error_t network_get_blockchain_height(network_context_t* ctx, uint64_t* height);

/**
 * Free block
 * 
 * @param block Block to free
 */
void network_free_block(network_block_t* block);

/**
 * Address Operations
 */

/**
 * Get address info
 * 
 * @param ctx Network context
 * @param address Address string
 * @return Address info or NULL on error (caller must free)
 */
network_address_info_t* network_get_address_info(network_context_t* ctx, const char* address);

/**
 * Validate address
 * 
 * @param ctx Network context
 * @param address Address string
 * @return true if valid
 */
bool network_validate_address(network_context_t* ctx, const char* address);

/**
 * Free address info
 * 
 * @param info Address info to free
 */
void network_free_address_info(network_address_info_t* info);

/**
 * Peer Operations
 */

/**
 * Get connected peers
 * 
 * @param ctx Network context
 * @param peers Output array of peer info (caller must free)
 * @param num_peers Output number of peers
 * @return NETWORK_OK on success
 */
network_error_t network_get_peers(network_context_t* ctx,
                                  network_peer_info_t** peers,
                                  size_t* num_peers);

/**
 * Connect to peer
 * 
 * @param ctx Network context
 * @param address Peer address
 * @param port Peer port
 * @return NETWORK_OK on success
 */
network_error_t network_connect_peer(network_context_t* ctx,
                                     const char* address,
                                     int port);

/**
 * Disconnect from peer
 * 
 * @param ctx Network context
 * @param address Peer address
 * @param port Peer port
 * @return NETWORK_OK on success
 */
network_error_t network_disconnect_peer(network_context_t* ctx,
                                        const char* address,
                                        int port);

/**
 * Free peer info
 * 
 * @param peer Peer info to free
 */
void network_free_peer_info(network_peer_info_t* peer);

/**
 * Mining Operations
 */

/**
 * Get mining info
 * 
 * @param ctx Network context
 * @param difficulty Output for current difficulty
 * @param hashrate Output for network hashrate
 * @return NETWORK_OK on success
 */
network_error_t network_get_mining_info(network_context_t* ctx,
                                        double* difficulty,
                                        double* hashrate);

/**
 * Generate blocks (regtest only)
 * 
 * @param ctx Network context
 * @param num_blocks Number of blocks to generate
 * @param address Address to receive rewards
 * @return NETWORK_OK on success
 */
network_error_t network_generate_blocks(network_context_t* ctx,
                                        int num_blocks,
                                        const char* address);

/**
 * Utility Functions
 */

/**
 * Get error string
 * 
 * @param error Error code
 * @return Error string
 */
const char* network_error_string(network_error_t error);

/**
 * Get network type string
 * 
 * @param network Network type
 * @return Network type string
 */
const char* network_type_string(network_type_t network);

/**
 * Get connection type string
 * 
 * @param connection Connection type
 * @return Connection type string
 */
const char* connection_type_string(connection_type_t connection);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_NETWORK_H
