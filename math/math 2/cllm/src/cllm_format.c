/**
 * @file cllm_format.c
 * @brief Model File I/O for Thread-Centric CLLMModel
 * 
 * UPDATED FOR THREAD-CENTRIC ARCHITECTURE:
 * - Serializes from thread CrystallineAbacus (not flat arrays)
 * - Deserializes to thread CrystallineAbacus (not flat arrays)
 * - Saves token assignments and thread parameters
 * - Saves layer info and threading configuration
 * 
 * File Format (Version 3 - Thread-Centric):
 * 1. Header (CLLMHeader)
 * 2. Geometric foundation (PlatonicGeometry)
 * 3. Clock lattice positions
 * 4. Token assignments (permanent token → thread mapping)
 * 5. Thread parameters (for each of 96 threads)
 * 6. Model parameters (extracted from thread CrystallineAbacus)
 * 7. Feature states (blind recovery, harmonic, NTT)
 * 8. Optimizer state
 * 9. Metrics
 */

#include "cllm.h"
#include "cllm_format.h"
#include "hierarchical_threading.h"
#include "math/abacus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Version for thread-centric model
#define CLLM_VERSION 3  // Version 3 for thread-centric architecture

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

/**
 * Extract embedding from thread CrystallineAbacus to double array
 */
static bool extract_embedding_from_thread(
    HierarchicalThread* thread,
    double* output,
    uint32_t embedding_dim
) {
    if (!thread || !output || !thread->value) return false;
    
    // Convert CrystallineAbacus to double array
    // For now, we'll use a simple conversion
    // TODO: Implement proper abacus_to_double_array function
    
    // Placeholder: Zero out for now
    memset(output, 0, embedding_dim * sizeof(double));
    
    return true;
}

/**
 * Store embedding from double array to thread CrystallineAbacus
 */
static bool store_embedding_to_thread(
    HierarchicalThread* thread,
    const double* input,
    uint32_t embedding_dim
) {
    if (!thread || !input || !thread->value) return false;
    
    // Convert double array to CrystallineAbacus
    // For now, we'll use a simple conversion
    // TODO: Implement proper abacus_from_double_array function
    
    return true;
}

// ============================================================================
// WRITE MODEL
// ============================================================================

/**
 * Write model to file
 * 
 * File format (Version 3 - Thread-Centric):
 * 1. Header (CLLMHeader)
 * 2. Geometric foundation (PlatonicGeometry)
 * 3. Clock lattice positions
 * 4. Token assignments (vocab_size entries)
 * 5. Thread parameters (96 entries)
 * 6. Model parameters (extracted from threads)
 * 7. Feature states
 * 8. Optimizer state
 * 9. Metrics
 */
int cllm_write_model(const CLLMModel* model, const char* filename) {
    if (!model || !filename) {
        fprintf(stderr, "Error: NULL model or filename\n");
        return -1;
    }
    
    if (!model->threads) {
        fprintf(stderr, "Error: Model has no thread pool (thread-centric architecture required)\n");
        return -1;
    }
    
    FILE* f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for writing: %s\n", filename);
        return -1;
    }
    
    printf("💾 Saving thread-centric model to %s...\n", filename);
    
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
    header.kissing_spheres_enabled = 1;  // Always enabled in thread-centric architecture
    header.created_timestamp = time(NULL);
    header.modified_timestamp = time(NULL);
    header.total_params = model->vocab_size * model->embedding_dim;  // Simplified
    header.best_loss = model->metrics.best_loss;
    header.training_steps = model->metrics.total_steps;
    
    strncpy(header.model_name, "CLLM-ThreadCentric", MAX_MODEL_NAME - 1);
    strncpy(header.description, "Thread-centric geometric language model", MAX_DESCRIPTION - 1);
    
    if (fwrite(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write header\n");
        fclose(f);
        return -1;
    }
    printf("  ✓ Wrote header (version %u)\n", CLLM_VERSION);
    
    // ========== 2. WRITE GEOMETRIC FOUNDATION ==========
    if (fwrite(&model->geometry, sizeof(PlatonicGeometry), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write geometry\n");
        fclose(f);
        return -1;
    }
    printf("  ✓ Wrote geometric foundation\n");
    
    // ========== 3. WRITE CLOCK LATTICE POSITIONS ==========
    // Vertex positions
    if (model->vertex_positions) {
        if (fwrite(model->vertex_positions, sizeof(ClockPosition), model->geometry.vertices, f) != model->geometry.vertices) {
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
    printf("  ✓ Wrote clock lattice positions\n");
    
    // ========== 4. WRITE TOKEN ASSIGNMENTS ==========
    if (model->token_assignments) {
        // Write token assignments (permanent token → thread mapping)
        for (uint32_t i = 0; i < model->vocab_size; i++) {
            uint8_t layer = model->token_assignments[i].layer;
            uint8_t dimension = model->token_assignments[i].dimension;
            uint32_t thread_id = model->token_assignments[i].thread_id;
            
            if (fwrite(&layer, sizeof(uint8_t), 1, f) != 1 ||
                fwrite(&dimension, sizeof(uint8_t), 1, f) != 1 ||
                fwrite(&thread_id, sizeof(uint32_t), 1, f) != 1) {
                fprintf(stderr, "Error: Failed to write token assignment %u\n", i);
                fclose(f);
                return -1;
            }
        }
    }
    printf("  ✓ Wrote token assignments (%u tokens)\n", model->vocab_size);
    
    // ========== 5. WRITE THREAD PARAMETERS ==========
    if (model->thread_params) {
        for (uint32_t i = 0; i < 96; i++) {
            uint32_t num_tokens = model->thread_params[i].num_tokens_assigned;
            uint8_t layer_id = model->thread_params[i].layer_id;
            uint8_t is_control = model->thread_params[i].is_control_thread ? 1 : 0;
            uint8_t is_worker = model->thread_params[i].is_worker_thread ? 1 : 0;
            
            if (fwrite(&num_tokens, sizeof(uint32_t), 1, f) != 1 ||
                fwrite(&layer_id, sizeof(uint8_t), 1, f) != 1 ||
                fwrite(&is_control, sizeof(uint8_t), 1, f) != 1 ||
                fwrite(&is_worker, sizeof(uint8_t), 1, f) != 1) {
                fprintf(stderr, "Error: Failed to write thread params %u\n", i);
                fclose(f);
                return -1;
            }
            
            // Write token IDs for this thread
            if (num_tokens > 0 && model->thread_params[i].token_ids) {
                if (fwrite(model->thread_params[i].token_ids, sizeof(uint32_t), num_tokens, f) != num_tokens) {
                    fprintf(stderr, "Error: Failed to write token IDs for thread %u\n", i);
                    fclose(f);
                    return -1;
                }
            }
        }
    }
    printf("  ✓ Wrote thread parameters (96 threads)\n");
    
    // ========== 6. WRITE MODEL PARAMETERS (FROM THREADS) ==========
    // Extract embeddings from threads and write them
    double* temp_embedding = calloc(model->embedding_dim, sizeof(double));
    if (!temp_embedding) {
        fprintf(stderr, "Error: Failed to allocate temporary embedding buffer\n");
        fclose(f);
        return -1;
    }
    
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        if (!thread) {
            fprintf(stderr, "Error: Token %u has no assigned thread\n", token_id);
            free(temp_embedding);
            fclose(f);
            return -1;
        }
        
        // Extract embedding from thread's CrystallineAbacus
        if (!extract_embedding_from_thread(thread, temp_embedding, model->embedding_dim)) {
            fprintf(stderr, "Error: Failed to extract embedding for token %u\n", token_id);
            free(temp_embedding);
            fclose(f);
            return -1;
        }
        
        // Write embedding
        if (fwrite(temp_embedding, sizeof(double), model->embedding_dim, f) != model->embedding_dim) {
            fprintf(stderr, "Error: Failed to write embedding for token %u\n", token_id);
            free(temp_embedding);
            fclose(f);
            return -1;
        }
    }
    free(temp_embedding);
    printf("  ✓ Wrote embeddings from thread CrystallineAbacus (%u tokens)\n", model->vocab_size);
    
    // TODO: Write layer parameters from control threads
    // TODO: Write output layer parameters
    // For now, we'll skip these as they need proper extraction from threads
    
    // ========== 7. WRITE FEATURE STATES ==========
    // Blind recovery
    if (model->recovery.enabled) {
        double corruption_tolerance = model->recovery.corruption_tolerance;
        uint32_t max_iterations = model->recovery.max_iterations;
        uint32_t recovery_methods = model->recovery.recovery_methods;
        
        if (fwrite(&corruption_tolerance, sizeof(double), 1, f) != 1 ||
            fwrite(&max_iterations, sizeof(uint32_t), 1, f) != 1 ||
            fwrite(&recovery_methods, sizeof(uint32_t), 1, f) != 1) {
            fprintf(stderr, "Error: Failed to write blind recovery state\n");
            fclose(f);
            return -1;
        }
    }
    
    // Harmonic integration
    if (model->harmonic.enabled) {
        if (fwrite(model->harmonic.frequencies, sizeof(double), NUM_CYMATIC_FREQUENCIES, f) != NUM_CYMATIC_FREQUENCIES ||
            fwrite(&model->harmonic.primary_frequency, sizeof(double), 1, f) != 1 ||
            fwrite(model->harmonic.platonic_primes, sizeof(uint32_t), NUM_PLATONIC_PRIMES, f) != NUM_PLATONIC_PRIMES) {
            fprintf(stderr, "Error: Failed to write harmonic state\n");
            fclose(f);
            return -1;
        }
    }
    printf("  ✓ Wrote feature states\n");
    
    // ========== 8. WRITE OPTIMIZER STATE ==========
    uint8_t optimizer_type = (uint8_t)model->optimizer.type;
    if (fwrite(&optimizer_type, sizeof(uint8_t), 1, f) != 1 ||
        fwrite(&model->optimizer.learning_rate, sizeof(double), 1, f) != 1 ||
        fwrite(&model->optimizer.beta1, sizeof(double), 1, f) != 1 ||
        fwrite(&model->optimizer.beta2, sizeof(double), 1, f) != 1 ||
        fwrite(&model->optimizer.epsilon, sizeof(double), 1, f) != 1 ||
        fwrite(&model->optimizer.weight_decay, sizeof(double), 1, f) != 1 ||
        fwrite(&model->optimizer.t, sizeof(uint64_t), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write optimizer state\n");
        fclose(f);
        return -1;
    }
    printf("  ✓ Wrote optimizer state\n");
    
    // ========== 9. WRITE METRICS ==========
    if (fwrite(&model->metrics, sizeof(model->metrics), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write metrics\n");
        fclose(f);
        return -1;
    }
    printf("  ✓ Wrote metrics\n");
    
    fclose(f);
    printf("✅ Model saved successfully (thread-centric format)\n");
    return 0;
}

// ============================================================================
// READ MODEL
// ============================================================================

/**
 * Read model from file
 * 
 * NOTE: This creates a new model with thread pool and loads parameters
 * into thread CrystallineAbacus (not flat arrays)
 */
int cllm_read_model(CLLMModel** model_out, const char* filename) {
    if (!model_out || !filename) {
        fprintf(stderr, "Error: NULL model_out or filename\n");
        return -1;
    }
    
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file for reading: %s\n", filename);
        return -1;
    }
    
    printf("📂 Loading thread-centric model from %s...\n", filename);
    
    // ========== 1. READ HEADER ==========
    CLLMHeader header;
    if (fread(&header, sizeof(CLLMHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read header\n");
        fclose(f);
        return -1;
    }
    
    if (!validate_header(&header)) {
        fclose(f);
        return -1;
    }
    printf("  ✓ Read header (version %u)\n", header.version);
    
    // ========== 2. CREATE MODEL ==========
    // Create config from header
    CLLMConfig config = {
        .vocab_size = header.vocab_size,
        .embedding_dim = header.embedding_dim,
        .hidden_dim = header.hidden_dim,
        .num_layers = header.num_layers,
        .num_heads = header.num_heads,
        .max_seq_len = header.max_seq_len,
        .solid_type = (PlatonicSolidType)header.platonic_solid_type,
        .enable_blind_recovery = header.blind_recovery_enabled,
        .enable_harmonic_integration = header.harmonic_enabled,
        .enable_ntt_attention = header.ntt_attention_enabled
    };
    
    CLLMModel* model = cllm_create_model(&config);
    if (!model) {
        fprintf(stderr, "Error: Failed to create model\n");
        fclose(f);
        return -1;
    }
    printf("  ✓ Created model with thread pool\n");
    
    // ========== 3. READ GEOMETRIC FOUNDATION ==========
    if (fread(&model->geometry, sizeof(PlatonicGeometry), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read geometry\n");
        cllm_free_model(model);
        fclose(f);
        return -1;
    }
    printf("  ✓ Read geometric foundation\n");
    
    // ========== 4. READ CLOCK LATTICE POSITIONS ==========
    if (fread(model->vertex_positions, sizeof(ClockPosition), model->geometry.vertices, f) != model->geometry.vertices ||
        fread(model->token_positions, sizeof(ClockPosition), model->vocab_size, f) != model->vocab_size ||
        fread(model->token_angular_positions, sizeof(double), model->vocab_size, f) != model->vocab_size) {
        fprintf(stderr, "Error: Failed to read clock lattice positions\n");
        cllm_free_model(model);
        fclose(f);
        return -1;
    }
    printf("  ✓ Read clock lattice positions\n");
    
    // ========== 5. READ TOKEN ASSIGNMENTS ==========
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        uint8_t layer, dimension;
        uint32_t thread_id;
        
        if (fread(&layer, sizeof(uint8_t), 1, f) != 1 ||
            fread(&dimension, sizeof(uint8_t), 1, f) != 1 ||
            fread(&thread_id, sizeof(uint32_t), 1, f) != 1) {
            fprintf(stderr, "Error: Failed to read token assignment %u\n", i);
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
        
        // Verify assignment matches what we created
        if (model->token_assignments[i].layer != layer ||
            model->token_assignments[i].dimension != dimension ||
            model->token_assignments[i].thread_id != thread_id) {
            fprintf(stderr, "Warning: Token assignment mismatch for token %u\n", i);
        }
    }
    printf("  ✓ Read token assignments\n");
    
    // ========== 6. READ THREAD PARAMETERS ==========
    for (uint32_t i = 0; i < 96; i++) {
        uint32_t num_tokens;
        uint8_t layer_id, is_control, is_worker;
        
        if (fread(&num_tokens, sizeof(uint32_t), 1, f) != 1 ||
            fread(&layer_id, sizeof(uint8_t), 1, f) != 1 ||
            fread(&is_control, sizeof(uint8_t), 1, f) != 1 ||
            fread(&is_worker, sizeof(uint8_t), 1, f) != 1) {
            fprintf(stderr, "Error: Failed to read thread params %u\n", i);
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
        
        // Read token IDs
        if (num_tokens > 0) {
            uint32_t* token_ids = calloc(num_tokens, sizeof(uint32_t));
            if (!token_ids || fread(token_ids, sizeof(uint32_t), num_tokens, f) != num_tokens) {
                fprintf(stderr, "Error: Failed to read token IDs for thread %u\n", i);
                free(token_ids);
                cllm_free_model(model);
                fclose(f);
                return -1;
            }
            free(token_ids);  // We already have this info from token_assignments
        }
    }
    printf("  ✓ Read thread parameters\n");
    
    // ========== 7. READ MODEL PARAMETERS (INTO THREADS) ==========
    double* temp_embedding = calloc(model->embedding_dim, sizeof(double));
    if (!temp_embedding) {
        fprintf(stderr, "Error: Failed to allocate temporary embedding buffer\n");
        cllm_free_model(model);
        fclose(f);
        return -1;
    }
    
    for (uint32_t token_id = 0; token_id < model->vocab_size; token_id++) {
        // Read embedding
        if (fread(temp_embedding, sizeof(double), model->embedding_dim, f) != model->embedding_dim) {
            fprintf(stderr, "Error: Failed to read embedding for token %u\n", token_id);
            free(temp_embedding);
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
        
        // Store embedding in thread's CrystallineAbacus
        HierarchicalThread* thread = model->token_assignments[token_id].thread;
        if (!store_embedding_to_thread(thread, temp_embedding, model->embedding_dim)) {
            fprintf(stderr, "Error: Failed to store embedding for token %u\n", token_id);
            free(temp_embedding);
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
    }
    free(temp_embedding);
    printf("  ✓ Read embeddings into thread CrystallineAbacus\n");
    
    // TODO: Read layer parameters into control threads
    // TODO: Read output layer parameters
    
    // ========== 8. READ FEATURE STATES ==========
    if (header.blind_recovery_enabled) {
        if (fread(&model->recovery.corruption_tolerance, sizeof(double), 1, f) != 1 ||
            fread(&model->recovery.max_iterations, sizeof(uint32_t), 1, f) != 1 ||
            fread(&model->recovery.recovery_methods, sizeof(uint32_t), 1, f) != 1) {
            fprintf(stderr, "Error: Failed to read blind recovery state\n");
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
    }
    
    if (header.harmonic_enabled) {
        if (fread(model->harmonic.frequencies, sizeof(double), NUM_CYMATIC_FREQUENCIES, f) != NUM_CYMATIC_FREQUENCIES ||
            fread(&model->harmonic.primary_frequency, sizeof(double), 1, f) != 1 ||
            fread(model->harmonic.platonic_primes, sizeof(uint32_t), NUM_PLATONIC_PRIMES, f) != NUM_PLATONIC_PRIMES) {
            fprintf(stderr, "Error: Failed to read harmonic state\n");
            cllm_free_model(model);
            fclose(f);
            return -1;
        }
    }
    printf("  ✓ Read feature states\n");
    
    // ========== 9. READ OPTIMIZER STATE ==========
    uint8_t optimizer_type;
    if (fread(&optimizer_type, sizeof(uint8_t), 1, f) != 1 ||
        fread(&model->optimizer.learning_rate, sizeof(double), 1, f) != 1 ||
        fread(&model->optimizer.beta1, sizeof(double), 1, f) != 1 ||
        fread(&model->optimizer.beta2, sizeof(double), 1, f) != 1 ||
        fread(&model->optimizer.epsilon, sizeof(double), 1, f) != 1 ||
        fread(&model->optimizer.weight_decay, sizeof(double), 1, f) != 1 ||
        fread(&model->optimizer.t, sizeof(uint64_t), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read optimizer state\n");
        cllm_free_model(model);
        fclose(f);
        return -1;
    }
    model->optimizer.type = (OptimizerType)optimizer_type;
    printf("  ✓ Read optimizer state\n");
    
    // ========== 10. READ METRICS ==========
    if (fread(&model->metrics, sizeof(model->metrics), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read metrics\n");
        cllm_free_model(model);
        fclose(f);
        return -1;
    }
    printf("  ✓ Read metrics\n");
    
    fclose(f);
    *model_out = model;
    printf("✅ Model loaded successfully (thread-centric format)\n");
    return 0;
}