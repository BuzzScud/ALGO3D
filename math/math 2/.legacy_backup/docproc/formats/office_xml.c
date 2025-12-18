/**
 * Office Open XML Extractors (DOCX, XLSX, PPTX)
 * 
 * Pure C implementation using ZIP extraction and XML parsing.
 */

#include "../docproc.h"
#include "../utils/zip_utils.h"
#include "../utils/xml_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEMP_BUFFER_SIZE (10 * 1024 * 1024)  // 10MB temp buffer

/**
 * Extract text from DOCX
 * DOCX structure: word/document.xml contains the main text
 */
DocProcStatus docproc_extract_docx(const char* filepath, char* text, size_t text_size) {
    // Allocate temporary buffer for XML content
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Extract document.xml from DOCX (which is a ZIP file)
    int bytes_read = zip_extract_file(filepath, "word/document.xml", 
                                     xml_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read < 0) {
        free(xml_buffer);
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    // Extract text from XML
    // DOCX uses <w:t> elements for text content
    int result = xml_extract_elements(xml_buffer, "w:t", text, text_size);
    
    free(xml_buffer);
    
    if (result < 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from XLSX
 * XLSX structure: xl/sharedStrings.xml contains shared strings
 *                 xl/worksheets/sheet*.xml contain worksheet data
 */
DocProcStatus docproc_extract_xlsx(const char* filepath, char* text, size_t text_size) {
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    char* out = text;
    char* out_end = text + text_size - 1;
    
    // First, try to extract shared strings
    int bytes_read = zip_extract_file(filepath, "xl/sharedStrings.xml",
                                     xml_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read > 0) {
        // Extract text from shared strings
        size_t available = out_end - out;
        if (available > 0) {
            xml_extract_elements(xml_buffer, "t", out, available);
            out += strlen(out);
            
            if (out < out_end - 2) {
                *out++ = '\n';
                *out++ = '\n';
            }
        }
    }
    
    // Extract from worksheets (try sheet1.xml, sheet2.xml, etc.)
    for (int i = 1; i <= 10 && out < out_end; i++) {
        char sheet_path[256];
        snprintf(sheet_path, sizeof(sheet_path), "xl/worksheets/sheet%d.xml", i);
        
        bytes_read = zip_extract_file(filepath, sheet_path, 
                                     xml_buffer, TEMP_BUFFER_SIZE);
        
        if (bytes_read > 0) {
            size_t available = out_end - out;
            if (available > 0) {
                // Add sheet header
                int header_len = snprintf(out, available, "=== Sheet %d ===\n", i);
                if (header_len > 0 && header_len < (int)available) {
                    out += header_len;
                }
                
                // Extract cell values
                available = out_end - out;
                if (available > 0) {
                    xml_extract_elements(xml_buffer, "v", out, available);
                    out += strlen(out);
                    
                    if (out < out_end - 2) {
                        *out++ = '\n';
                        *out++ = '\n';
                    }
                }
            }
        } else {
            break;  // No more sheets
        }
    }
    
    *out = '\0';
    free(xml_buffer);
    
    if (out == text) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from PPTX
 * PPTX structure: ppt/slides/slide*.xml contain slide content
 */
DocProcStatus docproc_extract_pptx(const char* filepath, char* text, size_t text_size) {
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    char* out = text;
    char* out_end = text + text_size - 1;
    
    // Extract from slides (try slide1.xml, slide2.xml, etc.)
    for (int i = 1; i <= 100 && out < out_end; i++) {
        char slide_path[256];
        snprintf(slide_path, sizeof(slide_path), "ppt/slides/slide%d.xml", i);
        
        int bytes_read = zip_extract_file(filepath, slide_path,
                                         xml_buffer, TEMP_BUFFER_SIZE);
        
        if (bytes_read > 0) {
            size_t available = out_end - out;
            if (available > 0) {
                // Add slide header
                int header_len = snprintf(out, available, "\n=== Slide %d ===\n", i);
                if (header_len > 0 && header_len < (int)available) {
                    out += header_len;
                }
                
                // Extract text from slide
                // PPTX uses <a:t> elements for text content
                available = out_end - out;
                if (available > 0) {
                    xml_extract_elements(xml_buffer, "a:t", out, available);
                    out += strlen(out);
                    
                    if (out < out_end - 2) {
                        *out++ = '\n';
                        *out++ = '\n';
                    }
                }
            }
        } else {
            break;  // No more slides
        }
    }
    
    *out = '\0';
    free(xml_buffer);
    
    if (out == text) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}