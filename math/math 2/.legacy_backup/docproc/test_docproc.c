/**
 * Simple test program for libdocproc
 */

#include "docproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    printf("=== libdocproc Test Program ===\n\n");
    
    // Initialize library
    DocProcStatus status = docproc_init();
    if (status != DOCPROC_SUCCESS) {
        fprintf(stderr, "Failed to initialize libdocproc\n");
        return 1;
    }
    printf("✓ Library initialized\n");
    
    // Test format detection
    printf("\n--- Format Detection Tests ---\n");
    
    const char* test_files[] = {
        "document.docx",
        "spreadsheet.xlsx",
        "presentation.pptx",
        "document.odt",
        "book.epub",
        "config.yaml",
        "email.eml",
        "page.html",
        "data.json",
        "archive.zip"
    };
    
    for (int i = 0; i < 10; i++) {
        DocProcFormat format = docproc_detect_format(test_files[i], NULL);
        printf("  %s -> %s\n", test_files[i], docproc_format_name(format));
    }
    
    // Test supported extensions
    printf("\n--- Supported Extensions ---\n");
    const char* extensions[50];
    int count = docproc_get_supported_extensions(extensions, 50);
    printf("  Total: %d formats\n", count);
    printf("  First 10: ");
    for (int i = 0; i < 10 && i < count; i++) {
        printf("%s ", extensions[i]);
    }
    printf("\n");
    
    // Test format support check
    printf("\n--- Format Support Check ---\n");
    printf("  DOCX supported: %s\n", docproc_is_format_supported(DOCPROC_FORMAT_DOCX) ? "yes" : "no");
    printf("  XLSX supported: %s\n", docproc_is_format_supported(DOCPROC_FORMAT_XLSX) ? "yes" : "no");
    printf("  EPUB supported: %s\n", docproc_is_format_supported(DOCPROC_FORMAT_EPUB) ? "yes" : "no");
    printf("  YAML supported: %s\n", docproc_is_format_supported(DOCPROC_FORMAT_YAML) ? "yes" : "no");
    
    // Test with a real file if provided
    if (argc > 1) {
        printf("\n--- Extracting from: %s ---\n", argv[1]);
        
        DocProcResult result;
        DocProcOptions options;
        docproc_default_options(&options);
        
        status = docproc_extract(argv[1], &options, &result);
        
        if (status == DOCPROC_SUCCESS) {
            printf("✓ Extraction successful\n");
            printf("  Format: %s\n", docproc_format_name(result.format));
            printf("  Text length: %zu bytes\n", result.text_length);
            printf("  First 200 chars: %.200s\n", result.text);
            docproc_free_result(&result);
        } else {
            printf("✗ Extraction failed: %s\n", result.error_message);
        }
    }
    
    // Cleanup
    docproc_cleanup();
    printf("\n✓ Library cleaned up\n");
    
    printf("\n=== All tests passed ===\n");
    return 0;
}