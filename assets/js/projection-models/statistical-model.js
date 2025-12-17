/**
 * Statistical Projection Model
 * ARIMA-like model using statistical analysis
 */

class StatisticalModel extends ProjectionModel {
    constructor(config = {}) {
        super('Statistical', config);
    }

    project(historicalPrices, params) {
        const steps = parseInt(params.steps) || 20;
        
        // Calculate statistical properties
        const stats = this.calculateStatistics(historicalPrices);
        
        // Calculate trend
        const trend = this.calculateTrend(historicalPrices);
        
        // Calculate volatility
        const volatility = this.calculateVolatility(historicalPrices);
        
        // Generate projection
        const points = [];
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        
        for (let i = 0; i < steps; i++) {
            // Trend component
            const trendComponent = trend * (i + 1);
            
            // Mean reversion component
            const meanReversion = (stats.mean - lastPrice) * (1 - Math.exp(-i / 10));
            
            // Random walk component (scaled by volatility)
            const randomWalk = this.generateRandomWalk(i, volatility, stats.stdDev);
            
            // Combine components
            const projectedPrice = lastPrice + trendComponent + meanReversion + randomWalk;
            points.push(this.trunc(projectedPrice, 4));
        }

        return {
            points: points,
            confidence: this.getConfidence(),
            metadata: {
                model: this.name,
                mean: stats.mean,
                stdDev: stats.stdDev,
                trend: trend,
                volatility: volatility
            }
        };
    }

    /**
     * Calculate basic statistics
     */
    calculateStatistics(prices) {
        const mean = prices.reduce((a, b) => a + b, 0) / prices.length;
        
        const variance = prices.reduce((sum, price) => {
            return sum + Math.pow(price - mean, 2);
        }, 0) / prices.length;
        
        const stdDev = Math.sqrt(variance);
        
        return { mean, variance, stdDev };
    }

    /**
     * Calculate linear trend
     */
    calculateTrend(prices) {
        if (prices.length < 2) return 0;

        const n = prices.length;
        let sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

        for (let i = 0; i < n; i++) {
            sumX += i;
            sumY += prices[i];
            sumXY += i * prices[i];
            sumX2 += i * i;
        }

        const slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
        return slope;
    }

    /**
     * Calculate volatility (standard deviation of returns)
     */
    calculateVolatility(prices) {
        if (prices.length < 2) return 0;

        const returns = [];
        for (let i = 1; i < prices.length; i++) {
            returns.push((prices[i] - prices[i-1]) / prices[i-1]);
        }

        const meanReturn = returns.reduce((a, b) => a + b, 0) / returns.length;
        const variance = returns.reduce((sum, ret) => {
            return sum + Math.pow(ret - meanReturn, 2);
        }, 0) / returns.length;

        return Math.sqrt(variance);
    }

    /**
     * Generate random walk component
     */
    generateRandomWalk(step, volatility, stdDev) {
        // Use simplified random walk with volatility scaling
        // In practice, this could use more sophisticated methods
        const randomFactor = (Math.random() - 0.5) * 2; // -1 to 1
        return randomFactor * stdDev * volatility * Math.sqrt(step + 1) * 0.1;
    }

    trunc(x, decimals) {
        const multiplier = Math.pow(10, decimals);
        return Math.floor(x * multiplier) / multiplier;
    }
}

// Make available globally
if (typeof window !== 'undefined') {
    window.StatisticalModel = StatisticalModel;
}



