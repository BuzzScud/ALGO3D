/**
 * Email Extractor (EML)
 * 
 * Simple email parser that extracts headers and body.
 */

#include "../docproc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Extract text from EML file
 */
DocProcStatus docproc_extract_eml(const char* filepath, char* text, size_t text_size) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        return DOCPROC_ERROR_FILE_NOT_FOUND;
    }
    
    char* out = text;
    char* out_end = text + text_size - 1;
    char line[4096];
    int in_headers = 1;
    int in_body = 0;
    
    // Parse email
    while (fgets(line, sizeof(line), fp) && out < out_end) {
        // Check for end of headers (empty line)
        if (in_headers && (line[0] == '\n' || line[0] == '\r')) {
            in_headers = 0;
            in_body = 1;
            
            // Add separator
            if (out + 2 < out_end) {
                *out++ = '\n';
                *out++ = '\n';
            }
            continue;
        }
        
        // Extract important headers
        if (in_headers) {
            if (strncasecmp(line, "From:", 5) == 0 ||
                strncasecmp(line, "To:", 3) == 0 ||
                strncasecmp(line, "Subject:", 8) == 0 ||
                strncasecmp(line, "Date:", 5) == 0) {
                
                size_t len = strlen(line);
                if (out + len < out_end) {
                    strcpy(out, line);
                    out += len;
                }
            }
            continue;
        }
        
        // Extract body
        if (in_body) {
            // Skip quoted text (lines starting with >)
            char* trimmed = line;
            while (*trimmed && isspace(*trimmed)) trimmed++;
            
            if (*trimmed == '>') continue;
            
            // Copy line
            size_t len = strlen(line);
            if (out + len < out_end) {
                strcpy(out, line);
                out += len;
            }
        }
    }
    
    *out = '\0';
    fclose(fp);
    
    if (out == text) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}