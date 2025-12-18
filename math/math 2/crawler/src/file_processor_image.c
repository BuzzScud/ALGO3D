/**
 * Image File Processor
 * 
 * Extracts text from images using OCR (tesseract)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Extract text from image file using OCR
 * 
 * Uses tesseract command
 * Returns 0 on success, -1 on failure
 */
int process_image_file(const char* input_path, const char* output_path) {
    // Check if tesseract is available
    if (system("which tesseract > /dev/null 2>&1") != 0) {
        fprintf(stderr, "tesseract not found. Install tesseract-ocr.\n");
        return -1;
    }
    
    // Create temporary file for tesseract output (without extension)
    char temp_base[2048];
    snprintf(temp_base, sizeof(temp_base), "%s.tmp", output_path);
    
    // Build tesseract command
    // tesseract adds .txt automatically
    char command[4096];
    snprintf(command, sizeof(command), 
             "tesseract '%s' '%s' 2>/dev/null",
             input_path, temp_base);
    
    // Execute tesseract
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "tesseract failed for: %s\n", input_path);
        return -1;
    }
    
    // Tesseract adds .txt extension
    char temp_txt[2048];
    // Truncate temp_base to prevent buffer overflow (max 2043 chars for ".txt")
    size_t base_len = strlen(temp_base);
    if (base_len > 2043) {
        char truncated[2044];
        strncpy(truncated, temp_base, 2043);
        truncated[2043] = '\0';
        snprintf(temp_txt, sizeof(temp_txt), "%s.txt", truncated);
    } else {
        // Truncate temp_base to prevent buffer overflow (max 2043 chars for ".txt")
    size_t base_len = strlen(temp_base);
    if (base_len > 2043) {
        char truncated[2044];
        strncpy(truncated, temp_base, 2043);
        truncated[2043] = '\0';
        snprintf(temp_txt, sizeof(temp_txt), "%s.txt", truncated);
    } else {
        // Truncate temp_base to prevent buffer overflow (max 2043 chars for ".txt")
    size_t base_len = strlen(temp_base);
    if (base_len > 2043) {
        char truncated[2044];
        strncpy(truncated, temp_base, 2043);
        truncated[2043] = '\0';
        snprintf(temp_txt, sizeof(temp_txt), "%s.txt", truncated);
    } else {
        // Truncate temp_base to prevent buffer overflow (max 2043 chars for ".txt")
    size_t base_len = strlen(temp_base);
    if (base_len > 2043) {
        char truncated[2044];
        strncpy(truncated, temp_base, 2043);
        truncated[2043] = '\0';
        snprintf(temp_txt, sizeof(temp_txt), "%s.txt", truncated);
    } else {
        snprintf(temp_txt, sizeof(temp_txt), "%s.txt", temp_base);
    }
    }
    }
    }
    
    // Check if output file was created
    FILE* f = fopen(temp_txt, "r");
    if (!f) {
        fprintf(stderr, "No output from tesseract: %s\n", input_path);
        return -1;
    }
    
    // Check file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    if (size < 5) {
        fprintf(stderr, "OCR produced too little text: %ld bytes\n", size);
        unlink(temp_txt);
        return -1;
    }
    
    // Move temp file to final location
    rename(temp_txt, output_path);
    
    printf("  ✓ Extracted %ld bytes via OCR\n", size);
    return 0;
}
