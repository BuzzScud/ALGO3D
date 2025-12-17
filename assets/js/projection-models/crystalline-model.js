/**
 * Crystalline Projection Model
 * Enhanced version of the original crystalline projection
 */

class CrystallineModel extends ProjectionModel {
    constructor(config = {}) {
        super('Crystalline', config);
        
        // Constants from ALGO-1
        this.PHI_D = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31];
        this.PRIME_STOPS = [11, 13, 17, 29, 31, 47, 59, 61, 97, 101];
        this.SECTORS = 12;
        this.TWO_PI = Math.PI * 2;
        this.LAMBDA_DEFAULT = ['dub', 'kubt', "k'anch", 'dub', 'kubt', "k'anch"];
    }

    project(historicalPrices, params) {
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const depthPrime = parseInt(params.depthPrime) || 31;
        const omegaHz = params.omegaHz || 432;
        const steps = parseInt(params.steps) || 20;
        const triad = params.triad || [2, 5, 7];

        const points = this.computeCrystallineProjection({
            lastPrice,
            depthPrime,
            omegaHz,
            triad,
            decimals: 4,
            lambdaSchedule: this.LAMBDA_DEFAULT,
            N: steps
        });

        return {
            points: points,
            confidence: this.getConfidence(),
            metadata: {
                model: this.name,
                depthPrime,
                triad,
                omegaHz
            }
        };
    }

    computeCrystallineProjection({
        lastPrice,
        depthPrime,
        omegaHz = 432,
        triad = [2, 5, 7],
        decimals = 8,
        lambdaSchedule = this.LAMBDA_DEFAULT,
        omegaSchedule = null,
        N = 120
    }) {
        const psi = this.psiFromDepth(depthPrime);
        const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
        const tau = Math.log(triProd) / Math.log(3);

        let g = 1 + 0.01 * tau + 0.001 * (depthPrime % 7);
        const points = [];

        for (let i = 0; i < N; i++) {
            const lambda = lambdaSchedule[i % lambdaSchedule.length];
            const wHz = this.omegaAt(i, omegaSchedule || omegaHz);
            const theta_i = this.thetaStep(i, psi, lambda, wHz, depthPrime);
            g = this.growthStep(g, theta_i, wHz, triad);

            let latticeSum = 0;

            for (let s = 0; s < this.SECTORS; s++) {
                const angleBase = (i) * (this.TWO_PI / this.SECTORS) + (s * this.TWO_PI / this.SECTORS);
                const phiTerm = (this.PHI_D[s] % 360) * (Math.PI / 180);
                const nuVal = this.nuLambda(lambda);
                const lambdaNudge = (nuVal % 3) * (Math.PI / 360);
                const { phase: omegaPhase } = this.omegaGate(wHz);
                const quadrant = Math.floor(s / 3);
                const polQuad = ((quadrant % 2) === 0) ? 1 : -1;
                const polMob = ((i + s) % 2 === 0) ? 1 : -1;
                const ang = angleBase + phiTerm + lambdaNudge + 0.5 * omegaPhase;
                const base = Math.cos(ang);
                const gNorm = Math.tanh(g / 1e5);
                const term = base * polQuad * polMob * psi * (1 + 0.5 * gNorm);
                latticeSum += term;
            }

            const depthScale = Math.log(depthPrime) / Math.log(2);
            const triScale = Math.max(1, tau);
            const delta = this.trunc(latticeSum * depthScale * 0.5 * triScale, decimals);
            const pricePoint = this.trunc(lastPrice + delta, decimals);
            points.push(pricePoint);
        }

        return points;
    }

    psiFromDepth(depthPrime) {
        const idx = this.PRIME_STOPS.indexOf(depthPrime);
        if (idx === -1) return this.psiPlimpton(depthPrime, depthPrime - 2);
        const p = depthPrime;
        const q = idx > 0 ? this.PRIME_STOPS[idx - 1] : 2;
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
        const baseFreq = 432;
        const ratio = omegaHz / baseFreq;
        const phase = (Math.log(ratio) / Math.log(2)) * (Math.PI / 2);
        return { phase, magnitude: Math.sqrt(ratio) };
    }

    omegaAt(i, schedule) {
        if (Array.isArray(schedule)) {
            return schedule[i % schedule.length];
        }
        return schedule;
    }

    calculateTheta(n, k, lambda, omega, psi, depthPrime = 31) {
        const goldenRatio = (1 + Math.sqrt(5)) / 2;
        const PI = Math.PI;

        const term1 = k * PI * goldenRatio;
        const term2 = n * 2 * PI / 12;

        const nuValue = this.nuLambda(lambda);
        const nuVal = nuValue > 0 ? nuValue : 1;
        const term3 = Math.log(nuVal) / Math.log(3);

        const omegaValue = omega || 144000;
        const term4 = omegaValue / 432;

        const dp = depthPrime || 31;
        const idx = this.PRIME_STOPS.indexOf(dp);
        const p = dp;
        const q = idx > 0 ? this.PRIME_STOPS[idx - 1] : 2;
        const term5 = p * p - q * q;

        return term1 + term2 + term3 + term4 + term5;
    }

    thetaStep(i, psi, lambda, omegaHz, depthPrime = 31) {
        const n = i;
        const k = i;
        const psiValue = typeof psi === 'number' ? psi : depthPrime;
        return this.calculateTheta(n, k, lambda, omegaHz, psiValue, depthPrime);
    }

    growthStep(gPrev, theta, omegaHz, triad) {
        const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
        const tau = Math.log(triProd) / Math.log(3);
        const exponent = theta / 100;
        const base3Term = Math.pow(3, exponent);
        const tauTerm = 1 + tau / 1000;
        return gPrev * base3Term * tauTerm;
    }

    trunc(x, decimals) {
        const multiplier = Math.pow(10, decimals);
        return Math.floor(x * multiplier) / multiplier;
    }
}

// Make available globally
if (typeof window !== 'undefined') {
    window.CrystallineModel = CrystallineModel;
}



