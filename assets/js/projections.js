// Projections Module - Price Projection Engine
const ProjectionsModule = (function() {
    let projectionChart = null;
    let currentSymbol = '';
    let currentInterval = '1d';
    let historicalPrices = [];
    let historicalLabels = [];
    
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
    
    // Fetch market data
    async function fetchMarketData(symbol, interval) {
        try {
            // Map interval to timeframe
            const timeframeMap = {
                '1d': '1D',
                '5d': '5D',
                '1mo': '1M',
                '3mo': '3M',
                '6mo': '6M',
                '1y': '1Y'
            };
            const timeframe = timeframeMap[interval] || '1D';
            
            const response = await fetch(`api/charts.php?action=chart&symbol=${symbol}&timeframe=${timeframe}`);
            
            if (!response.ok) {
                const errorText = await response.text();
                let errorMessage = 'Failed to fetch market data';
                try {
                    const errorData = JSON.parse(errorText);
                    errorMessage = errorData.message || errorMessage;
                } catch (e) {
                    // Not JSON, use default message
                }
                throw new Error(errorMessage);
            }
            
            const data = await response.json();
            
            // Log the response for debugging
            console.log('Chart API response:', {
                success: data.success,
                hasCandles: !!data.candles,
                candlesCount: data.candles ? data.candles.length : 0,
                symbol: data.symbol,
                message: data.message
            });
            
            // Check if response indicates failure
            if (data.success === false) {
                const errorMsg = data.message || 'Unable to fetch market data. Please check your API configuration or try again later.';
                console.error('API returned error:', errorMsg);
                throw new Error(errorMsg);
            }
            
            // Check if we have candles data
            if (!data.candles || !Array.isArray(data.candles) || data.candles.length === 0) {
                const errorMsg = data.message || 'No historical data available for this symbol. Please try a different symbol or timeframe.';
                console.error('No candles data:', errorMsg);
                throw new Error(errorMsg);
            }
            
            console.log('Successfully fetched', data.candles.length, 'candles for', symbol);
            return data;
        } catch (error) {
            console.error('Error fetching market data:', error);
            throw error;
        }
    }
    
    // Load chart data and calculate projections
    async function loadProjectionData() {
        const symbolInput = document.getElementById('projection-symbol-input');
        const symbol = symbolInput.value.trim().toUpperCase();
        
        if (!symbol) {
            showError('Please enter a symbol');
            return;
        }
        
        currentSymbol = symbol;
        currentInterval = document.getElementById('projection-interval-select').value;
        
        showLoading(true);
        hideError();
        
        try {
            const data = await fetchMarketData(symbol, currentInterval);
            
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
            
            // Extract prices from candles
            historicalPrices = data.candles.map(c => {
                const price = parseFloat(c.close || c.price || 0);
                if (isNaN(price) || price <= 0) {
                    console.warn('Invalid price data:', c);
                }
                return price;
            }).filter(price => !isNaN(price) && price > 0);
            
            if (historicalPrices.length === 0) {
                throw new Error('No valid price data found');
            }
            
            historicalLabels = data.candles.map((c, i) => {
                // Time is already in milliseconds from the API
                if (c.time) {
                    const date = new Date(c.time);
                    return date.toLocaleDateString();
                }
                if (c.timestamp) {
                    const date = new Date(c.timestamp);
                    return date.toLocaleDateString();
                }
                return `Point ${i + 1}`;
            });
            
            const params = {
                steps: parseInt(document.getElementById('projection-steps').value) || 20,
                base: parseFloat(document.getElementById('projection-base').value) || 3,
                projectionCount: parseInt(document.getElementById('projection-count').value) || 12,
                depthPrime: parseInt(document.getElementById('projection-depth').value) || 31
            };
            
            const projectionLines = calculateProjections(historicalPrices, params);
            renderChart(historicalPrices, historicalLabels, projectionLines, params);
            updateMetrics(historicalPrices, projectionLines, params);
            
            document.getElementById('projection-refresh-btn').style.display = 'inline-block';
            document.getElementById('reset-zoom-btn').style.display = 'inline-block';
            document.getElementById('save-projection-btn').style.display = 'inline-block';
            document.getElementById('projections-metrics-section').style.display = 'block';
            
        } catch (error) {
            console.error('Error loading projection data:', error);
            const errorMessage = error.message || 'Failed to load projection data. Please check the symbol and try again.';
            showError(errorMessage);
            
            // Hide metrics section on error
            document.getElementById('projections-metrics-section').style.display = 'none';
            document.getElementById('projection-refresh-btn').style.display = 'none';
            document.getElementById('reset-zoom-btn').style.display = 'none';
            document.getElementById('save-projection-btn').style.display = 'none';
        } finally {
            showLoading(false);
        }
    }
    
    // Render chart
    function renderChart(historicalPrices, historicalLabels, projectionLines, params) {
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
        
        // Generate projection labels
        const projectedLabels = [];
        for (let i = 1; i <= steps; i++) {
            projectedLabels.push(`Step ${i}`);
        }
        
        const allLabels = [...historicalLabels, ...projectedLabels];
        
        // Historical data
        const historicalData = [...historicalPrices];
        for (let i = 0; i < steps; i++) {
            historicalData.push(null);
        }
        
        // Build datasets
        const datasets = [
            {
                label: `${currentSymbol} Historical`,
                data: historicalData,
                borderColor: 'rgb(59, 130, 246)',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                tension: 0.4,
                fill: false,
                pointRadius: 2,
                borderWidth: 2
            }
        ];
        
        // Add projection lines
        const colors = [
            '#8b5cf6', '#10b981', '#f59e0b', '#ef4444', '#ec4899',
            '#14b8a6', '#f97316', '#6366f1', '#84cc16', '#06b6d4',
            '#a855f7', '#22c55e'
        ];
        
        projectionLines.forEach((line, idx) => {
            const lineData = [];
            for (let i = 0; i < historicalPrices.length; i++) {
                lineData.push(null);
            }
            
            // First point connects to last historical
            if (line.points.length > 0) {
                const firstProjected = line.points[0];
                const connectionPrice = lastPrice * 0.8 + firstProjected * 0.2;
                lineData.push(connectionPrice);
            }
            
            // Rest of projection points
            for (let i = 1; i < line.points.length; i++) {
                lineData.push(line.points[i]);
            }
            
            // Fill remaining with nulls if needed
            while (lineData.length < allLabels.length) {
                lineData.push(null);
            }
            
            datasets.push({
                label: `Triad [${line.triad.join('-')}]`,
                data: lineData,
                borderColor: colors[idx % colors.length],
                backgroundColor: 'transparent',
                borderWidth: 2,
                pointRadius: 0,
                tension: 0.4,
                borderDash: [5, 5]
            });
        });
        
        // Destroy existing chart
        if (projectionChart) {
            projectionChart.destroy();
        }
        
        // Register zoom plugin if available
        if (typeof Chart !== 'undefined' && Chart.register) {
            try {
                // Try to register zoom plugin if it's available
                if (typeof zoomPlugin !== 'undefined') {
                    Chart.register(zoomPlugin);
                }
            } catch (e) {
                console.warn('Zoom plugin not available:', e);
            }
        }
        
        // Create new chart
        projectionChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: allLabels,
                datasets: datasets
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: true,
                        position: 'top'
                    },
                    tooltip: {
                        mode: 'index',
                        intersect: false
                    }
                },
                scales: {
                    x: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    },
                    y: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Price'
                        }
                    }
                }
            }
        });
        
        // Add zoom plugin options if available
        if (typeof zoomPlugin !== 'undefined' && projectionChart.options.plugins) {
            projectionChart.options.plugins.zoom = {
                zoom: {
                    wheel: {
                        enabled: true
                    },
                    pinch: {
                        enabled: true
                    },
                    mode: 'xy'
                },
                pan: {
                    enabled: true,
                    mode: 'xy'
                }
            };
            projectionChart.update();
        }
        
        // Update chart title
        document.getElementById('projection-chart-title').textContent = 
            `${currentSymbol} - ${currentInterval} - ${steps} Step Projection`;
    }
    
    // Update metrics
    function updateMetrics(historicalPrices, projectionLines, params) {
        const firstPrice = historicalPrices[0];
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        const change = lastPrice - firstPrice;
        const changePercent = firstPrice !== 0 ? (change / firstPrice) * 100 : 0;
        
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
        
        // Update DOM
        document.getElementById('metric-current-price').textContent = `$${lastPrice.toFixed(2)}`;
        
        const histChangeEl = document.getElementById('metric-historical-change');
        histChangeEl.textContent = `${change >= 0 ? '+' : ''}$${Math.abs(change).toFixed(2)}`;
        histChangeEl.className = 'metric-value ' + (change >= 0 ? 'positive' : 'negative');
        
        document.getElementById('metric-historical-percent').textContent = 
            `${changePercent >= 0 ? '+' : ''}${changePercent.toFixed(2)}%`;
        
        document.getElementById('metric-projected-price').textContent = `$${avgProjectedPrice.toFixed(2)}`;
        
        const projChangeEl = document.getElementById('metric-projected-change');
        projChangeEl.textContent = `${projectedChange >= 0 ? '+' : ''}$${Math.abs(projectedChange).toFixed(2)}`;
        projChangeEl.className = 'metric-value ' + (projectedChange >= 0 ? 'positive' : 'negative');
        
        document.getElementById('metric-projected-percent').textContent = 
            `${projectedChangePercent >= 0 ? '+' : ''}${projectedChangePercent.toFixed(2)}%`;
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
    
    // Reset zoom
    function resetZoom() {
        if (projectionChart) {
            if (projectionChart.resetZoom) {
                projectionChart.resetZoom();
            } else {
                // Fallback: update chart to reset view
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
            const params = {
                steps: parseInt(document.getElementById('projection-steps').value) || 20,
                base: parseFloat(document.getElementById('projection-base').value) || 3,
                projectionCount: parseInt(document.getElementById('projection-count').value) || 12,
                depthPrime: parseInt(document.getElementById('projection-depth').value) || 31
            };
            
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
            
            const saveData = {
                symbol: currentSymbol,
                title: `${currentSymbol} - ${currentInterval} Projection`,
                projection_data: {
                    symbol: currentSymbol,
                    interval: currentInterval,
                    historicalPrices: historicalPrices,
                    historicalLabels: historicalLabels,
                    projectionLines: projectionLines,
                    params: params,
                    lastPrice: historicalPrices[historicalPrices.length - 1]
                },
                chart_data: chartData,
                params: params,
                notes: `Projection for ${currentSymbol} with ${params.steps} steps, base ${params.base}, depth ${params.depthPrime}`
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
            refreshBtn.addEventListener('click', loadProjectionData);
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
        
        // Watch for page activation
        watchPageActivation();
    }
    
    // Watch for page activation
    function watchPageActivation() {
        const pageEl = document.getElementById('page-projections');
        if (!pageEl) return;
        
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (pageEl.classList.contains('active')) {
                        // Page is active, ensure chart is ready
                        if (projectionChart && currentSymbol) {
                            // Chart already exists, do nothing
                        }
                    }
                }
            });
        });
        
        observer.observe(pageEl, {
            attributes: true,
            attributeFilter: ['class']
        });
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    return {
        init: init,
        loadProjectionData: loadProjectionData
    };
})();

