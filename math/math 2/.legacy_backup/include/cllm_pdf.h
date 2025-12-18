#ifndef CLLM_PDF_H
#define CLLM_PDF_H

#include <stddef.h>
#include <stdbool.h>

/**
 * PDF Processing Module
 * 
 * Provides functionality for extracting text from PDF files,
 * including both text-based and image-based (scanned) PDFs.
 */

// PDF type detection
typedef enum {
    PDF_TYPE_UNKNOWN = 0,
    PDF_TYPE_TEXT,           // Contains extractable text
    PDF_TYPE_IMAGE,          // Contains only images (scanned)
    PDF_TYPE_MIXED           // Contains both text and images
} PDFType;

// PDF metadata
typedef struct {
    char title[256];
    char author[256];
    char subject[256];
    char creator[256];
    int page_count;
    PDFType type;
} PDFMetadata;

// PDF extraction result
typedef struct {
    char* text;              // Extracted text (caller must free)
    size_t text_length;      // Length of extracted text
    PDFMetadata metadata;    // PDF metadata
    int error_code;          // 0 = success, non-zero = error
    char error_message[256]; // Error description if any
} PDFExtractionResult;

/**
 * Extract text from a PDF file
 * 
 * @param pdf_path Path to the PDF file
 * @param use_ocr If true, apply OCR to image-based pages
 * @return PDFExtractionResult structure (caller must free result.text)
 */
PDFExtractionResult cllm_pdf_extract_text(const char* pdf_path, bool use_ocr);

/**
 * Detect PDF type (text, image, or mixed)
 * 
 * @param pdf_path Path to the PDF file
 * @return PDFType enum value
 */
PDFType cllm_pdf_detect_type(const char* pdf_path);

/**
 * Extract PDF metadata
 * 
 * @param pdf_path Path to the PDF file
 * @param metadata Pointer to PDFMetadata structure to fill
 * @return 0 on success, -1 on error
 */
int cllm_pdf_get_metadata(const char* pdf_path, PDFMetadata* metadata);

/**
 * Extract text from a specific page
 * 
 * @param pdf_path Path to the PDF file
 * @param page_num Page number (1-indexed)
 * @param use_ocr If true, apply OCR if page is image-based
 * @return Extracted text (caller must free), or NULL on error
 */
char* cllm_pdf_extract_page(const char* pdf_path, int page_num, bool use_ocr);

/**
 * Free PDF extraction result
 * 
 * @param result Pointer to PDFExtractionResult to free
 */
void cllm_pdf_free_result(PDFExtractionResult* result);

#endif // CLLM_PDF_H