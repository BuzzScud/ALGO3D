/**
 * Projection Validation Framework
 * Provides statistical validation, accuracy metrics, and confidence intervals
 */

const ProjectionValidator = (function() {
    'use strict';

    /**
     * Calculate Mean Absolute Error (MAE)
     */
    function calculateMAE(actual, predicted) {
        if (!actual || !predicted || actual.length !== predicted.length) {
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
    function calculateRMSE(actual, predicted) {
        if (!actual || !predicted || actual.length !== predicted.length) {
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
    function calculateMAPE(actual, predicted) {
        if (!actual || !predicted || actual.length !== predicted.length) {
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
     * Returns percentage of correct direction predictions
     */
    function calculateDirectionalAccuracy(actual, predicted) {
        if (!actual || !predicted || actual.length < 2 || predicted.length < 2) {
            return null;
        }

        let correct = 0;
        let total = 0;

        for (let i = 1; i < actual.length; i++) {
            const actualDir = actual[i] - actual[i - 1];
            const predictedDir = predicted[i] - predicted[i - 1];

            if (actualDir !== 0 && predictedDir !== 0) {
                if ((actualDir > 0 && predictedDir > 0) || 
                    (actualDir < 0 && predictedDir < 0)) {
                    correct++;
                }
                total++;
            }
        }

        return total > 0 ? (correct / total) * 100 : null;
    }

    /**
     * Calculate standard deviation
     */
    function calculateStdDev(values) {
        if (!values || values.length === 0) return 0;

        const mean = values.reduce((a, b) => a + b, 0) / values.length;
        const variance = values.reduce((sum, val) => sum + Math.pow(val - mean, 2), 0) / values.length;
        return Math.sqrt(variance);
    }

    /**
     * Calculate prediction intervals using normal distribution
     * @param {number} predicted - Predicted value
     * @param {number} stdDev - Standard deviation of errors
     * @param {number} confidence - Confidence level (0.95 for 95%)
     * @returns {Object} Lower and upper bounds
     */
    function calculatePredictionInterval(predicted, stdDev, confidence = 0.95) {
        // Z-score for confidence level
        const zScores = {
            0.90: 1.645,
            0.95: 1.96,
            0.99: 2.576
        };
        const z = zScores[confidence] || 1.96;

        const margin = z * stdDev;
        return {
            lower: predicted - margin,
            upper: predicted + margin,
            confidence: confidence
        };
    }

    /**
     * Perform backtesting
     * @param {Array} historicalPrices - Full historical price data
     * @param {Function} projectionFunction - Function that generates projections
     * @param {Object} params - Parameters for projection function
     * @param {number} trainRatio - Ratio of data for training (default 0.8)
     * @returns {Object} Backtest results
     */
    function backtest(historicalPrices, projectionFunction, params, trainRatio = 0.8) {
        if (!historicalPrices || historicalPrices.length < 10) {
            return { error: 'Insufficient historical data' };
        }

        const splitIndex = Math.floor(historicalPrices.length * trainRatio);
        const trainData = historicalPrices.slice(0, splitIndex);
        const testData = historicalPrices.slice(splitIndex);

        // Generate projections from training data
        const projections = projectionFunction(trainData, params);
        
        // Extract predicted prices (use last point of each projection line)
        const predicted = [];
        const actual = [];

        // For each projection line, compare final predicted value with actual
        if (projections && projections.length > 0) {
            // Use average of all projection lines
            const avgProjection = projections.map(p => p.points[p.points.length - 1])
                .reduce((a, b) => a + b, 0) / projections.length;
            
            // Compare with actual future prices
            const testSteps = Math.min(testData.length, params.steps || 20);
            for (let i = 0; i < testSteps && i < testData.length; i++) {
                predicted.push(avgProjection);
                actual.push(testData[i]);
            }
        }

        if (predicted.length === 0) {
            return { error: 'No predictions generated' };
        }

        // Calculate metrics
        const mae = calculateMAE(actual, predicted);
        const rmse = calculateRMSE(actual, predicted);
        const mape = calculateMAPE(actual, predicted);
        const directionalAccuracy = calculateDirectionalAccuracy(actual, predicted);

        // Calculate errors for confidence intervals
        const errors = actual.map((a, i) => Math.abs(a - predicted[i]));
        const errorStdDev = calculateStdDev(errors);

        return {
            mae,
            rmse,
            mape,
            directionalAccuracy,
            errorStdDev,
            testSize: testData.length,
            trainSize: trainData.length
        };
    }

    /**
     * Calculate confidence intervals for projection points
     * @param {Array} projectionPoints - Array of predicted price points
     * @param {number} errorStdDev - Standard deviation of historical errors
     * @param {number} confidence - Confidence level
     * @returns {Array} Array of {point, lower, upper} objects
     */
    function calculateConfidenceIntervals(projectionPoints, errorStdDev, confidence = 0.95) {
        if (!projectionPoints || projectionPoints.length === 0) {
            return [];
        }

        return projectionPoints.map(point => {
            const interval = calculatePredictionInterval(point, errorStdDev, confidence);
            return {
                point: point,
                lower: interval.lower,
                upper: interval.upper,
                confidence: confidence
            };
        });
    }

    /**
     * Validate projection accuracy
     * @param {Array} historicalPrices - Historical prices
     * @param {Array} projectionLines - Projection results
     * @param {Object} params - Projection parameters
     * @returns {Object} Validation results
     */
    function validate(historicalPrices, projectionLines, params) {
        if (!historicalPrices || historicalPrices.length === 0) {
            return { error: 'No historical data provided' };
        }

        if (!projectionLines || projectionLines.length === 0) {
            return { error: 'No projections provided' };
        }

        // Calculate average projection
        const avgProjection = [];
        const numSteps = projectionLines[0].points.length;

        for (let i = 0; i < numSteps; i++) {
            const sum = projectionLines.reduce((acc, line) => acc + line.points[i], 0);
            avgProjection.push(sum / projectionLines.length);
        }

        // Use recent historical data for comparison
        const recentHistorical = historicalPrices.slice(-Math.min(20, historicalPrices.length));
        const comparisonLength = Math.min(recentHistorical.length, avgProjection.length);

        // Calculate basic metrics
        const actual = recentHistorical.slice(-comparisonLength);
        const predicted = avgProjection.slice(0, comparisonLength);

        const mae = calculateMAE(actual, predicted);
        const rmse = calculateRMSE(actual, predicted);
        const mape = calculateMAPE(actual, predicted);

        // Calculate error standard deviation
        const errors = actual.map((a, i) => Math.abs(a - predicted[i]));
        const errorStdDev = calculateStdDev(errors);

        // Calculate confidence intervals
        const confidenceIntervals = calculateConfidenceIntervals(avgProjection, errorStdDev);

        return {
            mae,
            rmse,
            mape,
            errorStdDev,
            confidenceIntervals,
            projectionVariance: calculateStdDev(projectionLines.map(l => l.points[l.points.length - 1]))
        };
    }

    return {
        calculateMAE,
        calculateRMSE,
        calculateMAPE,
        calculateDirectionalAccuracy,
        calculatePredictionInterval,
        calculateConfidenceIntervals,
        backtest,
        validate
    };
})();








