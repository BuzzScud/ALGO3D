// app/input_registration.c - Centralized Input Registration System
// This file ensures ALL tabs have their inputs registered at startup

#include "app_common.h"
#include "input_manager.h"
#include <stdio.h>

// Helper function to register training tab inputs
static void register_training_inputs_internal(InputManager* manager) {
    // Training tab uses control panel on right side
    // Panel X = SIDEBAR_WIDTH + RENDER_WIDTH = 200 + 1080 = 1280
    int panel_x = 1280 + 10;  // RENDER_OFFSET_X + RENDER_WIDTH + padding
    int input_width = 300;     // CONTROL_PANEL_WIDTH - 20
    
    input_manager_register(manager, "training.learning_rate", TAB_TRAINING, INPUT_TYPE_NUMBER,
                         (SDL_Rect){panel_x, 417, input_width, 25});
    input_manager_set_text(manager, "training.learning_rate", "0.001");
    
    input_manager_register(manager, "training.epochs", TAB_TRAINING, INPUT_TYPE_NUMBER,
                         (SDL_Rect){panel_x, 474, input_width, 25});
    input_manager_set_text(manager, "training.epochs", "10");
    
    input_manager_register(manager, "training.batch_size", TAB_TRAINING, INPUT_TYPE_NUMBER,
                         (SDL_Rect){panel_x, 531, input_width, 25});
    input_manager_set_text(manager, "training.batch_size", "32");
    
    input_manager_register(manager, "training.thread_count", TAB_TRAINING, INPUT_TYPE_NUMBER,
                         (SDL_Rect){panel_x, 588, input_width, 25});
    input_manager_set_text(manager, "training.thread_count", "0");
    
    input_manager_register(manager, "training.crawler_url", TAB_TRAINING, INPUT_TYPE_URL,
                         (SDL_Rect){panel_x, 686, input_width, 30});
    input_manager_set_text(manager, "training.crawler_url", "");
}

// Helper function to register crawler tab inputs
static void register_crawler_inputs_internal(InputManager* manager) {
    // Crawler tab uses 3-column layout
    // Layout calculation: content_width = 1360px, 3 columns = 440px each
    // Column 1: X = 220 (SIDEBAR + margin)
    // Column 2: X = 680 (220 + 440 + 20 spacing)
    // Column 3: X = 1140 (680 + 440 + 20 spacing)
    int col1_x = 230;  // 220 + 10 padding
    int col2_x = 690;  // 680 + 10 padding
    int col_width = 420;  // 440 - 20 (10px padding each side)
    
    // Column 1 inputs (prime configuration)
    input_manager_register(manager, "crawler.frequency", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){col1_x, 115, 150, 22});
    input_manager_set_text(manager, "crawler.frequency", "7");
    
    input_manager_register(manager, "crawler.selection", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){col1_x, 163, 150, 22});
    input_manager_set_text(manager, "crawler.selection", "13");
    
    input_manager_register(manager, "crawler.delay_min", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){col1_x, 211, 150, 22});
    input_manager_set_text(manager, "crawler.delay_min", "3");
    
    input_manager_register(manager, "crawler.delay_max", TAB_CRAWLER, INPUT_TYPE_NUMBER,
                         (SDL_Rect){col1_x, 259, 150, 22});
    input_manager_set_text(manager, "crawler.delay_max", "11");
    
    // Column 2 input (add URL) - fixed width to fit within column
    input_manager_register(manager, "crawler.add_url", TAB_CRAWLER, INPUT_TYPE_URL,
                         (SDL_Rect){col2_x, 123, col_width, 22});
    input_manager_set_text(manager, "crawler.add_url", "");
}

// Helper function to register LLM tab inputs
static void register_llm_inputs_internal(InputManager* manager) {
    // LLM tab has chat input at bottom
    // Made smaller and more reasonable - single line input
    int input_x = 210;
    int input_y = 860;  // Moved down slightly
    int input_width = 950;  // Reasonable width
    int input_height = 40;  // Single line height
    
    input_manager_register(manager, "llm.chat_input", TAB_LLM, INPUT_TYPE_TEXT,
                         (SDL_Rect){input_x, input_y, input_width, input_height});
    input_manager_set_text(manager, "llm.chat_input", "");
}

// Helper function to register research tab inputs
static void register_research_inputs_internal(InputManager* manager) {
    // Research tab has search input at top
    int search_x = 220;
    int search_y = 60;
    int search_width = 400;
    
    input_manager_register(manager, "research.search", TAB_RESEARCH, INPUT_TYPE_TEXT,
                         (SDL_Rect){search_x, search_y, search_width, 30});
    input_manager_set_text(manager, "research.search", "");
}

// Helper function to register models tab inputs
static void register_models_inputs_internal(InputManager* manager) {
    // Models tab has create dialog inputs (will be shown when dialog is open)
    // For now, register with placeholder positions
    int dialog_x = 550;
    int dialog_y = 250;
    
    input_manager_register(manager, "models.name", TAB_MODELS, INPUT_TYPE_TEXT,
                         (SDL_Rect){dialog_x, dialog_y, 400, 30});
    input_manager_set_text(manager, "models.name", "");
    
    input_manager_register(manager, "models.vocab_size", TAB_MODELS, INPUT_TYPE_NUMBER,
                         (SDL_Rect){dialog_x, dialog_y + 50, 400, 30});
    input_manager_set_text(manager, "models.vocab_size", "10000");
    
    input_manager_register(manager, "models.embedding_dim", TAB_MODELS, INPUT_TYPE_NUMBER,
                         (SDL_Rect){dialog_x, dialog_y + 100, 400, 30});
    input_manager_set_text(manager, "models.embedding_dim", "512");
    
    input_manager_register(manager, "models.num_layers", TAB_MODELS, INPUT_TYPE_NUMBER,
                         (SDL_Rect){dialog_x, dialog_y + 150, 400, 30});
    input_manager_set_text(manager, "models.num_layers", "6");
    
    input_manager_register(manager, "models.num_heads", TAB_MODELS, INPUT_TYPE_NUMBER,
                         (SDL_Rect){dialog_x, dialog_y + 200, 400, 30});
    input_manager_set_text(manager, "models.num_heads", "8");
}

// Helper function to register URL manager tab inputs
static void register_url_manager_inputs_internal(InputManager* manager) {
    // URL Manager tab has search and add URL inputs
    int search_x = 230;
    int search_y = 60;
    int add_url_x = 230;
    int add_url_y = 100;
    
    input_manager_register(manager, "url_manager.search", TAB_URL_MANAGER, INPUT_TYPE_TEXT,
                         (SDL_Rect){search_x, search_y, 400, 30});
    input_manager_set_text(manager, "url_manager.search", "");
    
    input_manager_register(manager, "url_manager.add_url", TAB_URL_MANAGER, INPUT_TYPE_URL,
                         (SDL_Rect){add_url_x, add_url_y, 600, 30});
    input_manager_set_text(manager, "url_manager.add_url", "");
}

/**
 * Initialize all inputs for all tabs at startup.
 * This ensures inputs are available immediately, regardless of which tab is visited first.
 * 
 * CRITICAL: This function MUST be called during app initialization, before the main loop.
 */
void init_all_inputs(InputManager* manager, void* state) {
    if (!manager) {
        printf("ERROR: Cannot initialize inputs - InputManager is NULL\n");
        return;
    }
    if (!state) {
        printf("ERROR: Cannot initialize inputs - AppState is NULL\n");
        return;
    }
    
    printf("\n=== Initializing All Tab Inputs ===\n");
    
    // Training tab (TAB_TRAINING = 6)
    printf("Registering Training tab inputs...\n");
    register_training_inputs_internal(manager);
    
    // Crawler tab (TAB_CRAWLER = 8)
    printf("Registering Crawler tab inputs...\n");
    register_crawler_inputs_internal(manager);
    
    // LLM tab (TAB_LLM = 5)
    printf("Registering LLM tab inputs...\n");
    register_llm_inputs_internal(manager);
    
    // Set callbacks for LLM input (defined in tab_llm.c)
    ManagedInput* llm_input = input_manager_get(manager, "llm.chat_input");
    if (llm_input) {
        extern void llm_input_on_change(const char* text, void* user_data);
        extern void llm_input_on_submit(const char* text, void* user_data);
        llm_input->on_change = llm_input_on_change;
        llm_input->on_submit = llm_input_on_submit;
        llm_input->user_data = state;
    }
    
    // Research tab (TAB_RESEARCH = 7)
    printf("Registering Research tab inputs...\n");
    register_research_inputs_internal(manager);
    
    // Models tab (TAB_MODELS = 9)
    printf("Registering Models tab inputs...\n");
    register_models_inputs_internal(manager);
    
    // URL Manager tab (TAB_URL_MANAGER = 10)
    printf("Registering URL Manager tab inputs...\n");
    register_url_manager_inputs_internal(manager);
    
    printf("=== Input Registration Complete ===\n");
    printf("Total inputs registered: %d\n\n", manager->input_count);
}