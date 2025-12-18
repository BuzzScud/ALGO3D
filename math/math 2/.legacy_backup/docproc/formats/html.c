/**
 * HTML Extractor
 * 
 * Pure C implementation of HTML tag removal and text extraction.
 */

#include "../docproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Remove HTML tags from text
 */
static void remove_html_tags(const char* html, char* text, size_t text_size) {
    const char* p = html;
    char* out = text;
    char* out_end = text + text_size - 1;
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;
    
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
        
        // Handle tags
        if (*p == '<') {
            in_tag = 1;
            p++;
            continue;
        } else if (*p == '>') {
            in_tag = 0;
            p++;
            // Add space after tag
            if (out > text && *(out-1) != ' ' && *(out-1) != '\n') {
                *out++ = ' ';
            }
            continue;
        }
        
        // Skip tag content
        if (in_tag) {
            p++;
            continue;
        }
        
        // Copy text content
        *out++ = *p++;
    }
    
    *out = '\0';
}

/**
 * Clean and normalize text
 */
static void clean_text(char* text) {
    char* src = text;
    char* dst = text;
    int last_was_space = 1;
    
    while (*src) {
        // Convert HTML entities (simple version)
        if (strncmp(src, "&nbsp;", 6) == 0) {
            *dst++ = ' ';
            src += 6;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&lt;", 4) == 0) {
            *dst++ = '<';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&gt;", 4) == 0) {
            *dst++ = '>';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&amp;", 5) == 0) {
            *dst++ = '&';
            src += 5;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&quot;", 6) == 0) {
            *dst++ = '"';
            src += 6;
            last_was_space = 0;
            continue;
        }
        
        // Normalize whitespace
        if (isspace(*src)) {
            if (!last_was_space) {
                *dst++ = ' ';
                last_was_space = 1;
            }
            src++;
        } else {
            *dst++ = *src++;
            last_was_space = 0;
        }
    }
    
    *dst = '\0';
    
    // Trim leading/trailing whitespace
    while (*text && isspace(*text)) text++;
    
    size_t len = strlen(text);
    while (len > 0 && isspace(text[len-1])) {
        text[len-1] = '\0';
        len--;
    }
}

/**
 * Extract text from HTML
 */
DocProcStatus docproc_extract_html(const char* filepath, char* text, size_t text_size) {
    // Read HTML file
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* html = (char*)malloc(size + 1);
    if (!html) {
        fclose(fp);
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    fread(html, 1, size, fp);
    html[size] = '\0';
    fclose(fp);
    
    // Remove tags
    remove_html_tags(html, text, text_size);
    free(html);
    
    // Clean text
    clean_text(text);
    
    return DOCPROC_SUCCESS;
}