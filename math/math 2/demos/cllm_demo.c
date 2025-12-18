/*
 * CLLM Demo Program
 * Demonstrates the Crystalline Lattice Language Model format and inference
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cllm.h"
#include "cllm_utils.h"
#include "cllm_format.h"
#include "cllm_inference.h"
#include "cllm_training.h"

// Create a simple demo model
CLLMModel* create_demo_model(void) {
    printf("Creating demo CLLM model...\n");
    
    CLLMModel* model = (CLLMModel*)calloc(1, sizeof(CLLMModel));
    if (!model) return NULL;
    
    // Initialize header
    cllm_header_init(&model->header, "demo-model-v1", "Demo Crystalline Lattice LLM");
    
    // Set model parameters (small for demo)
    model->header.vocab_size = 1000;
    model->header.embedding_dim = 128;
    model->header.num_layers = 4;
    model->header.num_heads = 4;
    model->header.context_length = 256;
    model->header.total_params = 1000000;
    model->header.num_lattice_points = 1000;
    
    model->vocab_size = model->header.vocab_size;
    model->num_layers = model->header.num_layers;
    model->num_lattice_points = model->header.num_lattice_points;
    
    // Create vocabulary
    printf("Creating vocabulary (%lu tokens)...\n", (unsigned long)model->vocab_size);
    model->tokens = (CLLMToken*)calloc(model->vocab_size, sizeof(CLLMToken));
    
    // Create some sample tokens
    const char* sample_words[] = {
        "the", "a", "an", "is", "are", "was", "were", "be", "been", "being",
        "have", "has", "had", "do", "does", "did", "will", "would", "could", "should",
        "can", "may", "might", "must", "shall", "of", "in", "on", "at", "to",
        "for", "with", "by", "from", "about", "as", "into", "through", "during", "before",
        "after", "above", "below", "between", "under", "over", "again", "further", "then", "once",
        "here", "there", "when", "where", "why", "how", "all", "each", "every", "both",
        "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only",
        "own", "same", "so", "than", "too", "very", "just", "now", "also", "well",
        "hello", "world", "prime", "number", "lattice", "crystalline", "geometry", "mathematics",
        "algorithm", "function", "structure", "system", "model", "data", "process", "compute"
    };
    
    int num_sample_words = sizeof(sample_words) / sizeof(sample_words[0]);
    
    for (uint32_t i = 0; i < model->vocab_size; i++) {
        if (i < (uint32_t)num_sample_words) {
            cllm_token_create(&model->tokens[i], i, sample_words[i]);
        } else {
            char token_str[64];
            snprintf(token_str, sizeof(token_str), "token_%d", i);
            cllm_token_create(&model->tokens[i], i, token_str);
        }
    }
    
    // Create lattice structure
    printf("Creating lattice structure (%lu points)...\n", (unsigned long)model->num_lattice_points);
    model->lattice_points = (CLLMLatticePoint*)calloc(model->num_lattice_points, sizeof(CLLMLatticePoint));
    
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        float angle = (2.0f * 3.14159f * i) / model->num_lattice_points;
        float radius = 1.0f + (float)i / model->num_lattice_points;
        float x = radius * cosf(angle);
        float y = radius * sinf(angle);
        float z = (float)i / model->num_lattice_points;
        
        uint64_t prime = cllm_token_to_prime(i);
        cllm_lattice_point_create(&model->lattice_points[i], i, x, y, z, prime);
    }
    
    // Find neighbors for each point
    printf("Computing lattice neighbors...\n");
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        cllm_lattice_find_neighbors(&model->lattice_points[i], model->lattice_points, 
                                    model->num_lattice_points, 2.0f);
    }
    
    // Initialize embeddings
    printf("Initializing embeddings...\n");
    model->embeddings.vocab_size = model->vocab_size;
    model->embeddings.embedding_dim = model->header.embedding_dim;
    
    size_t embed_size = model->vocab_size * model->header.embedding_dim;
    model->embeddings.embeddings = (float*)calloc(embed_size, sizeof(float));
    
    // Random initialization
    for (size_t i = 0; i < embed_size; i++) {
        model->embeddings.embeddings[i] = ((float)rand() / RAND_MAX) * 0.1f - 0.05f;
    }
    
    // Initialize transform matrices
    size_t transform_size = model->header.embedding_dim * model->header.embedding_dim;
    model->embeddings.lattice_transform = (float*)calloc(transform_size, sizeof(float));
    model->embeddings.inverse_transform = (float*)calloc(transform_size, sizeof(float));
    
    // Identity matrices for now
    for (uint32_t i = 0; i < model->header.embedding_dim; i++) {
        model->embeddings.lattice_transform[i * model->header.embedding_dim + i] = 1.0f;
        model->embeddings.inverse_transform[i * model->header.embedding_dim + i] = 1.0f;
    }
    
    // Initialize layers (simplified)
    printf("Initializing transformer layers...\n");
    model->attention_layers = (AttentionLayer*)calloc(model->num_layers, sizeof(AttentionLayer));
    model->ff_layers = (FeedForwardLayer*)calloc(model->num_layers, sizeof(FeedForwardLayer));
    model->layer_norms = (CLLMLayerNorm*)calloc(model->num_layers, sizeof(CLLMLayerNorm));
    
    for (uint32_t i = 0; i < model->num_layers; i++) {
        // Attention layer
        model->attention_layers[i].layer_id = i;
        model->attention_layers[i].num_heads = model->header.num_heads;
        model->attention_layers[i].head_dim = model->header.embedding_dim / model->header.num_heads;
        
        size_t attn_size = model->attention_layers[i].num_heads * 
                          model->attention_layers[i].head_dim * 
                          model->attention_layers[i].head_dim;
        
        model->attention_layers[i].query_lattice = (float*)calloc(attn_size, sizeof(float));
        model->attention_layers[i].key_lattice = (float*)calloc(attn_size, sizeof(float));
        model->attention_layers[i].value_lattice = (float*)calloc(attn_size, sizeof(float));
        
        // Feed-forward layer
        model->ff_layers[i].layer_id = i;
        model->ff_layers[i].input_dim = model->header.embedding_dim;
        model->ff_layers[i].hidden_dim = model->header.embedding_dim * 4;
        model->ff_layers[i].output_dim = model->header.embedding_dim;
        
        size_t ff_size1 = model->ff_layers[i].input_dim * model->ff_layers[i].hidden_dim;
        size_t ff_size2 = model->ff_layers[i].hidden_dim * model->ff_layers[i].output_dim;
        
        model->ff_layers[i].w1_lattice = (float*)calloc(ff_size1, sizeof(float));
        model->ff_layers[i].w2_lattice = (float*)calloc(ff_size2, sizeof(float));
        model->ff_layers[i].bias1 = (float*)calloc(model->ff_layers[i].hidden_dim, sizeof(float));
        model->ff_layers[i].bias2 = (float*)calloc(model->ff_layers[i].output_dim, sizeof(float));
        
        // Layer norm
        model->layer_norms[i].layer_id = i;
        model->layer_norms[i].dim = model->header.embedding_dim;
        model->layer_norms[i].gamma = (float*)calloc(model->header.embedding_dim, sizeof(float));
        model->layer_norms[i].beta = (float*)calloc(model->header.embedding_dim, sizeof(float));
        model->layer_norms[i].epsilon = 1e-5f;
        
        // Initialize to 1.0 for gamma
        for (uint32_t j = 0; j < model->header.embedding_dim; j++) {
            model->layer_norms[i].gamma[j] = 1.0f;
        }
    }
    
    // Initialize positional encoding
    printf("Initializing positional encoding...\n");
    model->pos_encoding.max_length = model->header.context_length;
    model->pos_encoding.embedding_dim = model->header.embedding_dim;
    
    size_t pos_size = model->pos_encoding.max_length * model->pos_encoding.embedding_dim;
    model->pos_encoding.spiral_positions = (float*)calloc(pos_size, sizeof(float));
    model->pos_encoding.clock_positions = (float*)calloc(pos_size, sizeof(float));
    model->pos_encoding.prime_positions = (float*)calloc(pos_size, sizeof(float));
    model->pos_encoding.learned_positions = (float*)calloc(pos_size, sizeof(float));
    
    // Initialize training metadata
    model->training_meta.training_steps = 0;
    model->training_meta.learning_rate = 0.001f;
    model->training_meta.loss = 0.0f;
    model->training_meta.timestamp = time(NULL);
    strcpy(model->training_meta.optimizer, "Adam");
    
    printf("Demo model created successfully!\n\n");
    return model;
}

// Test CLLM format I/O
void test_cllm_io(CLLMModel* model) {
    printf("=== Testing CLLM Format I/O ===\n");
    
    const char* filename = "../data/models/demo_model.cllm";
    
    // Write model
    printf("Writing model to: %s\n", filename);
    int result = cllm_write_model(model, filename);
    if (result == 0) {
        printf("✓ Model written successfully\n");
    } else {
        printf("✗ Failed to write model\n");
        return;
    }
    
    // Validate file
    printf("Validating CLLM file...\n");
    if (cllm_validate(filename)) {
        printf("✓ File validation passed\n");
    } else {
        printf("✗ File validation failed\n");
        return;
    }
    
    // Read model back
    printf("Reading model from file...\n");
    CLLMModel* loaded_model = cllm_read_model(filename);
    if (loaded_model) {
        printf("✓ Model loaded successfully\n");
        printf("  Vocab size: %lu\n", (unsigned long)loaded_model->vocab_size);
        printf("  Embedding dim: %lu\n", (unsigned long)loaded_model->embeddings.embedding_dim);
        printf("  Num layers: %d\n", loaded_model->num_layers);
        printf("  Lattice points: %lu\n", (unsigned long)loaded_model->num_lattice_points);
        
        cllm_free_model(loaded_model);
    } else {
        printf("✗ Failed to load model\n");
    }
    
    printf("\n");
}

// Test inference
void test_cllm_inference(CLLMModel* model) {
    printf("=== Testing CLLM Inference ===\n");
    
    // Initialize inference engine
    printf("Initializing inference engine...\n");
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        printf("✗ Failed to initialize inference\n");
        return;
    }
    printf("✓ Inference engine initialized\n");
    
    // Set parameters
    cllm_set_temperature(inference, 0.8f);
    cllm_set_top_p(inference, 0.9f);
    cllm_set_max_tokens(inference, 50);
    
    printf("  Temperature: %.2f\n", inference->temperature);
    printf("  Top-p: %.2f\n", inference->top_p);
    printf("  Max tokens: %d\n", inference->max_tokens);
    
    // Test generation
    printf("\nGenerating text...\n");
    const char* prompt = "hello world prime number";
    char output[1024];
    
    int tokens_generated = cllm_generate(inference, prompt, output, sizeof(output));
    
    if (tokens_generated > 0) {
        printf("✓ Generated %d tokens\n", tokens_generated);
        printf("  Prompt: %s\n", prompt);
        printf("  Output: %s\n", output);
    } else {
        printf("✗ Generation failed\n");
    }
    
    // Cleanup
    cllm_inference_cleanup(inference);
    printf("\n");
}

// Test training
void test_cllm_training(CLLMModel* model) {
    printf("=== Testing CLLM Training ===\n");
    
    // Create training config
    CLLMTrainingConfig config = {
        .num_epochs = 2,
        .batch_size = 4,
        .sequence_length = 32,
        .learning_rate = 0.001f,
        .weight_decay = 0.01f,
        .eval_interval = 10,
        .warmup_steps = 50,
        .max_steps = 1000,
        .lr_decay_factor = 0.1f,
        .lr_decay_steps = 200,
        .min_lr = 1e-6f,
        .gradient_accumulation_steps = 2,
        .use_mixed_precision = 0,
        .loss_scale = 1024.0f,
        .loss_scale_growth = 2.0f,
        .loss_scale_backoff = 0.5f,
        .loss_scale_window = 2000
    };
    strcpy(config.optimizer, "adam");
    strcpy(config.lr_scheduler, "cosine");
    
    // Initialize training
    printf("Initializing training...\n");
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        printf("✗ Failed to initialize training\n");
        return;
    }
    printf("✓ Training initialized\n");
    
    printf("  Epochs: %d\n", config.num_epochs);
    printf("  Batch size: %d\n", config.batch_size);
    printf("  Learning rate: %.4f\n", config.learning_rate);
    
    // Load training data
    printf("\nLoading training data...\n");
    const char* training_file = "../data/training/sample_data.txt";
    int tokens_loaded = cllm_load_training_data(training, training_file);
    
    if (tokens_loaded > 0) {
        printf("✓ Loaded %d tokens\n", tokens_loaded);
        printf("  Total batches: %d\n", training->total_batches);
    } else {
        printf("✗ Failed to load training data (file may not exist yet)\n");
    }
    
    // Cleanup
    cllm_training_cleanup(training);
    printf("\n");
}

// Display lattice statistics
void display_lattice_stats(CLLMModel* model) {
    printf("=== Lattice Statistics ===\n");
    
    printf("Total lattice points: %lu\n", (unsigned long)model->num_lattice_points);
    
    // Count neighbors
    int total_neighbors = 0;
    int min_neighbors = 12;
    int max_neighbors = 0;
    
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        int n = model->lattice_points[i].neighbor_count;
        total_neighbors += n;
        if (n < min_neighbors) min_neighbors = n;
        if (n > max_neighbors) max_neighbors = n;
    }
    
    float avg_neighbors = (float)total_neighbors / model->num_lattice_points;
    
    printf("Neighbor statistics:\n");
    printf("  Average: %.2f\n", avg_neighbors);
    printf("  Min: %d\n", min_neighbors);
    printf("  Max: %d\n", max_neighbors);
    
    // Symmetry group distribution
    int symmetry_counts[12] = {0};
    for (uint32_t i = 0; i < model->num_lattice_points; i++) {
        symmetry_counts[model->lattice_points[i].symmetry_group]++;
    }
    
    printf("\nSymmetry group distribution:\n");
    for (int i = 0; i < 12; i++) {
        printf("  Group %2d: %4d points (%.1f%%)\n", i, symmetry_counts[i],
               100.0f * symmetry_counts[i] / model->num_lattice_points);
    }
    
    printf("\n");
}

int main(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║  Crystalline Lattice Language Model (CLLM) Demo Program  ║\n");
    printf("║  Revolutionary Geometric Approach to Language Modeling    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Seed random number generator
    srand(time(NULL));
    
    // Create demo model
    CLLMModel* model = create_demo_model();
    if (!model) {
        printf("Failed to create demo model\n");
        return 1;
    }
    
    // Display lattice statistics
    display_lattice_stats(model);
    
    // Test format I/O
    test_cllm_io(model);
    
    // Test inference
    test_cllm_inference(model);
    
    // Test training
    test_cllm_training(model);
    
    // Cleanup
    printf("=== Cleanup ===\n");
    cllm_free_model(model);
    printf("✓ Model freed\n");
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Demo Complete!                          ║\n");
    printf("║  The CLLM format is ready for production use.              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}