/**
 * YAML Parser and Extractor
 * 
 * Simple YAML parser that extracts text values.
 * Handles basic YAML structures (key-value pairs, lists, nested objects).
 */

#include "../docproc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Simple YAML to text converter
 * Extracts values from YAML structure
 */
static void yaml_to_text(const char* yaml, char* output, size_t output_size) {
    const char* p = yaml;
    char* out = output;
    char* out_end = output + output_size - 1;
    
    while (*p && out < out_end) {
        // Skip leading whitespace
        while (*p && isspace(*p) && *p != '\n') p++;
        
        // Skip comments
        if (*p == '#') {
            while (*p && *p != '\n') p++;
            if (*p == '\n') p++;
            continue;
        }
        
        // Handle key-value pairs
        if (*p && *p != '\n' && *p != '-') {
            // Find colon
            const char* colon = strchr(p, ':');
            if (colon && colon < strchr(p, '\n')) {
                // Extract key
                const char* key_start = p;
                const char* key_end = colon;
                
                // Skip whitespace before colon
                while (key_end > key_start && isspace(*(key_end - 1))) key_end--;
                
                // Write key
                size_t key_len = key_end - key_start;
                if (out + key_len + 2 < out_end) {
                    memcpy(out, key_start, key_len);
                    out += key_len;
                    *out++ = ':';
                    *out++ = ' ';
                }
                
                // Move past colon
                p = colon + 1;
                
                // Skip whitespace after colon
                while (*p && isspace(*p) && *p != '\n') p++;
                
                // Extract value
                const char* value_start = p;
                const char* value_end = strchr(p, '\n');
                if (!value_end) value_end = p + strlen(p);
                
                // Skip trailing whitespace
                while (value_end > value_start && isspace(*(value_end - 1))) value_end--;
                
                // Write value
                size_t value_len = value_end - value_start;
                if (value_len > 0 && out + value_len + 1 < out_end) {
                    memcpy(out, value_start, value_len);
                    out += value_len;
                    *out++ = '\n';
                }
                
                p = value_end;
                if (*p == '\n') p++;
                continue;
            }
        }
        
        // Handle list items
        if (*p == '-' && (p == yaml || *(p-1) == '\n')) {
            p++;  // Skip dash
            while (*p && isspace(*p) && *p != '\n') p++;
            
            // Extract list item
            const char* item_start = p;
            const char* item_end = strchr(p, '\n');
            if (!item_end) item_end = p + strlen(p);
            
            // Skip trailing whitespace
            while (item_end > item_start && isspace(*(item_end - 1))) item_end--;
            
            // Write list item
            size_t item_len = item_end - item_start;
            if (item_len > 0 && out + item_len + 3 < out_end) {
                *out++ = '-';
                *out++ = ' ';
                memcpy(out, item_start, item_len);
                out += item_len;
                *out++ = '\n';
            }
            
            p = item_end;
            if (*p == '\n') p++;
            continue;
        }
        
        // Skip empty lines
        if (*p == '\n') {
            p++;
            continue;
        }
        
        // Skip unhandled content
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
    
    *out = '\0';
}

/**
 * Extract text from YAML
 */
DocProcStatus docproc_extract_yaml(const char* filepath, char* text, size_t text_size) {
    // Read YAML file
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        return DOCPROC_ERROR_FILE_NOT_FOUND;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* yaml_content = (char*)malloc(size + 1);
    if (!yaml_content) {
        fclose(fp);
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    fread(yaml_content, 1, size, fp);
    yaml_content[size] = '\0';
    fclose(fp);
    
    // Convert YAML to text
    yaml_to_text(yaml_content, text, text_size);
    
    free(yaml_content);
    
    if (strlen(text) == 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}