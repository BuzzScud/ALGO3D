/**
 * Base Projection Model
 * Abstract base class for all projection models
 */

class ProjectionModel {
    constructor(name, config = {}) {
        this.name = name;
        this.config = config;
        this.accuracyHistory = [];
        this.totalPredictions = 0;
        this.totalErrors = [];
    }

    /**
     * Calculate projections
     * @param {Array<number>} historicalPrices - Historical price data
     * @param {Object} params - Model parameters
     * @returns {Object} {points: number[], confidence: number, metadata: Object}
     */
    project(historicalPrices, params) {
        throw new Error('project() must be implemented by subclass');
    }

    /**
     * Update accuracy tracking
     * @param {number} actual - Actual price
     * @param {number} predicted - Predicted price
     */
    updateAccuracy(actual, predicted) {
        if (actual && predicted) {
            const error = Math.abs(actual - predicted);
            this.totalErrors.push(error);
            this.totalPredictions++;

            // Keep only last 100 errors for rolling accuracy
            if (this.totalErrors.length > 100) {
                this.totalErrors.shift();
            }

            // Calculate recent accuracy
            const recentErrors = this.totalErrors.slice(-20);
            const avgError = recentErrors.reduce((a, b) => a + b, 0) / recentErrors.length;
            this.accuracyHistory.push({
                timestamp: Date.now(),
                error: error,
                avgError: avgError
            });

            // Keep only last 50 accuracy records
            if (this.accuracyHistory.length > 50) {
                this.accuracyHistory.shift();
            }
        }
    }

    /**
     * Get model weight based on inverse error
     * @returns {number} Weight (0-1)
     */
    getWeight() {
        if (this.totalErrors.length === 0) {
            return 0.5; // Default weight for new models
        }

        const avgError = this.totalErrors.reduce((a, b) => a + b, 0) / this.totalErrors.length;
        if (avgError === 0) return 1.0;

        // Inverse error weighting (lower error = higher weight)
        // Normalize to 0-1 range
        const maxError = Math.max(...this.totalErrors);
        if (maxError === 0) return 1.0;

        return 1.0 - (avgError / maxError);
    }

    /**
     * Get recent accuracy
     * @returns {number} Average error over last 20 predictions
     */
    getRecentAccuracy() {
        if (this.totalErrors.length === 0) return null;
        const recent = this.totalErrors.slice(-20);
        return recent.reduce((a, b) => a + b, 0) / recent.length;
    }

    /**
     * Get confidence score (0-1)
     * @returns {number} Confidence based on historical accuracy
     */
    getConfidence() {
        const weight = this.getWeight();
        const recentAccuracy = this.getRecentAccuracy();
        
        if (recentAccuracy === null) return 0.5;

        // Confidence based on consistency (lower variance = higher confidence)
        if (this.totalErrors.length < 5) return 0.5;

        const errors = this.totalErrors.slice(-20);
        const mean = errors.reduce((a, b) => a + b, 0) / errors.length;
        const variance = errors.reduce((sum, e) => sum + Math.pow(e - mean, 2), 0) / errors.length;
        const stdDev = Math.sqrt(variance);

        // Lower std dev relative to mean = higher confidence
        const consistency = mean > 0 ? 1.0 / (1.0 + stdDev / mean) : 0.5;

        return (weight + consistency) / 2;
    }
}

// Make available globally
if (typeof window !== 'undefined') {
    window.ProjectionModel = ProjectionModel;
}


