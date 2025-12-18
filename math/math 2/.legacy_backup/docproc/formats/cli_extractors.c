/**
 * CLI-Based Extractors
 * 
 * Extractors that use external CLI tools (pdftotext, antiword, unrtf, tesseract, jq).
 */

#include "../docproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Execute command and capture output
 */
static DocProcStatus execute_command(const char* cmd, char* output, size_t output_size) {
    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    size_t bytes_read = fread(output, 1, output_size - 1, fp);
    output[bytes_read] = '\0';
    
    int status = pclose(fp);
    if (status != 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from PDF using pdftotext
 */
DocProcStatus docproc_extract_pdf(const char* filepath, char* text, size_t text_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "pdftotext -layout -nopgbrk '%s' -", filepath);
    return execute_command(cmd, text, text_size);
}

/**
 * Extract text from DOC using antiword
 */
DocProcStatus docproc_extract_doc(const char* filepath, char* text, size_t text_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "antiword '%s'", filepath);
    return execute_command(cmd, text, text_size);
}

/**
 * Extract text from RTF using unrtf
 */
DocProcStatus docproc_extract_rtf(const char* filepath, char* text, size_t text_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "unrtf --text '%s'", filepath);
    return execute_command(cmd, text, text_size);
}

/**
 * Extract text from image using tesseract OCR
 */
DocProcStatus docproc_extract_image_ocr(const char* filepath, char* text, size_t text_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "tesseract '%s' stdout 2>/dev/null", filepath);
    return execute_command(cmd, text, text_size);
}

/**
 * Extract text from JSON using jq
 */
DocProcStatus docproc_extract_json(const char* filepath, char* text, size_t text_size) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "jq -r '.. | strings' '%s' 2>/dev/null", filepath);
    return execute_command(cmd, text, text_size);
}