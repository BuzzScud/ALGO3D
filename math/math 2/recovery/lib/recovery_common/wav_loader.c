/*
 * WAV File Loader Implementation
 * Implements WAV file loading for signal recovery
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../../include/recovery_common/recovery_common.h"

// WAV file header structures
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t file_size;     // File size - 8
    char wave[4];           // "WAVE"
} WAVHeader;

typedef struct {
    char fmt[4];            // "fmt "
    uint32_t chunk_size;    // Size of format chunk
    uint16_t audio_format;  // 1 = PCM
    uint16_t num_channels;  // Number of channels
    uint32_t sample_rate;   // Sample rate
    uint32_t byte_rate;     // Byte rate
    uint16_t block_align;   // Block align
    uint16_t bits_per_sample; // Bits per sample
} WAVFormat;

typedef struct {
    char data[4];           // "data"
    uint32_t data_size;     // Size of data chunk
} WAVData;

bool load_wav_file(FILE* file, SignalData* data) {
    if (!file || !data) {
        return false;
    }
    
    // Read WAV header
    WAVHeader header;
    if (fread(&header, sizeof(WAVHeader), 1, file) != 1) {
        return false;
    }
    
    // Verify RIFF/WAVE format
    if (memcmp(header.riff, "RIFF", 4) != 0 || 
        memcmp(header.wave, "WAVE", 4) != 0) {
        return false;
    }
    
    // Read format chunk
    WAVFormat format;
    if (fread(&format, sizeof(WAVFormat), 1, file) != 1) {
        return false;
    }
    
    // Verify format chunk
    if (memcmp(format.fmt, "fmt ", 4) != 0) {
        return false;
    }
    
    // Skip any extra format bytes
    if (format.chunk_size > 16) {
        fseek(file, format.chunk_size - 16, SEEK_CUR);
    }
    
    // Find data chunk
    WAVData data_chunk;
    while (1) {
        if (fread(&data_chunk, sizeof(WAVData), 1, file) != 1) {
            return false;
        }
        
        if (memcmp(data_chunk.data, "data", 4) == 0) {
            break;
        }
        
        // Skip this chunk
        fseek(file, data_chunk.data_size, SEEK_CUR);
    }
    
    // Calculate number of samples
    size_t bytes_per_sample = format.bits_per_sample / 8;
    size_t num_samples = data_chunk.data_size / (bytes_per_sample * format.num_channels);
    
    // Allocate sample buffer
    data->samples = (double*)malloc(num_samples * format.num_channels * sizeof(double));
    if (!data->samples) {
        return false;
    }
    
    data->num_samples = num_samples;
    data->num_channels = format.num_channels;
    data->sample_rate = format.sample_rate;
    data->metadata = NULL;
    
    // Read and convert samples
    if (format.bits_per_sample == 16) {
        // 16-bit PCM
        int16_t* buffer = (int16_t*)malloc(data_chunk.data_size);
        if (!buffer) {
            free(data->samples);
            return false;
        }
        
        if (fread(buffer, data_chunk.data_size, 1, file) != 1) {
            free(buffer);
            free(data->samples);
            return false;
        }
        
        // Convert to double [-1.0, 1.0]
        for (size_t i = 0; i < num_samples * format.num_channels; i++) {
            data->samples[i] = (double)buffer[i] / 32768.0;
        }
        
        free(buffer);
    } else if (format.bits_per_sample == 8) {
        // 8-bit PCM
        uint8_t* buffer = (uint8_t*)malloc(data_chunk.data_size);
        if (!buffer) {
            free(data->samples);
            return false;
        }
        
        if (fread(buffer, data_chunk.data_size, 1, file) != 1) {
            free(buffer);
            free(data->samples);
            return false;
        }
        
        // Convert to double [-1.0, 1.0]
        for (size_t i = 0; i < num_samples * format.num_channels; i++) {
            data->samples[i] = ((double)buffer[i] - 128.0) / 128.0;
        }
        
        free(buffer);
    } else {
        // Unsupported bit depth
        free(data->samples);
        return false;
    }
    
    return true;
}

bool save_wav_file(FILE* file, SignalData* data) {
    if (!file || !data || !data->samples) {
        return false;
    }
    
    // Prepare WAV header
    WAVHeader header;
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    
    // Prepare format chunk
    WAVFormat format;
    memcpy(format.fmt, "fmt ", 4);
    format.chunk_size = 16;
    format.audio_format = 1; // PCM
    format.num_channels = data->num_channels;
    format.sample_rate = (uint32_t)data->sample_rate;
    format.bits_per_sample = 16;
    format.block_align = format.num_channels * format.bits_per_sample / 8;
    format.byte_rate = format.sample_rate * format.block_align;
    
    // Prepare data chunk
    WAVData data_chunk;
    memcpy(data_chunk.data, "data", 4);
    data_chunk.data_size = data->num_samples * data->num_channels * sizeof(int16_t);
    
    // Calculate file size
    header.file_size = sizeof(WAVHeader) + sizeof(WAVFormat) + sizeof(WAVData) + 
                       data_chunk.data_size - 8;
    
    // Write header
    if (fwrite(&header, sizeof(WAVHeader), 1, file) != 1) {
        return false;
    }
    
    // Write format
    if (fwrite(&format, sizeof(WAVFormat), 1, file) != 1) {
        return false;
    }
    
    // Write data chunk header
    if (fwrite(&data_chunk, sizeof(WAVData), 1, file) != 1) {
        return false;
    }
    
    // Convert and write samples
    int16_t* buffer = (int16_t*)malloc(data_chunk.data_size);
    if (!buffer) {
        return false;
    }
    
    for (size_t i = 0; i < data->num_samples * data->num_channels; i++) {
        // Clamp to [-1.0, 1.0] and convert to int16
        double sample = data->samples[i];
        if (sample > 1.0) sample = 1.0;
        if (sample < -1.0) sample = -1.0;
        buffer[i] = (int16_t)(sample * 32767.0);
    }
    
    if (fwrite(buffer, data_chunk.data_size, 1, file) != 1) {
        free(buffer);
        return false;
    }
    
    free(buffer);
    fflush(file);
    return true;
}
