/**
 * CLLM - Unified Command-Line Interface
 * 
 * Single robust CLI tool that mirrors all UI functionality.
 * Replaces: train_model, cllm_inference, cllm_tokenize, cllm_vocab_build, etc.
 * 
 * Usage:
 *   cllm train [options]     - Train a model
 *   cllm infer [options]     - Run inference
 *   cllm create [options]    - Create a new model
 *   cllm tokenize [options]  - Tokenize text
 *   cllm vocab [options]     - Build vocabulary
 *   cllm test [options]      - Run tests
 *   cllm help [command]      - Show help
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "cllm.h"
#include "cllm_training.h"
#include "cllm_training_threaded.h"
#include "cllm_inference.h"
#include "cllm_tokenizer.h"
#include "cllm_vocab_builder.h"
#include "cllm_batch.h"
#include "cllm_data_loader.h"
#include "cllm_format.h"
#include "cllm_utils.h"

// ============================================================================
// BANNER & VERSION
// ============================================================================

#define CLLM_VERSION "1.0.0"

void print_banner() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         CLLM - Crystalline Lattice Language Model         ║\n");
    printf("║                    Version %s                          ║\n", CLLM_VERSION);
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
    printf("  tokenize   Tokenize text files\n");
    printf("  vocab      Build vocabulary from corpus\n");
    printf("  test       Run system tests\n");
    printf("  help       Show help for a specific command\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm train --data corpus.txt --epochs 10\n");
    printf("  cllm infer --model model.cllm --prompt &quot;Hello&quot;\n");
    printf("  cllm create --vocab 10000 --layers 6\n");
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
    printf("  -d, --data FILE          Training data file\n");
    printf("\n");
    printf("Model Options:\n");
    printf("  -m, --model FILE         Load existing model (or create new)\n");
    printf("  -v, --vocab SIZE         Vocabulary size (default: 10000)\n");
    printf("  -e, --embed DIM          Embedding dimension (default: 256)\n");
    printf("  -l, --layers NUM         Number of layers (default: 6)\n");
    printf("  -h, --heads NUM          Number of attention heads (default: 8)\n");
    printf("\n");
    printf("Training Options:\n");
    printf("  -E, --epochs NUM         Number of epochs (default: 10)\n");
    printf("  -b, --batch SIZE         Batch size (default: 32)\n");
    printf("  -s, --seq-len LEN        Sequence length (default: 128)\n");
    printf("  -r, --lr RATE            Learning rate (default: 0.001)\n");
    printf("  -t, --threads NUM        Number of threads (0=auto, default: 0)\n");
    printf("\n");
    printf("Output Options:\n");
    printf("  -o, --output FILE        Output model file (default: model.cllm)\n");
    printf("  -c, --checkpoint DIR     Checkpoint directory\n");
    printf("  --save-interval NUM      Save every N epochs (default: 5)\n");
    printf("\n");
    printf("Framework Options:\n");
    printf("  --lattice                Use lattice embeddings (default: on)\n");
    printf("  --angular                Use angular attention (default: on)\n");
    printf("  --crystalline            Use crystalline loss (default: on)\n");
    printf("  --ntt                    Use NTT attention (default: off)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm train -d corpus.txt -E 10 -b 32\n");
    printf("  cllm train -d data.txt -m existing.cllm -E 5\n");
    printf("  cllm train -d corpus.txt -v 5000 -l 4 -t 8\n");
    printf("\n");
}

int cmd_train(int argc, char** argv) {
    // Training parameters with defaults
    const char* data_file = NULL;
    const char* model_file = NULL;
    const char* output_file = "model.cllm";
    const char* checkpoint_dir = NULL;
    
    int vocab_size = 10000;
    int embed_dim = 256;
    int num_layers = 6;
    int num_heads = 8;
    int epochs = 10;
    int batch_size = 32;
    int seq_len = 128;
    int num_threads = 0;  // 0 = auto
    int save_interval = 5;
    float learning_rate = 0.001f;
    
    // Parse options
    static struct option long_options[] = {
        {"data",          required_argument, 0, 'd'},
        {"model",         required_argument, 0, 'm'},
        {"output",        required_argument, 0, 'o'},
        {"vocab",         required_argument, 0, 'v'},
        {"embed",         required_argument, 0, 'e'},
        {"layers",        required_argument, 0, 'l'},
        {"heads",         required_argument, 0, 'h'},
        {"epochs",        required_argument, 0, 'E'},
        {"batch",         required_argument, 0, 'b'},
        {"seq-len",       required_argument, 0, 's'},
        {"lr",            required_argument, 0, 'r'},
        {"threads",       required_argument, 0, 't'},
        {"checkpoint",    required_argument, 0, 'c'},
        {"save-interval", required_argument, 0, 'S'},
        {"help",          no_argument,       0, 'H'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "d:m:o:v:e:l:H:E:b:s:r:t:c:S:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd': data_file = optarg; break;
            case 'm': model_file = optarg; break;
            case 'o': output_file = optarg; break;
            case 'v': vocab_size = atoi(optarg); break;
            case 'e': embed_dim = atoi(optarg); break;
            case 'l': num_layers = atoi(optarg); break;
            case 'h': num_heads = atoi(optarg); break;
            case 'E': epochs = atoi(optarg); break;
            case 'b': batch_size = atoi(optarg); break;
            case 's': seq_len = atoi(optarg); break;
            case 'r': learning_rate = atof(optarg); break;
            case 't': num_threads = atoi(optarg); break;
            case 'c': checkpoint_dir = optarg; break;
            case 'S': save_interval = atoi(optarg); break;
            case 'H': print_train_help(); return 0;
            default: print_train_help(); return 1;
        }
    }
    
    // Validate required arguments
    if (!data_file) {
        fprintf(stderr, "Error: --data is required\n");
        print_train_help();
        return 1;
    }
    
    printf("\n=== CLLM Training ===\n\n");
    printf("Configuration:\n");
    printf("  Data file:      %s\n", data_file);
    printf("  Model file:     %s\n", model_file ? model_file : "(create new)");
    printf("  Output file:    %s\n", output_file);
    printf("  Vocab size:     %d\n", vocab_size);
    printf("  Embedding dim:  %d\n", embed_dim);
    printf("  Layers:         %d\n", num_layers);
    printf("  Heads:          %d\n", num_heads);
    printf("  Epochs:         %d\n", epochs);
    printf("  Batch size:     %d\n", batch_size);
    printf("  Sequence len:   %d\n", seq_len);
    printf("  Learning rate:  %.6f\n", learning_rate);
    printf("  Threads:        %d %s\n", num_threads, num_threads == 0 ? "(auto)" : "");
    printf("\n");
    
    // Step 1: Load or create model
    CLLMModel* model = NULL;
    if (model_file) {
        printf("Loading model from %s...\n", model_file);
        model = cllm_read_model(model_file);
        if (!model) {
            fprintf(stderr, "Failed to load model\n");
            return 1;
        }
        printf("Model loaded\n\n");
    } else {
        printf("Creating new model...\n");
        CLLMConfig* config = cllm_create_config(vocab_size, embed_dim, num_layers, num_heads, embed_dim * 4);
        if (!config) {
            fprintf(stderr, "Failed to create config\n");
            return 1;
        }
        model = cllm_create_model(config);
        cllm_free_config(config);
        if (!model) {
            fprintf(stderr, "Failed to create model\n");
            return 1;
        }
        printf("Model created\n\n");
    }
    
    // Step 2: Load training data
    printf("Loading training data from %s...\n", data_file);
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(vocab_size);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        cllm_free_model(model);
        return 1;
    }
    
    CLLMDataLoader* loader = cllm_data_loader_create(tokenizer);
    if (!loader) {
        fprintf(stderr, "Failed to create data loader\n");
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    if (cllm_data_loader_load_file(loader, data_file) != 0) {
        fprintf(stderr, "Failed to load data file\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    TokenDataset* dataset = cllm_data_loader_create_dataset(loader);
    if (!dataset) {
        fprintf(stderr, "Failed to create dataset\n");
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    printf("Dataset loaded: %zu tokens\n\n", dataset->num_tokens);
    
    // Step 3: Create training context
    printf("Initializing training...\n");
    CLLMTrainingConfig config = {
        .num_epochs = epochs,
        .batch_size = batch_size,
        .sequence_length = seq_len,
        .learning_rate = learning_rate,
        .gradient_accumulation_steps = 1,
        .save_every = save_interval,
        .use_mixed_precision = false
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    training->tokens = dataset->tokens;
    training->num_tokens = dataset->num_tokens;
    training->total_batches = (dataset->num_tokens / (batch_size * seq_len));
    printf("Training initialized\n\n");
    
    // Step 4: Train
    printf("Starting training...\n");
    int result = cllm_train(training);
    if (result != 0) {
        fprintf(stderr, "Training failed\n");
        cllm_training_cleanup(training);
        cllm_token_dataset_free(dataset);
        cllm_data_loader_free(loader);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    // Step 5: Save model
    printf("\nSaving model to %s...\n", output_file);
    if (cllm_write_model(model, output_file) != 0) {
        fprintf(stderr, "Failed to save model\n");
    } else {
        printf("Model saved successfully\n");
    }
    
    // Cleanup
    cllm_training_cleanup(training);
    cllm_token_dataset_free(dataset);
    cllm_data_loader_free(loader);
    cllm_free_tokenizer(tokenizer);
    cllm_free_model(model);
    
    printf("\n=== Training Complete ===\n");
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
    printf("  -k, --top-k K            Top-k sampling (default: 40)\n");
    printf("  --top-p P                Top-p (nucleus) sampling (default: 0.9)\n");
    printf("\n");
    printf("Output Options:\n");
    printf("  -o, --output FILE        Save output to file\n");
    printf("  -v, --verbose            Show generation details\n");
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
    int top_k = 40;
    float top_p = 0.9f;
    bool verbose = false;
    bool interactive = false;
    
    static struct option long_options[] = {
        {"model",       required_argument, 0, 'm'},
        {"prompt",      required_argument, 0, 'p'},
        {"tokens",      required_argument, 0, 'n'},
        {"temperature", required_argument, 0, 't'},
        {"top-k",       required_argument, 0, 'k'},
        {"top-p",       required_argument, 0, 'P'},
        {"output",      required_argument, 0, 'o'},
        {"verbose",     no_argument,       0, 'v'},
        {"interactive", no_argument,       0, 'i'},
        {"help",        no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "m:p:n:t:k:P:o:vih", long_options, NULL)) != -1) {
        switch (opt) {
            case 'm': model_file = optarg; break;
            case 'p': prompt = optarg; break;
            case 'n': max_tokens = atoi(optarg); break;
            case 't': temperature = atof(optarg); break;
            case 'k': top_k = atoi(optarg); break;
            case 'P': top_p = atof(optarg); break;
            case 'o': output_file = optarg; break;
            case 'v': verbose = true; break;
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
    
    printf("\n=== CLLM Inference ===\n\n");
    printf("Model: %s\n", model_file);
    if (interactive) {
        printf("Mode: Interactive\n");
    } else {
        printf("Prompt: %s\n", prompt);
        printf("Max tokens: %d\n", max_tokens);
        printf("Temperature: %.2f\n", temperature);
    }
    printf("\n");
    
    // Load model
    printf("Loading model...\n");
    CLLMModel* model = cllm_read_model(model_file);
    if (!model) {
        fprintf(stderr, "Failed to load model from %s\n", model_file);
        return 1;
    }
    printf("Model loaded\n\n");
    
    // Create inference context
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to initialize inference\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Set generation parameters
    cllm_set_temperature(inference, temperature);
    cllm_set_top_p(inference, top_p);
    cllm_set_max_tokens(inference, max_tokens);
    
    if (interactive) {
        // Interactive mode
        printf("Interactive mode (type 'quit' to exit)\n\n");
        char input[1024];
        char output[4096];
        
        while (1) {
            printf("> ");
            fflush(stdout);
            
            if (!fgets(input, sizeof(input), stdin)) {
                break;
            }
            
            // Remove newline
            input[strcspn(input, "\n")] = 0;
            
            if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
                break;
            }
            
            if (strlen(input) == 0) {
                continue;
            }
            
            // Generate
            int tokens = cllm_generate(inference, input, output, sizeof(output));
            if (tokens > 0) {
                printf("%s\n\n", output);
            } else {
                printf("Generation failed\n\n");
            }
        }
    } else {
        // Single prompt mode
        char output[4096];
        printf("Generating...\n");
        int tokens = cllm_generate(inference, prompt, output, sizeof(output));
        
        if (tokens > 0) {
            printf("\nGenerated text:\n");
            printf("%s\n", output);
            printf("\nTokens generated: %d\n", tokens);
            
            // Save to file if requested
            if (output_file) {
                FILE* f = fopen(output_file, "w");
                if (f) {
                    fprintf(f, "%s\n", output);
                    fclose(f);
                    printf("Output saved to: %s\n", output_file);
                }
            }
        } else {
            fprintf(stderr, "Generation failed\n");
            cllm_inference_cleanup(inference);
            cllm_free_model(model);
            return 1;
        }
    }
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_free_model(model);
    
    printf("\n=== Inference Complete ===\n");
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
    printf("  -h, --heads NUM          Number of attention heads (default: 8)\n");
    printf("  -f, --ff-dim DIM         Feedforward dimension (default: 1024)\n");
    printf("\n");
    printf("Output:\n");
    printf("  -o, --output FILE        Output model file (default: model.cllm)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  cllm create -v 5000 -l 4 -o small_model.cllm\n");
    printf("  cllm create -v 50000 -e 512 -l 12 -h 16\n");
    printf("\n");
}

int cmd_create(int argc, char** argv) {
    // TODO: Implement model creation
    printf("Model creation implementation coming soon...\n");
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