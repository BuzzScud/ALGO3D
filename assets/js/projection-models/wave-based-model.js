/**
 * Wave-Based Projection Model
 * Based on formula_wave_z and formula_psi_mn (multi-frequency wave synthesis)
 */

class WaveBasedModel extends ProjectionModel {
    constructor(config = {}) {
        super('Wave-Based', config);
    }

    project(historicalPrices, params) {
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const steps = parseInt(params.steps) || 20;
        const numModes = params.numModes || 3;

        // Extract wave parameters from historical data
        const waveParams = this.extractWaveParameters(historicalPrices, numModes);

        // Generate projection using wave synthesis
        const points = [];
        for (let i = 0; i < steps; i++) {
            const waveValue = this.waveZ(waveParams, i, historicalPrices.length);
            const projectedPrice = lastPrice * (1 + waveValue * 0.01); // Scale to 1% max change per step
            points.push(this.trunc(projectedPrice, 4));
        }

        return {
            points: points,
            confidence: this.getConfidence(),
            metadata: {
                model: this.name,
                numModes: numModes,
                waveParams: waveParams.length
            }
        };
    }

    /**
     * Extract wave parameters from price series
     * Returns array of {P1, P2, P3, P4} parameters
     */
    extractWaveParameters(prices, numModes) {
        if (prices.length < 4) return [];

        // Normalize prices to 0-1 range
        const minPrice = Math.min(...prices);
        const maxPrice = Math.max(...prices);
        const range = maxPrice - minPrice;
        const normalized = range > 0 
            ? prices.map(p => (p - minPrice) / range)
            : prices.map(() => 0.5);

        const params = [];
        const n = normalized.length;

        // Extract multiple wave modes
        for (let mode = 1; mode <= numModes; mode++) {
            // Use FFT-like analysis to find wave parameters
            let P1 = 0, P2 = 0, P3 = 0, P4 = 0;

            // Analyze frequency content
            for (let t = 0; t < n; t++) {
                const x = t / n; // Normalized position
                const y = normalized[t];

                // Estimate wave parameters
                const freq1 = mode * 2;
                const freq2 = mode * 3;

                P1 += y * Math.sin(freq1 * Math.PI * x);
                P2 += y * Math.cos(freq1 * Math.PI * x);
                P3 += y * Math.sin(freq2 * Math.PI * x);
                P4 += y * Math.cos(freq2 * Math.PI * x);
            }

            // Normalize
            P1 /= n;
            P2 /= n;
            P3 /= n;
            P4 /= n;

            // Scale to reasonable range
            const scale = 0.1;
            params.push({
                P1: P1 * scale,
                P2: P2 * scale,
                P3: P3 * scale,
                P4: P4 * scale
            });
        }

        return params;
    }

    /**
     * Wave Z function (formula_wave_z)
     * Multi-frequency wave synthesis
     */
    waveZ(params, x, totalLength) {
        if (params.length === 0) return 0;

        let sum = 0;
        const normalizedX = x / totalLength;

        params.forEach((param, idx) => {
            const { P1, P2, P3, P4 } = param;
            const freq = (idx + 1) * 2; // Different frequency for each mode

            const term1 = Math.sin(P1 * Math.PI * normalizedX * freq) * 
                         Math.cos(P2 * Math.PI * normalizedX * freq);
            const term2 = Math.sin(P3 * Math.PI * normalizedX * freq) * 
                         Math.cos(P4 * Math.PI * normalizedX * freq);
            
            sum += (term1 + term2) / params.length; // Average contributions
        });

        return sum;
    }

    /**
     * Psi MN function (formula_psi_mn)
     * 2D wave function with m,n modes
     */
    psiMN(m, n, x, y, L, W) {
        if (L === 0 || W === 0) return 0;

        const term1 = Math.sin(m * Math.PI * x / L);
        const term2 = Math.sin(n * Math.PI * y / W);
        return term1 * term2;
    }

    trunc(x, decimals) {
        const multiplier = Math.pow(10, decimals);
        return Math.floor(x * multiplier) / multiplier;
    }
}

// Make available globally
if (typeof window !== 'undefined') {
    window.WaveBasedModel = WaveBasedModel;
}


