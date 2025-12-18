/**
 * CLLM Model Manager CLI Tool
 * 
 * Command-line interface for managing CLLM models
 */

#include "cllm.h"
#include "cllm_model_manager.h"
#include "cllm_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  list                  List all models in model manager\n");
    printf("  create <name>         Create a new model\n");
    printf("  delete <name>         Delete a model\n");
    printf("  info <name>           Show model information\n");
    printf("  save <name>           Save model to disk\n");
    printf("  load <name> <file>    Load model from file\n");
    printf("\nCreate Options:\n");
    printf("  --vocab-size <n>      Vocabulary size (default: 10000)\n");
    printf("  --embed-dim <n>       Embedding dimension (default: 256)\n");
    printf("  --num-layers <n>      Number of layers (default: 6)\n");
    printf("  --num-heads <n>       Number of attention heads (default: 8)\n");
    printf("\nExamples:\n");
    printf("  %s list\n", program_name);
    printf("  %s create my_model --vocab-size 5000 --embed-dim 512\n", program_name);
    printf("  %s info my_model\n", program_name);
    printf("  %s save my_model\n", program_name);
    printf("  %s delete my_model\n", program_name);
}

static void cmd_list(void) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Model Manager - List                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    uint32_t count = 0;
    ManagedModel** models = model_manager_list(&count);
    
    if (!models) {
        printf("Error: Failed to list models\n");
        return;
    }
    
    if (count == 0) {
        printf("No models found in model manager.\n");
        printf("Use 'create' command to create a new model.\n");
        free(models);
        return;
    }
    
    printf("Found %u model(s):\n\n", count);
    
    for (uint32_t i = 0; i < count; i++) {
        printf("  %u. %s\n", i + 1, models[i]->name);
        
        // Get model status
        bool is_loaded = false;
        bool is_training = false;
        uint32_t read_count = 0;
        
        if (model_manager_get_status(models[i]->name, &is_loaded, &is_training, &read_count)) {
            printf("     Loaded: %s\n", is_loaded ? "Yes" : "No");
            if (is_training) {
                printf("     Status: Training (write lock)\n");
            } else if (read_count > 0) {
                printf("     Status: In use (%u readers)\n", read_count);
            } else {
                printf("     Status: Available\n");
            }
        }
        printf("\n");
    }
    
    free(models);
}

static void cmd_create(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Error: Missing model name\n");
        fprintf(stderr, "Usage: cllm_model_manager create <name> [options]\n");
        return;
    }
    
    const char* model_name = argv[2];
    
    // Parse options
    uint32_t vocab_size = 10000;
    uint32_t embed_dim = 256;
    uint32_t num_layers = 6;
    uint32_t num_heads = 8;
    
    for (int i = 3; i < argc - 1; i++) {
        if (strcmp(argv[i], "--vocab-size") == 0) {
            vocab_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--embed-dim") == 0) {
            embed_dim = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--num-layers") == 0) {
            num_layers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--num-heads") == 0) {
            num_heads = atoi(argv[++i]);
        }
    }
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  Model Manager - Create                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Creating model '%s' with configuration:\n", model_name);
    printf("  Vocabulary size:  %u\n", vocab_size);
    printf("  Embedding dim:    %u\n", embed_dim);
    printf("  Num layers:       %u\n", num_layers);
    printf("  Num heads:        %u\n", num_heads);
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
    
    if (model_manager_create(model_name, &config) != 0) {
        fprintf(stderr, "Error: Failed to create model\n");
        return;
    }
    
    printf("✓ Model '%s' created successfully\n", model_name);
}

static void cmd_delete(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Error: Missing model name\n");
        fprintf(stderr, "Usage: cllm_model_manager delete <name>\n");
        return;
    }
    
    const char* model_name = argv[2];
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  Model Manager - Delete                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Deleting model '%s'...\n", model_name);
    
    // Delete model and its file
    if (!model_manager_delete(model_name, true)) {
        fprintf(stderr, "Error: Failed to delete model\n");
        return;
    }
    
    printf("✓ Model '%s' deleted successfully\n", model_name);
}

static void cmd_info(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Error: Missing model name\n");
        fprintf(stderr, "Usage: cllm_model_manager info <name>\n");
        return;
    }
    
    const char* model_name = argv[2];
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   Model Manager - Info                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Check if model exists
    if (!model_manager_exists(model_name)) {
        fprintf(stderr, "Error: Model '%s' not found\n", model_name);
        return;
    }
    
    // Get model status
    bool is_loaded = false;
    bool is_training = false;
    uint32_t read_count = 0;
    
    printf("Model: %s\n", model_name);
    
    if (model_manager_get_status(model_name, &is_loaded, &is_training, &read_count)) {
        printf("Loaded: %s\n", is_loaded ? "Yes" : "No");
        if (is_training) {
            printf("Status: Training (write lock)\n");
        } else if (read_count > 0) {
            printf("Status: In use (%u readers)\n", read_count);
        } else {
            printf("Status: Available\n");
        }
    }
    
    // Try to acquire model for reading to get details
    CLLMModel* model = model_manager_acquire_read(model_name);
    if (model) {
        printf("\nModel Configuration:\n");
        printf("  Vocabulary size:  %lu\n", (unsigned long)model->vocab_size);
        printf("  Embedding dim:    %lu\n", (unsigned long)model->embedding_dim);
        printf("  Num layers:       %u\n", model->num_layers);
        printf("  Num heads:        %u\n", model->num_heads);
        printf("  Hidden dim:       %u\n", model->hidden_dim);
        printf("  Num layers:       %u\n", model->num_layers);
        printf("  Total params:     %lu\n", (unsigned long)model->header.total_params);
        
        model_manager_release_read(model_name);
    }
}

static void cmd_save(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Error: Missing model name\n");
        fprintf(stderr, "Usage: cllm_model_manager save <name>\n");
        return;
    }
    
    const char* model_name = argv[2];
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   Model Manager - Save                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Saving model '%s' to disk...\n", model_name);
    
    if (!model_manager_save(model_name)) {
        fprintf(stderr, "Error: Failed to save model\n");
        return;
    }
    
    printf("✓ Model saved successfully\n");
}

static void cmd_load(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "Error: Missing arguments\n");
        fprintf(stderr, "Usage: cllm_model_manager load <name> <file>\n");
        return;
    }
    
    const char* model_name = argv[2];
    const char* file_path = argv[3];
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   Model Manager - Load                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Loading model from '%s' as '%s'...\n", file_path, model_name);
    
    if (!model_manager_load(model_name, file_path)) {
        fprintf(stderr, "Error: Failed to load model\n");
        return;
    }
    
    printf("✓ Model loaded successfully\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "list") == 0) {
        cmd_list();
    } else if (strcmp(command, "create") == 0) {
        cmd_create(argc, argv);
    } else if (strcmp(command, "delete") == 0) {
        cmd_delete(argc, argv);
    } else if (strcmp(command, "info") == 0) {
        cmd_info(argc, argv);
    } else if (strcmp(command, "save") == 0) {
        cmd_save(argc, argv);
    } else if (strcmp(command, "load") == 0) {
        cmd_load(argc, argv);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}