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
 * Initialize OCR with default configuration
 */
OCRConfig cllm_ocr_default_config(void) {
    OCRConfig config;
    memset(&config, 0, sizeof(config));
    
    strcpy(config.language, "eng");
    config.dpi = 300;
    config.preprocess = true;
    config.psm = 3; // Fully automatic page segmentation
    config.tesseract_data[0] = '\0';
    
    return config;
}

/**
 * Check if Tesseract is available
 */
bool cllm_ocr_is_available(void) {
    int ret = system("which tesseract > /dev/null 2>&1");
    return ret == 0;
}

/**
 * Get list of available languages
 */
int cllm_ocr_get_languages(char* languages, size_t buffer_size) {
    if (!languages || buffer_size == 0) {
        return -1;
    }

    char* output = execute_command("tesseract --list-langs 2>&1 | tail -n +2", NULL);
    if (!output) {
        return -1;
    }

    // Convert newlines to commas
    size_t len = strlen(output);
    size_t out_pos = 0;
    
    for (size_t i = 0; i < len && out_pos < buffer_size - 1; i++) {
        if (output[i] == '\n') {
            if (out_pos > 0 && languages[out_pos - 1] != ',') {
                languages[out_pos++] = ',';
            }
        } else if (output[i] != ' ' && output[i] != '\t') {
            languages[out_pos++] = output[i];
        }
    }
    
    // Remove trailing comma
    if (out_pos > 0 && languages[out_pos - 1] == ',') {
        out_pos--;
    }
    
    languages[out_pos] = '\0';
    free(output);
    
    return 0;
}

/**
 * Detect image format from file
 */
ImageFormat cllm_ocr_detect_format(const char* image_path) {
    if (!image_path || !file_exists(image_path)) {
        return IMAGE_FORMAT_UNKNOWN;
    }

    // Use file command to detect format
    char command[2048];
    snprintf(command, sizeof(command), "file -b --mime-type '%s' 2>/dev/null", image_path);
    
    char* output = execute_command(command, NULL);
    if (!output) {
        return IMAGE_FORMAT_UNKNOWN;
    }

    ImageFormat format = IMAGE_FORMAT_UNKNOWN;
    
    if (strstr(output, "image/png")) {
        format = IMAGE_FORMAT_PNG;
    } else if (strstr(output, "image/jpeg") || strstr(output, "image/jpg")) {
        format = IMAGE_FORMAT_JPG;
    } else if (strstr(output, "image/tiff")) {
        format = IMAGE_FORMAT_TIFF;
    } else if (strstr(output, "image/bmp")) {
        format = IMAGE_FORMAT_BMP;
    } else if (strstr(output, "image/gif")) {
        format = IMAGE_FORMAT_GIF;
    }

    free(output);
    return format;
}

/**
 * Preprocess image for better OCR results
 */
int cllm_ocr_preprocess_image(const char* input_path, const char* output_path) {
    if (!input_path || !output_path || !file_exists(input_path)) {
        return -1;
    }

    // Use ImageMagick convert to preprocess
    // - Convert to grayscale
    // - Increase contrast
    // - Denoise
    // - Sharpen
    char command[4096];
    snprintf(command, sizeof(command),
             "convert '%s' -colorspace Gray -contrast-stretch 0 "
             "-normalize -despeckle -sharpen 0x1 '%s' 2>/dev/null",
             input_path, output_path);
    
    int ret = system(command);
    return (ret == 0) ? 0 : -1;
}

/**
 * Extract text from an image file using OCR
 */
OCRResult cllm_ocr_extract_text(const char* image_path, const OCRConfig* config) {
    OCRResult result;
    memset(&result, 0, sizeof(result));
    result.error_code = -1;

    if (!image_path) {
        snprintf(result.error_message, sizeof(result.error_message), "NULL path provided");
        return result;
    }

    if (!file_exists(image_path)) {
        snprintf(result.error_message, sizeof(result.error_message), "File not found: %s", image_path);
        return result;
    }

    if (!cllm_ocr_is_available()) {
        snprintf(result.error_message, sizeof(result.error_message), "Tesseract OCR not available");
        return result;
    }

    // Use default config if none provided
    OCRConfig default_config;
    if (!config) {
        default_config = cllm_ocr_default_config();
        config = &default_config;
    }

    // Preprocess image if requested
    char processed_image[512];
    const char* ocr_input = image_path;
    
    if (config->preprocess) {
        snprintf(processed_image, sizeof(processed_image), 
                 "/tmp/ocr_preprocessed_%d.png", getpid());
        
        if (cllm_ocr_preprocess_image(image_path, processed_image) == 0) {
            ocr_input = processed_image;
        }
    }

    // Build tesseract command
    char command[4096];
    char temp_output[256];
    snprintf(temp_output, sizeof(temp_output), "/tmp/ocr_output_%d", getpid());
    
    // Build command with options
    snprintf(command, sizeof(command),
             "tesseract '%s' '%s' -l %s --psm %d 2>/dev/null",
             ocr_input, temp_output, config->language, config->psm);
    
    // Add DPI if specified
    if (config->dpi > 0) {
        char dpi_opt[64];
        snprintf(dpi_opt, sizeof(dpi_opt), " --dpi %d", config->dpi);
        strcat(command, dpi_opt);
    }

    // Execute OCR
    int ret = system(command);
    
    // Clean up preprocessed image
    if (config->preprocess && ocr_input == processed_image) {
        unlink(processed_image);
    }

    if (ret != 0) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Tesseract execution failed");
        return result;
    }

    // Read output file
    char output_file[512];
    snprintf(output_file, sizeof(output_file), "%s.txt", temp_output);
    
    if (!file_exists(output_file)) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "OCR output file not created");
        return result;
    }

    FILE* f = fopen(output_file, "r");
    if (!f) {
        unlink(output_file);
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to read OCR output");
        return result;
    }

    // Read file content
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    result.text = malloc(file_size + 1);
    if (!result.text) {
        fclose(f);
        unlink(output_file);
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Memory allocation failed");
        return result;
    }

    size_t bytes_read = fread(result.text, 1, file_size, f);
    result.text[bytes_read] = '\0';
    result.text_length = bytes_read;
    
    fclose(f);
    unlink(output_file);

    // Set success
    result.error_code = 0;
    result.confidence = 0.85f; // Default confidence (Tesseract doesn't provide this easily)
    snprintf(result.error_message, sizeof(result.error_message), "Success");

    return result;
}

/**
 * Extract text from image data in memory
 */
OCRResult cllm_ocr_extract_from_memory(const unsigned char* image_data, 
                                        size_t data_size,
                                        ImageFormat format,
                                        const OCRConfig* config) {
    OCRResult result;
    memset(&result, 0, sizeof(result));
    result.error_code = -1;

    if (!image_data || data_size == 0) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Invalid image data");
        return result;
    }

    // Write to temporary file
    const char* ext = ".png";
    switch (format) {
        case IMAGE_FORMAT_JPG:
        case IMAGE_FORMAT_JPEG:
            ext = ".jpg";
            break;
        case IMAGE_FORMAT_TIFF:
            ext = ".tiff";
            break;
        case IMAGE_FORMAT_BMP:
            ext = ".bmp";
            break;
        case IMAGE_FORMAT_GIF:
            ext = ".gif";
            break;
        default:
            ext = ".png";
            break;
    }

    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "/tmp/ocr_input_%d%s", getpid(), ext);
    
    FILE* f = fopen(temp_file, "wb");
    if (!f) {
        snprintf(result.error_message, sizeof(result.error_message), 
                 "Failed to create temporary file");
        return result;
    }

    fwrite(image_data, 1, data_size, f);
    fclose(f);

    // Process the temporary file
    result = cllm_ocr_extract_text(temp_file, config);
    
    // Clean up
    unlink(temp_file);

    return result;
}

/**
 * Free OCR result
 */
void cllm_ocr_free_result(OCRResult* result) {
    if (result && result->text) {
        free(result->text);
        result->text = NULL;
        result->text_length = 0;
    }
}