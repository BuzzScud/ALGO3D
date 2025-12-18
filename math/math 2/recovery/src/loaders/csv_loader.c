/*
 * CSV Loader Implementation
 * Supports I/Q data, time series, and multi-channel signals
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Forward declarations
typedef struct {
    double* samples;
    size_t num_samples;
    size_t num_channels;
    double sample_rate;
    void* metadata;
} SignalData;

typedef enum {
    DATA_TYPE_SIGNAL,
    DATA_TYPE_UNKNOWN
} DataType;

typedef struct {
    union {
        SignalData signal;
    };
} GenericData;

/**
 * Count columns in CSV header
 */
static int count_csv_columns(const char* line) {
    int count = 1;
    for (const char* p = line; *p; p++) {
        if (*p == ',') count++;
    }
    return count;
}

/**
 * Parse CSV line into values
 */
static int parse_csv_line(const char* line, double* values, int max_values) {
    int count = 0;
    char* line_copy = strdup(line);
    char* token = strtok(line_copy, ",");
    
    while (token && count < max_values) {
        // Skip whitespace
        while (isspace(*token)) token++;
        
        // Parse value
        values[count++] = atof(token);
        token = strtok(NULL, ",");
    }
    
    free(line_copy);
    return count;
}

/**
 * Load CSV signal data
 */
bool load_csv_signal_data(FILE* file, SignalData* data) {
    if (!file || !data) return false;
    
    char line[4096];
    int num_channels = 0;
    size_t num_samples = 0;
    size_t capacity = 1000;
    
    // Allocate initial buffer
    double* buffer = (double*)malloc(capacity * 10 * sizeof(double));  // Max 10 channels
    if (!buffer) return false;
    
    // Read header line
    if (fgets(line, sizeof(line), file)) {
        // Check if it's a header (contains letters)
        bool is_header = false;
        for (char* p = line; *p; p++) {
            if (isalpha(*p)) {
                is_header = true;
                break;
            }
        }
        
        if (is_header) {
            num_channels = count_csv_columns(line);
        } else {
            // No header, parse first line as data
            rewind(file);
        }
    }
    
    // Read data lines
    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') continue;
        
        // Expand buffer if needed
        if (num_samples >= capacity) {
            capacity *= 2;
            double* new_buffer = (double*)realloc(buffer, capacity * 10 * sizeof(double));
            if (!new_buffer) {
                free(buffer);
                return false;
            }
            buffer = new_buffer;
        }
        
        // Parse line
        double values[10];
        int count = parse_csv_line(line, values, 10);
        
        if (num_channels == 0) {
            num_channels = count;
        }
        
        // Store values
        for (int i = 0; i < count && i < num_channels; i++) {
            buffer[num_samples * num_channels + i] = values[i];
        }
        
        num_samples++;
    }
    
    // Allocate final buffer
    data->samples = (double*)malloc(num_samples * num_channels * sizeof(double));
    if (!data->samples) {
        free(buffer);
        return false;
    }
    
    memcpy(data->samples, buffer, num_samples * num_channels * sizeof(double));
    free(buffer);
    
    data->num_samples = num_samples;
    data->num_channels = num_channels;
    data->sample_rate = 48000.0;  // Default, can be overridden
    data->metadata = NULL;
    
    return true;
}

/**
 * Load I/Q CSV data (complex samples)
 */
bool load_iq_csv_data(FILE* file, SignalData* data) {
    if (!file || !data) return false;
    
    // I/Q data has 2 columns: I (in-phase) and Q (quadrature)
    if (!load_csv_signal_data(file, data)) {
        return false;
    }
    
    // Verify we have 2 channels
    if (data->num_channels != 2) {
        fprintf(stderr, "I/Q data must have exactly 2 columns (I and Q)\n");
        free(data->samples);
        return false;
    }
    
    return true;
}

/**
 * Save signal data as CSV
 */
bool save_csv_signal_data(FILE* file, SignalData* data) {
    if (!file || !data || !data->samples) return false;
    
    // Write header
    if (data->num_channels == 2) {
        fprintf(file, "I,Q\n");
    } else {
        for (size_t i = 0; i < data->num_channels; i++) {
            fprintf(file, "Channel%zu%s", i, (i < data->num_channels - 1) ? "," : "\n");
        }
    }
    
    // Write data
    for (size_t i = 0; i < data->num_samples; i++) {
        for (size_t j = 0; j < data->num_channels; j++) {
            fprintf(file, "%.6f%s", 
                   data->samples[i * data->num_channels + j],
                   (j < data->num_channels - 1) ? "," : "\n");
        }
    }
    
    fflush(file);
    return true;
}

/**
 * Load generic CSV data
 */
bool load_csv_generic(FILE* file, GenericData* data, DataType* type) {
    if (!file || !data || !type) return false;
    
    *type = DATA_TYPE_SIGNAL;
    return load_csv_signal_data(file, &data->signal);
}
