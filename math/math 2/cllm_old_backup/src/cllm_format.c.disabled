/*
 * Crystalline Lattice Language Model (CLLM) Format Implementation
 * Revolutionary LLM format based on prime number lattices
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/cllm_format.h"
#include "../include/cllm_utils.h"
#include "../include/prime_float_math.h"
#include "../include/clock_lattice.h"  // For validate_prime_by_clock_position()

// Use GOLDEN_RATIO from prime_types.h (included via cllm_format.h)
#ifndef GOLDEN_RATIO
#define GOLDEN_RATIO 1.618033988749895
#endif
#define SYMMETRY_ORDER 12
#define PI 3.14159265358979323846

// Initialize CLLM header
void cllm_header_init(CLLMHeader* header, const char* model_name, const char* description) {
    memset(header, 0, sizeof(CLLMHeader));
    
    // Magic number
    memcpy(header->magic, "CLLM\x01\x00\x00\x00", 8);
    
    // Version and architecture
    header->version = 1;
    header->architecture = 1;  // Transformer-based
    
    // Model parameters (defaults)
    header->vocab_size = 50000;
    header->embedding_dim = 768;
    header->num_layers = 12;
    header->num_heads = 12;
    header->context_length = 2048;
    
    // Lattice parameters
    header->symmetry_order = SYMMETRY_ORDER;
    header->golden_ratio = GOLDEN_RATIO;
    
    // Metadata
    header->timestamp = time(NULL);
    strncpy(header->model_name, model_name, sizeof(header->model_name) - 1);
    strncpy(header->description, description, sizeof(header->description) - 1);
}

// Calculate lattice coordinates from prime number
void cllm_prime_to_lattice(uint64_t prime, float coords[3], float* angle, float* radius) {
    // Use prime factorization properties to determine position
    uint64_t p = prime;
    
    // Angle based on prime modulo symmetry order
    *angle = (2.0 * PI * (p % SYMMETRY_ORDER)) / SYMMETRY_ORDER;
    
    // Radius based on golden ratio spiral
    *radius = prime_powf(GOLDEN_RATIO, prime_logf(p) / prime_logf(2.0));
    
    // 3D coordinates
    coords[0] = *radius * prime_cosf(*angle);
    coords[1] = *radius * prime_sinf(*angle);
    coords[2] = prime_logf(p) / prime_logf(GOLDEN_RATIO);  // Height based on prime magnitude
}

// REMOVED: Local is_prime() implementation
// Internal code trusts the deterministic clock lattice structure
// Use validate_prime_by_clock_position() directly

// Find nearest prime for a given number
uint64_t cllm_nearest_prime(uint64_t n) {
    if (n < 2) return 2;
    
    // Search upward for next prime
    // Internal: Trust deterministic clock lattice
    while (!validate_prime_by_clock_position(n)) n++;
    return n;
}

// Encode token as prime number
uint64_t cllm_token_to_prime(uint32_t token_id) {
    // Map token ID to prime number space
    // Use nth prime formula approximation
    if (token_id == 0) return 2;
    
    // Approximate nth prime: p_n ≈ n * ln(n)
    uint64_t approx = token_id * (uint64_t)(prime_logf(token_id + 1) + 1);
    return cllm_nearest_prime(approx);
}

// Create token entry
void cllm_token_create(CLLMToken* token, uint32_t token_id, const char* token_str) {
    memset(token, 0, sizeof(CLLMToken));
    
    token->token_id = token_id;
    token->prime_encoding = cllm_token_to_prime(token_id);
    
    // Calculate lattice position
    cllm_prime_to_lattice(token->prime_encoding, token->lattice_coords, 
                         &token->spiral_angle, &token->radial_distance);
    
    // Determine symmetry group
    token->symmetry_group = token->prime_encoding % SYMMETRY_ORDER;
    
    // Copy token string
    strncpy(token->token_str, token_str, sizeof(token->token_str) - 1);
}

// Create lattice point
void cllm_lattice_point_create(CLLMLatticePoint* point, uint32_t point_id, 
                               float x, float y, float z, uint32_t prime) {
    memset(point, 0, sizeof(CLLMLatticePoint));
    
    point->point_id = point_id;
    point->coords[0] = x;
    point->coords[1] = y;
    point->coords[2] = z;
    point->prime_factor = prime;
    point->symmetry_group = prime % SYMMETRY_ORDER;
    
    // Calculate resonance (based on prime properties)
    point->resonance = 1.0 / prime_logf(prime + 1);
}

// Calculate distance between lattice points
float cllm_lattice_distance(const CLLMLatticePoint* p1, const CLLMLatticePoint* p2) {
    float dx = p1->coords[0] - p2->coords[0];
    float dy = p1->coords[1] - p2->coords[1];
    float dz = p1->coords[2] - p2->coords[2];
    return prime_sqrtf(dx*dx + dy*dy + dz*dz);
}

// Find neighbors for a lattice point
void cllm_lattice_find_neighbors(CLLMLatticePoint* point, CLLMLatticePoint* all_points, 
                                 int num_points, float max_distance) {
    point->neighbor_count = 0;
    
    for (int i = 0; i < num_points && point->neighbor_count < 12; i++) {
        if (all_points[i].point_id == point->point_id) continue;
        
        float dist = cllm_lattice_distance(point, &all_points[i]);
        if (dist <= max_distance) {
            point->neighbors[point->neighbor_count++] = all_points[i].point_id;
        }
    }
}

// Write CLLM file

// Read CLLM file

// Validate CLLM file
bool cllm_validate(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error reading from file\n");
        fclose(f);
        return NULL;

    }
    fclose(f);
    
    // Check magic number
    if (memcmp(header.magic, "CLLM\x01\x00\x00\x00", 8) != 0) {
        return false;
    }
    
    // Check version
    if (header.version != 1) {
        return false;
    }
    
    // Check symmetry order
    if (header.symmetry_order != SYMMETRY_ORDER) {
        return false;
    }
    
    return true;
}

// Free CLLM model
void cllm_free(CLLMModel* model) {
    if (!model) return;
    
    free(model->tokens);
    free(model->lattice_points);
    
    // CRITICAL FIX: Don't free individual weight pointers!
    // All weights (embeddings, attention, FF, layer norms) are allocated
    // as a single block in model->weights. Only free the main weights array.
    // The individual pointers (embeddings.embeddings, attention_layers[i].query_lattice, etc.)
    // are just offsets into this single allocation.
    
    // Free the main weights array (contains all model parameters)
    free(model->weights);
    
    // Free transform matrices if they were separately allocated
    if (model->embeddings.lattice_transform && 
        model->embeddings.lattice_transform != model->weights) {
        free(model->embeddings.lattice_transform);
    }
    if (model->embeddings.inverse_transform && 
        model->embeddings.inverse_transform != model->weights) {
        free(model->embeddings.inverse_transform);
    }
    
    // Free layer structure arrays (but not the weights they point to)
    free(model->attention_layers);
    free(model->ff_layers);
    free(model->layer_norms);
    
    // Free positional encoding arrays if separately allocated
    free(model->pos_encoding.spiral_positions);
    free(model->pos_encoding.clock_positions);
    free(model->pos_encoding.prime_positions);
    free(model->pos_encoding.learned_positions);
    
    free(model);
}

/**
 * Validate CLLM Header
 */
static bool cllm_validate_header(const CLLMHeader* header) {
    if (!header) return false;
    if (strncmp(header->magic, "CLLM", 4) != 0) return false;
    if (header->version == 0 || header->version > 100) return false;
    if (header->vocab_size == 0 || header->vocab_size > 1000000) return false;
    if (header->embedding_dim == 0 || header->embedding_dim > 10000) return false;
    if (header->num_layers == 0 || header->num_layers > 100) return false;
    return true;
}

/**
 * Read CLLM Model from File
 * 
 * Loads a complete model from disk including all weights and configuration.
 */
CLLMModel* cllm_read_model(const char* filepath) {
    if (!filepath) return NULL;
    
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open model file: %s\n", filepath);
        return NULL;
    }
    
    // Read and validate header
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, file) != 1) {
        fprintf(stderr, "Failed to read model header\n");
        fclose(file);
        return NULL;
    }
    
    if (!cllm_validate_header(&header)) {
        fprintf(stderr, "Invalid model header\n");
        fclose(file);
        return NULL;
    }
    
    // Create model configuration from header
    CLLMConfig config = {
        .vocab_size = header.vocab_size,
        .embedding_dim = header.embedding_dim,
        .num_layers = header.num_layers,
        .num_heads = header.num_heads,
        .ff_dim = header.embedding_dim * 4,  // Standard transformer ratio
        .max_seq_len = header.context_length,
        .dropout = 0.1f
    };
    
    // Create model structure
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Failed to create model structure\n");
        fclose(file);
        return NULL;
    }
    
    // Read embeddings - CRITICAL: This overwrites the NaN values from lazy init
    if (model->embeddings) {
        size_t emb_size = model->vocab_size * model->embedding_dim;
        printf("DEBUG: About to read %zu embeddings from file\n", emb_size);
        printf("DEBUG: First embedding before read: %f\n", model->embeddings[0]);
        
        size_t read_count = fread(model->embeddings, sizeof(double), emb_size, file);
        printf("DEBUG: Read %zu embeddings (expected %zu)\n", read_count, emb_size);
        printf("DEBUG: First embedding after read: %f\n", model->embeddings[0]);
        
        if (read_count != emb_size) {
            fprintf(stderr, "Failed to read embeddings: got %zu, expected %zu\n", read_count, emb_size);
            cllm_free(model);
            fclose(file);
            return NULL;
        }
        
        // Verify embeddings are not NaN after loading
        int nan_count = 0;
        for (size_t i = 0; i < emb_size && nan_count < 10; i++) {
            if (prime_isnan(model->embeddings[i])) {
                nan_count++;
                fprintf(stderr, "WARNING: NaN in loaded embedding at index %zu\n", i);
            }
        }
        if (nan_count > 0) {
            fprintf(stderr, "ERROR: Loaded model has %d NaN embeddings after file read!\n", nan_count);
        } else {
            printf("✓ All embeddings loaded successfully (no NaN)\n");
        }
        printf("  Loaded embeddings: %zu doubles\n", emb_size);
    }
    
    // Read lattice transforms
    if (model->embeddings.lattice_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        size_t read = fread(model->embeddings.lattice_transform, sizeof(double), transform_size, file);
        if (read != transform_size) {
            fprintf(stderr, "Warning: Expected %zu floats, read %zu for lattice_transform\n", transform_size, read);
        }
    }
    if (model->embeddings.inverse_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        size_t read = fread(model->embeddings.inverse_transform, sizeof(double), transform_size, file);
        if (read != transform_size) {
            fprintf(stderr, "Warning: Expected %zu floats, read %zu for inverse_transform\n", transform_size, read);
        }
    }
    
    // Read attention layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* attn = &model->attention_layers[i];
        uint32_t d_model = attn->num_heads * attn->head_dim;
        
        if (attn->query_lattice) {
            size_t read = fread(attn->query_lattice, sizeof(double), d_model * d_model, file);
            (void)read; // Suppress warning
        }
        if (attn->key_lattice) {
            size_t read = fread(attn->key_lattice, sizeof(double), d_model * d_model, file);
            (void)read;
        }
        if (attn->value_lattice) {
            size_t read = fread(attn->value_lattice, sizeof(double), d_model * d_model, file);
            (void)read;
        }
    }
    
    // Read feedforward layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        FeedForwardLayer* ff = &model->ff_layers[i];
        
        if (ff->w1_lattice) {
            size_t read = fread(ff->w1_lattice, sizeof(double), ff->input_dim * ff->hidden_dim, file);
            (void)read;
        }
        if (ff->bias1) {
            size_t read = fread(ff->bias1, sizeof(double), ff->hidden_dim, file);
            (void)read;
        }
        if (ff->w2_lattice) {
            size_t read = fread(ff->w2_lattice, sizeof(double), ff->hidden_dim * ff->output_dim, file);
            (void)read;
        }
        if (ff->bias2) {
            size_t read = fread(ff->bias2, sizeof(double), ff->output_dim, file);
            (void)read;
        }
    }
    
    // CRITICAL FIX: Read vocabulary tokens if present
    // Check for vocabulary marker
    uint32_t vocab_marker = 0;
    if (fread(&vocab_marker, sizeof(uint32_t), 1, file) == 1 && vocab_marker == 0x564F4301) {
        // Read number of tokens
        uint32_t num_tokens = 0;
        if (fread(&num_tokens, sizeof(uint32_t), 1, file) == 1 && num_tokens > 0) {
            // Allocate tokens array
            model->tokens = (CLLMToken*)calloc(num_tokens, sizeof(CLLMToken));
            if (model->tokens) {
                // Read each token
                for (uint32_t i = 0; i < num_tokens; i++) {
                    // Read token_id
                    fread(&model->tokens[i].token_id, sizeof(uint32_t), 1, file);
                    
                    // Read token string
                    uint32_t len = 0;
                    fread(&len, sizeof(uint32_t), 1, file);
                    if (len < sizeof(model->tokens[i].token_str)) {
                        fread(model->tokens[i].token_str, sizeof(char), len, file);
                        model->tokens[i].token_str[len] = '\0';
                    } else {
                        // Token too long, skip it
                        fseek(file, len, SEEK_CUR);
                        snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
                    }
                    
                    // Read frequency
                    fread(&model->tokens[i].frequency, sizeof(float), 1, file);
                    
                    // Read prime_encoding
                    fread(&model->tokens[i].prime_encoding, sizeof(uint64_t), 1, file);
                    
                    // Read symmetry_group
                    fread(&model->tokens[i].symmetry_group, sizeof(uint32_t), 1, file);
                }
                
                printf("✓ Loaded vocabulary: %u tokens\n", num_tokens);
            }
        }
    } else {
        // No vocabulary in file, use default tokens
        printf("Warning: No vocabulary found in model file, using default tokens\n");
        model->tokens = (CLLMToken*)calloc(model->vocab_size, sizeof(CLLMToken));
        if (model->tokens) {
            for (uint32_t i = 0; i < model->vocab_size; i++) {
                model->tokens[i].token_id = i;
                snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "token_%u", i);
                model->tokens[i].frequency = 0;
                model->tokens[i].symmetry_group = i % 12;
            }
        }
    }
    
    fclose(file);
    printf("✓ Model loaded: %s\n", filepath);
    printf("  Vocab: %lu | Embedding: %lu | Layers: %lu\n",
           (unsigned long)header.vocab_size, (unsigned long)header.embedding_dim, 
           (unsigned long)header.num_layers);
    
    return model;
}

/**
 * Write CLLM Model to File
 */
int cllm_write_model(const CLLMModel* model, const char* filepath) {
    if (!model || !filepath) return -1;
    
    FILE* file = fopen(filepath, "wb");
    if (!file) {
        fprintf(stderr, "Failed to create model file: %s\n", filepath);
        return -1;
    }
    
    // Create header
    CLLMHeader header;
    memset(&header, 0, sizeof(CLLMHeader));
    memcpy(header.magic, "CLLM\x01\x00\x00\x00", 8);  // Full 8-byte magic number
    header.version = 1;
    header.vocab_size = model->vocab_size;
    header.embedding_dim = model->embedding_dim;
    header.num_layers = model->num_layers;
    header.num_heads = 8;  // Default value
    header.context_length = 512;  // Default value
    
    // Use the model's actual weight count instead of recalculating
    header.total_params = model->num_weights;
    
    // Write header
    if (fwrite(&header, sizeof(CLLMHeader), 1, file) != 1) {
        fprintf(stderr, "Failed to write header\n");
        fclose(file);
        return -1;
    }
    
    // Write embeddings
    if (model->embeddings) {
        size_t emb_size = model->vocab_size * model->embedding_dim;
        if (fwrite(model->embeddings, sizeof(double), emb_size, file) != emb_size) {
            fprintf(stderr, "Failed to write embeddings\n");
            fclose(file);
            return -1;
        }
        printf("  Saved embeddings: %zu floats\n", emb_size);
    }
    
    // Write lattice transforms
    if (model->embeddings.lattice_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        fwrite(model->embeddings.lattice_transform, sizeof(double), transform_size, file);
    }
    if (model->embeddings.inverse_transform) {
        size_t transform_size = model->embedding_dim * model->embedding_dim;
        fwrite(model->embeddings.inverse_transform, sizeof(double), transform_size, file);
    }
    
    // Write attention layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        AttentionLayer* attn = &model->attention_layers[i];
        uint32_t d_model = attn->num_heads * attn->head_dim;
        
        if (attn->query_lattice) fwrite(attn->query_lattice, sizeof(double), d_model * d_model, file);
        if (attn->key_lattice) fwrite(attn->key_lattice, sizeof(double), d_model * d_model, file);
        if (attn->value_lattice) fwrite(attn->value_lattice, sizeof(double), d_model * d_model, file);
    }
    
    // Write feedforward layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        FeedForwardLayer* ff = &model->ff_layers[i];
        
        if (ff->w1_lattice) fwrite(ff->w1_lattice, sizeof(double), ff->input_dim * ff->hidden_dim, file);
        if (ff->bias1) fwrite(ff->bias1, sizeof(double), ff->hidden_dim, file);
        if (ff->w2_lattice) fwrite(ff->w2_lattice, sizeof(double), ff->hidden_dim * ff->output_dim, file);
        if (ff->bias2) fwrite(ff->bias2, sizeof(double), ff->output_dim, file);
    }
    
    // CRITICAL FIX: Write vocabulary tokens
    // This is essential for inference to work properly
    if (model->tokens && model->vocab_size > 0) {
        // Write vocabulary marker
        uint32_t vocab_marker = 0x564F4301;  // Magic marker "VOC\x01" for vocabulary section
        fwrite(&vocab_marker, sizeof(uint32_t), 1, file);
        
        // Write number of tokens
        fwrite(&model->vocab_size, sizeof(uint32_t), 1, file);
        
        // Write each token
        for (uint32_t i = 0; i < model->vocab_size; i++) {
            // Write token_id
            fwrite(&model->tokens[i].token_id, sizeof(uint32_t), 1, file);
            
            // Write token string length and string
            uint32_t len = strlen(model->tokens[i].token_str);
            fwrite(&len, sizeof(uint32_t), 1, file);
            fwrite(model->tokens[i].token_str, sizeof(char), len, file);
            
            // Write frequency
            fwrite(&model->tokens[i].frequency, sizeof(float), 1, file);
            
            // Write prime_encoding
            fwrite(&model->tokens[i].prime_encoding, sizeof(uint64_t), 1, file);
            
            // Write symmetry_group
            fwrite(&model->tokens[i].symmetry_group, sizeof(uint32_t), 1, file);
        }
        
        printf("  Saved vocabulary: %u tokens\n", (unsigned int)model->vocab_size);
    }
    
    fclose(file);
    printf("✓ Model saved: %s\n", filepath);
    printf("  Saved %u layers with embeddings\n", model->num_layers);
    return 0;
}