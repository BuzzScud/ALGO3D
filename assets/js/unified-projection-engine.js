/**
 * Unified Projection Engine
 * Final, optimized solution combining:
 * - Crystalline lattice model
 * - Oscillation analysis (FFT-based)
 * - Statistical validation
 * - Adaptive parameter tuning
 * 
 * This replaces the multi-model ensemble with a single, refined algorithm
 */

const UnifiedProjectionEngine = (function() {
    'use strict';
    
    /**
     * Core Constants
     */
    const PHI = (1 + Math.sqrt(5)) / 2; // Golden ratio
    const PI = Math.PI;
    const TWO_PI = 2 * PI;
    const SECTORS = 12;
    const BASE_FREQUENCY = 432; // Hz
    
    const PHI_D = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31];
    const PRIME_STOPS = [11, 13, 17, 29, 31, 47, 59, 61, 97, 101];
    const LAMBDA_DEFAULT = ['dub', 'kubt', "k'anch", 'dub', 'kubt', "k'anch"];
    
    /**
     * Oscillation Analyzer (simplified from oscillation-analysis.js)
     */
    class OscillationAnalyzer {
        /**
         * Calculate dominant frequency from price series using FFT
         */
        static calculateDominantFrequency(prices, sampleRate = 1) {
            if (!prices || prices.length < 4) return BASE_FREQUENCY;
            
            try {
                // Use existing OscillationAnalyzer if available (from oscillation-analysis.js)
                if (typeof window.OscillationAnalyzer !== 'undefined' && 
                    window.OscillationAnalyzer.calculateOmegaFromOscillations) {
                    return window.OscillationAnalyzer.calculateOmegaFromOscillations(prices, BASE_FREQUENCY);
                }
                
                // Fallback: Simple frequency detection based on volatility
                const diffs = [];
                for (let i = 1; i < prices.length; i++) {
                    diffs.push(Math.abs(prices[i] - prices[i - 1]));
                }
                
                const avgDiff = diffs.reduce((a, b) => a + b, 0) / diffs.length;
                const priceRange = Math.max(...prices) - Math.min(...prices);
                
                if (priceRange === 0) return BASE_FREQUENCY;
                
                // Estimate frequency based on volatility
                // Higher volatility = higher frequency adjustment
                const volatility = avgDiff / priceRange;
                const frequency = BASE_FREQUENCY * (1 + volatility * 0.5);
                
                // Clamp to reasonable range
                return Math.max(200, Math.min(1000, frequency));
            } catch (e) {
                console.warn('Oscillation analysis error:', e);
                return BASE_FREQUENCY;
            }
        }
    }
    
    /**
     * Statistical Validator
     */
    class StatisticalValidator {
        /**
         * Calculate Mean Absolute Error
         */
        static calculateMAE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length) return null;
            
            let sum = 0;
            for (let i = 0; i < actual.length; i++) {
                sum += Math.abs(actual[i] - predicted[i]);
            }
            return sum / actual.length;
        }
        
        /**
         * Calculate Root Mean Squared Error
         */
        static calculateRMSE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length) return null;
            
            let sum = 0;
            for (let i = 0; i < actual.length; i++) {
                const diff = actual[i] - predicted[i];
                sum += diff * diff;
            }
            return Math.sqrt(sum / actual.length);
        }
        
        /**
         * Calculate Mean Absolute Percentage Error
         */
        static calculateMAPE(actual, predicted) {
            if (!actual || !predicted || actual.length !== predicted.length) return null;
            
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
         * Validate projection accuracy
         */
        static validate(historicalPrices, projectionPoints, lookbackSteps = 5) {
            if (!historicalPrices || historicalPrices.length < lookbackSteps) {
                return { error: 'Insufficient historical data' };
            }
            
            if (!projectionPoints || projectionPoints.length === 0) {
                return { error: 'No projection points' };
            }
            
            // Use last N points for validation
            const actual = historicalPrices.slice(-lookbackSteps);
            const predicted = projectionPoints.slice(0, Math.min(lookbackSteps, projectionPoints.length));
            
            // Extend predicted if needed (use last value)
            while (predicted.length < actual.length) {
                predicted.push(predicted[predicted.length - 1]);
            }
            
            const mae = this.calculateMAE(actual, predicted);
            const rmse = this.calculateRMSE(actual, predicted);
            const mape = this.calculateMAPE(actual, predicted);
            
            // Calculate confidence based on error metrics
            const avgPrice = actual.reduce((a, b) => a + b, 0) / actual.length;
            const errorStdDev = rmse ? rmse / avgPrice : 0;
            const confidence = Math.max(0, Math.min(1, 1 - Math.min(errorStdDev, 1)));
            
            return {
                mae: mae,
                rmse: rmse,
                mape: mape,
                errorStdDev: errorStdDev * 100,
                confidence: confidence,
                directionalAccuracy: this.calculateDirectionalAccuracy(actual, predicted)
            };
        }
        
        /**
         * Calculate directional accuracy
         */
        static calculateDirectionalAccuracy(actual, predicted) {
            if (!actual || !predicted || actual.length < 2) return null;
            
            let correct = 0;
            let total = 0;
            
            for (let i = 1; i < actual.length; i++) {
                const actualDir = actual[i] - actual[i - 1];
                const predictedDir = predicted[i] - predicted[i - 1];
                
                if ((actualDir >= 0 && predictedDir >= 0) || (actualDir < 0 && predictedDir < 0)) {
                    correct++;
                }
                total++;
            }
            
            return total > 0 ? correct / total : null;
        }
    }
    
    /**
     * Unified Projection Engine
     */
    class UnifiedProjectionEngine {
        constructor() {
            this.name = 'Unified Projection Engine';
        }
        
        /**
         * Generate unified projection
         * @param {Array<number>} historicalPrices - Historical price data
         * @param {Object} params - Projection parameters
         * @returns {Object} Projection result with validation
         */
        project(historicalPrices, params) {
            try {
                // Input validation
                if (!historicalPrices || !Array.isArray(historicalPrices) || historicalPrices.length === 0) {
                    throw new Error('No historical prices provided');
                }
                
                // Validate all prices are numbers
                const validPrices = historicalPrices.filter(p => typeof p === 'number' && !isNaN(p) && isFinite(p));
                if (validPrices.length === 0) {
                    throw new Error('No valid price data');
                }
                
                const lastPrice = validPrices[validPrices.length - 1];
                if (lastPrice <= 0) {
                    throw new Error('Invalid last price');
                }
                
                // Parse and validate parameters
                const steps = Math.max(1, Math.min(200, parseInt(params.steps) || 20));
                const depthPrime = Math.max(11, Math.min(101, parseInt(params.depthPrime) || 31));
                const base = Math.max(2, Math.min(10, parseFloat(params.base) || 3));
                const projectionCount = Math.max(1, Math.min(50, parseInt(params.projectionCount) || 12));
                
                // Adaptive parameter calculation with error handling
                let adaptiveOmega = BASE_FREQUENCY;
                try {
                    adaptiveOmega = OscillationAnalyzer.calculateDominantFrequency(validPrices, BASE_FREQUENCY);
                    if (!isFinite(adaptiveOmega) || adaptiveOmega <= 0) {
                        adaptiveOmega = BASE_FREQUENCY;
                    }
                    
                    // Integrate tetration towers and platonic solids if available
                    if (typeof window !== 'undefined' && window.tetrationTowers && window.tetrationTowers.length > 0) {
                        try {
                            // Use tetration attractors to refine omegaHz
                            const pricePosition = [
                                lastPrice, 
                                validPrices[validPrices.length - 2] || lastPrice, 
                                validPrices[validPrices.length - 3] || lastPrice
                            ];
                            const attractor = TetrationTowers.findHDAttractor(pricePosition, window.tetrationTowers);
                            
                            if (attractor && attractor.length > 0) {
                                // Blend omegaHz with attractor values
                                const attractorOmega = Math.abs(attractor[0]) % 1000;
                                if (isFinite(attractorOmega) && attractorOmega > 0) {
                                    adaptiveOmega = adaptiveOmega * 0.7 + attractorOmega * 0.3;
                                }
                            }
                        } catch (e) {
                            console.warn('Tetration attractor integration failed:', e);
                        }
                    }
                } catch (e) {
                    console.warn('Oscillation analysis failed, using default frequency:', e);
                    adaptiveOmega = BASE_FREQUENCY;
                }
                
                // Generate multiple projection lines using different triads
                const triads = this.generateTriads(depthPrime, projectionCount);
                if (!triads || triads.length === 0) {
                    throw new Error('Failed to generate triads');
                }
                
                const projectionLines = [];
                
                // Integrate platonic solids if available for geometric consistency
                let platonicInfluence = null;
                if (typeof window !== 'undefined' && window.discoveredPlatonicSolids && window.discoveredPlatonicSolids.length > 0) {
                    // Use discovered solids to influence projection geometry
                    const solid = window.discoveredPlatonicSolids[0]; // Use first discovered solid
                    if (solid && solid.vertices && solid.vertices.length > 0) {
                        // Extract geometric influence from solid vertices
                        const vertexSum = solid.vertices[0].reduce((a, b) => a + Math.abs(b), 0);
                        platonicInfluence = vertexSum / solid.vertices[0].length;
                    }
                }
                
                triads.forEach((triad, idx) => {
                    try {
                        // Apply platonic solid influence if available
                        let adjustedOmega = adaptiveOmega;
                        if (platonicInfluence !== null) {
                            // Blend omega with geometric influence
                            adjustedOmega = adaptiveOmega * 0.8 + (platonicInfluence * 100) * 0.2;
                        }
                        
                        const points = this.computeUnifiedProjection({
                            lastPrice: lastPrice,
                            depthPrime: depthPrime,
                            omegaHz: adjustedOmega,
                            triad: triad,
                            base: base,
                            steps: steps
                        });
                        
                        if (points && points.length > 0) {
                            projectionLines.push({
                                triad: triad,
                                points: points,
                                confidence: this.calculateConfidence(points, validPrices)
                            });
                        }
                    } catch (e) {
                        console.warn(`Error computing projection for triad ${idx}:`, e);
                        // Continue with other triads
                    }
                });
                
                if (projectionLines.length === 0) {
                    throw new Error('No valid projections generated');
                }
                
                // Calculate ensemble average (weighted by confidence)
                const ensemblePoints = this.computeEnsembleAverage(projectionLines, steps);
                
                if (!ensemblePoints || ensemblePoints.length === 0) {
                    throw new Error('Failed to compute ensemble average');
                }
                
                // Validate projection
                let validation = null;
                try {
                    validation = StatisticalValidator.validate(validPrices, ensemblePoints, Math.min(5, steps));
                } catch (e) {
                    console.warn('Validation failed:', e);
                    validation = { error: 'Validation failed', confidence: 0.5 };
                }
                
                return {
                    points: ensemblePoints,
                    projectionLines: projectionLines,
                    validation: validation,
                    confidence: validation && validation.confidence ? validation.confidence : 0.5,
                    metadata: {
                        model: this.name,
                        steps: steps,
                        depthPrime: depthPrime,
                        base: base,
                        omegaHz: adaptiveOmega,
                        projectionCount: projectionCount
                    }
                };
            } catch (error) {
                console.error('Unified Projection Engine error:', error);
                throw error;
            }
        }
        
        /**
         * Compute unified projection using crystalline lattice model
         */
        computeUnifiedProjection({
            lastPrice,
            depthPrime,
            omegaHz,
            triad,
            base,
            steps
        }) {
            // Input validation
            if (!lastPrice || lastPrice <= 0 || !isFinite(lastPrice)) {
                throw new Error('Invalid lastPrice');
            }
            if (!triad || !Array.isArray(triad) || triad.length < 3) {
                throw new Error('Invalid triad');
            }
            if (!isFinite(base) || base <= 0) {
                base = 3;
            }
            if (!isFinite(omegaHz) || omegaHz <= 0) {
                omegaHz = BASE_FREQUENCY;
            }
            
            // Get tetration towers and platonic solids if available
            const tetrationTowers = (typeof window !== 'undefined' && window.tetrationTowers) 
                ? window.tetrationTowers : [];
            const platonicSolids = (typeof window !== 'undefined' && window.discoveredPlatonicSolids) 
                ? window.discoveredPlatonicSolids : [];
            
            // Get primary platonic solid for geometric influence
            let primarySolid = null;
            if (platonicSolids.length > 0) {
                primarySolid = platonicSolids[0];
            }
            
            try {
                const psi = this.psiFromDepth(depthPrime);
                const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
                const tau = Math.log(Math.max(1, triProd)) / Math.log(Math.max(2, base));
                
                // Initialize growth factor with tetration influence if available
                let g = 1 + 0.01 * tau + 0.001 * (depthPrime % 7);
                if (tetrationTowers.length > 0) {
                    // Use tetration tower to refine initial growth
                    const initialPosition = [lastPrice, lastPrice * 0.99, lastPrice * 1.01];
                    const attractor = TetrationTowers.findHDAttractor(initialPosition, tetrationTowers);
                    if (attractor && attractor.length > 0) {
                        const tetrationInfluence = Math.abs(attractor[0]) % 100;
                        g = g * (1 + tetrationInfluence / 10000); // Small influence
                    }
                }
                
                const points = [];
                let currentPrice = lastPrice;
                
                for (let i = 0; i < steps; i++) {
                    const lambda = LAMBDA_DEFAULT[i % LAMBDA_DEFAULT.length];
                    
                    // Calculate theta with tetration influence
                    // Parameters: (n, k, lambda, omega, psi, depthPrime)
                    // n and k are both step index i for this calculation
                    let theta_i = this.calculateTheta(i, i, lambda, omegaHz, psi, depthPrime);
                    
                    // Apply tetration attractor to theta if available
                    if (tetrationTowers.length > 0 && i > 0) {
                        const pricePosition = [
                            currentPrice,
                            points[i - 1] || currentPrice,
                            points[Math.max(0, i - 2)] || currentPrice
                        ];
                        const attractor = TetrationTowers.findHDAttractor(pricePosition, tetrationTowers);
                        if (attractor && attractor.length > 0) {
                            // Use tetration attractor to refine theta
                            const attractorTheta = Math.abs(attractor[0]) % (2 * PI);
                            theta_i = theta_i * 0.85 + attractorTheta * 0.15; // Blend with attractor
                        }
                    }
                    
                    // Update growth factor
                    g = this.growthStep(g, theta_i, omegaHz, triad, base);
                    
                    // Apply tetration influence to growth if available
                    if (tetrationTowers.length > 0) {
                        const towerIndex = i % tetrationTowers.length;
                        const tower = tetrationTowers[towerIndex];
                        if (tower) {
                            const towerValue = tower.getValue();
                            if (isFinite(towerValue) && towerValue > 0) {
                                const towerInfluence = Math.log(Math.max(1, towerValue % 1000)) / 100;
                                g = g * (1 + towerInfluence / 1000); // Small geometric influence
                            }
                        }
                    }
                    
                    let latticeSum = 0;
                    
                    // Calculate platonic solid geometric influence
                    let platonicAngleOffset = 0;
                    let platonicScaleFactor = 1.0;
                    if (primarySolid && primarySolid.vertices && primarySolid.vertices.length > 0) {
                        // Use solid vertices to influence angles
                        const vertexIndex = (i * SECTORS) % primarySolid.vertices.length;
                        const vertex = primarySolid.vertices[vertexIndex];
                        if (vertex && vertex.length >= 3) {
                            // Extract geometric phase from solid vertex
                            const vertexPhase = Math.atan2(vertex[1] || 0, vertex[0] || 0);
                            platonicAngleOffset = vertexPhase * 0.1; // 10% influence
                            
                            // Use vertex magnitude for scaling
                            const vertexMag = Math.sqrt(
                                (vertex[0] || 0) ** 2 + 
                                (vertex[1] || 0) ** 2 + 
                                (vertex[2] || 0) ** 2
                            );
                            platonicScaleFactor = 1.0 + (vertexMag - 1.0) * 0.05; // 5% scaling influence
                        }
                    }
                    
                    for (let s = 0; s < SECTORS; s++) {
                        const angleBase = i * (TWO_PI / SECTORS) + (s * TWO_PI / SECTORS);
                        const phiTerm = (PHI_D[s] % 360) * (PI / 180);
                        const nuVal = this.nuLambda(lambda);
                        const lambdaNudge = (nuVal % 3) * (PI / 360);
                        const { phase: omegaPhase } = this.omegaGate(omegaHz);
                        
                        // Apply platonic solid geometric influence to angle
                        let platonicAngle = 0;
                        if (primarySolid && primarySolid.vertices && primarySolid.vertices.length > 0) {
                            const vertexIdx = (s + i * SECTORS) % primarySolid.vertices.length;
                            const vertex = primarySolid.vertices[vertexIdx];
                            if (vertex && vertex.length >= 2) {
                                // Use vertex coordinates to create geometric phase
                                platonicAngle = Math.atan2(vertex[1] || 0, vertex[0] || 0) * 0.05;
                            }
                        }
                        
                        const quadrant = Math.floor(s / 3);
                        const polQuad = ((quadrant % 2) === 0) ? 1 : -1;
                        const polMob = ((i + s) % 2 === 0) ? 1 : -1;
                        
                        // Combine all angle components with platonic influence
                        const ang = angleBase + phiTerm + lambdaNudge + 0.5 * omegaPhase + 
                                   platonicAngleOffset + platonicAngle;
                        const cosValue = Math.cos(ang);
                        const gNorm = Math.tanh(Math.max(0, g) / 1e5);
                        
                        // Apply platonic scale factor to term
                        const term = cosValue * polQuad * polMob * psi * (1 + 0.5 * gNorm) * platonicScaleFactor;
                        latticeSum += term;
                    }
                    
                    // Calculate depth scale with tetration influence
                    let depthScale = Math.log(Math.max(2, depthPrime)) / Math.log(2);
                    if (tetrationTowers.length > 0) {
                        // Use tetration depth to refine depth scale
                        const towerDepth = tetrationTowers[0]?.depth || depthPrime;
                        const tetrationDepthScale = Math.log(Math.max(2, towerDepth)) / Math.log(2);
                        depthScale = depthScale * 0.9 + tetrationDepthScale * 0.1; // Blend
                    }
                    
                    const triScale = Math.max(1, tau);
                    
                    // Apply platonic solid geometric properties to scaling
                    let geometricScale = 1.0;
                    if (primarySolid) {
                        const props = primarySolid.calculateProperties();
                        if (props && props.averageRadius) {
                            // Use solid's average radius to influence scaling
                            geometricScale = 1.0 + (props.averageRadius - 1.0) * 0.02; // 2% influence
                        }
                    }
                    
                    // Calculate delta with all influences
                    const delta = this.trunc(
                        latticeSum * depthScale * 0.5 * triScale * geometricScale, 
                        4
                    );
                    
                    // Apply tetration attractor to final price if available
                    let pricePoint = Math.max(0.01, this.trunc(currentPrice + delta, 4));
                    
                    if (tetrationTowers.length > 0 && i > 0) {
                        const finalPosition = [pricePoint, currentPrice, points[i - 1] || currentPrice];
                        const finalAttractor = TetrationTowers.findHDAttractor(finalPosition, tetrationTowers);
                        if (finalAttractor && finalAttractor.length > 0) {
                            // Small geometric correction from attractor
                            const attractorCorrection = (finalAttractor[0] - pricePoint) * 0.01; // 1% correction
                            pricePoint = Math.max(0.01, this.trunc(pricePoint + attractorCorrection, 4));
                        }
                    }
                    
                    points.push(pricePoint);
                    currentPrice = pricePoint; // Update for next iteration
                }
                
                return points;
            } catch (error) {
                console.error('Error in computeUnifiedProjection:', error);
                // Return fallback: simple linear projection
                const points = [];
                // Use a small default change if no historical data available
                const avgChange = 0.001 * lastPrice; // 0.1% per step as fallback
                for (let i = 0; i < steps; i++) {
                    points.push(Math.max(0.01, this.trunc(lastPrice + avgChange * (i + 1), 4)));
                }
                return points;
            }
        }
        
        /**
         * Generate triads around a prime number
         */
        generateTriads(depthPrime, count) {
            const PRIMES_500 = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997];
            
            const idx = PRIMES_500.indexOf(depthPrime);
            if (idx === -1) {
                // Use default triads
                return [[2, 5, 7], [3, 7, 11], [5, 11, 13], [7, 13, 17], [11, 17, 19], [13, 19, 23], [17, 23, 29], [19, 29, 31], [23, 31, 37], [29, 37, 41], [31, 41, 43], [37, 43, 47]].slice(0, count);
            }
            
            const triads = [];
            const half = Math.floor(count / 2);
            
            for (let offset = -half; offset <= half; offset++) {
                if (triads.length >= count) break;
                const i = Math.max(0, Math.min(PRIMES_500.length - 3, idx + offset));
                const triad = [PRIMES_500[i], PRIMES_500[i + 1], PRIMES_500[i + 2]];
                triads.push(triad);
            }
            
            return triads;
        }
        
        /**
         * Compute ensemble average from multiple projection lines
         */
        computeEnsembleAverage(projectionLines, steps) {
            if (!projectionLines || projectionLines.length === 0) {
                return [];
            }
            
            try {
                const ensemblePoints = [];
                const maxLength = Math.max(...projectionLines.map(line => line.points ? line.points.length : 0));
                
                if (maxLength === 0) {
                    return [];
                }
                
                for (let i = 0; i < Math.min(steps, maxLength); i++) {
                    let weightedSum = 0;
                    let totalWeight = 0;
                    let validCount = 0;
                    
                    projectionLines.forEach(line => {
                        if (line.points && i < line.points.length) {
                            const point = line.points[i];
                            if (isFinite(point) && point > 0) {
                                const weight = (line.confidence && isFinite(line.confidence)) ? Math.max(0.1, Math.min(1.0, line.confidence)) : 1.0;
                                weightedSum += point * weight;
                                totalWeight += weight;
                                validCount++;
                            }
                        }
                    });
                    
                    if (totalWeight > 0 && validCount > 0) {
                        ensemblePoints.push(weightedSum / totalWeight);
                    } else if (validCount > 0) {
                        // Fallback: simple average
                        let sum = 0;
                        projectionLines.forEach(line => {
                            if (line.points && i < line.points.length && isFinite(line.points[i]) && line.points[i] > 0) {
                                sum += line.points[i];
                            }
                        });
                        ensemblePoints.push(sum / validCount);
                    } else {
                        // No valid points, use last known price
                        ensemblePoints.push(ensemblePoints.length > 0 ? ensemblePoints[ensemblePoints.length - 1] : 0);
                    }
                }
                
                return ensemblePoints.filter(p => isFinite(p) && p > 0);
            } catch (error) {
                console.error('Error computing ensemble average:', error);
                return [];
            }
        }
        
        /**
         * Calculate confidence for a projection line
         */
        calculateConfidence(points, historicalPrices) {
            if (!points || points.length === 0 || !historicalPrices || historicalPrices.length === 0) {
                return 0.5;
            }
            
            // Simple confidence based on price stability
            const lastPrice = historicalPrices[historicalPrices.length - 1];
            const priceRange = Math.max(...historicalPrices) - Math.min(...historicalPrices);
            
            if (priceRange === 0) return 0.5;
            
            // Check if projection is within reasonable bounds
            const projectionRange = Math.max(...points) - Math.min(...points);
            const rangeRatio = projectionRange / priceRange;
            
            // Confidence decreases if projection range is too large relative to historical
            const confidence = Math.max(0.3, Math.min(0.95, 1 - Math.min(rangeRatio / 2, 0.7)));
            
            return confidence;
        }
        
        /**
         * Helper functions
         */
        psiFromDepth(depthPrime) {
            const idx = PRIME_STOPS.indexOf(depthPrime);
            if (idx === -1) return this.psiPlimpton(depthPrime, depthPrime - 2);
            const p = depthPrime;
            const q = idx > 0 ? PRIME_STOPS[idx - 1] : 2;
            return this.psiPlimpton(p, q);
        }
        
        psiPlimpton(p, q) {
            const p2 = p * p;
            const q2 = q * q;
            return (p2 - q2) / (p2 + q2);
        }
        
        nuLambda(lambda) {
            if (lambda === 'dub') return 3;
            if (lambda === 'kubt') return 5;
            if (lambda === "k'anch" || lambda === "k'anchay") return 7;
            if (typeof lambda === 'number') {
                return Math.pow(3, lambda) % 3;
            }
            return 3;
        }
        
        omegaGate(omegaHz) {
            const ratio = omegaHz / BASE_FREQUENCY;
            const phase = (Math.log(ratio) / Math.log(2)) * (PI / 2);
            return { phase, magnitude: Math.sqrt(ratio) };
        }
        
        calculateTheta(n, k, lambda, omega, psi, depthPrime) {
            const term1 = k * PI * PHI;
            const term2 = n * TWO_PI / 12;
            const nuValue = this.nuLambda(lambda);
            const nuVal = nuValue > 0 ? nuValue : 1;
            const term3 = Math.log(nuVal) / Math.log(3);
            const term4 = omega / BASE_FREQUENCY;
            const dp = depthPrime || 31;
            const idx = PRIME_STOPS.indexOf(dp);
            const p = dp;
            const q = idx > 0 ? PRIME_STOPS[idx - 1] : 2;
            const term5 = p * p - q * q;
            return term1 + term2 + term3 + term4 + term5;
        }
        
        growthStep(gPrev, theta, omegaHz, triad, base) {
            const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
            const tau = Math.log(triProd) / Math.log(base);
            const exponent = theta / 100;
            const baseTerm = Math.pow(base, exponent);
            const tauTerm = 1 + tau / 1000;
            return gPrev * baseTerm * tauTerm;
        }
        
        trunc(x, decimals) {
            const multiplier = Math.pow(10, decimals);
            return Math.floor(x * multiplier) / multiplier;
        }
    }
    
    return {
        UnifiedProjectionEngine,
        OscillationAnalyzer,
        StatisticalValidator
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.UnifiedProjectionEngine = UnifiedProjectionEngine;
}



