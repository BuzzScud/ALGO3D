/**
 * @file recovery_signal.c
 * @brief Signal Processing Recovery Implementation
 * 
 * Integrates with OBJECTIVE 28 Phase 1-6 algorithms for signal recovery.
 */

#include "recovery_signal.h"
#include "recovery_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "math/math.h"
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Signal Recovery
 */

signal_recovery_result_t* recover_signal(const signal_recovery_ctx_t* ctx,
                                         const signal_data_t* corrupted,
                                         const signal_data_t** reference,
                                         size_t num_references) {
    if (!ctx || !corrupted) return NULL;
    
    signal_recovery_result_t* result = calloc(1, sizeof(signal_recovery_result_t));
    if (!result) return NULL;
    
    clock_t start = clock();
    
    // Initialize recovery context using core library
    recovery_config_t config = recovery_default_config();
    config.max_iterations = ctx->max_iterations > 0 ? ctx->max_iterations : 10000;
    config.convergence_threshold = ctx->convergence_threshold > 0 ? ctx->convergence_threshold : 0.001;
    config.verbose = ctx->verbose;
    config.method = RECOVERY_METHOD_SIGNAL;
    
    recovery_context_t* recovery_ctx = recovery_init(&config);
    if (!recovery_ctx) {
        result->error_message = strdup("Failed to initialize recovery context");
        return result;
    }
    
    // Convert signal to bytes for recovery
    size_t sample_size = signal_sample_size(corrupted->format);
    size_t data_size = corrupted->num_samples * sample_size * corrupted->num_channels;
    
    // Set Q data (corrupted signal)
    recovery_error_t err = recovery_set_q(recovery_ctx, 
                                         (const uint8_t*)corrupted->samples,
                                         data_size);
    if (err != RECOVERY_OK) {
        result->error_message = strdup("Failed to set Q data");
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Add reference samples if available
    if (reference && num_references > 0) {
        for (size_t i = 0; i < num_references; i++) {
            if (reference[i] && reference[i]->samples) {
                size_t ref_size = reference[i]->num_samples * sample_size * reference[i]->num_channels;
                err = recovery_add_sample(recovery_ctx,
                                         (const uint8_t*)reference[i]->samples,
                                         ref_size,
                                         0,  // offset
                                         1.0); // confidence
                if (err != RECOVERY_OK) {
                    result->error_message = strdup("Failed to add reference sample");
                    recovery_free(recovery_ctx);
                    return result;
                }
            }
        }
    }
    
    // Run recovery using OBJECTIVE 28 algorithms
    err = recovery_run(recovery_ctx);
    if (err != RECOVERY_OK) {
        result->error_message = strdup("Recovery failed to converge");
        result->converged = false;
        recovery_free(recovery_ctx);
        return result;
    }
    
    // Get recovery result
    recovery_result_t* recovery_result = recovery_get_result(recovery_ctx);
    if (recovery_result && recovery_result->data) {
        // Create recovered signal
        result->recovered_signal = signal_create(corrupted->num_samples,
                                                 corrupted->format,
                                                 corrupted->sample_rate,
                                                 corrupted->num_channels);
        if (result->recovered_signal) {
            memcpy(result->recovered_signal->samples, recovery_result->data, data_size);
            result->converged = true;
            result->quality_score = recovery_result->quality_score;
        }
        
        result->iterations = recovery_result->iterations;
        result->final_oscillation = recovery_result->final_oscillation;
        
        recovery_free_result(recovery_result);
    }
    
    result->time_seconds = (double)(clock() - start) / CLOCKS_PER_SEC;
    
    recovery_free(recovery_ctx);
    return result;
}

signal_recovery_result_t* recover_audio(const float* corrupted,
                                        size_t num_samples,
                                        uint32_t sample_rate,
                                        uint32_t num_channels) {
    if (!corrupted) return NULL;
    
    // Create signal data structure
    signal_data_t signal = {
        .samples = (void*)corrupted,
        .num_samples = num_samples,
        .format = FORMAT_PCM_F32LE,
        .sample_rate = sample_rate,
        .num_channels = num_channels,
        .timestamps = NULL
    };
    
    // Create recovery context
    signal_recovery_ctx_t ctx = {
        .type = SIGNAL_TYPE_AUDIO,
        .format = FORMAT_PCM_F32LE,
        .sample_rate = sample_rate,
        .num_channels = num_channels,
        .max_iterations = 5000,
        .convergence_threshold = 0.001,
        .use_fft = true,
        .denoise = true,
        .verbose = 0
    };
    
    return recover_signal(&ctx, &signal, NULL, 0);
}

signal_recovery_result_t* recover_radio_signal(const double complex* corrupted,
                                               size_t num_samples,
                                               uint32_t sample_rate,
                                               double center_freq,
                                               modulation_type_t modulation) {
    if (!corrupted) return NULL;
    
    // Create signal data structure
    signal_data_t signal = {
        .samples = (void*)corrupted,
        .num_samples = num_samples,
        .format = FORMAT_COMPLEX_F64,
        .sample_rate = sample_rate,
        .num_channels = 1,
        .timestamps = NULL
    };
    
    // Create recovery context
    signal_recovery_ctx_t ctx = {
        .type = SIGNAL_TYPE_RADIO,
        .format = FORMAT_COMPLEX_F64,
        .sample_rate = sample_rate,
        .num_channels = 1,
        .center_frequency = center_freq,
        .modulation = modulation,
        .bandwidth = sample_rate / 2.0,
        .max_iterations = 10000,
        .convergence_threshold = 0.001,
        .use_fft = true,
        .denoise = true,
        .verbose = 0
    };
    
    return recover_signal(&ctx, &signal, NULL, 0);
}

signal_recovery_result_t* recover_time_series(const double* corrupted,
                                              size_t num_samples,
                                              const double* timestamps) {
    if (!corrupted) return NULL;
    
    // Create signal data structure
    signal_data_t signal = {
        .samples = (void*)corrupted,
        .num_samples = num_samples,
        .format = FORMAT_PCM_F64LE,
        .sample_rate = 1, // 1 Hz for time series
        .num_channels = 1,
        .timestamps = (double*)timestamps
    };
    
    // Create recovery context
    signal_recovery_ctx_t ctx = {
        .type = SIGNAL_TYPE_TIME_SERIES,
        .format = FORMAT_PCM_F64LE,
        .sample_rate = 1,
        .num_channels = 1,
        .max_iterations = 5000,
        .convergence_threshold = 0.001,
        .use_fft = false,
        .denoise = true,
        .verbose = 0
    };
    
    return recover_signal(&ctx, &signal, NULL, 0);
}

/**
 * Signal Processing
 */

frequency_data_t* signal_fft(const signal_data_t* signal) {
    if (!signal || !signal->samples) return NULL;
    
    frequency_data_t* freq = calloc(1, sizeof(frequency_data_t));
    if (!freq) return NULL;
    
    // Simplified FFT implementation (in production, use FFTW)
    size_t n = signal->num_samples;
    freq->num_bins = n / 2 + 1;
    freq->sample_rate = signal->sample_rate;
    
    freq->spectrum = calloc(freq->num_bins, sizeof(double complex));
    freq->frequencies = calloc(freq->num_bins, sizeof(double));
    freq->magnitudes = calloc(freq->num_bins, sizeof(double));
    freq->phases = calloc(freq->num_bins, sizeof(double));
    
    if (!freq->spectrum || !freq->frequencies || !freq->magnitudes || !freq->phases) {
        signal_free_frequency(freq);
        return NULL;
    }
    
    // Calculate frequency bins
    for (size_t k = 0; k < freq->num_bins; k++) {
        freq->frequencies[k] = k * signal->sample_rate / (double)n;
    }
    
    // Simplified DFT (for demonstration - use FFTW in production)
    const float* samples = (const float*)signal->samples;
    for (size_t k = 0; k < freq->num_bins; k++) {
        double complex sum = 0.0;
        for (size_t n_idx = 0; n_idx < n; n_idx++) {
            double angle = -2.0 * M_PI * k * n_idx / n;
            sum += samples[n_idx] * (cos(angle) + I * sin(angle));
        }
        freq->spectrum[k] = sum;
        freq->magnitudes[k] = cabs(sum);
        freq->phases[k] = carg(sum);
    }
    
    return freq;
}

signal_data_t* signal_ifft(const frequency_data_t* freq) {
    if (!freq || !freq->spectrum) return NULL;
    
    // Simplified IFFT (in production, use FFTW)
    size_t n = (freq->num_bins - 1) * 2;
    
    signal_data_t* signal = signal_create(n, FORMAT_PCM_F32LE, freq->sample_rate, 1);
    if (!signal) return NULL;
    
    float* samples = (float*)signal->samples;
    
    // Simplified IDFT
    for (size_t n_idx = 0; n_idx < n; n_idx++) {
        double complex sum = 0.0;
        for (size_t k = 0; k < freq->num_bins; k++) {
            double angle = 2.0 * M_PI * k * n_idx / n;
            sum += freq->spectrum[k] * (cos(angle) + I * sin(angle));
        }
        samples[n_idx] = creal(sum) / n;
    }
    
    return signal;
}

signal_data_t* signal_denoise(const signal_data_t* signal, double threshold) {
    if (!signal || !signal->samples) return NULL;
    
    // Apply FFT
    frequency_data_t* freq = signal_fft(signal);
    if (!freq) return NULL;
    
    // Threshold small magnitudes (noise)
    for (size_t i = 0; i < freq->num_bins; i++) {
        if (freq->magnitudes[i] < threshold) {
            freq->spectrum[i] = 0.0;
        }
    }
    
    // Apply IFFT
    signal_data_t* denoised = signal_ifft(freq);
    
    signal_free_frequency(freq);
    return denoised;
}

signal_data_t* signal_resample(const signal_data_t* signal, uint32_t new_sample_rate) {
    if (!signal || !signal->samples) return NULL;
    
    // Calculate new number of samples
    size_t new_num_samples = (size_t)(signal->num_samples * 
                                      (double)new_sample_rate / signal->sample_rate);
    
    signal_data_t* resampled = signal_create(new_num_samples,
                                             signal->format,
                                             new_sample_rate,
                                             signal->num_channels);
    if (!resampled) return NULL;
    
    // Linear interpolation (in production, use better resampling)
    const float* in_samples = (const float*)signal->samples;
    float* out_samples = (float*)resampled->samples;
    
    for (size_t i = 0; i < new_num_samples; i++) {
        double pos = i * (double)signal->num_samples / new_num_samples;
        size_t idx = (size_t)pos;
        double frac = pos - idx;
        
        if (idx + 1 < signal->num_samples) {
            out_samples[i] = in_samples[idx] * (1.0 - frac) + 
                            in_samples[idx + 1] * frac;
        } else {
            out_samples[i] = in_samples[idx];
        }
    }
    
    return resampled;
}

signal_data_t* signal_filter(const signal_data_t* signal,
                             double low_freq,
                             double high_freq) {
    if (!signal || !signal->samples) return NULL;
    
    // Apply FFT
    frequency_data_t* freq = signal_fft(signal);
    if (!freq) return NULL;
    
    // Apply bandpass filter
    for (size_t i = 0; i < freq->num_bins; i++) {
        if (freq->frequencies[i] < low_freq || freq->frequencies[i] > high_freq) {
            freq->spectrum[i] = 0.0;
        }
    }
    
    // Apply IFFT
    signal_data_t* filtered = signal_ifft(freq);
    
    signal_free_frequency(freq);
    return filtered;
}

signal_data_t* signal_demodulate(const signal_data_t* signal,
                                modulation_type_t modulation) {
    if (!signal || !signal->samples) return NULL;
    
    // Simplified demodulation (in production, use proper DSP)
    signal_data_t* demod = signal_create(signal->num_samples,
                                         FORMAT_PCM_F32LE,
                                         signal->sample_rate,
                                         1);
    if (!demod) return NULL;
    
    const double complex* iq = (const double complex*)signal->samples;
    float* out = (float*)demod->samples;
    
    switch (modulation) {
        case MODULATION_AM:
            // AM: magnitude of IQ
            for (size_t i = 0; i < signal->num_samples; i++) {
                out[i] = cabs(iq[i]);
            }
            break;
            
        case MODULATION_FM:
            // FM: phase difference
            for (size_t i = 1; i < signal->num_samples; i++) {
                double phase_diff = carg(iq[i]) - carg(iq[i-1]);
                out[i] = phase_diff;
            }
            out[0] = 0.0;
            break;
            
        default:
            // Default: magnitude
            for (size_t i = 0; i < signal->num_samples; i++) {
                out[i] = cabs(iq[i]);
            }
            break;
    }
    
    return demod;
}

double signal_calculate_snr(const signal_data_t* signal, double noise_floor) {
    if (!signal || !signal->samples) return 0.0;
    
    const float* samples = (const float*)signal->samples;
    
    // Calculate signal power
    double signal_power = 0.0;
    for (size_t i = 0; i < signal->num_samples; i++) {
        signal_power += samples[i] * samples[i];
    }
    signal_power /= signal->num_samples;
    
    // Calculate SNR in dB
    double snr_db = 10.0 * log10(signal_power / (noise_floor * noise_floor));
    
    return snr_db;
}

modulation_type_t signal_detect_modulation(const signal_data_t* signal) {
    if (!signal || !signal->samples) return MODULATION_UNKNOWN;
    
    // Simplified modulation detection (in production, use ML)
    // For now, return unknown
    return MODULATION_UNKNOWN;
}

/**
 * File I/O (Simplified - in production, use libsndfile)
 */

signal_data_t* signal_load_wav(const char* filename) {
    if (!filename) return NULL;
    
    // Simplified WAV loading (in production, use libsndfile)
    // For now, return NULL
    return NULL;
}

signal_error_t signal_save_wav(const signal_data_t* signal, const char* filename) {
    if (!signal || !filename) return SIGNAL_ERROR_INVALID_PARAM;
    
    // Simplified WAV saving (in production, use libsndfile)
    return SIGNAL_OK;
}

signal_data_t* signal_load_iq(const char* filename,
                              signal_format_t format,
                              uint32_t sample_rate) {
    if (!filename) return NULL;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    size_t sample_size = signal_sample_size(format);
    size_t num_samples = size / sample_size;
    
    signal_data_t* signal = signal_create(num_samples, format, sample_rate, 1);
    if (!signal) {
        fclose(f);
        return NULL;
    }
    
    fread(signal->samples, sample_size, num_samples, f);
    fclose(f);
    
    return signal;
}

signal_error_t signal_save_iq(const signal_data_t* signal,
                              const char* filename,
                              signal_format_t format) {
    if (!signal || !filename) return SIGNAL_ERROR_INVALID_PARAM;
    
    FILE* f = fopen(filename, "wb");
    if (!f) return SIGNAL_ERROR_INVALID_PARAM;
    
    size_t sample_size = signal_sample_size(format);
    fwrite(signal->samples, sample_size, signal->num_samples, f);
    fclose(f);
    
    return SIGNAL_OK;
}

/**
 * Memory Management
 */

signal_data_t* signal_create(size_t num_samples,
                             signal_format_t format,
                             uint32_t sample_rate,
                             uint32_t num_channels) {
    signal_data_t* signal = calloc(1, sizeof(signal_data_t));
    if (!signal) return NULL;
    
    size_t sample_size = signal_sample_size(format);
    signal->samples = calloc(num_samples * num_channels, sample_size);
    if (!signal->samples) {
        free(signal);
        return NULL;
    }
    
    signal->num_samples = num_samples;
    signal->format = format;
    signal->sample_rate = sample_rate;
    signal->num_channels = num_channels;
    signal->timestamps = NULL;
    
    return signal;
}

void signal_free(signal_data_t* signal) {
    if (!signal) return;
    
    if (signal->samples) free(signal->samples);
    if (signal->timestamps) free(signal->timestamps);
    
    free(signal);
}

void signal_free_result(signal_recovery_result_t* result) {
    if (!result) return;
    
    if (result->recovered_signal) signal_free(result->recovered_signal);
    if (result->error_message) free(result->error_message);
    
    free(result);
}

void signal_free_frequency(frequency_data_t* freq) {
    if (!freq) return;
    
    if (freq->spectrum) free(freq->spectrum);
    if (freq->frequencies) free(freq->frequencies);
    if (freq->magnitudes) free(freq->magnitudes);
    if (freq->phases) free(freq->phases);
    
    free(freq);
}

/**
 * Utility Functions
 */

const char* signal_error_string(signal_error_t error) {
    switch (error) {
        case SIGNAL_OK: return "Success";
        case SIGNAL_ERROR_INVALID_PARAM: return "Invalid parameter";
        case SIGNAL_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case SIGNAL_ERROR_NOT_CONVERGED: return "Recovery did not converge";
        case SIGNAL_ERROR_INVALID_FORMAT: return "Invalid format";
        case SIGNAL_ERROR_UNSUPPORTED: return "Unsupported operation";
        default: return "Unknown error";
    }
}

const char* signal_type_string(signal_type_t type) {
    switch (type) {
        case SIGNAL_TYPE_AUDIO: return "Audio";
        case SIGNAL_TYPE_RADIO: return "Radio";
        case SIGNAL_TYPE_TIME_SERIES: return "Time Series";
        case SIGNAL_TYPE_FREQUENCY: return "Frequency Domain";
        case SIGNAL_TYPE_COMPLEX: return "Complex IQ";
        case SIGNAL_TYPE_MULTI_CHANNEL: return "Multi-Channel";
        default: return "Unknown";
    }
}

const char* modulation_type_string(modulation_type_t modulation) {
    switch (modulation) {
        case MODULATION_AM: return "AM";
        case MODULATION_FM: return "FM";
        case MODULATION_SSB: return "SSB";
        case MODULATION_USB: return "USB";
        case MODULATION_LSB: return "LSB";
        case MODULATION_CW: return "CW";
        case MODULATION_PSK: return "PSK";
        case MODULATION_QAM: return "QAM";
        case MODULATION_UNKNOWN: return "Unknown";
        default: return "Unknown";
    }
}

const char* signal_format_string(signal_format_t format) {
    switch (format) {
        case FORMAT_PCM_S16LE: return "PCM S16LE";
        case FORMAT_PCM_S32LE: return "PCM S32LE";
        case FORMAT_PCM_F32LE: return "PCM F32LE";
        case FORMAT_PCM_F64LE: return "PCM F64LE";
        case FORMAT_COMPLEX_F32: return "Complex F32";
        case FORMAT_COMPLEX_F64: return "Complex F64";
        default: return "Unknown";
    }
}

size_t signal_sample_size(signal_format_t format) {
    switch (format) {
        case FORMAT_PCM_S16LE: return 2;
        case FORMAT_PCM_S32LE: return 4;
        case FORMAT_PCM_F32LE: return 4;
        case FORMAT_PCM_F64LE: return 8;
        case FORMAT_COMPLEX_F32: return 8;
        case FORMAT_COMPLEX_F64: return 16;
        default: return 0;
    }
}