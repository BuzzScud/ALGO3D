#include "cllm_pdf.h"
#include "cllm_ocr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Check if a file exists
 */
static bool file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * Execute a command and capture output
 */
static char* execute_command(const char* command, size_t* output_size) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return NULL;
    }

    // Read output
    size_t buffer_size = 4096;
    size_t total_size = 0;
    char* output = malloc(buffer_size);
    if (!output) {
        pclose(pipe);
        return NULL;
    }

    size_t bytes_read;
    while ((bytes_read = fread(output + total_size, 1, buffer_size - total_size - 1, pipe)) > 0) {
        total_size += bytes_read;
        if (total_size + 1024 >= buffer_size) {
            buffer_size *= 2;
            char* new_output = realloc(output, buffer_size);
            if (!new_output) {
                free(output);
                pclose(pipe);
                return NULL;
            }
            output = new_output;
        }
    }

    output[total_size] = '\0';
    pclose(pipe);

    if (output_size) {
        *output_size = total_size;
    }

    return output;
}

/**
 * Detect PDF type (text, image, or mixed)
 */
PDFType cllm_pdf_detect_type(const char* pdf_path) {
    if (!pdf_path || !file_exists(pdf_path)) {
        return PDF_TYPE_UNKNOWN;
    }

    // Try to extract text
    char command[2048];
    snprintf(command, sizeof(command), "pdftotext '%s' - 2>/dev/null | wc -c", pdf_path);
    
    size_t text_size = 0;
    char* output = execute_command(command, &text_size);
    if (!output) {
        return PDF_TYPE_UNKNOWN;
    }

    int char_count = atoi(output);
    free(output);

    // If we got significant text, it's a text PDF
    if (char_count > 100) {
        return PDF_TYPE_TEXT;
    }

    // Check if it has images
    snprintf(command, sizeof(command), "pdfimages -list '%s' 2>/dev/null | wc -l", pdf_path);
    output = execute_command(command, NULL);
    if (!output) {
        return PDF_TYPE_UNKNOWN;
    }

    int image_count = atoi(output);
    free(output);

    if (image_count > 2) { // More than header lines
        if (char_count > 10) {
            return PDF_TYPE_MIXED;
        }
        return PDF_TYPE_IMAGE;
    }

    return PDF_TYPE_UNKNOWN;
}

/**
 * Extract PDF metadata
 */
int cllm_pdf_get_metadata(const char* pdf_path, PDFMetadata* metadata) {
    if (!pdf_path || !metadata || !file_exists(pdf_path)) {
        return -1;
    }

    memset(metadata, 0, sizeof(PDFMetadata));

    // Get page count
    char command[2048];
    snprintf(command, sizeof(command), "pdfinfo '%s' 2>/dev/null | grep 'Pages:' | awk '{print $2}'", pdf_path);
    char* output = execute_command(command, NULL);
    if (output) {
        metadata->page_count = atoi(output);
        free(output);
    }

    // Get title
    snprintf(command, sizeof(command), "pdfinfo '%s' 2>/dev/null | grep 'Title:' | cut -d':' -f2- | sed 's/^[[:space:]]*//'", pdf_path);
    output = execute_command(command, NULL);
    if (output) {
        strncpy(metadata->title, output, sizeof(metadata->title) - 1);
        // Remove trailing newline
        size_t len = strlen(metadata->title);
        if (len > 0 && metadata->title[len - 1] == '\n') {
            metadata->title[len - 1] = '\0';
        }
        free(output);
    }

    // Get author
    snprintf(command, sizeof(command), "pdfinfo '%s' 2>/dev/null | grep 'Author:' | cut -d':' -f2- | sed 's/^[[:space:]]*//'", pdf_path);
    output = execute_command(command, NULL);
    if (output) {
        strncpy(metadata->author, output, sizeof(metadata->author) - 1);
        size_t len = strlen(metadata->author);
        if (len > 0 && metadata->author[len - 1] == '\n') {
            metadata->author[len - 1] = '\0';
        }
        free(output);
    }

    // Detect type
    metadata->type = cllm_pdf_detect_type(pdf_path);

    return 0;
}

/**
 * Extract text from a specific page
 */
char* cllm_pdf_extract_page(const char* pdf_path, int page_num, bool use_ocr) {
    if (!pdf_path || !file_exists(pdf_path) || page_num < 1) {
        return NULL;
    }

    // First try text extraction
    char command[2048];
    snprintf(command, sizeof(command), 
             "pdftotext -f %d -l %d '%s' - 2>/dev/null", 
             page_num, page_num, pdf_path);
    
    size_t text_size = 0;
    char* text = execute_command(command, &text_size);
    
    // If we got significant text, return it
    if (text && text_size > 50) {
        return text;
    }
    
    if (text) {
        free(text);
    }

    // If OCR is requested and text extraction failed, try OCR
    if (use_ocr) {
        // Extract page as image
        char temp_image[256];
        snprintf(temp_image, sizeof(temp_image), "/tmp/pdf_page_%d_%d.png", getpid(), page_num);
        
        snprintf(command, sizeof(command),
                 "pdftoppm -f %d -l %d -png '%s' > '%s' 2>/dev/null",
                 page_num, page_num, pdf_path, temp_image);
        
        int ret = system(command);
        if (ret == 0 && file_exists(temp_image)) {
            // Apply OCR
            OCRResult ocr_result = cllm_ocr_extract_text(temp_image, NULL);
            unlink(temp_image); // Clean up temp file
            
            if (ocr_result.error_code == 0 && ocr_result.text) {
                return ocr_result.text;
            }
            
            if (ocr_result.text) {
                free(ocr_result.text);
            }
        }
    }

    return NULL;
}

/**
 * Extract text from a PDF file
 */
PDFExtractionResult cllm_pdf_extract_text(const char* pdf_path, bool use_ocr) {
    PDFExtractionResult result;
    memset(&result, 0, sizeof(result));
    result.error_code = -1;

    if (!pdf_path) {
        snprintf(result.error_message, sizeof(result.error_message), "NULL path provided");
        return result;
    }

    if (!file_exists(pdf_path)) {
        snprintf(result.error_message, sizeof(result.error_message), "File not found: %s", pdf_path);
        return result;
    }

    // Get metadata
    if (cllm_pdf_get_metadata(pdf_path, &result.metadata) != 0) {
        snprintf(result.error_message, sizeof(result.error_message), "Failed to read PDF metadata");
        return result;
    }

    // Determine extraction strategy based on PDF type
    PDFType type = result.metadata.type;
    
    if (type == PDF_TYPE_TEXT || type == PDF_TYPE_MIXED) {
        // Try text extraction first
        char command[2048];
        snprintf(command, sizeof(command), "pdftotext -layout '%s' - 2>/dev/null", pdf_path);
        
        result.text = execute_command(command, &result.text_length);
        
        if (result.text && result.text_length > 100) {
            result.error_code = 0;
            snprintf(result.error_message, sizeof(result.error_message), "Success");
            return result;
        }
        
        if (result.text) {
            free(result.text);
            result.text = NULL;
            result.text_length = 0;
        }
    }

    // If text extraction failed or PDF is image-based, try OCR
    if (use_ocr && (type == PDF_TYPE_IMAGE || type == PDF_TYPE_MIXED || type == PDF_TYPE_UNKNOWN)) {
        // Extract all pages and apply OCR
        size_t total_size = 0;
        size_t buffer_size = 4096;
        char* combined_text = malloc(buffer_size);
        if (!combined_text) {
            snprintf(result.error_message, sizeof(result.error_message), "Memory allocation failed");
            return result;
        }
        combined_text[0] = '\0';

        for (int page = 1; page <= result.metadata.page_count; page++) {
            char* page_text = cllm_pdf_extract_page(pdf_path, page, true);
            if (page_text) {
                size_t page_len = strlen(page_text);
                
                // Resize buffer if needed
                while (total_size + page_len + 100 >= buffer_size) {
                    buffer_size *= 2;
                    char* new_buffer = realloc(combined_text, buffer_size);
                    if (!new_buffer) {
                        free(page_text);
                        free(combined_text);
                        snprintf(result.error_message, sizeof(result.error_message), 
                                "Memory allocation failed during OCR");
                        return result;
                    }
                    combined_text = new_buffer;
                }

                // Add page separator
                if (total_size > 0) {
                    strcat(combined_text, "\n\n--- Page ");
                    char page_num[16];
                    snprintf(page_num, sizeof(page_num), "%d", page);
                    strcat(combined_text, page_num);
                    strcat(combined_text, " ---\n\n");
                    total_size = strlen(combined_text);
                }

                strcat(combined_text, page_text);
                total_size += page_len;
                free(page_text);
            }
        }

        if (total_size > 0) {
            result.text = combined_text;
            result.text_length = total_size;
            result.error_code = 0;
            snprintf(result.error_message, sizeof(result.error_message), "Success (OCR)");
            return result;
        }

        free(combined_text);
    }

    snprintf(result.error_message, sizeof(result.error_message), 
             "Failed to extract text from PDF");
    return result;
}

/**
 * Free PDF extraction result
 */
void cllm_pdf_free_result(PDFExtractionResult* result) {
    if (result && result->text) {
        free(result->text);
        result->text = NULL;
        result->text_length = 0;
    }
}