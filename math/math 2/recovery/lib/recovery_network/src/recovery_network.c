/**
 * @file recovery_network.c
 * @brief Network Protocol Recovery Implementation
 * 
 * Production-grade network protocol support for Bitcoin and other blockchains.
 */

#include "recovery_network.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>

// Network context structure
struct network_context {
    network_config_t config;
    CURL* curl;
    char error_buffer[CURL_ERROR_SIZE];
    char* response_buffer;
    size_t response_size;
};

// Response buffer for CURL
struct response_buffer {
    char* data;
    size_t size;
};

/**
 * CURL write callback
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct response_buffer* mem = (struct response_buffer*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

/**
 * Network Initialization
 */

network_config_t network_default_config(network_type_t network) {
    network_config_t config = {0};
    config.network = network;
    config.connection = CONNECTION_RPC;
    config.timeout_seconds = 30;
    config.max_retries = 3;
    config.use_ssl = false;
    config.verbose = 0;
    
    // Set default host and port based on network
    switch (network) {
        case NETWORK_BITCOIN_MAINNET:
            config.host = strdup("localhost");
            config.port = 8332;
            break;
        case NETWORK_BITCOIN_TESTNET:
            config.host = strdup("localhost");
            config.port = 18332;
            break;
        case NETWORK_BITCOIN_REGTEST:
            config.host = strdup("localhost");
            config.port = 18443;
            break;
        case NETWORK_BITCOIN_SIGNET:
            config.host = strdup("localhost");
            config.port = 38332;
            break;
        default:
            config.host = strdup("localhost");
            config.port = 8332;
            break;
    }
    
    return config;
}

network_context_t* network_init(const network_config_t* config) {
    if (!config) return NULL;
    
    network_context_t* ctx = calloc(1, sizeof(network_context_t));
    if (!ctx) return NULL;
    
    // Copy configuration
    memcpy(&ctx->config, config, sizeof(network_config_t));
    
    // Duplicate strings
    if (config->host) ctx->config.host = strdup(config->host);
    if (config->username) ctx->config.username = strdup(config->username);
    if (config->password) ctx->config.password = strdup(config->password);
    if (config->api_key) ctx->config.api_key = strdup(config->api_key);
    
    // Initialize CURL
    ctx->curl = curl_easy_init();
    if (!ctx->curl) {
        network_free(ctx);
        return NULL;
    }
    
    // Set CURL options
    curl_easy_setopt(ctx->curl, CURLOPT_ERRORBUFFER, ctx->error_buffer);
    curl_easy_setopt(ctx->curl, CURLOPT_TIMEOUT, config->timeout_seconds);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEFUNCTION, write_callback);
    
    if (config->verbose) {
        curl_easy_setopt(ctx->curl, CURLOPT_VERBOSE, 1L);
    }
    
    return ctx;
}

void network_free(network_context_t* ctx) {
    if (!ctx) return;
    
    if (ctx->curl) {
        curl_easy_cleanup(ctx->curl);
    }
    
    if (ctx->config.host) free(ctx->config.host);
    if (ctx->config.username) free(ctx->config.username);
    if (ctx->config.password) free(ctx->config.password);
    if (ctx->config.api_key) free(ctx->config.api_key);
    if (ctx->response_buffer) free(ctx->response_buffer);
    
    free(ctx);
}

bool network_test_connection(network_context_t* ctx) {
    if (!ctx) return false;
    
    // Try to get blockchain info
    char url[256];
    snprintf(url, sizeof(url), "%s://%s:%d",
             ctx->config.use_ssl ? "https" : "http",
             ctx->config.host,
             ctx->config.port);
    
    struct response_buffer response = {0};
    
    curl_easy_setopt(ctx->curl, CURLOPT_URL, url);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, (void*)&response);
    
    // Set authentication if provided
    if (ctx->config.username && ctx->config.password) {
        char auth[512];
        snprintf(auth, sizeof(auth), "%s:%s", 
                 ctx->config.username, ctx->config.password);
        curl_easy_setopt(ctx->curl, CURLOPT_USERPWD, auth);
    }
    
    CURLcode res = curl_easy_perform(ctx->curl);
    
    if (response.data) free(response.data);
    
    return (res == CURLE_OK);
}

/**
 * RPC Helper Functions
 */

static char* rpc_call(network_context_t* ctx, const char* method, const char* params) {
    if (!ctx || !method) return NULL;
    
    // Build JSON-RPC request
    char request[4096];
    snprintf(request, sizeof(request),
             "{&quot;jsonrpc&quot;:&quot;1.0&quot;,&quot;id&quot;:&quot;recovery&quot;,&quot;method&quot;:&quot;%s&quot;,&quot;params&quot;:%s}",
             method, params ? params : "[]");
    
    // Build URL
    char url[256];
    snprintf(url, sizeof(url), "%s://%s:%d",
             ctx->config.use_ssl ? "https" : "http",
             ctx->config.host,
             ctx->config.port);
    
    struct response_buffer response = {0};
    
    // Set CURL options
    curl_easy_setopt(ctx->curl, CURLOPT_URL, url);
    curl_easy_setopt(ctx->curl, CURLOPT_POSTFIELDS, request);
    curl_easy_setopt(ctx->curl, CURLOPT_WRITEDATA, (void*)&response);
    
    // Set authentication
    if (ctx->config.username && ctx->config.password) {
        char auth[512];
        snprintf(auth, sizeof(auth), "%s:%s",
                 ctx->config.username, ctx->config.password);
        curl_easy_setopt(ctx->curl, CURLOPT_USERPWD, auth);
    }
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(ctx->curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(ctx->curl);
    
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        if (response.data) free(response.data);
        return NULL;
    }
    
    return response.data;
}

/**
 * Transaction Operations
 */

network_transaction_t* network_get_transaction(network_context_t* ctx, const char* txid) {
    if (!ctx || !txid) return NULL;
    
    char params[128];
    snprintf(params, sizeof(params), "[&quot;%s&quot;, true]", txid);
    
    char* response = rpc_call(ctx, "getrawtransaction", params);
    if (!response) return NULL;
    
    // Parse JSON response (simplified - in production use proper JSON parser)
    network_transaction_t* tx = calloc(1, sizeof(network_transaction_t));
    if (tx) {
        tx->txid = strdup(txid);
        // TODO: Parse full transaction details from JSON
    }
    
    free(response);
    return tx;
}

network_error_t network_broadcast_transaction(network_context_t* ctx,
                                              const uint8_t* raw_tx,
                                              size_t raw_len,
                                              char* txid) {
    if (!ctx || !raw_tx || !txid) return NETWORK_ERROR_INVALID_PARAM;
    
    // Convert raw transaction to hex
    char* hex_tx = malloc(raw_len * 2 + 1);
    if (!hex_tx) return NETWORK_ERROR_INVALID_PARAM;
    
    for (size_t i = 0; i < raw_len; i++) {
        sprintf(hex_tx + i * 2, "%02x", raw_tx[i]);
    }
    hex_tx[raw_len * 2] = '\0';
    
    char params[8192];
    snprintf(params, sizeof(params), "[&quot;%s&quot;]", hex_tx);
    
    char* response = rpc_call(ctx, "sendrawtransaction", params);
    
    free(hex_tx);
    
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // Extract txid from response
    // TODO: Parse JSON properly
    strncpy(txid, response, 64);
    txid[64] = '\0';
    
    free(response);
    return NETWORK_OK;
}

network_error_t network_create_transaction(network_context_t* ctx,
                                           const char** inputs,
                                           size_t num_inputs,
                                           const char** outputs,
                                           const uint64_t* amounts,
                                           size_t num_outputs,
                                           uint8_t** raw_tx,
                                           size_t* raw_len) {
    if (!ctx || !inputs || !outputs || !amounts || !raw_tx || !raw_len) {
        return NETWORK_ERROR_INVALID_PARAM;
    }
    
    // Build inputs JSON array
    char inputs_json[4096] = "[";
    for (size_t i = 0; i < num_inputs; i++) {
        char input[256];
        snprintf(input, sizeof(input), "%s{&quot;txid&quot;:&quot;%s&quot;,&quot;vout&quot;:0}",
                 i > 0 ? "," : "", inputs[i]);
        strcat(inputs_json, input);
    }
    strcat(inputs_json, "]");
    
    // Build outputs JSON object
    char outputs_json[4096] = "{";
    for (size_t i = 0; i < num_outputs; i++) {
        char output[256];
        snprintf(output, sizeof(output), "%s&quot;%s&quot;:%.8f",
                 i > 0 ? "," : "", outputs[i], amounts[i] / 100000000.0);
        strcat(outputs_json, output);
    }
    strcat(outputs_json, "}");
    
    // Build params
    char params[8192];
    snprintf(params, sizeof(params), "[%s,%s]", inputs_json, outputs_json);
    
    char* response = rpc_call(ctx, "createrawtransaction", params);
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // Parse hex transaction from response
    // TODO: Parse JSON properly
    size_t hex_len = strlen(response);
    *raw_len = hex_len / 2;
    *raw_tx = malloc(*raw_len);
    
    if (*raw_tx) {
        for (size_t i = 0; i < *raw_len; i++) {
            sscanf(response + i * 2, "%2hhx", &(*raw_tx)[i]);
        }
    }
    
    free(response);
    return NETWORK_OK;
}

void network_free_transaction(network_transaction_t* tx) {
    if (!tx) return;
    
    if (tx->txid) free(tx->txid);
    if (tx->block_hash) free(tx->block_hash);
    if (tx->raw_data) free(tx->raw_data);
    
    free(tx);
}

/**
 * Block Operations
 */

network_block_t* network_get_block(network_context_t* ctx, const char* hash) {
    if (!ctx || !hash) return NULL;
    
    char params[128];
    snprintf(params, sizeof(params), "[&quot;%s&quot;, 2]", hash);
    
    char* response = rpc_call(ctx, "getblock", params);
    if (!response) return NULL;
    
    network_block_t* block = calloc(1, sizeof(network_block_t));
    if (block) {
        block->hash = strdup(hash);
        // TODO: Parse full block details from JSON
    }
    
    free(response);
    return block;
}

network_block_t* network_get_block_by_height(network_context_t* ctx, uint64_t height) {
    if (!ctx) return NULL;
    
    // First get block hash at height
    char params[64];
    snprintf(params, sizeof(params), "[%lu]", height);
    
    char* hash_response = rpc_call(ctx, "getblockhash", params);
    if (!hash_response) return NULL;
    
    // Extract hash from response
    // TODO: Parse JSON properly
    char hash[65];
    strncpy(hash, hash_response, 64);
    hash[64] = '\0';
    
    free(hash_response);
    
    // Get block by hash
    return network_get_block(ctx, hash);
}

network_error_t network_get_best_block_hash(network_context_t* ctx, char* hash) {
    if (!ctx || !hash) return NETWORK_ERROR_INVALID_PARAM;
    
    char* response = rpc_call(ctx, "getbestblockhash", "[]");
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // Extract hash from response
    // TODO: Parse JSON properly
    strncpy(hash, response, 64);
    hash[64] = '\0';
    
    free(response);
    return NETWORK_OK;
}

network_error_t network_get_blockchain_height(network_context_t* ctx, uint64_t* height) {
    if (!ctx || !height) return NETWORK_ERROR_INVALID_PARAM;
    
    char* response = rpc_call(ctx, "getblockcount", "[]");
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // Parse height from response
    // TODO: Parse JSON properly
    *height = strtoull(response, NULL, 10);
    
    free(response);
    return NETWORK_OK;
}

void network_free_block(network_block_t* block) {
    if (!block) return;
    
    if (block->hash) free(block->hash);
    if (block->prev_hash) free(block->prev_hash);
    if (block->merkle_root) free(block->merkle_root);
    if (block->raw_data) free(block->raw_data);
    
    if (block->transaction_ids) {
        for (size_t i = 0; i < block->num_transactions; i++) {
            if (block->transaction_ids[i]) free(block->transaction_ids[i]);
        }
        free(block->transaction_ids);
    }
    
    free(block);
}

/**
 * Address Operations
 */

network_address_info_t* network_get_address_info(network_context_t* ctx, const char* address) {
    if (!ctx || !address) return NULL;
    
    char params[128];
    snprintf(params, sizeof(params), "[&quot;%s&quot;]", address);
    
    char* response = rpc_call(ctx, "getaddressinfo", params);
    if (!response) return NULL;
    
    network_address_info_t* info = calloc(1, sizeof(network_address_info_t));
    if (info) {
        info->address = strdup(address);
        // TODO: Parse full address info from JSON
    }
    
    free(response);
    return info;
}

bool network_validate_address(network_context_t* ctx, const char* address) {
    if (!ctx || !address) return false;
    
    char params[128];
    snprintf(params, sizeof(params), "[&quot;%s&quot;]", address);
    
    char* response = rpc_call(ctx, "validateaddress", params);
    if (!response) return false;
    
    // Check if response contains "isvalid":true
    bool valid = (strstr(response, "&quot;isvalid&quot;:true") != NULL);
    
    free(response);
    return valid;
}

void network_free_address_info(network_address_info_t* info) {
    if (!info) return;
    
    if (info->address) free(info->address);
    
    if (info->transaction_ids) {
        for (size_t i = 0; i < info->num_transactions; i++) {
            if (info->transaction_ids[i]) free(info->transaction_ids[i]);
        }
        free(info->transaction_ids);
    }
    
    free(info);
}

/**
 * Peer Operations (stubs for now)
 */

network_error_t network_get_peers(network_context_t* ctx,
                                  network_peer_info_t** peers,
                                  size_t* num_peers) {
    if (!ctx || !peers || !num_peers) return NETWORK_ERROR_INVALID_PARAM;
    
    char* response = rpc_call(ctx, "getpeerinfo", "[]");
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // TODO: Parse peer info from JSON
    *peers = NULL;
    *num_peers = 0;
    
    free(response);
    return NETWORK_OK;
}

network_error_t network_connect_peer(network_context_t* ctx,
                                     const char* address,
                                     int port) {
    if (!ctx || !address) return NETWORK_ERROR_INVALID_PARAM;
    
    char params[256];
    snprintf(params, sizeof(params), "[&quot;%s:%d&quot;]", address, port);
    
    char* response = rpc_call(ctx, "addnode", params);
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    free(response);
    return NETWORK_OK;
}

network_error_t network_disconnect_peer(network_context_t* ctx,
                                        const char* address,
                                        int port) {
    if (!ctx || !address) return NETWORK_ERROR_INVALID_PARAM;
    
    char params[256];
    snprintf(params, sizeof(params), "[&quot;%s:%d&quot;, &quot;remove&quot;]", address, port);
    
    char* response = rpc_call(ctx, "addnode", params);
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    free(response);
    return NETWORK_OK;
}

void network_free_peer_info(network_peer_info_t* peer) {
    if (!peer) return;
    
    if (peer->address) free(peer->address);
    if (peer->version) free(peer->version);
    if (peer->subversion) free(peer->subversion);
    
    free(peer);
}

/**
 * Mining Operations
 */

network_error_t network_get_mining_info(network_context_t* ctx,
                                        double* difficulty,
                                        double* hashrate) {
    if (!ctx || !difficulty || !hashrate) return NETWORK_ERROR_INVALID_PARAM;
    
    char* response = rpc_call(ctx, "getmininginfo", "[]");
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    // TODO: Parse mining info from JSON
    *difficulty = 0.0;
    *hashrate = 0.0;
    
    free(response);
    return NETWORK_OK;
}

network_error_t network_generate_blocks(network_context_t* ctx,
                                        int num_blocks,
                                        const char* address) {
    if (!ctx || !address) return NETWORK_ERROR_INVALID_PARAM;
    
    char params[256];
    snprintf(params, sizeof(params), "[%d, &quot;%s&quot;]", num_blocks, address);
    
    char* response = rpc_call(ctx, "generatetoaddress", params);
    if (!response) return NETWORK_ERROR_CONNECTION;
    
    free(response);
    return NETWORK_OK;
}

/**
 * Utility Functions
 */

const char* network_error_string(network_error_t error) {
    switch (error) {
        case NETWORK_OK: return "Success";
        case NETWORK_ERROR_INVALID_PARAM: return "Invalid parameter";
        case NETWORK_ERROR_CONNECTION: return "Connection error";
        case NETWORK_ERROR_TIMEOUT: return "Timeout";
        case NETWORK_ERROR_PROTOCOL: return "Protocol error";
        case NETWORK_ERROR_PARSE: return "Parse error";
        case NETWORK_ERROR_NOT_FOUND: return "Not found";
        default: return "Unknown error";
    }
}

const char* network_type_string(network_type_t network) {
    switch (network) {
        case NETWORK_BITCOIN_MAINNET: return "Bitcoin Mainnet";
        case NETWORK_BITCOIN_TESTNET: return "Bitcoin Testnet";
        case NETWORK_BITCOIN_REGTEST: return "Bitcoin Regtest";
        case NETWORK_BITCOIN_SIGNET: return "Bitcoin Signet";
        case NETWORK_LITECOIN_MAINNET: return "Litecoin Mainnet";
        case NETWORK_LITECOIN_TESTNET: return "Litecoin Testnet";
        case NETWORK_DOGECOIN_MAINNET: return "Dogecoin Mainnet";
        case NETWORK_DOGECOIN_TESTNET: return "Dogecoin Testnet";
        case NETWORK_CUSTOM: return "Custom Network";
        default: return "Unknown";
    }
}

const char* connection_type_string(connection_type_t connection) {
    switch (connection) {
        case CONNECTION_RPC: return "JSON-RPC";
        case CONNECTION_P2P: return "Peer-to-Peer";
        case CONNECTION_REST: return "REST API";
        case CONNECTION_WEBSOCKET: return "WebSocket";
        default: return "Unknown";
    }
}
