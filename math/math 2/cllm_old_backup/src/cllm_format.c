/**
 * @file cllm_format.c
 * @brief Model File I/O for Geometric CLLMModel
 * 
 * Implements save/load functionality for the new geometric CLLMModel structure
 * with Platonic solid foundation, clock lattice mapping, and revolutionary features.
 */

#include "cllm.h"
#include "cllm_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Version for geometric model
#define CLLM_VERSION 2  // Version 2 for geometric model

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Validate header
 */
static bool validate_header(const CLLMHeader* header) {
    if (!header) return false;
    
    // Check magic number
    if (header->magic[0] != 'C' || header->magic[1] != 'L' || 
        header->magic[2] != 'L' || header->magic[3] != 'M') {
        fprintf(stderr, "Invalid magic number\n");
        return false;
    }
    
    // Check version
    if (header->version != CLLM_VERSION) {
        fprintf(stderr, "Unsupported version: %u (expected %u)\n", 
                header->version, CLLM_VERSION);
        return false;
    }
    
    // Check basic parameters
    if (header->vocab_size == 0 || header->embedding_dim == 0 || 
        header->num_layers == 0) {
        fprintf(stderr, "Invalid model parameters\n");
        return false;
    }
    
    return true;
}

// ============================================================================
// WRITE MODEL
// ============================================================================

/**
 * Write model to file
 * 
 * File format:
 * 1. Header (CLLMHeader)
 * 2. Geometric foundation (PlatonicGeometry)
 * 3. Clock lattice positions (vertex_positions, token_positions, angular_positions)
 * 4. Model parameters (embeddings, layers, output)
 * 5. Feature states (blind recovery, harmonic, NTT, threading)
 * 6. Optimizer state
 * 7. Metrics
 */
int cllm_write_model(const CLLMModel* model, const char* filename) {
    if (!model || !filename) {
        fprintf(stderr, "Error: NULL model or filename\n");
        return -1;
    }
    
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for writing: %s\n", filename);
        return -1;
    }
    
    // ========== 1. WRITE HEADER ==========
    CLLMHeader header = {0};
    header.magic[0] = 'C';
    header.magic[1] = 'L';
    header.magic[2] = 'L';
    header.magic[3] = 'M';
    header.version = CLLM_VERSION;
    header.vocab_size = model->vocab_size;
    header.embedding_dim = model->embedding_dim;
    header.hidden_dim = model->hidden_dim;
    header.num_layers = model->num_layers;
    header.num_heads = model->num_heads;
    header.max_seq_len = model->max_seq_len;
    header.platonic_solid_type = model->solid_type;
    header.vertices = model->geometry.vertices;
    header.edges = model->geometry.edges;
    header.faces = model->geometry.faces;
    header.blind_recovery_enabled = model->recovery.enabled ? 1 : 0;
    header.harmonic_enabled = model->harmonic.enabled ? 1 : 0;
    header.ntt_attention_enabled = model->ntt.enabled ? 1 : 0;
    header.kissing_spheres_enabled = (model->threading.num_spheres > 0) ? 1 : 0;
    header.created_timestamp = time(NULL);
    header.modified_timestamp = time(NULL);
    header.best_loss = model->metrics.best_loss;
    header.training_steps = model->metrics.total_steps;
    
    if (fwrite(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write header\n");
        fclose(f);
        return -1;
    }
    
    // ========== 2. WRITE GEOMETRIC FOUNDATION ==========
    if (fwrite(&model->geometry, sizeof(PlatonicGeometry), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write geometry\n");
        fclose(f);
        return -1;
    }
    
    // ========== 3. WRITE CLOCK LATTICE POSITIONS ==========
    // Vertex positions
    if (model->vertex_positions) {
        size_t count = model->geometry.vertices;
        if (fwrite(model->vertex_positions, sizeof(ClockPosition), count, f) != count) {
            fprintf(stderr, "Error: Failed to write vertex positions\n");
            fclose(f);
            return -1;
        }
    }
    
    // Token positions
    if (model->token_positions) {
        if (fwrite(model->token_positions, sizeof(ClockPosition), model->vocab_size, f) != model->vocab_size) {
            fprintf(stderr, "Error: Failed to write token positions\n");
            fclose(f);
            return -1;
        }
    }
    
    // Angular positions
    if (model->token_angular_positions) {
        if (fwrite(model->token_angular_positions, sizeof(double), model->vocab_size, f) != model->vocab_size) {
            fprintf(stderr, "Error: Failed to write angular positions\n");
            fclose(f);
            return -1;
        }
    }
    
    // ========== 4. WRITE MODEL PARAMETERS ==========
    // Embeddings
    size_t emb_size = model->vocab_size * model->embedding_dim;
    if (model->embeddings && fwrite(model->embeddings, sizeof(double), emb_size, f) != emb_size) {
        fprintf(stderr, "Error: Failed to write embeddings\n");
        fclose(f);
        return -1;
    }
    
    // Positional encoding
    size_t pos_size = model->max_seq_len * model->embedding_dim;
    if (model->positional_encoding && fwrite(model->positional_encoding, sizeof(double), pos_size, f) != pos_size) {
        fprintf(stderr, "Error: Failed to write positional encoding\n");
        fclose(f);
        return -1;
    }
    
    // Layers
    for (uint32_t i = 0; i < model->num_layers; i++) {
        size_t qkv_size = model->embedding_dim * model->embedding_dim;
        size_t ffn1_size = model->embedding_dim * model->hidden_dim;
        size_t ffn2_size = model->hidden_dim * model->embedding_dim;
        
        // Attention weights
        if (fwrite(model->layers[i].query_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fwrite(model->layers[i].key_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fwrite(model->layers[i].value_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fwrite(model->layers[i].output_weights, sizeof(double), qkv_size, f) != qkv_size) {
            fprintf(stderr, "Error: Failed to write attention weights for layer %u\n", i);
            fclose(f);
            return -1;
        }
        
        // Feed-forward weights
        if (fwrite(model->layers[i].ffn_w1, sizeof(double), ffn1_size, f) != ffn1_size ||
            fwrite(model->layers[i].ffn_w2, sizeof(double), ffn2_size, f) != ffn2_size ||
            fwrite(model->layers[i].ffn_b1, sizeof(double), model->hidden_dim, f) != model->hidden_dim ||
            fwrite(model->layers[i].ffn_b2, sizeof(double), model->embedding_dim, f) != model->embedding_dim) {
            fprintf(stderr, "Error: Failed to write FFN weights for layer %u\n", i);
            fclose(f);
            return -1;
        }
        
        // Layer norm parameters
        if (fwrite(model->layers[i].ln1_gamma, sizeof(double), model->embedding_dim, f) != model->embedding_dim ||
            fwrite(model->layers[i].ln1_beta, sizeof(double), model->embedding_dim, f) != model->embedding_dim ||
            fwrite(model->layers[i].ln2_gamma, sizeof(double), model->embedding_dim, f) != model->embedding_dim ||
            fwrite(model->layers[i].ln2_beta, sizeof(double), model->embedding_dim, f) != model->embedding_dim) {
            fprintf(stderr, "Error: Failed to write layer norm parameters for layer %u\n", i);
            fclose(f);
            return -1;
        }
    }
    
    // Output projection
    size_t out_size = model->embedding_dim * model->vocab_size;
    if (model->output_weights && fwrite(model->output_weights, sizeof(double), out_size, f) != out_size) {
        fprintf(stderr, "Error: Failed to write output weights\n");
        fclose(f);
        return -1;
    }
    if (model->output_bias && fwrite(model->output_bias, sizeof(double), model->vocab_size, f) != model->vocab_size) {
        fprintf(stderr, "Error: Failed to write output bias\n");
        fclose(f);
        return -1;
    }
    
    // ========== 5. WRITE FEATURE STATES ==========
    // Write feature flags
    uint8_t flags = 0;
    if (model->recovery.enabled) flags |= 0x01;
    if (model->harmonic.enabled) flags |= 0x02;
    if (model->ntt.enabled) flags |= 0x04;
    if (model->threading.num_spheres > 0) flags |= 0x08;
    fwrite(&flags, sizeof(uint8_t), 1, f);
    
    // ========== 6. WRITE OPTIMIZER STATE ==========
    fwrite(&model->optimizer.type, sizeof(OptimizerType), 1, f);
    fwrite(&model->optimizer.learning_rate, sizeof(double), 1, f);
    fwrite(&model->optimizer.beta1, sizeof(double), 1, f);
    fwrite(&model->optimizer.beta2, sizeof(double), 1, f);
    fwrite(&model->optimizer.epsilon, sizeof(double), 1, f);
    fwrite(&model->optimizer.t, sizeof(uint64_t), 1, f);
    
    // ========== 7. WRITE METRICS ==========
    fwrite(&model->metrics, sizeof(model->metrics), 1, f);
    
    fclose(f);
    printf("✓ Model written to %s\n", filename);
    printf("  Vocab size: %u\n", model->vocab_size);
    printf("  Embedding dim: %u\n", model->embedding_dim);
    printf("  Layers: %u\n", model->num_layers);
    printf("  Platonic solid: %d\n", model->solid_type);
    
    return 0;
}

// ============================================================================
// READ MODEL
// ============================================================================

/**
 * Read model from file
 */
CLLMModel* cllm_read_model(const char* filename) {
    if (!filename) {
        fprintf(stderr, "Error: NULL filename\n");
        return NULL;
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for reading: %s\n", filename);
        return NULL;
    }
    
    // ========== 1. READ HEADER ==========
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read header\n");
        fclose(f);
        return NULL;
    }
    
    if (!validate_header(&header)) {
        fclose(f);
        return NULL;
    }
    
    // ========== 2. READ GEOMETRIC FOUNDATION ==========
    PlatonicGeometry geometry;
    if (fread(&geometry, sizeof(PlatonicGeometry), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read geometry\n");
        fclose(f);
        return NULL;
    }
    
    // ========== 3. CREATE MODEL ==========
    CLLMConfig config = {
        .solid_type = (PlatonicSolidType)header.platonic_solid_type,
        .vocab_size = header.vocab_size,
        .max_seq_len = header.max_seq_len,
        .embedding_dim = header.embedding_dim,
        .hidden_dim = header.hidden_dim,
        .num_layers = header.num_layers,
        .num_heads = header.num_heads,
        .enable_blind_recovery = header.blind_recovery_enabled != 0,
        .enable_harmonic_integration = header.harmonic_enabled != 0,
        .enable_ntt_attention = header.ntt_attention_enabled != 0,
        .enable_kissing_spheres = header.kissing_spheres_enabled != 0,
        .num_threads = 0,
        .optimizer_type = OPTIMIZER_ADAM,
        .learning_rate = 0.001,
        .beta1 = 0.9,
        .beta2 = 0.999,
        .epsilon = 1e-8,
        .weight_decay = 0.01,
        .ntt_threshold_seq_len = 512,
        .ntt_auto_select = true
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Error: Failed to create model\n");
        fclose(f);
        return NULL;
    }
    
    // ========== 4. READ CLOCK LATTICE POSITIONS ==========
    if (fread(model->vertex_positions, sizeof(ClockPosition), geometry.vertices, f) != geometry.vertices ||
        fread(model->token_positions, sizeof(ClockPosition), header.vocab_size, f) != header.vocab_size ||
        fread(model->token_angular_positions, sizeof(double), header.vocab_size, f) != header.vocab_size) {
        fprintf(stderr, "Error: Failed to read clock lattice positions\n");
        cllm_free_model(model);
        fclose(f);
        return NULL;
    }
    
    // ========== 5. READ MODEL PARAMETERS ==========
    size_t emb_size = header.vocab_size * header.embedding_dim;
    size_t pos_size = header.max_seq_len * header.embedding_dim;
    
    if (fread(model->embeddings, sizeof(double), emb_size, f) != emb_size ||
        fread(model->positional_encoding, sizeof(double), pos_size, f) != pos_size) {
        fprintf(stderr, "Error: Failed to read embeddings\n");
        cllm_free_model(model);
        fclose(f);
        return NULL;
    }
    
    // Read layers
    for (uint32_t i = 0; i < header.num_layers; i++) {
        size_t qkv_size = header.embedding_dim * header.embedding_dim;
        size_t ffn1_size = header.embedding_dim * header.hidden_dim;
        size_t ffn2_size = header.hidden_dim * header.embedding_dim;
        
        if (fread(model->layers[i].query_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fread(model->layers[i].key_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fread(model->layers[i].value_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fread(model->layers[i].output_weights, sizeof(double), qkv_size, f) != qkv_size ||
            fread(model->layers[i].ffn_w1, sizeof(double), ffn1_size, f) != ffn1_size ||
            fread(model->layers[i].ffn_w2, sizeof(double), ffn2_size, f) != ffn2_size ||
            fread(model->layers[i].ffn_b1, sizeof(double), header.hidden_dim, f) != header.hidden_dim ||
            fread(model->layers[i].ffn_b2, sizeof(double), header.embedding_dim, f) != header.embedding_dim ||
            fread(model->layers[i].ln1_gamma, sizeof(double), header.embedding_dim, f) != header.embedding_dim ||
            fread(model->layers[i].ln1_beta, sizeof(double), header.embedding_dim, f) != header.embedding_dim ||
            fread(model->layers[i].ln2_gamma, sizeof(double), header.embedding_dim, f) != header.embedding_dim ||
            fread(model->layers[i].ln2_beta, sizeof(double), header.embedding_dim, f) != header.embedding_dim) {
            fprintf(stderr, "Error: Failed to read layer %u\n", i);
            cllm_free_model(model);
            fclose(f);
            return NULL;
        }
    }
    
    // Read output projection
    size_t out_size = header.embedding_dim * header.vocab_size;
    if (fread(model->output_weights, sizeof(double), out_size, f) != out_size ||
        fread(model->output_bias, sizeof(double), header.vocab_size, f) != header.vocab_size) {
        fprintf(stderr, "Error: Failed to read output projection\n");
        cllm_free_model(model);
        fclose(f);
        return NULL;
    }
    
    // ========== 6. READ FEATURE STATES ==========
    uint8_t flags;
    fread(&flags, sizeof(uint8_t), 1, f);
    model->recovery.enabled = (flags & 0x01) != 0;
    model->harmonic.enabled = (flags & 0x02) != 0;
    model->ntt.enabled = (flags & 0x04) != 0;
    
    // ========== 7. READ OPTIMIZER STATE ==========
    fread(&model->optimizer.type, sizeof(OptimizerType), 1, f);
    fread(&model->optimizer.learning_rate, sizeof(double), 1, f);
    fread(&model->optimizer.beta1, sizeof(double), 1, f);
    fread(&model->optimizer.beta2, sizeof(double), 1, f);
    fread(&model->optimizer.epsilon, sizeof(double), 1, f);
    fread(&model->optimizer.t, sizeof(uint64_t), 1, f);
    
    // ========== 8. READ METRICS ==========
    fread(&model->metrics, sizeof(model->metrics), 1, f);
    
    fclose(f);
    printf("✓ Model loaded from %s\n", filename);
    printf("  Vocab size: %u\n", model->vocab_size);
    printf("  Embedding dim: %u\n", model->embedding_dim);
    printf("  Layers: %u\n", model->num_layers);
    printf("  Platonic solid: %d\n", model->solid_type);
    
    return model;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Free model (alias for cllm_free_model)
 */
void cllm_free(CLLMModel* model) {
    cllm_free_model(model);
}
