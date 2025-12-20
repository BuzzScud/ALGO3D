// Charts Module - Trading Charts with Technical Analysis
const ChartsModule = (function() {
    let mainChart = null;
    let volumeChart = null;
    let currentSymbol = '';
    let currentTimeframe = '1D';
    let currentChartType = 'candlestick';
    let chartData = null;
    let currentView = '2d'; // '2d' or '3d'
    let activeIndicators = {
        sma20: false,
        sma50: false,
        ema12: false,
        ema26: false,
        bollinger: false,
        fibonacci: false,
        volume: true
    };
    
    // Pan state management (moved outside renderChart to avoid duplicates)
    let panState = {
        isPanning: false,
        startX: 0,
        startY: 0,
        lastX: 0,
        lastY: 0,
        boundHandlers: null
    };

    // 3D Chart state
    let scene3d = null;
    let camera3d = null;
    let renderer3d = null;
    let candlesticks3d = [];
    let animationId3d = null;
    let isAnimating = true;
    let autoRotate = true;
    let raycaster3d = null;
    let hoveredCandlestick = null;
    let tooltip3d = null;
    let axisLabels3d = [];
    let chartOffsetX = 0; // For panning left/right
    let minPrice3d = 0;
    let maxPrice3d = 0;
    let priceRange3d = 1;

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
        text: '#9ca3af',
        // Fibonacci retracement colors
        fib0: '#22c55e',      // 0% (High) - Green
        fib236: '#84cc16',    // 23.6% - Lime
        fib382: '#eab308',    // 38.2% - Yellow
        fib50: '#f97316',     // 50% - Orange
        fib618: '#ef4444',    // 61.8% - Red (Golden Ratio)
        fib786: '#dc2626',    // 78.6% - Dark Red
        fib100: '#991b1b',    // 100% (Low) - Deep Red
        // Fibonacci extension colors
        fib1618: '#9333ea',   // 161.8% - Purple
        fib2618: '#7c3aed',   // 261.8% - Violet
        fib4236: '#6366f1'    // 423.6% - Indigo
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

    // ==========================================
    // 3D CHART FUNCTIONS
    // ==========================================

    function toggleChartView() {
        try {
            const container2d = document.getElementById('chart-2d-container');
            const container3d = document.getElementById('chart-3d-container');
            
            if (!container2d || !container3d) {
                console.error('Chart containers not found');
                return;
            }
            
            if (currentView === '3d') {
                // Hide 2D container
                container2d.style.display = 'none';
                
                // Show 3D container
                container3d.style.display = 'block';
                container3d.style.visibility = 'visible';
                
                // Check if we have chart data
                if (!chartData || !chartData.candles || chartData.candles.length === 0) {
                    const loading = document.getElementById('chart-3d-loading');
                    if (loading) {
                        loading.style.display = 'flex';
                        loading.innerHTML = '<i class="fas fa-info-circle"></i><span>Please load a chart first to view in 3D</span>';
                    }
                    console.warn('No chart data available for 3D view');
                    return;
                }
                
                // Small delay to ensure container is visible and has dimensions
                setTimeout(() => {
                    try {
                        render3DChart();
                    } catch (err) {
                        console.error('Error rendering 3D chart:', err);
                        const loading = document.getElementById('chart-3d-loading');
                        if (loading) {
                            loading.style.display = 'flex';
                            loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Error loading 3D view. Check console for details.</span>';
                        }
                    }
                }, 200);
            } else {
                // Show 2D container
                container2d.style.display = 'block';
                
                // Hide 3D container
                container3d.style.display = 'none';
                cleanup3DScene();
            }
        } catch (err) {
            console.error('Error toggling chart view:', err);
            // Fallback to 2D view on error
            currentView = '2d';
            const container2d = document.getElementById('chart-2d-container');
            const container3d = document.getElementById('chart-3d-container');
            if (container2d) {
                container2d.style.display = 'block';
            }
            if (container3d) {
                container3d.style.display = 'none';
                cleanup3DScene();
            }
            
            // Show error notification if available
            if (typeof showError === 'function') {
                showError('Error switching chart view: ' + err.message);
            }
        }
    }

    function render3DChart() {
        console.log('render3DChart called, chartData:', chartData);
        
        if (!chartData || !chartData.candles || chartData.candles.length === 0) {
            console.warn('No chart data available for 3D rendering');
            const loading = document.getElementById('chart-3d-loading');
            if (loading) {
                loading.style.display = 'flex';
                loading.innerHTML = '<i class="fas fa-info-circle"></i><span>No chart data available. Load a chart first.</span>';
            }
            return;
        }

        const container = document.getElementById('chart-3d-container');
        const canvas = document.getElementById('chart-3d-canvas');
        const loading = document.getElementById('chart-3d-loading');
        
        if (!container) {
            console.error('3D chart container not found');
            return;
        }
        
        if (!canvas) {
            console.error('3D chart canvas not found');
            if (loading) {
                loading.style.display = 'flex';
                loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Canvas element not found</span>';
            }
            return;
        }
        
        // Ensure container is visible and has dimensions
        const containerWidth = container.clientWidth || container.offsetWidth;
        const containerHeight = container.clientHeight || container.offsetHeight;
        
        if (containerWidth === 0 || containerHeight === 0) {
            console.warn('Container has zero dimensions, waiting...');
            setTimeout(() => render3DChart(), 100);
            return;
        }
        
        if (loading) loading.style.display = 'flex';
        
        console.log('Starting 3D chart initialization...');

        // Load Three.js if not available
        if (typeof THREE === 'undefined') {
            console.log('Loading Three.js library...');
            loadThreeJs().then(() => {
                console.log('Three.js loaded successfully');
                try {
                    init3DScene(container, canvas);
                    if (loading) loading.style.display = 'none';
                    console.log('3D scene initialized successfully');
                } catch (err) {
                    console.error('Failed to initialize 3D scene:', err);
                    console.error('Error stack:', err.stack);
                    if (loading) {
                        loading.style.display = 'flex';
                        loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Failed to initialize 3D scene: ' + err.message + '</span>';
                    }
                }
            }).catch(err => {
                console.error('Failed to load Three.js:', err);
                if (loading) {
                    loading.style.display = 'flex';
                    loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Failed to load 3D engine. Please check your internet connection.</span>';
                }
            });
        } else {
            console.log('Three.js already available');
            try {
                init3DScene(container, canvas);
                if (loading) loading.style.display = 'none';
                console.log('3D scene initialized successfully');
            } catch (err) {
                console.error('Failed to initialize 3D scene:', err);
                console.error('Error stack:', err.stack);
                if (loading) {
                    loading.style.display = 'flex';
                    loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Failed to initialize 3D scene: ' + err.message + '</span>';
                }
            }
        }
    }

    function loadThreeJs() {
        return new Promise((resolve, reject) => {
            if (typeof THREE !== 'undefined') {
                resolve();
                return;
            }
            
            // Check if script is already being loaded
            const existingScript = document.querySelector('script[src*="three.js"]');
            if (existingScript) {
                existingScript.addEventListener('load', resolve);
                existingScript.addEventListener('error', reject);
                return;
            }
            
            const script = document.createElement('script');
            script.src = 'https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js';
            script.onload = () => {
                // Verify THREE is actually available
                if (typeof THREE !== 'undefined') {
                    resolve();
                } else {
                    reject(new Error('Three.js loaded but not available'));
                }
            };
            script.onerror = () => reject(new Error('Failed to load Three.js from CDN'));
            document.head.appendChild(script);
        });
    }

    function init3DScene(container, canvas) {
        // Cleanup existing scene
        cleanup3DScene();

        if (typeof THREE === 'undefined') {
            throw new Error('Three.js is not loaded');
        }

        // Force container to be visible and have dimensions
        container.style.display = 'flex'; // Keep flex for proper layout
        container.style.visibility = 'visible';
        container.style.position = 'relative';
        
        // Ensure parent has height
        const parent = container.parentElement;
        if (parent) {
            const parentHeight = parent.clientHeight || parent.offsetHeight;
            if (parentHeight > 0) {
                container.style.minHeight = parentHeight + 'px';
            }
        }
        
        // Get dimensions function
        const getDimensions = () => {
            let width = container.clientWidth;
            let height = container.clientHeight;
            
            if (!width || width === 0) {
                width = container.offsetWidth || container.getBoundingClientRect().width || 800;
            }
            
            if (!height || height === 0) {
                height = container.offsetHeight || container.getBoundingClientRect().height || 500;
            }
            
            // Ensure minimum dimensions
            if (width < 100) width = 800;
            if (height < 100) height = 500;
            
            return { width, height };
        };
        
        // Initialize scene with proper dimensions
        const initializeScene = () => {
            const { width, height } = getDimensions();
            console.log('Initializing 3D scene with dimensions:', width, 'x', height);

            // Scene
            scene3d = new THREE.Scene();
            scene3d.background = new THREE.Color(0x0f172a);

            // Camera
            camera3d = new THREE.PerspectiveCamera(50, width / height, 0.1, 2000);
            camera3d.position.set(50, 40, 80);
            camera3d.lookAt(0, 0, 0);

            // Renderer
            renderer3d = new THREE.WebGLRenderer({ 
                canvas: canvas, 
                antialias: true,
                alpha: true 
            });
            renderer3d.setSize(width, height);
            renderer3d.setPixelRatio(Math.min(window.devicePixelRatio, 2));
            renderer3d.shadowMap.enabled = true;
            renderer3d.shadowMap.type = THREE.PCFSoftShadowMap;

            // Lighting
            const ambientLight = new THREE.AmbientLight(0xffffff, 0.4);
            scene3d.add(ambientLight);

            const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
            directionalLight.position.set(50, 100, 50);
            directionalLight.castShadow = true;
            scene3d.add(directionalLight);

            const pointLight = new THREE.PointLight(0x3b82f6, 0.5, 200);
            pointLight.position.set(0, 50, 0);
            scene3d.add(pointLight);

            // Add grid floor
            createGridFloor();

            // Create 3D candlesticks (must be before axes to calculate price range)
            create3DCandlesticks();

            // Add axes (after candlesticks so we have price range)
            createAxes();

            // Setup mouse controls
            setup3DControls(canvas);

            // Start animation loop
            animate3D();

            // Handle resize with fallback if ResizeObserver is not available
            if (typeof ResizeObserver !== 'undefined') {
                const resizeObserver = new ResizeObserver(() => {
                    if (!container || !camera3d || !renderer3d) return;
                    const { width: newWidth, height: newHeight } = getDimensions();
                    camera3d.aspect = newWidth / newHeight;
                    camera3d.updateProjectionMatrix();
                    renderer3d.setSize(newWidth, newHeight);
                });
                resizeObserver.observe(container);
            } else {
                // Fallback to window resize event
                const handleResize = () => {
                    if (!container || !camera3d || !renderer3d) return;
                    const { width: newWidth, height: newHeight } = getDimensions();
                    camera3d.aspect = newWidth / newHeight;
                    camera3d.updateProjectionMatrix();
                    renderer3d.setSize(newWidth, newHeight);
                };
                window.addEventListener('resize', handleResize);
            }
        };
        
        // Try to get dimensions immediately, but retry if needed
        const dims = getDimensions();
        
        if (dims.width === 800 || dims.height === 500) {
            // Might need to wait for layout, try again after a frame
            requestAnimationFrame(initializeScene);
        } else {
            initializeScene();
        }
    }

    function create3DCandlesticks() {
        if (!chartData || !chartData.candles || !scene3d) {
            console.warn('Cannot create 3D candlesticks: missing data or scene');
            return;
        }

        const candles = chartData.candles;
        if (candles.length === 0) {
            console.warn('No candle data available');
            return;
        }

        const numCandles = Math.min(candles.length, 100); // Limit for performance
        
        // Calculate price range for scaling
        const prices = candles.flatMap(c => [c.high, c.low]);
        if (prices.length === 0) {
            console.warn('No price data available');
            return;
        }
        const minPrice = Math.min(...prices);
        const maxPrice = Math.max(...prices);
        const priceRange = maxPrice - minPrice || 1; // Prevent division by zero
        
        // Calculate volume range
        const volumes = candles.map(c => c.volume || 0);
        const maxVolume = Math.max(...volumes, 1); // Prevent division by zero

        // Clear existing candlesticks
        candlesticks3d.forEach(group => {
            if (group && scene3d) {
                scene3d.remove(group);
            }
        });
        candlesticks3d = [];

        // Spacing and sizing
        const spacing = 1.2;
        const startX = -(numCandles * spacing) / 2;
        const bodyWidth = 0.6;
        const wickWidth = 0.08;
        const heightScale = 40 / priceRange;

        // Create candlesticks from most recent data
        const startIdx = Math.max(0, candles.length - numCandles);
        
        for (let i = startIdx; i < candles.length; i++) {
            const candle = candles[i];
            const idx = i - startIdx;
            const x = startX + idx * spacing;

            const open = (candle.open - minPrice) * heightScale;
            const close = (candle.close - minPrice) * heightScale;
            const high = (candle.high - minPrice) * heightScale;
            const low = (candle.low - minPrice) * heightScale;

            const isBullish = candle.close >= candle.open;
            const color = isBullish ? 0x22c55e : 0xef4444;
            const emissiveColor = isBullish ? 0x166534 : 0x991b1b;

            const group = new THREE.Group();

            // Body
            const bodyHeight = Math.max(Math.abs(close - open), 0.1);
            const bodyBottom = Math.min(open, close);
            const bodyGeometry = new THREE.BoxGeometry(bodyWidth, bodyHeight, bodyWidth);
            const bodyMaterial = new THREE.MeshPhongMaterial({ 
                color: color,
                emissive: emissiveColor,
                emissiveIntensity: 0.2,
                shininess: 30
            });
            const bodyMesh = new THREE.Mesh(bodyGeometry, bodyMaterial);
            bodyMesh.position.set(0, bodyBottom + bodyHeight / 2, 0);
            bodyMesh.castShadow = true;
            bodyMesh.receiveShadow = true;
            bodyMesh.userData.parentGroup = group;
            bodyMesh.userData.isCandlestick = true;
            group.add(bodyMesh);

            // Upper wick
            let upperWickMesh = null;
            const upperWickHeight = high - Math.max(open, close);
            if (upperWickHeight > 0.01) {
                const upperWickGeometry = new THREE.BoxGeometry(wickWidth, upperWickHeight, wickWidth);
                const wickMaterial = new THREE.MeshPhongMaterial({ color: color });
                upperWickMesh = new THREE.Mesh(upperWickGeometry, wickMaterial);
                upperWickMesh.position.set(0, Math.max(open, close) + upperWickHeight / 2, 0);
                upperWickMesh.userData.parentGroup = group;
                upperWickMesh.userData.isCandlestick = true;
                group.add(upperWickMesh);
            }

            // Lower wick
            let lowerWickMesh = null;
            const lowerWickHeight = Math.min(open, close) - low;
            if (lowerWickHeight > 0.01) {
                const lowerWickGeometry = new THREE.BoxGeometry(wickWidth, lowerWickHeight, wickWidth);
                const wickMaterial = new THREE.MeshPhongMaterial({ color: color });
                lowerWickMesh = new THREE.Mesh(lowerWickGeometry, wickMaterial);
                lowerWickMesh.position.set(0, low + lowerWickHeight / 2, 0);
                lowerWickMesh.userData.parentGroup = group;
                lowerWickMesh.userData.isCandlestick = true;
                group.add(lowerWickMesh);
            }

            // Volume bar (behind candlestick)
            if (candle.volume && maxVolume > 0) {
                const volumeHeight = (candle.volume / maxVolume) * 15;
                const volumeGeometry = new THREE.BoxGeometry(bodyWidth * 0.8, volumeHeight, bodyWidth * 0.3);
                const volumeMaterial = new THREE.MeshPhongMaterial({ 
                    color: isBullish ? 0x22c55e : 0xef4444,
                    opacity: 0.3,
                    transparent: true
                });
                const volumeMesh = new THREE.Mesh(volumeGeometry, volumeMaterial);
                volumeMesh.position.set(0, volumeHeight / 2, -1);
                group.add(volumeMesh);
            }

            group.position.x = x;
            group.userData = { 
                candle: candle, 
                index: idx,
                originalX: x // Store original position for panning
            };
            
            // Make candlestick parts detectable by raycasting
            bodyMesh.userData.isCandlestick = true;
            bodyMesh.userData.parentGroup = group;
            if (upperWickMesh) {
                upperWickMesh.userData.isCandlestick = true;
                upperWickMesh.userData.parentGroup = group;
            }
            if (lowerWickMesh) {
                lowerWickMesh.userData.isCandlestick = true;
                lowerWickMesh.userData.parentGroup = group;
            }
            
            scene3d.add(group);
            candlesticks3d.push(group);
        }
        
        // Reset pan offset when creating new candlesticks
        chartOffsetX = 0;
    }

    function createGridFloor() {
        const gridSize = 120;
        const gridDivisions = 30;
        const gridHelper = new THREE.GridHelper(gridSize, gridDivisions, 0x3b82f6, 0x1e3a5f);
        gridHelper.position.y = -1;
        scene3d.add(gridHelper);

        // Add floor plane
        const floorGeometry = new THREE.PlaneGeometry(gridSize, gridSize);
        const floorMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x0f172a,
            side: THREE.DoubleSide,
            opacity: 0.8,
            transparent: true
        });
        const floor = new THREE.Mesh(floorGeometry, floorMaterial);
        floor.rotation.x = -Math.PI / 2;
        floor.position.y = -1.01;
        floor.receiveShadow = true;
        scene3d.add(floor);
    }

    function createAxes() {
        // Clear existing axis labels
        axisLabels3d.forEach(label => {
            if (label && scene3d) {
                scene3d.remove(label);
            }
        });
        axisLabels3d = [];

        if (!chartData || !chartData.candles || chartData.candles.length === 0) return;

        const candles = chartData.candles;
        const numCandles = Math.min(candles.length, 100);
        const startIdx = Math.max(0, candles.length - numCandles);
        
        // Calculate price range
        const prices = candles.flatMap(c => [c.high, c.low]);
        minPrice3d = Math.min(...prices);
        maxPrice3d = Math.max(...prices);
        priceRange3d = maxPrice3d - minPrice3d || 1;
        
        // Price axis (Y) - Left side
        const axisHeight = 50;
        const priceAxisGeometry = new THREE.BoxGeometry(0.15, axisHeight, 0.15);
        const priceAxisMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x3b82f6,
            emissive: 0x1e40af,
            emissiveIntensity: 0.3
        });
        const priceAxis = new THREE.Mesh(priceAxisGeometry, priceAxisMaterial);
        priceAxis.position.set(-65, axisHeight / 2 - 1, 0);
        priceAxis.castShadow = true;
        scene3d.add(priceAxis);

        // Price axis labels (Y-axis)
        const numPriceLabels = 8;
        const priceStep = priceRange3d / (numPriceLabels - 1);
        
        for (let i = 0; i < numPriceLabels; i++) {
            const price = maxPrice3d - (priceStep * i);
            const yPos = (i / (numPriceLabels - 1)) * axisHeight - 1;
            
            // Create label sprite using canvas
            const canvas = document.createElement('canvas');
            canvas.width = 120;
            canvas.height = 40;
            const context = canvas.getContext('2d');
            
            context.fillStyle = '#1e293b';
            context.fillRect(0, 0, canvas.width, canvas.height);
            
            context.fillStyle = '#ffffff';
            context.font = 'bold 14px "SF Mono", "Monaco", "Consolas", monospace';
            context.textAlign = 'right';
            context.textBaseline = 'middle';
            context.fillText('$' + price.toFixed(2), canvas.width - 10, canvas.height / 2);
            
            const texture = new THREE.CanvasTexture(canvas);
            texture.needsUpdate = true;
            
            const spriteMaterial = new THREE.SpriteMaterial({ 
                map: texture,
                transparent: true,
                alphaTest: 0.1
            });
            const sprite = new THREE.Sprite(spriteMaterial);
            sprite.scale.set(8, 3, 1);
            sprite.position.set(-72, yPos, 0);
            sprite.userData.isAxisLabel = true;
            scene3d.add(sprite);
            axisLabels3d.push(sprite);
            
            // Add tick mark
            const tickGeometry = new THREE.BoxGeometry(0.05, 0.3, 0.05);
            const tickMaterial = new THREE.MeshBasicMaterial({ color: 0x64748b });
            const tick = new THREE.Mesh(tickGeometry, tickMaterial);
            tick.position.set(-64.8, yPos, 0);
            scene3d.add(tick);
        }

        // Y-axis title
        const yAxisTitleCanvas = document.createElement('canvas');
        yAxisTitleCanvas.width = 200;
        yAxisTitleCanvas.height = 40;
        const yAxisTitleContext = yAxisTitleCanvas.getContext('2d');
        yAxisTitleContext.fillStyle = '#1e293b';
        yAxisTitleContext.fillRect(0, 0, yAxisTitleCanvas.width, yAxisTitleCanvas.height);
        yAxisTitleContext.fillStyle = '#3b82f6';
        yAxisTitleContext.font = 'bold 16px Arial';
        yAxisTitleContext.textAlign = 'center';
        yAxisTitleContext.textBaseline = 'middle';
        yAxisTitleContext.fillText('PRICE', yAxisTitleCanvas.width / 2, yAxisTitleCanvas.height / 2);
        
        const yAxisTitleTexture = new THREE.CanvasTexture(yAxisTitleCanvas);
        yAxisTitleTexture.needsUpdate = true;
        const yAxisTitleSprite = new THREE.Sprite(new THREE.SpriteMaterial({ 
            map: yAxisTitleTexture,
            transparent: true
        }));
        yAxisTitleSprite.scale.set(10, 2, 1);
        yAxisTitleSprite.position.set(-72, axisHeight / 2 + 5, 0);
        yAxisTitleSprite.rotation.z = Math.PI / 2;
        scene3d.add(yAxisTitleSprite);

        // Time axis (X) - Bottom
        const axisWidth = 130;
        const timeAxisGeometry = new THREE.BoxGeometry(axisWidth, 0.15, 0.15);
        const timeAxisMaterial = new THREE.MeshPhongMaterial({ 
            color: 0x3b82f6,
            emissive: 0x1e40af,
            emissiveIntensity: 0.3
        });
        const timeAxis = new THREE.Mesh(timeAxisGeometry, timeAxisMaterial);
        timeAxis.position.set(0, -1, 0);
        timeAxis.castShadow = true;
        scene3d.add(timeAxis);

        // Time axis labels (X-axis)
        const numTimeLabels = Math.min(10, numCandles);
        const timeStep = Math.floor(numCandles / numTimeLabels);
        
        for (let i = 0; i < numTimeLabels; i++) {
            const candleIdx = startIdx + (i * timeStep);
            if (candleIdx >= candles.length) break;
            
            const candle = candles[candleIdx];
            const xPos = (-(numCandles * 1.2) / 2) + (i * timeStep * 1.2);
            
            // Format time - CRITICAL: Use EST timezone to show correct current time
            const date = new Date(candle.time);
            let timeStr = '';
            if (currentTimeframe === '1D' || currentTimeframe === '15MIN') {
                timeStr = date.toLocaleTimeString('en-US', { 
                    timeZone: 'America/New_York',
                    hour: '2-digit', 
                    minute: '2-digit', 
                    hour12: false 
                });
            } else if (currentTimeframe === '1H' || currentTimeframe === '4H') {
                timeStr = date.toLocaleDateString('en-US', { 
                    timeZone: 'America/New_York',
                    month: 'short', 
                    day: 'numeric' 
                }) + ' ' +
                         date.toLocaleTimeString('en-US', { 
                             timeZone: 'America/New_York',
                             hour: '2-digit', 
                             minute: '2-digit', 
                             hour12: false 
                         });
            } else {
                timeStr = date.toLocaleDateString('en-US', { 
                    timeZone: 'America/New_York',
                    month: 'short', 
                    day: 'numeric' 
                });
            }
            
            // Create label sprite
            const timeCanvas = document.createElement('canvas');
            timeCanvas.width = 120;
            timeCanvas.height = 40;
            const timeContext = timeCanvas.getContext('2d');
            
            timeContext.fillStyle = '#1e293b';
            timeContext.fillRect(0, 0, timeCanvas.width, timeCanvas.height);
            
            timeContext.fillStyle = '#ffffff';
            timeContext.font = 'bold 12px "SF Mono", "Monaco", "Consolas", monospace';
            timeContext.textAlign = 'center';
            timeContext.textBaseline = 'middle';
            timeContext.fillText(timeStr, timeCanvas.width / 2, timeCanvas.height / 2);
            
            const timeTexture = new THREE.CanvasTexture(timeCanvas);
            timeTexture.needsUpdate = true;
            
            const timeSprite = new THREE.Sprite(new THREE.SpriteMaterial({ 
                map: timeTexture,
                transparent: true,
                alphaTest: 0.1
            }));
            timeSprite.scale.set(6, 2, 1);
            timeSprite.position.set(xPos, -4, 0);
            timeSprite.userData.isAxisLabel = true;
            timeSprite.userData.originalX = xPos; // Store original position for panning
            scene3d.add(timeSprite);
            axisLabels3d.push(timeSprite);
            
            // Add tick mark
            const timeTickGeometry = new THREE.BoxGeometry(0.3, 0.05, 0.05);
            const timeTickMaterial = new THREE.MeshBasicMaterial({ color: 0x64748b });
            const timeTick = new THREE.Mesh(timeTickGeometry, timeTickMaterial);
            timeTick.position.set(xPos, -1.1, 0);
            scene3d.add(timeTick);
        }

        // X-axis title
        const xAxisTitleCanvas = document.createElement('canvas');
        xAxisTitleCanvas.width = 200;
        xAxisTitleCanvas.height = 40;
        const xAxisTitleContext = xAxisTitleCanvas.getContext('2d');
        xAxisTitleContext.fillStyle = '#1e293b';
        xAxisTitleContext.fillRect(0, 0, xAxisTitleCanvas.width, xAxisTitleCanvas.height);
        xAxisTitleContext.fillStyle = '#3b82f6';
        xAxisTitleContext.font = 'bold 16px Arial';
        xAxisTitleContext.textAlign = 'center';
        xAxisTitleContext.textBaseline = 'middle';
        xAxisTitleContext.fillText('TIME', xAxisTitleCanvas.width / 2, xAxisTitleCanvas.height / 2);
        
        const xAxisTitleTexture = new THREE.CanvasTexture(xAxisTitleCanvas);
        xAxisTitleTexture.needsUpdate = true;
        const xAxisTitleSprite = new THREE.Sprite(new THREE.SpriteMaterial({ 
            map: xAxisTitleTexture,
            transparent: true
        }));
        xAxisTitleSprite.scale.set(10, 2, 1);
        xAxisTitleSprite.position.set(0, -7, 0);
        scene3d.add(xAxisTitleSprite);
    }

    function create3DTooltip() {
        // Remove existing tooltip if any
        const existing = document.getElementById('chart-3d-tooltip');
        if (existing) existing.remove();
        
        // Create tooltip element
        tooltip3d = document.createElement('div');
        tooltip3d.id = 'chart-3d-tooltip';
        tooltip3d.style.cssText = `
            position: absolute;
            background: rgba(15, 23, 42, 0.95);
            color: white;
            padding: 0.75rem 1rem;
            border-radius: 0.5rem;
            font-size: 0.85rem;
            pointer-events: none;
            z-index: 10000;
            border: 1px solid rgba(59, 130, 246, 0.5);
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
            display: none;
            font-family: 'SF Mono', 'Monaco', 'Consolas', monospace;
        `;
        document.body.appendChild(tooltip3d);
        return tooltip3d;
    }

    function setup3DControls(canvas) {
        let isDragging = false;
        let isPanning = false;
        let previousMousePosition = { x: 0, y: 0 };
        let panStartPosition = { x: 0, y: 0 };
        
        // Initialize raycaster for hover detection
        if (typeof THREE !== 'undefined') {
            raycaster3d = new THREE.Raycaster();
        }
        
        // Create tooltip
        create3DTooltip();

        canvas.addEventListener('mousedown', (e) => {
            const button = e.button;
            
            if (button === 0) { // Left click - rotate
                isDragging = true;
                autoRotate = false;
                previousMousePosition = { x: e.clientX, y: e.clientY };
                
                // Hide tooltip when dragging
                if (tooltip3d) tooltip3d.style.display = 'none';
            } else if (button === 2 || e.shiftKey) { // Right click or Shift - pan
                isPanning = true;
                panStartPosition = { x: e.clientX, y: e.clientY };
                canvas.style.cursor = 'move';
            }
        });

        canvas.addEventListener('contextmenu', (e) => {
            e.preventDefault(); // Prevent right-click context menu
        });

        canvas.addEventListener('mousemove', (e) => {
            const rect = canvas.getBoundingClientRect();
            const mouseX = e.clientX - rect.left;
            const mouseY = e.clientY - rect.top;
            
            if (isDragging && !e.shiftKey) {
                const deltaX = e.clientX - previousMousePosition.x;
                const deltaY = e.clientY - previousMousePosition.y;

                // Rotate camera around scene
                const rotationSpeed = 0.01;
                const theta = deltaX * rotationSpeed;

                // Calculate new camera position (orbit around Y axis)
                const x = camera3d.position.x;
                const z = camera3d.position.z;
                camera3d.position.x = x * Math.cos(theta) + z * Math.sin(theta);
                camera3d.position.z = z * Math.cos(theta) - x * Math.sin(theta);
                
                // Vertical movement (limited)
                camera3d.position.y = Math.max(10, Math.min(100, camera3d.position.y - deltaY * 0.2));

                camera3d.lookAt(0, 15, 0);

                previousMousePosition = { x: e.clientX, y: e.clientY };
            } else if (isPanning || e.shiftKey) {
                // Pan chart left/right (move candlesticks and axes)
                const deltaX = e.clientX - panStartPosition.x;
                const panSpeed = 0.05;
                chartOffsetX += deltaX * panSpeed;
                
                // Limit panning range
                const maxPan = 50;
                chartOffsetX = Math.max(-maxPan, Math.min(maxPan, chartOffsetX));
                
                // Update candlestick positions
                updateChartPan();
                
                panStartPosition = { x: e.clientX, y: e.clientY };
                canvas.style.cursor = 'move';
            } else {
                canvas.style.cursor = 'grab';
                // Hover detection using raycasting
                handle3DHover(mouseX, mouseY, e.clientX, e.clientY);
            }
        });

        canvas.addEventListener('mouseup', (e) => {
            isDragging = false;
            isPanning = false;
            canvas.style.cursor = 'grab';
        });

        canvas.addEventListener('mouseleave', () => {
            isDragging = false;
            isPanning = false;
            canvas.style.cursor = 'default';
            if (tooltip3d) tooltip3d.style.display = 'none';
            if (hoveredCandlestick) {
                hoveredCandlestick = null;
            }
        });

        // Zoom with scroll
        canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            const zoomSpeed = 0.1;
            const direction = e.deltaY > 0 ? 1 : -1;
            
            const distance = camera3d.position.length();
            const newDistance = Math.max(30, Math.min(200, distance + direction * distance * zoomSpeed));
            
            camera3d.position.normalize().multiplyScalar(newDistance);
            camera3d.lookAt(0, 15, 0);
        }, { passive: false });
    }

    function updateChartPan() {
        if (!scene3d || !candlesticks3d) return;
        
        // Update candlestick positions
        candlesticks3d.forEach(group => {
            if (group && group.userData.originalX !== undefined) {
                group.position.x = group.userData.originalX + chartOffsetX;
            }
        });
        
        // Update axis labels positions (only X-axis labels)
        axisLabels3d.forEach((label, index) => {
            if (label && label.userData.originalX !== undefined) {
                label.position.x = label.userData.originalX + chartOffsetX;
            }
        });
    }

    function handle3DHover(mouseX, mouseY, clientX, clientY) {
        if (!raycaster3d || !scene3d || !camera3d || !renderer3d || candlesticks3d.length === 0) {
            return;
        }

        // Normalize mouse coordinates to -1 to +1 range
        const mouse = new THREE.Vector2();
        mouse.x = (mouseX / renderer3d.domElement.clientWidth) * 2 - 1;
        mouse.y = -(mouseY / renderer3d.domElement.clientHeight) * 2 + 1;

        // Update raycaster with camera and mouse position
        raycaster3d.setFromCamera(mouse, camera3d);

        // Get all candlestick meshes (check intersections recursively)
        const intersects = raycaster3d.intersectObjects(candlesticks3d, true);

        if (intersects.length > 0) {
            // Find the parent group (candlestick) by traversing up the parent chain
            let candlestickGroup = null;
            for (let intersect of intersects) {
                let obj = intersect.object;
                
                // Traverse up the parent chain to find the group
                while (obj && obj.parent && obj.parent !== scene3d) {
                    // Check if this object's parent is a candlestick group
                    if (obj.userData && obj.userData.parentGroup) {
                        candlestickGroup = obj.userData.parentGroup;
                        break;
                    }
                    // Check if parent is directly in candlesticks3d array
                    if (candlesticks3d.includes(obj.parent)) {
                        candlestickGroup = obj.parent;
                        break;
                    }
                    obj = obj.parent;
                }
                
                if (candlestickGroup) break;
            }

            if (candlestickGroup && candlestickGroup.userData && candlestickGroup.userData.candle) {
                const candle = candlestickGroup.userData.candle;
                if (hoveredCandlestick !== candlestickGroup) {
                    hoveredCandlestick = candlestickGroup;
                }
                show3DTooltip(candle, clientX, clientY);
            } else {
                if (tooltip3d) tooltip3d.style.display = 'none';
                hoveredCandlestick = null;
            }
        } else {
            if (tooltip3d) tooltip3d.style.display = 'none';
            hoveredCandlestick = null;
        }
    }

    function show3DTooltip(candle, x, y) {
        if (!tooltip3d) return;

        const formatPrice = (price) => {
            return typeof price === 'number' ? price.toFixed(2) : '--';
        };

        const formatDate = (timestamp) => {
            if (!timestamp) return '--';
            const date = new Date(timestamp);
            // CRITICAL: Format in EST timezone to show correct current time
            return date.toLocaleString('en-US', {
                timeZone: 'America/New_York',
                month: 'short',
                day: 'numeric',
                year: 'numeric',
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                hour12: true
            }) + ' EST';
        };

        const change = candle.close - candle.open;
        const changePercent = candle.open !== 0 ? ((change / candle.open) * 100).toFixed(2) : '0.00';
        const changeColor = change >= 0 ? '#22c55e' : '#ef4444';

        tooltip3d.innerHTML = `
            <div style="margin-bottom: 0.5rem; font-weight: 600; font-size: 0.9rem; color: #3b82f6;">
                ${formatDate(candle.time)}
            </div>
            <div style="display: grid; grid-template-columns: auto 1fr; gap: 0.5rem 1rem; font-size: 0.8rem;">
                <div style="color: #94a3b8;">Open:</div>
                <div style="color: white; text-align: right;">$${formatPrice(candle.open)}</div>
                <div style="color: #94a3b8;">High:</div>
                <div style="color: #22c55e; text-align: right;">$${formatPrice(candle.high)}</div>
                <div style="color: #94a3b8;">Low:</div>
                <div style="color: #ef4444; text-align: right;">$${formatPrice(candle.low)}</div>
                <div style="color: #94a3b8;">Close:</div>
                <div style="color: white; text-align: right; font-weight: 600;">$${formatPrice(candle.close)}</div>
                <div style="color: #94a3b8;">Change:</div>
                <div style="color: ${changeColor}; text-align: right; font-weight: 600;">
                    ${change >= 0 ? '+' : ''}$${formatPrice(change)} (${change >= 0 ? '+' : ''}${changePercent}%)
                </div>
                ${candle.volume ? `
                    <div style="color: #94a3b8;">Volume:</div>
                    <div style="color: white; text-align: right;">${candle.volume.toLocaleString()}</div>
                ` : ''}
            </div>
        `;

        tooltip3d.style.display = 'block';
        
        // Position tooltip near cursor
        const tooltipRect = tooltip3d.getBoundingClientRect();
        const offsetX = 15;
        const offsetY = 15;
        
        let left = x + offsetX;
        let top = y + offsetY;
        
        // Keep tooltip within viewport
        if (left + tooltipRect.width > window.innerWidth) {
            left = x - tooltipRect.width - offsetX;
        }
        if (top + tooltipRect.height > window.innerHeight) {
            top = y - tooltipRect.height - offsetY;
        }
        
        tooltip3d.style.left = left + 'px';
        tooltip3d.style.top = top + 'px';
    }

    function animate3D() {
        if (!renderer3d || !scene3d || !camera3d) {
            if (animationId3d) {
                cancelAnimationFrame(animationId3d);
                animationId3d = null;
            }
            return;
        }

        try {
            animationId3d = requestAnimationFrame(animate3D);

            // Auto-rotate if enabled
            if (autoRotate && isAnimating) {
                const theta = 0.002;
                const x = camera3d.position.x;
                const z = camera3d.position.z;
                camera3d.position.x = x * Math.cos(theta) + z * Math.sin(theta);
                camera3d.position.z = z * Math.cos(theta) - x * Math.sin(theta);
                camera3d.lookAt(0, 15, 0);
            }

            // Subtle animation on candlesticks
            if (isAnimating && candlesticks3d.length > 0) {
                candlesticks3d.forEach((group, i) => {
                    if (group && group.position) {
                        // Store original Y position for animation
                        if (!group.userData.originalY) {
                            group.userData.originalY = group.position.y;
                        }
                        // Slight floating effect
                        const time = Date.now() * 0.001;
                        group.position.y = group.userData.originalY + Math.sin(time + i * 0.1) * 0.05;
                    }
                });
            }

            renderer3d.render(scene3d, camera3d);
        } catch (err) {
            console.error('Error in 3D animation loop:', err);
            if (animationId3d) {
                cancelAnimationFrame(animationId3d);
                animationId3d = null;
            }
        }
    }

    function rotate3DChart(amount) {
        if (!camera3d) return;
        
        const x = camera3d.position.x;
        const z = camera3d.position.z;
        camera3d.position.x = x * Math.cos(amount) + z * Math.sin(amount);
        camera3d.position.z = z * Math.cos(amount) - x * Math.sin(amount);
        camera3d.lookAt(0, 15, 0);
        autoRotate = false;
    }

    function reset3DView() {
        if (!camera3d) return;
        
        camera3d.position.set(50, 40, 80);
        camera3d.lookAt(0, 15, 0);
        autoRotate = true;
    }

    function toggle3DAnimation() {
        isAnimating = !isAnimating;
        autoRotate = isAnimating;
        
        const btn = document.getElementById('toggle-animation-btn');
        if (btn) {
            const icon = btn.querySelector('i');
            if (icon) {
                icon.className = isAnimating ? 'fas fa-pause' : 'fas fa-play';
            }
        }
    }

    function cleanup3DScene() {
        if (animationId3d) {
            cancelAnimationFrame(animationId3d);
            animationId3d = null;
        }
        
        if (renderer3d) {
            renderer3d.dispose();
            renderer3d = null;
        }
        
        // Remove tooltip
        if (tooltip3d) {
            tooltip3d.remove();
            tooltip3d = null;
        }
        
        // Clear axis labels
        axisLabels3d.forEach(label => {
            if (label && scene3d) {
                scene3d.remove(label);
            }
        });
        axisLabels3d = [];
        
        candlesticks3d = [];
        scene3d = null;
        camera3d = null;
        raycaster3d = null;
        hoveredCandlestick = null;
        chartOffsetX = 0;
        minPrice3d = 0;
        maxPrice3d = 0;
        priceRange3d = 1;
    }

    // ==========================================
    // END 3D CHART FUNCTIONS
    // ==========================================

    // Setup manual pan functionality for the chart
    function setupChartPan(chart) {
        if (!chart || !chart.canvas) return;
        
        const canvas = chart.canvas;
        
        // Remove existing handlers if any
        if (panState.boundHandlers) {
            canvas.removeEventListener('mousedown', panState.boundHandlers.mousedown);
            document.removeEventListener('mousemove', panState.boundHandlers.mousemove);
            document.removeEventListener('mouseup', panState.boundHandlers.mouseup);
        }
        
        // Ensure canvas has proper styles for interaction
        canvas.style.cursor = 'grab';
        canvas.style.touchAction = 'none';
        canvas.style.userSelect = 'none';
        
        // Create bound handlers
        const handleMouseDown = (e) => {
            // Only respond to left mouse button
            if (e.button !== 0) return;
            
            panState.isPanning = true;
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
        
        // Store handlers for cleanup
        panState.boundHandlers = {
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
                
                // Show/hide Fibonacci levels panel
                if (indicator === 'fibonacci') {
                    const fibPanel = document.getElementById('fibonacci-levels');
                    if (fibPanel) {
                        fibPanel.style.display = e.target.checked ? 'block' : 'none';
                    }
                }
                
                // Only render if we have valid chart data
                if (chartData && chartData.candles && chartData.candles.length > 0) {
                    try {
                        renderChart();
                    } catch (error) {
                        console.error('Error rendering chart with indicator:', indicator, error);
                        // Revert the checkbox state on error
                        e.target.checked = !e.target.checked;
                        activeIndicators[indicator] = !activeIndicators[indicator];
                        showChartError('Failed to add indicator. Please try again.', currentSymbol);
                    }
                }
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

        // 2D/3D View Toggle
        document.querySelectorAll('.view-toggle-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.view-toggle-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                currentView = btn.dataset.view;
                toggleChartView();
            });
        });

        // 3D Chart Controls
        document.getElementById('rotate-left-btn')?.addEventListener('click', () => rotate3DChart(-0.5));
        document.getElementById('rotate-right-btn')?.addEventListener('click', () => rotate3DChart(0.5));
        document.getElementById('reset-3d-btn')?.addEventListener('click', reset3DView);
        document.getElementById('toggle-animation-btn')?.addEventListener('click', toggle3DAnimation);
    }

    async function loadChart(symbol, forceRefresh = false) {
        if (!symbol) return;
        
        currentSymbol = symbol;
        showLoading(true);
        hideEmpty();

        try {
            // Fetch chart data with refresh parameter if forceRefresh is true
            const refreshParam = forceRefresh ? '&refresh=true' : '';
            const chartResponse = await fetch(`api/charts.php?action=chart&symbol=${symbol}&timeframe=${currentTimeframe}${refreshParam}`);
            
            if (!chartResponse.ok) {
                throw new Error(`HTTP error! status: ${chartResponse.status}`);
            }
            
            const responseText = await chartResponse.text();
            
            // Check if response is empty
            if (!responseText || responseText.trim() === '') {
                throw new Error('Empty response from server');
            }
            
            // Parse JSON
            try {
                chartData = JSON.parse(responseText);
            } catch (parseError) {
                console.error('JSON parse error:', parseError, 'Response:', responseText.substring(0, 200));
                throw new Error('Invalid response format from server');
            }
            
            if (chartData.success && chartData.candles && chartData.candles.length > 0) {
                // Hide error display first
                hideEmpty();
                
                // Use chart data for all price displays (ensures consistency)
                updateSymbolInfoFromChart(chartData);
                updateStatistics(chartData.statistics);
                updateIndicatorValues(chartData.indicators);
                updateSignals(chartData.indicators);
                updateFibonacciDisplay(chartData.indicators);
                
                // Render chart with error handling
                try {
                    renderChart();
                    
                    // If currently in 3D view, re-render 3D chart with new timeframe data
                    if (currentView === '3d') {
                        console.log('Re-rendering 3D chart with new timeframe data...');
                        setTimeout(() => {
                            render3DChart();
                        }, 100);
                    }
                } catch (renderError) {
                    console.error('Chart render error:', renderError);
                    showChartError('Chart rendering failed. Please try again.', symbol);
                    return;
                }
                
                // Show notification about data source
                if (chartData.source) {
                    const sourceName = chartData.source === 'finnhub' ? 'Finnhub' : 'Yahoo Finance';
                    console.log(`Chart data loaded from ${sourceName} for timeframe: ${currentTimeframe}`);
                }
                
                // Auto-update Fibonacci Calculator with the same symbol
                if (typeof FibCalcModule !== 'undefined' && FibCalcModule.calculateForSymbol) {
                    FibCalcModule.calculateForSymbol(symbol);
                }
            } else {
                // Show error message
                const errorMsg = chartData.message || 'Unable to fetch real-time market data';
                showChartError(errorMsg, symbol);
            }
        } catch (error) {
            console.error('Chart loading error:', error);
            showChartError(`Error: ${error.message}`, symbol);
        } finally {
            showLoading(false);
        }
    }

    function showChartError(message, symbol) {
        const chartEmpty = document.getElementById('chart-empty');
        
        if (chartEmpty) {
            chartEmpty.style.display = 'flex';
            chartEmpty.innerHTML = `
                <i class="fas fa-exclamation-triangle" style="color: #ef4444;"></i>
                <p style="color: #ef4444;">${message}</p>
                <p style="font-size: 0.8rem; color: var(--text-secondary);">Symbol: ${symbol}</p>
                <button class="btn btn-primary btn-sm" onclick="ChartsModule.loadChart('${symbol}')" style="margin-top: 10px;">
                    <i class="fas fa-redo"></i> Retry
                </button>
            `;
        }
        
        // Update header to show symbol with no data
        const tickerEl = document.getElementById('chart-ticker');
        const companyEl = document.getElementById('chart-company');
        const priceEl = document.getElementById('chart-current-price');
        const changeEl = document.getElementById('chart-price-change');
        
        if (tickerEl) tickerEl.textContent = symbol;
        if (companyEl) companyEl.textContent = '(No Data)';
        if (priceEl) priceEl.textContent = '--';
        if (changeEl) changeEl.textContent = '--';
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

    // Use chart data for symbol info to ensure price consistency
    function updateSymbolInfoFromChart(data) {
        if (!data || !data.statistics) return;
        
        const stats = data.statistics;
        const candles = data.candles || [];
        
        // Calculate change from chart data
        const currentPrice = stats.close;
        const openPrice = stats.open;
        const change = currentPrice - openPrice;
        const changePercent = openPrice > 0 ? ((change / openPrice) * 100) : 0;
        
        // Determine data source label
        let sourceLabel = '';
        if (data.source === 'finnhub') {
            sourceLabel = '(Finnhub)';
        } else if (data.source === 'yahoo') {
            sourceLabel = '(Yahoo Finance)';
        }
        
        // Main header info
        document.getElementById('chart-ticker').textContent = data.symbol;
        document.getElementById('chart-company').textContent = sourceLabel;
        document.getElementById('chart-current-price').textContent = formatPrice(currentPrice);
        
        const changeEl = document.getElementById('chart-price-change');
        changeEl.textContent = `${change >= 0 ? '+' : ''}${formatPrice(change)} (${changePercent >= 0 ? '+' : ''}${changePercent.toFixed(2)}%)`;
        changeEl.className = `price-change ${change >= 0 ? 'positive' : 'negative'}`;

        // Update detail row items using chart statistics
        updateDetailElement('detail-open', formatPrice(stats.open));
        updateDetailElement('detail-high', formatPrice(stats.high), 'positive');
        updateDetailElement('detail-low', formatPrice(stats.low), 'negative');
        updateDetailElement('detail-prev-close', formatPrice(stats.open)); // Use open as approximation
        updateDetailElement('detail-volume', formatVolume(stats.volume || 0));
        updateDetailElement('detail-day-range', `${formatPriceShort(stats.low)} - ${formatPriceShort(stats.high)}`);
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
        if (indicators.fibonacci) {
            // Show key Fib level (0.618 Golden Ratio)
            const fibValue = document.getElementById('value-fib');
            if (fibValue && indicators.fibonacci.levels) {
                fibValue.textContent = `0.618: ${formatPrice(indicators.fibonacci.levels['618'])}`;
            }
        }
        if (chartData?.statistics?.volume) {
            document.getElementById('value-volume').textContent = formatVolume(chartData.statistics.volume);
        }
    }

    function updateFibonacciDisplay(indicators) {
        if (!indicators.fibonacci) return;
        
        const fib = indicators.fibonacci;
        
        // Update Fibonacci retracement levels
        if (fib.levels) {
            updateElementText('fib-0', formatPrice(fib.levels['0']));
            updateElementText('fib-236', formatPrice(fib.levels['236']));
            updateElementText('fib-382', formatPrice(fib.levels['382']));
            updateElementText('fib-50', formatPrice(fib.levels['50']));
            updateElementText('fib-618', formatPrice(fib.levels['618']));
            updateElementText('fib-786', formatPrice(fib.levels['786']));
            updateElementText('fib-100', formatPrice(fib.levels['100']));
        }
        
        // Update Fibonacci extension levels
        if (fib.extensions) {
            updateElementText('fib-1618', formatPrice(fib.extensions['1618']));
            updateElementText('fib-2618', formatPrice(fib.extensions['2618']));
            updateElementText('fib-4236', formatPrice(fib.extensions['4236']));
        }
        
        // Update Fibonacci spiral targets
        if (fib.spiralTargets) {
            updateElementText('fib-target-1', formatPrice(fib.spiralTargets.target1));
            updateElementText('fib-target-2', formatPrice(fib.spiralTargets.target2));
            updateElementText('fib-support-1', formatPrice(fib.spiralTargets.support1));
            updateElementText('fib-support-2', formatPrice(fib.spiralTargets.support2));
        }
    }

    function updateElementText(id, text) {
        const el = document.getElementById(id);
        if (el) el.textContent = text;
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
        if (!chartData || !chartData.candles || chartData.candles.length === 0) {
            console.error('No chart data available for rendering');
            return;
        }
        
        const canvas = document.getElementById('main-chart');
        if (!canvas) {
            console.error('Chart canvas not found');
            return;
        }
        
        const ctx = canvas.getContext('2d');
        
        // Destroy existing chart and clear canvas
        if (mainChart) {
            mainChart.destroy();
            mainChart = null;
        }
        
        // Clear canvas to prevent rendering artifacts
        // Get actual canvas dimensions
        const rect = canvas.getBoundingClientRect();
        canvas.width = rect.width;
        canvas.height = rect.height;
        ctx.clearRect(0, 0, canvas.width, canvas.height);

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

        // Add indicator overlays - ensure data exists and is valid
        if (activeIndicators.sma20 && chartData.indicators && chartData.indicators.sma20 && chartData.indicators.sma20.values && Array.isArray(chartData.indicators.sma20.values) && chartData.indicators.sma20.values.length > 0) {
            const smaData = padIndicatorData(chartData.indicators.sma20.values, candles.length);
            datasets.push({
                label: 'SMA 20',
                data: smaData,
                borderColor: colors.sma20,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [],
                tension: 0.1,
                pointRadius: 0,
                yAxisID: 'y'
            });
        }

        if (activeIndicators.sma50 && chartData.indicators && chartData.indicators.sma50 && chartData.indicators.sma50.values && Array.isArray(chartData.indicators.sma50.values) && chartData.indicators.sma50.values.length > 0) {
            const smaData = padIndicatorData(chartData.indicators.sma50.values, candles.length);
            datasets.push({
                label: 'SMA 50',
                data: smaData,
                borderColor: colors.sma50,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [],
                tension: 0.1,
                pointRadius: 0,
                yAxisID: 'y'
            });
        }

        if (activeIndicators.ema12 && chartData.indicators && chartData.indicators.ema12 && chartData.indicators.ema12.values && Array.isArray(chartData.indicators.ema12.values) && chartData.indicators.ema12.values.length > 0) {
            const emaData = padIndicatorData(chartData.indicators.ema12.values, candles.length);
            datasets.push({
                label: 'EMA 12',
                data: emaData,
                borderColor: colors.ema12,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [5, 5],
                tension: 0.1,
                pointRadius: 0,
                yAxisID: 'y'
            });
        }

        if (activeIndicators.ema26 && chartData.indicators && chartData.indicators.ema26 && chartData.indicators.ema26.values && Array.isArray(chartData.indicators.ema26.values) && chartData.indicators.ema26.values.length > 0) {
            const emaData = padIndicatorData(chartData.indicators.ema26.values, candles.length);
            datasets.push({
                label: 'EMA 26',
                data: emaData,
                borderColor: colors.ema26,
                backgroundColor: 'transparent',
                borderWidth: 1.5,
                borderDash: [5, 5],
                tension: 0.1,
                pointRadius: 0,
                yAxisID: 'y'
            });
        }

        // Bollinger Bands
        if (activeIndicators.bollinger && chartData.indicators && chartData.indicators.bollinger && typeof chartData.indicators.bollinger === 'object' && chartData.indicators.bollinger.upper !== undefined) {
            const bb = chartData.indicators.bollinger;
            const bbUpper = Array(candles.length).fill(bb.upper);
            const bbLower = Array(candles.length).fill(bb.lower);
            
            datasets.push({
                label: 'BB Upper',
                data: bbUpper,
                borderColor: colors.bollingerUpper,
                backgroundColor: 'transparent',
                borderWidth: 1,
                borderDash: [3, 3],
                tension: 0,
                pointRadius: 0,
                yAxisID: 'y'
            });
            datasets.push({
                label: 'BB Lower',
                data: bbLower,
                borderColor: colors.bollingerLower,
                backgroundColor: 'transparent',
                borderWidth: 1,
                borderDash: [3, 3],
                tension: 0,
                pointRadius: 0,
                yAxisID: 'y'
            });
        }

        // Fibonacci Retracement and Extension Levels
        if (activeIndicators.fibonacci && chartData.indicators && chartData.indicators.fibonacci && typeof chartData.indicators.fibonacci === 'object' && chartData.indicators.fibonacci.levels) {
            const fib = chartData.indicators.fibonacci;
            
            // Retracement levels (from high to low)
            const retracementLevels = [
                { level: '0', color: colors.fib0, label: 'Fib 0.0', isKey: true },
                { level: '236', color: colors.fib236, label: 'Fib 0.236', isKey: false },
                { level: '382', color: colors.fib382, label: 'Fib 0.382', isKey: false },
                { level: '50', color: colors.fib50, label: 'Fib 0.5', isKey: false },
                { level: '618', color: colors.fib618, label: 'Fib 0.618', isKey: true },
                { level: '786', color: colors.fib786, label: 'Fib 0.786', isKey: false },
                { level: '100', color: colors.fib100, label: 'Fib 1.0', isKey: true }
            ];
            
            // Extension levels (above high)
            const extensionLevels = [
                { level: '1618', color: colors.fib1618, label: 'Fib 1.618', isKey: true },
                { level: '2618', color: colors.fib2618, label: 'Fib 2.618', isKey: true },
                { level: '4236', color: colors.fib4236, label: 'Fib 4.236', isKey: false }
            ];
            
            // Add retracement levels
            retracementLevels.forEach(({ level, color, label, isKey }) => {
                const price = fib.levels[level];
                if (price !== undefined) {
                    datasets.push({
                        label: label,
                        data: Array(candles.length).fill(price),
                        borderColor: color,
                        backgroundColor: 'transparent',
                        borderWidth: isKey ? 2 : 1,
                        borderDash: isKey ? [] : [5, 5],
                        tension: 0,
                        pointRadius: 0,
                        yAxisID: 'y'
                    });
                }
            });
            
            // Add extension levels
            if (fib.extensions) {
                extensionLevels.forEach(({ level, color, label, isKey }) => {
                    const price = fib.extensions[level];
                    if (price !== undefined && !isNaN(price)) {
                        datasets.push({
                            label: label,
                            data: Array(candles.length).fill(price),
                            borderColor: color,
                            backgroundColor: 'transparent',
                            borderWidth: isKey ? 2 : 1,
                            borderDash: isKey ? [] : [5, 5],
                            tension: 0,
                            pointRadius: 0,
                            yAxisID: 'y'
                        });
                    }
                });
            }
        }

        // Ensure we have at least one dataset (the main price data)
        if (datasets.length === 0) {
            console.error('No datasets to render');
            showChartError('No data available to render chart.', currentSymbol);
            return;
        }
        
        // Validate that all datasets have valid data
        for (let i = 0; i < datasets.length; i++) {
            const dataset = datasets[i];
            if (!dataset.data || !Array.isArray(dataset.data) || dataset.data.length === 0) {
                console.error(`Dataset ${i} (${dataset.label}) has invalid data:`, dataset);
                // Remove invalid dataset
                datasets.splice(i, 1);
                i--;
            }
        }
        
        if (datasets.length === 0) {
            console.error('All datasets were invalid');
            showChartError('Invalid chart data. Please try again.', currentSymbol);
            return;
        }
        
        try {
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
                            title: function(context) {
                                // CRITICAL: Format timestamp in EST timezone to show correct current time
                                // The timestamp from the API is in UTC milliseconds - we format it for EST display
                                if (context.length > 0 && context[0].parsed) {
                                    const candle = candles[context[0].dataIndex];
                                    if (candle && candle.time) {
                                        // candle.time is UTC timestamp in milliseconds
                                        const date = new Date(candle.time);
                                        
                                        // Verify date is valid
                                        if (isNaN(date.getTime())) {
                                            console.warn('Invalid timestamp:', candle.time);
                                            return '';
                                        }
                                        
                                        // Format in EST timezone - this correctly converts UTC to EST for display
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
                                        
                                        return estString + ' EST';
                                    }
                                }
                                return '';
                            },
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
                            mode: 'xy'
                        },
                        pan: {
                            enabled: false // Using custom pan implementation
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
                            },
                            // CRITICAL: Use EST timezone for all time displays
                            // Note: date-fns adapter may need timezone configuration
                            parser: function(value) {
                                // Parse timestamp and return as-is (timestamps are already in milliseconds)
                                return value;
                            },
                            tooltipFormat: 'MMM d, yyyy h:mm:ss a',
                            unit: currentTimeframe === '15MIN' ? 'minute' : 
                                  currentTimeframe === '1H' ? 'hour' : 
                                  currentTimeframe === '4H' ? 'hour' : 'day'
                        },
                        grid: {
                            color: colors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: colors.text,
                            maxRotation: 0,
                            padding: 8,
                            // CRITICAL: Custom callback to format times in EST timezone
                            callback: function(value, index, ticks) {
                                if (value === null || value === undefined) return '';
                                const date = new Date(value);
                                // Format in EST timezone
                                if (currentTimeframe === '1D' || currentTimeframe === '15MIN') {
                                    return date.toLocaleTimeString('en-US', {
                                        timeZone: 'America/New_York',
                                        hour: '2-digit',
                                        minute: '2-digit',
                                        hour12: false
                                    });
                                } else if (currentTimeframe === '1H' || currentTimeframe === '4H') {
                                    return date.toLocaleString('en-US', {
                                        timeZone: 'America/New_York',
                                        month: 'short',
                                        day: 'numeric',
                                        hour: '2-digit',
                                        minute: '2-digit',
                                        hour12: false
                                    });
                                } else {
                                    return date.toLocaleDateString('en-US', {
                                        timeZone: 'America/New_York',
                                        month: 'short',
                                        day: 'numeric'
                                    });
                                }
                            }
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
        } catch (error) {
            console.error('Error creating chart:', error);
            showChartError('Failed to render chart. Please try again.', currentSymbol);
            return;
        }

        // Store original scale ranges for reset functionality and setup pan
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

            // Setup manual pan (only once per canvas)
            setupChartPan(mainChart);
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

        // Also update 3D view if active
        if (currentView === '3d') {
            render3DChart();
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
        if (!values || !Array.isArray(values) || values.length === 0) {
            console.warn('Invalid indicator values, returning empty array');
            return Array(targetLength).fill(null);
        }
        
        // Filter out any null/undefined/NaN values and ensure all are numbers
        const validValues = values.filter(v => v !== null && v !== undefined && !isNaN(v)).map(v => parseFloat(v));
        
        if (validValues.length === 0) {
            console.warn('No valid indicator values after filtering');
            return Array(targetLength).fill(null);
        }
        
        const padding = Math.max(0, targetLength - validValues.length);
        const padded = [...Array(padding).fill(null), ...validValues];
        
        // Ensure the result matches the target length exactly
        if (padded.length < targetLength) {
            padded.push(...Array(targetLength - padded.length).fill(null));
        } else if (padded.length > targetLength) {
            return padded.slice(-targetLength);
        }
        
        return padded;
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
        if (dropdown) {
            dropdown.classList.remove('open');
        }
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
        const modal = document.getElementById('load-study-modal');
        if (modal) {
            modal.classList.remove('active');
        }
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
            showLoading(true);
            
            const response = await fetch(`api/charts.php?action=study&id=${id}`);
            
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const data = await response.json();
            
            if (data.success && data.study) {
                const study = data.study;
                
                // Set timeframe
                currentTimeframe = study.timeframe || '1D';
                document.querySelectorAll('.timeframe-btn').forEach(btn => {
                    btn.classList.toggle('active', btn.dataset.timeframe === currentTimeframe);
                });
                
                // Set chart type
                currentChartType = study.chart_type || 'candlestick';
                document.querySelectorAll('.chart-type-btn').forEach(btn => {
                    btn.classList.toggle('active', btn.dataset.type === currentChartType);
                });
                
                // Set indicators
                if (study.indicators && typeof study.indicators === 'object') {
                    Object.entries(study.indicators).forEach(([key, value]) => {
                        if (activeIndicators.hasOwnProperty(key)) {
                            activeIndicators[key] = value;
                            const checkbox = document.querySelector(`[data-indicator="${key}"]`);
                            if (checkbox) checkbox.checked = value;
                            
                            // Show/hide Fibonacci panel if needed
                            if (key === 'fibonacci') {
                                const fibPanel = document.getElementById('fibonacci-levels');
                                if (fibPanel) fibPanel.style.display = value ? 'block' : 'none';
                            }
                        }
                    });
                }
                
                // Update symbol input
                const symbolInput = document.getElementById('chart-symbol-input');
                if (symbolInput) {
                    symbolInput.value = study.symbol;
                }
                
                // Close any open modals/dropdowns first
                closeLoadStudyModal();
                closeStudiesDropdown();
                
                // Load chart with the study's symbol
                await loadChart(study.symbol);
                
                showNotification(`Loaded study: ${study.name}`, 'success');
            } else {
                showNotification(data.message || 'Failed to load study', 'error');
            }
        } catch (error) {
            console.error('Error loading study:', error);
            showNotification(`Error loading study: ${error.message}`, 'error');
        } finally {
            showLoading(false);
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

    // Listen for API source changes and reload chart
    window.addEventListener('apiSourceChanged', function(event) {
        if (currentSymbol) {
            console.log('API source changed, reloading chart with new source:', event.detail.source);
            loadChart(currentSymbol, true); // Force refresh to use new API
        }
    });

    // Public API
    return {
        loadChart,
        loadStudyById,
        reloadChart: function(forceRefresh = true) {
            if (currentSymbol) {
                loadChart(currentSymbol, forceRefresh);
            }
        },
        init
    };
})();

