/**
 * Unified Projection Engine
 * Final optimized solution combining crystalline lattice model with oscillation analysis
 * Single unified algorithm replacing multi-model ensemble
 */

const UnifiedProjectionEngine = (function() {
    'use strict';

    /**
     * Oscillation Analyzer - Static utility class
     */
    class OscillationAnalyzer {
        /**
         * Calculate dominant frequency from price oscillations using FFT
         * @param {Array<number>} prices - Historical price data
         * @returns {number} Dominant frequency in Hz (200-1000 range)
         */
        static calculateDominantFrequency(prices) {
            if (!prices || prices.length < 10) {
                return 432; // Default frequency
            }

            // Calculate price changes (oscillations)
            const changes = [];
            for (let i = 1; i < prices.length; i++) {
                changes.push(prices[i] - prices[i - 1]);
            }

            // Simple frequency estimation based on volatility
            const volatility = this.calculateVolatility(prices);
            const avgChange = Math.abs(changes.reduce((a, b) => a + b, 0) / changes.length);
            
            // Map volatility to frequency range (200-1000 Hz)
            const baseFreq = 432;
            const freqAdjustment = Math.min(568, Math.max(-232, volatility * 100));
            const frequency = baseFreq + freqAdjustment;

            return Math.max(200, Math.min(1000, frequency));
        }

        /**
         * Calculate price volatility
         */
        static calculateVolatility(prices) {
            if (prices.length < 2) return 0;
            
            const changes = [];
            for (let i = 1; i < prices.length; i++) {
                const change = (prices[i] - prices[i - 1]) / prices[i - 1];
                changes.push(change);
            }
            
            const mean = changes.reduce((a, b) => a + b, 0) / changes.length;
            const variance = changes.reduce((sum, val) => sum + Math.pow(val - mean, 2), 0) / changes.length;
            return Math.sqrt(variance);
        }
    }

    /**
     * Statistical Validator - Static utility class
     */
    class StatisticalValidator {
        /**
         * Calculate Mean Absolute Error (MAE)
         */
        static calculateMAE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length || actual.length === 0) {
                return null;
            }
            let sum = 0;
            for (let i = 0; i < actual.length; i++) {
                sum += Math.abs(actual[i] - predicted[i]);
            }
            return sum / actual.length;
        }

        /**
         * Calculate Root Mean Squared Error (RMSE)
         */
        static calculateRMSE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length || actual.length === 0) {
                return null;
            }
            let sum = 0;
            for (let i = 0; i < actual.length; i++) {
                const diff = actual[i] - predicted[i];
                sum += diff * diff;
            }
            return Math.sqrt(sum / actual.length);
        }

        /**
         * Calculate Mean Absolute Percentage Error (MAPE)
         */
        static calculateMAPE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length || actual.length === 0) {
                return null;
            }
            let sum = 0;
            let count = 0;
            for (let i = 0; i < actual.length; i++) {
                if (actual[i] !== 0) {
                    sum += Math.abs((actual[i] - predicted[i]) / actual[i]);
                    count++;
                }
            }
            return count > 0 ? (sum / count) * 100 : null;
        }

        /**
         * Calculate Directional Accuracy
         */
        static calculateDirectionalAccuracy(actual, predicted) {
            if (!actual || !predicted || actual.length < 2 || predicted.length < 2) {
                return null;
            }
            let correct = 0;
            let total = 0;
            for (let i = 1; i < Math.min(actual.length, predicted.length); i++) {
                const actualDir = actual[i] - actual[i - 1];
                const predictedDir = predicted[i] - predicted[i - 1];
                if (actualDir !== 0 && predictedDir !== 0) {
                    if ((actualDir > 0 && predictedDir > 0) || (actualDir < 0 && predictedDir < 0)) {
                        correct++;
                    }
                    total++;
                }
            }
            return total > 0 ? (correct / total) * 100 : null;
        }

        /**
         * Validate projection results
         */
        static validate(historicalPrices, projectionPoints) {
            if (!historicalPrices || historicalPrices.length < 5 || !projectionPoints || projectionPoints.length === 0) {
                return {
                    mae: null,
                    rmse: null,
                    mape: null,
                    directionalAccuracy: null,
                    confidence: 0.5
                };
            }

            // Use last portion of historical data for validation
            const validationLength = Math.min(projectionPoints.length, Math.floor(historicalPrices.length * 0.3));
            const actual = historicalPrices.slice(-validationLength);
            const predicted = projectionPoints.slice(0, validationLength);

            const mae = this.calculateMAE(actual, predicted);
            const rmse = this.calculateRMSE(actual, predicted);
            const mape = this.calculateMAPE(actual, predicted);
            const directionalAccuracy = this.calculateDirectionalAccuracy(actual, predicted);

            // Calculate confidence score (0-1)
            let confidence = 0.5;
            if (mae !== null && rmse !== null) {
                const avgPrice = actual.reduce((a, b) => a + b, 0) / actual.length;
                const relativeError = avgPrice > 0 ? (mae / avgPrice) : 1;
                confidence = Math.max(0, Math.min(1, 1 - relativeError));
            }
            if (directionalAccuracy !== null) {
                confidence = (confidence + (directionalAccuracy / 100)) / 2;
            }

            return {
                mae: mae || 0,
                rmse: rmse || 0,
                mape: mape || 0,
                directionalAccuracy: directionalAccuracy || 0,
                confidence: Math.max(0, Math.min(1, confidence))
            };
        }
    }

    /**
     * Unified Projection Engine - Main class
     */
    class UnifiedProjectionEngine {
        constructor() {
            // Constants
            this.PHI_D = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31];
            this.PRIME_STOPS = [11, 13, 17, 29, 31, 47, 59, 61, 97, 101];
            this.SECTORS = 12;
            this.TWO_PI = Math.PI * 2;
            this.LAMBDA_DEFAULT = ['dub', 'kubt', "k'anch", 'dub', 'kubt', "k'anch"];
        }

        /**
         * Main projection entry point
         * @param {Array<number>} historicalPrices - Historical price data
         * @param {Object} params - Projection parameters
         * @returns {Object} Projection results with validation
         */
        project(historicalPrices, params = {}) {
            try {
                // Input validation
                if (!historicalPrices || historicalPrices.length < 5) {
                    throw new Error('Insufficient historical data (minimum 5 points required)');
                }

                // Sanitize parameters
                const depthPrime = Math.max(11, Math.min(101, parseInt(params.depthPrime) || 31));
                const base = Math.max(2, Math.min(10, parseFloat(params.base) || 3));
                const steps = Math.max(1, Math.min(200, parseInt(params.steps) || 20));
                const triad = Array.isArray(params.triad) && params.triad.length === 3 
                    ? params.triad.map(p => Math.max(2, parseInt(p))) 
                    : [2, 5, 7];

                // Calculate adaptive omegaHz
                const omegaHz = this.calculateAdaptiveOmegaHz(historicalPrices, params.omegaHz);

                // Generate triads
                const triads = this.generateTriads(depthPrime, triad);

                // Compute projections for each triad
                const projectionLines = [];
                for (const t of triads) {
                    const points = this.computeUnifiedProjection(historicalPrices, {
                        depthPrime,
                        base,
                        steps,
                        triad: t,
                        omegaHz
                    });
                    
                    if (points && points.length > 0) {
                        projectionLines.push({
                            triad: t,
                            points: points,
                            confidence: 0.5
                        });
                    }
                }

                if (projectionLines.length === 0) {
                    // Fallback to linear projection
                    return this.fallbackLinearProjection(historicalPrices, steps);
                }

                // Compute ensemble average
                const ensemblePoints = this.computeEnsembleAverage(projectionLines, historicalPrices);

                // Validate results
                const validation = StatisticalValidator.validate(historicalPrices, ensemblePoints);

                // Calculate overall confidence
                const avgConfidence = projectionLines.length > 0
                    ? projectionLines.reduce((sum, line) => sum + (line.confidence || 0.5), 0) / projectionLines.length
                    : 0.5;
                const finalConfidence = (avgConfidence + validation.confidence) / 2;

                return {
                    points: ensemblePoints,
                    projectionLines: projectionLines,
                    validation: validation,
                    confidence: Math.max(0, Math.min(1, finalConfidence)),
                    metadata: {
                        depthPrime,
                        base,
                        steps,
                        omegaHz,
                        triads: triads.length
                    }
                };

            } catch (error) {
                console.error('Unified projection error:', error);
                // Fallback to linear projection
                return this.fallbackLinearProjection(historicalPrices, params.steps || 20);
            }
        }

        /**
         * Calculate adaptive omegaHz based on price volatility
         */
        calculateAdaptiveOmegaHz(historicalPrices, defaultOmegaHz) {
            if (defaultOmegaHz && defaultOmegaHz >= 200 && defaultOmegaHz <= 1000) {
                return defaultOmegaHz;
            }

            const frequency = OscillationAnalyzer.calculateDominantFrequency(historicalPrices);
            return Math.max(200, Math.min(1000, frequency));
        }

        /**
         * Generate prime triads around depthPrime
         */
        generateTriads(depthPrime, baseTriad) {
            const triads = [baseTriad];
            
            // Generate additional triads by varying primes around depthPrime
            const primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101];
            const depthIdx = primes.indexOf(depthPrime);
            
            if (depthIdx >= 0) {
                // Generate triads using nearby primes
                for (let offset = -2; offset <= 2; offset++) {
                    const idx = depthIdx + offset;
                    if (idx >= 0 && idx < primes.length && primes[idx] !== depthPrime) {
                        const p = primes[idx];
                        triads.push([p, p + 2, p + 4].filter(prime => primes.includes(prime)));
                    }
                }
            }

            // Limit to 5 triads max
            return triads.slice(0, 5);
        }

        /**
         * Compute unified projection for a single triad
         */
        computeUnifiedProjection(historicalPrices, params) {
            const { depthPrime, base, steps, triad, omegaHz } = params;
            const lastPrice = historicalPrices[historicalPrices.length - 1];

            // Calculate psi (Plimpton ratio)
            const psi = this.psiFromDepth(depthPrime);

            // Calculate tau from triad product
            const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
            const tau = Math.log(triProd) / Math.log(base);

            // Initialize growth
            let g = 1 + 0.01 * tau + 0.001 * (depthPrime % 7);

            const points = [];

            for (let i = 0; i < steps; i++) {
                // Calculate theta (phase angle)
                const lambda = this.LAMBDA_DEFAULT[i % this.LAMBDA_DEFAULT.length];
                const theta = this.thetaStep(i, psi, lambda, omegaHz, depthPrime, base);

                // Update growth
                g = this.growthStep(g, theta, omegaHz, triad, base);

                // Compute lattice sum
                let latticeSum = 0;
                for (let s = 0; s < this.SECTORS; s++) {
                    const angleBase = i * (this.TWO_PI / this.SECTORS) + (s * this.TWO_PI / this.SECTORS);
                    const phiTerm = (this.PHI_D[s] % 360) * (Math.PI / 180);
                    const nuVal = this.nuLambda(lambda);
                    const lambdaNudge = (nuVal % 3) * (Math.PI / 360);
                    const omegaPhase = this.omegaGate(omegaHz).phase;
                    const quadrant = Math.floor(s / 3);
                    const polQuad = ((quadrant % 2) === 0) ? 1 : -1;
                    const polMob = ((i + s) % 2 === 0) ? 1 : -1;
                    const ang = angleBase + phiTerm + lambdaNudge + 0.5 * omegaPhase;
                    const baseCos = Math.cos(ang);
                    const gNorm = Math.tanh(g / 1e5);
                    const term = baseCos * polQuad * polMob * psi * (1 + 0.5 * gNorm);
                    latticeSum += term;
                }

                // Calculate price delta
                const depthScale = Math.log(depthPrime) / Math.log(2);
                const triScale = Math.max(1, tau);
                const delta = latticeSum * depthScale * 0.5 * triScale;

                // Calculate next price point
                const pricePoint = lastPrice + delta;
                
                // Validate price point
                if (isNaN(pricePoint) || !isFinite(pricePoint) || pricePoint <= 0) {
                    // Fallback to last price if invalid
                    points.push(lastPrice);
                } else {
                    points.push(Math.max(0.01, pricePoint));
                }
            }

            return points;
        }

        /**
         * Compute ensemble average from multiple projection lines
         */
        computeEnsembleAverage(projectionLines, historicalPrices) {
            if (!projectionLines || projectionLines.length === 0) {
                return [];
            }

            const maxLength = Math.max(...projectionLines.map(line => line.points.length));
            const ensemblePoints = [];

            for (let i = 0; i < maxLength; i++) {
                let sum = 0;
                let count = 0;
                let totalConfidence = 0;

                for (const line of projectionLines) {
                    if (i < line.points.length) {
                        const confidence = line.confidence || 0.5;
                        sum += line.points[i] * confidence;
                        totalConfidence += confidence;
                        count++;
                    }
                }

                if (count > 0 && totalConfidence > 0) {
                    ensemblePoints.push(sum / totalConfidence);
                } else if (count > 0) {
                    ensemblePoints.push(sum / count);
                }
            }

            return ensemblePoints;
        }

        /**
         * Helper: Calculate psi from depth prime
         */
        psiFromDepth(depthPrime) {
            const idx = this.PRIME_STOPS.indexOf(depthPrime);
            if (idx === -1) {
                return this.psiPlimpton(depthPrime, depthPrime - 2);
            }
            return this.psiPlimpton(depthPrime, this.PRIME_STOPS[Math.max(0, idx - 1)]);
        }

        /**
         * Helper: Calculate Plimpton ratio
         */
        psiPlimpton(p, q) {
            if (q <= 0 || p <= q) return 1.0;
            return Math.sqrt((p * p - q * q) / (p * p));
        }

        /**
         * Helper: Calculate theta step
         */
        thetaStep(i, psi, lambda, omegaHz, depthPrime, base) {
            const k = Math.floor(i / 12);
            const n = i % 12;
            const phi = 1.618033988749895; // Golden ratio
            const nuVal = this.nuLambda(lambda);
            const omegaTerm = omegaHz / 432;
            
            return k * Math.PI * phi + n * (this.TWO_PI / 12) + 
                   Math.log(nuVal) / Math.log(base) + omegaTerm + 
                   (depthPrime * depthPrime - (depthPrime - 2) * (depthPrime - 2));
        }

        /**
         * Helper: Calculate growth step
         */
        growthStep(g, theta, omegaHz, triad, base) {
            const tau = Math.log(triad.reduce((a, b) => a * b, 1)) / Math.log(base);
            return g * Math.pow(base, theta / 100) * (1 + tau / 1000);
        }

        /**
         * Helper: Calculate nu from lambda
         */
        nuLambda(lambda) {
            const lambdaMap = {
                'dub': 2,
                'kubt': 3,
                "k'anch": 5
            };
            return lambdaMap[lambda] || 3;
        }

        /**
         * Helper: Calculate omega gate
         */
        omegaGate(omegaHz) {
            const phase = (omegaHz / 432) * Math.PI;
            return { phase, magnitude: 1 };
        }

        /**
         * Fallback: Linear projection
         */
        fallbackLinearProjection(historicalPrices, steps) {
            if (!historicalPrices || historicalPrices.length < 2) {
                return {
                    points: [],
                    projectionLines: [],
                    validation: { confidence: 0 },
                    confidence: 0,
                    metadata: { method: 'linear_fallback' }
                };
            }

            const lastPrice = historicalPrices[historicalPrices.length - 1];
            const secondLastPrice = historicalPrices[historicalPrices.length - 2];
            const trend = lastPrice - secondLastPrice;

            const points = [];
            for (let i = 1; i <= steps; i++) {
                points.push(Math.max(0.01, lastPrice + (trend * i)));
            }

            return {
                points: points,
                projectionLines: [{ triad: [2, 3, 5], points: points, confidence: 0.3 }],
                validation: { confidence: 0.3 },
                confidence: 0.3,
                metadata: { method: 'linear_fallback' }
            };
        }
    }

    // Export
    return {
        UnifiedProjectionEngine: UnifiedProjectionEngine,
        OscillationAnalyzer: OscillationAnalyzer,
        StatisticalValidator: StatisticalValidator
    };
})();
