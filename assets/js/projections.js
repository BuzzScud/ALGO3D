/**
 * Projections Module - Price Projection Engine
 * FINAL UNIFIED SOLUTION
 * 
 * Features:
 * - Unified Projection Engine (single optimized algorithm)
 * - Integrated oscillation analysis (FFT-based)
 * - Built-in statistical validation
 * - Adaptive parameter tuning
 * - Fully interactive chart with:
 *   - Zoom (mouse wheel, pinch, buttons)
 *   - Pan (drag to scroll)
 *   - Range selection (Shift + drag)
 *   - Toggleable data series
 *   - Enhanced tooltips
 *   - Export functionality
 *   - Responsive design
 */
const ProjectionsModule = (function() {
    let projectionChart = null;
    let currentSymbol = '';
    let currentInterval = '1D'; // Changed to match timeframe format (15MIN, 1H, 4H, 1D)
    let historicalPrices = [];
    let historicalLabels = [];
    let historicalCandles = [];
    let validationResults = null;
    let savedProjectionData = null; // Store saved projection when loading
    let actualPriceData = null; // Store actual price data for comparison
    let refreshInterval = null; // Auto-refresh interval
    let isRefreshing = false; // Prevent concurrent refreshes
    // Unified Projection Engine is now the primary method (replaces ensemble)
    
    // Pan state management (similar to charts.js)
    let panState = {
        isPanning: false,
        startX: 0,
        startY: 0,
        lastX: 0,
        lastY: 0,
        boundHandlers: null
    };
    
    // Chart colors matching charts.js design exactly
    const chartColors = {
        grid: 'rgba(255, 255, 255, 0.1)',
        text: '#9ca3af',
        primary: '#3b82f6',
        bullish: '#22c55e',
        bearish: '#ef4444',
        sma20: '#3b82f6',
        sma50: '#f59e0b',
        ema12: '#8b5cf6',
        ema26: '#ec4899'
    };
    
    // Format price function (matching charts.js)
    function formatPrice(value) {
        if (value === null || value === undefined || isNaN(value)) return '$0.00';
        return '$' + value.toFixed(2);
    }
    
    /**
     * Convert date to EST timezone
     * Returns a Date object representing the same moment in EST
     * This properly handles timezone conversion
     */
    function toEST(date) {
        if (!date) return null;
        const d = date instanceof Date ? date : new Date(date);
        
        // If date is invalid, return current EST
        if (isNaN(d.getTime())) {
            return getCurrentEST();
        }
        
        // Get the time components in EST timezone
        const estString = d.toLocaleString('en-US', { 
            timeZone: 'America/New_York',
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            hour12: false
        });
        
        // Parse the EST string back to a Date object
        // Format: "MM/DD/YYYY, HH:MM:SS"
        const parts = estString.split(', ');
        if (parts.length !== 2) {
            // Fallback: return the date as-is if parsing fails
            return d;
        }
        
        const datePart = parts[0].split('/');
        const timePart = parts[1].split(':');
        
        if (datePart.length !== 3 || timePart.length !== 3) {
            return d;
        }
        
        const estDate = new Date(
            parseInt(datePart[2]), // year
            parseInt(datePart[0]) - 1, // month (0-indexed)
            parseInt(datePart[1]), // day
            parseInt(timePart[0]), // hour
            parseInt(timePart[1]), // minute
            parseInt(timePart[2]) // second
        );
        
        // Verify the date is valid
        if (isNaN(estDate.getTime())) {
            return d;
        }
        
        return estDate;
    }
    
    /**
     * Format date in EST timezone
     */
    function formatDateEST(timestamp) {
        if (!timestamp) return '';
        const date = timestamp instanceof Date ? timestamp : new Date(timestamp);
        const estDate = toEST(date);
        return estDate.toLocaleDateString('en-US', { 
            timeZone: 'America/New_York',
            year: 'numeric',
            month: 'short',
            day: 'numeric'
        });
    }
    
    /**
     * Format date with time in EST timezone (for intraday intervals)
     */
    function formatDateTimeEST(timestamp, includeTime = false) {
        if (!timestamp) return '';
        const date = timestamp instanceof Date ? timestamp : new Date(timestamp);
        const estDate = toEST(date);
        
        if (includeTime) {
            return estDate.toLocaleString('en-US', { 
                timeZone: 'America/New_York',
                year: 'numeric',
                month: 'short',
                day: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                hour12: true
            }) + ' EST';
        }
        
        return estDate.toLocaleDateString('en-US', { 
            timeZone: 'America/New_York',
            year: 'numeric',
            month: 'short',
            day: 'numeric'
        });
    }
    
    /**
     * Generate projection date labels based on interval (EST)
     */
    function generateProjectionLabels(steps, interval, historicalLabels) {
        // Prefer using historical candles for accurate last date
        let lastDate;
        
        if (historicalCandles && historicalCandles.length > 0) {
            // Use the last candle's timestamp (already converted to EST)
            const lastCandle = historicalCandles[historicalCandles.length - 1];
            const timestamp = lastCandle.time || lastCandle.timestamp;
            if (timestamp) {
                lastDate = new Date(timestamp);
                // Verify it's valid
                if (isNaN(lastDate.getTime())) {
                    lastDate = null;
                }
            }
        }
        
        // Fallback: try to get from last historical price date
        if (!lastDate || isNaN(lastDate.getTime())) {
            // Try to extract from the last historical label if it's a date string
            if (historicalLabels && historicalLabels.length > 0) {
                const lastLabel = historicalLabels[historicalLabels.length - 1];
                // Try parsing common date formats
                try {
                    // Try parsing as date string
                    lastDate = new Date(lastLabel);
                    if (isNaN(lastDate.getTime())) {
                        // Try parsing with EST timezone
                        const estDate = new Date(lastLabel + ' EST');
                        if (!isNaN(estDate.getTime())) {
                            lastDate = estDate;
                        } else {
                            lastDate = null;
                        }
                    }
                } catch (e) {
                    lastDate = null;
                }
            }
        }
        
        // Final fallback: use current EST time
        if (!lastDate || isNaN(lastDate.getTime())) {
            lastDate = getCurrentEST();
        }
        
        // Get the date components in EST timezone to ensure accurate date calculation
        // This ensures we're working with the actual EST date, not a timezone-shifted date
        const estDateString = lastDate.toLocaleString('en-US', {
            timeZone: 'America/New_York',
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            hour12: false
        });
        
        // Parse EST date string to get accurate date components
        const estParts = estDateString.split(', ');
        const datePart = estParts[0].split('/');
        
        // Extract date components (year, month, day) in EST
        const estYear = parseInt(datePart[2]);
        const estMonth = parseInt(datePart[0]) - 1; // Month is 0-indexed
        const estDay = parseInt(datePart[1]);
        
        // Map timeframe to interval configuration for projection labels
        // Timeframes: 15MIN, 1H, 4H, 1D
        const timeframeMap = {
            '15MIN': { minutes: 15 },
            '1H': { hours: 1 },
            '4H': { hours: 4 },
            '1D': { days: 1 },
            // Legacy support for old interval format
            '1d': { days: 1 },
            '5d': { days: 5 },
            '1mo': { months: 1 },
            '3mo': { months: 3 },
            '6mo': { months: 6 },
            '1y': { years: 1 }
        };
        
        const intervalConfig = timeframeMap[interval] || timeframeMap['1D'];
        const labels = [];
        
        for (let i = 1; i <= steps; i++) {
            // Calculate the projected date by working with EST components
            // Create a date object from EST components and perform date arithmetic
            const baseDate = new Date(Date.UTC(estYear, estMonth, estDay, 12, 0, 0));
            
            // Add the interval offset using UTC methods to avoid timezone issues
            let projectedDate = new Date(baseDate);
            
            if (intervalConfig.minutes) {
                projectedDate.setUTCMinutes(projectedDate.getUTCMinutes() + (intervalConfig.minutes * i));
            } else if (intervalConfig.hours) {
                projectedDate.setUTCHours(projectedDate.getUTCHours() + (intervalConfig.hours * i));
            } else if (intervalConfig.days) {
                projectedDate.setUTCDate(projectedDate.getUTCDate() + (intervalConfig.days * i));
            } else if (intervalConfig.months) {
                projectedDate.setUTCMonth(projectedDate.getUTCMonth() + (intervalConfig.months * i));
            } else if (intervalConfig.years) {
                projectedDate.setUTCFullYear(projectedDate.getUTCFullYear() + (intervalConfig.years * i));
            }
            
            // Format the date in EST timezone
            // For intraday timeframes (15MIN, 1H, 4H), include time in the label
            let formattedDate;
            if (interval === '15MIN' || interval === '1H' || interval === '4H') {
                formattedDate = projectedDate.toLocaleString('en-US', {
                    timeZone: 'America/New_York',
                    year: 'numeric',
                    month: 'short',
                    day: 'numeric',
                    hour: '2-digit',
                    minute: '2-digit',
                    hour12: false
                }) + ' EST';
            } else {
                formattedDate = projectedDate.toLocaleDateString('en-US', {
                    timeZone: 'America/New_York',
                    year: 'numeric',
                    month: 'short',
                    day: 'numeric'
                });
            }
            
            labels.push(formattedDate);
        }
        
        return labels;
    }
    
    
    /**
     * Get current time in EST
     */
    function getCurrentEST() {
        return new Date(new Date().toLocaleString('en-US', { timeZone: 'America/New_York' }));
    }
    
    /**
     * Convert timestamp to EST milliseconds
     * Properly converts a timestamp to EST timezone
     */
    function toESTTimestamp(timestamp) {
        if (!timestamp) return null;
        
        // Handle both Date objects and numeric timestamps
        let date;
        if (timestamp instanceof Date) {
            date = timestamp;
        } else if (typeof timestamp === 'number') {
            date = new Date(timestamp);
        } else {
            date = new Date(timestamp);
        }
        
        // If invalid date, return null
        if (isNaN(date.getTime())) {
            console.warn('Invalid timestamp:', timestamp);
            return null;
        }
        
        // Get the date/time components in EST
        const estComponents = date.toLocaleString('en-US', {
            timeZone: 'America/New_York',
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            hour12: false
        });
        
        // Parse EST components and create new Date
        const parts = estComponents.split(', ');
        if (parts.length !== 2) {
            return date.getTime(); // Fallback
        }
        
        const datePart = parts[0].split('/');
        const timePart = parts[1].split(':');
        
        if (datePart.length !== 3 || timePart.length !== 3) {
            return date.getTime(); // Fallback
        }
        
        const estDate = new Date(
            parseInt(datePart[2]),
            parseInt(datePart[0]) - 1,
            parseInt(datePart[1]),
            parseInt(timePart[0]),
            parseInt(timePart[1]),
            parseInt(timePart[2])
        );
        
        return estDate.getTime();
    }
    
    // Constants from ALGO-1
    const PHI_D = [3, 7, 31, 12, 19, 5, 11, 13, 17, 23, 29, 31];
    const PRIME_STOPS = [11, 13, 17, 29, 31, 47, 59, 61, 97, 101];
    const SECTORS = 12;
    const TWO_PI = Math.PI * 2;
    const LAMBDA_DEFAULT = ['dub', 'kubt', "k'anch", 'dub', 'kubt', "k'anch"];
    
    // Q8 Fixed-point constants
    const MOD_BITS = 72n;
    const MOD = 1n << MOD_BITS;
    const LAMBDA = 1n << (MOD_BITS - 2n);
    const Q_FRAC_BITS = 8n;
    const Q8 = 1 << 8;
    
    // First 500 primes
    const PRIMES_500 = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,2591,2593,2609,2617,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741,2749,2753,2767,2777,2789,2791,2797,2801,2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079,3083,3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,3371,3373,3389,3391,3407,3413,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571];
    
    // Helper functions
    function modPow(a, e, m) {
        a = ((a % m) + m) % m;
        let result = 1n;
        while (e > 0n) {
            if (e & 1n) result = (result * a) % m;
            a = (a * a) % m;
            e >>= 1n;
        }
        return result;
    }
    
    function psiPlimpton(p, q) {
        const p2 = p * p;
        const q2 = q * q;
        return (p2 - q2) / (p2 + q2);
    }
    
    function psiFromDepth(depthPrime) {
        const idx = PRIME_STOPS.indexOf(depthPrime);
        if (idx === -1) return psiPlimpton(depthPrime, depthPrime - 2);
        const p = depthPrime;
        const q = idx > 0 ? PRIME_STOPS[idx - 1] : 2;
        return psiPlimpton(p, q);
    }
    
    function nuLambda(lambda) {
        if (lambda === 'dub') return 3;
        if (lambda === 'kubt') return 5;
        if (lambda === "k'anch" || lambda === "k'anchay") return 7;
        if (typeof lambda === 'number') {
            return Math.pow(3, lambda) % 3;
        }
        return 3;
    }
    
    function omegaGate(omegaHz) {
        const baseFreq = 432;
        const ratio = omegaHz / baseFreq;
        const phase = (Math.log(ratio) / Math.log(2)) * (Math.PI / 2);
        return { phase, magnitude: Math.sqrt(ratio) };
    }
    
    function omegaAt(i, schedule) {
        if (Array.isArray(schedule)) {
            return schedule[i % schedule.length];
        }
        return schedule;
    }
    
    function calculateTheta(n, k, lambda, omega, psi, depthPrime = 31) {
        const goldenRatio = (1 + Math.sqrt(5)) / 2;
        const PI = Math.PI;
        
        const term1 = k * PI * goldenRatio;
        const term2 = n * 2 * PI / 12;
        
        const nuValue = nuLambda(lambda);
        const nuVal = nuValue > 0 ? nuValue : 1;
        const term3 = Math.log(nuVal) / Math.log(3);
        
        const omegaValue = omega || 144000;
        const term4 = omegaValue / 432;
        
        let term5 = 0;
        if (psi && typeof psi === 'object' && psi.p && psi.q) {
            term5 = psi.p * psi.p - psi.q * psi.q;
        } else if (typeof psi === 'number') {
            const idx = PRIME_STOPS.indexOf(psi);
            const p = psi;
            const q = idx > 0 ? PRIME_STOPS[idx - 1] : 2;
            term5 = p * p - q * q;
        } else {
            const dp = depthPrime || 31;
            const idx = PRIME_STOPS.indexOf(dp);
            const p = dp;
            const q = idx > 0 ? PRIME_STOPS[idx - 1] : 2;
            term5 = p * p - q * q;
        }
        
        return term1 + term2 + term3 + term4 + term5;
    }
    
    function thetaStep(i, psi, lambda, omegaHz, depthPrime = 31) {
        const n = i;
        const k = i;
        const psiValue = typeof psi === 'number' ? psi : depthPrime;
        return calculateTheta(n, k, lambda, omegaHz, psiValue, depthPrime);
    }
    
    function growthStep(gPrev, theta, omegaHz, triad) {
        const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
        const tau = Math.log(triProd) / Math.log(3);
        const exponent = theta / 100;
        const base3Term = Math.pow(3, exponent);
        const tauTerm = 1 + tau / 1000;
        return gPrev * base3Term * tauTerm;
    }
    
    function trunc(x, decimals) {
        const multiplier = Math.pow(10, decimals);
        return Math.floor(x * multiplier) / multiplier;
    }
    
    function isPrime(n) {
        if (n < 2) return false;
        if (n === 2) return true;
        if (n % 2 === 0) return false;
        for (let i = 3; i * i <= n; i += 2) {
            if (n % i === 0) return false;
        }
        return true;
    }
    
    function generateTriadsAroundPrime(pDepth, count, primes) {
        if (!isPrime(pDepth)) {
            throw new Error(`Depth must be a prime number, got: ${pDepth}`);
        }
        
        const idx = primes.indexOf(pDepth);
        if (idx === -1) {
            throw new Error(`Depth prime ${pDepth} not found in PRIMES_500`);
        }
        
        const triads = [];
        const half = Math.floor(count / 2);
        
        for (let offset = -half; offset <= half; offset++) {
            if (triads.length >= count) break;
            const i = Math.max(0, Math.min(primes.length - 3, idx + offset));
            const triad = [primes[i], primes[i + 1], primes[i + 2]];
            triads.push(triad);
        }
        
        return triads;
    }
    
    // Main projection calculation
    function computeCrystallineProjection({
        lastPrice,
        depthPrime,
        omegaHz = 432,
        triad = [2, 5, 7],
        decimals = 8,
        lambdaSchedule = LAMBDA_DEFAULT,
        omegaSchedule = null,
        N = 120
    }) {
        const psi = psiFromDepth(depthPrime);
        const triProd = triad.slice(0, 3).reduce((a, b) => a * b, 1);
        const tau = Math.log(triProd) / Math.log(3);
        
        let g = 1 + 0.01 * tau + 0.001 * (depthPrime % 7);
        const points = [];
        let currentPrice = lastPrice; // Start with last price and update cumulatively
        
        for (let i = 0; i < N; i++) {
            const lambda = lambdaSchedule[i % lambdaSchedule.length];
            const wHz = omegaAt(i, omegaSchedule || omegaHz);
            const theta_i = thetaStep(i, psi, lambda, wHz, depthPrime);
            g = growthStep(g, theta_i, wHz, triad);
            
            let latticeSum = 0;
            
            for (let s = 0; s < SECTORS; s++) {
                const angleBase = (i) * (TWO_PI / SECTORS) + (s * TWO_PI / SECTORS);
                const phiTerm = (PHI_D[s] % 360) * (Math.PI / 180);
                const nuVal = nuLambda(lambda);
                const lambdaNudge = (nuVal % 3) * (Math.PI / 360);
                const { phase: omegaPhase } = omegaGate(wHz);
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
            const delta = trunc(latticeSum * depthScale * 0.5 * triScale, decimals);
            const pricePoint = Math.max(0.01, trunc(currentPrice + delta, decimals));
            points.push(pricePoint);
            currentPrice = pricePoint; // Update for next iteration (cumulative projection)
        }
        
        return points;
    }
    
    // Calculate projections using multiple triads
    function calculateProjections(historicalPrices, params) {
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const depthPrime = parseInt(params.depthPrime) || 31;
        const base = parseFloat(params.base) || 3;
        const projectionCount = parseInt(params.projectionCount) || 12;
        const steps = parseInt(params.steps) || 20;
        
        // Generate triads around the depth prime
        const triads = generateTriadsAroundPrime(depthPrime, projectionCount, PRIMES_500);
        
        const projectionLines = [];
        
        triads.forEach((triad, idx) => {
            const projectedPrices = computeCrystallineProjection({
                lastPrice: lastPrice,
                depthPrime: depthPrime,
                omegaHz: 432,
                triad: triad,
                decimals: 4,
                N: steps
            });
            
            projectionLines.push({
                triad: triad,
                points: projectedPrices
            });
        });
        
        return projectionLines;
    }
    
    // Fetch market data with real-time support
    async function fetchMarketData(symbol, interval, forceRefresh = false) {
        try {
            // Validate inputs
            if (!symbol || typeof symbol !== 'string' || symbol.trim() === '') {
                throw new Error('Invalid symbol provided');
            }
            
            // Interval is now already in timeframe format (15MIN, 1H, 4H, 1D)
            const timeframe = interval || '1D';
            
            // Build URL with cache-busting parameter for real-time data
            const url = `api/charts.php?action=chart&symbol=${encodeURIComponent(symbol)}&timeframe=${encodeURIComponent(timeframe)}${forceRefresh ? '&refresh=true' : ''}`;
            
            console.log('Fetching market data:', { symbol, timeframe, forceRefresh, url });
            
            const response = await fetch(url, {
                method: 'GET',
                headers: {
                    'Accept': 'application/json',
                    'Cache-Control': forceRefresh ? 'no-cache' : 'max-age=30'
                }
            });
            
            if (!response.ok) {
                const errorText = await response.text();
                let errorMessage = `HTTP ${response.status}: Failed to fetch market data`;
                try {
                    const errorData = JSON.parse(errorText);
                    errorMessage = errorData.message || errorMessage;
                } catch (e) {
                    // Not JSON, use default message
                    if (errorText) {
                        errorMessage = errorText.substring(0, 200);
                    }
                }
                throw new Error(errorMessage);
            }
            
            const data = await response.json();
            
            // Validate response structure
            if (!data || typeof data !== 'object') {
                throw new Error('Invalid response format from server');
            }
            
            // Log the response for debugging
            console.log('Chart API response:', {
                success: data.success,
                hasCandles: !!data.candles,
                candlesCount: data.candles ? data.candles.length : 0,
                symbol: data.symbol,
                timeframe: data.timeframe,
                source: data.source,
                message: data.message
            });
            
            // Check if response indicates failure
            if (data.success === false) {
                const errorMsg = data.message || 'Unable to fetch market data. Please check your API configuration or try again later.';
                console.error('API returned error:', errorMsg);
                throw new Error(errorMsg);
            }
            
            // Check if we have candles data
            if (!data.candles || !Array.isArray(data.candles)) {
                const errorMsg = data.message || 'Invalid candles data format received from server.';
                console.error('Invalid candles data structure:', data);
                throw new Error(errorMsg);
            }
            
            if (data.candles.length === 0) {
                const errorMsg = data.message || 'No historical data available for this symbol. Please try a different symbol or timeframe.';
                console.error('No candles data:', errorMsg);
                throw new Error(errorMsg);
            }
            
            // Validate candle data structure
            const invalidCandles = data.candles.filter(c => {
                return !c || 
                       (c.close === undefined && c.price === undefined) ||
                       (c.time === undefined && c.timestamp === undefined);
            });
            
            if (invalidCandles.length > 0) {
                console.warn(`Found ${invalidCandles.length} invalid candles out of ${data.candles.length}`);
            }
            
            // Filter out invalid candles
            data.candles = data.candles.filter(c => {
                if (!c) return false;
                const hasPrice = c.close !== undefined || c.price !== undefined;
                const hasTime = c.time !== undefined || c.timestamp !== undefined;
                return hasPrice && hasTime;
            });
            
            if (data.candles.length === 0) {
                throw new Error('No valid candle data after validation. Please try again.');
            }
            
            console.log(`Successfully fetched ${data.candles.length} valid candles for ${symbol} (${timeframe})`);
            return data;
        } catch (error) {
            console.error('Error fetching market data:', error);
            throw error;
        }
    }
    
    // Fetch real-time market quote
    async function fetchRealTimeQuote(symbol) {
        try {
            if (!symbol || typeof symbol !== 'string' || symbol.trim() === '') {
                throw new Error('Invalid symbol provided');
            }
            
            const url = `api/charts.php?action=quote&symbol=${encodeURIComponent(symbol)}`;
            
            console.log('Fetching real-time quote:', { symbol, url });
            
            const response = await fetch(url, {
                method: 'GET',
                headers: {
                    'Accept': 'application/json',
                    'Cache-Control': 'no-cache'
                }
            });
            
            if (!response.ok) {
                const errorText = await response.text();
                let errorMessage = `HTTP ${response.status}: Failed to fetch real-time quote`;
                try {
                    const errorData = JSON.parse(errorText);
                    errorMessage = errorData.message || errorMessage;
                } catch (e) {
                    if (errorText) {
                        errorMessage = errorText.substring(0, 200);
                    }
                }
                throw new Error(errorMessage);
            }
            
            const data = await response.json();
            
            if (!data || typeof data !== 'object') {
                throw new Error('Invalid response format from server');
            }
            
            if (data.success === false) {
                const errorMsg = data.message || 'Unable to fetch real-time quote data';
                console.error('Quote API returned error:', errorMsg);
                throw new Error(errorMsg);
            }
            
            if (!data.current || data.current <= 0) {
                throw new Error('Invalid quote data: current price is missing or invalid');
            }
            
            console.log(`Successfully fetched real-time quote for ${symbol}: $${data.current}`);
            return data;
        } catch (error) {
            console.error('Error fetching real-time quote:', error);
            throw error;
        }
    }
    
    // Load chart data and calculate projections
    async function loadProjectionData(forceRefresh = false) {
        // Prevent concurrent refreshes
        if (isRefreshing) {
            console.log('Refresh already in progress, skipping...');
            return;
        }
        
        const symbolInput = document.getElementById('projection-symbol-input');
        const symbol = symbolInput.value.trim().toUpperCase();
        
        if (!symbol) {
            showError('Please enter a symbol');
            return;
        }
        
        currentSymbol = symbol;
        // Get timeframe from active button
        const activeTimeframeBtn = document.querySelector('.projection-timeframe-selector .timeframe-btn.active');
        currentInterval = activeTimeframeBtn ? activeTimeframeBtn.dataset.timeframe : '1D';
        
        isRefreshing = true;
        showLoading(true);
        hideError();
        
        try {
            const data = await fetchMarketData(symbol, currentInterval, forceRefresh);
            
            // Check if data is valid
            if (!data) {
                throw new Error('No data received from server');
            }
            
            // Check for error response
            if (data.success === false) {
                throw new Error(data.message || 'Failed to fetch market data');
            }
            
            // Check if candles array exists and has data
            if (!data.candles || !Array.isArray(data.candles) || data.candles.length === 0) {
                throw new Error('No historical data available for this symbol');
            }
            
            // Store candles data for high/low calculations
            // Note: API already returns timestamps in EST, but we'll ensure they're properly formatted
            historicalCandles = data.candles.map(c => {
                const candle = { ...c };
                
                // Ensure we have valid price data
                const close = parseFloat(candle.close || candle.price || 0);
                const open = parseFloat(candle.open || close);
                const high = parseFloat(candle.high || close);
                const low = parseFloat(candle.low || close);
                
                // Validate and set prices
                candle.close = !isNaN(close) && close > 0 ? close : null;
                candle.open = !isNaN(open) && open > 0 ? open : close;
                candle.high = !isNaN(high) && high > 0 ? high : close;
                candle.low = !isNaN(low) && low > 0 ? low : close;
                candle.volume = parseInt(candle.volume || 0) || 0;
                
                // Keep original timestamps - API already converts to EST
                // But ensure we have valid timestamps
                if (candle.time && typeof candle.time === 'number') {
                    // Timestamp is already in milliseconds (EST from API)
                    candle.originalTime = candle.time;
                } else if (candle.timestamp && typeof candle.timestamp === 'number') {
                    candle.time = candle.timestamp;
                    candle.originalTime = candle.timestamp;
                } else {
                    // Generate timestamp if missing (shouldn't happen, but handle gracefully)
                    console.warn('Missing timestamp in candle data:', candle);
                    candle.time = Date.now();
                    candle.originalTime = candle.time;
                }
                
                return candle;
            }).filter(c => c.close !== null && c.close > 0); // Filter out invalid candles
            
            if (historicalCandles.length === 0) {
                throw new Error('No valid candle data found after processing');
            }
            
            // Validate we have at least 5 days of data
            if (historicalCandles.length > 0) {
                const firstCandle = historicalCandles[0];
                const lastCandle = historicalCandles[historicalCandles.length - 1];
                const firstTime = firstCandle.time || firstCandle.timestamp || firstCandle.originalTime;
                const lastTime = lastCandle.time || lastCandle.timestamp || lastCandle.originalTime;
                
                if (firstTime && lastTime) {
                    const daysDiff = (lastTime - firstTime) / (1000 * 60 * 60 * 24);
                    if (daysDiff < 5) {
                        console.warn(`Only ${daysDiff.toFixed(1)} days of data available. Minimum 5 days recommended for accurate projections.`);
                    } else {
                        console.log(`✓ Validated: ${daysDiff.toFixed(1)} days of historical data available`);
                    }
                }
            }
            
            // Extract prices from candles (using EST-adjusted timestamps)
            historicalPrices = historicalCandles.map(c => {
                const price = parseFloat(c.close);
                if (isNaN(price) || price <= 0) {
                    console.warn('Invalid price in processed candle:', c);
                    return null;
                }
                return price;
            }).filter(price => price !== null && !isNaN(price) && price > 0);
            
            if (historicalPrices.length === 0) {
                throw new Error('No valid price data found after extraction');
            }
            
            console.log(`Processed ${historicalPrices.length} valid price points from ${data.candles.length} candles`);
            
            // Generate historical labels with proper EST formatting based on interval
            // Format dates consistently in EST timezone to ensure proper plotting
            historicalLabels = historicalCandles.map((c, i) => {
                // Get timestamp from candle
                let timestamp = c.time || c.timestamp || c.originalTime;
                if (!timestamp) {
                    // Fallback to original data
                    timestamp = data.candles[i]?.time || data.candles[i]?.timestamp;
                    if (!timestamp) {
                        return `Point ${i + 1}`;
                    }
                }
                
                // Create Date object from timestamp
                const date = new Date(timestamp);
                
                // Verify date is valid
                if (isNaN(date.getTime())) {
                    console.warn('Invalid date for candle', i, timestamp);
                    return `Point ${i + 1}`;
                }
                
                // Format date in EST timezone to ensure accurate date display
                // This ensures dates are properly plotted on the chart
                const estDateString = date.toLocaleString('en-US', {
                    timeZone: 'America/New_York',
                    year: 'numeric',
                    month: 'short',
                    day: 'numeric',
                    hour: '2-digit',
                    minute: '2-digit',
                    hour12: false
                });
                
                // Parse the EST date string to extract components
                const estParts = estDateString.split(', ');
                if (estParts.length >= 2) {
                    // Has time component: "Dec 16, 2025, 10:10"
                    const datePart = estParts[0]; // "Dec 16, 2025"
                    const timePart = estParts[1]; // "10:10"
                    
                    // For intraday timeframes (15MIN, 1H, 4H), always show time
                    if (currentInterval === '15MIN' || currentInterval === '1H' || currentInterval === '4H') {
                        return `${datePart} ${timePart} EST`;
                    }
                    
                    // For 1D timeframe with many candles, might be intraday - show time if needed
                    if (currentInterval === '1D' && historicalCandles.length > 50) {
                        // Likely intraday data - show date and time
                        return `${datePart} ${timePart} EST`;
                    }
                    
                    // For daily+ intervals, show date only
                    return datePart;
                } else {
                    // Fallback: format date only
                    const dateOnly = date.toLocaleDateString('en-US', {
                        timeZone: 'America/New_York',
                        year: 'numeric',
                        month: 'short',
                        day: 'numeric'
                    });
                    
                    // For 1d interval with many candles, try to include time
                    if (currentInterval === '1d' && historicalCandles.length > 50) {
                        const timeOnly = date.toLocaleTimeString('en-US', {
                            timeZone: 'America/New_York',
                            hour: '2-digit',
                            minute: '2-digit',
                            hour12: false
                        });
                        return `${dateOnly} ${timeOnly} EST`;
                    }
                    
                    return dateOnly;
                }
            });
            
            // Fetch real-time quote and update last price
            try {
                console.log('Fetching real-time quote to update projection starting point...');
                const quoteData = await fetchRealTimeQuote(symbol);
                
                if (quoteData && quoteData.current && quoteData.current > 0) {
                    const realTimePrice = parseFloat(quoteData.current);
                    const lastHistoricalPrice = historicalPrices[historicalPrices.length - 1];
                    
                    // Validate real-time price is reasonable (within 50% of last historical price)
                    const priceDiff = Math.abs(realTimePrice - lastHistoricalPrice) / lastHistoricalPrice;
                    if (priceDiff > 0.5) {
                        console.warn(`Real-time price ($${realTimePrice.toFixed(2)}) differs significantly from last historical price ($${lastHistoricalPrice.toFixed(2)}). Using historical price.`);
                    } else {
                        // Update the last price with real-time quote
                        if (historicalPrices.length > 0) {
                            historicalPrices[historicalPrices.length - 1] = realTimePrice;
                            console.log(`✓ Updated last price from $${lastHistoricalPrice.toFixed(2)} to real-time price $${realTimePrice.toFixed(2)}`);
                        } else {
                            // If no historical prices, add the real-time price
                            historicalPrices.push(realTimePrice);
                            console.log(`✓ Added real-time price $${realTimePrice.toFixed(2)} as starting point`);
                        }
                    }
                    
                    // Update the last label with current time
                    const currentTime = new Date();
                    const estTimeString = currentTime.toLocaleString('en-US', {
                        timeZone: 'America/New_York',
                        year: 'numeric',
                        month: 'short',
                        day: 'numeric',
                        hour: '2-digit',
                        minute: '2-digit',
                        hour12: false
                    });
                    
                    if (historicalLabels.length > 0) {
                        // For intraday timeframes, show time
                        if (currentInterval === '15MIN' || currentInterval === '1H' || currentInterval === '4H') {
                            historicalLabels[historicalLabels.length - 1] = `${estTimeString} EST (Live)`;
                        } else {
                            historicalLabels[historicalLabels.length - 1] = `${estTimeString} EST (Live)`;
                        }
                    } else {
                        historicalLabels.push(`${estTimeString} EST (Live)`);
                    }
                    
                    // Update the last candle with real-time data
                    if (historicalCandles.length > 0) {
                        const lastCandle = historicalCandles[historicalCandles.length - 1];
                        lastCandle.close = realTimePrice;
                        lastCandle.price = realTimePrice;
                        if (quoteData.high && quoteData.high > realTimePrice) {
                            lastCandle.high = quoteData.high;
                        }
                        if (quoteData.low && quoteData.low < realTimePrice) {
                            lastCandle.low = quoteData.low;
                        }
                        lastCandle.time = quoteData.timestamp ? quoteData.timestamp * 1000 : Date.now();
                    }
                } else {
                    console.warn('Real-time quote data is invalid, using last historical price');
                }
            } catch (quoteError) {
                console.warn('Failed to fetch real-time quote, using last historical price:', quoteError.message);
                // Continue with historical data if quote fetch fails
            }
            
            // Get parameters based on selected preset
            const selectedPreset = document.querySelector('input[name="projection-preset"]:checked');
            let params;
            
            if (selectedPreset && selectedPreset.value === 'custom') {
                // Custom preset: read from visible inputs
                params = {
                    steps: parseInt(document.getElementById('projection-steps').value) || 20,
                    base: parseFloat(document.getElementById('projection-base').value) || 3,
                    projectionCount: parseInt(document.getElementById('projection-count').value) || 12,
                    depthPrime: parseInt(document.getElementById('projection-depth').value) || 31
                };
            } else {
                // Preset selected: read from hidden value inputs (updated by preset)
                params = {
                    steps: parseInt(document.getElementById('projection-steps-value').value) || 20,
                    base: parseFloat(document.getElementById('projection-base-value').value) || 3,
                    projectionCount: parseInt(document.getElementById('projection-count-value').value) || 12,
                    depthPrime: parseInt(document.getElementById('projection-depth-value').value) || 31
                };
            }
            
            // Update active parameters display
            updateActiveParamsDisplay(params);
            
            // Log parameters for debugging
            console.log('Projection parameters:', {
                preset: selectedPreset ? selectedPreset.value : 'none',
                params: params,
                steps: params.steps,
                base: params.base,
                projectionCount: params.projectionCount,
                depthPrime: params.depthPrime
            });
            
            // Validate parameters are different from defaults to ensure presets are working
            if (selectedPreset && selectedPreset.value !== 'custom') {
                const presetConfig = PRESET_CONFIGS[selectedPreset.value];
                if (presetConfig) {
                    const paramsMatch = 
                        params.steps === presetConfig.steps &&
                        params.base === presetConfig.base &&
                        params.projectionCount === presetConfig.projectionCount &&
                        params.depthPrime === presetConfig.depthPrime;
                    
                    if (!paramsMatch) {
                        console.warn('Parameter mismatch! Expected:', presetConfig, 'Got:', params);
                    } else {
                        console.log('✓ Parameters match preset configuration');
                    }
                }
            }
            
            // Use Unified Projection Engine (FINAL SOLUTION)
            let projectionLines = [];
            let validationResults = null;
            
            try {
                // Check for unified engine
                if (typeof UnifiedProjectionEngine !== 'undefined' && 
                    UnifiedProjectionEngine.UnifiedProjectionEngine) {
                    
                    // Use the unified engine (primary method)
                    const engine = new UnifiedProjectionEngine.UnifiedProjectionEngine();
                    const result = engine.project(historicalPrices, params);
                    
                    if (result && result.projectionLines) {
                        // Convert to projection lines format
                        projectionLines = result.projectionLines.map(line => ({
                            triad: line.triad || ['Unknown'],
                            points: line.points || [],
                            confidence: line.confidence || 0.5
                        })).filter(line => line.points && line.points.length > 0);
                        
                        // Add ensemble line as primary projection (weighted average)
                        if (result.points && result.points.length > 0) {
                            projectionLines.unshift({
                                triad: ['Unified Ensemble'],
                                points: result.points,
                                confidence: result.confidence || 0.5,
                                isEnsemble: true
                            });
                        }
                        
                        // Use validation from unified engine
                        validationResults = result.validation;
                    } else {
                        throw new Error('Invalid result from unified engine');
                    }
                } else {
                    // Fallback to original method if unified engine not available
                    console.warn('Unified Projection Engine not available, using fallback method');
                    projectionLines = calculateProjections(historicalPrices, params);
                    
                    // Validate with ProjectionValidator if available
                    if (typeof ProjectionValidator !== 'undefined') {
                        try {
                            validationResults = ProjectionValidator.validate(historicalPrices, projectionLines, params);
                        } catch (e) {
                            console.warn('Validation error:', e);
                            validationResults = null;
                        }
                    }
                }
            } catch (error) {
                console.error('Error in unified projection engine:', error);
                // Fallback to original method with error handling
                try {
                    projectionLines = calculateProjections(historicalPrices, params);
                    
                    if (typeof ProjectionValidator !== 'undefined') {
                        try {
                            validationResults = ProjectionValidator.validate(historicalPrices, projectionLines, params);
                        } catch (e) {
                            console.warn('Validation error:', e);
                            validationResults = null;
                        }
                    }
                } catch (fallbackError) {
                    console.error('Fallback projection also failed:', fallbackError);
                    showError('Failed to generate projections. Please try again.');
                    return;
                }
            }
            
            // Ensure we have at least some projection lines
            if (!projectionLines || projectionLines.length === 0) {
                console.error('No projection lines generated');
                showError('Failed to generate projections. Please check your parameters.');
                return;
            }
            
            // Log projection summary with preset info
            const currentPreset = document.querySelector('input[name="projection-preset"]:checked');
            const presetLabel = currentPreset && currentPreset.value !== 'custom' 
                ? (currentPreset.closest('.param-toggle-item')?.querySelector('.param-toggle-name')?.textContent || currentPreset.value)
                : 'Custom';
            
            console.log('Projection generated:', {
                preset: presetLabel,
                steps: params.steps,
                base: params.base,
                projectionCount: params.projectionCount,
                depthPrime: params.depthPrime,
                projectionLines: projectionLines.length,
                totalPoints: projectionLines.reduce((sum, line) => sum + (line.points ? line.points.length : 0), 0)
            });
            
            renderChart(historicalPrices, historicalLabels, projectionLines, params);
            updateMetrics(historicalPrices, projectionLines, params);
            
            // Update validation metrics if available
            // Always try to validate, even if ensemble wasn't used
            if (!validationResults && typeof ProjectionValidator !== 'undefined') {
                try {
                    validationResults = ProjectionValidator.validate(historicalPrices, projectionLines, params);
                } catch (e) {
                    console.warn('Validation failed:', e);
                }
            }
            
            if (validationResults && !validationResults.error) {
                updateValidationMetrics(validationResults);
            } else {
                // Keep validation section visible but reset to placeholders if no validation data
                resetValidationMetricsToPlaceholders();
            }
            
            document.getElementById('projection-refresh-btn').style.display = 'inline-block';
            document.getElementById('reset-zoom-btn').style.display = 'inline-block';
            document.getElementById('save-projection-btn').style.display = 'inline-block';
            // Metrics sections are now always visible, no need to show them
            
            // Setup auto-refresh for real-time data
            setupAutoRefresh();
            
        } catch (error) {
            console.error('Error loading projection data:', error);
            const errorMessage = error.message || 'Failed to load projection data. Please check the symbol and try again.';
            showError(errorMessage);
            
            // Reset metrics to placeholders on error (keep sections visible)
            resetMetricsToPlaceholders();
            document.getElementById('projection-refresh-btn').style.display = 'none';
            document.getElementById('reset-zoom-btn').style.display = 'none';
            document.getElementById('save-projection-btn').style.display = 'none';
            
            // Stop auto-refresh on error
            stopAutoRefresh();
        } finally {
            isRefreshing = false;
            showLoading(false);
        }
    }
    
    // Setup auto-refresh for real-time data updates
    function setupAutoRefresh() {
        // Clear existing interval
        stopAutoRefresh();
        
        // Determine refresh interval based on timeframe
        // More frequent for intraday, less frequent for daily
        let refreshIntervalMs = 60000; // Default: 1 minute
        
        if (currentInterval === '15MIN') {
            refreshIntervalMs = 30000; // 30 seconds for 15-minute bars
        } else if (currentInterval === '1H' || currentInterval === '4H') {
            refreshIntervalMs = 60000; // 1 minute for hourly bars
        } else if (currentInterval === '1D') {
            refreshIntervalMs = 120000; // 2 minutes for daily bars
        }
        
        // Only refresh if we have a valid symbol
        if (currentSymbol && currentSymbol.trim() !== '') {
            refreshInterval = setInterval(() => {
                // Only refresh if not already refreshing and chart is visible
                if (!isRefreshing && projectionChart) {
                    const chartsPage = document.getElementById('page-charts');
                    const projectionsTab = document.getElementById('tab-projections');
                    
                    if (chartsPage && chartsPage.classList.contains('active') &&
                        projectionsTab && projectionsTab.classList.contains('active')) {
                        console.log('Auto-refreshing projection data...');
                        loadProjectionData(true); // Force refresh
                    }
                }
            }, refreshIntervalMs);
            
            console.log(`Auto-refresh enabled: ${refreshIntervalMs / 1000} seconds`);
        }
    }
    
    // Stop auto-refresh
    function stopAutoRefresh() {
        if (refreshInterval) {
            clearInterval(refreshInterval);
            refreshInterval = null;
            console.log('Auto-refresh stopped');
        }
    }
    
    // Render chart with optional actual price data for comparison
    function renderChart(historicalPrices, historicalLabels, projectionLines, params, actualPrices = null, actualLabels = null) {
        const ctx = document.getElementById('projection-chart');
        if (!ctx) {
            console.error('Chart canvas not found');
            return;
        }
        
        if (!historicalPrices || historicalPrices.length === 0) {
            console.error('No historical prices to render');
            showError('No historical data available');
            return;
        }
        
        // Hide error and show chart
        hideError();
        
        // Ensure chart canvas is visible
        const chartCanvas = document.getElementById('projection-chart');
        if (chartCanvas) {
            chartCanvas.style.display = 'block';
        }
        
        const steps = params.steps || 20;
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        
        // Generate projection labels based on interval (EST dates)
        const projectedLabels = generateProjectionLabels(steps, currentInterval, historicalLabels);
        
        const allLabels = [...historicalLabels, ...projectedLabels];
        
        // Calculate Y-axis min/max from all data points (historical + projections)
        // For candlestick charts, use high/low prices; for line charts, use close prices
        let minPrice, maxPrice;
        
        if (historicalCandles && historicalCandles.length > 0 && 
            historicalCandles[0].high !== undefined && historicalCandles[0].low !== undefined) {
            // Use high/low from candlesticks for better range
            const allHighs = historicalCandles.map(c => parseFloat(c.high)).filter(p => !isNaN(p) && p > 0);
            const allLows = historicalCandles.map(c => parseFloat(c.low)).filter(p => !isNaN(p) && p > 0);
            
            if (allHighs.length > 0 && allLows.length > 0) {
                maxPrice = Math.max(...allHighs);
                minPrice = Math.min(...allLows);
            } else {
                // Fallback to close prices
                const validHistoricalPrices = historicalPrices.filter(p => p !== null && !isNaN(p) && p > 0);
                minPrice = validHistoricalPrices.length > 0 ? Math.min(...validHistoricalPrices) : 0;
                maxPrice = validHistoricalPrices.length > 0 ? Math.max(...validHistoricalPrices) : 100;
            }
        } else {
            // Fallback to close prices for line chart
            const validHistoricalPrices = historicalPrices.filter(p => p !== null && !isNaN(p) && p > 0);
            minPrice = validHistoricalPrices.length > 0 ? Math.min(...validHistoricalPrices) : 0;
            maxPrice = validHistoricalPrices.length > 0 ? Math.max(...validHistoricalPrices) : 100;
        }
        
        // Include projection points in min/max calculation
        if (projectionLines && projectionLines.length > 0) {
            projectionLines.forEach(line => {
                if (line.points && line.points.length > 0) {
                    const validPoints = line.points.filter(p => p !== null && !isNaN(p) && p > 0);
                    if (validPoints.length > 0) {
                        const lineMin = Math.min(...validPoints);
                        const lineMax = Math.max(...validPoints);
                        minPrice = Math.min(minPrice, lineMin);
                        maxPrice = Math.max(maxPrice, lineMax);
                    }
                }
            });
        }
        
        // Include actual prices if available
        if (actualPrices && actualPrices.length > 0) {
            const validActualPrices = actualPrices.filter(p => p !== null && !isNaN(p) && p > 0);
            if (validActualPrices.length > 0) {
                const actualMin = Math.min(...validActualPrices);
                const actualMax = Math.max(...validActualPrices);
                minPrice = Math.min(minPrice, actualMin);
                maxPrice = Math.max(maxPrice, actualMax);
            }
        }
        
        // Add padding to Y-axis range (5% on each side)
        const priceRange = maxPrice - minPrice;
        const padding = priceRange > 0 ? priceRange * 0.05 : Math.max(minPrice * 0.05, 1);
        const yAxisMin = Math.max(0, minPrice - padding);
        const yAxisMax = maxPrice + padding;
        
        // Check if candlestick controller is available
        let chartType = 'line';
        let useCandlestick = false;
        
        if (typeof Chart !== 'undefined' && Chart.registry) {
            try {
                const candlestickController = Chart.registry.getController('candlestick');
                if (candlestickController) {
                    // Check if we have candlestick data available
                    const hasCandlestickData = historicalCandles && historicalCandles.length > 0 && 
                                               historicalCandles[0].open !== undefined && 
                                               historicalCandles[0].high !== undefined &&
                                               historicalCandles[0].low !== undefined &&
                                               historicalCandles[0].close !== undefined;
                    
                    if (hasCandlestickData) {
                        useCandlestick = true;
                        // Use 'line' chart type to allow mixing candlestick dataset with line datasets (projections)
                        // The candlestick dataset will be rendered via its type: 'candlestick'
                        chartType = 'line';
                    }
                }
            } catch (e) {
                console.warn('Candlestick controller not available, falling back to line chart');
            }
        }
        
        // Build datasets
        const datasets = [];
        
        if (useCandlestick) {
            // Create candlestick dataset with OHLC data
            const candlestickData = [];
            
            // Add historical candles with proper validation
            for (let i = 0; i < historicalCandles.length; i++) {
                const c = historicalCandles[i];
                const open = parseFloat(c.open);
                const high = parseFloat(c.high);
                const low = parseFloat(c.low);
                const close = parseFloat(c.close);
                const time = c.time || c.timestamp;
                
                // Validate all required fields
                if (!isNaN(open) && !isNaN(high) && !isNaN(low) && !isNaN(close) && 
                    open > 0 && high > 0 && low > 0 && close > 0 &&
                    time && typeof time === 'number' && time > 0 &&
                    high >= low && high >= open && high >= close &&
                    low <= open && low <= close) {
                    candlestickData.push({
                        x: time,
                        o: open,
                        h: high,
                        l: low,
                        c: close
                    });
                } else {
                    console.warn(`Invalid candlestick data at index ${i}:`, c);
                }
            }
            
            if (candlestickData.length === 0) {
                console.warn('No valid candlestick data, falling back to line chart');
                useCandlestick = false;
            }
            
            // For projection period, extend with last candle to match labels array length
            // This is required because candlestick datasets cannot have null/undefined values
            if (candlestickData.length > 0 && steps > 0) {
                const lastCandle = candlestickData[candlestickData.length - 1];
                const lastTime = lastCandle.x;
                
                // Calculate time increment based on interval
                let timeIncrement = 86400000; // Default: 1 day in milliseconds
                if (currentInterval === '15MIN') {
                    timeIncrement = 15 * 60 * 1000; // 15 minutes
                } else if (currentInterval === '1H') {
                    timeIncrement = 60 * 60 * 1000; // 1 hour
                } else if (currentInterval === '4H') {
                    timeIncrement = 4 * 60 * 60 * 1000; // 4 hours
                } else if (currentInterval === '1D') {
                    timeIncrement = 24 * 60 * 60 * 1000; // 1 day
                }
                
                for (let i = 0; i < steps; i++) {
                    // Use last candle's values to create valid candlestick objects
                    // The projection lines will overlay on top, so these won't be visible
                    let projectionTime;
                    if (projectedLabels[i]) {
                        // Try to parse the label as a date
                        const parsedDate = new Date(projectedLabels[i]);
                        if (!isNaN(parsedDate.getTime())) {
                            projectionTime = parsedDate.getTime();
                        } else {
                            projectionTime = lastTime + ((i + 1) * timeIncrement);
                        }
                    } else {
                        projectionTime = lastTime + ((i + 1) * timeIncrement);
                    }
                    
                    candlestickData.push({
                        x: projectionTime,
                        o: lastCandle.c, // Use close as open
                        h: lastCandle.c, // Use close as high
                        l: lastCandle.c, // Use close as low
                        c: lastCandle.c  // Use close as close (flat line)
                    });
                }
            }
            
            // Only add candlestick dataset if we have valid data
            if (candlestickData.length > 0) {
                // Ensure data length matches labels (or is close enough)
                if (Math.abs(candlestickData.length - allLabels.length) <= 1) {
                    datasets.push({
                        label: `${currentSymbol} Historical`,
                        type: 'candlestick',
                        data: candlestickData,
                        color: {
                            up: chartColors.bullish,
                            down: chartColors.bearish,
                            unchanged: '#6b7280'
                        },
                        yAxisID: 'y',
                        order: 1 // Render behind projections
                    });
                } else {
                    // Fallback if data doesn't match
                    console.warn(`Candlestick data length mismatch: ${candlestickData.length} vs ${allLabels.length}, falling back to line chart`);
                    useCandlestick = false;
                }
            } else {
                useCandlestick = false;
            }
        }
        
        // If not using candlestick (or fallback), use line chart
        if (!useCandlestick) {
            // Use close prices from candles if available, otherwise use historicalPrices
            let historicalData = [];
            
            if (historicalCandles && historicalCandles.length > 0 && historicalCandles[0].close !== undefined) {
                historicalData = historicalCandles.map(c => {
                    const close = parseFloat(c.close);
                    return (!isNaN(close) && close > 0) ? close : null;
                }).filter(p => p !== null);
            } else {
                historicalData = historicalPrices.filter(p => p !== null && !isNaN(p) && p > 0);
            }
            
            // Ensure we have data
            if (historicalData.length === 0) {
                console.error('No valid historical data for line chart');
                showError('No valid price data to display');
                return;
            }
            
            // Add nulls for projection period to match labels array
            const projectionNulls = new Array(steps).fill(null);
            historicalData = [...historicalData, ...projectionNulls];
            
            datasets.push({
                label: `${currentSymbol} Historical`,
                data: historicalData,
                borderColor: chartColors.primary, // Same blue as charts.js
                backgroundColor: 'transparent',
                tension: 0.1, // Same as charts.js
                fill: false,
                pointRadius: 0, // Same as charts.js
                borderWidth: 2,
                yAxisID: 'y',
                order: 1 // Render behind projections
            });
        }
        
        // Add projection lines - matching charts.js design style
        const projectionColors = [
            '#8b5cf6', '#10b981', '#f59e0b', '#ef4444', '#ec4899',
            '#14b8a6', '#f97316', '#6366f1', '#84cc16', '#06b6d4',
            '#a855f7', '#22c55e'
        ];
        
        projectionLines.forEach((line, idx) => {
            if (!line || !line.points || !Array.isArray(line.points) || line.points.length === 0) {
                console.warn(`Skipping invalid projection line at index ${idx}:`, line);
                return;
            }
            
            const lineData = [];
            
            // Fill historical period with nulls
            for (let i = 0; i < historicalPrices.length; i++) {
                lineData.push(null);
            }
            
            // Validate and add projection points
            const validPoints = line.points.filter(p => p !== null && !isNaN(p) && p > 0);
            
            if (validPoints.length === 0) {
                console.warn(`No valid points in projection line at index ${idx}`);
                return;
            }
            
            // First point connects to last historical price
            const firstProjected = validPoints[0];
            if (lastPrice > 0 && firstProjected > 0) {
                // Smooth transition: 80% last price, 20% first projected
                const connectionPrice = lastPrice * 0.8 + firstProjected * 0.2;
                lineData.push(connectionPrice);
            } else {
                // Fallback: use first projected point directly
                lineData.push(firstProjected);
            }
            
            // Add rest of projection points
            for (let i = 1; i < validPoints.length; i++) {
                lineData.push(validPoints[i]);
            }
            
            // Fill remaining with nulls to match labels array length
            while (lineData.length < allLabels.length) {
                lineData.push(null);
            }
            
            // Trim if too long (shouldn't happen, but handle gracefully)
            if (lineData.length > allLabels.length) {
                lineData.splice(allLabels.length);
            }
            
            // Determine if this is ensemble (solid line) or individual (dashed)
            const isEnsemble = line.isEnsemble || (line.triad && line.triad[0] === 'Ensemble');
            const isHistorical = line.triad && line.triad[0] === currentSymbol;
            
            datasets.push({
                label: `Projection [${line.triad.join('-')}]`,
                data: lineData,
                borderColor: projectionColors[idx % projectionColors.length],
                backgroundColor: 'transparent',
                borderWidth: isEnsemble ? 2.5 : 1.5,
                pointRadius: 0,
                tension: 0.1,
                borderDash: isEnsemble ? [] : [5, 5],
                yAxisID: 'y' // Same axis as historical data
            });
        });
        
        // Add actual price data if available (for comparison with saved projections)
        if (actualPrices && actualPrices.length > 0 && actualLabels && actualLabels.length > 0) {
            const actualData = [];
            
            // Create a map of actual prices by label for matching
            const actualPriceMap = new Map();
            for (let i = 0; i < actualLabels.length; i++) {
                if (actualPrices[i] !== null && !isNaN(actualPrices[i])) {
                    actualPriceMap.set(actualLabels[i], actualPrices[i]);
                }
            }
            
            // Fill with nulls for historical period
            for (let i = 0; i < historicalPrices.length; i++) {
                actualData.push(null);
            }
            
            // Match actual prices to projection labels by comparing label strings
            for (let i = historicalPrices.length; i < allLabels.length; i++) {
                const projectionLabel = allLabels[i];
                // Try exact match first
                let matchedPrice = actualPriceMap.get(projectionLabel);
                
                // If no exact match, try fuzzy matching (check if labels are similar)
                if (matchedPrice === undefined) {
                    // Find closest matching label
                    for (const [label, price] of actualPriceMap.entries()) {
                        // Check if labels are similar (same date, ignore time differences)
                        const labelDate = projectionLabel.split(',')[0]; // Get date part
                        const actualLabelDate = label.split(',')[0];
                        if (labelDate === actualLabelDate) {
                            matchedPrice = price;
                            break;
                        }
                    }
                }
                
                actualData.push(matchedPrice !== undefined ? matchedPrice : null);
            }
            
            // Ensure array length matches labels
            while (actualData.length < allLabels.length) {
                actualData.push(null);
            }
            
            // Filter out leading/trailing nulls for cleaner display, but keep alignment
            const hasActualData = actualData.some((val, idx) => idx >= historicalPrices.length && val !== null);
            
            if (hasActualData) {
                datasets.push({
                    label: 'Actual Price (Post-Projection)',
                    data: actualData,
                    borderColor: '#22c55e', // Green for actual
                    backgroundColor: 'rgba(34, 197, 94, 0.1)', // Light green fill
                    borderWidth: 3,
                    pointRadius: 3,
                    pointBackgroundColor: '#22c55e',
                    pointBorderColor: '#ffffff',
                    pointBorderWidth: 1,
                    tension: 0.1,
                    borderDash: [],
                    fill: false,
                    yAxisID: 'y',
                    order: 0 // Render on top
                });
                
                // Calculate and display accuracy metrics
                // Extract only non-null actual prices for comparison
                const validActualPrices = actualPrices.filter(p => p !== null && !isNaN(p) && p > 0);
                if (validActualPrices.length > 0) {
                    calculateAccuracyMetrics(projectionLines, validActualPrices, historicalPrices.length);
                }
            }
        }
        
        // Destroy existing chart and cleanup pan handlers
        if (projectionChart) {
            // Cleanup pan handlers before destroying
            if (panState.boundHandlers && projectionChart.canvas) {
                const canvas = projectionChart.canvas;
                canvas.removeEventListener('mousedown', panState.boundHandlers.mousedown);
                document.removeEventListener('mousemove', panState.boundHandlers.mousemove);
                document.removeEventListener('mouseup', panState.boundHandlers.mouseup);
                canvas.removeEventListener('touchstart', panState.boundHandlers.touchstart);
                canvas.removeEventListener('touchmove', panState.boundHandlers.touchmove);
                canvas.removeEventListener('touchend', panState.boundHandlers.touchend);
                panState.boundHandlers = null;
            }
            
            // Cleanup keyboard handlers
            if (projectionChart._keyboardHandlers) {
                document.removeEventListener('keydown', projectionChart._keyboardHandlers.keydown);
                document.removeEventListener('keyup', projectionChart._keyboardHandlers.keyup);
                projectionChart._keyboardHandlers = null;
            }
            
            // Clear title update interval if exists
            if (projectionChart._titleUpdateInterval) {
                clearInterval(projectionChart._titleUpdateInterval);
                projectionChart._titleUpdateInterval = null;
            }
            
            projectionChart.destroy();
            projectionChart = null;
        }
        
        // Stop auto-refresh when destroying chart (will restart after new chart is created)
        stopAutoRefresh();
        
        // Verify zoom plugin is available
        // The plugin should be registered in index.php
        const zoomPluginRegistered = Chart.registry && Chart.registry.getPlugin('zoom') !== undefined;
        
        if (!zoomPluginRegistered) {
            console.warn('Zoom plugin not registered - attempting to register now');
            try {
                if (typeof zoomPlugin !== 'undefined') {
                    Chart.register(zoomPlugin);
                    console.log('Zoom plugin registered successfully');
                } else if (window.zoomPlugin) {
                    Chart.register(window.zoomPlugin);
                    console.log('Zoom plugin registered from window');
                } else {
                    console.error('Zoom plugin not found - zoom/pan features will not work');
                }
            } catch (e) {
                console.error('Failed to register zoom plugin:', e);
            }
        } else {
            console.log('Zoom plugin is registered and ready');
        }
        
        // Create interactive legend checkboxes
        createInteractiveLegend(datasets);
        
        // Create new chart
        projectionChart = new Chart(ctx, {
            type: chartType,
            data: {
                labels: allLabels,
                datasets: datasets
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index',
                    includeInvisible: true
                },
                onHover: (event, activeElements) => {
                    // Don't change cursor if currently panning
                    if (panState.isPanning) return;
                    
                    const canvas = event.native?.target;
                    if (canvas) {
                        canvas.style.cursor = 'grab';
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top',
                        align: 'start',
                        onClick: function(e, legendItem, legend) {
                            // Toggle dataset visibility
                            const index = legendItem.datasetIndex;
                            const chart = legend.chart;
                            const meta = chart.getDatasetMeta(index);
                            
                            meta.hidden = meta.hidden === null ? !chart.data.datasets[index].hidden : null;
                            chart.update();
                            
                            // Update checkbox state
                            updateLegendCheckbox(index, !meta.hidden);
                        },
                        labels: {
                            color: chartColors.text,
                            usePointStyle: true,
                            padding: 12,
                            boxWidth: 12,
                            font: {
                                size: 11
                            }
                        },
                        onHover: function(e) {
                            e.native.target.style.cursor = 'pointer';
                        },
                        onLeave: function(e) {
                            e.native.target.style.cursor = 'default';
                        }
                    },
                    tooltip: {
                        backgroundColor: 'rgba(0, 0, 0, 0.8)',
                        titleColor: '#fff',
                        bodyColor: '#fff',
                        borderColor: 'rgba(255, 255, 255, 0.1)',
                        borderWidth: 1,
                        callbacks: {
                            title: function(context) {
                                if (context.length > 0) {
                                    const index = context[0].dataIndex;
                                    const label = allLabels[index];
                                    return label || `Point ${index + 1}`;
                                }
                                return '';
                            },
                            label: function(context) {
                                // Handle candlestick data
                                if (context.dataset.type === 'candlestick' && context.raw) {
                                    const raw = context.raw;
                                    if (typeof raw === 'object' && raw.o !== undefined) {
                                        return [
                                            `Open: ${formatPrice(raw.o)}`,
                                            `High: ${formatPrice(raw.h)}`,
                                            `Low: ${formatPrice(raw.l)}`,
                                            `Close: ${formatPrice(raw.c)}`
                                        ];
                                    }
                                }
                                
                                // Handle line data
                                const value = context.raw;
                                if (value === null || value === undefined || isNaN(value)) {
                                    return `${context.dataset.label}: N/A`;
                                }
                                return `${context.dataset.label}: ${formatPrice(value)}`;
                            }
                        }
                    },
                    zoom: {
                        zoom: {
                            wheel: {
                                enabled: true,
                                speed: 0.1,
                                modifierKey: null
                            },
                            pinch: {
                                enabled: true
                            },
                            drag: {
                                enabled: false
                            },
                            mode: 'xy'
                        },
                        pan: {
                            enabled: false // Using custom pan implementation
                        }
                    }
                },
                layout: {
                    padding: {
                        bottom: 60,
                        right: 20,
                        top: 20,
                        left: 20
                    }
                },
                scales: {
                    x: {
                        display: false,
                        title: {
                            display: false,
                            text: 'Date/Time (EST)',
                            color: chartColors.text,
                            font: {
                                size: 12,
                                weight: 'bold'
                            },
                            padding: {
                                top: 10,
                                bottom: 5
                            }
                        },
                        grid: {
                            color: chartColors.grid,
                            drawBorder: true,
                            borderColor: chartColors.grid
                        },
                        ticks: {
                            color: chartColors.text,
                            maxRotation: 45,
                            minRotation: 0,
                            padding: 8,
                            maxTicksLimit: 15,
                            callback: function(value, index, ticks) {
                                // Access labels from chart data
                                const chart = this.chart;
                                if (chart && chart.data && chart.data.labels) {
                                    const label = chart.data.labels[index];
                                    if (label) {
                                        return label;
                                    }
                                }
                                // Fallback: return empty string
                                return '';
                            },
                            autoSkip: true,
                            autoSkipPadding: 10
                        }
                    },
                    y: {
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Price ($)',
                            color: chartColors.text,
                            font: {
                                size: 12,
                                weight: 'bold'
                            },
                            padding: {
                                top: 5,
                                right: 10
                            }
                        },
                        grid: {
                            color: chartColors.grid,
                            drawBorder: true,
                            borderColor: chartColors.grid
                        },
                        ticks: {
                            color: chartColors.text,
                            padding: 8,
                            callback: function(value) {
                                if (value === null || value === undefined || isNaN(value)) {
                                    return '';
                                }
                                return formatPrice(value);
                            },
                            precision: 2
                        },
                        beginAtZero: false,
                        min: yAxisMin,
                        max: yAxisMax
                    }
                }
            }
        });
        
        // Update chart title with EST timezone
        const titleElement = document.getElementById('projection-chart-title');
        if (titleElement) {
            const intervalMap = {
                '15MIN': '15 Minutes',
                '1H': '1 Hour',
                '4H': '4 Hours',
                '1D': '1 Day',
                // Legacy support
                '1d': '1 Day',
                '5d': '5 Days',
                '1mo': '1 Month',
                '3mo': '3 Months',
                '6mo': '6 Months',
                '1y': '1 Year'
            };
            const intervalLabel = intervalMap[currentInterval] || currentInterval;
            const currentEST = getCurrentEST();
            const estTimeStr = currentEST.toLocaleString('en-US', {
                timeZone: 'America/New_York',
                month: 'short',
                day: 'numeric',
                year: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                hour12: true
            });
            const refreshIndicator = refreshInterval ? '🟢 LIVE' : '';
            titleElement.textContent = `${currentSymbol} Price Projection - ${intervalLabel} (${steps} Steps) | EST | Last Updated: ${estTimeStr} EST ${refreshIndicator}`;
        }
        
        // Update last update time in title periodically
        if (refreshInterval) {
            // Update title every 10 seconds to show current time
            const titleUpdateInterval = setInterval(() => {
                const titleElement = document.getElementById('projection-chart-title');
                if (titleElement && projectionChart) {
                    const currentEST = getCurrentEST();
                    const estTimeStr = currentEST.toLocaleString('en-US', {
                        timeZone: 'America/New_York',
                        month: 'short',
                        day: 'numeric',
                        year: 'numeric',
                        hour: '2-digit',
                        minute: '2-digit',
                        hour12: true
                    });
                    const intervalMap = {
                        '15MIN': '15 Minutes',
                        '1H': '1 Hour',
                        '4H': '4 Hours',
                        '1D': '1 Day',
                        '1d': '1 Day',
                        '5d': '5 Days',
                        '1mo': '1 Month',
                        '3mo': '3 Months',
                        '6mo': '6 Months',
                        '1y': '1 Year'
                    };
                    const intervalLabel = intervalMap[currentInterval] || currentInterval;
                    const steps = params.steps || 20;
                    titleElement.textContent = `${currentSymbol} Price Projection - ${intervalLabel} (${steps} Steps) | EST | Last Updated: ${estTimeStr} EST 🟢 LIVE`;
                } else {
                    clearInterval(titleUpdateInterval);
                }
            }, 10000);
            
            // Store interval ID for cleanup
            if (!projectionChart._titleUpdateInterval) {
                projectionChart._titleUpdateInterval = titleUpdateInterval;
            }
        }
        
        // Always show zoom controls (plugin should be available)
        const resetZoomBtn = document.getElementById('reset-zoom-btn');
        const zoomInBtn = document.getElementById('zoom-in-btn');
        const zoomOutBtn = document.getElementById('zoom-out-btn');
        
        if (resetZoomBtn) resetZoomBtn.style.display = 'inline-block';
        if (zoomInBtn) zoomInBtn.style.display = 'inline-block';
        if (zoomOutBtn) zoomOutBtn.style.display = 'inline-block';
        
        // Setup zoom button handlers
        setupZoomControls();
        
        // Setup export functionality
        setupExportFunctionality();
        
        // Setup custom pan functionality (like charts.js)
        setupChartPan(projectionChart);
        
        // Setup keyboard listeners for Shift key detection
        setupKeyboardListeners();
        
        // Log chart creation for debugging
        console.log('Chart created with zoom/pan/drag enabled');
    }
    
    /**
     * Setup keyboard listeners for better cursor feedback
     */
    function setupKeyboardListeners() {
        const chartWrapper = document.querySelector('.chart-wrapper');
        if (!chartWrapper) return;
        
        // Track Shift key state
        const handleKeyDown = (e) => {
            if (e.key === 'Shift') {
                chartWrapper.classList.add('shift-active');
            }
        };
        
        const handleKeyUp = (e) => {
            if (e.key === 'Shift') {
                chartWrapper.classList.remove('shift-active');
            }
        };
        
        // Add listeners
        document.addEventListener('keydown', handleKeyDown);
        document.addEventListener('keyup', handleKeyUp);
        
        // Cleanup on chart destroy (stored for later cleanup if needed)
        if (!projectionChart._keyboardHandlers) {
            projectionChart._keyboardHandlers = {
                keydown: handleKeyDown,
                keyup: handleKeyUp
            };
        }
    }
    
    /**
     * Setup custom pan functionality for the chart
     * Similar to charts.js implementation for consistency
     */
    function setupChartPan(chart) {
        if (!chart || !chart.canvas) {
            console.warn('Cannot setup pan: chart or canvas not available');
            return;
        }
        
        const canvas = chart.canvas;
        
        // Remove existing handlers if any
        if (panState.boundHandlers) {
            canvas.removeEventListener('mousedown', panState.boundHandlers.mousedown);
            document.removeEventListener('mousemove', panState.boundHandlers.mousemove);
            document.removeEventListener('mouseup', panState.boundHandlers.mouseup);
            canvas.removeEventListener('touchstart', panState.boundHandlers.touchstart);
            canvas.removeEventListener('touchmove', panState.boundHandlers.touchmove);
            canvas.removeEventListener('touchend', panState.boundHandlers.touchend);
        }
        
        // Ensure canvas has proper styles for interaction
        canvas.style.cursor = 'grab';
        canvas.style.touchAction = 'none';
        canvas.style.userSelect = 'none';
        canvas.style.webkitUserSelect = 'none';
        canvas.style.mozUserSelect = 'none';
        canvas.style.msUserSelect = 'none';
        
        // Mouse handlers
        const handleMouseDown = (e) => {
            // Only respond to left mouse button
            // Shift key is reserved for zoom drag selection, so don't pan when Shift is held
            if (e.button !== 0) return;
            
            // Don't pan if Shift is held (that's for zoom selection)
            if (e.shiftKey) return;
            
            panState.isPanning = true;
            panState.startX = e.clientX;
            panState.startY = e.clientY;
            panState.lastX = e.clientX;
            panState.lastY = e.clientY;
            
            canvas.style.cursor = 'grabbing';
            
            // Prevent text selection during drag
            e.preventDefault();
            e.stopPropagation();
        };
        
        const handleMouseMove = (e) => {
            if (!panState.isPanning) return;
            
            const xScale = chart.scales.x;
            const yScale = chart.scales.y;
            
            if (!xScale || !yScale) return;
            
            const deltaX = e.clientX - panState.lastX;
            const deltaY = e.clientY - panState.lastY;
            
            // Skip if no movement
            if (deltaX === 0 && deltaY === 0) return;
            
            // Get the chart area dimensions for accurate scaling
            const chartArea = chart.chartArea;
            if (!chartArea) return;
            
            const chartWidth = chartArea.right - chartArea.left;
            const chartHeight = chartArea.bottom - chartArea.top;
            
            // Calculate pan distance in data units
            const xRange = xScale.max - xScale.min;
            const yRange = yScale.max - yScale.min;
            
            const xDelta = -(deltaX / chartWidth) * xRange;
            const yDelta = (deltaY / chartHeight) * yRange;
            
            // Initialize scale options if not already set
            if (xScale.options.min === undefined || xScale.options.max === undefined) {
                xScale.options.min = xScale.min;
                xScale.options.max = xScale.max;
            }
            if (yScale.options.min === undefined || yScale.options.max === undefined) {
                yScale.options.min = yScale.min;
                yScale.options.max = yScale.max;
            }
            
            // Apply pan
            xScale.options.min += xDelta;
            xScale.options.max += xDelta;
            yScale.options.min += yDelta;
            yScale.options.max += yDelta;
            
            // Update chart without animation for smooth panning
            chart.update('none');
            
            panState.lastX = e.clientX;
            panState.lastY = e.clientY;
            
            e.preventDefault();
        };
        
        const handleMouseUp = () => {
            if (panState.isPanning) {
                panState.isPanning = false;
                canvas.style.cursor = 'grab';
            }
        };
        
        // Touch handlers for mobile
        let touchState = { isPanning: false, lastX: 0, lastY: 0 };
        
        const handleTouchStart = (e) => {
            // Only handle single touch (multi-touch is for pinch zoom)
            if (e.touches.length !== 1) return;
            
            const touch = e.touches[0];
            touchState.isPanning = true;
            touchState.lastX = touch.clientX;
            touchState.lastY = touch.clientY;
            
            e.preventDefault();
        };
        
        const handleTouchMove = (e) => {
            if (!touchState.isPanning || e.touches.length !== 1) return;
            
            const touch = e.touches[0];
            const xScale = chart.scales.x;
            const yScale = chart.scales.y;
            
            if (!xScale || !yScale) return;
            
            const deltaX = touch.clientX - touchState.lastX;
            const deltaY = touch.clientY - touchState.lastY;
            
            const chartArea = chart.chartArea;
            if (!chartArea) return;
            
            const chartWidth = chartArea.right - chartArea.left;
            const chartHeight = chartArea.bottom - chartArea.top;
            
            const xRange = xScale.max - xScale.min;
            const yRange = yScale.max - yScale.min;
            
            const xDelta = -(deltaX / chartWidth) * xRange;
            const yDelta = (deltaY / chartHeight) * yRange;
            
            if (xScale.options.min === undefined || xScale.options.max === undefined) {
                xScale.options.min = xScale.min;
                xScale.options.max = xScale.max;
            }
            if (yScale.options.min === undefined || yScale.options.max === undefined) {
                yScale.options.min = yScale.min;
                yScale.options.max = yScale.max;
            }
            
            xScale.options.min += xDelta;
            xScale.options.max += xDelta;
            yScale.options.min += yDelta;
            yScale.options.max += yDelta;
            
            chart.update('none');
            
            touchState.lastX = touch.clientX;
            touchState.lastY = touch.clientY;
            
            e.preventDefault();
        };
        
        const handleTouchEnd = () => {
            touchState.isPanning = false;
        };
        
        // Store handlers for cleanup
        panState.boundHandlers = {
            mousedown: handleMouseDown,
            mousemove: handleMouseMove,
            mouseup: handleMouseUp,
            touchstart: handleTouchStart,
            touchmove: handleTouchMove,
            touchend: handleTouchEnd
        };
        
        // Add event listeners
        canvas.addEventListener('mousedown', handleMouseDown);
        document.addEventListener('mousemove', handleMouseMove);
        document.addEventListener('mouseup', handleMouseUp);
        
        // Touch events
        canvas.addEventListener('touchstart', handleTouchStart, { passive: false });
        canvas.addEventListener('touchmove', handleTouchMove, { passive: false });
        canvas.addEventListener('touchend', handleTouchEnd);
        
        // Prevent context menu on right-click
        canvas.addEventListener('contextmenu', (e) => e.preventDefault());
    }
    
    /**
     * Create interactive legend with checkboxes
     */
    function createInteractiveLegend(datasets) {
        const legendControls = document.getElementById('chart-legend-controls');
        const checkboxesContainer = document.getElementById('legend-checkboxes');
        
        if (!legendControls || !checkboxesContainer) return;
        
        // Clear existing checkboxes
        checkboxesContainer.innerHTML = '';
        
        // Create checkbox for each dataset
        datasets.forEach((dataset, index) => {
            const checkboxWrapper = document.createElement('div');
            checkboxWrapper.className = 'legend-checkbox-item';
            
            const checkbox = document.createElement('input');
            checkbox.type = 'checkbox';
            checkbox.id = `legend-checkbox-${index}`;
            checkbox.checked = true;
            checkbox.dataset.index = index;
            checkbox.className = 'legend-checkbox';
            
            const label = document.createElement('label');
            label.htmlFor = `legend-checkbox-${index}`;
            label.className = 'legend-checkbox-label';
            
            const colorBox = document.createElement('span');
            colorBox.className = 'legend-color-box';
            colorBox.style.backgroundColor = dataset.borderColor;
            
            const text = document.createElement('span');
            text.textContent = dataset.label;
            text.className = 'legend-text';
            
            label.appendChild(colorBox);
            label.appendChild(text);
            
            checkboxWrapper.appendChild(checkbox);
            checkboxWrapper.appendChild(label);
            checkboxesContainer.appendChild(checkboxWrapper);
            
            // Add event listener
            checkbox.addEventListener('change', function() {
                toggleDatasetVisibility(index, this.checked);
            });
        });
        
        legendControls.style.display = 'block';
    }
    
    /**
     * Update legend checkbox state
     */
    function updateLegendCheckbox(index, visible) {
        const checkbox = document.getElementById(`legend-checkbox-${index}`);
        if (checkbox) {
            checkbox.checked = visible;
        }
    }
    
    /**
     * Toggle dataset visibility
     */
    function toggleDatasetVisibility(index, visible) {
        if (!projectionChart) return;
        
        const meta = projectionChart.getDatasetMeta(index);
        if (meta) {
            meta.hidden = !visible;
            projectionChart.update();
        }
    }
    
    /**
     * Setup zoom controls
     */
    function setupZoomControls() {
        const zoomInBtn = document.getElementById('zoom-in-btn');
        const zoomOutBtn = document.getElementById('zoom-out-btn');
        const resetZoomBtn = document.getElementById('reset-zoom-btn');
        
        if (zoomInBtn) {
            zoomInBtn.addEventListener('click', function() {
                zoomChart(1.2);
            });
        }
        
        if (zoomOutBtn) {
            zoomOutBtn.addEventListener('click', function() {
                zoomChart(0.8);
            });
        }
        
        if (resetZoomBtn) {
            resetZoomBtn.addEventListener('click', resetZoom);
        }
    }
    
    /**
     * Zoom chart programmatically
     * @param {number} factor - Zoom factor (>1 = zoom in, <1 = zoom out)
     */
    function zoomChart(factor) {
        if (!projectionChart) {
            console.warn('No chart available for zooming');
            return;
        }
        
        try {
            const xScale = projectionChart.scales.x;
            const yScale = projectionChart.scales.y;
            
            if (!xScale || !yScale) {
                console.warn('Chart scales not available');
                return;
            }
            
            // Get current scale ranges
            let xMin = xScale.min;
            let xMax = xScale.max;
            let yMin = yScale.min;
            let yMax = yScale.max;
            
            // If scales are not limited, get from data
            if (xMin === undefined || xMax === undefined) {
                const data = projectionChart.data;
                if (data && data.labels && data.labels.length > 0) {
                    xMin = 0;
                    xMax = data.labels.length - 1;
                } else {
                    console.warn('Cannot zoom: no data available');
                    return;
                }
            }
            
            if (yMin === undefined || yMax === undefined) {
                // Get from datasets
                const datasets = projectionChart.data.datasets;
                let allValues = [];
                datasets.forEach(dataset => {
                    if (dataset.data) {
                        dataset.data.forEach(point => {
                            if (point !== null && typeof point === 'number') {
                                allValues.push(point);
                            }
                        });
                    }
                });
                if (allValues.length > 0) {
                    yMin = Math.min(...allValues) * 0.98;
                    yMax = Math.max(...allValues) * 1.02;
                } else {
                    console.warn('Cannot zoom: no price data available');
                    return;
                }
            }
            
            // Calculate centers and ranges
            const xCenter = (xMin + xMax) / 2;
            const yCenter = (yMin + yMax) / 2;
            const xRange = xMax - xMin;
            const yRange = yMax - yMin;
            
            // Calculate new ranges (inverse factor for zoom in/out)
            const newXRange = xRange / factor;
            const newYRange = yRange / factor;
            
            // Set new scale limits
            xScale.options.min = xCenter - newXRange / 2;
            xScale.options.max = xCenter + newXRange / 2;
            yScale.options.min = yCenter - newYRange / 2;
            yScale.options.max = yCenter + newYRange / 2;
            
            // Update chart
            projectionChart.update('none');
        } catch (error) {
            console.error('Error zooming chart:', error);
        }
    }
    
    /**
     * Setup export functionality
     */
    function setupExportFunctionality() {
        const exportBtn = document.getElementById('export-chart-btn');
        
        if (exportBtn) {
            exportBtn.addEventListener('click', function() {
                exportChart();
            });
        }
    }
    
    /**
     * Export chart as image
     */
    function exportChart() {
        if (!projectionChart) {
            showError('No chart to export');
            return;
        }
        
        try {
            // Get chart canvas
            const canvas = projectionChart.canvas;
            
            // Create download link
            const link = document.createElement('a');
            // Use EST timestamp for filename
            const estNow = getCurrentEST();
            link.download = `${currentSymbol}_projection_${estNow.getTime()}.png`;
            link.href = canvas.toDataURL('image/png');
            
            // Trigger download
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
            
            // Show success message
            const exportBtn = document.getElementById('export-chart-btn');
            if (exportBtn) {
                const originalHTML = exportBtn.innerHTML;
                exportBtn.innerHTML = '<i class="fas fa-check"></i> Exported!';
                exportBtn.style.background = 'var(--success-color)';
                
                setTimeout(() => {
                    exportBtn.innerHTML = originalHTML;
                    exportBtn.style.background = '';
                }, 2000);
            }
        } catch (error) {
            console.error('Error exporting chart:', error);
            showError('Failed to export chart');
        }
    }
    
    /**
     * Update price metrics display
     * Matches the design from the screenshot
     */
    // Calculate accuracy metrics comparing projection to actual prices
    function calculateAccuracyMetrics(projectionLines, actualPrices, historicalLength) {
        if (!actualPrices || actualPrices.length === 0 || !projectionLines || projectionLines.length === 0) {
            return;
        }
        
        // Find the ensemble projection (main projection)
        const ensembleProjection = projectionLines.find(line => 
            line.isEnsemble || (line.triad && line.triad[0] === 'Ensemble')
        ) || projectionLines[0];
        
        if (!ensembleProjection || !ensembleProjection.points) {
            return;
        }
        
        const projectedPoints = ensembleProjection.points;
        const minLength = Math.min(projectedPoints.length, actualPrices.length);
        
        if (minLength === 0) return;
        
        const lastPrice = historicalPrices[historicalPrices.length - 1] || 1;
        
        // Calculate errors
        let totalError = 0;
        let totalAbsoluteError = 0;
        let totalPercentageError = 0;
        let maxError = 0;
        
        for (let i = 0; i < minLength; i++) {
            const projected = projectedPoints[i];
            const actual = actualPrices[i];
            
            if (projected && actual && !isNaN(projected) && !isNaN(actual)) {
                const error = projected - actual;
                const absError = Math.abs(error);
                const pctError = (absError / actual) * 100;
                
                totalError += error;
                totalAbsoluteError += absError;
                totalPercentageError += pctError;
                maxError = Math.max(maxError, absError);
            }
        }
        
        const mae = totalAbsoluteError / minLength; // Mean Absolute Error
        const mape = totalPercentageError / minLength; // Mean Absolute Percentage Error
        const bias = totalError / minLength; // Bias (positive = overestimate, negative = underestimate)
        
        // Display accuracy metrics
        const accuracySection = document.getElementById('accuracy-metrics-section');
        if (accuracySection) {
            accuracySection.style.display = 'block';
            
            const maeEl = document.getElementById('accuracy-mae');
            const mapeEl = document.getElementById('accuracy-mape');
            const biasEl = document.getElementById('accuracy-bias');
            const maxErrorEl = document.getElementById('accuracy-max-error');
            
            if (maeEl) {
                maeEl.textContent = formatPrice(mae);
                maeEl.className = 'metric-value ' + (mae < lastPrice * 0.02 ? 'positive' : mae < lastPrice * 0.05 ? '' : 'negative');
            }
            
            if (mapeEl) {
                mapeEl.textContent = mape.toFixed(2) + '%';
                mapeEl.className = 'metric-value ' + (mape < 2 ? 'positive' : mape < 5 ? '' : 'negative');
            }
            
            if (biasEl) {
                const biasPct = (bias / lastPrice) * 100;
                biasEl.textContent = formatPrice(bias) + ' (' + (biasPct >= 0 ? '+' : '') + biasPct.toFixed(2) + '%)';
                biasEl.className = 'metric-value ' + (Math.abs(biasPct) < 2 ? 'positive' : Math.abs(biasPct) < 5 ? '' : 'negative');
            }
            
            if (maxErrorEl) {
                maxErrorEl.textContent = formatPrice(maxError);
                maxErrorEl.className = 'metric-value ' + (maxError < lastPrice * 0.05 ? 'positive' : maxError < lastPrice * 0.1 ? '' : 'negative');
            }
        }
    }
    
    // Load saved projection with actual price comparison
    async function loadSavedProjectionWithActual(projectionId) {
        try {
            // Fetch saved projection
            const response = await fetch('api/projections.php');
            if (!response.ok) throw new Error('Failed to fetch projections');
            
            const result = await response.json();
            if (!result.success || !result.projections) {
                throw new Error('No projections found');
            }
            
            const proj = result.projections.find(p => p.id == projectionId);
            if (!proj) {
                throw new Error('Projection not found');
            }
            
            // Parse projection data
            let projectionData = typeof proj.projection_data === 'string' 
                ? JSON.parse(proj.projection_data) 
                : proj.projection_data;
            
            if (!projectionData) {
                throw new Error('Invalid projection data');
            }
            
            savedProjectionData = projectionData;
            currentSymbol = projectionData.symbol || proj.symbol;
            
            // Handle both new timeframe format (15MIN, 1H, 4H, 1D) and legacy format (1d, 5d, etc.)
            const savedInterval = projectionData.interval || '1D';
            // Map legacy intervals to new timeframes
            const intervalToTimeframeMap = {
                '1d': '1D',
                '5d': '5D',
                '1mo': '1M',
                '3mo': '3M',
                '6mo': '6M',
                '1y': '1Y'
            };
            currentInterval = intervalToTimeframeMap[savedInterval] || savedInterval;
            
            // Set symbol and interval in UI
            const symbolInput = document.getElementById('projection-symbol-input');
            if (symbolInput) symbolInput.value = currentSymbol;
            
            // Update active timeframe button
            const timeframeBtns = document.querySelectorAll('.projection-timeframe-selector .timeframe-btn');
            timeframeBtns.forEach(btn => {
                btn.classList.remove('active');
                if (btn.dataset.timeframe === currentInterval) {
                    btn.classList.add('active');
                }
            });
            
            // If no matching button found, default to 1D
            const activeBtn = document.querySelector('.projection-timeframe-selector .timeframe-btn.active');
            if (!activeBtn) {
                currentInterval = '1D';
                const defaultBtn = document.querySelector('.projection-timeframe-selector .timeframe-btn[data-timeframe="1D"]');
                if (defaultBtn) {
                    defaultBtn.classList.add('active');
                }
            }
            
            // Use saved historical data
            historicalPrices = projectionData.historicalPrices || [];
            historicalLabels = projectionData.historicalLabels || [];
            const savedProjectionLines = projectionData.projectionLines || [];
            const savedParams = projectionData.params || (typeof proj.params === 'string' ? JSON.parse(proj.params) : proj.params) || {};
            
            // Use saved projection labels if available, otherwise generate them
            let projectionLabels = projectionData.projectionLabels || [];
            const savedTimestamp = projectionData.savedTimestamp || new Date(proj.saved_at).getTime();
            
            // Fetch actual price data from saved date to now
            const savedDate = new Date(savedTimestamp);
            showLoading(true);
            
            // Generate projection labels if not saved
            const steps = savedParams.steps || 20;
            const projectionStartDate = new Date(savedDate);
            
            if (projectionLabels.length === 0) {
                // Calculate time increment based on interval
                let timeIncrementMs = 24 * 60 * 60 * 1000; // Default: 1 day
                if (currentInterval === '15MIN') {
                    timeIncrementMs = 15 * 60 * 1000; // 15 minutes
                } else if (currentInterval === '1H') {
                    timeIncrementMs = 60 * 60 * 1000; // 1 hour
                } else if (currentInterval === '4H') {
                    timeIncrementMs = 4 * 60 * 60 * 1000; // 4 hours
                } else if (currentInterval === '1D') {
                    timeIncrementMs = 24 * 60 * 60 * 1000; // 1 day
                }
                
                // Generate projection labels starting from saved date
                for (let i = 0; i < steps; i++) {
                    const projectionDate = new Date(projectionStartDate.getTime() + (i + 1) * timeIncrementMs);
                    const estDateString = projectionDate.toLocaleString('en-US', {
                        timeZone: 'America/New_York',
                        year: 'numeric',
                        month: 'short',
                        day: 'numeric',
                        hour: (currentInterval === '15MIN' || currentInterval === '1H' || currentInterval === '4H') ? '2-digit' : undefined,
                        minute: (currentInterval === '15MIN' || currentInterval === '1H' || currentInterval === '4H') ? '2-digit' : undefined,
                        hour12: false
                    });
                    projectionLabels.push(estDateString);
                }
            }
            
            // Combine historical and projection labels
            const allLabels = [...historicalLabels, ...projectionLabels];
            
            try {
                // Fetch current market data with force refresh to get latest
                const currentData = await fetchMarketData(currentSymbol, currentInterval, true);
                
                if (currentData && currentData.candles && currentData.candles.length > 0) {
                    // Convert saved date to timestamp for comparison
                    const savedTimestamp = savedDate.getTime();
                    
                    // Filter candles to only include those after the saved date
                    const actualCandles = currentData.candles.filter(c => {
                        const candleTimestamp = c.time || c.timestamp;
                        if (!candleTimestamp) return false;
                        // Compare timestamps directly
                        return candleTimestamp >= savedTimestamp;
                    });
                    
                    // Map actual prices to projection timeline
                    const actualPrices = [];
                    const actualLabels = [];
                    
                    // Create a map of actual prices by timestamp for quick lookup
                    const priceMap = new Map();
                    actualCandles.forEach(c => {
                        const timestamp = c.time || c.timestamp;
                        if (timestamp) {
                            priceMap.set(timestamp, parseFloat(c.close || c.price || 0));
                        }
                    });
                    
                    // Match actual prices to projection labels
                    for (let i = 0; i < projectionLabels.length; i++) {
                        const projectionLabel = projectionLabels[i];
                        const projectionDate = new Date(projectionStartDate.getTime() + (i + 1) * timeIncrementMs);
                        
                        // Find closest actual price within a reasonable window (e.g., ±50% of interval)
                        let closestPrice = null;
                        let closestTimestamp = null;
                        let minDiff = Infinity;
                        
                        priceMap.forEach((price, timestamp) => {
                            const diff = Math.abs(timestamp - projectionDate.getTime());
                            // Allow matching within 50% of interval
                            const window = timeIncrementMs * 0.5;
                            if (diff < window && diff < minDiff) {
                                minDiff = diff;
                                closestPrice = price;
                                closestTimestamp = timestamp;
                            }
                        });
                        
                        if (closestPrice && !isNaN(closestPrice) && closestPrice > 0) {
                            actualPrices.push(closestPrice);
                            actualLabels.push(projectionLabel);
                            // Remove matched price to avoid duplicates
                            priceMap.delete(closestTimestamp);
                        } else {
                            // No match found, use null to maintain alignment
                            actualPrices.push(null);
                            actualLabels.push(projectionLabel);
                        }
                    }
                    
                    // Filter out nulls but keep alignment info
                    actualPriceData = {
                        prices: actualPrices,
                        labels: actualLabels,
                        projectionLabels: projectionLabels
                    };
                    
                    console.log(`✓ Fetched ${actualPrices.filter(p => p !== null).length} actual price points for comparison`);
                } else {
                    console.warn('No current market data available for comparison');
                    actualPriceData = null;
                }
            } catch (error) {
                console.warn('Could not fetch actual price data:', error);
                actualPriceData = null;
            }
            
            // Render chart with both saved projection and actual data
            renderChart(
                historicalPrices, 
                allLabels, // Use combined labels
                savedProjectionLines, 
                savedParams,
                actualPriceData ? actualPriceData.prices : null,
                actualPriceData ? actualPriceData.labels : null
            );
            
            // Update metrics
            updateMetrics(historicalPrices, savedProjectionLines, savedParams);
            
            showLoading(false);
            hideError();
            
        } catch (error) {
            console.error('Error loading saved projection:', error);
            showError(error.message || 'Failed to load saved projection');
            showLoading(false);
        }
    }
    
    function updateMetrics(historicalPrices, projectionLines, params) {
        if (!historicalPrices || historicalPrices.length === 0) return;
        
        const firstPrice = historicalPrices[0];
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const change = lastPrice - firstPrice;
        const changePercent = firstPrice !== 0 ? (change / firstPrice) * 100 : 0;
        
        // Calculate Period High and Low from candles data
        let periodHigh = lastPrice;
        let periodLow = lastPrice;
        
        if (historicalCandles && historicalCandles.length > 0) {
            const highs = historicalCandles
                .map(c => parseFloat(c.high || c.close || 0))
                .filter(h => !isNaN(h) && h > 0);
            const lows = historicalCandles
                .map(c => parseFloat(c.low || c.close || 0))
                .filter(l => !isNaN(l) && l > 0);
            
            if (highs.length > 0) {
                periodHigh = Math.max(...highs);
            }
            if (lows.length > 0) {
                periodLow = Math.min(...lows);
            }
        } else {
            // Fallback to prices if candles not available
            periodHigh = Math.max(...historicalPrices);
            periodLow = Math.min(...historicalPrices);
        }
        
        // Calculate average projected price from all lines
        const allProjectedPrices = [];
        projectionLines.forEach(line => {
            if (line.points && line.points.length > 0) {
                allProjectedPrices.push(line.points[line.points.length - 1]);
            }
        });
        
        const avgProjectedPrice = allProjectedPrices.length > 0
            ? allProjectedPrices.reduce((a, b) => a + b, 0) / allProjectedPrices.length
            : lastPrice;
        
        const projectedChange = avgProjectedPrice - lastPrice;
        const projectedChangePercent = lastPrice !== 0 ? (projectedChange / lastPrice) * 100 : 0;
        
        // Update Current Price
        const currentPriceEl = document.getElementById('metric-current-price');
        if (currentPriceEl) {
            currentPriceEl.textContent = formatPrice(lastPrice);
            currentPriceEl.className = 'metric-value';
        }
        
        // Update Historical Change
        const histChangeEl = document.getElementById('metric-historical-change');
        if (histChangeEl) {
            const changeSign = change >= 0 ? '+' : '';
            histChangeEl.textContent = `${changeSign}${formatPrice(Math.abs(change))}`;
            histChangeEl.className = 'metric-value ' + (change >= 0 ? 'positive' : 'negative');
        }
        
        const histPercentEl = document.getElementById('metric-historical-percent');
        if (histPercentEl) {
            const percentSign = changePercent >= 0 ? '+' : '';
            histPercentEl.textContent = `${percentSign}${changePercent.toFixed(2)}%`;
            histPercentEl.className = 'metric-percent ' + (changePercent >= 0 ? 'positive' : 'negative');
        }
        
        // Update Projected Price
        const projectedPriceEl = document.getElementById('metric-projected-price');
        if (projectedPriceEl) {
            projectedPriceEl.textContent = formatPrice(avgProjectedPrice);
            projectedPriceEl.className = 'metric-value';
        }
        
        // Update Projected Change
        const projChangeEl = document.getElementById('metric-projected-change');
        if (projChangeEl) {
            const changeSign = projectedChange >= 0 ? '+' : '';
            projChangeEl.textContent = `${changeSign}${formatPrice(Math.abs(projectedChange))}`;
            projChangeEl.className = 'metric-value ' + (projectedChange >= 0 ? 'positive' : 'negative');
        }
        
        const projPercentEl = document.getElementById('metric-projected-percent');
        if (projPercentEl) {
            const percentSign = projectedChangePercent >= 0 ? '+' : '';
            projPercentEl.textContent = `${percentSign}${projectedChangePercent.toFixed(2)}%`;
            projPercentEl.className = 'metric-percent ' + (projectedChangePercent >= 0 ? 'positive' : 'negative');
        }
        
        // Update Period High
        const periodHighEl = document.getElementById('metric-period-high');
        if (periodHighEl) {
            periodHighEl.textContent = formatPrice(periodHigh);
            periodHighEl.className = 'metric-value';
        }
        
        // Update Period Low
        const periodLowEl = document.getElementById('metric-period-low');
        if (periodLowEl) {
            periodLowEl.textContent = formatPrice(periodLow);
            periodLowEl.className = 'metric-value';
        }
        
        // Format volume with K, M, B suffixes
        function formatVolume(volume) {
            if (volume === 0 || isNaN(volume)) return 'N/A';
            if (volume >= 1000000000) {
                return (volume / 1000000000).toFixed(2) + 'B';
            } else if (volume >= 1000000) {
                return (volume / 1000000).toFixed(2) + 'M';
            } else if (volume >= 1000) {
                return (volume / 1000).toFixed(2) + 'K';
            }
            return volume.toLocaleString();
        }
        
        // Calculate Average Volume
        let averageVolume = 0;
        let actualVolume = 0;
        if (historicalCandles && historicalCandles.length > 0) {
            const volumes = historicalCandles
                .map(c => parseFloat(c.volume || 0))
                .filter(v => !isNaN(v) && v > 0);
            
            if (volumes.length > 0) {
                averageVolume = volumes.reduce((a, b) => a + b, 0) / volumes.length;
                // Actual volume is the most recent (last) candle's volume
                actualVolume = volumes[volumes.length - 1] || 0;
            }
        }
        
        // Update Average Volume
        const avgVolumeEl = document.getElementById('metric-average-volume');
        if (avgVolumeEl) {
            avgVolumeEl.textContent = formatVolume(averageVolume);
            avgVolumeEl.className = 'metric-value';
        }
        
        // Update Actual Volume
        const actualVolumeEl = document.getElementById('metric-actual-volume');
        if (actualVolumeEl) {
            actualVolumeEl.textContent = formatVolume(actualVolume);
            actualVolumeEl.className = 'metric-value';
        }
    }
    
    // UI helpers
    function showLoading(show) {
        const loadingEl = document.getElementById('projection-loading');
        const chartWrapper = document.querySelector('.chart-wrapper');
        if (loadingEl) {
            loadingEl.style.display = show ? 'flex' : 'none';
        }
        // Hide chart canvas while loading
        const chartCanvas = document.getElementById('projection-chart');
        if (chartCanvas && chartWrapper) {
            chartCanvas.style.display = show ? 'none' : 'block';
        }
    }
    
    function showError(message) {
        const errorEl = document.getElementById('projection-error');
        const chartWrapper = document.querySelector('.chart-wrapper');
        if (errorEl) {
            errorEl.style.display = 'flex';
            const errorText = errorEl.querySelector('p');
            if (errorText) {
                errorText.textContent = message;
            }
        }
        // Hide chart canvas when showing error
        const chartCanvas = document.getElementById('projection-chart');
        if (chartCanvas && chartWrapper) {
            chartCanvas.style.display = 'none';
        }
        // Hide loading if showing error
        showLoading(false);
    }
    
    function hideError() {
        const errorEl = document.getElementById('projection-error');
        const chartCanvas = document.getElementById('projection-chart');
        if (errorEl) {
            errorEl.style.display = 'none';
        }
        // Show chart canvas when hiding error
        if (chartCanvas) {
            chartCanvas.style.display = 'block';
        }
    }
    
    /**
     * Reset zoom to original view
     */
    function resetZoom() {
        if (!projectionChart) {
            console.warn('No chart available to reset zoom');
            return;
        }
        
        try {
            // Try zoom plugin's resetZoom method first
            if (typeof projectionChart.resetZoom === 'function') {
                projectionChart.resetZoom();
                console.log('Zoom reset using plugin method');
                return;
            }
            
            // Fallback: reset scales manually
            const xScale = projectionChart.scales.x;
            const yScale = projectionChart.scales.y;
            
            if (xScale) {
                xScale.options.min = undefined;
                xScale.options.max = undefined;
            }
            if (yScale) {
                yScale.options.min = undefined;
                yScale.options.max = undefined;
            }
            
            projectionChart.update('none');
            console.log('Zoom reset using manual method');
        } catch (error) {
            console.error('Error resetting zoom:', error);
            // Fallback: update chart
            if (projectionChart) {
                projectionChart.update('none');
            }
        }
    }
    
    // Save projection
    async function saveProjection() {
        if (!currentSymbol || !historicalPrices || historicalPrices.length === 0) {
            showError('No projection data to save. Please load a projection first.');
            return;
        }
        
        try {
            // Get parameters (same logic as loadProjectionData)
            const selectedPreset = document.querySelector('input[name="projection-preset"]:checked');
            let params;
            
            if (selectedPreset && selectedPreset.value === 'custom') {
                params = {
                    steps: parseInt(document.getElementById('projection-steps').value) || 20,
                    base: parseFloat(document.getElementById('projection-base').value) || 3,
                    projectionCount: parseInt(document.getElementById('projection-count').value) || 12,
                    depthPrime: parseInt(document.getElementById('projection-depth').value) || 31
                };
            } else {
                params = {
                    steps: parseInt(document.getElementById('projection-steps-value').value) || 20,
                    base: parseFloat(document.getElementById('projection-base-value').value) || 3,
                    projectionCount: parseInt(document.getElementById('projection-count-value').value) || 12,
                    depthPrime: parseInt(document.getElementById('projection-depth-value').value) || 31
                };
            }
            
            const projectionLines = calculateProjections(historicalPrices, params);
            
            // Get chart data if available
            let chartData = null;
            if (projectionChart && projectionChart.data) {
                chartData = {
                    labels: projectionChart.data.labels,
                    datasets: projectionChart.data.datasets.map(ds => ({
                        label: ds.label,
                        data: ds.data,
                        borderColor: ds.borderColor,
                        backgroundColor: ds.backgroundColor,
                        borderWidth: ds.borderWidth,
                        pointRadius: ds.pointRadius,
                        tension: ds.tension
                    }))
                };
            }
            
            // Ensure all data is in EST before saving
            const estTimestamp = getCurrentEST().getTime();
            const estDateStr = getCurrentEST().toLocaleString('en-US', {
                timeZone: 'America/New_York',
                year: 'numeric',
                month: 'short',
                day: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                hour12: true
            }) + ' EST';
            
            // Generate projection labels for saving (so we can match actual prices later)
            const steps = params.steps || 20;
            const projectionLabels = generateProjectionLabels(steps, currentInterval, historicalLabels);
            
            const saveData = {
                symbol: currentSymbol,
                title: `${currentSymbol} - ${currentInterval} Projection (Saved ${estDateStr})`,
                projection_data: {
                    symbol: currentSymbol,
                    interval: currentInterval,
                    historicalPrices: historicalPrices,
                    historicalLabels: historicalLabels, // Already in EST format
                    projectionLines: projectionLines,
                    projectionLabels: projectionLabels, // Save projection labels for alignment
                    params: params,
                    lastPrice: historicalPrices[historicalPrices.length - 1],
                    timezone: 'EST',
                    savedAtEST: estDateStr,
                    savedTimestamp: estTimestamp // Save timestamp for accurate date matching
                },
                chart_data: chartData,
                params: params,
                notes: `Projection for ${currentSymbol} with ${params.steps} steps, base ${params.base}, depth ${params.depthPrime} | Timezone: EST | Saved: ${estDateStr}`
            };
            
            const response = await fetch('api/projections.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(saveData)
            });
            
            const result = await response.json();
            
            if (result.success) {
                // Show success message
                showSaveSuccess();
                
                // Refresh data page if it exists
                if (typeof DataPageModule !== 'undefined' && DataPageModule.refresh) {
                    // Check if data page is active
                    const dataPage = document.getElementById('page-data');
                    if (dataPage && dataPage.classList.contains('active')) {
                        DataPageModule.refresh();
                    }
                }
            } else {
                showError(result.message || 'Failed to save projection');
            }
        } catch (error) {
            console.error('Error saving projection:', error);
            showError('Failed to save projection. Please try again.');
        }
    }
    
    // Show save success message
    function showSaveSuccess() {
        const saveBtn = document.getElementById('save-projection-btn');
        if (saveBtn) {
            const originalHTML = saveBtn.innerHTML;
            saveBtn.innerHTML = '<i class="fas fa-check"></i> Saved!';
            saveBtn.style.background = 'var(--success-color)';
            saveBtn.disabled = true;
            
            setTimeout(() => {
                saveBtn.innerHTML = originalHTML;
                saveBtn.style.background = '';
                saveBtn.disabled = false;
            }, 2000);
        }
    }
    
    // Initialize ensemble with all models
    function initializeEnsemble() {
        try {
            // Check if models are available
            if (typeof ProjectionEnsemble === 'undefined' || 
                typeof CrystallineModel === 'undefined') {
                console.warn('Projection models not loaded, using original method');
                useEnsemble = false;
                return;
            }
            
            ensemble = new ProjectionEnsemble.Ensemble();
            
            // Add all available models
            if (typeof CrystallineModel !== 'undefined') {
                ensemble.addModel(new CrystallineModel());
            }
            if (typeof HarmonicModel !== 'undefined') {
                ensemble.addModel(new HarmonicModel());
            }
            if (typeof WaveBasedModel !== 'undefined') {
                ensemble.addModel(new WaveBasedModel());
            }
            if (typeof StatisticalModel !== 'undefined') {
                ensemble.addModel(new StatisticalModel());
            }
            
            console.log('Ensemble initialized with', ensemble.models.length, 'models');
        } catch (error) {
            console.error('Error initializing ensemble:', error);
            useEnsemble = false;
        }
    }
    
    // Convert ensemble result to projection lines format
    function convertEnsembleToProjectionLines(ensembleResult) {
        const projectionLines = [];
        
        // Add ensemble projection as main line
        projectionLines.push({
            triad: ['Ensemble'],
            points: ensembleResult.points,
            confidence: ensembleResult.confidence,
            isEnsemble: true
        });
        
        // Add individual model projections if available
        if (ensembleResult.individualProjections) {
            ensembleResult.individualProjections.forEach((proj, idx) => {
                projectionLines.push({
                    triad: [proj.model],
                    points: proj.points,
                    confidence: proj.confidence,
                    isIndividual: true
                });
            });
        }
        
        return projectionLines;
    }
    
    /**
     * Update validation metrics display
     * Matches the design of Price Metrics section (same card style)
     */
    function updateValidationMetrics(validation) {
        const validationSection = document.getElementById('validation-metrics-section');
        if (!validationSection) {
            console.warn('Validation metrics section not found in DOM');
            return;
        }
        
        if (!validation || validation.error) {
            validationSection.style.display = 'none';
            return;
        }
        
        // Show validation section
        validationSection.style.display = 'block';
        
        // Update MAE (Mean Absolute Error) - lower is better
        const maeEl = document.getElementById('validation-mae');
        if (maeEl) {
            if (validation.mae !== null && validation.mae !== undefined && !isNaN(validation.mae)) {
                maeEl.textContent = validation.mae.toFixed(4);
                // Color code: lower MAE = better
                // < 0.5 = excellent (green), 0.5-1.0 = good (neutral), > 1.0 = poor (red)
                const maeClass = validation.mae < 0.5 ? 'positive' : validation.mae < 1.0 ? '' : 'negative';
                maeEl.className = 'metric-value ' + maeClass;
            } else {
                maeEl.textContent = 'N/A';
                maeEl.className = 'metric-value';
            }
        }
        
        // Update RMSE (Root Mean Squared Error) - lower is better
        const rmseEl = document.getElementById('validation-rmse');
        if (rmseEl) {
            if (validation.rmse !== null && validation.rmse !== undefined && !isNaN(validation.rmse)) {
                rmseEl.textContent = validation.rmse.toFixed(4);
                // Color code: lower RMSE = better
                // < 0.6 = excellent (green), 0.6-1.0 = good (neutral), > 1.0 = poor (red)
                const rmseClass = validation.rmse < 0.6 ? 'positive' : validation.rmse < 1.0 ? '' : 'negative';
                rmseEl.className = 'metric-value ' + rmseClass;
            } else {
                rmseEl.textContent = 'N/A';
                rmseEl.className = 'metric-value';
            }
        }
        
        // Update MAPE (Mean Absolute Percentage Error) - lower is better
        const mapeEl = document.getElementById('validation-mape');
        if (mapeEl) {
            if (validation.mape !== null && validation.mape !== undefined && !isNaN(validation.mape)) {
                mapeEl.textContent = validation.mape.toFixed(2) + '%';
                // Color code based on MAPE value (lower is better)
                // < 1% = excellent (green), 1-5% = good (neutral), > 5% = poor (red)
                const mapeClass = validation.mape < 1 ? 'positive' : validation.mape < 5 ? '' : 'negative';
                mapeEl.className = 'metric-value ' + mapeClass;
            } else {
                mapeEl.textContent = 'N/A';
                mapeEl.className = 'metric-value';
            }
        }
        
        // Update Confidence - higher is better
        const confidenceEl = document.getElementById('validation-confidence');
        if (confidenceEl) {
            let confidenceValue = 0.5; // Default
            
            if (validation.errorStdDev !== null && validation.errorStdDev !== undefined && !isNaN(validation.errorStdDev)) {
                // Calculate confidence: lower error = higher confidence
                // Normalize errorStdDev to 0-1 range (assuming max reasonable error of 10)
                const normalizedError = Math.min(validation.errorStdDev / 10, 1);
                confidenceValue = Math.max(0, Math.min(1, 1 - normalizedError));
            } else if (validation.confidenceIntervals && validation.confidenceIntervals.length > 0) {
                // Use confidence from intervals if available
                confidenceValue = validation.confidenceIntervals[0].confidence || 0.95;
            } else if (validation.confidence !== null && validation.confidence !== undefined && !isNaN(validation.confidence)) {
                // Use direct confidence value if available
                confidenceValue = validation.confidence;
            }
            
            confidenceEl.textContent = confidenceValue.toFixed(2);
            // Color code: higher confidence = better (green)
            // >= 0.8 = excellent (green), 0.6-0.8 = good (neutral), < 0.6 = poor (red)
            const confClass = confidenceValue >= 0.8 ? 'positive' : confidenceValue >= 0.6 ? '' : 'negative';
            confidenceEl.className = 'metric-value ' + confClass;
        }
    }
    
    /**
     * Preset parameter configurations
     */
    const PRESET_CONFIGS = {
        standard: {
            steps: 20,
            base: 3,
            projectionCount: 12,
            depthPrime: 31
        },
        extended: {
            steps: 40,
            base: 3.5,
            projectionCount: 18,
            depthPrime: 47
        },
        deep: {
            steps: 60,
            base: 4,
            projectionCount: 24,
            depthPrime: 61
        },
        quick: {
            steps: 10,
            base: 2.5,
            projectionCount: 6,
            depthPrime: 17
        },
        maximum: {
            steps: 100,
            base: 5,
            projectionCount: 36,
            depthPrime: 97
        }
    };
    
    /**
     * Apply preset configuration
     */
    function applyPreset(presetName) {
        const config = PRESET_CONFIGS[presetName];
        if (!config) {
            console.warn(`Preset configuration not found: ${presetName}`);
            return;
        }
        
        // Update hidden inputs (these are used by non-custom presets)
        const stepsValueInput = document.getElementById('projection-steps-value');
        const baseValueInput = document.getElementById('projection-base-value');
        const countValueInput = document.getElementById('projection-count-value');
        const depthValueInput = document.getElementById('projection-depth-value');
        
        if (stepsValueInput) stepsValueInput.value = config.steps;
        if (baseValueInput) baseValueInput.value = config.base;
        if (countValueInput) countValueInput.value = config.projectionCount;
        if (depthValueInput) depthValueInput.value = config.depthPrime;
        
        // If custom panel is visible, also update those inputs for consistency
        const customPanel = document.getElementById('custom-params-panel');
        if (customPanel && customPanel.style.display !== 'none') {
            const stepsInput = document.getElementById('projection-steps');
            const baseInput = document.getElementById('projection-base');
            const countInput = document.getElementById('projection-count');
            const depthSelect = document.getElementById('projection-depth');
            
            if (stepsInput) stepsInput.value = config.steps;
            if (baseInput) baseInput.value = config.base;
            if (countInput) countInput.value = config.projectionCount;
            if (depthSelect) depthSelect.value = config.depthPrime;
        }
        
        // Update active parameters display
        updateActiveParamsDisplay(config);
        
        // Log applied configuration
        console.log(`Applied preset "${presetName}":`, {
            steps: config.steps,
            base: config.base,
            projectionCount: config.projectionCount,
            depthPrime: config.depthPrime
        });
    }
    
    /**
     * Update active parameters display
     */
    function updateActiveParamsDisplay(params) {
        const display = document.getElementById('active-params-display');
        const stepsEl = document.getElementById('active-steps');
        const baseEl = document.getElementById('active-base');
        const countEl = document.getElementById('active-count');
        const depthEl = document.getElementById('active-depth');
        
        if (display && stepsEl && baseEl && countEl && depthEl) {
            stepsEl.textContent = params.steps || 20;
            baseEl.textContent = params.base || 3;
            countEl.textContent = params.projectionCount || 12;
            depthEl.textContent = params.depthPrime || 31;
            display.style.display = 'flex';
        }
    }
    
    /**
     * Setup parameter toggle switches
     */
    function setupParameterToggles() {
        const presetRadios = document.querySelectorAll('input[name="projection-preset"]');
        const customPanel = document.getElementById('custom-params-panel');
        
        presetRadios.forEach(radio => {
            radio.addEventListener('change', function() {
                const presetName = this.value;
                
                // Update active state for visual feedback
                document.querySelectorAll('.param-toggle-item').forEach(item => {
                    item.classList.remove('active');
                });
                const toggleItem = this.closest('.param-toggle-item');
                if (toggleItem) {
                    toggleItem.classList.add('active');
                }
                
                if (presetName === 'custom') {
                    // Show custom parameters panel
                    if (customPanel) {
                        customPanel.style.display = 'block';
                    }
                } else {
                    // Hide custom parameters panel
                    if (customPanel) {
                        customPanel.style.display = 'none';
                    }
                    
                    // Apply preset configuration
                    applyPreset(presetName);
                    
                    // Log preset change for debugging
                    console.log(`Preset changed to: ${presetName}`, PRESET_CONFIGS[presetName]);
                    
                    // Automatically trigger projection update if data is loaded
                    const symbolInput = document.getElementById('projection-symbol-input');
                    if (symbolInput && symbolInput.value.trim() !== '') {
                        // Check if we have historical data loaded
                        if (historicalPrices && historicalPrices.length > 0) {
                            // Trigger projection update automatically with a slight delay
                            // to ensure DOM updates are complete
                            setTimeout(() => {
                                console.log('Auto-updating projection with new preset parameters');
                                loadProjectionData();
                            }, 150);
                        }
                    }
                }
            });
            
            // Set initial active state
            if (radio.checked) {
                const toggleItem = radio.closest('.param-toggle-item');
                if (toggleItem) {
                    toggleItem.classList.add('active');
                }
            }
        });
        
        // Setup custom parameter inputs to update hidden values
        const stepsInput = document.getElementById('projection-steps');
        const baseInput = document.getElementById('projection-base');
        const countInput = document.getElementById('projection-count');
        const depthSelect = document.getElementById('projection-depth');
        
        if (stepsInput) {
            stepsInput.addEventListener('input', function() {
                document.getElementById('projection-steps-value').value = this.value;
            });
        }
        
        if (baseInput) {
            baseInput.addEventListener('input', function() {
                document.getElementById('projection-base-value').value = this.value;
            });
        }
        
        if (countInput) {
            countInput.addEventListener('input', function() {
                document.getElementById('projection-count-value').value = this.value;
            });
        }
        
        if (depthSelect) {
            depthSelect.addEventListener('change', function() {
                document.getElementById('projection-depth-value').value = this.value;
            });
        }
        
        // Apply default preset (standard)
        const defaultPreset = document.querySelector('input[name="projection-preset"]:checked');
        if (defaultPreset && defaultPreset.value !== 'custom') {
            applyPreset(defaultPreset.value);
        }
    }
    
    // Initialize
    function init() {
        const searchBtn = document.getElementById('projection-search-btn');
        const refreshBtn = document.getElementById('projection-refresh-btn');
        const resetZoomBtn = document.getElementById('reset-zoom-btn');
        const saveBtn = document.getElementById('save-projection-btn');
        const symbolInput = document.getElementById('projection-symbol-input');
        
        if (searchBtn) {
            searchBtn.addEventListener('click', loadProjectionData);
        }
        
        if (refreshBtn) {
            refreshBtn.addEventListener('click', () => {
                // Force refresh with cache bypass
                loadProjectionData(true);
            });
        }
        
        if (resetZoomBtn) {
            resetZoomBtn.addEventListener('click', resetZoom);
        }
        
        if (saveBtn) {
            saveBtn.addEventListener('click', saveProjection);
        }
        
        if (symbolInput) {
            symbolInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    loadProjectionData();
                }
            });
        }
        
        // Setup timeframe buttons (matching Trading Charts tab)
        document.querySelectorAll('.projection-timeframe-selector .timeframe-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                // Remove active class from all timeframe buttons
                document.querySelectorAll('.projection-timeframe-selector .timeframe-btn').forEach(b => b.classList.remove('active'));
                // Add active class to clicked button
                btn.classList.add('active');
                // Update current interval
                currentInterval = btn.dataset.timeframe;
                // Reload projection data if we have a symbol loaded
                if (currentSymbol) {
                    loadProjectionData();
                }
            });
        });
        
        // Setup parameter toggle switches
        setupParameterToggles();
        
        // Watch for page activation
        watchPageActivation();
    }
    
        // Watch for page activation (now watches for tab activation)
        function watchPageActivation() {
            const chartsPage = document.getElementById('page-charts');
            const projectionsTab = document.getElementById('tab-projections');
            
            if (!chartsPage || !projectionsTab) return;
            
            // Check if charts page is active and projections tab is visible
            function checkAndLoad() {
                const isActive = chartsPage.classList.contains('active') && projectionsTab.classList.contains('active');
                
                if (isActive) {
                    if (!currentSymbol || currentSymbol === '') {
                        autoLoadSPY();
                    } else {
                        // Restart auto-refresh if we have a symbol and chart
                        if (projectionChart) {
                            setupAutoRefresh();
                        }
                    }
                } else {
                    // Stop auto-refresh when tab is not active
                    stopAutoRefresh();
                }
            }
            
            // Check on init
            checkAndLoad();
            
            // Watch for charts page activation
            const chartsObserver = new MutationObserver((mutations) => {
                mutations.forEach((mutation) => {
                    if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                        checkAndLoad();
                    }
                });
            });
            
            chartsObserver.observe(chartsPage, {
                attributes: true,
                attributeFilter: ['class']
            });
            
            // Watch for tab activation
            const tabObserver = new MutationObserver((mutations) => {
                mutations.forEach((mutation) => {
                    if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                        checkAndLoad();
                    }
                });
            });
            
            tabObserver.observe(projectionsTab, {
                attributes: true,
                attributeFilter: ['class']
            });
            
            // Stop auto-refresh when page is hidden (browser tab switch)
            document.addEventListener('visibilitychange', () => {
                if (document.hidden) {
                    stopAutoRefresh();
                } else {
                    // Restart if tab is active
                    checkAndLoad();
                }
            });
        }
    
    // Auto-load SPY on projections page
    function autoLoadSPY() {
        const symbolInput = document.getElementById('projection-symbol-input');
        if (!symbolInput) return;
        
        // Only auto-load if input is empty
        if (!symbolInput.value.trim()) {
            symbolInput.value = 'SPY';
            // Small delay to ensure DOM is ready
            setTimeout(() => {
                loadProjectionData();
            }, 100);
        }
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    return {
        init: init,
        loadProjectionData: loadProjectionData,
        autoLoadSPY: autoLoadSPY,
        loadSavedProjectionWithActual: loadSavedProjectionWithActual
    };
})();

