/**
 * Harmonic Projection Model
 * Based on formula_harm_score and formula_fhs (Fractional Harmonic Series)
 */

class HarmonicModel extends ProjectionModel {
    constructor(config = {}) {
        super('Harmonic', config);
        this.goldenRatio = (1 + Math.sqrt(5)) / 2;
    }

    project(historicalPrices, params) {
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const steps = parseInt(params.steps) || 20;
        const baseFreq = params.baseFreq || 432;

        // Analyze harmonic content in historical prices
        const harmonics = this.analyzeHarmonics(historicalPrices);
        
        // Calculate harmonic score
        const harmScore = this.calculateHarmonicScore(historicalPrices, harmonics);

        // Generate projection using harmonic series
        const points = [];
        for (let i = 0; i < steps; i++) {
            const harmonicTerm = this.calculateHarmonicTerm(i, harmonics, harmScore, baseFreq);
            const projectedPrice = lastPrice * (1 + harmonicTerm);
            points.push(this.trunc(projectedPrice, 4));
        }

        return {
            points: points,
            confidence: this.getConfidence(),
            metadata: {
                model: this.name,
                harmonics: harmonics.length,
                harmScore: harmScore
            }
        };
    }

    /**
     * Analyze harmonic frequencies in price series
     * Returns array of {frequency, amplitude, phase}
     */
    analyzeHarmonics(prices) {
        if (prices.length < 4) return [];

        // Calculate price changes
        const changes = [];
        for (let i = 1; i < prices.length; i++) {
            changes.push((prices[i] - prices[i-1]) / prices[i-1]);
        }

        // Find dominant frequencies using simplified FFT
        const harmonics = [];
        const n = changes.length;
        const maxHarmonics = Math.min(5, Math.floor(n / 4));

        for (let k = 1; k <= maxHarmonics; k++) {
            const frequency = k / n;
            let realSum = 0;
            let imagSum = 0;

            for (let t = 0; t < n; t++) {
                const angle = 2 * Math.PI * frequency * t;
                realSum += changes[t] * Math.cos(angle);
                imagSum += changes[t] * Math.sin(angle);
            }

            const amplitude = Math.sqrt(realSum * realSum + imagSum * imagSum) / n;
            const phase = Math.atan2(imagSum, realSum);

            if (amplitude > 0.001) { // Filter out noise
                harmonics.push({ frequency, amplitude, phase });
            }
        }

        // Sort by amplitude
        harmonics.sort((a, b) => b.amplitude - a.amplitude);

        return harmonics;
    }

    /**
     * Calculate harmonic score (formula_harm_score)
     */
    calculateHarmonicScore(prices, harmonics) {
        if (harmonics.length === 0) return 0;

        // Base harmonic value
        let H = harmonics.reduce((sum, h) => sum + h.amplitude, 0) / harmonics.length;

        // Cycle uniqueness factor (simplified)
        const cycleUnique = Math.min(5, harmonics.length);
        const score = H + (5 - cycleUnique) * 2.0;

        return score;
    }

    /**
     * Calculate fractional harmonic series term (formula_fhs)
     */
    calculateFHS(k, P) {
        if (P === 0 || k === 0) return 0;

        const logP = Math.log(P);
        if (logP === 0) return 0;

        let sum = 0;
        for (let i = 1; i <= k; i++) {
            sum += 1.0 / (i * logP);
        }

        return sum;
    }

    /**
     * Calculate harmonic term for projection step
     */
    calculateHarmonicTerm(step, harmonics, harmScore, baseFreq) {
        let term = 0;

        // Sum contributions from each harmonic
        harmonics.forEach((harmonic, idx) => {
            const k = idx + 1;
            const P = baseFreq * (1 + harmonic.frequency);
            const fhs = this.calculateFHS(k, P);
            
            const angle = 2 * Math.PI * harmonic.frequency * step + harmonic.phase;
            const contribution = harmonic.amplitude * Math.cos(angle) * fhs;
            term += contribution;
        });

        // Scale by harmonic score
        return term * (harmScore / 100) * 0.01; // Scale to reasonable magnitude
    }

    trunc(x, decimals) {
        const multiplier = Math.pow(10, decimals);
        return Math.floor(x * multiplier) / multiplier;
    }
}

// Make available globally
if (typeof window !== 'undefined') {
    window.HarmonicModel = HarmonicModel;
}


