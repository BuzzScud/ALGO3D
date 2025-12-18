/**
 * @file recovery_signal.h
 * @brief Signal Processing Recovery Library
 * 
 * Production-grade signal processing and recovery for:
 * - HAM radio signal recovery
 * - Audio signal recovery
 * - Time series recovery
 * - Frequency domain recovery
 * - Multi-channel signal recovery
 * - Real-time signal processing
 * 
 * Uses samples-as-anchors with OBJECTIVE 28 Phase 1-6 algorithms.
 */

#ifndef RECOVERY_SIGNAL_H
#define RECOVERY_SIGNAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "math/complex.h"

#ifdef __cplusplus
extern "C" {
#endif

// Version
#define RECOVERY_SIGNAL_VERSION_MAJOR 1
#define RECOVERY_SIGNAL_VERSION_MINOR 0
#define RECOVERY_SIGNAL_VERSION_PATCH 0

// Error codes
typedef enum {
    SIGNAL_OK = 0,
    SIGNAL_ERROR_INVALID_PARAM = -1,
    SIGNAL_ERROR_OUT_OF_MEMORY = -2,
    SIGNAL_ERROR_NOT_CONVERGED = -3,
    SIGNAL_ERROR_INVALID_FORMAT = -4,
    SIGNAL_ERROR_UNSUPPORTED = -5
} signal_error_t;

// Signal types
typedef enum {
    SIGNAL_TYPE_AUDIO,              // Audio signal (PCM, WAV, etc.)
    SIGNAL_TYPE_RADIO,              // Radio signal (AM, FM, SSB, etc.)
    SIGNAL_TYPE_TIME_SERIES,        // Time series data
    SIGNAL_TYPE_FREQUENCY,          // Frequency domain data
    SIGNAL_TYPE_COMPLEX,            // Complex IQ data
    SIGNAL_TYPE_MULTI_CHANNEL       // Multi-channel signal
} signal_type_t;

// Modulation types
typedef enum {
    MODULATION_AM,                  // Amplitude Modulation
    MODULATION_FM,                  // Frequency Modulation
    MODULATION_SSB,                 // Single Sideband
    MODULATION_USB,                 // Upper Sideband
    MODULATION_LSB,                 // Lower Sideband
    MODULATION_CW,                  // Continuous Wave
    MODULATION_PSK,                 // Phase Shift Keying
    MODULATION_QAM,                 // Quadrature Amplitude Modulation
    MODULATION_UNKNOWN              // Unknown modulation
} modulation_type_t;

// Signal format
typedef enum {
    FORMAT_PCM_S16LE,               // 16-bit signed little-endian PCM
    FORMAT_PCM_S32LE,               // 32-bit signed little-endian PCM
    FORMAT_PCM_F32LE,               // 32-bit float little-endian PCM
    FORMAT_PCM_F64LE,               // 64-bit float little-endian PCM
    FORMAT_COMPLEX_F32,             // 32-bit complex float (IQ)
    FORMAT_COMPLEX_F64              // 64-bit complex float (IQ)
} signal_format_t;

// Signal recovery context
typedef struct {
    signal_type_t type;             // Signal type
    signal_format_t format;         // Signal format
    uint32_t sample_rate;           // Sample rate (Hz)
    uint32_t num_channels;          // Number of channels
    double center_frequency;        // Center frequency (Hz, for radio)
    modulation_type_t modulation;   // Modulation type (for radio)
    double bandwidth;               // Bandwidth (Hz)
    int max_iterations;             // Maximum iterations
    double convergence_threshold;   // Convergence threshold
    bool use_fft;                   // Use FFT for frequency domain
    bool denoise;                   // Apply denoising
    int verbose;                    // Verbose output
} signal_recovery_ctx_t;

// Signal data structure
typedef struct {
    void* samples;                  // Sample data
    size_t num_samples;             // Number of samples
    signal_format_t format;         // Sample format
    uint32_t sample_rate;           // Sample rate (Hz)
    uint32_t num_channels;          // Number of channels
    double* timestamps;             // Timestamps (optional)
} signal_data_t;

// Recovery result
typedef struct {
    signal_data_t* recovered_signal; // Recovered signal
    int iterations;                 // Iterations taken
    double final_oscillation;       // Final oscillation value
    double snr;                     // Signal-to-noise ratio
    double quality_score;           // Quality score (0-1)
    bool converged;                 // Whether it converged
    double time_seconds;            // Time taken
    char* error_message;            // Error message (if any)
} signal_recovery_result_t;

// Frequency domain data
typedef struct {
    double complex* spectrum;       // Frequency spectrum
    size_t num_bins;                // Number of frequency bins
    double* frequencies;            // Frequency values (Hz)
    double* magnitudes;             // Magnitude values
    double* phases;                 // Phase values
    double sample_rate;             // Sample rate (Hz)
} frequency_data_t;

/**
 * Signal Recovery
 */

/**
 * Recover corrupted signal
 * 
 * @param ctx Recovery context
 * @param corrupted Corrupted signal data
 * @param reference Reference signal samples (optional)
 * @param num_references Number of reference samples
 * @return Recovery result (caller must free)
 */
signal_recovery_result_t* recover_signal(const signal_recovery_ctx_t* ctx,
                                         const signal_data_t* corrupted,
                                         const signal_data_t** reference,
                                         size_t num_references);

/**
 * Recover audio signal
 * 
 * @param corrupted Corrupted audio data
 * @param sample_rate Sample rate (Hz)
 * @param num_channels Number of channels
 * @return Recovery result (caller must free)
 */
signal_recovery_result_t* recover_audio(const float* corrupted,
                                        size_t num_samples,
                                        uint32_t sample_rate,
                                        uint32_t num_channels);

/**
 * Recover radio signal
 * 
 * @param corrupted Corrupted IQ data
 * @param num_samples Number of IQ samples
 * @param sample_rate Sample rate (Hz)
 * @param center_freq Center frequency (Hz)
 * @param modulation Modulation type
 * @return Recovery result (caller must free)
 */
signal_recovery_result_t* recover_radio_signal(const double complex* corrupted,
                                               size_t num_samples,
                                               uint32_t sample_rate,
                                               double center_freq,
                                               modulation_type_t modulation);

/**
 * Recover time series
 * 
 * @param corrupted Corrupted time series data
 * @param num_samples Number of samples
 * @param timestamps Timestamps (optional)
 * @return Recovery result (caller must free)
 */
signal_recovery_result_t* recover_time_series(const double* corrupted,
                                              size_t num_samples,
                                              const double* timestamps);

/**
 * Signal Processing
 */

/**
 * Apply FFT to signal
 * 
 * @param signal Signal data
 * @return Frequency domain data (caller must free)
 */
frequency_data_t* signal_fft(const signal_data_t* signal);

/**
 * Apply inverse FFT
 * 
 * @param freq Frequency domain data
 * @return Signal data (caller must free)
 */
signal_data_t* signal_ifft(const frequency_data_t* freq);

/**
 * Denoise signal
 * 
 * @param signal Signal data
 * @param threshold Noise threshold
 * @return Denoised signal (caller must free)
 */
signal_data_t* signal_denoise(const signal_data_t* signal, double threshold);

/**
 * Resample signal
 * 
 * @param signal Signal data
 * @param new_sample_rate New sample rate (Hz)
 * @return Resampled signal (caller must free)
 */
signal_data_t* signal_resample(const signal_data_t* signal, uint32_t new_sample_rate);

/**
 * Filter signal
 * 
 * @param signal Signal data
 * @param low_freq Low frequency cutoff (Hz)
 * @param high_freq High frequency cutoff (Hz)
 * @return Filtered signal (caller must free)
 */
signal_data_t* signal_filter(const signal_data_t* signal,
                             double low_freq,
                             double high_freq);

/**
 * Demodulate signal
 * 
 * @param signal Signal data (IQ)
 * @param modulation Modulation type
 * @return Demodulated signal (caller must free)
 */
signal_data_t* signal_demodulate(const signal_data_t* signal,
                                modulation_type_t modulation);

/**
 * Calculate SNR
 * 
 * @param signal Signal data
 * @param noise_floor Noise floor estimate
 * @return SNR in dB
 */
double signal_calculate_snr(const signal_data_t* signal, double noise_floor);

/**
 * Detect modulation type
 * 
 * @param signal Signal data (IQ)
 * @return Detected modulation type
 */
modulation_type_t signal_detect_modulation(const signal_data_t* signal);

/**
 * File I/O
 */

/**
 * Load signal from WAV file
 * 
 * @param filename WAV file path
 * @return Signal data (caller must free)
 */
signal_data_t* signal_load_wav(const char* filename);

/**
 * Save signal to WAV file
 * 
 * @param signal Signal data
 * @param filename WAV file path
 * @return SIGNAL_OK on success
 */
signal_error_t signal_save_wav(const signal_data_t* signal, const char* filename);

/**
 * Load IQ data from file
 * 
 * @param filename File path
 * @param format Data format
 * @param sample_rate Sample rate (Hz)
 * @return Signal data (caller must free)
 */
signal_data_t* signal_load_iq(const char* filename,
                              signal_format_t format,
                              uint32_t sample_rate);

/**
 * Save IQ data to file
 * 
 * @param signal Signal data
 * @param filename File path
 * @param format Data format
 * @return SIGNAL_OK on success
 */
signal_error_t signal_save_iq(const signal_data_t* signal,
                              const char* filename,
                              signal_format_t format);

/**
 * Memory Management
 */

/**
 * Create signal data
 * 
 * @param num_samples Number of samples
 * @param format Sample format
 * @param sample_rate Sample rate (Hz)
 * @param num_channels Number of channels
 * @return Signal data (caller must free)
 */
signal_data_t* signal_create(size_t num_samples,
                             signal_format_t format,
                             uint32_t sample_rate,
                             uint32_t num_channels);

/**
 * Free signal data
 * 
 * @param signal Signal data to free
 */
void signal_free(signal_data_t* signal);

/**
 * Free recovery result
 * 
 * @param result Result to free
 */
void signal_free_result(signal_recovery_result_t* result);

/**
 * Free frequency data
 * 
 * @param freq Frequency data to free
 */
void signal_free_frequency(frequency_data_t* freq);

/**
 * Utility Functions
 */

/**
 * Get error string
 * 
 * @param error Error code
 * @return Error string
 */
const char* signal_error_string(signal_error_t error);

/**
 * Get signal type string
 * 
 * @param type Signal type
 * @return Signal type string
 */
const char* signal_type_string(signal_type_t type);

/**
 * Get modulation type string
 * 
 * @param modulation Modulation type
 * @return Modulation type string
 */
const char* modulation_type_string(modulation_type_t modulation);

/**
 * Get format string
 * 
 * @param format Signal format
 * @return Format string
 */
const char* signal_format_string(signal_format_t format);

/**
 * Get sample size in bytes
 * 
 * @param format Signal format
 * @return Sample size in bytes
 */
size_t signal_sample_size(signal_format_t format);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_SIGNAL_H