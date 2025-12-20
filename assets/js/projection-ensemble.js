/**
 * Projection Ensemble System
 * Combines multiple projection models with weighted averaging
 */

const ProjectionEnsemble = (function() {
    'use strict';

    class Ensemble {
        constructor(models = []) {
            this.models = models;
            this.weights = {};
            this.updateWeights();
        }

        /**
         * Add a model to the ensemble
         */
        addModel(model) {
            if (!(model instanceof ProjectionModel)) {
                throw new Error('Model must be an instance of ProjectionModel');
            }
            this.models.push(model);
            this.updateWeights();
        }

        /**
         * Remove a model from the ensemble
         */
        removeModel(modelName) {
            this.models = this.models.filter(m => m.name !== modelName);
            this.updateWeights();
        }

        /**
         * Update model weights based on accuracy
         */
        updateWeights() {
            if (this.models.length === 0) {
                this.weights = {};
                return;
            }

            // Calculate weights based on inverse error
            const weights = {};
            let totalWeight = 0;

            this.models.forEach(model => {
                const weight = model.getWeight();
                weights[model.name] = weight;
                totalWeight += weight;
            });

            // Normalize weights
            if (totalWeight > 0) {
                Object.keys(weights).forEach(name => {
                    weights[name] = weights[name] / totalWeight;
                });
            } else {
                // Equal weights if no accuracy data
                const equalWeight = 1.0 / this.models.length;
                Object.keys(weights).forEach(name => {
                    weights[name] = equalWeight;
                });
            }

            this.weights = weights;
        }

        /**
         * Combine projections from all models
         * @param {Array} projections - Array of {model, points, confidence} objects
         * @returns {Object} Combined projection
         */
        combine(projections) {
            if (projections.length === 0) {
                return { points: [], confidence: 0 };
            }

            // Find maximum length
            const maxLength = Math.max(...projections.map(p => p.points.length));

            // Weighted average of projections
            const combinedPoints = [];
            let totalConfidence = 0;
            let totalWeight = 0;

            for (let i = 0; i < maxLength; i++) {
                let weightedSum = 0;
                let sumWeights = 0;

                projections.forEach(proj => {
                    if (i < proj.points.length) {
                        const weight = this.weights[proj.model] || 0;
                        weightedSum += proj.points[i] * weight;
                        sumWeights += weight;
                    }
                });

                combinedPoints.push(sumWeights > 0 ? weightedSum / sumWeights : 0);
            }

            // Calculate ensemble confidence
            projections.forEach(proj => {
                const weight = this.weights[proj.model] || 0;
                totalConfidence += proj.confidence * weight;
                totalWeight += weight;
            });

            const ensembleConfidence = totalWeight > 0 ? totalConfidence / totalWeight : 0;

            return {
                points: combinedPoints,
                confidence: ensembleConfidence,
                modelContributions: Object.keys(this.weights).map(name => ({
                    model: name,
                    weight: this.weights[name]
                }))
            };
        }

        /**
         * Generate ensemble projection
         * @param {Array<number>} historicalPrices - Historical price data
         * @param {Object} params - Parameters for all models
         * @returns {Object} Ensemble projection result
         */
        project(historicalPrices, params) {
            if (this.models.length === 0) {
                throw new Error('No models in ensemble');
            }

            // Generate projections from all models
            const projections = this.models.map(model => {
                try {
                    const result = model.project(historicalPrices, params);
                    return {
                        model: model.name,
                        points: result.points,
                        confidence: result.confidence,
                        metadata: result.metadata
                    };
                } catch (error) {
                    console.error(`Error in model ${model.name}:`, error);
                    return null;
                }
            }).filter(p => p !== null);

            if (projections.length === 0) {
                throw new Error('No valid projections generated');
            }

            // Combine projections
            const combined = this.combine(projections);

            return {
                points: combined.points,
                confidence: combined.confidence,
                individualProjections: projections,
                modelContributions: combined.modelContributions,
                weights: this.weights
            };
        }

        /**
         * Get model statistics
         */
        getModelStats() {
            return this.models.map(model => ({
                name: model.name,
                weight: this.weights[model.name] || 0,
                recentAccuracy: model.getRecentAccuracy(),
                confidence: model.getConfidence(),
                totalPredictions: model.totalPredictions
            }));
        }
    }

    return {
        Ensemble
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.ProjectionEnsemble = ProjectionEnsemble;
}








