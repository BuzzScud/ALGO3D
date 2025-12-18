/**
 * Office Document Processor
 * 
 * Extracts text from Office documents (DOCX, XLSX, PPTX, DOC, XLS, PPT)
 * Uses external tools: antiword (DOC), catdoc (XLS), unzip + grep (DOCX/XLSX/PPTX)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Process DOCX file (ZIP + XML)
 */
static int process_docx(const char* input_path, const char* output_path) {
    // DOCX is a ZIP file containing XML
    // Main content is in word/document.xml
    
    char temp_dir[2048];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/docx_%d", getpid());
    
    // Extract ZIP
    char unzip_cmd[4096];
    snprintf(unzip_cmd, sizeof(unzip_cmd),
             "unzip -q -o '%s' -d '%s' 2>/dev/null",
             input_path, temp_dir);
    
    if (system(unzip_cmd) != 0) {
        fprintf(stderr, "Failed to extract DOCX: %s\n", input_path);
        return -1;
    }
    
    // Extract text from document.xml
    char xml_path[2048];
    // Truncate temp_dir to prevent buffer overflow (max 2029 chars for "/word/document.xml")
    size_t temp_len = strlen(temp_dir);
    if (temp_len > 2029) {
        char truncated[2030];
        strncpy(truncated, temp_dir, 2029);
        truncated[2029] = '\0';
        snprintf(xml_path, sizeof(xml_path), "%s/word/document.xml", truncated);
    } else {
        snprintf(xml_path, sizeof(xml_path), "%s/word/document.xml", temp_dir);
    }
    
    // Use grep to extract text between <w:t> tags
    char extract_cmd[4096];
    snprintf(extract_cmd, sizeof(extract_cmd),
             "grep -oP '(?<=<w:t>)[^<]+' '%s' > '%s' 2>/dev/null",
             xml_path, output_path);
    
    int result = system(extract_cmd);
    
    // Cleanup temp directory
    char cleanup_cmd[4096];  // Increased to accommodate long paths
    // Truncate temp_dir to prevent buffer overflow (max 4080 chars for "rm -rf ''")
    size_t cleanup_len = strlen(temp_dir);
    if (cleanup_len > 4080) {
        char truncated[4081];
        strncpy(truncated, temp_dir, 4080);
        truncated[4080] = '\0';
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf '%s'", truncated);
    } else {
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf '%s'", temp_dir);
    }
    int cleanup_ret = system(cleanup_cmd);
    (void)cleanup_ret; // Suppress unused result warning
    
    if (result != 0) {
        fprintf(stderr, "Failed to extract text from DOCX\n");
        return -1;
    }
    
    // Check output size
    FILE* f = fopen(output_path, "r");
    if (!f) return -1;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    if (size < 10) {
        fprintf(stderr, "DOCX extraction produced too little text\n");
        unlink(output_path);
        return -1;
    }
    
    printf("  ✓ Extracted %ld bytes from DOCX\n", size);
    return 0;
}

/**
 * Process DOC file (legacy Word)
 */
static int process_doc(const char* input_path, const char* output_path) {
    // Check if antiword is available
    if (system("which antiword > /dev/null 2>&1") != 0) {
        fprintf(stderr, "antiword not found. Install antiword package.\n");
        return -1;
    }
    
    // Use antiword to extract text
    char command[4096];
    snprintf(command, sizeof(command),
             "antiword '%s' > '%s' 2>/dev/null",
             input_path, output_path);
    
    if (system(command) != 0) {
        fprintf(stderr, "antiword failed for: %s\n", input_path);
        return -1;
    }
    
    // Check output size
    FILE* f = fopen(output_path, "r");
    if (!f) return -1;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    if (size < 10) {
        fprintf(stderr, "DOC extraction produced too little text\n");
        unlink(output_path);
        return -1;
    }
    
    printf("  ✓ Extracted %ld bytes from DOC\n", size);
    return 0;
}

/**
 * Main office document processor
 * Detects format and routes to appropriate handler
 */
int process_office_file(const char* input_path, const char* output_path) {
    // Read first few bytes to detect format
    FILE* f = fopen(input_path, "rb");
    if (!f) return -1;
    
    unsigned char magic[8];
    size_t bytes_read = fread(magic, 1, 8, f);
    if (bytes_read != 8) {
        fprintf(stderr, "Warning: Could not read magic bytes from office file\n");
    }
    fclose(f);
    
    // Check for ZIP signature (DOCX, XLSX, PPTX)
    if (magic[0] == 0x50 && magic[1] == 0x4B && magic[2] == 0x03 && magic[3] == 0x04) {
        // Modern Office format (ZIP-based)
        // For now, assume DOCX (most common)
        return process_docx(input_path, output_path);
    }
    
    // Check for OLE signature (DOC, XLS, PPT)
    if (magic[0] == 0xD0 && magic[1] == 0xCF && magic[2] == 0x11 && magic[3] == 0xE0) {
        // Legacy Office format (OLE)
        // For now, assume DOC (most common)
        return process_doc(input_path, output_path);
    }
    
    fprintf(stderr, "Unknown Office document format\n");
    return -1;
}
