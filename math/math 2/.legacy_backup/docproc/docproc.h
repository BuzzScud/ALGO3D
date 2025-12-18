#ifndef DOCPROC_H
#define DOCPROC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Document Processing Library (libdocproc)
 * 
 * Unified library for extracting text from various document formats.
 * Pure C implementation with minimal external dependencies.
 * 
 * Supported formats:
 * - Office: DOCX, XLSX, PPTX, DOC, RTF
 * - OpenDocument: ODT, ODS, ODP
 * - eBooks: EPUB
 * - Documents: PDF, TXT, MD, HTML, XML, LaTeX
 * - Data: JSON, CSV, YAML, TOML, SQL
 * - Code: C, C++, Python, JavaScript, Java, Go, Rust
 * - Email: EML
 * - Images: JPG, PNG, GIF, WEBP, BMP, TIFF (OCR)
 * - Archives: ZIP, TAR, TAR.GZ, etc.
 */

// ============================================================================
// TYPES AND ENUMERATIONS
// ============================================================================

/**
 * Document format types
 */
typedef enum {
    DOCPROC_FORMAT_HTML = 0,
    DOCPROC_FORMAT_PDF,
    DOCPROC_FORMAT_DOC,
    DOCPROC_FORMAT_DOCX,
    DOCPROC_FORMAT_XLSX,
    DOCPROC_FORMAT_PPTX,
    DOCPROC_FORMAT_ODT,
    DOCPROC_FORMAT_ODS,
    DOCPROC_FORMAT_ODP,
    DOCPROC_FORMAT_EPUB,
    DOCPROC_FORMAT_TXT,
    DOCPROC_FORMAT_JSON,
    DOCPROC_FORMAT_XML,
    DOCPROC_FORMAT_CSV,
    DOCPROC_FORMAT_CODE,
    DOCPROC_FORMAT_MARKDOWN,
    DOCPROC_FORMAT_YAML,
    DOCPROC_FORMAT_TOML,
    DOCPROC_FORMAT_SQL,
    DOCPROC_FORMAT_LATEX,
    DOCPROC_FORMAT_EML,
    DOCPROC_FORMAT_IMAGE,
    DOCPROC_FORMAT_ARCHIVE,
    DOCPROC_FORMAT_RTF,
    DOCPROC_FORMAT_UNKNOWN
} DocProcFormat;

/**
 * Extraction result status
 */
typedef enum {
    DOCPROC_SUCCESS = 0,
    DOCPROC_ERROR_FILE_NOT_FOUND = -1,
    DOCPROC_ERROR_UNSUPPORTED_FORMAT = -2,
    DOCPROC_ERROR_EXTRACTION_FAILED = -3,
    DOCPROC_ERROR_BUFFER_TOO_SMALL = -4,
    DOCPROC_ERROR_INVALID_PARAMETER = -5,
    DOCPROC_ERROR_OUT_OF_MEMORY = -6
} DocProcStatus;

/**
 * Document metadata
 */
typedef struct {
    char title[256];
    char author[256];
    char subject[256];
    char keywords[512];
    char creator[256];
    char producer[256];
    char creation_date[64];
    char modification_date[64];
    int page_count;
    size_t file_size;
} DocProcMetadata;

/**
 * Extraction options
 */
typedef struct {
    bool extract_metadata;      // Extract document metadata
    bool preserve_structure;    // Preserve headings, lists, tables
    bool extract_images;        // Extract images (OCR if enabled)
    bool use_ocr;              // Use OCR for images
    bool clean_text;           // Clean and normalize text
    size_t min_text_length;    // Minimum text length (skip if shorter)
    size_t max_text_size;      // Maximum text buffer size
} DocProcOptions;

/**
 * Extraction result
 */
typedef struct {
    DocProcStatus status;
    DocProcFormat format;
    char* text;
    size_t text_length;
    DocProcMetadata metadata;
    char error_message[256];
} DocProcResult;

// ============================================================================
// CORE API
// ============================================================================

/**
 * Initialize document processing library
 * Must be called before any other functions
 * 
 * @return DOCPROC_SUCCESS on success, error code otherwise
 */
DocProcStatus docproc_init(void);

/**
 * Cleanup document processing library
 * Should be called when done using the library
 */
void docproc_cleanup(void);

/**
 * Detect document format from file path and content
 * 
 * @param filepath Path to the file
 * @param content_type Optional content-type header (can be NULL)
 * @return Detected format
 */
DocProcFormat docproc_detect_format(const char* filepath, const char* content_type);

/**
 * Get format name as string
 * 
 * @param format Format enum value
 * @return Format name (e.g., "PDF", "DOCX")
 */
const char* docproc_format_name(DocProcFormat format);

/**
 * Extract text from document
 * 
 * @param filepath Path to the document
 * @param options Extraction options (NULL for defaults)
 * @param result Output result structure
 * @return DOCPROC_SUCCESS on success, error code otherwise
 */
DocProcStatus docproc_extract(const char* filepath, 
                              const DocProcOptions* options,
                              DocProcResult* result);

/**
 * Extract text from document with explicit format
 * 
 * @param filepath Path to the document
 * @param format Document format
 * @param options Extraction options (NULL for defaults)
 * @param result Output result structure
 * @return DOCPROC_SUCCESS on success, error code otherwise
 */
DocProcStatus docproc_extract_format(const char* filepath,
                                     DocProcFormat format,
                                     const DocProcOptions* options,
                                     DocProcResult* result);

/**
 * Free extraction result
 * 
 * @param result Result structure to free
 */
void docproc_free_result(DocProcResult* result);

/**
 * Get default extraction options
 * 
 * @param options Output options structure
 */
void docproc_default_options(DocProcOptions* options);

// ============================================================================
// FORMAT-SPECIFIC EXTRACTORS
// ============================================================================

/**
 * Extract text from PDF
 */
DocProcStatus docproc_extract_pdf(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from DOCX (Office Open XML)
 */
DocProcStatus docproc_extract_docx(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from XLSX (Excel)
 */
DocProcStatus docproc_extract_xlsx(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from PPTX (PowerPoint)
 */
DocProcStatus docproc_extract_pptx(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from ODT (OpenDocument Text)
 */
DocProcStatus docproc_extract_odt(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from ODS (OpenDocument Spreadsheet)
 */
DocProcStatus docproc_extract_ods(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from ODP (OpenDocument Presentation)
 */
DocProcStatus docproc_extract_odp(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from EPUB
 */
DocProcStatus docproc_extract_epub(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from HTML
 */
DocProcStatus docproc_extract_html(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from Markdown
 */
DocProcStatus docproc_extract_markdown(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from YAML
 */
DocProcStatus docproc_extract_yaml(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from TOML
 */
DocProcStatus docproc_extract_toml(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from JSON
 */
DocProcStatus docproc_extract_json(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from CSV
 */
DocProcStatus docproc_extract_csv(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from SQL
 */
DocProcStatus docproc_extract_sql(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from email (EML)
 */
DocProcStatus docproc_extract_eml(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from image using OCR
 */
DocProcStatus docproc_extract_image_ocr(const char* filepath, char* text, size_t text_size);

/**
 * Extract text from archive (processes contents)
 */
DocProcStatus docproc_extract_archive(const char* filepath, char* text, size_t text_size);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if format is supported
 * 
 * @param format Format to check
 * @return true if supported, false otherwise
 */
bool docproc_is_format_supported(DocProcFormat format);

/**
 * Get list of supported file extensions
 * 
 * @param extensions Output array of extension strings
 * @param max_count Maximum number of extensions to return
 * @return Number of extensions returned
 */
int docproc_get_supported_extensions(const char** extensions, int max_count);

/**
 * Check if file extension is supported
 * 
 * @param extension File extension (e.g., "pdf", "docx")
 * @return true if supported, false otherwise
 */
bool docproc_is_extension_supported(const char* extension);

/**
 * Get library version
 * 
 * @return Version string (e.g., "1.0.0")
 */
const char* docproc_version(void);

#endif // DOCPROC_H