/**
 * @file cllm_cymatic_frequencies.c
 * @brief Implementation of cymatic frequency integration
 */

#include "ai/cllm_cymatic_frequencies.h"
#include <string.h>
#include "math/transcendental.h"

/**
 * @brief Small epsilon for floating point comparisons
 */
#define CYMATIC_EPSILON 1e-6

/**
 * @brief Calculate cymatic correction for angular position
 */
double cymatic_correction(double omega, double amplitude, double phase) {
    if (omega < CYMATIC_EPSILON) {
        return 0.0;
    }
    
    // Calculate normalized frequency ratio
    double freq_ratio = omega / FREQ_REFERENCE;
    
    // Calculate angular correction: amplitude * sin(2π * ratio + phase)
    double angle = 2.0 * M_PI * freq_ratio + phase;
    double correction = amplitude * math_sin(angle);
    
    return correction;
}

/**
 * @brief Calculate cymatic correction from frequency configuration
 */
double cymatic_correction_from_config(const CymaticFrequency *config) {
    if (!config) {
        return 0.0;
    }
    
    return cymatic_correction(config->frequency, config->amplitude, config->phase);
}

/**
 * @brief Get predefined cymatic frequency configuration
 */
CymaticFrequency cymatic_get_frequency_config(CymaticFrequencyType type) {
    CymaticFrequency config;
    config.type = type;
    config.amplitude = 1.0;
    config.phase = 0.0;
    
    switch (type) {
        case CYMATIC_FREQ_432_HZ:
            config.frequency = FREQ_432_HZ;
            break;
        case CYMATIC_FREQ_528_HZ:
            config.frequency = FREQ_528_HZ;
            break;
        case CYMATIC_FREQ_SCHUMANN:
            config.frequency = FREQ_SCHUMANN;
            break;
        case CYMATIC_FREQ_GAMMA:
            config.frequency = FREQ_GAMMA;
            break;
        case CYMATIC_FREQ_ALPHA:
            config.frequency = FREQ_ALPHA;
            break;
        case CYMATIC_FREQ_THETA:
            config.frequency = FREQ_THETA;
            break;
        case CYMATIC_FREQ_DELTA:
            config.frequency = FREQ_DELTA;
            break;
        case CYMATIC_FREQ_BETA:
            config.frequency = FREQ_BETA;
            break;
        case CYMATIC_FREQ_CUSTOM:
        default:
            config.frequency = FREQ_432_HZ;
            break;
    }
    
    return config;
}

/**
 * @brief Calculate resonance factor between two frequencies
 */
double cymatic_resonance_factor(double freq1, double freq2) {
    if (freq1 < CYMATIC_EPSILON || freq2 < CYMATIC_EPSILON) {
        return 0.0;
    }
    
    // Calculate frequency ratio
    double ratio = (freq1 > freq2) ? (freq1 / freq2) : (freq2 / freq1);
    
    // Check if ratio is close to a simple rational ratio (harmonic relationship)
    // Perfect resonance occurs at simple ratios: 1:1, 2:1, 3:2, 4:3, 5:4, etc.
    
    // Check common musical intervals
    double common_ratios[] = {
        1.0,      // Unison (1:1)
        2.0,      // Octave (2:1)
        1.5,      // Perfect fifth (3:2)
        1.333333, // Perfect fourth (4:3)
        1.25,     // Major third (5:4)
        1.2,      // Minor third (6:5)
        1.125,    // Major second (9:8)
        3.0,      // Octave + fifth (3:1)
        4.0       // Double octave (4:1)
    };
    
    // Find closest common ratio
    double min_error = 1.0;
    for (size_t i = 0; i < sizeof(common_ratios) / sizeof(common_ratios[0]); i++) {
        double error = math_abs(ratio - common_ratios[i]);
        if (error < min_error) {
            min_error = error;
        }
    }
    
    // Resonance factor decreases with ratio error
    // Use exponential decay: e^(-error/tolerance)
    double tolerance = 0.05; // 5% tolerance
    double resonance = math_exp(-min_error / tolerance);
    
    // Clamp to [0, 1]
    if (resonance < 0.0) resonance = 0.0;
    if (resonance > 1.0) resonance = 1.0;
    
    return resonance;
}

/**
 * @brief Calculate harmonic series for a base frequency
 */
double cymatic_harmonic(double base_freq, uint32_t harmonic_number) {
    if (harmonic_number == 0) {
        return 0.0;
    }
    
    return base_freq * (double)harmonic_number;
}

/**
 * @brief Check if frequency is in natural harmonic series
 */
bool cymatic_is_natural_harmonic(double freq, double tolerance) {
    if (freq < CYMATIC_EPSILON) {
        return false;
    }
    
    // Check if freq is a harmonic of 432 Hz
    double ratio = freq / FREQ_432_HZ;
    double closest_integer = math_round(ratio);
    double error = math_abs(ratio - closest_integer);
    
    return (error < tolerance / FREQ_432_HZ);
}

/**
 * @brief Calculate combined correction from multiple frequencies
 */
double cymatic_combined_correction(const CymaticFrequency *frequencies, size_t count) {
    if (!frequencies || count == 0) {
        return 0.0;
    }
    
    double total_correction = 0.0;
    double total_amplitude = 0.0;
    
    // Weighted sum of corrections
    for (size_t i = 0; i < count; i++) {
        double correction = cymatic_correction_from_config(&frequencies[i]);
        total_correction += correction * frequencies[i].amplitude;
        total_amplitude += frequencies[i].amplitude;
    }
    
    // Normalize by total amplitude
    if (total_amplitude > CYMATIC_EPSILON) {
        total_correction /= total_amplitude;
    }
    
    return total_correction;
}

/**
 * @brief Get frequency name as string
 */
const char* cymatic_frequency_name(CymaticFrequencyType type) {
    switch (type) {
        case CYMATIC_FREQ_432_HZ:
            return "432 Hz (Verdi's A)";
        case CYMATIC_FREQ_528_HZ:
            return "528 Hz (Love Frequency)";
        case CYMATIC_FREQ_SCHUMANN:
            return "7.83 Hz (Schumann Resonance)";
        case CYMATIC_FREQ_GAMMA:
            return "40 Hz (Gamma Brainwave)";
        case CYMATIC_FREQ_ALPHA:
            return "10 Hz (Alpha Brainwave)";
        case CYMATIC_FREQ_THETA:
            return "6 Hz (Theta Brainwave)";
        case CYMATIC_FREQ_DELTA:
            return "2 Hz (Delta Brainwave)";
        case CYMATIC_FREQ_BETA:
            return "20 Hz (Beta Brainwave)";
        case CYMATIC_FREQ_CUSTOM:
            return "Custom Frequency";
        default:
            return "Unknown Frequency";
    }
}

/**
 * @brief Calculate Schumann resonance harmonics
 */
double cymatic_schumann_harmonic(uint32_t harmonic_number) {
    if (harmonic_number == 0 || harmonic_number > 5) {
        return 0.0;
    }
    
    // Schumann resonance harmonics (measured values)
    static const double schumann_harmonics[] = {
        7.83,   // Fundamental
        14.3,   // 2nd harmonic
        20.8,   // 3rd harmonic
        27.3,   // 4th harmonic
        33.8    // 5th harmonic
    };
    
    return schumann_harmonics[harmonic_number - 1];
}

/**
 * @brief Convert frequency to period in nanoseconds
 */
uint64_t cymatic_frequency_to_ns(double frequency) {
    if (frequency <= 0.0) {
        return 0;
    }
    
    double period_seconds = 1.0 / frequency;
    return (uint64_t)(period_seconds * NS_PER_SECOND);
}

/**
 * @brief Convert period in nanoseconds to frequency
 */
double cymatic_ns_to_frequency(uint64_t period_ns) {
    if (period_ns == 0) {
        return 0.0;
    }
    
    double period_seconds = (double)period_ns / NS_PER_SECOND;
    return 1.0 / period_seconds;
}

/**
 * @brief Get period in nanoseconds for a frequency type
 */
uint64_t cymatic_get_period_ns(CymaticFrequencyType type) {
    switch (type) {
        case CYMATIC_FREQ_432_HZ:
            return NS_432_HZ;
        case CYMATIC_FREQ_528_HZ:
            return NS_528_HZ;
        case CYMATIC_FREQ_SCHUMANN:
            return NS_SCHUMANN;
        case CYMATIC_FREQ_GAMMA:
            return NS_GAMMA;
        case CYMATIC_FREQ_ALPHA:
            return NS_ALPHA;
        case CYMATIC_FREQ_THETA:
            return NS_THETA;
        case CYMATIC_FREQ_DELTA:
            return NS_DELTA;
        case CYMATIC_FREQ_BETA:
            return NS_BETA;
        case CYMATIC_FREQ_CUSTOM:
        default:
            return 0;
    }
}
