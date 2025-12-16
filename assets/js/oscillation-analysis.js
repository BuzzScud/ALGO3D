/**
 * Oscillation Analysis Module
 * FFT-based oscillation decomposition for price analysis
 * Ported from math/src/oscillation_decomposition.c
 */

const OscillationAnalyzer = (function() {
    'use strict';

    /**
     * Complex number representation
     */
    class Complex {
        constructor(real = 0, imag = 0) {
            this.real = real;
            this.imag = imag;
        }

        add(other) {
            return new Complex(this.real + other.real, this.imag + other.imag);
        }

        sub(other) {
            return new Complex(this.real - other.real, this.imag - other.imag);
        }

        mul(other) {
            return new Complex(
                this.real * other.real - this.imag * other.imag,
                this.real * other.imag + this.imag * other.real
            );
        }

        magnitude() {
            return Math.sqrt(this.real * this.real + this.imag * this.imag);
        }

        phase() {
            return Math.atan2(this.imag, this.real);
        }
    }

    /**
     * Find next power of 2
     */
    function nextPowerOf2(n) {
        let power = 1;
        while (power < n) power *= 2;
        return power;
    }

    /**
     * FFT implementation (Cooley-Tukey radix-2)
     * @param {Array<Complex>} x - Input signal (complex numbers)
     * @param {number} n - Size (must be power of 2)
     */
    function fft(x, n) {
        if (n <= 1) return;

        // Split into even and odd
        const even = [];
        const odd = [];
        for (let i = 0; i < n; i += 2) {
            even.push(x[i]);
            odd.push(x[i + 1] || new Complex(0, 0));
        }

        // Recursively compute FFT
        fft(even, even.length);
        fft(odd, odd.length);

        // Combine results
        for (let k = 0; k < n / 2; k++) {
            const angle = -2.0 * Math.PI * k / n;
            const twiddle = new Complex(Math.cos(angle), Math.sin(angle));
            const t = twiddle.mul(odd[k]);

            x[k] = even[k].add(t);
            x[k + n / 2] = even[k].sub(t);
        }
    }

    /**
     * Compute power spectrum from FFT result
     */
    function computePowerSpectrum(fftResult) {
        return fftResult.slice(0, fftResult.length / 2).map(c => c.magnitude());
    }

    /**
     * Oscillation Component
     */
    class OscillationComponent {
        constructor(frequency, amplitude, phase, period) {
            this.frequency = frequency;
            this.amplitude = amplitude;
            this.phase = phase;
            this.period = period;
            this.reconstructed = null;
        }
    }

    /**
     * Oscillation Decomposer
     */
    class OscillationDecomposer {
        constructor(signal, samplingRate, maxComponents = 10) {
            this.originalSignal = [...signal];
            this.samplingRate = samplingRate;
            this.maxComponents = maxComponents;
            this.components = [];
            this.residual = [...signal];
            this.signalLength = signal.length;
        }

        /**
         * Decompose signal into oscillation components
         */
        decompose() {
            const n = this.signalLength;
            const fftSize = nextPowerOf2(n);

            // Convert signal to complex numbers (zero-padded)
            const fftInput = [];
            for (let i = 0; i < fftSize; i++) {
                if (i < n) {
                    fftInput.push(new Complex(this.residual[i], 0));
                } else {
                    fftInput.push(new Complex(0, 0));
                }
            }

            // Compute FFT
            fft(fftInput, fftSize);

            // Compute power spectrum
            const powerSpectrum = computePowerSpectrum(fftInput);

            // Find dominant frequencies
            for (let comp = 0; comp < this.maxComponents; comp++) {
                // Find peak in power spectrum (skip DC component at index 0)
                let peakIdx = 1;
                let peakPower = powerSpectrum[1];

                for (let i = 2; i < powerSpectrum.length; i++) {
                    if (powerSpectrum[i] > peakPower) {
                        peakPower = powerSpectrum[i];
                        peakIdx = i;
                    }
                }

                // Stop if peak is too small
                const signalEnergy = this.originalSignal.reduce((sum, val) => sum + val * val, 0);
                if (peakPower * peakPower < 0.01 * signalEnergy) {
                    break;
                }

                // Extract component
                const component = new OscillationComponent(
                    peakIdx / fftSize,  // Normalized frequency
                    2.0 * fftInput[peakIdx].magnitude() / n,  // Amplitude
                    fftInput[peakIdx].phase(),  // Phase
                    Math.floor(fftSize / peakIdx)  // Period
                );

                // Reconstruct this component
                component.reconstructed = [];
                for (let i = 0; i < n; i++) {
                    const angle = 2.0 * Math.PI * component.frequency * i + component.phase;
                    component.reconstructed.push(component.amplitude * Math.cos(angle));
                }

                // Remove component from residual
                for (let i = 0; i < n; i++) {
                    this.residual[i] -= component.reconstructed[i];
                }

                this.components.push(component);

                // Zero out peak and nearby bins
                for (let i = Math.max(0, peakIdx - 2); i <= Math.min(powerSpectrum.length - 1, peakIdx + 2); i++) {
                    powerSpectrum[i] = 0;
                }
            }

            // Calculate residual energy
            this.residualEnergy = this.residual.reduce((sum, val) => sum + val * val, 0);

            return this.components;
        }

        /**
         * Get dominant frequencies
         */
        getDominantFrequencies(count = 5) {
            return this.components
                .slice(0, count)
                .map(c => ({
                    frequency: c.frequency * this.samplingRate,
                    amplitude: c.amplitude,
                    phase: c.phase,
                    period: c.period
                }));
        }

        /**
         * Reconstruct signal from components
         */
        reconstruct(componentIndices = null) {
            const indices = componentIndices || this.components.map((_, i) => i);
            const reconstructed = new Array(this.signalLength).fill(0);

            indices.forEach(idx => {
                if (this.components[idx] && this.components[idx].reconstructed) {
                    for (let i = 0; i < this.signalLength; i++) {
                        reconstructed[i] += this.components[idx].reconstructed[i];
                    }
                }
            });

            return reconstructed;
        }
    }

    /**
     * Analyze price oscillations
     * @param {Array<number>} prices - Price series
     * @param {number} samplingRate - Sampling rate (e.g., 1 for daily, 252 for yearly)
     * @param {number} maxComponents - Maximum number of components to extract
     * @returns {OscillationDecomposer} Decomposer with analysis results
     */
    function analyzeOscillations(prices, samplingRate = 1, maxComponents = 10) {
        if (!prices || prices.length < 4) {
            throw new Error('Insufficient data for oscillation analysis');
        }

        const decomposer = new OscillationDecomposer(prices, samplingRate, maxComponents);
        decomposer.decompose();
        return decomposer;
    }

    /**
     * Get dominant frequency for use in projections
     * @param {Array<number>} prices - Price series
     * @param {number} samplingRate - Sampling rate
     * @returns {number} Dominant frequency in Hz
     */
    function getDominantFrequency(prices, samplingRate = 1) {
        const decomposer = analyzeOscillations(prices, samplingRate, 5);
        const dominant = decomposer.getDominantFrequencies(1);
        return dominant.length > 0 ? dominant[0].frequency : 0;
    }

    /**
     * Calculate omegaHz based on price oscillations
     * @param {Array<number>} prices - Price series
     * @param {number} baseFreq - Base frequency (default 432)
     * @returns {number} Adjusted omegaHz
     */
    function calculateOmegaFromOscillations(prices, baseFreq = 432) {
        const dominantFreq = getDominantFrequency(prices);
        if (dominantFreq === 0) return baseFreq;

        // Scale base frequency by dominant oscillation frequency
        return baseFreq * (1 + dominantFreq / 100);
    }

    return {
        OscillationDecomposer,
        OscillationComponent,
        analyzeOscillations,
        getDominantFrequency,
        calculateOmegaFromOscillations,
        Complex
    };
})();


