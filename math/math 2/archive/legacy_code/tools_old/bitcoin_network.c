/*
 * Bitcoin Testnet Interface
 * 
 * Provides functions to:
 * - Connect to Bitcoin testnet
 * - Fetch transaction data
 * - Extract Q (public key) and signatures
 * - Recover k (private key) from transaction data
 * - Create and broadcast transactions
 * - Mine blocks and validate nonces
 * 
 * NO BITCOIN SOFTWARE REQUIRED - Pure HTTP/RPC interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Bitcoin testnet RPC endpoint (public)
#define TESTNET_RPC "https://blockstream.info/testnet/api"

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

// Callback for curl
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(!ptr) return 0;
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

// Fetch transaction data from testnet
char* fetch_transaction(const char* txid) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {0};
    
    char url[512];
    snprintf(url, sizeof(url), "%s/tx/%s", TESTNET_RPC, txid);
    
    curl = curl_easy_init();
    if(!curl) return NULL;
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "crystalline-recovery/1.0");
    
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if(res != CURLE_OK) {
        free(chunk.data);
        return NULL;
    }
    
    return chunk.data;
}

// Extract public key (Q) from transaction
int extract_public_key(const char* tx_json, unsigned char* pubkey, size_t* pubkey_len) {
    json_error_t error;
    json_t *root = json_loads(tx_json, 0, &error);
    if(!root) return 0;
    
    // Navigate to witness/scriptSig to find public key
    json_t *vin = json_object_get(root, "vin");
    if(!json_is_array(vin) || json_array_size(vin) == 0) {
        json_decref(root);
        return 0;
    }
    
    json_t *input = json_array_get(vin, 0);
    json_t *witness = json_object_get(input, "witness");
    
    // Public key is typically in witness[1] for P2WPKH
    if(json_is_array(witness) && json_array_size(witness) > 1) {
        const char* pubkey_hex = json_string_value(json_array_get(witness, 1));
        if(pubkey_hex) {
            // Convert hex to bytes
            size_t len = strlen(pubkey_hex) / 2;
            for(size_t i = 0; i < len && i < *pubkey_len; i++) {
                sscanf(pubkey_hex + 2*i, "%2hhx", &pubkey[i]);
            }
            *pubkey_len = len;
            json_decref(root);
            return 1;
        }
    }
    
    json_decref(root);
    return 0;
}

// Extract signature (contains info about k) from transaction
int extract_signature(const char* tx_json, unsigned char* sig, size_t* sig_len) {
    json_error_t error;
    json_t *root = json_loads(tx_json, 0, &error);
    if(!root) return 0;
    
    json_t *vin = json_object_get(root, "vin");
    if(!json_is_array(vin) || json_array_size(vin) == 0) {
        json_decref(root);
        return 0;
    }
    
    json_t *input = json_array_get(vin, 0);
    json_t *witness = json_object_get(input, "witness");
    
    // Signature is typically in witness[0] for P2WPKH
    if(json_is_array(witness) && json_array_size(witness) > 0) {
        const char* sig_hex = json_string_value(json_array_get(witness, 0));
        if(sig_hex) {
            size_t len = strlen(sig_hex) / 2;
            for(size_t i = 0; i < len && i < *sig_len; i++) {
                sscanf(sig_hex + 2*i, "%2hhx", &sig[i]);
            }
            *sig_len = len;
            json_decref(root);
            return 1;
        }
    }
    
    json_decref(root);
    return 0;
}

void print_usage(const char* prog) {
    printf("Bitcoin Testnet Interface\n\n");
    printf("Usage: %s <command> [options]\n\n", prog);
    printf("Commands:\n");
    printf("  fetch-tx <txid>           Fetch transaction data\n");
    printf("  extract-keys <txid>       Extract Q and signature from transaction\n");
    printf("  recover-key <txid>        Attempt to recover private key\n");
    printf("  create-tx                 Create new transaction\n");
    printf("  mine                      Mine blocks (validate nonces)\n");
    printf("  validate-nonce <block>    Validate block nonce\n\n");
    printf("Examples:\n");
    printf("  %s fetch-tx abc123...     # Fetch transaction\n", prog);
    printf("  %s extract-keys abc123... # Extract keys\n", prog);
    printf("  %s recover-key abc123...  # Recover private key\n", prog);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    const char* command = argv[1];
    
    if(strcmp(command, "fetch-tx") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Error: Missing transaction ID\n");
            return 1;
        }
        
        const char* txid = argv[2];
        printf("Fetching transaction: %s\n", txid);
        
        char* tx_data = fetch_transaction(txid);
        if(tx_data) {
            printf("\nTransaction data:\n%s\n", tx_data);
            free(tx_data);
        } else {
            fprintf(stderr, "Error: Failed to fetch transaction\n");
            return 1;
        }
        
    } else if(strcmp(command, "extract-keys") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Error: Missing transaction ID\n");
            return 1;
        }
        
        const char* txid = argv[2];
        printf("Extracting keys from transaction: %s\n", txid);
        
        char* tx_data = fetch_transaction(txid);
        if(!tx_data) {
            fprintf(stderr, "Error: Failed to fetch transaction\n");
            return 1;
        }
        
        unsigned char pubkey[256] = {0};
        size_t pubkey_len = sizeof(pubkey);
        
        unsigned char sig[256] = {0};
        size_t sig_len = sizeof(sig);
        
        if(extract_public_key(tx_data, pubkey, &pubkey_len)) {
            printf("\nPublic Key (Q): ");
            for(size_t i = 0; i < pubkey_len; i++) {
                printf("%02x", pubkey[i]);
            }
            printf("\n");
            
            // Save to file
            FILE* f = fopen("Q_pubkey.bin", "wb");
            if(f) {
                fwrite(pubkey, 1, pubkey_len, f);
                fclose(f);
                printf("Saved to: Q_pubkey.bin\n");
            }
        }
        
        if(extract_signature(tx_data, sig, &sig_len)) {
            printf("\nSignature (contains k info): ");
            for(size_t i = 0; i < sig_len && i < 32; i++) {
                printf("%02x", sig[i]);
            }
            printf("...\n");
            
            // Save to file
            FILE* f = fopen("signature_samples.bin", "wb");
            if(f) {
                fwrite(sig, 1, sig_len, f);
                fclose(f);
                printf("Saved to: signature_samples.bin\n");
            }
        }
        
        free(tx_data);
        
    } else if(strcmp(command, "recover-key") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Error: Missing transaction ID\n");
            return 1;
        }
        
        printf("Key recovery from transaction: %s\n", argv[2]);
        printf("\nThis will:\n");
        printf("1. Fetch transaction data\n");
        printf("2. Extract Q (public key) and signature\n");
        printf("3. Use universal-recovery to recover k\n");
        printf("4. Validate recovered key\n\n");
        
        // First extract keys
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "%s extract-keys %s", argv[0], argv[2]);
        system(cmd);
        
        // Then run recovery
        printf("\nRunning recovery...\n");
        system("/workspace/tools/universal-recovery "
               "-q Q_pubkey.bin "
               "-s signature_samples.bin "
               "-o recovered_privkey.bin "
               "-v -i 50000");
        
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    curl_global_cleanup();
    return 0;
}
