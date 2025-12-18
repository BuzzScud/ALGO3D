/**
 * Simple Format Extractors
 * 
 * Extractors for simple text-based formats that don't require complex parsing.
 */

#include "../docproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Extract text from plain text file
 */
DocProcStatus docproc_extract_txt(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    size_t bytes_read = fread(text, 1, text_size - 1, fp);
    text[bytes_read] = '\0';
    fclose(fp);
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from Markdown (preserve structure)
 */
DocProcStatus docproc_extract_markdown(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    // Read file preserving markdown structure
    size_t bytes_read = fread(text, 1, text_size - 1, fp);
    text[bytes_read] = '\0';
    fclose(fp);
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from CSV
 */
DocProcStatus docproc_extract_csv(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    size_t bytes_read = fread(text, 1, text_size - 1, fp);
    text[bytes_read] = '\0';
    fclose(fp);
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from SQL
 */
DocProcStatus docproc_extract_sql(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    char* out = text;
    char* out_end = text + text_size - 1;
    char line[4096];
    
    // Read line by line, skip comments
    while (fgets(line, sizeof(line), fp) && out < out_end) {
        // Skip SQL comments
        char* trimmed = line;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        
        if (strncmp(trimmed, "--", 2) == 0) continue;
        if (trimmed[0] == '\0') continue;
        
        // Copy line
        size_t len = strlen(line);
        if (out + len < out_end) {
            strcpy(out, line);
            out += len;
        }
    }
    
    *out = '\0';
    fclose(fp);
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from TOML
 */
DocProcStatus docproc_extract_toml(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return DOCPROC_ERROR_FILE_NOT_FOUND;
    
    size_t bytes_read = fread(text, 1, text_size - 1, fp);
    text[bytes_read] = '\0';
    fclose(fp);
    
    return DOCPROC_SUCCESS;
}