/**
 * CLLM OCR Tool
 * 
 * Extracts text from images using Optical Character Recognition.
 */

#include "cllm_ocr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <image_file>\n\n", program_name);
    printf("Extract text from images using OCR.\n\n");
    printf("Options:\n");
    printf("  -o, --output FILE     Write output to FILE (default: stdout)\n");
    printf("  -l, --lang LANG       OCR language (default: eng)\n");
    printf("  -d, --dpi DPI         Image DPI (default: 300)\n");
    printf("  -p, --psm MODE        Page segmentation mode (default: 3)\n");
    printf("  -n, --no-preprocess   Disable image preprocessing\n");
    printf("  -L, --list-langs      List available languages\n");
    printf("  -c, --check           Check if OCR is available\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Page Segmentation Modes:\n");
    printf("  0 = Orientation and script detection (OSD) only\n");
    printf("  1 = Automatic page segmentation with OSD\n");
    printf("  3 = Fully automatic page segmentation (default)\n");
    printf("  4 = Assume a single column of text\n");
    printf("  6 = Assume a single uniform block of text\n");
    printf("  11 = Sparse text. Find as much text as possible\n\n");
    printf("Examples:\n");
    printf("  %s image.png\n", program_name);
    printf("  %s -l fra document.jpg -o output.txt\n", program_name);
    printf("  %s -d 600 --psm 6 scan.tiff\n", program_name);
    printf("  %s --list-langs\n", program_name);
}

int main(int argc, char* argv[]) {
    const char* image_path = NULL;
    const char* output_path = NULL;
    OCRConfig config = cllm_ocr_default_config();
    bool list_langs = false;
    bool check_available = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"lang", required_argument, 0, 'l'},
        {"dpi", required_argument, 0, 'd'},
        {"psm", required_argument, 0, 'p'},
        {"no-preprocess", no_argument, 0, 'n'},
        {"list-langs", no_argument, 0, 'L'},
        {"check", no_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:l:d:p:nLch", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 'l':
                strncpy(config.language, optarg, sizeof(config.language) - 1);
                break;
            case 'd':
                config.dpi = atoi(optarg);
                break;
            case 'p':
                config.psm = atoi(optarg);
                break;
            case 'n':
                config.preprocess = false;
                break;
            case 'L':
                list_langs = true;
                break;
            case 'c':
                check_available = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Check if OCR is available
    if (check_available) {
        if (cllm_ocr_is_available()) {
            printf("✓ Tesseract OCR is available\n");
            return 0;
        } else {
            printf("✗ Tesseract OCR is not available\n");
            return 1;
        }
    }

    // List available languages
    if (list_langs) {
        char languages[1024];
        if (cllm_ocr_get_languages(languages, sizeof(languages)) == 0) {
            printf("Available OCR languages:\n");
            
            // Split by comma and print each language
            char* lang = strtok(languages, ",");
            while (lang) {
                printf("  %s\n", lang);
                lang = strtok(NULL, ",");
            }
        } else {
            fprintf(stderr, "Error: Failed to get language list\n");
            return 1;
        }
        return 0;
    }

    // Get image file path
    if (optind >= argc) {
        fprintf(stderr, "Error: Image file path required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    image_path = argv[optind];

    // Check if OCR is available
    if (!cllm_ocr_is_available()) {
        fprintf(stderr, "Error: Tesseract OCR is not available\n");
        fprintf(stderr, "Please install tesseract-ocr package\n");
        return 1;
    }

    // Detect image format
    ImageFormat format = cllm_ocr_detect_format(image_path);
    if (format == IMAGE_FORMAT_UNKNOWN) {
        fprintf(stderr, "Warning: Unknown image format, attempting OCR anyway...\n");
    }

    // Perform OCR
    OCRResult result = cllm_ocr_extract_text(image_path, &config);

    if (result.error_code != 0) {
        fprintf(stderr, "Error: %s\n", result.error_message);
        return 1;
    }

    // Write output
    FILE* output_file = stdout;
    if (output_path) {
        output_file = fopen(output_path, "w");
        if (!output_file) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            cllm_ocr_free_result(&result);
            return 1;
        }
    }

    if (result.text) {
        fprintf(output_file, "%s", result.text);
    }

    if (output_file != stdout) {
        fclose(output_file);
        printf("Text extracted to: %s\n", output_path);
        printf("Confidence: %.1f%%\n", result.confidence * 100);
    }

    cllm_ocr_free_result(&result);
    return 0;
}