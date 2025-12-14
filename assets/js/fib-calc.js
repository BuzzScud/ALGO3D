// Fibonacci Calculator Module
const FibCalcModule = (function() {
    // State
    let precision = 2;
    let currentData = null;
    let fibChart = null;
    let currentChartType = 'line'; // 'line' or 'candlestick'
    
    // Key Fibonacci levels (highlighted in the display)
    const KEY_POSITIVE_LEVELS = [0, 1, 1.618, 2.618, 4.24];
    const KEY_NEGATIVE_LEVELS = [-1, -1.618, -2.618, -4.24];
    
    // DOM Elements
    let elements = {};
    
    // Pan state for custom pan implementation
    let fibPanState = {
        isPanning: false,
        lastX: 0,
        lastY: 0,
        boundHandlers: null
    };
    
    function init() {
        // Cache DOM elements
        elements = {
            symbolInput: document.getElementById('fib-symbol'),
            calculateBtn: document.getElementById('fib-calculate-btn'),
            precisionToggle: document.getElementById('fib-precision-toggle'),
            marketData: document.getElementById('fib-market-data'),
            levelsContainer: document.getElementById('fib-levels-container'),
            symbolName: document.getElementById('fib-symbol-name'),
            anchorInfo: document.getElementById('fib-anchor-info'),
            currentPrice: document.getElementById('fib-current-price'),
            ytdHigh: document.getElementById('fib-ytd-high'),
            ytdLow: document.getElementById('fib-ytd-low'),
            range: document.getElementById('fib-range'),
            positiveLevels: document.getElementById('fib-positive-levels'),
            negativeLevels: document.getElementById('fib-negative-levels'),
            loading: document.getElementById('fib-loading'),
            error: document.getElementById('fib-error'),
            chartCanvas: document.getElementById('fib-chart'),
            chartLoading: document.getElementById('fib-chart-loading'),
            chartEmpty: document.getElementById('fib-chart-empty')
        };
        
        if (!elements.calculateBtn) {
            console.log('Fibonacci Calculator: Elements not found, skipping initialization');
            return;
        }
        
        setupEventListeners();
        setupChartTypeToggle();
        initChart();
        console.log('Fibonacci Calculator initialized');
    }
    
    function setupChartTypeToggle() {
        const lineBtn = document.getElementById('fib-chart-type-line');
        const candlestickBtn = document.getElementById('fib-chart-type-candlestick');
        
        if (lineBtn) {
            lineBtn.addEventListener('click', () => {
                if (currentChartType === 'line') return;
                currentChartType = 'line';
                lineBtn.classList.add('active');
                candlestickBtn?.classList.remove('active');
                if (currentData) {
                    renderChart(currentData);
                }
            });
        }
        
        if (candlestickBtn) {
            candlestickBtn.addEventListener('click', () => {
                if (currentChartType === 'candlestick') return;
                currentChartType = 'candlestick';
                candlestickBtn.classList.add('active');
                lineBtn?.classList.remove('active');
                if (currentData) {
                    renderChart(currentData);
                }
            });
        }
    }
    
    function setupEventListeners() {
        // Calculate button
        if (elements.calculateBtn) {
            elements.calculateBtn.addEventListener('click', calculate);
        }
        
        // Enter key on symbol input
        if (elements.symbolInput) {
            elements.symbolInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    calculate();
                }
            });
        }
        
        // Precision toggle buttons
        if (elements.precisionToggle) {
            const buttons = elements.precisionToggle.querySelectorAll('.fib-toggle-btn');
            buttons.forEach(btn => {
                btn.addEventListener('click', function() {
                    buttons.forEach(b => b.classList.remove('active'));
                    this.classList.add('active');
                    precision = parseInt(this.dataset.value);
                    
                    // Recalculate if we have data
                    if (currentData) {
                        displayResults(currentData);
                        // Update chart if it exists
                        if (fibChart && currentData.candles) {
                            renderChart(currentData);
                        }
                    }
                });
            });
        }
    }
    
    async function calculate() {
        const symbol = elements.symbolInput?.value.trim().toUpperCase();
        
        if (!symbol) {
            showError('Please enter a valid symbol');
            return;
        }
        
        showLoading();
        hideError();
        
        try {
            const response = await fetch(`api/fib_calc.php?action=calculate&symbol=${symbol}&precision=${precision}`);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const data = await response.json();
            
            if (data.success) {
                currentData = data;
                displayResults(data);
            } else {
                showError(data.message || 'Failed to fetch data');
            }
        } catch (error) {
            console.error('Fibonacci calculation error:', error);
            showError(`Error: ${error.message}`);
        } finally {
            hideLoading();
        }
    }
    
    function displayResults(data) {
        // Show panels
        if (elements.marketData) elements.marketData.style.display = 'block';
        if (elements.levelsContainer) elements.levelsContainer.style.display = 'block';
        
        // Update market data
        if (elements.symbolName) {
            elements.symbolName.textContent = `${data.symbol} - YTD Analysis`;
        }
        
        if (elements.anchorInfo) {
            const fc = data.firstCandle;
            elements.anchorInfo.innerHTML = `
                <strong>Anchor:</strong> First Trading Day (${fc.date}) | 
                <strong>${fc.type}</strong> candle | 
                O: ${fc.open} H: ${fc.high} L: ${fc.low} C: ${fc.close} | 
                <strong>${data.fibonacci.anchorNote}</strong>
            `;
        }
        
        if (elements.currentPrice) {
            elements.currentPrice.textContent = `$${data.currentPrice.toFixed(precision)}`;
        }
        
        if (elements.ytdHigh) {
            elements.ytdHigh.textContent = `$${data.periodHigh.toFixed(precision)}`;
        }
        
        if (elements.ytdLow) {
            elements.ytdLow.textContent = `$${data.periodLow.toFixed(precision)}`;
        }
        
        if (elements.range) {
            elements.range.textContent = `$${data.range.toFixed(precision)} (${data.rangePercent}%)`;
        }
        
        // Display positive levels
        if (elements.positiveLevels) {
            elements.positiveLevels.innerHTML = data.positiveLevels.map(level => {
                const isKey = KEY_POSITIVE_LEVELS.includes(level.ratio);
                return createLevelItem(level, isKey);
            }).join('');
        }
        
        // Display negative levels
        if (elements.negativeLevels) {
            elements.negativeLevels.innerHTML = data.negativeLevels.map(level => {
                const isKey = KEY_NEGATIVE_LEVELS.includes(level.ratio);
                return createLevelItem(level, isKey);
            }).join('');
        }
        
        // Render chart with Fibonacci levels
        renderChart(data);
    }
    
    function createLevelItem(level, isKey) {
        return `
            <div class="fib-level-item" data-key="${isKey}">
                <span class="fib-level-label">${level.label}</span>
                <span class="fib-level-value">$${level.price.toFixed(precision)}</span>
            </div>
        `;
    }
    
    function initChart() {
        if (!elements.chartCanvas) return;
        
        // Wait for Chart.js to be available
        if (typeof Chart === 'undefined') {
            console.warn('Chart.js not loaded yet, retrying...');
            setTimeout(initChart, 100);
            return;
        }
        
        // Register candlestick controller if available
        // The financial plugin from CDN should auto-register, but we verify it's available
        try {
            // Check if candlestick controller is already registered
            const candlestickController = Chart.registry.getController('candlestick');
            if (!candlestickController) {
                // Try to register from various possible locations (for manual registration)
                if (typeof CandlestickController !== 'undefined' && typeof CandlestickElement !== 'undefined') {
                    Chart.register(CandlestickController, CandlestickElement);
                } else if (window.CandlestickController && window.CandlestickElement) {
                    Chart.register(window.CandlestickController, window.CandlestickElement);
                } else {
                    console.warn('Candlestick controller not found. The chartjs-chart-financial plugin should auto-register it.');
                }
            }
        } catch (e) {
            console.warn('Candlestick controller registration issue:', e);
        }
        
        // Register zoom plugin - ensure it's available
        // The CDN script should auto-register, but we verify and register if needed
        try {
            if (typeof Chart !== 'undefined') {
                // Check if plugin is already registered
                let zoomPluginRegistered = false;
                try {
                    zoomPluginRegistered = Chart.registry.getPlugin('zoom') !== undefined;
                } catch (e) {
                    // Registry might not be ready
                }
                
                if (!zoomPluginRegistered) {
                    // Try to register from various possible locations
                    if (typeof zoomPlugin !== 'undefined') {
                        Chart.register(zoomPlugin);
                        console.log('Zoom plugin registered from zoomPlugin');
                    } else if (window.zoomPlugin) {
                        Chart.register(window.zoomPlugin);
                        console.log('Zoom plugin registered from window.zoomPlugin');
                    } else if (typeof ChartZoom !== 'undefined') {
                        // Some CDN versions export as ChartZoom
                        Chart.register(ChartZoom);
                        console.log('Zoom plugin registered from ChartZoom');
                    } else {
                        // Plugin should be auto-registered by CDN
                        // Will check again after chart creation
                        console.log('Zoom plugin should be auto-registered by CDN script');
                    }
                } else {
                    console.log('Zoom plugin already registered');
                }
            }
        } catch (e) {
            console.warn('Zoom plugin registration issue:', e);
        }
        
        const ctx = elements.chartCanvas.getContext('2d');
        
        // Destroy existing chart if it exists
        if (fibChart) {
            fibChart.destroy();
        }
        
        // Always use 'line' chart type to support mixed datasets (candlestick + line for Fibonacci levels)
        // The candlestick dataset will use type: 'candlestick' within the dataset itself
        fibChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: []
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    mode: 'index',
                    intersect: false,
                    includeInvisible: true
                },
                onHover: (event, activeElements) => {
                    // Don't change cursor if currently panning
                    if (fibPanState.isPanning) return;
                    
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
                        labels: {
                            color: '#e2e8f0',
                            usePointStyle: true,
                            padding: 15,
                            font: {
                                size: 12,
                                weight: '500',
                                family: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
                            },
                            filter: function(item, chart) {
                                // Show all by default
                                return true;
                            },
                            boxWidth: 12,
                            boxHeight: 12,
                            generateLabels: function(chart) {
                                const original = Chart.defaults.plugins.legend.labels.generateLabels;
                                const labels = original.call(this, chart);
                                // Enhance labels with better styling
                                labels.forEach(label => {
                                    label.fontColor = label.strokeStyle || '#e2e8f0';
                                });
                                return labels;
                            }
                        },
                        onClick: function(e, legendItem) {
                            const index = legendItem.datasetIndex;
                            const chart = legendItem.chart;
                            const meta = chart.getDatasetMeta(index);
                            
                            meta.hidden = meta.hidden === null ? !chart.data.datasets[index].hidden : null;
                            chart.update('active');
                        }
                    },
                    tooltip: {
                        enabled: true,
                        mode: 'index',
                        intersect: false,
                        backgroundColor: 'rgba(15, 23, 42, 0.98)',
                        titleColor: '#f1f5f9',
                        bodyColor: '#cbd5e1',
                        borderColor: 'rgba(59, 130, 246, 0.3)',
                        borderWidth: 2,
                        padding: 16,
                        displayColors: true,
                        titleFont: {
                            size: 13,
                            weight: '600',
                            family: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
                        },
                        bodyFont: {
                            size: 12,
                            weight: '500',
                            family: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
                        },
                        cornerRadius: 8,
                        boxPadding: 8,
                        usePointStyle: true,
                        callbacks: {
                            title: function(context) {
                                if (context.length > 0) {
                                    const date = new Date(context[0].parsed.x);
                                    return date.toLocaleDateString('en-US', { 
                                        weekday: 'short',
                                        month: 'short', 
                                        day: 'numeric', 
                                        year: 'numeric',
                                        hour: '2-digit',
                                        minute: '2-digit'
                                    });
                                }
                                return '';
                            },
                            label: function(context) {
                                const label = context.dataset.label || '';
                                
                                // Handle candlestick data
                                if (context.dataset.type === 'candlestick' && context.raw) {
                                    const raw = context.raw;
                                    return [
                                        `Open: $${raw.o.toFixed(precision)}`,
                                        `High: $${raw.h.toFixed(precision)}`,
                                        `Low: $${raw.l.toFixed(precision)}`,
                                        `Close: $${raw.c.toFixed(precision)}`
                                    ];
                                }
                                
                                // Handle line chart data
                                const value = context.parsed.y;
                                if (value !== undefined && value !== null) {
                                    return `${label}: $${value.toFixed(precision)}`;
                                }
                                return label;
                            },
                            labelColor: function(context) {
                                return {
                                    borderColor: context.dataset.borderColor || context.dataset.color?.up || '#3b82f6',
                                    backgroundColor: context.dataset.borderColor || context.dataset.color?.up || '#3b82f6',
                                    borderWidth: 3,
                                    borderRadius: 2
                                };
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
                scales: {
                    x: {
                        type: 'time',
                        min: undefined,
                        max: undefined,
                        time: {
                            unit: 'day',
                            displayFormats: {
                                day: 'MMM dd'
                            }
                        },
                        grid: {
                            color: 'rgba(148, 163, 184, 0.15)',
                            drawBorder: true,
                            borderColor: 'rgba(148, 163, 184, 0.2)',
                            borderWidth: 1,
                            lineWidth: 1,
                            drawOnChartArea: true,
                            drawTicks: true
                        },
                        ticks: {
                            color: '#94a3b8',
                            font: {
                                size: 11,
                                weight: '500',
                                family: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
                            },
                            padding: 8,
                            maxRotation: 0,
                            autoSkip: true,
                            maxTicksLimit: 8
                        }
                    },
                    y: {
                        grid: {
                            color: 'rgba(148, 163, 184, 0.15)',
                            drawBorder: true,
                            borderColor: 'rgba(148, 163, 184, 0.2)',
                            borderWidth: 1,
                            lineWidth: 1,
                            drawOnChartArea: true,
                            drawTicks: true
                        },
                        ticks: {
                            color: '#94a3b8',
                            font: {
                                size: 11,
                                weight: '500',
                                family: "'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"
                            },
                            padding: 8,
                            callback: function(value) {
                                return '$' + value.toFixed(precision);
                            }
                        }
                    }
                }
            }
        });
        
        // Set initial cursor and ensure canvas is interactive
        if (elements.chartCanvas) {
            // Set canvas styles for interactivity - critical for pan to work
            elements.chartCanvas.style.cursor = 'grab';
            elements.chartCanvas.style.pointerEvents = 'auto';
            elements.chartCanvas.style.touchAction = 'pan-x pan-y pinch-zoom';
            elements.chartCanvas.style.userSelect = 'none';
            elements.chartCanvas.style.webkitUserSelect = 'none';
            elements.chartCanvas.style.mozUserSelect = 'none';
            elements.chartCanvas.style.msUserSelect = 'none';
            elements.chartCanvas.style.webkitTouchCallout = 'none';
            elements.chartCanvas.style.webkitTapHighlightColor = 'transparent';
            
            // Prevent text selection during drag (but don't block mouse events)
            const preventSelection = (e) => {
                e.preventDefault();
                return false;
            };
            
            // Remove old listeners if any
            elements.chartCanvas.removeEventListener('selectstart', preventSelection);
            elements.chartCanvas.removeEventListener('dragstart', preventSelection);
            elements.chartCanvas.removeEventListener('contextmenu', preventSelection);
            
            // Add event listeners with passive: false to allow preventDefault
            elements.chartCanvas.addEventListener('selectstart', preventSelection, { passive: false });
            elements.chartCanvas.addEventListener('dragstart', preventSelection, { passive: false });
            elements.chartCanvas.addEventListener('contextmenu', preventSelection, { passive: false });
            
            // Ensure canvas is focusable for keyboard events
            elements.chartCanvas.setAttribute('tabindex', '0');
        }
        
        setupChartControls();
        
        // Setup custom pan functionality (more reliable than zoom plugin pan)
        setupFibChartPan(fibChart);
        
        // Verify zoom plugin is available for scroll zoom
        setTimeout(() => {
            if (fibChart) {
                const zoomPlugin = Chart.registry.getPlugin('zoom');
                if (zoomPlugin) {
                    console.log('✓ Zoom plugin is registered (scroll zoom enabled)');
                } else {
                    console.warn('Zoom plugin not found - scroll zoom may not work');
                }
                console.log('✓ Custom pan implementation is active');
            }
        }, 500);
    }
    
    function addCandlestickRenderingPlugin(candles) {
        const pluginId = 'fibCandlestickRenderer';
        
        // Remove existing plugin if it exists
        try {
            const existingPlugin = Chart.registry.getPlugin(pluginId);
            if (existingPlugin) {
                Chart.unregister(existingPlugin);
            }
        } catch (e) {
            // Plugin doesn't exist, that's fine
        }
        
        // Store candles data and colors on chart
        if (fibChart) {
            fibChart._candlesData = candles;
            fibChart._candlestickColors = {
                bullish: '#22c55e',
                bearish: '#ef4444'
            };
        }
        
        const candlestickPlugin = {
            id: pluginId,
            beforeEvent: (chart, args) => {
                // Don't interfere with zoom/pan events - return true to allow event to continue
                return true;
            },
            afterDatasetsDraw: (chart) => {
                // Check if we should render candlesticks
                if (!chart._candlesData || currentChartType !== 'candlestick' || !chart._candlesData.length) {
                    return;
                }
                
                const ctx = chart.ctx;
                const datasets = chart.data.datasets;
                
                // Find the price dataset (first dataset, which is the placeholder)
                let priceDatasetIndex = 0;
                for (let i = 0; i < datasets.length; i++) {
                    if (datasets[i].label && datasets[i].label.includes('Price') && !datasets[i].label.includes('Fib')) {
                        priceDatasetIndex = i;
                        break;
                    }
                }
                
                const meta = chart.getDatasetMeta(priceDatasetIndex);
                if (!meta || !meta.data) return;
                
                const yScale = chart.scales.y;
                const xScale = chart.scales.x;
                const candlesData = chart._candlesData;
                const chartColors = chart._candlestickColors || {
                    bullish: '#22c55e',
                    bearish: '#ef4444'
                };
                
                if (!candlesData || candlesData.length === 0) return;
                
                // Get visible time range from x scale
                const xMin = xScale.min;
                const xMax = xScale.max;
                
                // Calculate body width based on available space
                const visibleCandles = candlesData.filter(c => c.time >= xMin && c.time <= xMax);
                const bodyWidth = visibleCandles.length > 0 
                    ? Math.max(4, Math.min(8, (xScale.width / visibleCandles.length) * 0.7))
                    : 6;
                
                candlesData.forEach((candle, index) => {
                    // Skip if outside visible time range
                    if (candle.time < xMin || candle.time > xMax) return;
                    
                    if (meta.data && meta.data[index]) {
                        const point = meta.data[index];
                        const x = point.x;
                        
                        // Skip if x is outside canvas bounds
                        if (x < 0 || x > chart.width) return;
                        
                        const open = yScale.getPixelForValue(candle.open);
                        const high = yScale.getPixelForValue(candle.high);
                        const low = yScale.getPixelForValue(candle.low);
                        const close = yScale.getPixelForValue(candle.close);
                        
                        // Determine if bullish (close > open) or bearish (close < open)
                        const isBullish = close < open; // In pixel coordinates, lower y = higher price
                        const color = isBullish ? chartColors.bullish : chartColors.bearish;
                        const bodyTop = Math.min(open, close);
                        const bodyBottom = Math.max(open, close);
                        const bodyHeight = Math.max(Math.abs(bodyBottom - bodyTop), 1);
                        
                        ctx.save();
                        
                        // Draw wick (high-low line)
                        ctx.strokeStyle = color;
                        ctx.lineWidth = 1.5;
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
    
    function removeCandlestickRenderingPlugin() {
        const pluginId = 'fibCandlestickRenderer';
        try {
            const existingPlugin = Chart.registry.getPlugin(pluginId);
            if (existingPlugin) {
                Chart.unregister(existingPlugin);
            }
        } catch (e) {
            // Plugin doesn't exist, that's fine
        }
    }
    
    // Setup manual pan functionality for the fib chart
    function setupFibChartPan(chart) {
        if (!chart || !chart.canvas) return;
        
        const canvas = chart.canvas;
        
        // Remove existing handlers if any
        if (fibPanState.boundHandlers) {
            canvas.removeEventListener('mousedown', fibPanState.boundHandlers.mousedown);
            document.removeEventListener('mousemove', fibPanState.boundHandlers.mousemove);
            document.removeEventListener('mouseup', fibPanState.boundHandlers.mouseup);
        }
        
        // Ensure canvas has proper styles for interaction
        canvas.style.cursor = 'grab';
        canvas.style.touchAction = 'none';
        canvas.style.userSelect = 'none';
        
        // Create bound handlers
        const handleMouseDown = (e) => {
            // Only respond to left mouse button
            if (e.button !== 0) return;
            
            fibPanState.isPanning = true;
            fibPanState.lastX = e.clientX;
            fibPanState.lastY = e.clientY;
            
            canvas.style.cursor = 'grabbing';
            
            // Prevent text selection during drag
            e.preventDefault();
            e.stopPropagation();
        };
        
        const handleMouseMove = (e) => {
            if (!fibPanState.isPanning) return;
            
            const xScale = chart.scales.x;
            const yScale = chart.scales.y;
            
            if (!xScale || !yScale) return;
            
            const deltaX = e.clientX - fibPanState.lastX;
            const deltaY = e.clientY - fibPanState.lastY;
            
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
            
            fibPanState.lastX = e.clientX;
            fibPanState.lastY = e.clientY;
            
            e.preventDefault();
        };
        
        const handleMouseUp = () => {
            if (fibPanState.isPanning) {
                fibPanState.isPanning = false;
                canvas.style.cursor = 'grab';
            }
        };
        
        // Store handlers for cleanup
        fibPanState.boundHandlers = {
            mousedown: handleMouseDown,
            mousemove: handleMouseMove,
            mouseup: handleMouseUp
        };
        
        // Add event listeners - use document for move/up to handle dragging outside canvas
        canvas.addEventListener('mousedown', handleMouseDown);
        document.addEventListener('mousemove', handleMouseMove);
        document.addEventListener('mouseup', handleMouseUp);
        
        // Prevent context menu on right-click
        canvas.addEventListener('contextmenu', (e) => e.preventDefault());
        
        // Touch support for mobile devices
        let touchState = { isPanning: false, lastX: 0, lastY: 0 };
        
        const handleTouchStart = (e) => {
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
        
        canvas.addEventListener('touchstart', handleTouchStart, { passive: false });
        canvas.addEventListener('touchmove', handleTouchMove, { passive: false });
        canvas.addEventListener('touchend', handleTouchEnd);
        canvas.addEventListener('touchcancel', handleTouchEnd);
        
        console.log('Custom pan handlers set up for fib chart');
    }
    
    function setupChartControls() {
        // Zoom In
        const zoomInBtn = document.getElementById('fib-zoom-in');
        if (zoomInBtn) {
            zoomInBtn.addEventListener('click', () => {
                if (!fibChart) return;
                const xScale = fibChart.scales.x;
                const yScale = fibChart.scales.y;
                const centerX = (xScale.max + xScale.min) / 2;
                const centerY = (yScale.max + yScale.min) / 2;
                const factor = 0.75;
                
                const xRange = xScale.max - xScale.min;
                const yRange = yScale.max - yScale.min;
                
                xScale.options.min = centerX - (xRange * factor) / 2;
                xScale.options.max = centerX + (xRange * factor) / 2;
                yScale.options.min = centerY - (yRange * factor) / 2;
                yScale.options.max = centerY + (yRange * factor) / 2;
                
                fibChart.update('none');
            });
        }
        
        // Zoom Out
        const zoomOutBtn = document.getElementById('fib-zoom-out');
        if (zoomOutBtn) {
            zoomOutBtn.addEventListener('click', () => {
                if (!fibChart) return;
                const xScale = fibChart.scales.x;
                const yScale = fibChart.scales.y;
                const centerX = (xScale.max + xScale.min) / 2;
                const centerY = (yScale.max + yScale.min) / 2;
                const factor = 1.33;
                
                const xRange = xScale.max - xScale.min;
                const yRange = yScale.max - yScale.min;
                
                xScale.options.min = centerX - (xRange * factor) / 2;
                xScale.options.max = centerX + (xRange * factor) / 2;
                yScale.options.min = centerY - (yRange * factor) / 2;
                yScale.options.max = centerY + (yRange * factor) / 2;
                
                fibChart.update('none');
            });
        }
        
        // Reset Zoom
        const zoomResetBtn = document.getElementById('fib-zoom-reset');
        if (zoomResetBtn) {
            zoomResetBtn.addEventListener('click', () => {
                if (!fibChart) return;
                const xScale = fibChart.scales.x;
                const yScale = fibChart.scales.y;
                
                delete xScale.options.min;
                delete xScale.options.max;
                delete yScale.options.min;
                delete yScale.options.max;
                
                fibChart.update('none');
            });
        }
        
        // Toggle Legend
        const toggleLegendBtn = document.getElementById('fib-toggle-legend');
        if (toggleLegendBtn) {
            toggleLegendBtn.addEventListener('click', () => {
                if (!fibChart) return;
                const legend = fibChart.options.plugins.legend;
                legend.display = !legend.display;
                fibChart.update('none');
                
                // Update button icon
                const icon = toggleLegendBtn.querySelector('i');
                if (icon) {
                    icon.className = legend.display ? 'fas fa-list' : 'fas fa-list-ul';
                }
            });
        }
    }
    
    function renderChart(data) {
        if (!fibChart || !data.candles || data.candles.length === 0) return;
        
        // Hide empty state, show chart
        if (elements.chartEmpty) elements.chartEmpty.style.display = 'none';
        if (elements.chartLoading) elements.chartLoading.style.display = 'none';
        
        // Create datasets based on chart type
        const datasets = [];
        
        // Prepare time data for Fibonacci levels (needed for both chart types)
        const times = data.candles.map(c => c.time * 1000);
        const minTime = Math.min(...times);
        const maxTime = Math.max(...times);
        
        if (currentChartType === 'candlestick') {
            // Check if candlestick controller is available
            let candlestickController;
            try {
                candlestickController = Chart.registry.getController('candlestick');
            } catch (e) {
                candlestickController = null;
            }
            
            // For candlestick rendering, we'll use a line dataset as placeholder
            // and render candlesticks via custom plugin
            // Store the OHLC data for the plugin
            const candlestickData = data.candles.map(c => {
                const open = typeof c.open === 'number' ? c.open : parseFloat(c.open);
                const high = typeof c.high === 'number' ? c.high : parseFloat(c.high);
                const low = typeof c.low === 'number' ? c.low : parseFloat(c.low);
                const close = typeof c.close === 'number' ? c.close : parseFloat(c.close);
                
                // Validate data
                if (isNaN(open) || isNaN(high) || isNaN(low) || isNaN(close)) {
                    console.warn('Invalid candlestick data:', c);
                    return null;
                }
                
                return {
                    time: c.time * 1000,
                    open: open,
                    high: high,
                    low: low,
                    close: close
                };
            }).filter(item => item !== null);
            
            if (candlestickData.length === 0) {
                console.warn('No valid candlestick data, falling back to line chart');
                // Fallback to line
                const priceData = data.candles.map(c => ({
                    x: c.time * 1000,
                    y: typeof c.close === 'number' ? c.close : parseFloat(c.close)
                }));
                
                datasets.push({
                    label: `${data.symbol} Price`,
                    data: priceData,
                    borderColor: '#3b82f6',
                    backgroundColor: 'rgba(59, 130, 246, 0.1)',
                    borderWidth: 2,
                    fill: false,
                    tension: 0.1,
                    pointRadius: 0,
                    pointHoverRadius: 4
                });
            } else {
                // Add a transparent line dataset as placeholder (will be overridden by plugin)
                datasets.push({
                    label: `${data.symbol} Price`,
                    data: candlestickData.map(c => ({ x: c.time, y: c.close })),
                    borderColor: 'transparent',
                    backgroundColor: 'transparent',
                    borderWidth: 0,
                    pointRadius: 0,
                    pointHoverRadius: 0
                });
                
                // Store candles data for plugin - ensure it's stored on the chart
                // This will be used by the rendering plugin
            }
            
            if (candlestickController) {
                // Controller is available, but we'll use custom rendering for mixed datasets
            } else {
                // Fallback to line chart if candlestick not available
                console.warn('Candlestick controller not available, using line chart');
                currentChartType = 'line'; // Update state to match reality
                
                // Update UI to reflect line chart
                const lineBtn = document.getElementById('fib-chart-type-line');
                const candlestickBtn = document.getElementById('fib-chart-type-candlestick');
                if (lineBtn) lineBtn.classList.add('active');
                if (candlestickBtn) candlestickBtn.classList.remove('active');
                
                const priceData = data.candles.map(c => ({
                    x: c.time * 1000,
                    y: typeof c.close === 'number' ? c.close : parseFloat(c.close)
                }));
                
                datasets.push({
                    label: `${data.symbol} Price`,
                    data: priceData,
                    borderColor: '#3b82f6',
                    backgroundColor: 'rgba(59, 130, 246, 0.1)',
                    borderWidth: 2,
                    fill: false,
                    tension: 0.1,
                    pointRadius: 0,
                    pointHoverRadius: 4
                });
            }
        } else {
            // Line chart data
            const priceData = data.candles.map(c => ({
                x: c.time * 1000, // Convert to milliseconds for Chart.js
                y: c.close
            }));
            
            datasets.push({
                label: `${data.symbol} Price`,
                data: priceData,
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                borderWidth: 2,
                fill: false,
                tension: 0.1,
                pointRadius: 0,
                pointHoverRadius: 4
            });
        }
        
        // Fibonacci level colors - distinct colors for each level
        const fibColors = {
            // Positive levels (extensions above)
            '0.0': '#22c55e',      // Green - Base (Low/High)
            '0.5': '#4ade80',      // Light green
            '1.0': '#eab308',      // Yellow - Key level
            '1.382': '#f97316',    // Orange
            '1.618': '#ef4444',    // Red - Golden ratio (key)
            '2.0': '#dc2626',      // Dark red
            '2.382': '#b91c1c',    // Darker red
            '2.618': '#991b1b',    // Deep red (key)
            '3.0': '#7f1d1d',      // Very dark red
            '3.382': '#881337',    // Maroon
            '3.618': '#9f1239',    // Rose red
            '4.24': '#be123c',     // Crimson (key)
            '5.08': '#e11d48',     // Bright red
            '6.86': '#f43f5e',     // Rose
            '11.01': '#fb7185',    // Light rose
            // Negative levels (retracements below)
            '-0.5': '#84cc16',     // Lime
            '-1.0': '#a3e635',     // Light lime (key)
            '-1.382': '#bef264',   // Pale lime
            '-1.618': '#d9f99d',   // Very pale lime (key)
            '-2.0': '#14b8a6',     // Teal
            '-2.382': '#2dd4bf',   // Light teal
            '-2.618': '#5eead4',   // Pale teal (key)
            '-3.0': '#06b6d4',     // Cyan
            '-3.382': '#22d3ee',   // Light cyan
            '-3.618': '#67e8f9',   // Pale cyan
            '-4.24': '#a5f3fc',    // Very pale cyan (key)
            '-5.08': '#818cf8',    // Indigo
            '-6.86': '#a78bfa',    // Purple
            '-11.01': '#c4b5fd'    // Light purple
        };
        
        // Determine which levels are key levels for styling (solid vs dashed)
        const keyRatios = [0, 1, 1.618, 2.618, 4.24, -1, -1.618, -2.618, -4.24];
        
        // Add ALL Fibonacci levels as horizontal lines
        const allLevels = [
            ...data.positiveLevels,
            ...data.negativeLevels
        ];
        
        allLevels.forEach(level => {
            const color = fibColors[level.label] || '#6b7280';
            const isKeyLevel = keyRatios.includes(level.ratio);
            
            datasets.push({
                label: `Fib ${level.label}`,
                data: [
                    { x: minTime, y: level.price },
                    { x: maxTime, y: level.price }
                ],
                borderColor: color,
                backgroundColor: color + '40',
                borderWidth: isKeyLevel ? 2 : 1,
                borderDash: isKeyLevel ? [] : [5, 5],
                fill: false,
                pointRadius: 0,
                pointHoverRadius: 0,
                spanGaps: true
            });
        });
        
        // For candlestick charts, we need to use 'line' chart type but with candlestick dataset
        // This allows us to mix candlestick data with line data for Fibonacci levels
        // Determine target chart type - always use 'line' to support mixed datasets
        const targetChartType = 'line';
        
        // Always use 'line' chart type to support mixed datasets
        if (fibChart.config.type !== 'line') {
            // Need to recreate chart with 'line' type
            const ctx = elements.chartCanvas.getContext('2d');
            const oldOptions = JSON.parse(JSON.stringify(fibChart.options)); // Deep copy
            
            // Preserve zoom state
            const xScale = fibChart.scales.x;
            const yScale = fibChart.scales.y;
            const zoomState = {
                xMin: xScale.options.min,
                xMax: xScale.options.max,
                yMin: yScale.options.min,
                yMax: yScale.options.max
            };
            
            fibChart.destroy();
            
            fibChart = new Chart(ctx, {
                type: 'line',
                data: {
                    datasets: datasets
                },
                options: oldOptions
            });
            
            // Setup custom pan after chart recreation
            setupFibChartPan(fibChart);
            
            // Restore zoom state if it exists
            if (zoomState.xMin !== undefined || zoomState.xMax !== undefined) {
                const newXScale = fibChart.scales.x;
                const newYScale = fibChart.scales.y;
                if (zoomState.xMin !== undefined) newXScale.options.min = zoomState.xMin;
                if (zoomState.xMax !== undefined) newXScale.options.max = zoomState.xMax;
                if (zoomState.yMin !== undefined) newYScale.options.min = zoomState.yMin;
                if (zoomState.yMax !== undefined) newYScale.options.max = zoomState.yMax;
            }
            
            setupChartControls();
        } else {
            // Just update data
            fibChart.data.datasets = datasets;
        }
        
        // Add custom candlestick rendering if needed
        if (currentChartType === 'candlestick') {
            // Prepare and store candles data for the plugin
            const candlestickData = data.candles.map(c => {
                const open = typeof c.open === 'number' ? c.open : parseFloat(c.open);
                const high = typeof c.high === 'number' ? c.high : parseFloat(c.high);
                const low = typeof c.low === 'number' ? c.low : parseFloat(c.low);
                const close = typeof c.close === 'number' ? c.close : parseFloat(c.close);
                
                if (isNaN(open) || isNaN(high) || isNaN(low) || isNaN(close)) {
                    return null;
                }
                
                return {
                    time: c.time * 1000,
                    open: open,
                    high: high,
                    low: low,
                    close: close
                };
            }).filter(item => item !== null);
            
            if (candlestickData.length > 0) {
                // Store on chart for plugin access - do this before registering plugin
                fibChart._candlesData = candlestickData;
                fibChart._candlestickColors = {
                    bullish: '#22c55e',
                    bearish: '#ef4444'
                };
                addCandlestickRenderingPlugin(candlestickData);
            } else {
                console.warn('No valid candlestick data for rendering');
                removeCandlestickRenderingPlugin();
            }
        } else {
            removeCandlestickRenderingPlugin();
            if (fibChart._candlesData) {
                delete fibChart._candlesData;
            }
            if (fibChart._candlestickColors) {
                delete fibChart._candlestickColors;
            }
        }
        
        // Force chart update to trigger plugin rendering
        fibChart.update('none');
        
        // Reset zoom after data update
        setTimeout(() => {
            if (fibChart) {
                const xScale = fibChart.scales.x;
                const yScale = fibChart.scales.y;
                delete xScale.options.min;
                delete xScale.options.max;
                delete yScale.options.min;
                delete yScale.options.max;
                fibChart.update('none');
            }
        }, 100);
    }
    
    function showLoading() {
        if (elements.loading) elements.loading.style.display = 'flex';
        if (elements.marketData) elements.marketData.style.display = 'none';
        if (elements.levelsContainer) elements.levelsContainer.style.display = 'none';
        if (elements.chartLoading) elements.chartLoading.style.display = 'flex';
        if (elements.chartEmpty) elements.chartEmpty.style.display = 'none';
    }
    
    function hideLoading() {
        if (elements.loading) elements.loading.style.display = 'none';
        if (elements.chartLoading) elements.chartLoading.style.display = 'none';
    }
    
    function showError(message) {
        if (elements.error) {
            elements.error.textContent = message;
            elements.error.style.display = 'block';
        }
        if (elements.marketData) elements.marketData.style.display = 'none';
        if (elements.levelsContainer) elements.levelsContainer.style.display = 'none';
        if (elements.chartEmpty) elements.chartEmpty.style.display = 'flex';
        if (elements.chartLoading) elements.chartLoading.style.display = 'none';
    }
    
    function hideError() {
        if (elements.error) elements.error.style.display = 'none';
    }
    
    // Auto-calculate when chart symbol changes
    function calculateForSymbol(symbol) {
        if (elements.symbolInput) {
            elements.symbolInput.value = symbol;
            calculate();
        }
    }
    
    // Initialize when DOM is ready and Chart.js is loaded
    function initializeWhenReady() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', function() {
                // Wait a bit for Chart.js to load
                setTimeout(init, 200);
            });
        } else {
            // Wait a bit for Chart.js to load
            setTimeout(init, 200);
        }
    }
    
    initializeWhenReady();
    
    // Public API
    return {
        init,
        calculate,
        calculateForSymbol
    };
})();



