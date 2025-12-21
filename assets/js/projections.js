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
    let currentInterval = '1H'; // ONLY 1H TIMEFRAME ALLOWED
    let historicalPrices = [];
    let historicalLabels = [];
    let historicalCandles = [];
    let validationResults = null;
    let savedProjectionData = null; // Store saved projection when loading
    let actualPriceData = null; // Store actual price data for comparison
    let refreshInterval = null; // Auto-refresh interval
    let isRefreshing = false; // Prevent concurrent refreshes
    let currentProjectionParams = null; // Store current projection parameters for export
    let currentProjectionLines = []; // Store current projection lines for CSV export
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
    
    // Format volume with K, M, B suffixes
    // CRITICAL: Define early so it's accessible in all callbacks
    function formatVolume(volume) {
        if (volume === null || volume === undefined || volume === 0 || isNaN(volume)) return '0';
        if (volume >= 1000000000) {
            return (volume / 1000000000).toFixed(2) + 'B';
        } else if (volume >= 1000000) {
            return (volume / 1000000).toFixed(2) + 'M';
        } else if (volume >= 1000) {
            return (volume / 1000).toFixed(2) + 'K';
        }
        return volume.toLocaleString();
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
        
        // FIXED: Start from actual last candle timestamp, not a fixed date
        let startTimestamp;
        if (historicalCandles && historicalCandles.length > 0) {
            const lastCandle = historicalCandles[historicalCandles.length - 1];
            startTimestamp = lastCandle.time || lastCandle.timestamp;
        }
        
        // If no candle timestamp, use current time (when data is loaded)
        if (!startTimestamp || startTimestamp <= 0) {
            const now = new Date();
            startTimestamp = alignToTimeframeBoundary(now.getTime(), interval);
        } else {
            // Align to timeframe boundary
            startTimestamp = alignToTimeframeBoundary(startTimestamp, interval);
        }
        
        // Calculate time increment based on interval
        let timeIncrement = 60 * 60 * 1000; // Default: 1 hour in milliseconds
        if (interval === '15MIN') {
            timeIncrement = 15 * 60 * 1000; // 15 minutes
        } else if (interval === '1H') {
            timeIncrement = 60 * 60 * 1000; // 1 hour
        } else if (interval === '4H') {
            timeIncrement = 4 * 60 * 60 * 1000; // 4 hours
        } else if (interval === '1D') {
            timeIncrement = 24 * 60 * 60 * 1000; // 1 day
        }
        
        // Track last day for projection labels to show dates on new days
        let lastProjectionDay = null;
        if (historicalCandles && historicalCandles.length > 0) {
            const lastCandle = historicalCandles[historicalCandles.length - 1];
            const lastTimestamp = lastCandle.time || lastCandle.timestamp;
            if (lastTimestamp) {
                const lastDate = new Date(lastTimestamp);
                const lastDateString = lastDate.toLocaleString('en-US', {
                    timeZone: 'America/New_York',
                    year: 'numeric',
                    month: 'short',
                    day: 'numeric'
                });
                lastProjectionDay = lastDateString.split(', ')[0]; // e.g., "Dec 19, 2025" -> "Dec 19"
            }
        }
        
        for (let i = 1; i <= steps; i++) {
            // Calculate projected time: start from last candle + (interval * i)
            const projectedTime = startTimestamp + (timeIncrement * i);
            const projectedDate = new Date(projectedTime);
            
            // Get date components in EST timezone
            const estDateString = projectedDate.toLocaleString('en-US', {
                timeZone: 'America/New_York',
                year: 'numeric',
                month: 'short',
                day: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                hour12: false
            });
            
            // Parse to get day
            const [datePart, timePart] = estDateString.split(', ');
            const currentDay = datePart; // e.g., "Dec 19, 2025"
            
            // Check if this is a new day
            const isNewDay = lastProjectionDay === null || currentDay !== lastProjectionDay;
            if (isNewDay) {
                lastProjectionDay = currentDay;
            }
            
            // Format the date in EST timezone
            // For intraday timeframes (15MIN, 1H, 4H), show date when new day appears
            let formattedDate;
            if (interval === '15MIN' || interval === '1H' || interval === '4H') {
                if (isNewDay) {
                    // Show date and time for new day: "Dec 19\n14:00" or "Dec 19, 14:00"
                    formattedDate = `${datePart}\n${timePart}`;
                } else {
                    // Same day, just show time
                    formattedDate = timePart;
                }
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
            const pricePoint = trunc(lastPrice + delta, decimals);
            points.push(pricePoint);
        }
        
        return points;
    }
    
    // Calculate projections using multiple triads
    /**
     * Calculate projections using CLLM with 88D threading via PHP backend
     * 
     * This function calls the PHP backend which uses C functions from the math2 library.
     * Falls back to JavaScript implementation if PHP backend fails.
     */
    async function calculateProjections(historicalPrices, params) {
        // CRITICAL: Always use the current/last price as the base for projections
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        
        // Validate last price
        if (!lastPrice || isNaN(lastPrice) || lastPrice <= 0) {
            throw new Error('Invalid current price: cannot project from invalid price');
        }
        
        const depthPrime = parseInt(params.depthPrime) || 31;
        // FIXED: Always use current price as base, ignore params.base (which is for algorithm tuning, not starting price)
        const base = lastPrice; // Always project from current price
        const projectionCount = parseInt(params.projectionCount) || 12;
        const steps = parseInt(params.steps) || 20;
        const omegaHz = parseFloat(params.omegaHz) || 432.0;
        
        console.log(`📊 Projecting from current price: $${lastPrice.toFixed(2)} (base parameter: ${base})`);
        
        // Try PHP backend first (uses CLLM with 88D threading)
        try {
            console.log('Calling PHP backend for price projections (CLLM with 88D threading)...');
            
            const response = await fetch('api/price_projection.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({
                    historical_prices: historicalPrices,
                    depth_prime: depthPrime,
                    base: base, // Current price - projections start from here
                    steps: steps,
                    projection_count: projectionCount,
                    omega_hz: omegaHz,
                    decimals: 8
                })
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.success && result.projection_lines) {
                const method = result.method || 'CLLM';
                const methodDisplay = method === 'ffi' ? 'CLLM (FFI - Direct C)' : 
                                     method === 'exec' ? 'CLLM (Exec - C Binary)' : 
                                     method === 'fallback' ? 'PHP Fallback' : 'CLLM';
                
                const numLines = result.num_lines || result.projection_lines.length;
                const stepsPerLine = result.steps_per_line || (result.projection_lines[0] ? result.projection_lines[0].length : 0);
                
                console.log(`✓ Projections computed via ${methodDisplay} (${numLines} lines, ${stepsPerLine} steps each)`);
                
                // Store method info for UI display
                window.lastProjectionMethod = {
                    method: method,
                    display: methodDisplay,
                    uses88D: method === 'ffi' || method === 'exec',
                    numLines: numLines,
                    stepsPerLine: stepsPerLine
                };
                
                // Show status notification
                showProjectionStatus(methodDisplay, method === 'ffi' || method === 'exec');
                
                // Convert to expected format - ensure all points are valid numbers
                const projectionLines = result.projection_lines.map((points, idx) => {
                    // Ensure points is an array
                    if (!Array.isArray(points)) {
                        console.warn(`Projection line ${idx} is not an array:`, points);
                        return null;
                    }
                    
                    // Filter and validate points
                    const validPoints = points
                        .map(p => {
                            const num = typeof p === 'number' ? p : parseFloat(p);
                            return !isNaN(num) && num > 0 ? num : null;
                        })
                        .filter(p => p !== null);
                    
                    if (validPoints.length === 0) {
                        console.warn(`Projection line ${idx} has no valid points`);
                        return null;
                    }
                    
                    // Generate triad for this projection (for display purposes)
                    const triads = generateTriadsAroundPrime(depthPrime, projectionCount, PRIMES_500);
                    const triad = triads[idx] || [2, 5, 7];
                    
                    return {
                        triad: triad,
                        points: validPoints
                    };
                }).filter(line => line !== null); // Remove invalid lines
                
                if (projectionLines.length === 0) {
                    throw new Error('No valid projection lines generated');
                }
                
                console.log(`✓ Converted ${projectionLines.length} valid projection lines for display`);
                
                return projectionLines;
            } else {
                throw new Error(result.error || result.message || 'Unknown error from PHP backend');
            }
            
        } catch (error) {
            console.warn('PHP backend failed, falling back to JavaScript implementation:', error);
            
            // Store method info
            window.lastProjectionMethod = {
                method: 'js',
                display: 'JavaScript Fallback',
                uses88D: false,
                numLines: 0,
                stepsPerLine: 0
            };
            
            // Show status notification
            showProjectionStatus('JavaScript Fallback', false);
            
            // Fallback to JavaScript implementation
            return calculateProjectionsJS(historicalPrices, params);
        }
    }
    
    /**
     * Update computation method display in UI
     */
    function updateComputationMethodDisplay() {
        const infoEl = document.getElementById('projection-computation-info');
        const methodTextEl = document.getElementById('computation-method-text');
        const methodIconEl = document.getElementById('computation-method-icon');
        const badgeEl = document.getElementById('computation-88d-badge');
        
        if (!infoEl || !methodTextEl || !methodIconEl) return;
        
        const methodInfo = window.lastProjectionMethod;
        if (!methodInfo) {
            infoEl.style.display = 'none';
            return;
        }
        
        infoEl.style.display = 'block';
        methodTextEl.textContent = methodInfo.display;
        
        if (methodInfo.uses88D) {
            methodIconEl.textContent = '⚡';
            methodIconEl.style.color = '#22c55e';
            if (badgeEl) {
                badgeEl.style.display = 'block';
            }
        } else {
            methodIconEl.textContent = '📊';
            methodIconEl.style.color = '';
            if (badgeEl) {
                badgeEl.style.display = 'none';
            }
        }
    }
    
    /**
     * Show projection computation status
     */
    function showProjectionStatus(method, uses88D) {
        // Create or update status indicator
        let statusEl = document.getElementById('projection-method-status');
        if (!statusEl) {
            statusEl = document.createElement('div');
            statusEl.id = 'projection-method-status';
            statusEl.className = 'projection-method-status';
            statusEl.style.cssText = `
                position: fixed;
                top: 80px;
                right: 20px;
                background: var(--dark-bg, #1a1a1a);
                border: 1px solid var(--border-color, #333);
                border-radius: 8px;
                padding: 12px 16px;
                z-index: 10000;
                box-shadow: 0 4px 6px rgba(0,0,0,0.3);
                font-size: 13px;
                max-width: 300px;
                transition: opacity 0.3s;
            `;
            document.body.appendChild(statusEl);
        }
        
        const icon = uses88D ? '⚡' : '📊';
        const badge = uses88D ? '<span style="color: #22c55e; font-weight: bold;">88D Active</span>' : '';
        statusEl.innerHTML = `
            <div style="display: flex; align-items: center; gap: 8px;">
                <span style="font-size: 18px;">${icon}</span>
                <div>
                    <div style="font-weight: 600; color: var(--text-color, #e0e0e0);">${method}</div>
                    ${badge ? `<div style="font-size: 11px; margin-top: 4px;">${badge}</div>` : ''}
                </div>
            </div>
        `;
        
        // Auto-hide after 5 seconds
        setTimeout(() => {
            if (statusEl) {
                statusEl.style.opacity = '0';
                setTimeout(() => {
                    if (statusEl && statusEl.parentNode) {
                        statusEl.parentNode.removeChild(statusEl);
                    }
                }, 300);
            }
        }, 5000);
    }
    
    /**
     * JavaScript fallback implementation (original code)
     * This is kept as a fallback if PHP backend is unavailable
     */
    function calculateProjectionsJS(historicalPrices, params) {
        // CRITICAL: Always use the current/last price as the base for projections
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        
        // Validate last price
        if (!lastPrice || isNaN(lastPrice) || lastPrice <= 0) {
            throw new Error('Invalid current price: cannot project from invalid price');
        }
        
        const depthPrime = parseInt(params.depthPrime) || 31;
        // Note: params.base is for algorithm tuning, not the starting price
        // Always project from current price (lastPrice)
        const projectionCount = parseInt(params.projectionCount) || 12;
        const steps = parseInt(params.steps) || 20;
        
        console.log(`📊 JS Fallback: Projecting from current price: $${lastPrice.toFixed(2)}`);
        
        // Generate triads around the depth prime
        const triads = generateTriadsAroundPrime(depthPrime, projectionCount, PRIMES_500);
        
        const projectionLines = [];
        
        triads.forEach((triad, idx) => {
            const projectedPrices = computeCrystallineProjection({
                lastPrice: lastPrice, // Always use current price
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
    
    /**
     * Align timestamp to timeframe boundary
     * Ensures candles are properly aligned to their timeframe (e.g., 1H candles at :00 minutes)
     * Uses proper EST/EDT timezone handling
     */
    function alignToTimeframeBoundary(timestamp, timeframe) {
        const date = new Date(timestamp);
        if (isNaN(date.getTime())) return timestamp;
        
        // Get date components in EST timezone
        const estFormatter = new Intl.DateTimeFormat('en-US', {
            timeZone: 'America/New_York',
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            hour12: false
        });
        
        const parts = estFormatter.formatToParts(date);
        const getPart = (type) => {
            const part = parts.find(p => p.type === type);
            return part ? part.value : '0';
        };
        
        let year = parseInt(getPart('year'));
        let month = parseInt(getPart('month'));
        let day = parseInt(getPart('day'));
        let hour = parseInt(getPart('hour'));
        let minute = parseInt(getPart('minute'));
        let second = parseInt(getPart('second'));
        
        // Align to timeframe boundaries
        switch (timeframe) {
            case '15MIN':
                // Align to 15-minute boundaries (0, 15, 30, 45)
                minute = Math.floor(minute / 15) * 15;
                second = 0;
                break;
            case '1H':
                // Align to hour boundaries (:00)
                minute = 0;
                second = 0;
                break;
            case '4H':
                // Align to 4-hour boundaries (0, 4, 8, 12, 16, 20)
                hour = Math.floor(hour / 4) * 4;
                minute = 0;
                second = 0;
                break;
            case '1D':
                // Align to day boundaries (market open: 9:30 AM EST)
                hour = 9;
                minute = 30;
                second = 0;
                break;
            default:
                // Default to hour boundary
                minute = 0;
                second = 0;
        }
        
        // Create date string in EST format
        const dateStr = `${year}-${month.toString().padStart(2, '0')}-${day.toString().padStart(2, '0')}T${hour.toString().padStart(2, '0')}:${minute.toString().padStart(2, '0')}:${second.toString().padStart(2, '0')}`;
        
        // Check if DST (EDT) by creating a test date
        const testDate = new Date(`${year}-${month.toString().padStart(2, '0')}-${day.toString().padStart(2, '0')}T12:00:00`);
        const dstCheck = testDate.toLocaleString('en-US', { 
            timeZone: 'America/New_York', 
            timeZoneName: 'short' 
        });
        const isDST = dstCheck.includes('EDT');
        
        // Create timestamp with proper timezone offset
        // EST is UTC-5, EDT is UTC-4
        const offset = isDST ? '-04:00' : '-05:00';
        const alignedDate = new Date(`${dateStr}${offset}`);
        
        return alignedDate.getTime();
    }
    
    /**
     * Process and validate candle data for proper timeframe alignment
     * Ensures each candle has correct timestamp aligned to timeframe boundaries
     */
    function processCandlesForTimeframe(candles, timeframe) {
        if (!candles || !Array.isArray(candles) || candles.length === 0) {
            return [];
        }
        
        const processedCandles = candles.map(candle => {
            if (!candle) return null;
            
            // Get timestamp
            let timestamp = candle.time || candle.timestamp || candle.originalTime;
            if (!timestamp) return null;
            
            // Convert to number if string
            if (typeof timestamp === 'string') {
                timestamp = parseInt(timestamp);
            }
            
            // Validate timestamp
            if (isNaN(timestamp) || timestamp <= 0) {
                return null;
            }
            
            // Align timestamp to timeframe boundary
            const alignedTimestamp = alignToTimeframeBoundary(timestamp, timeframe);
            
            // Get price data
            const close = parseFloat(candle.close || candle.price || 0);
            const open = parseFloat(candle.open || close);
            const high = parseFloat(candle.high || close);
            const low = parseFloat(candle.low || close);
            const volume = parseFloat(candle.volume || 0);
            
            // Validate price data
            if (isNaN(close) || close <= 0) {
                return null;
            }
            
            // Ensure proper OHLC relationships
            const validHigh = Math.max(open, high, close);
            const validLow = Math.min(open, low, close);
            
            return {
                time: alignedTimestamp,
                timestamp: alignedTimestamp,
                originalTime: timestamp,
                open: open,
                high: validHigh,
                low: validLow,
                close: close,
                price: close,
                volume: volume
            };
        }).filter(c => c !== null);
        
        // Sort by timestamp (oldest first)
        processedCandles.sort((a, b) => a.time - b.time);
        
        // Remove duplicates (same aligned timestamp)
        const uniqueCandles = [];
        const seenTimestamps = new Set();
        
        for (const candle of processedCandles) {
            if (!seenTimestamps.has(candle.time)) {
                seenTimestamps.add(candle.time);
                uniqueCandles.push(candle);
            }
        }
        
        return uniqueCandles;
    }
    
    /**
     * Fetch market data with real-time support and proper timeframe alignment
     * Redesigned to ensure correct time and price per timeframe
     */
    async function fetchMarketData(symbol, interval, forceRefresh = false) {
        try {
            // Validate inputs
            if (!symbol || typeof symbol !== 'string' || symbol.trim() === '') {
                throw new Error('Invalid symbol provided');
            }
            
            // Interval is now already in timeframe format (15MIN, 1H, 4H, 1D)
            const timeframe = interval || '1H'; // Default to 1H for projections
            
            // CRITICAL: For price projections, fetch last 7 days of data
            // Build URL with days parameter to fetch 7 days
            const url = `api/charts.php?action=chart&symbol=${encodeURIComponent(symbol)}&timeframe=${encodeURIComponent(timeframe)}&days=7${forceRefresh ? '&refresh=true' : ''}`;
            
            console.log('📊 Fetching market data:', { symbol, timeframe, forceRefresh, url });
            
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
            console.log('📈 Chart API response:', {
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
            
            // Process candles for proper timeframe alignment
            console.log(`🔄 Processing ${data.candles.length} candles for timeframe ${timeframe}...`);
            const processedCandles = processCandlesForTimeframe(data.candles, timeframe);
            
            if (processedCandles.length === 0) {
                throw new Error('No valid candle data after processing. Please try again.');
            }
            
            // Update data with processed candles
            data.candles = processedCandles;
            
            console.log(`✅ Successfully processed ${processedCandles.length} valid candles for ${symbol} (${timeframe})`);
            console.log(`📅 Time range: ${new Date(processedCandles[0].time).toLocaleString('en-US', { timeZone: 'America/New_York' })} to ${new Date(processedCandles[processedCandles.length - 1].time).toLocaleString('en-US', { timeZone: 'America/New_York' })} EST`);
            
            return data;
        } catch (error) {
            console.error('❌ Error fetching market data:', error);
            throw error;
        }
    }
    
    /**
     * Fetch real-time market quote with enhanced error handling
     * This ensures we always get the latest market data for accurate projections
     */
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
        // FORCE 1H TIMEFRAME - only allowed timeframe
        currentInterval = '1H';
        
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
            
            /**
             * REDESIGNED: Process candles with 100% accurate time and price data
             * Ensures proper OHLC relationships and correct timestamp alignment
             */
            historicalCandles = data.candles.map((c, index) => {
                if (!c) return null;
                
                // Get and validate timestamp - CRITICAL for accurate time display
                let timestamp = c.time || c.timestamp || c.originalTime;
                if (!timestamp || typeof timestamp !== 'number' || timestamp <= 0) {
                    console.warn(`Invalid timestamp at index ${index}:`, c);
                    return null;
                }
                
                // Align timestamp to 1H timeframe boundary (hour boundaries :00)
                const alignedTimestamp = alignToTimeframeBoundary(timestamp, currentInterval);
                
                // Get and validate price data
                const close = parseFloat(c.close || c.price || 0);
                if (isNaN(close) || close <= 0) {
                    console.warn(`Invalid close price at index ${index}:`, c);
                    return null;
                }
                
                // Get OHLC values with proper validation
                let open = parseFloat(c.open);
                let high = parseFloat(c.high);
                let low = parseFloat(c.low);
                
                // If open is invalid, use previous candle's close or current close
                if (isNaN(open) || open <= 0) {
                    if (index > 0 && historicalCandles[index - 1] && historicalCandles[index - 1].close) {
                        open = parseFloat(historicalCandles[index - 1].close);
                    } else {
                        open = close;
                    }
                }
                
                // Validate and correct high: must be >= max(open, close)
                const minHigh = Math.max(open, close);
                if (isNaN(high) || high < minHigh) {
                    high = minHigh;
                }
                // Ensure high is at least the maximum of all values
                high = Math.max(high, open, close);
                
                // Validate and correct low: must be <= min(open, close)
                const maxLow = Math.min(open, close);
                if (isNaN(low) || low > maxLow) {
                    low = maxLow;
                }
                // Ensure low is at most the minimum of all values
                low = Math.min(low, open, close);
                
                // Final validation: prevent doji candles (all values equal)
                if (high === low && open === close && high === open) {
                    const spread = close * 0.0001; // 0.01% spread
                    high = close + spread;
                    low = close - spread;
                }
                
                // Final OHLC relationship validation
                high = Math.max(high, open, close);
                low = Math.min(low, open, close);
                
                // Create properly formatted candle object
                const candle = {
                    time: alignedTimestamp,
                    timestamp: alignedTimestamp,
                    originalTime: timestamp,
                    open: parseFloat(open.toFixed(2)),
                    high: parseFloat(high.toFixed(2)),
                    low: parseFloat(low.toFixed(2)),
                    close: parseFloat(close.toFixed(2)),
                    price: parseFloat(close.toFixed(2)),
                    volume: parseInt(c.volume || 0) || 0
                };
                
                // Log validation for debugging
                if (index === 0 || index === data.candles.length - 1) {
                    const estTime = new Date(candle.time).toLocaleString('en-US', {
                        timeZone: 'America/New_York',
                        year: 'numeric',
                        month: 'short',
                        day: 'numeric',
                        hour: '2-digit',
                        minute: '2-digit',
                        hour12: false
                    });
                    console.log(`✓ Candle ${index === 0 ? 'first' : 'last'}: ${estTime} EST | O:$${candle.open} H:$${candle.high} L:$${candle.low} C:$${candle.close}`);
                }
                
                return candle;
            }).filter(c => c !== null && c.close > 0 && c.time > 0); // Filter out invalid candles
            
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
            
            /**
             * FIXED: Generate historical labels using actual candle timestamps
             * Shows dates whenever a new day appears on the X-axis
             */
            let lastDay = null; // Track the last day to detect day changes
            historicalLabels = historicalCandles.map((c, i) => {
                // Get timestamp from candle - use the actual aligned timestamp
                let timestamp = c.time || c.timestamp || c.originalTime;
                if (!timestamp) {
                    // Fallback to original data
                    timestamp = data.candles[i]?.time || data.candles[i]?.timestamp;
                    if (!timestamp) {
                        console.warn(`No timestamp for candle ${i}, using index`);
                        return `Point ${i + 1}`;
                    }
                }
                
                // Ensure timestamp is a number
                if (typeof timestamp !== 'number' || timestamp <= 0) {
                    console.warn(`Invalid timestamp for candle ${i}:`, timestamp);
                    return `Point ${i + 1}`;
                }
                
                // Create Date object from timestamp
                const date = new Date(timestamp);
                
                // Verify date is valid
                if (isNaN(date.getTime())) {
                    console.warn('Invalid date for candle', i, timestamp);
                    return `Point ${i + 1}`;
                }
                
                // Get date components in EST timezone
                const estDateString = date.toLocaleString('en-US', {
                    timeZone: 'America/New_York',
                    year: 'numeric',
                    month: 'short',
                    day: 'numeric',
                    hour: '2-digit',
                    minute: '2-digit',
                    hour12: false
                });
                
                // Parse to get day
                const [datePart, timePart] = estDateString.split(', ');
                const currentDay = datePart; // e.g., "Dec 19, 2025"
                
                // Check if this is a new day (or first candle)
                const isNewDay = lastDay === null || currentDay !== lastDay;
                lastDay = currentDay;
                
                // CRITICAL: Format date in EST timezone - show date when new day appears
                if (currentInterval === '15MIN' || currentInterval === '1H' || currentInterval === '4H') {
                    // For intraday timeframes, show date + time when new day, otherwise just time
                    if (isNewDay) {
                        // Show date and time for new day: "Dec 19\n14:00" or "Dec 19, 14:00"
                        return `${datePart}\n${timePart}`;
                    } else {
                        // Same day, just show time
                        return timePart;
                    }
                } else if (currentInterval === '1D') {
                    // For daily timeframe, show date and time if many candles (intraday data)
                    if (historicalCandles.length > 50) {
                        return datePart && timePart ? `${datePart} ${timePart} EST` : estDateString;
                    }
                    // For daily with few candles, show date only
                    return datePart;
                } else {
                    // For other intervals, show date only
                    return datePart;
                }
            });
            
            // Fetch real-time quote and update last price
            try {
                console.log('Fetching real-time quote to update projection starting point...');
                const quoteData = await fetchRealTimeQuote(symbol);
                
                if (quoteData && quoteData.current && quoteData.current > 0) {
                    const realTimePrice = parseFloat(quoteData.current);
                    const lastHistoricalPrice = historicalPrices[historicalPrices.length - 1];
                    
                    // Update the last price with real-time quote
                    if (historicalPrices.length > 0) {
                        historicalPrices[historicalPrices.length - 1] = realTimePrice;
                        console.log(`Updated last price from $${lastHistoricalPrice.toFixed(2)} to real-time price $${realTimePrice.toFixed(2)}`);
                    } else {
                        // If no historical prices, add the real-time price
                        historicalPrices.push(realTimePrice);
                        console.log(`Added real-time price $${realTimePrice.toFixed(2)} as starting point`);
                    }
                    
                    // FIXED: Update the last label to show the actual candle time, not forced 4:00 PM
                    if (historicalCandles.length > 0 && historicalLabels.length > 0) {
                        const lastCandle = historicalCandles[historicalCandles.length - 1];
                        const lastCandleTime = lastCandle.time || lastCandle.timestamp;
                        
                        if (lastCandleTime && typeof lastCandleTime === 'number' && lastCandleTime > 0) {
                            const date = new Date(lastCandleTime);
                            if (!isNaN(date.getTime())) {
                                const estDateString = date.toLocaleString('en-US', {
                                    timeZone: 'America/New_York',
                                    year: 'numeric',
                                    month: 'short',
                                    day: 'numeric',
                                    hour: '2-digit',
                                    minute: '2-digit',
                                    hour12: false
                                });
                                
                                // Format: "Dec 19, 2025, 14:00" -> "Dec 19, 2025 14:00 EST"
                                const [datePart, timePart] = estDateString.split(', ');
                                if (datePart && timePart) {
                                    historicalLabels[historicalLabels.length - 1] = `${datePart} ${timePart} EST`;
                                } else {
                                    historicalLabels[historicalLabels.length - 1] = `${estDateString} EST`;
                                }
                            }
                        }
                    }
                    
                    // Update the last candle with real-time data - CRITICAL: ensure exact match
                    if (historicalCandles.length > 0) {
                        const lastCandle = historicalCandles[historicalCandles.length - 1];
                        // Update close price to match real-time quote exactly
                        lastCandle.close = realTimePrice;
                        lastCandle.price = realTimePrice;
                        
                        // Update high/low if available from quote
                        // CRITICAL: Preserve proper OHLC relationships to avoid doji candles
                        if (quoteData.high && quoteData.high > realTimePrice) {
                            lastCandle.high = quoteData.high;
                        } else if (!lastCandle.high || lastCandle.high < realTimePrice) {
                            // Ensure high is at least the current price, but add small spread to avoid doji
                            const minHigh = Math.max(lastCandle.open || realTimePrice, realTimePrice);
                            lastCandle.high = minHigh + (realTimePrice * 0.0001); // 0.01% spread
                        }
                        
                        if (quoteData.low && quoteData.low < realTimePrice) {
                            lastCandle.low = quoteData.low;
                        } else if (!lastCandle.low || lastCandle.low > realTimePrice) {
                            // Ensure low is at most the current price, but subtract small spread to avoid doji
                            const maxLow = Math.min(lastCandle.open || realTimePrice, realTimePrice);
                            lastCandle.low = maxLow - (realTimePrice * 0.0001); // 0.01% spread
                        }
                        
                        // Update open if not set - use previous candle's close if available
                        if (!lastCandle.open || lastCandle.open <= 0) {
                            // Try to use previous candle's close
                            if (historicalCandles.length > 1) {
                                const prevCandle = historicalCandles[historicalCandles.length - 2];
                                if (prevCandle && prevCandle.close) {
                                    lastCandle.open = parseFloat(prevCandle.close);
                                } else {
                                    lastCandle.open = realTimePrice;
                                }
                            } else {
                                lastCandle.open = realTimePrice;
                            }
                        }
                        
                        // Final validation: ensure high >= max(open, close) and low <= min(open, close)
                        const finalOpen = parseFloat(lastCandle.open);
                        const finalClose = realTimePrice;
                        lastCandle.high = Math.max(parseFloat(lastCandle.high), Math.max(finalOpen, finalClose));
                        lastCandle.low = Math.min(parseFloat(lastCandle.low), Math.min(finalOpen, finalClose));
                        
                        // FIXED: Use the actual candle timestamp, not forced 4:00 PM
                        // The last candle should show its actual time, not market close
                        // Only align to 1H boundary if needed, but preserve the actual hour
                        if (!lastCandle.time || lastCandle.time <= 0) {
                            // If no timestamp, use current hour aligned to 1H boundary
                            const now = new Date();
                            const alignedTime = alignToTimeframeBoundary(now.getTime(), currentInterval);
                            lastCandle.time = alignedTime;
                        } else {
                            // Ensure timestamp is aligned to 1H boundary
                            const alignedTime = alignToTimeframeBoundary(lastCandle.time, currentInterval);
                            lastCandle.time = alignedTime;
                        }
                        
                        // Log the actual timestamp being used
                        const displayTime = new Date(lastCandle.time).toLocaleString('en-US', {
                            timeZone: 'America/New_York',
                            year: 'numeric',
                            month: 'short',
                            day: 'numeric',
                            hour: '2-digit',
                            minute: '2-digit',
                            hour12: false
                        });
                        console.log(`✓ Last candle timestamp: ${displayTime} EST (actual candle time, not forced 4:00 PM)`);
                        
                        // CRITICAL: Ensure historicalPrices array matches the last candle's close
                        // This ensures projections connect correctly
                        if (historicalPrices.length > 0) {
                            historicalPrices[historicalPrices.length - 1] = realTimePrice;
                        }
                        
                        // Log for debugging
                        console.log(`✓ Updated last candle: close=${realTimePrice.toFixed(2)}, high=${lastCandle.high.toFixed(2)}, low=${lastCandle.low.toFixed(2)}, historicalPrices[last]=${historicalPrices[historicalPrices.length - 1].toFixed(2)}`);
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
            
            // Use PHP Fallback FIRST (as requested)
            // Skip UnifiedProjectionEngine and use PHP API directly
            let projectionLines = [];
            let validationResults = null;
            
            try {
                // Use PHP fallback method first (primary method)
                console.log('📊 Using PHP Fallback method for projections...');
                projectionLines = await calculateProjections(historicalPrices, params);
                
                // Validate with ProjectionValidator if available
                if (typeof ProjectionValidator !== 'undefined') {
                    try {
                        validationResults = ProjectionValidator.validate(historicalPrices, projectionLines, params);
                    } catch (e) {
                        console.warn('Validation error:', e);
                        validationResults = null;
                    }
                }
            } catch (error) {
                console.error('Error in PHP fallback projection:', error);
                showError('Failed to generate projections. Please try again.');
                return;
            }
            
            // Ensure we have at least some projection lines
            if (!projectionLines || projectionLines.length === 0) {
                console.error('No projection lines generated');
                showError('Failed to generate projections. Please check your parameters.');
                return;
            }
            
            // Validate projection lines have valid data
            const validProjectionLines = projectionLines.filter(line => {
                if (!line || !line.points) {
                    console.warn('Invalid projection line (missing points):', line);
                    return false;
                }
                if (!Array.isArray(line.points) || line.points.length === 0) {
                    console.warn('Invalid projection line (empty points):', line);
                    return false;
                }
                const validPoints = line.points.filter(p => p !== null && !isNaN(p) && p > 0);
                if (validPoints.length === 0) {
                    console.warn('Invalid projection line (no valid points):', line);
                    return false;
                }
                return true;
            });
            
            if (validProjectionLines.length === 0) {
                console.error('No valid projection lines after validation');
                showError('All projection lines are invalid. Please check your parameters.');
                return;
            }
            
            // Use validated lines
            projectionLines = validProjectionLines;
            currentProjectionLines = projectionLines; // Store for CSV export
            console.log(`✓ Using ${projectionLines.length} valid projection lines for chart`);
            
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
            
            // Store params for export functionality
            currentProjectionParams = params;
            
            renderChart(historicalPrices, historicalLabels, projectionLines, params);
            updateMetrics(historicalPrices, projectionLines, params);
            updateComputationMethodDisplay();
            
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
            // Show export button when chart is loaded
            const exportBtn = document.getElementById('export-chart-btn');
            if (exportBtn) {
                exportBtn.style.display = 'inline-block';
                exportBtn.disabled = false;
                // Re-setup export functionality to ensure handler is attached
                setTimeout(() => {
                    setupExportFunctionality();
                }, 100);
            }
            // Reset zoom button is always visible, no need to show it
            // Metrics sections are now always visible, no need to show them
            
            // DO NOT setup auto-refresh - only refresh when user clicks Load button
            // setupAutoRefresh(); // DISABLED - user must manually click Load button
            stopAutoRefresh(); // Ensure auto-refresh is stopped
            
        } catch (error) {
            console.error('Error loading projection data:', error);
            const errorMessage = error.message || 'Failed to load projection data. Please check the symbol and try again.';
            showError(errorMessage);
            
            // Reset metrics to placeholders on error (keep sections visible)
            // Use resetValidationMetricsToPlaceholders if available, otherwise just hide error
            if (typeof resetValidationMetricsToPlaceholders === 'function') {
                resetValidationMetricsToPlaceholders();
            }
            document.getElementById('projection-refresh-btn').style.display = 'none';
            // Reset zoom button stays visible even on error
            
            // Stop auto-refresh on error
            stopAutoRefresh();
        } finally {
            isRefreshing = false;
            showLoading(false);
        }
    }
    
    /**
     * Setup auto-refresh for real-time data updates
     * DISABLED - Only refresh when user clicks Load button
     */
    function setupAutoRefresh() {
        // Auto-refresh is disabled - user must manually click Load button
        // This prevents automatic data fetching and projection switching
        stopAutoRefresh();
        console.log('ℹ️ Auto-refresh disabled - use Load button to refresh data');
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
                    const validPoints = line.points
                        .map(p => {
                            if (p === null || p === undefined) return null;
                            const num = typeof p === 'number' ? p : parseFloat(p);
                            return !isNaN(num) && isFinite(num) && num > 0 ? num : null;
                        })
                        .filter(p => p !== null);
                    if (validPoints.length > 0) {
                        const lineMin = Math.min(...validPoints);
                        const lineMax = Math.max(...validPoints);
                        if (!isNaN(lineMin) && !isNaN(lineMax) && isFinite(lineMin) && isFinite(lineMax)) {
                            minPrice = Math.min(minPrice, lineMin);
                            maxPrice = Math.max(maxPrice, lineMax);
                        }
                    }
                }
            });
        }
        
        // Include actual prices if available
        if (actualPrices && actualPrices.length > 0) {
            const validActualPrices = actualPrices
                .map(p => {
                    if (p === null || p === undefined) return null;
                    const num = typeof p === 'number' ? p : parseFloat(p);
                    return !isNaN(num) && isFinite(num) && num > 0 ? num : null;
                })
                .filter(p => p !== null);
            if (validActualPrices.length > 0) {
                const actualMin = Math.min(...validActualPrices);
                const actualMax = Math.max(...validActualPrices);
                if (!isNaN(actualMin) && !isNaN(actualMax) && isFinite(actualMin) && isFinite(actualMax)) {
                    minPrice = Math.min(minPrice, actualMin);
                    maxPrice = Math.max(maxPrice, actualMax);
                }
            }
        }
        
        // CRITICAL: Ensure real-time price (lastPrice) is included in Y-axis range
        // Add padding to Y-axis range (5% on each side)
        const priceRange = maxPrice - minPrice;
        const padding = priceRange > 0 ? priceRange * 0.05 : Math.max(minPrice * 0.05, 1);
        let yAxisMin = Math.max(0, minPrice - padding);
        let yAxisMax = maxPrice + padding;
        
        // Ensure lastPrice (real-time price) is within Y-axis range
        if (lastPrice < yAxisMin) {
            yAxisMin = Math.max(0, lastPrice - padding);
        }
        if (lastPrice > yAxisMax) {
            yAxisMax = lastPrice + padding;
        }
        
        // Log Y-axis range for debugging
        console.log(`✓ Y-axis range: $${yAxisMin.toFixed(2)} - $${yAxisMax.toFixed(2)} (current price: $${lastPrice.toFixed(2)})`);
        
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
            
            /**
             * REDESIGNED: Build candlestick data with 100% accurate time and price
             * Uses pre-validated historicalCandles with correct timestamps and OHLC
             */
            for (let i = 0; i < historicalCandles.length; i++) {
                const c = historicalCandles[i];
                
                // Use pre-validated data from historicalCandles
                const open = parseFloat(c.open);
                const high = parseFloat(c.high);
                const low = parseFloat(c.low);
                const close = parseFloat(c.close);
                const time = c.time || c.timestamp;
                
                // Final validation before adding to chart
                if (isNaN(open) || isNaN(high) || isNaN(low) || isNaN(close) ||
                    open <= 0 || high <= 0 || low <= 0 || close <= 0 ||
                    !time || typeof time !== 'number' || time <= 0) {
                    console.warn(`⚠️ Invalid candlestick data at index ${i}, skipping:`, c);
                    continue;
                }
                
                // Validate OHLC relationships one more time
                if (high < Math.max(open, close) || low > Math.min(open, close) || high < low) {
                    console.warn(`⚠️ OHLC relationship error at index ${i}, correcting:`, {open, high, low, close});
                    // Correct relationships
                    const correctedHigh = Math.max(high, open, close);
                    const correctedLow = Math.min(low, open, close);
                    
                    candlestickData.push({
                        x: time,
                        o: parseFloat(open.toFixed(2)),
                        h: parseFloat(correctedHigh.toFixed(2)),
                        l: parseFloat(correctedLow.toFixed(2)),
                        c: parseFloat(close.toFixed(2))
                    });
                } else {
                    // Data is valid, add as-is
                    candlestickData.push({
                        x: time,
                        o: parseFloat(open.toFixed(2)),
                        h: parseFloat(high.toFixed(2)),
                        l: parseFloat(low.toFixed(2)),
                        c: parseFloat(close.toFixed(2))
                    });
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
            
            // Validate all historical data points are numbers
            const validatedHistoricalData = historicalData.map(p => {
                if (p === null || p === undefined) return null;
                const num = typeof p === 'number' ? p : parseFloat(p);
                return !isNaN(num) && isFinite(num) && num > 0 ? num : null;
            });
            
            // Add nulls for projection period to match labels array
            const projectionNulls = new Array(steps).fill(null);
            historicalData = [...validatedHistoricalData, ...projectionNulls];
            
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
            
            // Validate and add projection points - ensure all are proper numbers
            const validPoints = line.points
                .map(p => {
                    if (p === null || p === undefined) return null;
                    const num = typeof p === 'number' ? p : parseFloat(p);
                    return !isNaN(num) && isFinite(num) && num > 0 ? num : null;
                })
                .filter(p => p !== null);
            
            if (validPoints.length === 0) {
                console.warn(`No valid points in projection line at index ${idx}`, line);
                return;
            }
            
            // CRITICAL: First projection point MUST start from current price
            // The projection algorithm calculates points relative to the starting price
            // We ensure the first point connects from the current price
            const firstProjected = validPoints[0];
            
            // Validate that first projected point is close to current price (within 5% tolerance)
            // This ensures projections are starting from current price
            const priceDiff = Math.abs(firstProjected - lastPrice);
            const priceTolerance = lastPrice * 0.05; // 5% tolerance
            
            if (priceDiff > priceTolerance) {
                console.warn(`First projection point ($${firstProjected.toFixed(2)}) differs from current price ($${lastPrice.toFixed(2)}) by ${((priceDiff / lastPrice) * 100).toFixed(2)}%. Adjusting to start from current price.`);
            }
            
            // Always connect from current price - this is the starting point
            if (lastPrice > 0 && !isNaN(lastPrice) && isFinite(lastPrice)) {
                // Connect directly to last historical price (current price)
                // This ensures projections start from the current price only
                lineData.push(lastPrice);
                console.log(`✓ Projection line ${idx} starts from current price: $${lastPrice.toFixed(2)}`);
            } else if (firstProjected > 0 && !isNaN(firstProjected) && isFinite(firstProjected)) {
                // Fallback: use first projected point if lastPrice is invalid
                console.warn('Using first projected point as connection point (lastPrice invalid)');
                lineData.push(firstProjected);
            } else {
                // Last resort: skip this projection line
                console.error('Cannot connect projection line: invalid lastPrice and firstProjected');
                return;
            }
            
            // Add projection points - these are calculated from the current price
            // The first point in validPoints is the first future projection (after current price)
            // We already added the connection point (current price), so start from index 0
            // This ensures projections start from current price and extend forward
            for (let i = 0; i < validPoints.length; i++) {
                const point = validPoints[i];
                if (point !== null && !isNaN(point) && isFinite(point) && point > 0) {
                    lineData.push(point);
                } else {
                    // Fill with null if point is invalid (Chart.js will skip it)
                    lineData.push(null);
                }
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
            // Fill with nulls for historical period
            for (let i = 0; i < historicalPrices.length; i++) {
                actualData.push(null);
            }
            
            // Add actual prices starting from where projection begins
            // Match actual labels to projection labels
            let actualIdx = 0;
            for (let i = historicalPrices.length; i < allLabels.length && actualIdx < actualPrices.length; i++) {
                actualData.push(actualPrices[actualIdx]);
                actualIdx++;
            }
            
            // Fill remaining with nulls
            while (actualData.length < allLabels.length) {
                actualData.push(null);
            }
            
            datasets.push({
                label: 'Actual Price (Post-Projection)',
                data: actualData,
                borderColor: '#22c55e', // Green for actual
                backgroundColor: 'transparent',
                borderWidth: 3,
                pointRadius: 2,
                pointBackgroundColor: '#22c55e',
                tension: 0.1,
                borderDash: [],
                yAxisID: 'y'
            });
            
            // Calculate and display accuracy metrics
            calculateAccuracyMetrics(projectionLines, actualPrices, historicalPrices.length);
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
                                // CRITICAL: Format timestamp in EST timezone to show correct current time
                                // Matching Trading Charts tab format: "Dec 19, 2025 at 3:14:27 PM EST"
                                if (context.length > 0) {
                                    const ctx = context[0];
                                    const index = ctx.dataIndex;
                                    
                                    // Get timestamp from candlestick data (most accurate)
                                    let timestamp = null;
                                    
                                    if (ctx.dataset.type === 'candlestick' && ctx.raw && typeof ctx.raw === 'object' && ctx.raw.x !== undefined) {
                                        timestamp = ctx.raw.x;
                                    } else if (index < historicalCandles.length && historicalCandles[index]) {
                                        // Fallback: use historicalCandles data
                                        const candle = historicalCandles[index];
                                        timestamp = candle.time || candle.timestamp;
                                    }
                                    
                                    // Format timestamp in EST timezone
                                    if (timestamp && typeof timestamp === 'number' && timestamp > 0) {
                                        const date = new Date(timestamp);
                                        
                                        // Verify date is valid
                                        if (!isNaN(date.getTime())) {
                                            // Format in EST timezone - matching Trading Charts format
                                            const estString = date.toLocaleString('en-US', {
                                                timeZone: 'America/New_York',
                                                month: 'short',
                                                day: 'numeric',
                                                year: 'numeric',
                                                hour: '2-digit',
                                                minute: '2-digit',
                                                second: '2-digit',
                                                hour12: true
                                            });
                                            
                                            // Format: "Dec 19, 2025 at 3:14:27 PM EST"
                                            return estString + ' EST';
                                        }
                                    }
                                    
                                    // Fallback to label from allLabels
                                    const label = allLabels[index];
                                    return label || `Point ${index + 1}`;
                                }
                                return '';
                            },
                            label: function(context) {
                                // Handle candlestick data - get OHLC from historicalCandles for accuracy
                                if (context.dataset.type === 'candlestick' && context.raw) {
                                    const raw = context.raw;
                                    if (typeof raw === 'object' && raw.o !== undefined) {
                                        // CRITICAL: Use OHLC from historicalCandles if available for accurate data
                                        const index = context.dataIndex;
                                        if (index < historicalCandles.length && historicalCandles[index]) {
                                            const candle = historicalCandles[index];
                                            
                                            // Format volume using formatVolume function (now defined at top of module)
                                            const volumeStr = formatVolume(candle.volume || 0);
                                            
                                            return [
                                                `Open (O): ${formatPrice(candle.open)}`,
                                                `High (H): ${formatPrice(candle.high)}`,
                                                `Low (L): ${formatPrice(candle.low)}`,
                                                `Close (C): ${formatPrice(candle.close)}`,
                                                `Volume (V): ${volumeStr}`
                                            ];
                                        }
                                        
                                        // Fallback to raw data
                                        return [
                                            `Open (O): ${formatPrice(raw.o)}`,
                                            `High (H): ${formatPrice(raw.h)}`,
                                            `Low (L): ${formatPrice(raw.l)}`,
                                            `Close (C): ${formatPrice(raw.c)}`
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
                        bottom: 20, // Reduced since x-axis is hidden
                        right: 20,
                        top: 20,
                        left: 20
                    }
                },
                scales: {
                    x: {
                        display: false, // X-axis hidden
                        title: {
                            display: false
                        },
                        grid: {
                            display: false
                        },
                        ticks: {
                            display: false
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
        
        // Reset zoom button is always visible
        if (zoomInBtn) zoomInBtn.style.display = 'inline-block';
        if (zoomOutBtn) zoomOutBtn.style.display = 'inline-block';
        
        // Setup zoom button handlers
        setupZoomControls();
        
        // Setup export functionality after chart is rendered
        setTimeout(() => {
            setupExportFunctionality();
        }, 200);
        
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
     * Setup export functionality - REDESIGNED for 100% reliability
     */
    function setupExportFunctionality() {
        console.log('🔧 Setting up export functionality...');
        
        // Find all export buttons (there might be multiple)
        const exportButtons = document.querySelectorAll('#export-chart-btn');
        
        if (exportButtons.length === 0) {
            console.log('⏳ Export button not found, will retry...');
            // Retry after a short delay
            setTimeout(setupExportFunctionality, 500);
            return;
        }
        
        console.log(`✓ Found ${exportButtons.length} export button(s)`);
        
        // Setup each export button
        exportButtons.forEach((btn, index) => {
            // Remove all existing event listeners by cloning
            const newBtn = btn.cloneNode(true);
            btn.parentNode.replaceChild(newBtn, btn);
            
            // Get the fresh button reference
            const freshBtn = document.querySelectorAll('#export-chart-btn')[index];
            
            if (freshBtn) {
                // Remove any existing onclick
                freshBtn.onclick = null;
                
                // Add multiple event listeners for maximum compatibility
                freshBtn.addEventListener('click', handleExportClick, false);
                freshBtn.onclick = handleExportClick;
                
                // Also handle mousedown for better responsiveness
                freshBtn.addEventListener('mousedown', function(e) {
                    e.preventDefault();
                }, false);
                
                console.log(`✅ Export button ${index + 1} handler attached`);
            }
        });
    }
    
    /**
     * Handle export button click - REDESIGNED
     */
    function handleExportClick(e) {
        if (e) {
            e.preventDefault();
            e.stopPropagation();
            e.stopImmediatePropagation();
        }
        
        console.log('📥 Export button clicked');
        console.log('Chart exists:', !!projectionChart);
        console.log('Current symbol:', currentSymbol);
        console.log('exportChart function type:', typeof exportChart);
        
        try {
            // Direct call to showExportModal for maximum reliability
            if (projectionChart && currentSymbol) {
                showExportModal();
            } else if (!projectionChart) {
                showError('No chart to export. Please load a projection first.');
            } else if (!currentSymbol) {
                showError('No symbol data available. Please load a projection first.');
            }
        } catch (error) {
            console.error('❌ Error in handleExportClick:', error);
            console.error('Error stack:', error.stack);
            showError('Error opening export dialog: ' + (error.message || 'Unknown error'));
        }
        
        return false;
    }
    
    /**
     * Collect metrics and configuration data for export
     */
    function collectExportData() {
        const data = {
            lastUpdated: null,
            priceMetrics: {},
            validationMetrics: {},
            projectionConfig: {}
        };
        
        // Get Last Updated
        const lastUpdateEl = document.getElementById('last-update');
        if (lastUpdateEl) {
            data.lastUpdated = lastUpdateEl.textContent || '--';
        }
        
        // Get Price Metrics
        const periodHighEl = document.getElementById('metric-period-high');
        const periodHighLabel = periodHighEl?.parentElement?.querySelector('.metric-label')?.textContent || 'Period High';
        const priceMetrics = {
            currentPrice: document.getElementById('metric-current-price')?.textContent || '--',
            historicalChange: document.getElementById('metric-historical-change')?.textContent || '--',
            historicalPercent: document.getElementById('metric-historical-percent')?.textContent || '--',
            projectedPrice: document.getElementById('metric-projected-price')?.textContent || '--',
            projectedChange: document.getElementById('metric-projected-change')?.textContent || '--',
            projectedPercent: document.getElementById('metric-projected-percent')?.textContent || '--',
            periodHigh: periodHighEl?.textContent || '--',
            periodLow: document.getElementById('metric-period-low')?.textContent || '--',
            averageVolume: document.getElementById('metric-average-volume')?.textContent || '--',
            actualVolume: document.getElementById('metric-actual-volume')?.textContent || '--'
        };
        data.priceMetrics = priceMetrics;
        
        // Get Validation Metrics
        const validationMetrics = {
            mae: document.getElementById('validation-mae')?.textContent || '--',
            rmse: document.getElementById('validation-rmse')?.textContent || '--',
            mape: document.getElementById('validation-mape')?.textContent || '--',
            confidence: document.getElementById('validation-confidence')?.textContent || '--'
        };
        data.validationMetrics = validationMetrics;
        
        // Get Projection Configuration
        const selectedPreset = document.querySelector('input[name="projection-preset"]:checked');
        const presetName = selectedPreset 
            ? (selectedPreset.closest('.param-toggle-item')?.querySelector('.param-toggle-name')?.textContent || selectedPreset.value)
            : 'Custom';
        
        data.projectionConfig = {
            preset: presetName,
            steps: currentProjectionParams?.steps || document.getElementById('projection-steps-value')?.value || document.getElementById('projection-steps')?.value || '--',
            base: currentProjectionParams?.base || document.getElementById('projection-base-value')?.value || document.getElementById('projection-base')?.value || '--',
            projectionCount: currentProjectionParams?.projectionCount || document.getElementById('projection-count-value')?.value || document.getElementById('projection-count')?.value || '--',
            depthPrime: currentProjectionParams?.depthPrime || document.getElementById('projection-depth-value')?.value || document.getElementById('projection-depth')?.value || '--',
            symbol: currentSymbol || '--',
            interval: currentInterval || '--'
        };
        
        return data;
    }
    
    /**
     * Create composite image with chart and additional information
     */
    function createCompositeExportImage(chartImageData, exportData) {
        return new Promise((resolve, reject) => {
            try {
                // Create a new canvas for the composite image
                const compositeCanvas = document.createElement('canvas');
                const ctx = compositeCanvas.getContext('2d');
                
                // Load chart image
                const chartImg = new Image();
                chartImg.onload = () => {
                    try {
                        // Design constants matching web app
                        const chartWidth = chartImg.width;
                        const chartHeight = chartImg.height;
                        const padding = 40;
                        const sectionSpacing = 30;
                        const cardPadding = 10;
                        const cardGap = 10;
                        const cardBorderRadius = 6;
                        
                        // Colors matching web app
                        const colors = {
                            background: '#1a1a1a',
                            cardBg: '#2a2a2a',
                            border: '#444444',
                            textPrimary: '#e0e0e0',
                            textSecondary: '#9ca3af',
                            sectionHeader: '#3b82f6',
                            success: '#22c55e',
                            danger: '#ef4444'
                        };
                        
                        // Font sizes
                        const fonts = {
                            title: 'bold 20px Inter, sans-serif',
                            sectionHeader: 'bold 15px Inter, sans-serif',
                            metricLabel: '500 9px Inter, sans-serif',
                            metricValue: '700 18px Inter, sans-serif',
                            metricDesc: '400 8px Inter, sans-serif',
                            configLabel: '500 12px Inter, sans-serif',
                            configValue: '400 12px Inter, sans-serif',
                            lastUpdated: '400 16px Inter, sans-serif'
                        };
                        
                        // Calculate section dimensions
                        ctx.font = fonts.title;
                        const titleWidth = ctx.measureText(`${exportData.projectionConfig.symbol} Price Projection Export`).width;
                        
                        ctx.font = fonts.lastUpdated;
                        const lastUpdatedWidth = ctx.measureText(`Last Updated: ${exportData.lastUpdated}`).width;
                        
                        // Price Metrics - Grid layout (4 columns)
                        const priceMetricsEntries = Object.entries(exportData.priceMetrics);
                        const priceMetricsCols = 4;
                        const priceMetricsRows = Math.ceil(priceMetricsEntries.length / priceMetricsCols);
                        
                        ctx.font = fonts.metricValue;
                        const maxMetricValueWidth = Math.max(...priceMetricsEntries.map(([k, v]) => {
                            const val = String(v || '--');
                            return ctx.measureText(val).width;
                        }));
                        
                        ctx.font = fonts.metricLabel;
                        const maxMetricLabelWidth = Math.max(...priceMetricsEntries.map(([k, v]) => {
                            const label = k.replace(/([A-Z])/g, ' $1').replace(/^./, str => str.toUpperCase());
                            return ctx.measureText(label.toUpperCase()).width;
                        }));
                        
                        const cardWidth = Math.max(140, maxMetricValueWidth + maxMetricLabelWidth + (cardPadding * 2));
                        const cardHeight = 60; // Label + Value + spacing
                        const priceMetricsSectionWidth = (cardWidth * priceMetricsCols) + (cardGap * (priceMetricsCols - 1));
                        const priceMetricsSectionHeight = 50 + (cardHeight * priceMetricsRows) + (cardGap * (priceMetricsRows - 1));
                        
                        // Validation Metrics - Grid layout (4 columns)
                        const validationMetricsEntries = Object.entries(exportData.validationMetrics);
                        const validationCols = 4;
                        const validationRows = Math.ceil(validationMetricsEntries.length / validationCols);
                        
                        ctx.font = fonts.metricValue;
                        const maxValidationValueWidth = Math.max(...validationMetricsEntries.map(([k, v]) => {
                            const val = String(v || '--');
                            return ctx.measureText(val).width;
                        }));
                        
                        ctx.font = fonts.metricLabel;
                        const maxValidationLabelWidth = Math.max(...validationMetricsEntries.map(([k, v]) => {
                            return ctx.measureText(k.toUpperCase()).width;
                        }));
                        
                        ctx.font = fonts.metricDesc;
                        const validationDescriptions = {
                            mae: 'Mean Absolute Error',
                            rmse: 'Root Mean Squared Error',
                            mape: 'Mean Absolute Percentage Error',
                            confidence: 'Model Confidence Score'
                        };
                        const maxValidationDescWidth = Math.max(...validationMetricsEntries.map(([k, v]) => {
                            return ctx.measureText(validationDescriptions[k] || '').width;
                        }));
                        
                        const validationCardWidth = Math.max(140, Math.max(maxValidationValueWidth, maxValidationLabelWidth, maxValidationDescWidth) + (cardPadding * 2));
                        const validationCardHeight = 70; // Label + Value + Description + spacing
                        const validationSectionWidth = (validationCardWidth * validationCols) + (cardGap * (validationCols - 1));
                        const validationSectionHeight = 50 + (validationCardHeight * validationRows) + (cardGap * (validationRows - 1));
                        
                        // Projection Configuration - Simple list
                        const configEntries = Object.entries(exportData.projectionConfig);
                        ctx.font = fonts.configLabel;
                        const maxConfigLabelWidth = Math.max(...configEntries.map(([k, v]) => {
                            const label = k.replace(/([A-Z])/g, ' $1').replace(/^./, str => str.toUpperCase());
                            return ctx.measureText(label + ':').width;
                        }));
                        
                        ctx.font = fonts.configValue;
                        const maxConfigValueWidth = Math.max(...configEntries.map(([k, v]) => {
                            return ctx.measureText(String(v || '--')).width;
                        }));
                        
                        const configSectionWidth = maxConfigLabelWidth + maxConfigValueWidth + 40;
                        const configSectionHeight = 50 + (configEntries.length * 24);
                        
                        // Calculate total dimensions
                        const headerHeight = 60;
                        const lastUpdatedHeight = 30;
                        const maxSectionWidth = Math.max(priceMetricsSectionWidth, validationSectionWidth, configSectionWidth, chartWidth, 800);
                        const totalInfoHeight = headerHeight + lastUpdatedHeight + sectionSpacing + 
                                             priceMetricsSectionHeight + sectionSpacing + 
                                             validationSectionHeight + sectionSpacing + 
                                             configSectionHeight;
                        
                        const totalWidth = Math.max(chartWidth, maxSectionWidth + (padding * 2));
                        const totalHeight = chartHeight + totalInfoHeight + (padding * 3);
                        
                        // Set canvas dimensions
                        compositeCanvas.width = totalWidth;
                        compositeCanvas.height = totalHeight;
                        
                        // Draw background
                        ctx.fillStyle = colors.background;
                        ctx.fillRect(0, 0, totalWidth, totalHeight);
                        
                        // Draw chart
                        const chartX = (totalWidth - chartWidth) / 2;
                        ctx.drawImage(chartImg, chartX, padding, chartWidth, chartHeight);
                        
                        let currentY = padding + chartHeight + padding;
                        
                        // Draw title
                        ctx.fillStyle = colors.textPrimary;
                        ctx.font = fonts.title;
                        ctx.textAlign = 'center';
                        ctx.fillText(`${exportData.projectionConfig.symbol} Price Projection Export`, totalWidth / 2, currentY);
                        currentY += headerHeight;
                        
                        // Draw Last Updated
                        ctx.fillStyle = colors.textSecondary;
                        ctx.font = fonts.lastUpdated;
                        ctx.textAlign = 'center';
                        ctx.fillText(`Last Updated: ${exportData.lastUpdated}`, totalWidth / 2, currentY);
                        currentY += lastUpdatedHeight + sectionSpacing;
                        
                        // Helper function to draw rounded rectangle
                        function drawRoundedRect(x, y, width, height, radius) {
                            ctx.beginPath();
                            ctx.moveTo(x + radius, y);
                            ctx.lineTo(x + width - radius, y);
                            ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
                            ctx.lineTo(x + width, y + height - radius);
                            ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
                            ctx.lineTo(x + radius, y + height);
                            ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
                            ctx.lineTo(x, y + radius);
                            ctx.quadraticCurveTo(x, y, x + radius, y);
                            ctx.closePath();
                        }
                        
                        // Draw Price Metrics Section
                        const priceMetricsX = (totalWidth - priceMetricsSectionWidth) / 2;
                        const priceMetricsY = currentY;
                        
                        // Section box
                        ctx.fillStyle = colors.cardBg;
                        drawRoundedRect(priceMetricsX - padding, priceMetricsY - 5, priceMetricsSectionWidth + (padding * 2), priceMetricsSectionHeight, cardBorderRadius);
                        ctx.fill();
                        
                        ctx.strokeStyle = colors.border;
                        ctx.lineWidth = 1;
                        drawRoundedRect(priceMetricsX - padding, priceMetricsY - 5, priceMetricsSectionWidth + (padding * 2), priceMetricsSectionHeight, cardBorderRadius);
                        ctx.stroke();
                        
                        // Section header
                        ctx.fillStyle = colors.sectionHeader;
                        ctx.font = fonts.sectionHeader;
                        ctx.textAlign = 'left';
                        ctx.fillText('Price Metrics', priceMetricsX, priceMetricsY + 20);
                        
                        // Draw border under header
                        ctx.strokeStyle = colors.border;
                        ctx.beginPath();
                        ctx.moveTo(priceMetricsX, priceMetricsY + 30);
                        ctx.lineTo(priceMetricsX + priceMetricsSectionWidth, priceMetricsY + 30);
                        ctx.stroke();
                        
                        // Draw metric cards in grid
                        let cardY = priceMetricsY + 50;
                        priceMetricsEntries.forEach(([key, value], index) => {
                            const col = index % priceMetricsCols;
                            const row = Math.floor(index / priceMetricsCols);
                            const cardX = priceMetricsX + (col * (cardWidth + cardGap));
                            const cardYPos = cardY + (row * (cardHeight + cardGap));
                            
                            // Draw card background
                            ctx.fillStyle = colors.background;
                            drawRoundedRect(cardX, cardYPos, cardWidth, cardHeight, cardBorderRadius);
                            ctx.fill();
                            
                            ctx.strokeStyle = colors.border;
                            ctx.lineWidth = 1;
                            drawRoundedRect(cardX, cardYPos, cardWidth, cardHeight, cardBorderRadius);
                            ctx.stroke();
                            
                            // Draw label
                            const label = key.replace(/([A-Z])/g, ' $1').replace(/^./, str => str.toUpperCase());
                            ctx.fillStyle = colors.textSecondary;
                            ctx.font = fonts.metricLabel;
                            ctx.textAlign = 'left';
                            ctx.fillText(label.toUpperCase(), cardX + cardPadding, cardYPos + 12);
                            
                            // Draw value (with color coding for changes)
                            const valStr = String(value || '--');
                            let valueColor = colors.textPrimary;
                            if (key.includes('Change') || key.includes('Percent')) {
                                const numVal = parseFloat(valStr.replace(/[^0-9.-]/g, ''));
                                if (!isNaN(numVal)) {
                                    valueColor = numVal >= 0 ? colors.success : colors.danger;
                                }
                            }
                            
                            ctx.fillStyle = valueColor;
                            ctx.font = fonts.metricValue;
                            ctx.fillText(valStr, cardX + cardPadding, cardYPos + 35);
                        });
                        
                        currentY += priceMetricsSectionHeight + sectionSpacing;
                        
                        // Draw Validation Metrics Section
                        const validationX = (totalWidth - validationSectionWidth) / 2;
                        const validationY = currentY;
                        
                        // Section box
                        ctx.fillStyle = colors.cardBg;
                        drawRoundedRect(validationX - padding, validationY - 5, validationSectionWidth + (padding * 2), validationSectionHeight, cardBorderRadius);
                        ctx.fill();
                        
                        ctx.strokeStyle = colors.border;
                        ctx.lineWidth = 1;
                        drawRoundedRect(validationX - padding, validationY - 5, validationSectionWidth + (padding * 2), validationSectionHeight, cardBorderRadius);
                        ctx.stroke();
                        
                        // Section header
                        ctx.fillStyle = colors.sectionHeader;
                        ctx.font = fonts.sectionHeader;
                        ctx.textAlign = 'left';
                        ctx.fillText('Validation Metrics', validationX, validationY + 20);
                        
                        // Draw border under header
                        ctx.strokeStyle = colors.border;
                        ctx.beginPath();
                        ctx.moveTo(validationX, validationY + 30);
                        ctx.lineTo(validationX + validationSectionWidth, validationY + 30);
                        ctx.stroke();
                        
                        // Draw validation metric cards in grid
                        cardY = validationY + 50;
                        validationMetricsEntries.forEach(([key, value], index) => {
                            const col = index % validationCols;
                            const row = Math.floor(index / validationCols);
                            const cardX = validationX + (col * (validationCardWidth + cardGap));
                            const cardYPos = cardY + (row * (validationCardHeight + cardGap));
                            
                            // Draw card background
                            ctx.fillStyle = colors.background;
                            drawRoundedRect(cardX, cardYPos, validationCardWidth, validationCardHeight, cardBorderRadius);
                            ctx.fill();
                            
                            ctx.strokeStyle = colors.border;
                            ctx.lineWidth = 1;
                            drawRoundedRect(cardX, cardYPos, validationCardWidth, validationCardHeight, cardBorderRadius);
                            ctx.stroke();
                            
                            // Draw label
                            ctx.fillStyle = colors.textSecondary;
                            ctx.font = fonts.metricLabel;
                            ctx.textAlign = 'left';
                            ctx.fillText(key.toUpperCase(), cardX + cardPadding, cardYPos + 12);
                            
                            // Draw value
                            const valStr = String(value || '--');
                            ctx.fillStyle = colors.textPrimary;
                            ctx.font = fonts.metricValue;
                            ctx.fillText(valStr, cardX + cardPadding, cardYPos + 35);
                            
                            // Draw description
                            const desc = validationDescriptions[key] || '';
                            if (desc) {
                                ctx.fillStyle = colors.textSecondary;
                                ctx.font = fonts.metricDesc;
                                ctx.fillText(desc, cardX + cardPadding, cardYPos + 55);
                            }
                        });
                        
                        currentY += validationSectionHeight + sectionSpacing;
                        
                        // Draw Projection Configuration Section
                        const configX = (totalWidth - configSectionWidth) / 2;
                        const configY = currentY;
                        const configBoxHeight = configSectionHeight;
                        
                        // Section box
                        ctx.fillStyle = colors.cardBg;
                        drawRoundedRect(configX - padding, configY - 5, configSectionWidth + (padding * 2), configBoxHeight, cardBorderRadius);
                        ctx.fill();
                        
                        ctx.strokeStyle = colors.border;
                        ctx.lineWidth = 1;
                        drawRoundedRect(configX - padding, configY - 5, configSectionWidth + (padding * 2), configBoxHeight, cardBorderRadius);
                        ctx.stroke();
                        
                        // Section header
                        ctx.fillStyle = colors.sectionHeader;
                        ctx.font = fonts.sectionHeader;
                        ctx.textAlign = 'left';
                        ctx.fillText('Projection Configuration', configX, configY + 20);
                        
                        // Draw border under header
                        ctx.strokeStyle = colors.border;
                        ctx.beginPath();
                        ctx.moveTo(configX, configY + 30);
                        ctx.lineTo(configX + configSectionWidth, configY + 30);
                        ctx.stroke();
                        
                        // Draw config items
                        let configItemY = configY + 50;
                        configEntries.forEach(([key, value]) => {
                            const label = key.replace(/([A-Z])/g, ' $1').replace(/^./, str => str.toUpperCase());
                            const valStr = String(value || '--');
                            
                            // Draw label
                            ctx.fillStyle = colors.textSecondary;
                            ctx.font = fonts.configLabel;
                            ctx.textAlign = 'left';
                            ctx.fillText(label + ':', configX, configItemY);
                            
                            // Draw value
                            ctx.fillStyle = colors.textPrimary;
                            ctx.font = fonts.configValue;
                            ctx.fillText(valStr, configX + maxConfigLabelWidth + 20, configItemY);
                            
                            configItemY += 24;
                        });
                        
                        // Convert to image data
                        const compositeImageData = compositeCanvas.toDataURL('image/png');
                        resolve(compositeImageData);
                    } catch (error) {
                        reject(error);
                    }
                };
                
                chartImg.onerror = () => {
                    reject(new Error('Failed to load chart image'));
                };
                
                chartImg.src = chartImageData;
            } catch (error) {
                reject(error);
            }
        });
    }
    
    /**
     * Show export format selection modal
     */
    function showExportModal() {
        console.log('📤 showExportModal() called');
        
        if (!projectionChart) {
            console.error('❌ No chart to export');
            showError('No chart to export. Please load a projection first.');
            return;
        }
        
        if (!currentSymbol) {
            console.error('❌ No symbol data');
            showError('No symbol data available. Please load a projection first.');
            return;
        }
        
        // Remove existing modal if present
        const existingModal = document.getElementById('export-format-modal');
        if (existingModal) {
            existingModal.remove();
        }
        
        // Create export format selection modal
        const modal = document.createElement('div');
        modal.id = 'export-format-modal';
        modal.className = 'export-format-modal';
        modal.innerHTML = `
            <div class="export-format-modal-overlay"></div>
            <div class="export-format-modal-content">
                <div class="export-format-modal-header">
                    <h3>
                        <i class="fas fa-download"></i>
                        Choose Export Format
                    </h3>
                    <button class="export-modal-close" id="close-export-modal" title="Close">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="export-format-modal-body">
                    <p style="margin-bottom: 1.5rem; color: var(--text-secondary);">
                        Select the format you want to export your price projection:
                    </p>
                    <div class="export-format-options">
                        <button class="export-format-option" data-format="png" id="export-png-btn">
                            <div class="export-format-icon">
                                <i class="fas fa-image"></i>
                            </div>
                            <div class="export-format-info">
                                <h4>PNG Image</h4>
                                <p>High-quality chart image with metrics and configuration</p>
                            </div>
                            <i class="fas fa-chevron-right"></i>
                        </button>
                        <button class="export-format-option" data-format="csv" id="export-csv-btn">
                            <div class="export-format-icon">
                                <i class="fas fa-file-csv"></i>
                            </div>
                            <div class="export-format-info">
                                <h4>CSV Data</h4>
                                <p>Spreadsheet data with historical and projected prices</p>
                            </div>
                            <i class="fas fa-chevron-right"></i>
                        </button>
                    </div>
                </div>
                <div class="export-format-modal-footer">
                    <button class="btn btn-secondary" id="cancel-export-btn">
                        Cancel
                    </button>
                </div>
            </div>
        `;
        
        document.body.appendChild(modal);
        
        // Show modal with animation
        setTimeout(() => {
            modal.classList.add('active');
        }, 10);
        
        // Close button handlers
        const closeModal = () => {
            modal.classList.remove('active');
            setTimeout(() => {
                modal.remove();
            }, 300);
        };
        
        const closeBtn = modal.querySelector('#close-export-modal');
        const cancelBtn = modal.querySelector('#cancel-export-btn');
        const overlay = modal.querySelector('.export-format-modal-overlay');
        
        if (closeBtn) closeBtn.addEventListener('click', closeModal);
        if (cancelBtn) cancelBtn.addEventListener('click', closeModal);
        if (overlay) overlay.addEventListener('click', closeModal);
        
        // Export button handlers - REDESIGNED for reliability
        const pngBtn = modal.querySelector('#export-png-btn');
        const csvBtn = modal.querySelector('#export-csv-btn');
        
        if (pngBtn) {
            // Remove any existing handlers and add fresh one
            const newPngBtn = pngBtn.cloneNode(true);
            pngBtn.parentNode.replaceChild(newPngBtn, pngBtn);
            const freshPngBtn = modal.querySelector('#export-png-btn');
            
            if (freshPngBtn) {
                freshPngBtn.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    console.log('📥 PNG export selected');
                    closeModal();
                    setTimeout(() => {
                        exportChartAsPNG().catch(err => {
                            console.error('PNG export error:', err);
                            showError('Failed to export PNG: ' + (err.message || 'Unknown error'));
                        });
                    }, 100);
                    return false;
                };
            }
        } else {
            console.error('PNG button not found in modal');
        }
        
        if (csvBtn) {
            // Remove any existing handlers and add fresh one
            const newCsvBtn = csvBtn.cloneNode(true);
            csvBtn.parentNode.replaceChild(newCsvBtn, csvBtn);
            const freshCsvBtn = modal.querySelector('#export-csv-btn');
            
            if (freshCsvBtn) {
                freshCsvBtn.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    console.log('📥 CSV export selected');
                    closeModal();
                    setTimeout(() => {
                        exportChartAsCSV().catch(err => {
                            console.error('CSV export error:', err);
                            showError('Failed to export CSV: ' + (err.message || 'Unknown error'));
                        });
                    }, 100);
                    return false;
                };
            }
        } else {
            console.error('CSV button not found in modal');
        }
        
        console.log('✅ Export modal setup complete');
    }
    
    /**
     * Export chart as high-quality PNG image
     * Supports PNG format with full chart resolution
     * Now includes Last Updated, Price Metrics, Validation Metrics, and Projection Configuration
     */
    async function exportChartAsPNG() {
        console.log('📤 exportChartAsPNG() called');
        console.log('Chart exists:', !!projectionChart);
        console.log('Current symbol:', currentSymbol);
        
        if (!projectionChart) {
            console.error('❌ No chart to export');
            showError('No chart to export. Please load a projection first.');
            return;
        }
        
        if (!currentSymbol) {
            console.error('❌ No symbol data');
            showError('No symbol data available. Please load a projection first.');
            return;
        }
        
        const exportBtn = document.getElementById('export-chart-btn');
        const originalHTML = exportBtn ? exportBtn.innerHTML : '';
        
        try {
            // Disable button during export
            if (exportBtn) {
                exportBtn.disabled = true;
                exportBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Exporting...';
            }
            
            // Get chart canvas - this is the most reliable method
            const canvas = projectionChart.canvas;
            
            if (!canvas) {
                throw new Error('Chart canvas not found');
            }
            
            console.log('✓ Canvas found:', canvas);
            console.log('Canvas dimensions:', canvas.width, 'x', canvas.height);
            
            // Try multiple methods to get image data
            let imageData = null;
            
            // Method 1: Try Chart.js toBase64Image (Chart.js 3.x+)
            if (typeof projectionChart.toBase64Image === 'function') {
                console.log('Using Chart.js toBase64Image()');
                try {
                    imageData = projectionChart.toBase64Image('image/png');
                    console.log('✓ Got image data via toBase64Image');
                } catch (e) {
                    console.warn('toBase64Image failed, trying fallback:', e);
                }
            }
            
            // Method 2: Try canvas toDataURL (most reliable)
            if (!imageData || imageData === 'data:,') {
                console.log('Using canvas.toDataURL()');
                try {
                    imageData = canvas.toDataURL('image/png');
                    console.log('✓ Got image data via toDataURL');
                } catch (e) {
                    console.error('toDataURL failed:', e);
                    throw new Error('Failed to generate image data: ' + e.message);
                }
            }
            
            // Validate image data
            if (!imageData || imageData === 'data:,') {
                throw new Error('Failed to generate image data - empty result');
            }
            
            if (imageData.length < 100) {
                throw new Error('Image data too short - export may have failed');
            }
            
            console.log('✓ Image data generated, length:', imageData.length);
            
            // Collect export data (metrics, configuration, etc.)
            const exportData = collectExportData();
            console.log('📊 Export data collected:', exportData);
            
            // Create composite image with chart and additional information
            console.log('🖼️ Creating composite image...');
            const compositeImageData = await createCompositeExportImage(imageData, exportData);
            console.log('✓ Composite image created, length:', compositeImageData.length);
            
            // Create filename
            const estNow = getCurrentEST();
            const dateStr = estNow.toISOString().split('T')[0]; // YYYY-MM-DD
            const timeStr = estNow.toTimeString().split(' ')[0].replace(/:/g, '-'); // HH-MM-SS
            const symbol = currentSymbol || 'PROJECTION';
            const interval = currentInterval || '1H';
            
            // Format: SYMBOL_INTERVAL_projection_YYYY-MM-DD_HH-MM-SS.png
            const filename = `${symbol}_${interval}_projection_${dateStr}_${timeStr}.png`;
            
            console.log('📁 Filename:', filename);
            
            // Create download link
            const link = document.createElement('a');
            link.download = filename;
            link.href = compositeImageData;
            link.style.display = 'none'; // Hide the link
            
            // Append to body and trigger download
            document.body.appendChild(link);
            
            // Use both click() and programmatic download
            try {
                link.click();
                console.log('✓ Download triggered via click()');
            } catch (clickError) {
                console.warn('click() failed, trying alternative method:', clickError);
                // Alternative: create a blob URL
                const blob = dataURLtoBlob(compositeImageData);
                const blobUrl = URL.createObjectURL(blob);
                link.href = blobUrl;
                link.click();
                setTimeout(() => URL.revokeObjectURL(blobUrl), 100);
            }
            
            // Remove link after a short delay
            setTimeout(() => {
                if (link.parentNode) {
                    document.body.removeChild(link);
                }
            }, 100);
            
            // Show success message
            if (exportBtn) {
                exportBtn.innerHTML = '<i class="fas fa-check"></i> Exported!';
                exportBtn.style.background = 'var(--success-color)';
                
                setTimeout(() => {
                    exportBtn.innerHTML = originalHTML;
                    exportBtn.style.background = '';
                    exportBtn.disabled = false;
                }, 2000);
            }
            
            console.log(`✅ Chart exported successfully: ${filename}`);
            
        } catch (error) {
            console.error('❌ Error exporting chart:', error);
            console.error('Error stack:', error.stack);
            showError(`Failed to export chart: ${error.message || 'Unknown error'}`);
            
            // Reset button on error
            if (exportBtn) {
                exportBtn.disabled = false;
                exportBtn.innerHTML = originalHTML || '<i class="fas fa-download"></i> Export';
                exportBtn.style.background = '';
            }
        }
    }
    
    /**
     * Export chart data as CSV file
     * Includes historical prices and projected prices
     */
    async function exportChartAsCSV() {
        console.log('📤 exportChartAsCSV() called');
        
        if (!projectionChart) {
            console.error('❌ No chart to export');
            showError('No chart to export. Please load a projection first.');
            return;
        }
        
        if (!currentSymbol) {
            console.error('❌ No symbol data');
            showError('No symbol data available. Please load a projection first.');
            return;
        }
        
        const exportBtn = document.getElementById('export-chart-btn');
        const originalHTML = exportBtn ? exportBtn.innerHTML : '';
        
        try {
            // Disable button during export
            if (exportBtn) {
                exportBtn.disabled = true;
                exportBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Exporting...';
            }
            
            // Collect all data points
            const csvRows = [];
            
            // Header row
            csvRows.push('Date,Time,Historical Price,Projected Price,Projection Type');
            
            // Generate projection labels
            const steps = currentProjectionParams?.steps || 20;
            const projectionLabels = generateProjectionLabels(steps, currentInterval || '1H', historicalLabels);
            
            // Combine historical and projection labels
            const allLabels = [...(historicalLabels || []), ...projectionLabels];
            const maxLength = Math.max(
                historicalPrices.length,
                currentProjectionLines.length > 0 ? currentProjectionLines[0].points.length : 0
            );
            
            // Process each data point
            for (let i = 0; i < allLabels.length; i++) {
                const label = allLabels[i];
                let historicalPrice = '';
                let projectedPrice = '';
                let projectionType = '';
                
                // Get historical price if available
                if (i < historicalPrices.length && historicalPrices[i] !== undefined && historicalPrices[i] !== null) {
                    historicalPrice = historicalPrices[i].toFixed(2);
                }
                
                // Get projected price from first projection line
                if (currentProjectionLines.length > 0 && currentProjectionLines[0].points) {
                    const projectionIndex = i - historicalPrices.length;
                    if (projectionIndex >= 0 && projectionIndex < currentProjectionLines[0].points.length) {
                        const price = currentProjectionLines[0].points[projectionIndex];
                        if (price !== undefined && price !== null && !isNaN(price)) {
                            projectedPrice = price.toFixed(2);
                            // Get projection type from line
                            const line = currentProjectionLines[0];
                            if (line.triad && Array.isArray(line.triad)) {
                                projectionType = line.triad.join('-');
                            } else if (line.label) {
                                projectionType = line.label;
                            } else {
                                projectionType = 'Projection';
                            }
                        }
                    }
                }
                
                // Parse label to separate date and time if possible
                let date = label || '';
                let time = '';
                if (label && label.includes(' ')) {
                    const parts = label.split(' ');
                    date = parts[0];
                    time = parts.slice(1).join(' ');
                }
                
                // Only add row if we have at least one price value
                if (historicalPrice || projectedPrice) {
                    csvRows.push(`"${date}","${time}","${historicalPrice}","${projectedPrice}","${projectionType}"`);
                }
            }
            
            // Add metadata section
            csvRows.push(''); // Empty row
            csvRows.push('Metadata');
            csvRows.push(`Symbol,${currentSymbol}`);
            csvRows.push(`Interval,${currentInterval || '1H'}`);
            csvRows.push(`Total Data Points,${allLabels.length}`);
            csvRows.push(`Historical Points,${historicalPrices.length}`);
            csvRows.push(`Projection Points,${currentProjectionLines.length > 0 ? currentProjectionLines[0].points.length : 0}`);
            
            // Add export data if available
            const exportData = collectExportData();
            if (exportData) {
                csvRows.push('');
                csvRows.push('Price Metrics');
                Object.entries(exportData.priceMetrics || {}).forEach(([key, value]) => {
                    csvRows.push(`${key},${value}`);
                });
                
                csvRows.push('');
                csvRows.push('Validation Metrics');
                Object.entries(exportData.validationMetrics || {}).forEach(([key, value]) => {
                    csvRows.push(`${key},${value}`);
                });
                
                csvRows.push('');
                csvRows.push('Projection Configuration');
                Object.entries(exportData.projectionConfig || {}).forEach(([key, value]) => {
                    csvRows.push(`${key},${value}`);
                });
            }
            
            // Create CSV content
            const csvContent = csvRows.join('\n');
            
            // Create filename
            const estNow = getCurrentEST();
            const dateStr = estNow.toISOString().split('T')[0]; // YYYY-MM-DD
            const timeStr = estNow.toTimeString().split(' ')[0].replace(/:/g, '-'); // HH-MM-SS
            const symbol = currentSymbol || 'PROJECTION';
            const interval = currentInterval || '1H';
            
            // Format: SYMBOL_INTERVAL_projection_YYYY-MM-DD_HH-MM-SS.csv
            const filename = `${symbol}_${interval}_projection_${dateStr}_${timeStr}.csv`;
            
            console.log('📁 CSV Filename:', filename);
            console.log('📊 CSV Rows:', csvRows.length);
            
            // Create download link
            const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
            const url = URL.createObjectURL(blob);
            const link = document.createElement('a');
            link.href = url;
            link.download = filename;
            link.style.display = 'none';
            
            // Append to body and trigger download
            document.body.appendChild(link);
            link.click();
            
            // Cleanup
            setTimeout(() => {
                if (link.parentNode) {
                    document.body.removeChild(link);
                }
                URL.revokeObjectURL(url);
            }, 100);
            
            // Show success message
            if (exportBtn) {
                exportBtn.innerHTML = '<i class="fas fa-check"></i> Exported!';
                exportBtn.style.background = 'var(--success-color)';
                
                setTimeout(() => {
                    exportBtn.innerHTML = originalHTML;
                    exportBtn.style.background = '';
                    exportBtn.disabled = false;
                }, 2000);
            }
            
            console.log(`✅ Chart exported successfully as CSV: ${filename}`);
            
        } catch (error) {
            console.error('❌ Error exporting CSV:', error);
            console.error('Error stack:', error.stack);
            showError(`Failed to export CSV: ${error.message || 'Unknown error'}`);
            
            // Reset button on error
            if (exportBtn) {
                exportBtn.disabled = false;
                exportBtn.innerHTML = originalHTML || '<i class="fas fa-download"></i> Export';
                exportBtn.style.background = '';
            }
        }
    }
    
    /**
     * Export chart - shows modal to choose format
     */
    async function exportChart() {
        console.log('📤 exportChart() called - showing modal');
        showExportModal();
    }
    
    /**
     * Helper function to convert data URL to Blob
     */
    function dataURLtoBlob(dataURL) {
        const arr = dataURL.split(',');
        const mime = arr[0].match(/:(.*?);/)[1];
        const bstr = atob(arr[1]);
        let n = bstr.length;
        const u8arr = new Uint8Array(n);
        while (n--) {
            u8arr[n] = bstr.charCodeAt(n);
        }
        return new Blob([u8arr], { type: mime });
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
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            if (!result.success || !result.projections) {
                throw new Error(result.message || 'No projections found');
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
            
            // Update active timeframe button (using chart-timeframe-selector like Trading Charts tab)
            const timeframeBtns = document.querySelectorAll('#tab-projections .chart-timeframe-selector .timeframe-btn');
            timeframeBtns.forEach(btn => {
                btn.classList.remove('active');
                if (btn.dataset.timeframe === currentInterval) {
                    btn.classList.add('active');
                }
            });
            
            // FORCE 1H TIMEFRAME - only allowed timeframe
            currentInterval = '1H';
            const oneHourBtn = document.querySelector('#tab-projections .chart-timeframe-selector .timeframe-btn[data-timeframe="1H"]');
            if (oneHourBtn) {
                oneHourBtn.classList.add('active');
            }
            
            // Use saved historical data
            historicalPrices = projectionData.historicalPrices || [];
            historicalLabels = projectionData.historicalLabels || [];
            const savedProjectionLines = projectionData.projectionLines || [];
            const savedParams = projectionData.params || (typeof proj.params === 'string' ? JSON.parse(proj.params) : proj.params) || {};
            
            // Fetch actual price data from saved date to now
            const savedDate = new Date(proj.saved_at);
            showLoading(true);
            
            try {
                // Fetch current market data
                const currentData = await fetchMarketData(currentSymbol, currentInterval);
                
                if (currentData && currentData.candles && currentData.candles.length > 0) {
                    // Convert saved date to EST for comparison
                    const savedDateEST = toEST(savedDate);
                    
                    // Filter candles to only include those after the saved date (using EST)
                    const actualCandles = currentData.candles.filter(c => {
                        const candleTimestamp = c.time || c.timestamp;
                        if (!candleTimestamp) return false;
                        const candleDateEST = toEST(new Date(candleTimestamp));
                        return candleDateEST >= savedDateEST;
                    });
                    
                    // Convert all actual candle timestamps to EST
                    const actualCandlesEST = actualCandles.map(c => {
                        const candle = { ...c };
                        if (candle.time) {
                            candle.time = toESTTimestamp(candle.time);
                        }
                        if (candle.timestamp) {
                            candle.timestamp = toESTTimestamp(candle.timestamp);
                        }
                        return candle;
                    });
                    
                    // Extract actual prices
                    actualPriceData = {
                        prices: actualCandlesEST.map(c => parseFloat(c.close || c.price || 0)).filter(p => !isNaN(p) && p > 0),
                        labels: actualCandlesEST.map(c => {
                            const timestamp = c.time || c.timestamp;
                            if (!timestamp) return '';
                            const estDate = new Date(timestamp);
                            return estDate.toLocaleDateString('en-US', {
                                timeZone: 'America/New_York',
                                year: 'numeric',
                                month: 'short',
                                day: 'numeric'
                            });
                        }).filter(l => l)
                    };
                }
            } catch (error) {
                console.warn('Could not fetch actual price data:', error);
                actualPriceData = null;
            }
            
            // Render chart with both saved projection and actual data
            renderChart(
                historicalPrices, 
                historicalLabels, 
                savedProjectionLines, 
                savedParams,
                actualPriceData ? actualPriceData.prices : null,
                actualPriceData ? actualPriceData.labels : null
            );
            
            // Update metrics
            updateMetrics(historicalPrices, savedProjectionLines, savedParams);
            updateComputationMethodDisplay();
            
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
        
        // formatVolume is now defined at the top of the module (line ~64) for accessibility
        
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
    
    /**
     * Reset metrics to placeholder values on error
     */
    function resetMetricsToPlaceholders() {
        // Reset all metric elements to placeholder values
        const metricElements = [
            'metric-current-price',
            'metric-historical-change',
            'metric-historical-percent',
            'metric-projected-price',
            'metric-projected-change',
            'metric-projected-percent',
            'metric-period-high',
            'metric-period-low',
            'metric-average-volume',
            'metric-actual-volume'
        ];
        
        metricElements.forEach(id => {
            const el = document.getElementById(id);
            if (el) {
                el.textContent = '--';
                el.className = 'metric-value';
            }
        });
        
        // Reset validation metrics if function exists
        if (typeof resetValidationMetricsToPlaceholders === 'function') {
            resetValidationMetricsToPlaceholders();
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
            showError('No chart available. Please load a projection first.');
            return;
        }
        
        try {
            // Try zoom plugin's resetZoom method first
            if (typeof projectionChart.resetZoom === 'function') {
                projectionChart.resetZoom();
                console.log('✓ Zoom reset using plugin method');
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
            
            // Force chart update with animation
            projectionChart.update('active');
            console.log('✓ Zoom reset using manual method');
        } catch (error) {
            console.error('Error resetting zoom:', error);
            // Fallback: update chart
            if (projectionChart) {
                projectionChart.update('active');
            }
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
        const loadBtn = document.getElementById('projection-load-btn');
        const refreshBtn = document.getElementById('projection-refresh-btn');
        const resetZoomBtn = document.getElementById('reset-zoom-btn');
        const symbolInput = document.getElementById('projection-symbol-input');
        
        // Load button (replaces search button)
        if (loadBtn) {
            loadBtn.addEventListener('click', loadProjectionData);
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
        
        // Setup export functionality - call multiple times to ensure it works
        setupExportFunctionality();
        
        // Also setup export on window load as fallback
        if (window.addEventListener) {
            window.addEventListener('load', function() {
                setTimeout(setupExportFunctionality, 1000);
            });
        }
        
        // Expose export function globally for debugging and backward compatibility
        window.exportProjectionChart = exportChart;
        console.log('📤 Export function exposed as window.exportProjectionChart()');
        console.log('📤 Export function type:', typeof exportChart);
        
        if (symbolInput) {
            symbolInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    loadProjectionData();
                }
            });
        }
        
        // Setup timeframe buttons (matching Trading Charts tab)
        // ONLY ALLOW 1H TIMEFRAME - disable others
        document.querySelectorAll('#tab-projections .chart-timeframe-selector .timeframe-btn').forEach(btn => {
            const timeframe = btn.dataset.timeframe;
            
            // Disable all timeframes except 1H
            if (timeframe !== '1H') {
                btn.disabled = true;
                btn.style.opacity = '0.5';
                btn.style.cursor = 'not-allowed';
                return; // Skip event listener for disabled buttons
            }
            
            // Only 1H is enabled
            btn.addEventListener('click', () => {
                // Remove active class from all timeframe buttons in this selector
                document.querySelectorAll('#tab-projections .chart-timeframe-selector .timeframe-btn').forEach(b => {
                    if (!b.disabled) {
                        b.classList.remove('active');
                    }
                });
                // Add active class to clicked button
                btn.classList.add('active');
                // Update current interval
                currentInterval = '1H'; // Force to 1H
                console.log(`Timeframe set to: ${currentInterval} (only allowed timeframe)`);
                // Reload projection data if we have a symbol loaded
                if (currentSymbol) {
                    loadProjectionData();
                }
            });
        });
        
        // Force 1H timeframe on initialization
        currentInterval = '1H';
        const oneHourBtn = document.querySelector('#tab-projections .chart-timeframe-selector .timeframe-btn[data-timeframe="1H"]');
        if (oneHourBtn) {
            oneHourBtn.classList.add('active');
        }
        
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
                    }
                    // Do NOT restart auto-refresh - user must click Load button
                    // stopAutoRefresh(); // Ensure auto-refresh is stopped
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

