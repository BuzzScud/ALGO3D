#ifndef CLLM_OCR_H
#define CLLM_OCR_H

#include <stddef.h>
#include <stdbool.h>

/**
 * OCR (Optical Character Recognition) Module
 * 
 * Provides functionality for extracting text from images using Tesseract OCR.
 * Supports multiple image formats and languages.
 */

// Supported image formats
typedef enum {
    IMAGE_FORMAT_UNKNOWN = 0,
    IMAGE_FORMAT_PNG,
    IMAGE_FORMAT_JPG,
    IMAGE_FORMAT_JPEG,
    IMAGE_FORMAT_TIFF,
    IMAGE_FORMAT_BMP,
    IMAGE_FORMAT_GIF
} ImageFormat;

// OCR configuration
typedef struct {
    char language[64];       // Language code (e.g., "eng", "fra", "deu")
    int dpi;                 // DPI for image processing (default: 300)
    bool preprocess;         // Apply image preprocessing
    int psm;                 // Page segmentation mode (default: 3)
    char tesseract_data[256]; // Path to tessdata directory (optional)
} OCRConfig;

// OCR result
typedef struct {
    char* text;              // Extracted text (caller must free)
    size_t text_length;      // Length of extracted text
    float confidence;        // OCR confidence score (0.0 - 1.0)
    int error_code;          // 0 = success, non-zero = error
    char error_message[256]; // Error description if any
} OCRResult;

/**
 * Initialize OCR with default configuration
 * 
 * @return Default OCRConfig structure
 */
OCRConfig cllm_ocr_default_config(void);

/**
 * Extract text from an image file using OCR
 * 
 * @param image_path Path to the image file
 * @param config OCR configuration (NULL for defaults)
 * @return OCRResult structure (caller must free result.text)
 */
OCRResult cllm_ocr_extract_text(const char* image_path, const OCRConfig* config);

/**
 * Extract text from image data in memory
 * 
 * @param image_data Pointer to image data
 * @param data_size Size of image data in bytes
 * @param format Image format
 * @param config OCR configuration (NULL for defaults)
 * @return OCRResult structure (caller must free result.text)
 */
OCRResult cllm_ocr_extract_from_memory(const unsigned char* image_data, 
                                        size_t data_size,
                                        ImageFormat format,
                                        const OCRConfig* config);

/**
 * Detect image format from file
 * 
 * @param image_path Path to the image file
 * @return ImageFormat enum value
 */
ImageFormat cllm_ocr_detect_format(const char* image_path);

/**
 * Check if Tesseract is available and properly configured
 * 
 * @return true if Tesseract is available, false otherwise
 */
bool cllm_ocr_is_available(void);

/**
 * Get list of available languages
 * 
 * @param languages Buffer to store language codes (comma-separated)
 * @param buffer_size Size of the buffer
 * @return 0 on success, -1 on error
 */
int cllm_ocr_get_languages(char* languages, size_t buffer_size);

/**
 * Preprocess image for better OCR results
 * 
 * @param input_path Path to input image
 * @param output_path Path to save preprocessed image
 * @return 0 on success, -1 on error
 */
int cllm_ocr_preprocess_image(const char* input_path, const char* output_path);

/**
 * Free OCR result
 * 
 * @param result Pointer to OCRResult to free
 */
void cllm_ocr_free_result(OCRResult* result);

#endif // CLLM_OCR_H