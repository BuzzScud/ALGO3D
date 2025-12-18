/**
 * EPUB Extractor
 * 
 * Pure C implementation using ZIP extraction and HTML parsing.
 * EPUB is a ZIP archive containing XHTML chapters.
 */

#include "../docproc.h"
#include "../utils/zip_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define TEMP_BUFFER_SIZE (10 * 1024 * 1024)  // 10MB temp buffer

/**
 * Simple HTML tag removal (reused from html.c logic)
 */
static void remove_html_tags_simple(const char* html, char* text, size_t text_size) {
    const char* p = html;
    char* out = text;
    char* out_end = text + text_size - 1;
    int in_tag = 0;
    
    while (*p && out < out_end) {
        if (*p == '<') {
            in_tag = 1;
            p++;
            continue;
        } else if (*p == '>') {
            in_tag = 0;
            p++;
            if (out > text && *(out-1) != ' ' && *(out-1) != '\n') {
                *out++ = ' ';
            }
            continue;
        }
        
        if (in_tag) {
            p++;
            continue;
        }
        
        *out++ = *p++;
    }
    
    *out = '\0';
}

/**
 * Callback for listing EPUB files
 */
typedef struct {
    char* output;
    char* output_end;
    char* temp_buffer;
    const char* zip_path;
} EpubContext;

static void epub_file_callback(const char* filename, void* user_data) {
    EpubContext* ctx = (EpubContext*)user_data;
    
    // Only process XHTML/HTML files
    if (!strstr(filename, ".xhtml") && !strstr(filename, ".html")) {
        return;
    }
    
    // Skip if output buffer is full
    if (ctx->output >= ctx->output_end - 1000) {
        return;
    }
    
    // Extract chapter
    int bytes_read = zip_extract_file(ctx->zip_path, filename,
                                     ctx->temp_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read > 0) {
        // Remove HTML tags
        size_t available = ctx->output_end - ctx->output;
        remove_html_tags_simple(ctx->temp_buffer, ctx->output, available);
        ctx->output += strlen(ctx->output);
        
        // Add chapter separator
        if (ctx->output < ctx->output_end - 4) {
            *ctx->output++ = '\n';
            *ctx->output++ = '\n';
        }
    }
}

/**
 * Extract text from EPUB
 */
DocProcStatus docproc_extract_epub(const char* filepath, char* text, size_t text_size) {
    char* temp_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!temp_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Set up context for file callback
    EpubContext ctx;
    ctx.output = text;
    ctx.output_end = text + text_size - 1;
    ctx.temp_buffer = temp_buffer;
    ctx.zip_path = filepath;
    
    // List all files in EPUB and process XHTML/HTML files
    int result = zip_list_files(filepath, epub_file_callback, &ctx);
    
    *ctx.output = '\0';
    free(temp_buffer);
    
    if (result < 0 || ctx.output == text) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}