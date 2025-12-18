/**
 * CLLM Tokenizer Tool
 * 
 * Standalone tokenization tool for text processing.
 * Updated to use current CLLMTokenizer API.
 */

#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [text]\n\n", program_name);
    printf("Tokenize text using CLLM tokenizer.\n\n");
    printf("Options:\n");
    printf("  -f, --file FILE       Read text from file\n");
    printf("  -o, --output FILE     Write output to file (default: stdout)\n");
    printf("  -d, --decode          Decode token IDs to text\n");
    printf("  -s, --stats           Show tokenization statistics\n");
    printf("  -v, --vocab FILE      Load vocabulary file (required)\n");
    printf("  -j, --json            Output in JSON format\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -v vocab.txt &quot;Hello, world!&quot;\n", program_name);
    printf("  %s -v vocab.txt -f input.txt -o tokens.txt\n", program_name);
    printf("  %s -v vocab.txt -d &quot;42 123 456&quot;\n", program_name);
    printf("  %s -v vocab.txt -f input.txt -s -j\n", program_name);
}

static char* read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(size + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);

    return content;
}

static void tokenize_text(CLLMTokenizer* tokenizer, const char* text, 
                         bool show_stats, bool json_output, FILE* output) {
    if (!tokenizer || !text) return;

    // Tokenize using current API
    uint32_t token_count = 0;
    uint32_t* tokens = cllm_tokenizer_encode(tokenizer, text, &token_count);
    
    if (!tokens || token_count == 0) {
        fprintf(stderr, "Error: Tokenization failed\n");
        return;
    }

    // Calculate statistics
    uint32_t max_token = 0;
    uint32_t min_token = tokens[0];
    
    if (show_stats) {
        for (uint32_t i = 0; i < token_count; i++) {
            if (tokens[i] > max_token) max_token = tokens[i];
            if (tokens[i] < min_token) min_token = tokens[i];
        }
    }

    // Output
    if (json_output) {
        fprintf(output, "{\n");
        fprintf(output, "  &quot;text_length&quot;: %zu,\n", strlen(text));
        fprintf(output, "  &quot;token_count&quot;: %u,\n", token_count);
        
        if (show_stats) {
            fprintf(output, "  &quot;min_token_id&quot;: %u,\n", min_token);
            fprintf(output, "  &quot;max_token_id&quot;: %u,\n", max_token);
            fprintf(output, "  &quot;compression_ratio&quot;: %.2f,\n", 
                   (float)strlen(text) / token_count);
        }
        
        fprintf(output, "  &quot;tokens&quot;: [");
        for (uint32_t i = 0; i < token_count; i++) {
            fprintf(output, "%u", tokens[i]);
            if (i < token_count - 1) fprintf(output, ", ");
        }
        fprintf(output, "]\n");
        fprintf(output, "}\n");
    } else {
        // Plain text output
        if (show_stats) {
            fprintf(output, "Text length: %zu\n", strlen(text));
            fprintf(output, "Token count: %u\n", token_count);
            fprintf(output, "Min token ID: %u\n", min_token);
            fprintf(output, "Max token ID: %u\n", max_token);
            fprintf(output, "Compression ratio: %.2f\n", 
                   (float)strlen(text) / token_count);
            fprintf(output, "\nTokens:\n");
        }
        
        for (uint32_t i = 0; i < token_count; i++) {
            fprintf(output, "%u", tokens[i]);
            if (i < token_count - 1) fprintf(output, " ");
        }
        fprintf(output, "\n");
    }

    free(tokens);
}

static void decode_tokens(CLLMTokenizer* tokenizer, const char* token_str, 
                         bool json_output, FILE* output) {
    if (!tokenizer || !token_str) return;

    // Parse token IDs from string
    uint32_t* tokens = malloc(strlen(token_str) * sizeof(uint32_t));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }

    uint32_t token_count = 0;
    const char* ptr = token_str;
    while (*ptr) {
        if (*ptr >= '0' && *ptr <= '9') {
            tokens[token_count++] = (uint32_t)atoi(ptr);
            while (*ptr >= '0' && *ptr <= '9') ptr++;
        } else {
            ptr++;
        }
    }

    if (token_count == 0) {
        fprintf(stderr, "Error: No valid token IDs found\n");
        free(tokens);
        return;
    }

    // Decode using current API
    char* text = cllm_tokenizer_decode(tokenizer, tokens, token_count);
    
    if (!text) {
        fprintf(stderr, "Error: Decoding failed\n");
        free(tokens);
        return;
    }

    // Output
    if (json_output) {
        fprintf(output, "{\n");
        fprintf(output, "  &quot;token_count&quot;: %u,\n", token_count);
        fprintf(output, "  &quot;text&quot;: &quot;%s&quot;\n", text);
        fprintf(output, "}\n");
    } else {
        fprintf(output, "%s\n", text);
    }

    free(text);
    free(tokens);
}

int main(int argc, char** argv) {
    const char* input_file = NULL;
    const char* output_path = NULL;
    const char* vocab_file = NULL;
    const char* text = NULL;
    bool decode_mode = false;
    bool show_stats = false;
    bool json_output = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"decode", no_argument, 0, 'd'},
        {"stats", no_argument, 0, 's'},
        {"vocab", required_argument, 0, 'v'},
        {"json", no_argument, 0, 'j'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "f:o:dsv:jh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                input_file = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'd':
                decode_mode = true;
                break;
            case 's':
                show_stats = true;
                break;
            case 'v':
                vocab_file = optarg;
                break;
            case 'j':
                json_output = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Get text from command line if not from file
    if (!input_file && optind < argc) {
        text = argv[optind];
    }

    // Validate input
    if (!input_file && !text) {
        fprintf(stderr, "Error: Input text required (use -f or provide text)\n\n");
        print_usage(argv[0]);
        return 1;
    }

    if (!vocab_file) {
        fprintf(stderr, "Error: Vocabulary file required (use -v)\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Create tokenizer
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(50000);
    if (!tokenizer) {
        fprintf(stderr, "Error: Failed to create tokenizer\n");
        return 1;
    }

    // Load vocabulary
    if (cllm_load_vocab(tokenizer, vocab_file) == 0) {
        fprintf(stderr, "Error: Failed to load vocabulary from %s\n", vocab_file);
        cllm_free_tokenizer(tokenizer);
        return 1;
    }

    // Read input file if specified
    char* input_text = NULL;
    if (input_file) {
        input_text = read_file(input_file);
        if (!input_text) {
            fprintf(stderr, "Error: Failed to read input file: %s\n", input_file);
            cllm_free_tokenizer(tokenizer);
            return 1;
        }
        text = input_text;
    }

    // Open output file if specified
    FILE* output = stdout;
    if (output_path) {
        output = fopen(output_path, "w");
        if (!output) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            if (input_text) free(input_text);
            cllm_free_tokenizer(tokenizer);
            return 1;
        }
    }

    // Process
    if (decode_mode) {
        decode_tokens(tokenizer, text, json_output, output);
    } else {
        tokenize_text(tokenizer, text, show_stats, json_output, output);
    }

    // Cleanup
    if (input_text) free(input_text);
    if (output != stdout) fclose(output);
    cllm_free_tokenizer(tokenizer);

    return 0;
}
