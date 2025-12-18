/**
 * Tokenizer
 * 
 * Converts preprocessed text to tokenized format for training
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_TOKEN_LENGTH 64
#define MAX_TOKENS 100000

typedef struct {
    char data_dir[1024];
    int running;
    int files_processed;
    pthread_mutex_t lock;
} TokenizerState;

/**
 * Simple word tokenization
 */
static int tokenize_text(const char* text, char** tokens, int max_tokens) {
    int count = 0;
    const char* p = text;
    char token[MAX_TOKEN_LENGTH];
    int token_len = 0;
    
    while (*p && count < max_tokens) {
        if (isalnum(*p) || *p == '\'' || *p == '-') {
            if (token_len < MAX_TOKEN_LENGTH - 1) {
                token[token_len++] = tolower(*p);
            }
            p++;
        } else {
            if (token_len > 0) {
                token[token_len] = '\0';
                tokens[count] = strdup(token);
                count++;
                token_len = 0;
            }
            p++;
        }
    }
    
    // Last token
    if (token_len > 0 && count < max_tokens) {
        token[token_len] = '\0';
        tokens[count] = strdup(token);
        count++;
    }
    
    return count;
}

/**
 * Process one text file
 */
static int tokenize_file(const char* input_path, const char* output_path) {
    // Read input
    FILE* f = fopen(input_path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open: %s\n", input_path);
        return -1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* text = (char*)malloc(size + 1);
    if (!text) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(text, 1, size, f);
    if (bytes_read != (size_t)size) {
        fprintf(stderr, "Warning: Only read %zu of %ld bytes from %s\n", 
                bytes_read, size, input_path);
    }
    text[size] = '\0';
    fclose(f);
    
    // Tokenize
    char** tokens = (char**)malloc(MAX_TOKENS * sizeof(char*));
    if (!tokens) {
        free(text);
        return -1;
    }
    
    int token_count = tokenize_text(text, tokens, MAX_TOKENS);
    free(text);
    
    if (token_count == 0) {
        free(tokens);
        return -1;
    }
    
    // Write output
    FILE* out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Failed to write: %s\n", output_path);
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        free(tokens);
        return -1;
    }
    
    fprintf(out, "# Source: %s\n", input_path);
    fprintf(out, "# Token count: %d\n", token_count);
    
    for (int i = 0; i < token_count; i++) {
        fprintf(out, "%s ", tokens[i]);
        free(tokens[i]);
    }
    fprintf(out, "\n");
    
    fclose(out);
    free(tokens);
    
    return token_count;
}

/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

/**
 * Tokenizer thread
 */
void* tokenizer_thread_func(void* arg) {
    TokenizerState* state = (TokenizerState*)arg;
    char timestamp[32];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === TOKENIZER STARTED ===\n", timestamp);
    
    char preprocessed_dir[2048];
    char queue_dir[2048];
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", state->data_dir);
    snprintf(queue_dir, sizeof(queue_dir), "%s/training_queue", state->data_dir);
    
    while (state->running) {
        DIR* dir = opendir(preprocessed_dir);
        if (!dir) {
            sleep(5);
            continue;
        }
        
        struct dirent* entry;
        int found_file = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strstr(entry->d_name, ".txt") == NULL) continue;
            
            // Check if already tokenized
            char output_path[4096];  // Increased from 2048 to accommodate long paths
            char* base = strdup(entry->d_name);
            char* dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            
            // Truncate base name to prevent buffer overflow (max 2000 chars for 4096 buffer)
            if (strlen(base) > 2000) {
                base[2000] = '\0';
            }
            
            snprintf(output_path, sizeof(output_path), 
                    "%s/%s.tok", queue_dir, base);
            
            // Check if output exists
            if (access(output_path, F_OK) == 0) {
                free(base);
                continue;
            }
            
            // Tokenize file
            char input_path[4096];  // Increased from 2048 to accommodate long paths
            // Truncate filename to prevent buffer overflow (max 2000 chars for 4096 buffer)
            char safe_name[2048];  // Increased from 256 to allow longer filenames
            strncpy(safe_name, entry->d_name, sizeof(safe_name) - 1);
            safe_name[sizeof(safe_name) - 1] = '\0';
            
            snprintf(input_path, sizeof(input_path), "%s/%s", preprocessed_dir, safe_name);
            
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s Tokenizing: %s\n", timestamp, entry->d_name);
            
            int token_count = tokenize_file(input_path, output_path);
            if (token_count > 0) {
                get_timestamp(timestamp, sizeof(timestamp));
                printf("%s ✓ Tokenized: %s (%d tokens)\n", timestamp, base, token_count);
                pthread_mutex_lock(&state->lock);
                state->files_processed++;
                pthread_mutex_unlock(&state->lock);
            }
            
            free(base);
            found_file = 1;
            break;  // Process one file at a time
        }
        
        closedir(dir);
        
        if (!found_file) {
            sleep(5);  // Wait for new files
        } else {
            sleep(1);  // Small delay between files
        }
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === TOKENIZER STOPPED ===\n", timestamp);
    return NULL;
}

/**
 * Initialize tokenizer
 */
TokenizerState* tokenizer_init(const char* data_dir) {
    TokenizerState* state = (TokenizerState*)calloc(1, sizeof(TokenizerState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    state->running = 1;
    state->files_processed = 0;
    pthread_mutex_init(&state->lock, NULL);
    
    return state;
}

/**
 * Cleanup tokenizer
 */
void tokenizer_cleanup(TokenizerState* state) {
    if (!state) return;
    pthread_mutex_destroy(&state->lock);
    free(state);
}