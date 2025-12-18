#!/usr/bin/env python3
import re

with open('app/ui/tabs/tab_models.c', 'r') as f:
    content = f.read()

# Find the on_load_button_click function
pattern = r'(static void on_load_button_click\(void\* user_data\) \{)(.*?)(\n\})'
match = re.search(pattern, content, re.DOTALL)

if match:
    new_function = '''static void on_load_button_click(void* user_data) {
    AppState* state = (AppState*)user_data;
    
    // Check if a model is selected
    if (selected_model_index < 0) {
        snprintf(status_message, sizeof(status_message), 
                 "Please select a model from the list");
        status_message_timer = 3.0f;
        return;
    }
    
    // Get selected model name
    ManagedModel** models = NULL;
    uint32_t model_count = 0;
    model_manager_list_models(&models, &model_count);
    
    if (selected_model_index >= (int)model_count || !models[selected_model_index]) {
        snprintf(status_message, sizeof(status_message), "Invalid model selection");
        status_message_timer = 3.0f;
        return;
    }
    
    const char* model_name = models[selected_model_index]->name;
    printf("Loading selected model: %s\\n", model_name);
    
    // CRITICAL: Prepare model before loading (expands abacus if needed)
    printf("Preparing model '%s'...\\n", model_name);
    if (!model_manager_prepare(model_name)) {
        snprintf(status_message, sizeof(status_message), 
                 "Failed to prepare model '%s'", model_name);
        status_message_timer = 3.0f;
        return;
    }
    printf("✓ Model prepared successfully\\n");
    
    // Now acquire the model for use
    CLLMModel* model = model_manager_acquire_read(model_name);
    if (!model) {
        snprintf(status_message, sizeof(status_message), 
                 "Failed to load model '%s'", model_name);
        status_message_timer = 3.0f;
        return;
    }
    
    // Update state manager
    StateManager* state_mgr = state_manager_get_instance();
    state_set_model(state_mgr, model, model_name, models[selected_model_index]->path);
    
    // Update model config
    ManagedModel* managed = models[selected_model_index];
    state_update_model_config(state_mgr, 
                            managed->vocab_size,
                            managed->embedding_dim,
                            0,  // ff_dim not in metadata
                            managed->num_layers,
                            managed->num_heads);
    
    // Dispatch MODEL_LOADED event for cross-tab synchronization
    EventSystem* event_sys = event_system_get_instance();
    event_model_loaded(event_sys, model_name);
    
    snprintf(status_message, sizeof(status_message), 
             "Model '%s' loaded successfully", model_name);
    status_message_timer = 3.0f;
    
    printf("✓ Model '%s' loaded and ready for use\\n", model_name);
    
    (void)state;
}'''
    
    content = content[:match.start()] + new_function + content[match.end():]
    
    with open('app/ui/tabs/tab_models.c', 'w') as f:
        f.write(content)
    
    print("✓ Updated on_load_button_click function")
else:
    print("✗ Could not find function")

