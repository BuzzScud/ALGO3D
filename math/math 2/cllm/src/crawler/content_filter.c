// content_filter.c - Smart content extraction implementation
#include "content_filter.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Check if a name (class or ID) indicates boilerplate content
 */
int is_boilerplate_name(const char* name) {
    if (!name) return 0;
    
    // Common boilerplate patterns
    const char* boilerplate_patterns[] = {
        "header", "footer", "sidebar", "aside",
        "banner", "cookie", "popup", "modal",
        "advertisement", "ad-", "ads-", "promo",
        NULL
    };
    
    for (int i = 0; boilerplate_patterns[i]; i++) {
        if (strstr(name, boilerplate_patterns[i])) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Check if a name indicates navigation
 */
int is_navigation_name(const char* name) {
    if (!name) return 0;
    
    const char* nav_patterns[] = {
        "nav", "menu", "breadcrumb", "pagination",
        "sitemap", "toc", "table-of-contents",
        NULL
    };
    
    for (int i = 0; nav_patterns[i]; i++) {
        if (strstr(name, nav_patterns[i])) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Check if a name indicates metadata
 */
int is_metadata_name(const char* name) {
    if (!name) return 0;
    
    const char* metadata_patterns[] = {
        "author", "date", "time", "category",
        "tag", "meta", "byline", "published",
        NULL
    };
    
    for (int i = 0; metadata_patterns[i]; i++) {
        if (strstr(name, metadata_patterns[i])) {
            return 1;
        }
    }
    
    return 0;
}

/**
 * Classify HTML element based on tag, class, and ID
 */
ContentType classify_html_element(const char* tag, const char* class_attr, const char* id_attr) {
    if (!tag) return CONTENT_UNKNOWN;
    
    // Check semantic HTML5 tags first
    if (strcasecmp(tag, "article") == 0 || 
        strcasecmp(tag, "main") == 0 ||
        strcasecmp(tag, "section") == 0) {
        return CONTENT_MAIN;
    }
    
    if (strcasecmp(tag, "nav") == 0 || 
        strcasecmp(tag, "menu") == 0) {
        return CONTENT_NAVIGATION;
    }
    
    if (strcasecmp(tag, "header") == 0 || 
        strcasecmp(tag, "footer") == 0) {
        return CONTENT_BOILERPLATE;
    }
    
    if (strcasecmp(tag, "aside") == 0) {
        return CONTENT_SIDEBAR;
    }
    
    if (strcasecmp(tag, "time") == 0) {
        return CONTENT_METADATA;
    }
    
    // Check class and ID attributes
    if (class_attr) {
        if (is_navigation_name(class_attr)) return CONTENT_NAVIGATION;
        if (is_boilerplate_name(class_attr)) return CONTENT_BOILERPLATE;
        if (is_metadata_name(class_attr)) return CONTENT_METADATA;
    }
    
    if (id_attr) {
        if (is_navigation_name(id_attr)) return CONTENT_NAVIGATION;
        if (is_boilerplate_name(id_attr)) return CONTENT_BOILERPLATE;
        if (is_metadata_name(id_attr)) return CONTENT_METADATA;
    }
    
    // Content tags (paragraphs, headings, etc.)
    if (strcasecmp(tag, "p") == 0 ||
        strcasecmp(tag, "h1") == 0 ||
        strcasecmp(tag, "h2") == 0 ||
        strcasecmp(tag, "h3") == 0 ||
        strcasecmp(tag, "h4") == 0 ||
        strcasecmp(tag, "h5") == 0 ||
        strcasecmp(tag, "h6") == 0 ||
        strcasecmp(tag, "blockquote") == 0 ||
        strcasecmp(tag, "pre") == 0) {
        return CONTENT_MAIN;
    }
    
    return CONTENT_UNKNOWN;
}

/**
 * Extract tag name, class, and ID from opening tag
 */
static int parse_tag(const char* tag_start, char* tag_name, size_t tag_name_size,
                     char* class_attr, size_t class_size,
                     char* id_attr, size_t id_size) {
    const char* p = tag_start;
    
    // Skip '<'
    if (*p != '<') return -1;
    p++;
    
    // Skip '/' for closing tags
    if (*p == '/') return -1;  // We only care about opening tags
    
    // Extract tag name
    char* tag_out = tag_name;
    char* tag_end = tag_name + tag_name_size - 1;
    while (*p && !isspace(*p) && *p != '>' && tag_out < tag_end) {
        *tag_out++ = tolower(*p++);
    }
    *tag_out = '\0';
    
    // Look for class and id attributes
    class_attr[0] = '\0';
    id_attr[0] = '\0';
    
    while (*p && *p != '>') {
        // Skip whitespace
        while (*p && isspace(*p)) p++;
        
        // Check for class attribute
        if (strncasecmp(p, "class=", 6) == 0) {
            p += 6;
            if (*p == '"' || *p == '\'') {
                char quote = *p++;
                char* class_out = class_attr;
                char* class_end = class_attr + class_size - 1;
                while (*p && *p != quote && class_out < class_end) {
                    *class_out++ = tolower(*p++);
                }
                *class_out = '\0';
                if (*p == quote) p++;
            }
        }
        // Check for id attribute
        else if (strncasecmp(p, "id=", 3) == 0) {
            p += 3;
            if (*p == '"' || *p == '\'') {
                char quote = *p++;
                char* id_out = id_attr;
                char* id_end = id_attr + id_size - 1;
                while (*p && *p != quote && id_out < id_end) {
                    *id_out++ = tolower(*p++);
                }
                *id_out = '\0';
                if (*p == quote) p++;
            }
        }
        else {
            p++;
        }
    }
    
    return 0;
}

/**
 * Extract content from HTML based on extraction mode
 */
int extract_content_smart(const char* html, char* output, size_t output_size, ExtractionMode mode) {
    if (!html || !output || output_size == 0) return -1;
    
    const char* p = html;
    char* out = output;
    char* out_end = output + output_size - 1;
    
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;
    int skip_depth = 0;  // Depth of tags we're skipping
    
    // Stack to track content types at each depth
    #define MAX_DEPTH 100
    ContentType type_stack[MAX_DEPTH] = {CONTENT_UNKNOWN};
    int stack_pos = 0;
    
    // Suppress unused warnings - type_stack is used for future enhancements
    (void)type_stack;
    
    while (*p && out < out_end) {
        // Check for script/style tags
        if (strncasecmp(p, "<script", 7) == 0) {
            in_script = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</script>", 9) == 0) {
            in_script = 0;
            p += 9;
            continue;
        } else if (strncasecmp(p, "<style", 6) == 0) {
            in_style = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</style>", 8) == 0) {
            in_style = 0;
            p += 8;
            continue;
        }
        
        // Skip script/style content
        if (in_script || in_style) {
            p++;
            continue;
        }
        
        // Handle opening tags
        if (*p == '<' && *(p+1) != '/') {
            in_tag = 1;
            
            // Parse tag to classify content
            char tag_name[64];
            char class_attr[256];
            char id_attr[256];
            
            if (parse_tag(p, tag_name, sizeof(tag_name), 
                         class_attr, sizeof(class_attr),
                         id_attr, sizeof(id_attr)) == 0) {
                
                ContentType type = classify_html_element(tag_name, class_attr, id_attr);
                
                // Push type onto stack
                if (stack_pos < MAX_DEPTH - 1) {
                    type_stack[++stack_pos] = type;
                }
                
                // Determine if we should skip this content based on mode
                int should_skip = 0;
                
                switch (mode) {
                    case EXTRACT_ALL:
                        // Never skip
                        should_skip = 0;
                        break;
                        
                    case EXTRACT_HUMAN_TEXT:
                        // Skip navigation, boilerplate, sidebar
                        if (type == CONTENT_NAVIGATION || 
                            type == CONTENT_BOILERPLATE || 
                            type == CONTENT_SIDEBAR) {
                            should_skip = 1;
                        }
                        break;
                        
                    case EXTRACT_METADATA:
                        // Only keep metadata
                        if (type != CONTENT_METADATA && type != CONTENT_UNKNOWN) {
                            should_skip = 1;
                        }
                        break;
                        
                    case EXTRACT_MIXED:
                        // Skip navigation and boilerplate, keep content and metadata
                        if (type == CONTENT_NAVIGATION || type == CONTENT_BOILERPLATE) {
                            should_skip = 1;
                        }
                        break;
                }
                
                if (should_skip && skip_depth == 0) {
                    skip_depth = stack_pos;
                }
            }
            
            p++;
            continue;
        }
        // Handle closing tags
        else if (*p == '<' && *(p+1) == '/') {
            in_tag = 1;
            
            // Pop from stack
            if (stack_pos > 0) {
                // If we're closing the tag we started skipping at, stop skipping
                if (skip_depth == stack_pos) {
                    skip_depth = 0;
                }
                stack_pos--;
            }
            
            p++;
            continue;
        }
        // Handle end of tag
        else if (*p == '>') {
            in_tag = 0;
            p++;
            // Add space after tag if not skipping
            if (skip_depth == 0 && out > output && *(out-1) != ' ' && *(out-1) != '\n') {
                *out++ = ' ';
            }
            continue;
        }
        
        // Skip tag content
        if (in_tag) {
            p++;
            continue;
        }
        
        // Skip content if we're in a skipped section
        if (skip_depth > 0) {
            p++;
            continue;
        }
        
        // Copy text content
        *out++ = *p++;
    }
    
    *out = '\0';
    return 0;
}