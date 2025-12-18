/**
 * PDF File Processor
 * 
 * Extracts text from PDF files using poppler-utils (pdftotext)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Extract text from PDF file
 * 
 * Uses pdftotext command from poppler-utils
 * Returns 0 on success, -1 on failure
 */
int process_pdf_file(const char* input_path, const char* output_path) {
    // Check if pdftotext is available
    if (system("which pdftotext > /dev/null 2>&1") != 0) {
        fprintf(stderr, "pdftotext not found. Install poppler-utils.\n");
        return -1;
    }
    
    // Create temporary file for pdftotext output
    char temp_path[2048];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", output_path);
    
    // Build pdftotext command
    // -layout: Preserve layout
    // -nopgbrk: No page breaks
    // -enc UTF-8: UTF-8 encoding
    char command[4096];
    snprintf(command, sizeof(command), 
             "pdftotext -layout -nopgbrk -enc UTF-8 '%s' '%s' 2>/dev/null",
             input_path, temp_path);
    
    // Execute pdftotext
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "pdftotext failed for: %s\n", input_path);
        unlink(temp_path);
        return -1;
    }
    
    // Check if output file was created and has content
    FILE* f = fopen(temp_path, "r");
    if (!f) {
        fprintf(stderr, "No output from pdftotext: %s\n", input_path);
        return -1;
    }
    
    // Check file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    if (size < 10) {
        fprintf(stderr, "PDF extraction produced too little text: %ld bytes\n", size);
        unlink(temp_path);
        return -1;
    }
    
    // Move temp file to final location
    rename(temp_path, output_path);
    
    printf("  ✓ Extracted %ld bytes from PDF\n", size);
    return 0;
}