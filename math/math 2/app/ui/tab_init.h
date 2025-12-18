#ifndef TAB_INIT_H
#define TAB_INIT_H

#include "app_common.h"

/**
 * Initialize all UI tabs that require setup
 * This must be called during application initialization
 */
void init_all_tabs(AppState* state);

/**
 * Cleanup all UI tabs
 * This must be called during application shutdown
 */
void cleanup_all_tabs(AppState* state);

#endif /* TAB_INIT_H */