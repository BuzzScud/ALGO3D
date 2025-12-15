#include "../include/ecdsa_sample_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/**
 * Parse hex string to BIGNUM
 */
static BIGNUM* parse_hex_to_bn(const char* hex_str) {
    BIGNUM* bn = BN_new();
    if (!bn) return NULL;
    
    if (!BN_hex2bn(&bn, hex_str)) {
        BN_free(bn);
        return NULL;
    }
    
    return bn;
}

/**
 * Parse hex string to byte array
 */
static void parse_hex_to_bytes(const char* hex_str, unsigned char* bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + (i * 2), "%2hhx", &bytes[i]);
    }
}

/**
 * Load ECDSA sample from file
 */
ECDSASample* load_ecdsa_sample(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }
    
    ECDSASample* sample = (ECDSASample*)calloc(1, sizeof(ECDSASample));
    if (!sample) {
        fclose(f);
        return NULL;
    }
    
    // Store filename
    sample->filename = strdup(filename);
    
    char line[1024];
    char key[64];
    char value[512];
    
    while (fgets(line, sizeof(line), f)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Parse key=value
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "k") == 0) {
                sample->k = parse_hex_to_bn(value);
            } else if (strcmp(key, "r") == 0) {
                sample->r = parse_hex_to_bn(value);
            } else if (strcmp(key, "s") == 0) {
                sample->s = parse_hex_to_bn(value);
            } else if (strcmp(key, "pubkey_x") == 0) {
                sample->pubkey_x = parse_hex_to_bn(value);
            } else if (strcmp(key, "pubkey_y") == 0) {
                sample->pubkey_y = parse_hex_to_bn(value);
            } else if (strcmp(key, "message") == 0) {
                parse_hex_to_bytes(value, sample->message, 32);
            } else if (strcmp(key, "bit_length") == 0) {
                sample->bit_length = atoi(value);
            } else if (strcmp(key, "curve_nid") == 0) {
                sample->curve_nid = atoi(value);
            }
        }
    }
    
    fclose(f);
    
    // Verify all fields loaded
    if (!sample->k || !sample->r || !sample->s || 
        !sample->pubkey_x || !sample->pubkey_y) {
        fprintf(stderr, "Incomplete sample in file: %s\n", filename);
        free_ecdsa_sample(sample);
        return NULL;
    }
    
    return sample;
}

/**
 * Load all samples from directory
 */
ECDSASample** load_all_samples(const char* directory, int* num_samples) {
    DIR* dir = opendir(directory);
    if (!dir) {
        fprintf(stderr, "Failed to open directory: %s\n", directory);
        *num_samples = 0;
        return NULL;
    }
    
    // Count .txt files
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt")) {
            count++;
        }
    }
    rewinddir(dir);
    
    if (count == 0) {
        closedir(dir);
        *num_samples = 0;
        return NULL;
    }
    
    // Allocate array
    ECDSASample** samples = (ECDSASample**)calloc(count, sizeof(ECDSASample*));
    if (!samples) {
        closedir(dir);
        *num_samples = 0;
        return NULL;
    }
    
    // Load samples
    int idx = 0;
    while ((entry = readdir(dir)) != NULL && idx < count) {
        if (strstr(entry->d_name, ".txt")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
            
            ECDSASample* sample = load_ecdsa_sample(filepath);
            if (sample) {
                samples[idx++] = sample;
            }
        }
    }
    
    closedir(dir);
    *num_samples = idx;
    return samples;
}

/**
 * Load samples for specific bit length
 */
ECDSASample** load_samples_by_bit_length(
    const char* directory,
    int bit_length,
    int* num_samples
) {
    // Load all samples first
    int total_samples = 0;
    ECDSASample** all_samples = load_all_samples(directory, &total_samples);
    
    if (!all_samples || total_samples == 0) {
        *num_samples = 0;
        return NULL;
    }
    
    // Count matching samples
    int count = 0;
    for (int i = 0; i < total_samples; i++) {
        if (all_samples[i]->bit_length == bit_length) {
            count++;
        }
    }
    
    if (count == 0) {
        free_ecdsa_samples(all_samples, total_samples);
        *num_samples = 0;
        return NULL;
    }
    
    // Create filtered array
    ECDSASample** filtered = (ECDSASample**)calloc(count, sizeof(ECDSASample*));
    if (!filtered) {
        free_ecdsa_samples(all_samples, total_samples);
        *num_samples = 0;
        return NULL;
    }
    
    // Copy matching samples
    int idx = 0;
    for (int i = 0; i < total_samples; i++) {
        if (all_samples[i]->bit_length == bit_length) {
            filtered[idx++] = all_samples[i];
        } else {
            free_ecdsa_sample(all_samples[i]);
        }
    }
    
    free(all_samples);
    *num_samples = count;
    return filtered;
}

/**
 * Load samples for specific curve
 */
ECDSASample** load_samples_by_curve(
    const char* directory,
    int curve_nid,
    int* num_samples
) {
    // Load all samples first
    int total_samples = 0;
    ECDSASample** all_samples = load_all_samples(directory, &total_samples);
    
    if (!all_samples || total_samples == 0) {
        *num_samples = 0;
        return NULL;
    }
    
    // Count matching samples
    int count = 0;
    for (int i = 0; i < total_samples; i++) {
        if (all_samples[i]->curve_nid == curve_nid) {
            count++;
        }
    }
    
    if (count == 0) {
        free_ecdsa_samples(all_samples, total_samples);
        *num_samples = 0;
        return NULL;
    }
    
    // Create filtered array
    ECDSASample** filtered = (ECDSASample**)calloc(count, sizeof(ECDSASample*));
    if (!filtered) {
        free_ecdsa_samples(all_samples, total_samples);
        *num_samples = 0;
        return NULL;
    }
    
    // Copy matching samples
    int idx = 0;
    for (int i = 0; i < total_samples; i++) {
        if (all_samples[i]->curve_nid == curve_nid) {
            filtered[idx++] = all_samples[i];
        } else {
            free_ecdsa_sample(all_samples[i]);
        }
    }
    
    free(all_samples);
    *num_samples = count;
    return filtered;
}

/**
 * Free ECDSA sample
 */
void free_ecdsa_sample(ECDSASample* sample) {
    if (!sample) return;
    
    if (sample->k) BN_free(sample->k);
    if (sample->r) BN_free(sample->r);
    if (sample->s) BN_free(sample->s);
    if (sample->pubkey_x) BN_free(sample->pubkey_x);
    if (sample->pubkey_y) BN_free(sample->pubkey_y);
    if (sample->filename) free(sample->filename);
    
    free(sample);
}

/**
 * Free array of samples
 */
void free_ecdsa_samples(ECDSASample** samples, int num_samples) {
    if (!samples) return;
    
    for (int i = 0; i < num_samples; i++) {
        free_ecdsa_sample(samples[i]);
    }
    
    free(samples);
}

/**
 * Print sample info
 */
void print_ecdsa_sample(const ECDSASample* sample) {
    if (!sample) return;
    
    printf("ECDSA Sample:\n");
    printf("  File: %s\n", sample->filename ? sample->filename : "N/A");
    printf("  Bit length: %d\n", sample->bit_length);
    printf("  Curve NID: %d (%s)\n", sample->curve_nid,
           sample->curve_nid == 714 ? "secp256k1" : "secp192k1");
    
    if (sample->k) {
        char* k_hex = BN_bn2hex(sample->k);
        printf("  k: %s\n", k_hex);
        OPENSSL_free(k_hex);
    }
    
    if (sample->r) {
        char* r_hex = BN_bn2hex(sample->r);
        printf("  r: %.*s...\n", 32, r_hex);
        OPENSSL_free(r_hex);
    }
    
    if (sample->s) {
        char* s_hex = BN_bn2hex(sample->s);
        printf("  s: %.*s...\n", 32, s_hex);
        OPENSSL_free(s_hex);
    }
    
    printf("  Message: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x", sample->message[i]);
    }
    printf("...\n");
}

/**
 * Verify sample signature (placeholder - would need full EC implementation)
 */
bool verify_ecdsa_sample(const ECDSASample* sample) {
    // This would require full ECDSA verification
    // For now, just check that all fields are present
    return (sample && sample->k && sample->r && sample->s &&
            sample->pubkey_x && sample->pubkey_y);
}

/**
 * Get sample statistics
 */
void print_sample_statistics(ECDSASample** samples, int num_samples) {
    if (!samples || num_samples == 0) {
        printf("No samples loaded.\n");
        return;
    }
    
    printf("\n=== Sample Statistics ===\n");
    printf("Total samples: %d\n\n", num_samples);
    
    // Count by bit length
    int bit_length_counts[17] = {0};  // 8, 16, 24, ..., 256
    int bit_lengths[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256};
    
    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < 16; j++) {
            if (samples[i]->bit_length == bit_lengths[j]) {
                bit_length_counts[j]++;
                break;
            }
        }
    }
    
    printf("Samples by bit length:\n");
    for (int i = 0; i < 16; i++) {
        if (bit_length_counts[i] > 0) {
            printf("  %3d-bit: %3d samples\n", bit_lengths[i], bit_length_counts[i]);
        }
    }
    
    // Count by curve
    int secp256k1_count = 0;
    int secp192k1_count = 0;
    
    for (int i = 0; i < num_samples; i++) {
        if (samples[i]->curve_nid == 714) {
            secp256k1_count++;
        } else {
            secp192k1_count++;
        }
    }
    
    printf("\nSamples by curve:\n");
    printf("  secp256k1: %d samples\n", secp256k1_count);
    printf("  secp192k1: %d samples\n", secp192k1_count);
    printf("\n");
}
