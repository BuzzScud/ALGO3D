/**
 * Tetration Towers Module
 * Proper tetration tower computation for hyper-dimensional space mapping
 * Ported from algorithms/src/platonic_model/tetration.c
 * 
 * Tetration: base^base^base^...^base (depth times)
 * Used for discovering new Platonic solids and mapping hyper-dimensional space
 */

const TetrationTowers = (function() {
    'use strict';
    
    /**
     * Tetration Tower Structure
     */
    class TetrationTower {
        constructor(base, depth) {
            this.base = base;
            this.depth = depth;
            this.logTower = [];
            this.isConverged = false;
            this.iterations = 0;
            this.logValue = 0;
            this.compute();
        }
        
        /**
         * Compute tetration tower in log space
         */
        compute() {
            if (this.base < 2 || this.depth < 1) {
                throw new Error('Invalid base or depth');
            }
            
            const logBase = Math.log(this.base);
            
            // Start with base^1 = base
            this.logTower[0] = logBase;
            
            // Compute each level of the tower
            for (let d = 1; d < this.depth; d++) {
                this.logTower[d] = this.tetrationStepLog(this.logTower[d - 1], logBase);
                
                // Check for overflow (infinity)
                if (!isFinite(this.logTower[d]) || isNaN(this.logTower[d])) {
                    this.isConverged = false;
                    this.iterations = d;
                    this.logValue = this.logTower[d - 1];
                    return;
                }
                
                // Check for convergence
                if (d >= 3 && this.checkConvergence(d, 1e-10)) {
                    this.isConverged = true;
                    this.iterations = d;
                    this.logValue = this.logTower[d];
                    return;
                }
            }
            
            // Completed full depth
            this.isConverged = this.checkConvergence(this.depth, 1e-10);
            this.iterations = this.depth;
            this.logValue = this.logTower[this.depth - 1];
        }
        
        /**
         * Compute a single level of tetration in log space
         */
        tetrationStepLog(logPrev, logBase) {
            // If log_prev is small enough, we can exponentiate
            if (logPrev < 100.0) {
                const prev = Math.exp(logPrev);
                // Compute base^prev in log space
                return prev * logBase;
            } else {
                // For very large values, use approximation
                // log(base^x) ≈ x * log(base) when x is large
                return Math.exp(logPrev) * logBase;
            }
        }
        
        /**
         * Check if tetration has converged
         */
        checkConvergence(depth, tolerance) {
            if (depth < 3) return false;
            
            const diff1 = Math.abs(this.logTower[depth - 1] - this.logTower[depth - 2]);
            const diff2 = Math.abs(this.logTower[depth - 2] - this.logTower[depth - 3]);
            
            return (diff1 < tolerance && diff2 < tolerance);
        }
        
        /**
         * Get actual value (if computable)
         */
        getValue() {
            if (this.logValue < 100.0) {
                return Math.exp(this.logValue);
            } else {
                // Too large, return log value as proxy
                return this.logValue;
            }
        }
        
        /**
         * Get value at specific level
         */
        getValueAtLevel(level) {
            if (level >= this.logTower.length) return null;
            if (this.logTower[level] < 100.0) {
                return Math.exp(this.logTower[level]);
            }
            return this.logTower[level];
        }
    }
    
    /**
     * Create a set of tetration towers
     */
    function createTowerSet(bases, minDepth, maxDepth) {
        const towers = [];
        
        for (let b = 0; b < bases.length; b++) {
            for (let depth = minDepth; depth <= maxDepth; depth++) {
                try {
                    const tower = new TetrationTower(bases[b], depth);
                    towers.push(tower);
                } catch (e) {
                    console.warn(`Failed to create tower base=${bases[b]}, depth=${depth}:`, e);
                }
            }
        }
        
        return towers;
    }
    
    /**
     * Find nearest tetration attractor in hyper-dimensional space
     */
    function findHDAttractor(position, towers) {
        if (!position || !towers || towers.length === 0) {
            return null;
        }
        
        const numDimensions = position.length;
        const attractor = [];
        
        // Find attractor for each dimension independently
        for (let d = 0; d < numDimensions; d++) {
            let nearest = position[d];
            let minDistance = Infinity;
            
            for (let t = 0; t < towers.length; t++) {
                const tower = towers[t];
                const towerValue = tower.getValue();
                
                if (!isFinite(towerValue)) continue;
                
                const distance = Math.abs(position[d] - towerValue);
                
                if (distance < minDistance) {
                    minDistance = distance;
                    nearest = towerValue;
                }
            }
            
            attractor.push(nearest);
        }
        
        return attractor;
    }
    
    return {
        TetrationTower,
        createTowerSet,
        findHDAttractor
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.TetrationTowers = TetrationTowers;
}



