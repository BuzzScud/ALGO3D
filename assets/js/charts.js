// Charts Module - Trading Charts with Technical Analysis
const ChartsModule = (function() {
    let mainChart = null;
    let volumeChart = null;
    let currentSymbol = '';
    let currentTimeframe = '1D';
    let currentChartType = 'candlestick';
    let chartData = null;
    let activeIndicators = {
        sma20: false,
        sma50: false,
        ema12: false,
        ema26: false,
        bollinger: false,
        volume: true
    };

    // Register zoom plugin when Chart.js is available
    function registerZoomPlugin() {
        if (typeof Chart !== 'undefined' && typeof zoomPlugin !== 'undefined') {
            Chart.register(zoomPlugin);
        } else if (typeof Chart !== 'undefined' && window.zoomPlugin) {
            Chart.register(window.zoomPlugin);
        }
    }

    // Try to register on load
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', registerZoomPlugin);
    } else {
        registerZoomPlugin();
    }

    // Chart colors
    const colors = {
        bullish: '#22c55e',
        bearish: '#ef4444',
        neutral: '#6b7280',
        sma20: '#3b82f6',
        sma50: '#f59e0b',
        ema12: '#8b5cf6',
        ema26: '#ec4899',
        bollingerUpper: 'rgba(156, 163, 175, 0.5)',
        bollingerLower: 'rgba(156, 163, 175, 0.5)',
        volume: 'rgba(99, 102, 241, 0.5)',
        grid: 'rgba(255, 255, 255, 0.1)',
        text: '#9ca3af'
    };

    function init() {
        setupEventListeners();
        loadWatchlist();
        
        // Auto-load QQQ when Charts page is active
        checkAndLoadDefaultChart();
        
        // Watch for page visibility changes
        observePageVisibility();
    }

    function checkAndLoadDefaultChart() {
        const chartsPage = document.getElementById('page-charts');
        if (chartsPage && chartsPage.classList.contains('active')) {
            // Auto-load QQQ symbol with a small delay to ensure DOM is ready
            setTimeout(() => {
                const symbolInput = document.getElementById('chart-symbol-input');
                if (symbolInput) {
                    symbolInput.value = 'QQQ';
                    loadChart('QQQ');
                }
            }, 200);
        }
    }

    function observePageVisibility() {
        // Watch for when Charts page becomes active
        const chartsPage = document.getElementById('page-charts');
        if (!chartsPage) return;

        // Check if page is already active
        if (chartsPage.classList.contains('active')) {
            setTimeout(() => {
                const symbolInput = document.getElementById('chart-symbol-input');
                if (symbolInput) {
                    symbolInput.value = 'QQQ';
                    loadChart('QQQ');
                }
            }, 300);
        }

        // Use MutationObserver to watch for class changes
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (chartsPage.classList.contains('active')) {
                        // Page just became active, auto-load QQQ
                        setTimeout(() => {
                            const symbolInput = document.getElementById('chart-symbol-input');
                            if (symbolInput) {
                                symbolInput.value = 'QQQ';
                                loadChart('QQQ');
                            }
                        }, 100);
                    }
                }
            });
        });

        observer.observe(chartsPage, {
            attributes: true,
            attributeFilter: ['class']
        });
    }

    // Zoom functions
    function zoomIn() {
        if (!mainChart) return;
        
        const xScale = mainChart.scales.x;
        const yScale = mainChart.scales.y;
        const centerX = (xScale.max + xScale.min) / 2;
        const centerY = (yScale.max + yScale.min) / 2;
        const factor = 0.75;
        
        const xRange = xScale.max - xScale.min;
        const yRange = yScale.max - yScale.min;
        
        const newXRange = xRange * factor;
        const newYRange = yRange * factor;
        
        xScale.options.min = centerX - newXRange / 2;
        xScale.options.max = centerX + newXRange / 2;
        yScale.options.min = centerY - newYRange / 2;
        yScale.options.max = centerY + newYRange / 2;
        
        mainChart.update('none');
    }

    function zoomOut() {
        if (!mainChart) return;
        
        const xScale = mainChart.scales.x;
        const yScale = mainChart.scales.y;
        const centerX = (xScale.max + xScale.min) / 2;
        const centerY = (yScale.max + yScale.min) / 2;
        const factor = 1.33;
        
        const xRange = xScale.max - xScale.min;
        const yRange = yScale.max - yScale.min;
        
        // Get original data range if available
        let maxXRange = Infinity;
        let maxYRange = Infinity;
        
        if (chartData && chartData.candles.length > 0) {
            const times = chartData.candles.map(c => c.time);
            const prices = chartData.candles.flatMap(c => [c.high, c.low]);
            maxXRange = Math.max(...times) - Math.min(...times);
            maxYRange = (Math.max(...prices) - Math.min(...prices)) * 1.1;
        }
        
        const newXRange = Math.min(xRange * factor, maxXRange);
        const newYRange = Math.min(yRange * factor, maxYRange);
        
        xScale.options.min = centerX - newXRange / 2;
        xScale.options.max = centerX + newXRange / 2;
        yScale.options.min = centerY - newYRange / 2;
        yScale.options.max = centerY + newYRange / 2;
        
        mainChart.update('none');
    }

    function resetZoom() {
        if (!mainChart) return;
        
        const xScale = mainChart.scales.x;
        const yScale = mainChart.scales.y;
        
        // Reset to original range
        delete xScale.options.min;
        delete xScale.options.max;
        delete yScale.options.min;
        delete yScale.options.max;
        
        mainChart.update('none');
    }

    function setupEventListeners() {
        // Symbol input
        const symbolInput = document.getElementById('chart-symbol-input');
        const loadChartBtn = document.getElementById('load-chart-btn');
        
        if (symbolInput) {
            symbolInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    loadChart(symbolInput.value.trim().toUpperCase());
                }
            });
        }
        
        if (loadChartBtn) {
            loadChartBtn.addEventListener('click', () => {
                const symbol = symbolInput.value.trim().toUpperCase();
                if (symbol) loadChart(symbol);
            });
        }

        // Timeframe buttons
        document.querySelectorAll('.timeframe-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.timeframe-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                currentTimeframe = btn.dataset.timeframe;
                if (currentSymbol) loadChart(currentSymbol);
            });
        });

        // Chart type buttons
        document.querySelectorAll('.chart-type-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.chart-type-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                currentChartType = btn.dataset.type;
                if (chartData) renderChart();
            });
        });

        // Indicator toggles
        document.querySelectorAll('.indicator-toggle input').forEach(checkbox => {
            checkbox.addEventListener('change', (e) => {
                const indicator = e.target.dataset.indicator;
                activeIndicators[indicator] = e.target.checked;
                if (chartData) renderChart();
            });
        });

        // Quick actions
        document.getElementById('refresh-chart-btn')?.addEventListener('click', () => {
            if (currentSymbol) loadChart(currentSymbol, true);
        });

        document.getElementById('add-to-watchlist-btn')?.addEventListener('click', addToWatchlist);
        document.getElementById('create-note-btn')?.addEventListener('click', createNoteFromChart);
        document.getElementById('export-chart-btn')?.addEventListener('click', exportChart);

        // Zoom controls
        document.getElementById('zoom-in-btn')?.addEventListener('click', zoomIn);
        document.getElementById('zoom-out-btn')?.addEventListener('click', zoomOut);
        document.getElementById('zoom-reset-btn')?.addEventListener('click', resetZoom);

        // Save/Load study buttons
        document.getElementById('save-study-btn')?.addEventListener('click', openSaveStudyModal);
        document.getElementById('toggle-studies-btn')?.addEventListener('click', toggleStudiesDropdown);
        document.getElementById('close-studies-panel')?.addEventListener('click', closeStudiesDropdown);
        
        // Close dropdown when clicking outside
        document.addEventListener('click', (e) => {
            const dropdown = document.querySelector('.saved-studies-dropdown');
            if (dropdown && !dropdown.contains(e.target)) {
                dropdown.classList.remove('open');
            }
        });
        
        // Save study modal
        document.getElementById('close-save-study-modal')?.addEventListener('click', closeSaveStudyModal);
        document.getElementById('cancel-save-study-btn')?.addEventListener('click', closeSaveStudyModal);
        document.getElementById('confirm-save-study-btn')?.addEventListener('click', saveStudy);
        
        // Load study modal (legacy support)
        document.getElementById('close-load-study-modal')?.addEventListener('click', closeLoadStudyModal);
    }

    async function loadChart(symbol, forceRefresh = false) {
        if (!symbol) return;
        
        currentSymbol = symbol;
        showLoading(true);
        hideEmpty();

        try {
            // Fetch quote data
            const quoteResponse = await fetch(`api/charts.php?action=quote&symbol=${symbol}`);
            const quoteData = await quoteResponse.json();
            
            if (quoteData.success) {
                updateSymbolInfo(quoteData);
            }

            // Fetch chart data
            const chartResponse = await fetch(`api/charts.php?action=chart&symbol=${symbol}&timeframe=${currentTimeframe}`);
            chartData = await chartResponse.json();
            
            if (chartData.success) {
                updateStatistics(chartData.statistics);
                updateIndicatorValues(chartData.indicators);
                updateSignals(chartData.indicators);
                renderChart();
            } else {
                showError('Failed to load chart data');
            }
        } catch (error) {
            console.error('Chart loading error:', error);
            showError('Error loading chart data');
        } finally {
            showLoading(false);
        }
    }

    // Store quote data for use across functions
    let currentQuoteData = null;

    function updateSymbolInfo(data) {
        currentQuoteData = data;
        
        // Main header info
        document.getElementById('chart-ticker').textContent = data.symbol;
        document.getElementById('chart-company').textContent = '';
        document.getElementById('chart-current-price').textContent = formatPrice(data.current);
        
        const changeEl = document.getElementById('chart-price-change');
        const change = data.change;
        const changePercent = data.changePercent;
        
        changeEl.textContent = `${change >= 0 ? '+' : ''}${formatPrice(change)} (${changePercent >= 0 ? '+' : ''}${changePercent.toFixed(2)}%)`;
        changeEl.className = `price-change ${change >= 0 ? 'positive' : 'negative'}`;

        // Update detail row items
        updateDetailElement('detail-open', formatPrice(data.open));
        updateDetailElement('detail-high', formatPrice(data.high), 'positive');
        updateDetailElement('detail-low', formatPrice(data.low), 'negative');
        updateDetailElement('detail-prev-close', formatPrice(data.previousClose));
        updateDetailElement('detail-volume', formatVolume(data.volume || 0));
        updateDetailElement('detail-day-range', `${formatPriceShort(data.low)} - ${formatPriceShort(data.high)}`);
    }

    function updateDetailElement(id, value, className = null) {
        const el = document.getElementById(id);
        if (el) {
            el.textContent = value;
            if (className) {
                el.classList.remove('positive', 'negative');
                el.classList.add(className);
            }
        }
    }

    function formatPriceShort(price) {
        if (price === null || price === undefined) return '--';
        return '$' + price.toFixed(2);
    }

    function updateStatistics(stats) {
        document.getElementById('stat-open').textContent = formatPrice(stats.open);
        document.getElementById('stat-high').textContent = formatPrice(stats.high);
        document.getElementById('stat-low').textContent = formatPrice(stats.low);
        document.getElementById('stat-close').textContent = formatPrice(stats.close);
        document.getElementById('stat-volume').textContent = formatVolume(stats.volume);
        document.getElementById('stat-avg-volume').textContent = formatVolume(stats.avgVolume);
        document.getElementById('stat-52w-high').textContent = formatPrice(stats.high52w);

        // Update detail row for 52W and Avg Vol
        updateDetailElement('detail-52w-range', `${formatPriceShort(stats.low52w)} - ${formatPriceShort(stats.high52w)}`);
        updateDetailElement('detail-avg-vol', formatVolume(stats.avgVolume));
        document.getElementById('stat-52w-low').textContent = formatPrice(stats.low52w);
    }

    function updateIndicatorValues(indicators) {
        if (indicators.sma20?.current) {
            document.getElementById('value-sma20').textContent = formatPrice(indicators.sma20.current);
        }
        if (indicators.sma50?.current) {
            document.getElementById('value-sma50').textContent = formatPrice(indicators.sma50.current);
        }
        if (indicators.ema12?.current) {
            document.getElementById('value-ema12').textContent = formatPrice(indicators.ema12.current);
        }
        if (indicators.ema26?.current) {
            document.getElementById('value-ema26').textContent = formatPrice(indicators.ema26.current);
        }
        if (indicators.bollinger) {
            document.getElementById('value-bb').textContent = 
                `${formatPrice(indicators.bollinger.lower)} - ${formatPrice(indicators.bollinger.upper)}`;
        }
        if (chartData?.statistics?.volume) {
            document.getElementById('value-volume').textContent = formatVolume(chartData.statistics.volume);
        }
    }

    function updateSignals(indicators) {
        // RSI
        if (indicators.rsi) {
            document.getElementById('signal-rsi').textContent = indicators.rsi.value;
            const rsiIndicator = document.getElementById('signal-rsi-indicator');
            rsiIndicator.textContent = indicators.rsi.signal;
            rsiIndicator.className = `signal-indicator ${getSignalClass(indicators.rsi.signal)}`;
        }

        // MACD
        if (indicators.macd) {
            document.getElementById('signal-macd').textContent = indicators.macd.value;
            const macdIndicator = document.getElementById('signal-macd-indicator');
            macdIndicator.textContent = indicators.macd.signal;
            macdIndicator.className = `signal-indicator ${getSignalClass(indicators.macd.signal)}`;
        }

        // Trend (based on SMAs)
        if (indicators.sma20 && indicators.sma50) {
            const trend = indicators.sma20.current > indicators.sma50.current ? 'Bullish' : 'Bearish';
            document.getElementById('signal-trend').textContent = 
                `SMA20 ${trend === 'Bullish' ? '>' : '<'} SMA50`;
            const trendIndicator = document.getElementById('signal-trend-indicator');
            trendIndicator.textContent = trend;
            trendIndicator.className = `signal-indicator ${getSignalClass(trend)}`;
        }

        // Momentum (based on price vs SMA20)
        if (indicators.sma20 && chartData?.statistics?.close) {
            const close = chartData.statistics.close;
            const momentum = close > indicators.sma20.current ? 'Bullish' : 'Bearish';
            document.getElementById('signal-momentum').textContent = 
                `Price ${momentum === 'Bullish' ? '>' : '<'} SMA20`;
            const momentumIndicator = document.getElementById('signal-momentum-indicator');
            momentumIndicator.textContent = momentum;
            momentumIndicator.className = `signal-indicator ${getSignalClass(momentum)}`;
        }
    }

    function getSignalClass(signal) {
        switch (signal.toLowerCase()) {
            case 'bullish':
            case 'oversold':
                return 'bullish';
            case 'bearish':
            case 'overbought':
                return 'bearish';
            default:
                return 'neutral';
        }
    }

    function renderChart() {
        const ctx = document.getElementById('main-chart').getContext('2d');
        
        // Destroy existing chart
        if (mainChart) {
            mainChart.destroy();
        }

        const candles = chartData.candles;
        const labels = candles.map(c => new Date(c.time));
        
        // Prepare datasets
        const datasets = [];

        // Main price data
        // Determine chart type first to know how to format data
        let chartType = 'line';
        if (currentChartType === 'candlestick') {
            // Check if candlestick chart type is available (from financial plugin)
            if (typeof Chart !== 'undefined' && Chart.registry) {
                try {
                    const candlestickController = Chart.registry.getController('candlestick');
                    if (candlestickController) {
                        chartType = 'candlestick';
                    }
                } catch (e) {
                    // Candlestick type not available, use line with custom rendering
                }
            }
        }
        
        if (currentChartType === 'candlestick' && chartType === 'candlestick') {
            // Use Chart.js financial plugin for candlestick charts
            const candlestickData = candles.map(c => ({
                x: c.time,
                o: parseFloat(c.open),
                h: parseFloat(c.high),
                l: parseFloat(c.low),
                c: parseFloat(c.close)
            }));
            
            datasets.push({
                label: 'Price',
                type: 'candlestick',
                data: candlestickData,
                color: {
                    up: colors.bullish,
                    down: colors.bearish,
                    unchanged: '#6b7280'
                }
            });
        } else if (currentChartType === 'candlestick' && chartType === 'line') {
            // Fallback: add placeholder data for custom candlestick rendering
            datasets.push({
                label: 'Price',
                data: candles.map(c => c.close),
                borderColor: 'transparent',
                backgroundColor: 'transparent',
                borderWidth: 0,
                pointRadius: 0
            });
        } else if (currentChartType === 'line') {
            datasets.push({
                label: 'Price',
                data: candles.map(c => c.close),
                borderColor: colors.sma20,
                backgroundColor: 'transparent',
                borderWidth: 2,
                tension: 0.1,
                pointRadius: 0
            });
        } else if (currentChartType === 'area') {
            datasets.push({
                label: 'Price',
                data: candles.map(c => c.close),
                borderColor: colors.sma20,
                backgroundColor: 'rgba(59, 130, 246, 0.2)',
                fill: true,
                borderWidth: 2,
                tension: 0.1,
                pointRadius: 0
            });
        }

        // Add indicator overlays
        if (activeIndicators.sma20 && chartData.indicators.sma20?.values) {
            const smaData = padIndicatorData(chartData.indicators.sma20.values, candles.length);
            datasets.push({
                label: 'SMA 20',
                data: smaData,
                borderColor: colors.sma20,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [],
                tension: 0.1,
                pointRadius: 0
            });
        }

        if (activeIndicators.sma50 && chartData.indicators.sma50?.values) {
            const smaData = padIndicatorData(chartData.indicators.sma50.values, candles.length);
            datasets.push({
                label: 'SMA 50',
                data: smaData,
                borderColor: colors.sma50,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [],
                tension: 0.1,
                pointRadius: 0
            });
        }

        if (activeIndicators.ema12 && chartData.indicators.ema12?.values) {
            const emaData = padIndicatorData(chartData.indicators.ema12.values, candles.length);
            datasets.push({
                label: 'EMA 12',
                data: emaData,
                borderColor: colors.ema12,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [5, 5],
                tension: 0.1,
                pointRadius: 0
            });
        }

        if (activeIndicators.ema26 && chartData.indicators.ema26?.values) {
            const emaData = padIndicatorData(chartData.indicators.ema26.values, candles.length);
            datasets.push({
                label: 'EMA 26',
                data: emaData,
                borderColor: colors.ema26,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [5, 5],
                tension: 0.1,
                pointRadius: 0
            });
        }

        // Bollinger Bands
        if (activeIndicators.bollinger && chartData.indicators.bollinger) {
            const bb = chartData.indicators.bollinger;
            const bbUpper = Array(candles.length).fill(bb.upper);
            const bbMiddle = Array(candles.length).fill(bb.middle);
            const bbLower = Array(candles.length).fill(bb.lower);
            
            datasets.push({
                label: 'BB Upper',
                data: bbUpper,
                borderColor: colors.bollingerUpper,
                backgroundColor: 'transparent',
                borderWidth: 1,
                borderDash: [3, 3],
                tension: 0,
                pointRadius: 0
            });
            datasets.push({
                label: 'BB Lower',
                data: bbLower,
                borderColor: colors.bollingerLower,
                backgroundColor: 'transparent',
                borderWidth: 1,
                borderDash: [3, 3],
                tension: 0,
                pointRadius: 0
            });
        }

        mainChart = new Chart(ctx, {
            type: chartType,
            data: {
                labels: labels,
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
                    const canvas = event.native?.target;
                    if (canvas) {
                        canvas.style.cursor = activeElements.length > 0 ? 'pointer' : 'grab';
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top',
                        align: 'start',
                        labels: {
                            color: colors.text,
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
                            label: function(context) {
                                if (currentChartType === 'candlestick' && context.datasetIndex === 0) {
                                    // For candlestick chart, show OHLC data
                                    const candle = candles[context.dataIndex];
                                    return [
                                        `O: ${formatPrice(candle.open)}`,
                                        `H: ${formatPrice(candle.high)}`,
                                        `L: ${formatPrice(candle.low)}`,
                                        `C: ${formatPrice(candle.close)}`,
                                        `V: ${formatVolume(candle.volume)}`
                                    ];
                                } else if (context.dataset.type === 'candlestick') {
                                    // Handle candlestick dataset in tooltip
                                    const candle = candles[context.dataIndex];
                                    return [
                                        `O: ${formatPrice(candle.open)}`,
                                        `H: ${formatPrice(candle.high)}`,
                                        `L: ${formatPrice(candle.low)}`,
                                        `C: ${formatPrice(candle.close)}`
                                    ];
                                }
                                return `${context.dataset.label}: ${formatPrice(context.raw)}`;
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
                            mode: 'xy',
                            limits: {
                                x: { min: 'original', max: 'original' },
                                y: { min: 'original', max: 'original' }
                            }
                        },
                        pan: {
                            enabled: true,
                            mode: 'xy',
                            modifierKey: null,
                            threshold: 0,
                            onPan: function({chart}) {
                                // Pan callback
                            },
                            onPanStart: function({chart, event}) {
                                const canvas = event.native?.target || chart.canvas;
                                if (canvas) {
                                    canvas.style.cursor = 'grabbing';
                                }
                            },
                            onPanComplete: function({chart}) {
                                const canvas = chart.canvas;
                                if (canvas) {
                                    canvas.style.cursor = 'grab';
                                }
                            }
                        },
                        limits: {
                            x: { min: 'original', max: 'original', minRange: null },
                            y: { min: 'original', max: 'original', minRange: null }
                        }
                    }
                },
                layout: {
                    padding: {
                        bottom: 50,
                        right: 10,
                        top: 10,
                        left: 10
                    }
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            displayFormats: {
                                minute: 'HH:mm',
                                hour: 'HH:mm',
                                day: 'MMM d',
                                week: 'MMM d',
                                month: 'MMM yyyy'
                            }
                        },
                        grid: {
                            color: colors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: colors.text,
                            maxRotation: 0,
                            padding: 8
                        }
                    },
                    y: {
                        position: 'right',
                        grid: {
                            color: colors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: colors.text,
                            padding: 8,
                            callback: function(value) {
                                return formatPrice(value);
                            }
                        }
                    }
                }
            }
        });

        // Store original scale ranges for reset functionality
        if (mainChart) {
            const xScale = mainChart.scales.x;
            const yScale = mainChart.scales.y;
            
            if (!xScale._originalRange && chartData && chartData.candles.length > 0) {
                const times = chartData.candles.map(c => c.time);
                const prices = chartData.candles.flatMap(c => [c.high, c.low]);
                
                xScale._originalRange = {
                    min: Math.min(...times),
                    max: Math.max(...times)
                };
                
                yScale._originalRange = {
                    min: Math.min(...prices) * 0.98,
                    max: Math.max(...prices) * 1.02
                };
            }

            // Ensure pan is enabled and add manual pan support
            const canvas = mainChart.canvas;
            if (canvas) {
                canvas.style.cursor = 'grab';
                
                let isPanning = false;
                let lastPanPoint = null;
                
                // Manual pan implementation as fallback
                canvas.addEventListener('mousedown', (e) => {
                    isPanning = true;
                    lastPanPoint = {
                        x: e.offsetX,
                        y: e.offsetY
                    };
                    canvas.style.cursor = 'grabbing';
                });
                
                canvas.addEventListener('mousemove', (e) => {
                    if (isPanning && lastPanPoint) {
                        const xScale = mainChart.scales.x;
                        const yScale = mainChart.scales.y;
                        
                        const deltaX = e.offsetX - lastPanPoint.x;
                        const deltaY = e.offsetY - lastPanPoint.y;
                        
                        // Calculate pan distance in data units
                        const xRange = xScale.max - xScale.min;
                        const yRange = yScale.max - yScale.min;
                        const canvasWidth = canvas.width;
                        const canvasHeight = canvas.height;
                        
                        const xDelta = -(deltaX / canvasWidth) * xRange;
                        const yDelta = (deltaY / canvasHeight) * yRange;
                        
                        // Apply pan
                        if (xScale.options.min !== undefined && xScale.options.max !== undefined) {
                            xScale.options.min += xDelta;
                            xScale.options.max += xDelta;
                        } else {
                            // Store original range if not set
                            if (!xScale._originalRange && chartData && chartData.candles.length > 0) {
                                const times = chartData.candles.map(c => c.time);
                                xScale._originalRange = {
                                    min: Math.min(...times),
                                    max: Math.max(...times)
                                };
                            }
                            if (xScale._originalRange) {
                                const currentRange = xScale.max - xScale.min;
                                xScale.options.min = xScale.min + xDelta;
                                xScale.options.max = xScale.max + xDelta;
                            }
                        }
                        
                        if (yScale.options.min !== undefined && yScale.options.max !== undefined) {
                            yScale.options.min += yDelta;
                            yScale.options.max += yDelta;
                        } else {
                            if (!yScale._originalRange && chartData && chartData.candles.length > 0) {
                                const prices = chartData.candles.flatMap(c => [c.high, c.low]);
                                yScale._originalRange = {
                                    min: Math.min(...prices) * 0.98,
                                    max: Math.max(...prices) * 1.02
                                };
                            }
                            if (yScale._originalRange) {
                                yScale.options.min = yScale.min + yDelta;
                                yScale.options.max = yScale.max + yDelta;
                            }
                        }
                        
                        mainChart.update('none');
                        
                        lastPanPoint = {
                            x: e.offsetX,
                            y: e.offsetY
                        };
                    }
                });
                
                canvas.addEventListener('mouseup', () => {
                    isPanning = false;
                    lastPanPoint = null;
                    canvas.style.cursor = 'grab';
                });
                
                canvas.addEventListener('mouseleave', () => {
                    isPanning = false;
                    lastPanPoint = null;
                    canvas.style.cursor = 'grab';
                });
            }
        }

        // Add custom candlestick rendering plugin if candlestick type is selected
        if (currentChartType === 'candlestick' && chartType === 'line') {
            // Store candles data for plugin access
            mainChart._candlesData = candles;
            mainChart._candlestickColors = colors;
            
            // Register custom plugin to draw candlesticks
            const candlestickPluginId = 'customCandlestickRenderer';
            
            // Unregister if already exists
            if (Chart.registry.getPlugin(candlestickPluginId)) {
                Chart.unregister(Chart.registry.getPlugin(candlestickPluginId));
            }
            
            const candlestickPlugin = {
                id: candlestickPluginId,
                afterDatasetsDraw: (chart) => {
                    if (!chart._candlesData) return;
                    
                    const ctx = chart.ctx;
                    const meta = chart.getDatasetMeta(0);
                    const yScale = chart.scales.y;
                    const candlesData = chart._candlesData;
                    const chartColors = chart._candlestickColors;
                    
                    candlesData.forEach((candle, index) => {
                        if (meta.data[index]) {
                            const x = meta.data[index].x;
                            const open = yScale.getPixelForValue(candle.open);
                            const high = yScale.getPixelForValue(candle.high);
                            const low = yScale.getPixelForValue(candle.low);
                            const close = yScale.getPixelForValue(candle.close);
                            
                            const isBullish = close <= open;
                            const color = isBullish ? chartColors.bullish : chartColors.bearish;
                            const bodyTop = Math.min(open, close);
                            const bodyBottom = Math.max(open, close);
                            const bodyHeight = Math.max(bodyBottom - bodyTop, 1);
                            const bodyWidth = 8;
                            
                            ctx.save();
                            
                            // Draw wick (high-low line)
                            ctx.strokeStyle = color;
                            ctx.lineWidth = 1;
                            ctx.beginPath();
                            ctx.moveTo(x, high);
                            ctx.lineTo(x, low);
                            ctx.stroke();
                            
                            // Draw body (open-close rectangle)
                            ctx.fillStyle = color;
                            ctx.fillRect(x - bodyWidth / 2, bodyTop, bodyWidth, bodyHeight);
                            ctx.strokeStyle = color;
                            ctx.lineWidth = 1;
                            ctx.strokeRect(x - bodyWidth / 2, bodyTop, bodyWidth, bodyHeight);
                            
                            ctx.restore();
                        }
                    });
                }
            };
            
            Chart.register(candlestickPlugin);
        }

        // Render volume chart
        if (activeIndicators.volume) {
            renderVolumeChart();
        }
    }

    function renderVolumeChart() {
        const ctx = document.getElementById('volume-chart').getContext('2d');
        
        if (volumeChart) {
            volumeChart.destroy();
        }

        const candles = chartData.candles;
        const labels = candles.map(c => new Date(c.time));
        const volumes = candles.map(c => c.volume);
        const volumeColors = candles.map((c, i) => 
            i > 0 && c.close >= candles[i-1].close 
                ? 'rgba(34, 197, 94, 0.6)' 
                : 'rgba(239, 68, 68, 0.6)'
        );

        volumeChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Volume',
                    data: volumes,
                    backgroundColor: volumeColors,
                    borderWidth: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        display: false
                    },
                    tooltip: {
                        callbacks: {
                            label: function(context) {
                                return 'Volume: ' + formatVolume(context.raw);
                            }
                        }
                    }
                },
                scales: {
                    x: {
                        display: false
                    },
                    y: {
                        position: 'right',
                        grid: {
                            color: colors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: colors.text,
                            callback: function(value) {
                                return formatVolume(value);
                            }
                        }
                    }
                }
            }
        });

        document.getElementById('volume-chart-container').style.display = 'block';
    }

    function padIndicatorData(values, targetLength) {
        const padding = targetLength - values.length;
        return [...Array(padding).fill(null), ...values];
    }

    function formatPrice(price) {
        if (price === null || price === undefined) return '--';
        return new Intl.NumberFormat('en-US', {
            style: 'currency',
            currency: 'USD',
            minimumFractionDigits: 2,
            maximumFractionDigits: 2
        }).format(price);
    }

    function formatVolume(volume) {
        if (volume === null || volume === undefined) return '--';
        if (volume >= 1000000000) {
            return (volume / 1000000000).toFixed(2) + 'B';
        } else if (volume >= 1000000) {
            return (volume / 1000000).toFixed(2) + 'M';
        } else if (volume >= 1000) {
            return (volume / 1000).toFixed(2) + 'K';
        }
        return volume.toLocaleString();
    }

    function showLoading(show) {
        const loading = document.getElementById('chart-loading');
        if (loading) {
            loading.style.display = show ? 'flex' : 'none';
        }
    }

    function hideEmpty() {
        const empty = document.getElementById('chart-empty');
        if (empty) {
            empty.style.display = 'none';
        }
    }

    function showError(message) {
        console.error(message);
    }

    // Watchlist functions
    async function loadWatchlist() {
        try {
            const response = await fetch('api/charts.php?action=watchlist');
            const data = await response.json();
            
            if (data.success && data.watchlist.length > 0) {
                // Could display watchlist somewhere if needed
            }
        } catch (error) {
            console.error('Error loading watchlist:', error);
        }
    }

    async function addToWatchlist() {
        if (!currentSymbol) {
            alert('Please load a chart first');
            return;
        }

        try {
            const response = await fetch('api/charts.php?action=watchlist', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ symbol: currentSymbol })
            });
            
            const data = await response.json();
            if (data.success) {
                showNotification('Added to watchlist', 'success');
            }
        } catch (error) {
            console.error('Error adding to watchlist:', error);
        }
    }

    // Study functions
    function openSaveStudyModal() {
        if (!currentSymbol || !chartData) {
            alert('Please load a chart first');
            return;
        }

        document.getElementById('preview-symbol').textContent = currentSymbol;
        document.getElementById('preview-timeframe').textContent = currentTimeframe;
        
        const activeIndicatorNames = Object.entries(activeIndicators)
            .filter(([k, v]) => v)
            .map(([k]) => k.toUpperCase())
            .join(', ');
        document.getElementById('preview-indicators').textContent = activeIndicatorNames || 'None';
        
        document.getElementById('save-study-modal').classList.add('active');
    }

    function closeSaveStudyModal() {
        document.getElementById('save-study-modal').classList.remove('active');
        document.getElementById('study-name-input').value = '';
        document.getElementById('study-notes-input').value = '';
    }

    async function saveStudy() {
        const name = document.getElementById('study-name-input').value.trim();
        const notes = document.getElementById('study-notes-input').value.trim();
        
        if (!name) {
            alert('Please enter a study name');
            return;
        }

        try {
            const response = await fetch('api/charts.php?action=study', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    name: name,
                    symbol: currentSymbol,
                    timeframe: currentTimeframe,
                    chartType: currentChartType,
                    indicators: activeIndicators,
                    notes: notes,
                    priceData: chartData?.statistics
                })
            });
            
            const data = await response.json();
            if (data.success) {
                showNotification('Study saved successfully', 'success');
                closeSaveStudyModal();
                loadStudiesDropdown();
            } else {
                alert('Failed to save study');
            }
        } catch (error) {
            console.error('Error saving study:', error);
            alert('Error saving study');
        }
    }

    // Studies Dropdown Functions
    async function toggleStudiesDropdown() {
        const dropdown = document.querySelector('.saved-studies-dropdown');
        const isOpen = dropdown.classList.contains('open');
        
        if (isOpen) {
            dropdown.classList.remove('open');
        } else {
            dropdown.classList.add('open');
            await loadStudiesDropdown();
        }
    }

    function closeStudiesDropdown() {
        const dropdown = document.querySelector('.saved-studies-dropdown');
        dropdown.classList.remove('open');
    }

    async function loadStudiesDropdown() {
        try {
            const response = await fetch('api/charts.php?action=studies');
            const data = await response.json();
            
            const contentEl = document.getElementById('studies-panel-content');
            const emptyEl = document.getElementById('studies-panel-empty');
            
            if (data.success && data.studies.length > 0) {
                emptyEl.style.display = 'none';
                contentEl.style.display = 'block';
                contentEl.innerHTML = data.studies.map(study => `
                    <div class="study-dropdown-item" data-id="${study.id}">
                        <div class="study-dropdown-info">
                            <div class="study-dropdown-name">${study.name}</div>
                            <div class="study-dropdown-meta">
                                <span class="study-symbol">${study.symbol}</span>
                                <span>${study.timeframe}</span>
                                <span>${formatDate(study.created_at)}</span>
                            </div>
                        </div>
                        <div class="study-dropdown-actions">
                            <button class="btn btn-primary btn-sm load-dropdown-study" data-id="${study.id}" title="Load Study">
                                <i class="fas fa-chart-line"></i>
                            </button>
                            <button class="btn btn-danger btn-sm delete-dropdown-study" data-id="${study.id}" title="Delete Study">
                                <i class="fas fa-trash"></i>
                            </button>
                        </div>
                    </div>
                `).join('');
                
                // Add event listeners for load buttons
                contentEl.querySelectorAll('.load-dropdown-study').forEach(btn => {
                    btn.addEventListener('click', (e) => {
                        e.stopPropagation();
                        loadStudyById(btn.dataset.id);
                        closeStudiesDropdown();
                    });
                });
                
                // Add event listeners for delete buttons
                contentEl.querySelectorAll('.delete-dropdown-study').forEach(btn => {
                    btn.addEventListener('click', (e) => {
                        e.stopPropagation();
                        deleteStudy(btn.dataset.id);
                    });
                });
                
                // Click on item to load
                contentEl.querySelectorAll('.study-dropdown-item').forEach(item => {
                    item.addEventListener('click', () => {
                        loadStudyById(item.dataset.id);
                        closeStudiesDropdown();
                    });
                });
            } else {
                contentEl.style.display = 'none';
                emptyEl.style.display = 'flex';
            }
        } catch (error) {
            console.error('Error loading studies:', error);
        }
    }

    async function openLoadStudyModal() {
        document.getElementById('load-study-modal').classList.add('active');
        await loadStudiesForModal();
    }

    function closeLoadStudyModal() {
        document.getElementById('load-study-modal').classList.remove('active');
    }

    async function loadStudiesForModal() {
        try {
            const response = await fetch('api/charts.php?action=studies');
            const data = await response.json();
            
            const listEl = document.getElementById('saved-studies-list');
            const emptyEl = document.getElementById('empty-studies');
            
            if (data.success && data.studies.length > 0) {
                emptyEl.style.display = 'none';
                listEl.innerHTML = data.studies.map(study => `
                    <div class="study-item" data-id="${study.id}">
                        <div class="study-info">
                            <h4 class="study-name">${study.name}</h4>
                            <div class="study-meta">
                                <span class="study-symbol">${study.symbol}</span>
                                <span class="study-timeframe">${study.timeframe}</span>
                                <span class="study-date">${formatDate(study.created_at)}</span>
                            </div>
                            ${study.notes ? `<p class="study-notes">${study.notes}</p>` : ''}
                        </div>
                        <div class="study-actions">
                            <button class="btn btn-primary btn-sm load-study-item-btn" data-id="${study.id}">Load</button>
                            <button class="btn btn-danger btn-sm delete-study-btn" data-id="${study.id}">Delete</button>
                        </div>
                    </div>
                `).join('');
                
                // Add event listeners
                listEl.querySelectorAll('.load-study-item-btn').forEach(btn => {
                    btn.addEventListener('click', () => loadStudyById(btn.dataset.id));
                });
                
                listEl.querySelectorAll('.delete-study-btn').forEach(btn => {
                    btn.addEventListener('click', () => deleteStudy(btn.dataset.id));
                });
            } else {
                emptyEl.style.display = 'flex';
                listEl.innerHTML = '';
            }
        } catch (error) {
            console.error('Error loading studies:', error);
        }
    }

    async function loadStudyById(id) {
        try {
            const response = await fetch(`api/charts.php?action=study&id=${id}`);
            const data = await response.json();
            
            if (data.success && data.study) {
                const study = data.study;
                
                // Set timeframe
                currentTimeframe = study.timeframe;
                document.querySelectorAll('.timeframe-btn').forEach(btn => {
                    btn.classList.toggle('active', btn.dataset.timeframe === study.timeframe);
                });
                
                // Set chart type
                currentChartType = study.chart_type;
                document.querySelectorAll('.chart-type-btn').forEach(btn => {
                    btn.classList.toggle('active', btn.dataset.type === study.chart_type);
                });
                
                // Set indicators
                if (study.indicators) {
                    Object.entries(study.indicators).forEach(([key, value]) => {
                        activeIndicators[key] = value;
                        const checkbox = document.querySelector(`[data-indicator="${key}"]`);
                        if (checkbox) checkbox.checked = value;
                    });
                }
                
                // Load chart
                document.getElementById('chart-symbol-input').value = study.symbol;
                await loadChart(study.symbol);
                
                closeLoadStudyModal();
                showNotification(`Loaded study: ${study.name}`, 'success');
            }
        } catch (error) {
            console.error('Error loading study:', error);
        }
    }

    async function deleteStudy(id) {
        if (!confirm('Are you sure you want to delete this study?')) return;
        
        try {
            const response = await fetch(`api/charts.php?action=study&id=${id}`, {
                method: 'DELETE'
            });
            
            const data = await response.json();
            if (data.success) {
                showNotification('Study deleted', 'success');
                loadStudiesForModal();
                loadStudiesDropdown();
            }
        } catch (error) {
            console.error('Error deleting study:', error);
        }
    }

    async function loadSavedStudies() {
        try {
            const response = await fetch('api/charts.php?action=studies');
            const data = await response.json();
            
            const gridEl = document.getElementById('studies-grid');
            if (!gridEl) return;
            
            if (data.success && data.studies.length > 0) {
                gridEl.innerHTML = data.studies.slice(0, 6).map(study => `
                    <div class="study-card" data-id="${study.id}">
                        <div class="study-card-header">
                            <span class="study-symbol">${study.symbol}</span>
                            <span class="study-timeframe">${study.timeframe}</span>
                        </div>
                        <h4 class="study-name">${study.name}</h4>
                        <p class="study-date">${formatDate(study.created_at)}</p>
                        <button class="btn btn-secondary btn-sm" onclick="ChartsModule.loadStudyById('${study.id}')">
                            Load Study
                        </button>
                    </div>
                `).join('');
            } else {
                gridEl.innerHTML = '<p class="no-studies">No saved studies yet. Save your first study to see it here.</p>';
            }
        } catch (error) {
            console.error('Error loading saved studies:', error);
        }
    }

    function createNoteFromChart() {
        if (!currentSymbol) {
            alert('Please load a chart first');
            return;
        }

        // Navigate to notes page and create a new note
        const noteContent = `Chart Analysis: ${currentSymbol}\n` +
            `Timeframe: ${currentTimeframe}\n` +
            `Price: ${chartData?.statistics?.close || '--'}\n` +
            `RSI: ${chartData?.indicators?.rsi?.value || '--'} (${chartData?.indicators?.rsi?.signal || '--'})\n` +
            `MACD: ${chartData?.indicators?.macd?.signal || '--'}\n\n` +
            `Notes: `;
        
        // Store in session storage for notes page to pick up
        sessionStorage.setItem('newNoteContent', noteContent);
        sessionStorage.setItem('newNoteTitle', `${currentSymbol} Analysis - ${new Date().toLocaleDateString()}`);
        
        // Navigate to notes page
        document.querySelector('[data-page="notes"]')?.click();
    }

    function exportChart() {
        if (!mainChart) {
            alert('Please load a chart first');
            return;
        }

        const link = document.createElement('a');
        link.download = `${currentSymbol}_${currentTimeframe}_${new Date().toISOString().split('T')[0]}.png`;
        link.href = mainChart.toBase64Image();
        link.click();
    }

    function formatDate(dateStr) {
        return new Date(dateStr).toLocaleDateString('en-US', {
            month: 'short',
            day: 'numeric',
            year: 'numeric'
        });
    }

    function showNotification(message, type = 'info') {
        // Simple notification
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.textContent = message;
        notification.style.cssText = `
            position: fixed;
            top: 80px;
            right: 20px;
            padding: 12px 20px;
            background: ${type === 'success' ? '#22c55e' : type === 'error' ? '#ef4444' : '#3b82f6'};
            color: white;
            border-radius: 8px;
            z-index: 10000;
            animation: slideIn 0.3s ease;
        `;
        document.body.appendChild(notification);
        
        setTimeout(() => {
            notification.remove();
        }, 3000);
    }

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        // DOM already loaded
        init();
    }
    
    // Also check on window load in case page was already active
    window.addEventListener('load', () => {
        setTimeout(() => {
            checkAndLoadDefaultChart();
        }, 200);
    });

    // Public API
    return {
        loadChart,
        loadStudyById,
        init
    };
})();

