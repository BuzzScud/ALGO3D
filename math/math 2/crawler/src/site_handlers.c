#include "site_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Maximum number of handlers
#define MAX_HANDLERS 32

// Handler registry
static struct {
    const SiteHandler* handlers[MAX_HANDLERS];
    int count;
    bool initialized;
} g_registry = {0};

/**
 * Initialize the handler registry
 */
void site_handlers_init(void) {
    if (g_registry.initialized) return;
    
    memset(&g_registry, 0, sizeof(g_registry));
    g_registry.initialized = true;
    
    printf("Site handlers registry initialized\n");
}

/**
 * Register a new site handler
 */
int site_handlers_register(const SiteHandler* handler) {
    if (!handler) {
        fprintf(stderr, "ERROR: Cannot register NULL handler\n");
        return -1;
    }
    
    if (!g_registry.initialized) {
        site_handlers_init();
    }
    
    if (g_registry.count >= MAX_HANDLERS) {
        fprintf(stderr, "ERROR: Handler registry full (max %d handlers)\n", MAX_HANDLERS);
        return -1;
    }
    
    // Check for duplicate names
    for (int i = 0; i < g_registry.count; i++) {
        if (strcmp(g_registry.handlers[i]->name, handler->name) == 0) {
            fprintf(stderr, "ERROR: Handler '%s' already registered\n", handler->name);
            return -1;
        }
    }
    
    g_registry.handlers[g_registry.count++] = handler;
    printf("Registered site handler: %s (pattern: %s)\n", handler->name, handler->url_pattern);
    
    return 0;
}

/**
 * Find a handler for the given URL
 */
const SiteHandler* site_handlers_find(const char* url) {
    if (!url || !g_registry.initialized) return NULL;
    
    // Try each handler in order
    for (int i = 0; i < g_registry.count; i++) {
        const SiteHandler* handler = g_registry.handlers[i];
        
        // Use custom can_handle function if provided
        if (handler->can_handle) {
            if (handler->can_handle(url)) {
                return handler;
            }
        }
        // Otherwise use simple pattern matching
        else if (handler->url_pattern) {
            if (url_matches_pattern(url, handler->url_pattern)) {
                return handler;
            }
        }
    }
    
    return NULL;
}

/**
 * Get handler by name
 */
const SiteHandler* site_handlers_get(const char* name) {
    if (!name || !g_registry.initialized) return NULL;
    
    for (int i = 0; i < g_registry.count; i++) {
        if (strcmp(g_registry.handlers[i]->name, name) == 0) {
            return g_registry.handlers[i];
        }
    }
    
    return NULL;
}

/**
 * List all registered handlers
 */
int site_handlers_list(const SiteHandler** handlers, int max_handlers) {
    if (!handlers || max_handlers <= 0) return 0;
    if (!g_registry.initialized) return 0;
    
    int count = g_registry.count < max_handlers ? g_registry.count : max_handlers;
    for (int i = 0; i < count; i++) {
        handlers[i] = g_registry.handlers[i];
    }
    
    return count;
}

/**
 * Cleanup all handlers and free registry
 */
void site_handlers_cleanup(void) {
    if (!g_registry.initialized) return;
    
    // Call cleanup function for each handler
    for (int i = 0; i < g_registry.count; i++) {
        if (g_registry.handlers[i]->cleanup) {
            g_registry.handlers[i]->cleanup();
        }
    }
    
    memset(&g_registry, 0, sizeof(g_registry));
    printf("Site handlers registry cleaned up\n");
}

/**
 * Check if URL matches pattern (case-insensitive substring match)
 */
bool url_matches_pattern(const char* url, const char* pattern) {
    if (!url || !pattern) return false;
    
    // Convert both to lowercase for comparison
    char url_lower[1024];
    char pattern_lower[256];
    
    size_t url_len = strlen(url);
    size_t pattern_len = strlen(pattern);
    
    if (url_len >= sizeof(url_lower)) url_len = sizeof(url_lower) - 1;
    if (pattern_len >= sizeof(pattern_lower)) pattern_len = sizeof(pattern_lower) - 1;
    
    for (size_t i = 0; i < url_len; i++) {
        url_lower[i] = tolower((unsigned char)url[i]);
    }
    url_lower[url_len] = '\0';
    
    for (size_t i = 0; i < pattern_len; i++) {
        pattern_lower[i] = tolower((unsigned char)pattern[i]);
    }
    pattern_lower[pattern_len] = '\0';
    
    // Check if pattern is substring of URL
    return strstr(url_lower, pattern_lower) != NULL;
}

/**
 * Extract domain from URL
 */
int extract_domain(const char* url, char* domain, size_t domain_size) {
    if (!url || !domain || domain_size == 0) return -1;
    
    // Find protocol separator
    const char* start = strstr(url, "://");
    if (start) {
        start += 3; // Skip "://"
    } else {
        start = url;
    }
    
    // Find end of domain (first '/', '?', or '#')
    const char* end = start;
    while (*end && *end != '/' && *end != '?' && *end != '#') {
        end++;
    }
    
    // Copy domain
    size_t len = end - start;
    if (len >= domain_size) len = domain_size - 1;
    
    strncpy(domain, start, len);
    domain[len] = '\0';
    
    return 0;
}
