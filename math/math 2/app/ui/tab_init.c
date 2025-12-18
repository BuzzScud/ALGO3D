#include "app_common.h"
#include "ui/tabs/tab_models.h"
#include "ui/tabs/tab_training.h"
#include "ui/tabs/tab_video.h"
#include "ui/tabs/tab_crawler.h"
#include "ui/tabs/tab_llm.h"

/**
 * Initialize all UI tabs that require setup
 * This must be called during application initialization
 */
void init_all_tabs(AppState* state) {
    if (!state) {
        printf("ERROR: Cannot initialize tabs - AppState is NULL\n");
        return;
    }
    
    printf("\n=== Initializing UI Tabs ===\n");
    
    // Models Tab
    printf("Initializing Models tab...\n");
    init_models_tab(state);
    
    // Training Tab
    printf("Initializing Training tab...\n");
    init_training_tab(state);
    
    // Crawler Tab
    printf("Initializing Crawler tab...\n");
    init_crawler_tab(state);
    
    // Video Tab
    printf("Initializing Video tab...\n");
    init_video_tab();
    
    printf("=== Tab Initialization Complete ===\n\n");
}

/**
 * Cleanup all UI tabs
 * This must be called during application shutdown
 */
void cleanup_all_tabs(AppState* state) {
    if (!state) return;
    
    printf("\n=== Cleaning Up UI Tabs ===\n");
    
    // Models Tab
    cleanup_models_tab();
    
    // Crawler Tab
    cleanup_crawler_tab();
    
    // LLM Tab
    cleanup_llm_tab();
    
    // Training Tab
    cleanup_training_tab();
    
    // Video Tab (no cleanup function exists)
    // cleanup_video_tab();
    
    printf("=== Tab Cleanup Complete ===\n\n");
}