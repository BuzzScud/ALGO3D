/**
 * CLLM - Unified Command-Line Interface
 * 
 * Single robust CLI tool that provides core CLLM functionality.
 * 
 * Usage:
 *   cllm train [options]     - Train a model
 *   cllm infer [options]     - Run inference
 *   cllm create [options]    - Create a new model
 *   cllm help [command]      - Show help
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_training_threaded.h"
#include "../include/ai/cllm_hierarchical_training.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_data_loader.h"
#include "../include/cllm_format.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_batch.h"
#include "../include/cllm_global_progress.h"

// ============================================================================
// BANNER & VERSION
// ============================================================================

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM - Crystalline Lattice Language Model         ║\n");
    printf("║                    Unified CLI Tool                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void print_main_help() {
    print_banner();
    printf("Usage: cllm <command> [options]\n\n");
    printf("Commands:\n");
    printf("  train      Train a model with kissing spheres architecture\n");
    printf("  infer      Run inference on a trained model\n");
    printf("  create     Create a new model from scratch\n");
    printf("  help       Show help for a specific command\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm train --data ./data --epochs 10\n");
    printf("  cllm infer --model model.cllm --prompt &quot;Hello&quot;\n");
    printf("  cllm create --vocab 10000 --layers 6 --output model.cllm\n");
    printf("\n");
    printf("For detailed help on a command:\n");
    printf("  cllm help <command>\n");
    printf("\n");
}

// ============================================================================
// TRAIN COMMAND
// ============================================================================

void print_train_help() {
    printf("\nUsage: cllm train [OPTIONS]\n\n");
    printf("Train a CLLM model using the 12-fold kissing spheres architecture.\n\n");
    printf("Required:\n");
    printf("  -d, --data DIR           Training data directory\n");
    printf("\n");
    printf("Model Options:\n");
    printf("  -v, --vocab SIZE         Vocabulary size (default: 10000)\n");
    printf("  -e, --embed DIM          Embedding dimension (default: 256)\n");
    printf("  -l, --layers NUM         Number of layers (default: 6)\n");
    printf("  -H, --heads NUM          Number of attention heads (default: 8)\n");
    printf("\n");
    printf("Training Options:\n");
    printf("  -E, --epochs NUM         Number of epochs (default: 10)\n");
    printf("  -b, --batch SIZE         Batch size (default: 32)\n");
    printf("  -s, --seq-len LEN        Sequence length (default: 128)\n");
    printf("  -r, --lr RATE            Learning rate (default: 0.0001)\n");
    printf("  -t, --threads NUM        Number of threads (0=auto, default: 0)\n");
    printf("\n");
    printf("Output Options:\n");
    printf("  -c, --checkpoint DIR     Checkpoint directory (default: ./checkpoints)\n");
    printf("      --verbose            Enable debug output (default: off)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm train -d ./data -E 10 -b 32\n");
    printf("  cllm train -d ./data -v 5000 -l 4 -t 8\n");
    printf("\n");
}

int cmd_train(int argc, char** argv) {
    // Training parameters with defaults
    const char* data_dir = NULL;
    const char* checkpoint_dir = "./checkpoints";
    
    uint32_t vocab_size = 10000;
    uint32_t embed_dim = 256;
    uint32_t num_layers = 6;
    uint32_t num_heads = 8;
    int epochs = 10;
    int batch_size = 32;
    int seq_len = 128;
    int num_threads = 0;  // 0 = auto
    float learning_rate = 0.0001f;
    int verbose = 0;  // Debug output disabled by default
    
    // Parse options
    static struct option long_options[] = {
        {"data",       required_argument, 0, 'd'},
        {"checkpoint", required_argument, 0, 'c'},
        {"vocab",      required_argument, 0, 'v'},
        {"embed",      required_argument, 0, 'e'},
        {"layers",     required_argument, 0, 'l'},
        {"heads",      required_argument, 0, 'H'},
        {"epochs",     required_argument, 0, 'E'},
        {"batch",      required_argument, 0, 'b'},
        {"seq-len",    required_argument, 0, 's'},
        {"lr",         required_argument, 0, 'r'},
        {"threads",    required_argument, 0, 't'},
        {"verbose",    no_argument,       0, 'V'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "d:c:v:e:l:H:E:b:s:r:t:Vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd': data_dir = optarg; break;
            case 'c': checkpoint_dir = optarg; break;
            case 'v': vocab_size = atoi(optarg); break;
            case 'e': embed_dim = atoi(optarg); break;
            case 'l': num_layers = atoi(optarg); break;
            case 'H': num_heads = atoi(optarg); break;
            case 'E': epochs = atoi(optarg); break;
            case 'b': batch_size = atoi(optarg); break;
            case 's': seq_len = atoi(optarg); break;
            case 'r': learning_rate = atof(optarg); break;
            case 't': num_threads = atoi(optarg); break;
            case 'V': verbose = 1; break;
            case 'h': print_train_help(); return 0;
            default: print_train_help(); return 1;
        }
    }
    
    // Validate required arguments
    if (!data_dir) {
        fprintf(stderr, "Error: --data is required\n");
        print_train_help();
        return 1;
    }
    
    printf("\n=== CLLM Training ===\n\n");
    printf("Configuration:\n");
    printf("  Data directory: %s\n", data_dir);
    printf("  Checkpoint dir: %s\n", checkpoint_dir);
    printf("  Vocab size:     %u\n", vocab_size);
    printf("  Embedding dim:  %u\n", embed_dim);
    printf("  Layers:         %u\n", num_layers);
    printf("  Heads:          %u\n", num_heads);
    printf("  Epochs:         %d\n", epochs);
    printf("  Batch size:     %d\n", batch_size);
    printf("  Sequence len:   %d\n", seq_len);
    printf("  Learning rate:  %.6f\n", learning_rate);
    printf("  Threads:        %d %s\n", num_threads, num_threads == 0 ? "(auto)" : "");
    printf("  Debug output:   %s\n", verbose ? "enabled (requires CLLM_DEBUG=1 build)" : "disabled");
    printf("\n");
    
    // Initialize global progress tracking
    cllm_global_progress_init();
    
    // Create checkpoint directory
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", checkpoint_dir);
    int mkdir_result = system(cmd);
    if (mkdir_result != 0) {
        fprintf(stderr, "Warning: Failed to create checkpoint directory\n");
    }
    
    // Create tokenizer and load data
    cllm_global_progress_start_phase(CLLM_PHASE_LOADING_DATA, "Loading Data Files", 1);
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        fprintf(stderr, "Error: Failed to create tokenizer\n");
        return 1;
    }
    
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        fprintf(stderr, "Error: Failed to create data loader\n");
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    int files_loaded = cllm_data_loader_load_directory(loader, data_dir);
    if (files_loaded == 0) {
        fprintf(stderr, "Error: No data files found in: %s\n", data_dir);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        cllm_global_progress_cleanup();
        return 1;
    }
    cllm_global_progress_update(1);
    cllm_global_progress_complete_phase();
    
    // Build vocabulary
    printf("Building vocabulary...\n");
    cllm_data_loader_build_vocab(loader);
    printf("✓ Vocabulary built\n\n");
    
    // Create training dataset
    printf("Creating training dataset...\n");
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        fprintf(stderr, "Error: Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Dataset created\n\n");
    
    // Auto-adjust parameters for small datasets
    size_t dataset_tokens = dataset->num_tokens;
    size_t tokens_per_batch = batch_size * seq_len;
    size_t estimated_batches = dataset_tokens / tokens_per_batch;
    
    // Get number of threads for comparison
    int actual_threads = num_threads;
    if (actual_threads == 0) {
        actual_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (actual_threads < 1) actual_threads = 1;
        if (actual_threads > 12) actual_threads = 12;
    }
    
    // Warn if too few batches
    if (estimated_batches < (size_t)actual_threads) {
        printf("⚠️  WARNING: Configuration creates only %zu batch(es) for %d workers\n", 
               estimated_batches, actual_threads);
        printf("   This will result in very slow training with poor thread utilization.\n");
        printf("   Recommendation: Use smaller batch/seq-len parameters:\n");
        
        // Calculate better parameters
        int recommended_batch = 4;
        int recommended_seq = 16;
        size_t recommended_batches = dataset_tokens / (recommended_batch * recommended_seq);
        
        printf("   Try: --batch %d --seq-len %d (creates ~%zu batches)\n\n",
               recommended_batch, recommended_seq, recommended_batches);
        
        // Auto-adjust if user didn't explicitly set parameters
        // (We can't easily detect if user set them, so we'll just warn for now)
    }
    
    // Warn if dataset is very small
    if (dataset_tokens < 1000) {
        printf("⚠️  WARNING: Very small dataset (%zu tokens)\n", dataset_tokens);
        printf("   Consider using a larger dataset for better model quality.\n\n");
    }
    
    // Create model with geometric foundation
    printf("Creating model...\n");
    CLLMConfig model_config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = tokenizer->vocab_size,
        .max_seq_len = seq_len * 2,
        .embedding_dim = embed_dim,
        .hidden_dim = embed_dim * 4,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .enable_blind_recovery = true,
        .enable_harmonic_integration = true,
        .enable_ntt_attention = true,
        .enable_kissing_spheres = true,
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
    
    CLLMModel* model = cllm_create_model(&model_config);
    if (!model) {
        fprintf(stderr, "Error: Failed to create model\n");
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    printf("✓ Model created\n\n");
    
    // CRITICAL FIX: Transfer vocabulary from tokenizer to model
    printf("Transferring vocabulary to model...\n");
    uint32_t actual_vocab_size = cllm_get_vocab_size(tokenizer);
    printf("  Tokenizer vocabulary size: %u\n", actual_vocab_size);
    
    if (model->tokens) {
        // Copy vocabulary from tokenizer to model
        for (uint32_t i = 0; i < actual_vocab_size && i < model->vocab_size; i++) {
            const char* token_str = cllm_get_token_string(tokenizer, i);
            if (token_str) {
                strncpy(model->tokens[i].token_str, token_str, sizeof(model->tokens[i].token_str) - 1);
                model->tokens[i].token_str[sizeof(model->tokens[i].token_str) - 1] = '\0';
                model->tokens[i].token_id = i;
                model->tokens[i].frequency = 1;  // Will be updated during training
            }
        }
        
        // Fill remaining slots with special tokens if vocab is smaller than model capacity
        for (uint32_t i = actual_vocab_size; i < model->vocab_size; i++) {
            snprintf(model->tokens[i].token_str, sizeof(model->tokens[i].token_str), "<UNK_%u>", i);
            model->tokens[i].token_id = i;
            model->tokens[i].frequency = 0;
        }
        
        printf("✓ Vocabulary transferred to model (%u tokens)\n\n", actual_vocab_size);
    } else {
        fprintf(stderr, "Warning: model->tokens is NULL, vocabulary not transferred\n");
    }
    
    // Create training configuration
    CLLMTrainingConfig config = {
        .learning_rate = learning_rate,
        .batch_size = batch_size,
        .num_epochs = epochs,
        .max_steps = 0,
        .sequence_length = seq_len,
        .weight_decay = 0.01f,
        .gradient_clip = 1.0f,
        .warmup_steps = 0,  // DISABLED FOR TESTING
        .save_interval = 10,
        .save_every = 5,  // Save checkpoint every 5 epochs
        .eval_interval = 100,
        .optimizer = "adam"
    };
    
    // Pre-compute all embeddings to avoid lazy initialization overhead during training
    extern void cllm_precompute_all_embeddings(CLLMModel* model);
    cllm_precompute_all_embeddings(model);
    printf("\n");
    
    // Initialize training
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Error: Failed to initialize training\n");
        cllm_free_model(model);
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    
    // CRITICAL: Calculate total_batches to prevent segfault
    training->total_batches = dataset->num_tokens / (config.batch_size * config.sequence_length);
    if (training->total_batches == 0 && dataset->num_tokens > 0) {
        fprintf(stderr, "ERROR: Dataset too small for given batch_size and sequence_length\n");
        fprintf(stderr, "  Total tokens: %zu\n", dataset->num_tokens);
        fprintf(stderr, "  Batch size: %d\n", config.batch_size);
        fprintf(stderr, "  Sequence length: %d\n", config.sequence_length);
        fprintf(stderr, "  Tokens per batch: %d\n", config.batch_size * config.sequence_length);
        fprintf(stderr, "  Suggestion: Reduce batch_size or sequence_length\n");
        cllm_token_dataset_free(dataset);
        cllm_free_model(model);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }
    
    // Determine threading mode with proper CPU detection
    int training_threads = num_threads;
    if (training_threads == 0) {
        // Auto-detect CPU cores
        training_threads = sysconf(_SC_NPROCESSORS_ONLN);
        if (training_threads < 1) training_threads = 1;
        
        // Cap at 12 for 12-fold symmetry architecture
        if (training_threads > 12) training_threads = 12;
        
        printf("Auto-detected %d CPU cores, using %d threads\n", 
               (int)sysconf(_SC_NPROCESSORS_ONLN), training_threads);
    }
    
    // Warn if thread count exceeds CPU cores
    int cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (training_threads > cpu_cores) {
        printf("⚠️  WARNING: Using %d threads on %d CPU cores\n", 
               training_threads, cpu_cores);
        printf("   This may cause thread oversubscription and reduced performance.\n");
        printf("   Consider using --threads %d for optimal performance.\n\n", cpu_cores);
    }
    
    bool use_threading = (training_threads > 1);
    
    if (use_threading) {
        printf("Starting THREADED training with %d threads...\n", training_threads);
        printf("Using kissing spheres architecture\n\n");
        
        // Create batch iterator for threaded training
        CLLMBatchIterator* batch_iter = cllm_batch_iterator_create(
            dataset->tokens,
            dataset->num_tokens,
            config.batch_size,
            config.sequence_length,
            0,  // shuffle = false
            1   // drop_last = true
        );
        
        if (!batch_iter) {
            fprintf(stderr, "Error: Failed to create batch iterator\n");
            training->tokens = NULL;
            cllm_training_free(training);
            cllm_token_dataset_free(dataset);
            cllm_data_loader_free(loader);
            cllm_free_tokenizer(tokenizer);
            cllm_free_model(model);
            return 1;
        }
        
        // Create threaded training system
        ThreadedTrainingSystem* threaded_system = threaded_training_create(
            training,
            batch_iter,
            training_threads
        );
        
        if (!threaded_system) {
            fprintf(stderr, "Error: Failed to create threaded training system\n");
            cllm_batch_iterator_free(batch_iter);
            training->tokens = NULL;
            cllm_training_free(training);
            cllm_token_dataset_free(dataset);
            cllm_data_loader_free(loader);
            cllm_free_tokenizer(tokenizer);
            cllm_free_model(model);
            return 1;
        }
        
        // Start training phase
        size_t total_batches = cllm_batch_iterator_num_batches(batch_iter);
        cllm_global_progress_start_phase(CLLM_PHASE_TRAINING, "Training Model", 
                                         config.num_epochs * total_batches);
        
        // Set total epochs for progress tracking
        threaded_training_set_total_epochs(threaded_system, config.num_epochs);
        
        // Training loop with threading
        for (int epoch = 0; epoch < config.num_epochs; epoch++) {
            float epoch_loss = threaded_train_epoch_lockfree(threaded_system, epoch);
            
            // Update training progress
            cllm_global_progress_update_training(epoch + 1, config.num_epochs, epoch_loss);
            cllm_global_progress_update((epoch + 1) * total_batches);
            
            // Save checkpoint
            if ((epoch + 1) % config.save_every == 0) {
                char checkpoint_path[256];
                snprintf(checkpoint_path, sizeof(checkpoint_path), 
                        "checkpoint_epoch_%d.cllm", epoch + 1);
                cllm_write_model(model, checkpoint_path);
                printf("Checkpoint saved: %s\n", checkpoint_path);
            }
        }
        
        // Complete training phase
        cllm_global_progress_complete_phase();
        
        // Cleanup threaded system
        threaded_training_free(threaded_system);
        cllm_batch_iterator_free(batch_iter);
        
    } else {
        printf("Starting SINGLE-THREADED training...\n\n");
        
        // Run the simple training loop
        int train_result = cllm_train(training);
        if (train_result != 0) {
            fprintf(stderr, "Error: Training failed\n");
            training->tokens = NULL;
            cllm_training_free(training);
            cllm_token_dataset_free(dataset);
            cllm_data_loader_free(loader);
            cllm_free_tokenizer(tokenizer);
            cllm_free_model(model);
            return 1;
        }
    }
    
    // Start saving phase
    cllm_global_progress_start_phase(CLLM_PHASE_SAVING, "Saving Model & Vocabulary", 2);
    
    // Save final model
    char final_model_path[512];
    snprintf(final_model_path, sizeof(final_model_path), "%s/final_model.cllm", checkpoint_dir);
    if (cllm_write_model(model, final_model_path) != 0) {
        fprintf(stderr, "Warning: Failed to save final model\n");
    }
    cllm_global_progress_update(1);
    
    // Save vocabulary
    char vocab_path[512];
    snprintf(vocab_path, sizeof(vocab_path), "%s/vocab.txt", checkpoint_dir);
    if (cllm_save_vocab(tokenizer, vocab_path) == 0) {
        fprintf(stderr, "Warning: Failed to save vocabulary\n");
    }
    cllm_global_progress_update(2);
    cllm_global_progress_complete_phase();
    
    // Complete entire program
    cllm_global_progress_complete();
    
    // Cleanup
    training->tokens = NULL;  // Don't free (belongs to dataset)
    cllm_training_free(training);
    cllm_token_dataset_free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    cllm_free_model(model);
    cllm_global_progress_cleanup();
    
    return 0;
}

// ============================================================================
// INFER COMMAND
// ============================================================================

void print_infer_help() {
    printf("\nUsage: cllm infer [OPTIONS]\n\n");
    printf("Run inference on a trained CLLM model.\n\n");
    printf("Required:\n");
    printf("  -m, --model FILE         Model file to load\n");
    printf("  -p, --prompt TEXT        Input prompt\n");
    printf("\n");
    printf("Generation Options:\n");
    printf("  -n, --tokens NUM         Max tokens to generate (default: 50)\n");
    printf("  -t, --temperature T      Sampling temperature (default: 0.8)\n");
    printf("\n");
    printf("Output Options:\n");
    printf("  -o, --output FILE        Save output to file\n");
    printf("  -i, --interactive        Interactive mode\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm infer -m model.cllm -p &quot;Hello world&quot;\n");
    printf("  cllm infer -m model.cllm -p &quot;int main&quot; -n 100 -t 0.5\n");
    printf("  cllm infer -m model.cllm -i\n");
    printf("\n");
}

int cmd_infer(int argc, char** argv) {
    const char* model_file = NULL;
    const char* prompt = NULL;
    const char* output_file = NULL;
    int max_tokens = 50;
    float temperature = 0.8f;
    bool interactive = false;
    
    static struct option long_options[] = {
        {"model",       required_argument, 0, 'm'},
        {"prompt",      required_argument, 0, 'p'},
        {"tokens",      required_argument, 0, 'n'},
        {"temperature", required_argument, 0, 't'},
        {"output",      required_argument, 0, 'o'},
        {"interactive", no_argument,       0, 'i'},
        {"help",        no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "m:p:n:t:o:ih", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm': model_file = optarg; break;
            case 'p': prompt = optarg; break;
            case 'n': max_tokens = atoi(optarg); break;
            case 't': temperature = atof(optarg); break;
            case 'o': output_file = optarg; break;
            case 'i': interactive = true; break;
            case 'h': print_infer_help(); return 0;
            default: print_infer_help(); return 1;
        }
    }
    
    if (!model_file) {
        fprintf(stderr, "Error: --model is required\n");
        print_infer_help();
        return 1;
    }
    
    if (!interactive && !prompt) {
        fprintf(stderr, "Error: --prompt is required (or use --interactive)\n");
        print_infer_help();
        return 1;
    }
    
    // Load model
    printf("\n=== CLLM Inference ===\n\n");
    printf("Loading model from: %s\n", model_file);
    CLLMModel* model = cllm_read_model(model_file);
    if (!model) {
        fprintf(stderr, "Error: Failed to load model\n");
        return 1;
    }
    printf("✓ Model loaded\n\n");
    
    // Try to load vocabulary from same directory as model
    char vocab_path[512];
    char* model_dir = strdup(model_file);
    char* last_slash = strrchr(model_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        snprintf(vocab_path, sizeof(vocab_path), "%s/vocab.txt", model_dir);
    } else {
        snprintf(vocab_path, sizeof(vocab_path), "vocab.txt");
    }
    free(model_dir);
    
    // Load vocabulary
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(model->vocab_size);
    if (tokenizer && cllm_load_vocab(tokenizer, vocab_path) != 0) {
        printf("✓ Vocabulary loaded from: %s\n", vocab_path);
        
        // Copy vocabulary to model tokens
        if (model->tokens) {
            for (uint32_t i = 0; i < tokenizer->vocab_size && i < model->vocab_size; i++) {
                if (tokenizer->vocab[i]) {
                    strncpy(model->tokens[i].token_str, tokenizer->vocab[i], 63);
                    model->tokens[i].token_str[63] = '\0';
                }
            }
        }
    } else {
        fprintf(stderr, "Warning: Could not load vocabulary from %s\n", vocab_path);
        fprintf(stderr, "Inference will use token IDs instead of text\n");
    }
    
    // Initialize inference
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Error: Failed to initialize inference\n");
        if (tokenizer) cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    // Store tokenizer in inference context for tokenization/detokenization
    // Note: This is a workaround - ideally inference should have its own tokenizer field
    if (tokenizer) {
        // We'll need to pass tokenizer to generate function or store it somewhere
        // For now, just keep it alive
    }
    
    // Set parameters
    cllm_set_temperature(inference, temperature);
    cllm_set_max_tokens(inference, max_tokens);
    
    if (interactive) {
        printf("Interactive mode (type 'quit' to exit)\n\n");
        char input[1024];
        while (1) {
            printf("You: ");
            fflush(stdout);
            if (!fgets(input, sizeof(input), stdin)) break;
            
            // Remove newline
            input[strcspn(input, "\n")] = 0;
            
            if (strcmp(input, "quit") == 0) break;
            if (strlen(input) == 0) continue;
            
            char output[2048];
            int tokens = cllm_generate(inference, input, output, sizeof(output));
            
            if (tokens > 0) {
                printf("AI: %s\n\n", output);
            } else {
                printf("AI: (generation failed)\n\n");
            }
        }
    } else {
        printf("Prompt: %s\n", prompt);
        printf("Generating...\n\n");
        
        char output[2048];
        int tokens = cllm_generate(inference, prompt, output, sizeof(output));
        
        if (tokens > 0) {
            printf("Generated (%d tokens):\n%s\n\n", tokens, output);
            
            if (output_file) {
                FILE* f = fopen(output_file, "w");
                if (f) {
                    fprintf(f, "%s\n", output);
                    fclose(f);
                    printf("✓ Output saved to: %s\n", output_file);
                }
            }
        } else {
            fprintf(stderr, "Error: Generation failed\n");
        }
    }
    
    // Cleanup
    cllm_inference_cleanup(inference);
    if (tokenizer) cllm_free_tokenizer(tokenizer);
    cllm_free_model(model);
    
    return 0;
}

// ============================================================================
// CREATE COMMAND
// ============================================================================

void print_create_help() {
    printf("\nUsage: cllm create [OPTIONS]\n\n");
    printf("Create a new CLLM model from scratch.\n\n");
    printf("Model Architecture:\n");
    printf("  -v, --vocab SIZE         Vocabulary size (default: 10000)\n");
    printf("  -e, --embed DIM          Embedding dimension (default: 256)\n");
    printf("  -l, --layers NUM         Number of layers (default: 6)\n");
    printf("  -H, --heads NUM          Number of attention heads (default: 8)\n");
    printf("\n");
    printf("Output:\n");
    printf("  -o, --output FILE        Output model file (default: model.cllm)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm create -v 5000 -l 4 -o small_model.cllm\n");
    printf("  cllm create -v 50000 -e 512 -l 12 -H 16\n");
    printf("\n");
}

int cmd_create(int argc, char** argv) {
    uint32_t vocab_size = 10000;
    uint32_t embed_dim = 256;
    uint32_t num_layers = 6;
    uint32_t num_heads = 8;
    const char* output_file = "model.cllm";
    
    static struct option long_options[] = {
        {"vocab",   required_argument, 0, 'v'},
        {"embed",   required_argument, 0, 'e'},
        {"layers",  required_argument, 0, 'l'},
        {"heads",   required_argument, 0, 'H'},
        {"output",  required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "v:e:l:H:o:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v': vocab_size = atoi(optarg); break;
            case 'e': embed_dim = atoi(optarg); break;
            case 'l': num_layers = atoi(optarg); break;
            case 'H': num_heads = atoi(optarg); break;
            case 'o': output_file = optarg; break;
            case 'h': print_create_help(); return 0;
            default: print_create_help(); return 1;
        }
    }
    
    printf("\n=== Creating CLLM Model ===\n\n");
    printf("Configuration:\n");
    printf("  Vocab size:     %u\n", vocab_size);
    printf("  Embedding dim:  %u\n", embed_dim);
    printf("  Layers:         %u\n", num_layers);
    printf("  Heads:          %u\n", num_heads);
    printf("  Output file:    %s\n", output_file);
    printf("\n");
    
    CLLMConfig config = {
        .solid_type = PLATONIC_CUBE,
        .vocab_size = vocab_size,
        .max_seq_len = 512,
        .embedding_dim = embed_dim,
        .hidden_dim = embed_dim * 4,
        .num_layers = num_layers,
        .num_heads = num_heads,
        .enable_blind_recovery = true,
        .enable_harmonic_integration = true,
        .enable_ntt_attention = true,
        .enable_kissing_spheres = true,
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
        return 1;
    }
    printf("✓ Model created\n\n");
    
    printf("Saving model to: %s\n", output_file);
    if (cllm_write_model(model, output_file) != 0) {
        fprintf(stderr, "Error: Failed to save model\n");
        cllm_free_model(model);
        return 1;
    }
    printf("✓ Model saved\n\n");
    
    cllm_free_model(model);
    printf("Model creation complete!\n");
    return 0;
}

// ============================================================================
// MAIN DISPATCHER
// ============================================================================

int main(int argc, char** argv) {
    if (argc < 2) {
        print_main_help();
        return 1;
    }
    
    const char* command = argv[1];
    
    // Shift arguments for subcommand
    argc--;
    argv++;
    
    if (strcmp(command, "train") == 0) {
        return cmd_train(argc, argv);
    } else if (strcmp(command, "infer") == 0) {
        return cmd_infer(argc, argv);
    } else if (strcmp(command, "create") == 0) {
        return cmd_create(argc, argv);
    } else if (strcmp(command, "help") == 0) {
        if (argc >= 2) {
            const char* help_cmd = argv[1];
            if (strcmp(help_cmd, "train") == 0) print_train_help();
            else if (strcmp(help_cmd, "infer") == 0) print_infer_help();
            else if (strcmp(help_cmd, "create") == 0) print_create_help();
            else print_main_help();
        } else {
            print_main_help();
        }
        return 0;
    } else {
        fprintf(stderr, "Unknown command: %s\n\n", command);
        print_main_help();
        return 1;
    }
}
