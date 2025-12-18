/**
 * OpenDocument Format Extractors (ODT, ODS, ODP)
 * 
 * Pure C implementation using ZIP extraction and XML parsing.
 * ODF files are ZIP archives containing content.xml
 */

#include "../docproc.h"
#include "../utils/zip_utils.h"
#include "../utils/xml_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TEMP_BUFFER_SIZE (10 * 1024 * 1024)  // 10MB temp buffer

/**
 * Extract text from ODT (OpenDocument Text)
 * ODT structure: content.xml contains the main text
 */
DocProcStatus docproc_extract_odt(const char* filepath, char* text, size_t text_size) {
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Extract content.xml from ODT (which is a ZIP file)
    int bytes_read = zip_extract_file(filepath, "content.xml",
                                     xml_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read < 0) {
        free(xml_buffer);
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    // Extract text from XML
    // ODT uses <text:p> for paragraphs and <text:span> for text spans
    char* out = text;
    char* out_end = text + text_size - 1;
    
    // Extract paragraphs
    xml_extract_elements(xml_buffer, "text:p", out, out_end - out);
    
    free(xml_buffer);
    
    if (strlen(text) == 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from ODS (OpenDocument Spreadsheet)
 * ODS structure: content.xml contains spreadsheet data
 */
DocProcStatus docproc_extract_ods(const char* filepath, char* text, size_t text_size) {
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Extract content.xml from ODS
    int bytes_read = zip_extract_file(filepath, "content.xml",
                                     xml_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read < 0) {
        free(xml_buffer);
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    // Extract text from cells
    // ODS uses <text:p> within <table:table-cell> for cell content
    xml_extract_elements(xml_buffer, "text:p", text, text_size);
    
    free(xml_buffer);
    
    if (strlen(text) == 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}

/**
 * Extract text from ODP (OpenDocument Presentation)
 * ODP structure: content.xml contains presentation slides
 */
DocProcStatus docproc_extract_odp(const char* filepath, char* text, size_t text_size) {
    char* xml_buffer = (char*)malloc(TEMP_BUFFER_SIZE);
    if (!xml_buffer) {
        return DOCPROC_ERROR_OUT_OF_MEMORY;
    }
    
    // Extract content.xml from ODP
    int bytes_read = zip_extract_file(filepath, "content.xml",
                                     xml_buffer, TEMP_BUFFER_SIZE);
    
    if (bytes_read < 0) {
        free(xml_buffer);
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    // Extract text from slides
    // ODP uses <text:p> for text content in slides
    xml_extract_elements(xml_buffer, "text:p", text, text_size);
    
    free(xml_buffer);
    
    if (strlen(text) == 0) {
        return DOCPROC_ERROR_EXTRACTION_FAILED;
    }
    
    return DOCPROC_SUCCESS;
}