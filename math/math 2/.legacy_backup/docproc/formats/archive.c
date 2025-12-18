/**
 * Archive Extractor
 * 
 * Extracts and processes contents of archive files.
 */

#include "../docproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Extract text from archive
 */
DocProcStatus docproc_extract_archive(const char* filepath, char* text, size_t text_size) {
    // Create temp directory
    char temp_dir[1024];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/docproc_extract_%d", getpid());
    mkdir(temp_dir, 0755);
    
    // Extract based on type
    char cmd[2048];
    if (strstr(filepath, ".zip")) {
        snprintf(cmd, sizeof(cmd), "unzip -q '%s' -d '%s' 2>/dev/null", filepath, temp_dir);
    } else if (strstr(filepath, ".tar.gz") || strstr(filepath, ".tgz")) {
        snprintf(cmd, sizeof(cmd), "tar -xzf '%s' -C '%s' 2>/dev/null", filepath, temp_dir);
    } else if (strstr(filepath, ".tar")) {
        snprintf(cmd, sizeof(cmd), "tar -xf '%s' -C '%s' 2>/dev/null", filepath, temp_dir);
    } else {
        return DOCPROC_ERROR_UNSUPPORTED_FORMAT;
    }
    
    int status = system(cmd);
    if (status != 0) {
        snprintf(cmd, sizeof(cmd), "rm -rf '%s'", temp_dir);
        system(cmd);
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    // Process all text files in extracted directory
    snprintf(cmd, sizeof(cmd), 
            "find '%s' -type f \\( -name '*.txt' -o -name '*.md' -o -name '*.c' -o -name '*.h' -o -name '*.py' \\) -exec cat {} \\;",
            temp_dir);
    
    FILE* fp = popen(cmd, "r");
    if (fp) {
        size_t bytes_read = fread(text, 1, text_size - 1, fp);
        text[bytes_read] = '\0';
        pclose(fp);
    }
    
    // Cleanup
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", temp_dir);
    system(cmd);
    
    return DOCPROC_SUCCESS;
}