// control_thread.c - Asynchronous Control Thread for Background Initialization
// Part of CRYSTALLINE CLLM - Implements MASTER_PLAN architecture

#include "app_common.h"
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "cllm_model_registry.h"

// Control thread handle
static pthread_t g_control_thread = 0;
static bool g_control_thread_running = false;

// Forward declarations
extern int app_initialize_global_abacus(void);

/**
 * Control Thread Main Function
 * 
 * This thread handles ALL heavy initialization in the background:
 * 1. Crystalline abacus (rainbow table) initialization
 * 2. Model loading (if default model exists)
 * 3. Inference context creation
 * 4. Worker thread spawning (when ready)
 * 
 * The main application loop continues running while this happens.
 * UI shows initialization status and enables features when ready.
 */
void* control_thread_main(void* arg) {
    AppState* state = (AppState*)arg;
    
    printf("\n=== Control Thread Started ===\n");
    printf("Main loop continues running - UI is responsive\n\n");
    
    // PHASE 1: Initialize Crystalline Abacus (Rainbow Table)
    printf("=== Phase 1: Initializing Crystalline Abacus ===\n");
    state->abacus_initializing = true;
    state->abacus_ready = false;
    
    if (app_initialize_global_abacus() == 0) {
        state->abacus_ready = true;
        printf("✓ Abacus initialization complete\n");
    } else {
        fprintf(stderr, "✗ Abacus initialization failed\n");
    }
    state->abacus_initializing = false;
    
    // PHASE 2: Scan for Models (OBJECTIVE 26 - read metadata only)
    printf("\n=== Phase 2: Scanning for Models ===\n");
    
    // Scan models directory for .cllm files
    DIR* models_dir = opendir("models");
    if (models_dir) {
        struct dirent* entry;
        int found_count = 0;
        
        while ((entry = readdir(models_dir)) != NULL) {
            // Check if file ends with .cllm
            size_t len = strlen(entry->d_name);
            if (len > 5 && strcmp(entry->d_name + len - 5, ".cllm") == 0) {
                char model_path[512];
                snprintf(model_path, sizeof(model_path), "models/%s", entry->d_name);
                
                // Read metadata from registry
                const ModelMetadata* metadata = model_registry_get(entry->d_name);
                if (metadata) {
                    uint64_t required_primes = metadata->vocab_size;  // Use vocab_size as estimate
                    
                    printf("  Found: %s\n", entry->d_name);
                    printf("    Vocab: %u | Layers: %u | Primes needed: %lu\n",
                           metadata->vocab_size,
                           metadata->num_layers,
                           (unsigned long)required_primes);
                    
                    // Check if abacus has enough primes
                    extern int rainbow_table_get_count(void);
                    int available_primes = rainbow_table_get_count();
                    
                    if ((uint64_t)available_primes >= required_primes) {
                        printf("    Status: ✓ Accessible (abacus has %d primes)\n", available_primes);
                    } else {
                        printf("    Status: ⚠ Needs Preparation (abacus has %d primes, needs %lu)\n",
                               available_primes, (unsigned long)required_primes);
                    }
                    
                    found_count++;
                }
            }
        }
        closedir(models_dir);
        
        if (found_count > 0) {
            printf("\nFound %d model(s) - use Models tab to prepare/access them\n", found_count);
            state->model_ready = false;  // Models exist but not prepared yet
            snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                    "Found %d model(s). Use Models tab to prepare and access them.", found_count);
        } else {
            printf("No models found in models/ directory\n");
            state->model_ready = false;
            snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                    "No models found. Create a new model in Training tab.");
        }
    } else {
        printf("Models directory not found - will be created when needed\n");
        state->model_ready = false;
        snprintf(state->llm_output_text, sizeof(state->llm_output_text),
                "No models directory. Create a new model in Training tab.");
    }
    
    // PHASE 3: System Ready
    printf("\n=== Control Thread Initialization Complete ===\n");
    printf("System is now ready for use\n");
    printf("Abacus: %s | Model: %s\n",
           state->abacus_ready ? "✓ Ready" : "✗ Not Ready",
           state->model_ready ? "✓ Ready" : "✗ Not Ready");
    printf("\n");
    
    // Control thread continues running for coordination
    // In full implementation, this would spawn worker threads
    // and coordinate training/inference operations
    
    while (g_control_thread_running) {
        // Control thread coordination loop
        // TODO: Implement worker thread management
        // TODO: Implement batch coordination
        // TODO: Implement 12-fold symmetry distribution
        sleep(1);
    }
    
    printf("=== Control Thread Exiting ===\n");
    return NULL;
}

/**
 * Start Control Thread
 * 
 * Creates and starts the control thread for background initialization.
 * Returns immediately - main loop continues running.
 */
void start_control_thread(AppState* state) {
    if (g_control_thread_running) {
        printf("Control thread already running\n");
        return;
    }
    
    g_control_thread_running = true;
    
    if (pthread_create(&g_control_thread, NULL, control_thread_main, state) != 0) {
        fprintf(stderr, "ERROR: Failed to create control thread\n");
        g_control_thread_running = false;
        return;
    }
    
    printf("✓ Control thread started (background initialization)\n");
}

/**
 * Stop Control Thread
 * 
 * Signals control thread to stop and waits for it to exit.
 */
void stop_control_thread(void) {
    if (!g_control_thread_running) {
        return;
    }
    
    printf("Stopping control thread...\n");
    g_control_thread_running = false;
    
    if (g_control_thread) {
        pthread_join(g_control_thread, NULL);
        g_control_thread = 0;
    }
    
    printf("✓ Control thread stopped\n");
}