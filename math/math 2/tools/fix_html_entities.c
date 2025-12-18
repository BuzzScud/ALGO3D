#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// HTML entity replacement function
void decode_html_entities(const char* input, char* output) {
    const char* p = input;
    char* q = output;
    
    while (*p) {
        if (strncmp(p, "&amp;", 5) == 0) {
            *q++ = '&';
            p += 5;
        } else if (strncmp(p, "&lt;", 4) == 0) {
            *q++ = '<';
            p += 4;
        } else if (strncmp(p, "&gt;", 4) == 0) {
            *q++ = '>';
            p += 4;
        } else if (strncmp(p, "&quot;", 6) == 0) {
            *q++ = '"';
            p += 6;
        } else if (strncmp(p, "&#39;", 5) == 0) {
            *q++ = '\'';
            p += 5;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    
    const char* filename = argv[1];
    
    // Read entire file
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* content = malloc(size + 1);
    if (!content) {
        fclose(f);
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);
    
    // Decode entities
    char* decoded = malloc(size + 1);
    if (!decoded) {
        free(content);
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    
    decode_html_entities(content, decoded);
    
    // Write back
    f = fopen(filename, "w");
    if (!f) {
        free(content);
        free(decoded);
        perror("fopen");
        return 1;
    }
    
    fwrite(decoded, 1, strlen(decoded), f);
    fclose(f);
    
    free(content);
    free(decoded);
    
    printf("Fixed HTML entities in %s\n", filename);
    return 0;
}