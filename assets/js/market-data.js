// Market Data Management
let marketDataInterval = null;
let currentApiSource = localStorage.getItem('apiSource') || 'finnhub';

// Initialize API source selector
function initApiSelector() {
    const apiSelect = document.getElementById('api-source');
    if (apiSelect) {
        apiSelect.value = currentApiSource;
        apiSelect.addEventListener('change', function() {
            currentApiSource = this.value;
            localStorage.setItem('apiSource', currentApiSource);
            fetchMarketData();
            updateApiStatus();
        });
    }
}

// Update API status display
async function updateApiStatus() {
    try {
        const response = await fetch('api/get_api_status.php');
        if (!response.ok) return;
        
        const status = await response.json();
        const currentStatus = status[currentApiSource];
        
        const apiStatusEl = document.getElementById('api-status');
        const rateLimitEl = document.getElementById('rate-limit');
        
        if (apiStatusEl && currentStatus) {
            if (currentStatus.available) {
                apiStatusEl.textContent = 'Available';
                apiStatusEl.className = 'api-status available';
            } else {
                apiStatusEl.textContent = 'Rate Limited';
                apiStatusEl.className = 'api-status limited';
            }
        }
        
        if (rateLimitEl && currentStatus) {
            rateLimitEl.textContent = `${currentStatus.remaining}/${currentStatus.limit} calls/min`;
        }
    } catch (error) {
        console.error('Error fetching API status:', error);
    }
}

// Fetch market data from API
async function fetchMarketData() {
    const marketGrid = document.getElementById('market-grid');
    if (!marketGrid) return;
    
    marketGrid.innerHTML = '<div class="loading"><i class="fas fa-spinner fa-spin"></i> Loading market data...</div>';
    
    try {
        const response = await fetch(`api/get_market_data.php?source=${currentApiSource}`);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const result = await response.json();
        
        if (result.success === false) {
            throw new Error(result.message || 'Failed to fetch market data');
        }
        
        if (result.data && Array.isArray(result.data)) {
            displayMarketData(result.data);
            updateDataInfo(result);
            updateApiStatus();
        } else {
            throw new Error('Invalid data format received');
        }
    } catch (error) {
        console.error('Error fetching market data:', error);
        if (marketGrid) {
            marketGrid.innerHTML = `
                <div class="loading error-message">
                    <i class="fas fa-exclamation-triangle"></i>
                    <p>Error loading market data: ${error.message}</p>
                    <button class="btn btn-secondary btn-sm" onclick="fetchMarketData()" style="margin-top: 10px;">
                        <i class="fas fa-redo"></i> Retry
                    </button>
                </div>
            `;
        }
    }
}

// Update data info display
function updateDataInfo(result) {
    const currentSourceEl = document.getElementById('current-source');
    const lastUpdateEl = document.getElementById('last-update');
    
    if (currentSourceEl) {
        const sourceName = result.source === 'finnhub' ? 'Finnhub' : 
                          result.source === 'yahoo' ? 'Yahoo Finance' : result.source;
        currentSourceEl.textContent = sourceName;
    }
    
    if (lastUpdateEl) {
        const now = new Date();
        lastUpdateEl.textContent = now.toLocaleTimeString();
    }
}

// Display market data cards
function displayMarketData(data) {
    const marketGrid = document.getElementById('market-grid');
    if (!marketGrid) return;

    if (!data || data.length === 0) {
        marketGrid.innerHTML = `
            <div class="loading">
                <i class="fas fa-info-circle"></i>
                <p>No market data available.</p>
                <button class="btn btn-primary btn-sm" onclick="openAddSymbolModal()" style="margin-top: 10px;">
                    <i class="fas fa-plus"></i> Add Symbol
                </button>
            </div>
        `;
        return;
    }

    marketGrid.innerHTML = data.map(item => createMarketCard(item)).join('');
    
    // Attach delete button event listeners
    document.querySelectorAll('.delete-symbol-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            const symbol = this.dataset.symbol;
            deleteSymbol(symbol);
        });
    });
}

// Create market card HTML
function createMarketCard(data) {
    // Safely extract and parse all values
    const price = parseFloat(data.price) || 0;
    const change = parseFloat(data.change) || 0;
    const changePercent = parseFloat(data.changePercent) || 0;
    
    const changeClass = change >= 0 ? 'positive' : 'negative';
    const changeSign = change >= 0 ? '+' : '';
    
    // Format price with exactly 2 decimal places
    const priceDisplay = price > 0 ? `$${price.toFixed(2)}` : 'N/A';
    const hasError = data.error || !price || price === 0;
    
    // Safely extract values with proper fallbacks - ensure they're numbers
    const open = (data.open !== undefined && data.open !== null && !isNaN(data.open) && parseFloat(data.open) > 0) 
        ? parseFloat(data.open) 
        : price;
    const high = (data.high !== undefined && data.high !== null && !isNaN(data.high) && parseFloat(data.high) > 0) 
        ? parseFloat(data.high) 
        : price;
    const low = (data.low !== undefined && data.low !== null && !isNaN(data.low) && parseFloat(data.low) > 0) 
        ? parseFloat(data.low) 
        : price;
    const prevClose = (data.previousClose !== undefined && data.previousClose !== null && !isNaN(data.previousClose) && parseFloat(data.previousClose) > 0) 
        ? parseFloat(data.previousClose) 
        : ((open > 0 && open !== price) ? open : price);
    
    // Format day range - use low/high if valid, otherwise use price
    const dayRangeLow = (low > 0) ? low : price;
    const dayRangeHigh = (high > 0) ? high : price;
    const dayRange = `${formatPriceShort(dayRangeLow)} - ${formatPriceShort(dayRangeHigh)}`;
    
    // Use day range as fallback for 52W range if not available
    const yearRange = (data.high52w && data.low52w && 
                      !isNaN(data.high52w) && !isNaN(data.low52w) && 
                      parseFloat(data.high52w) > 0 && parseFloat(data.low52w) > 0 && 
                      (parseFloat(data.high52w) !== high || parseFloat(data.low52w) !== low)) 
        ? `${formatPriceShort(parseFloat(data.low52w))} - ${formatPriceShort(parseFloat(data.high52w))}` 
        : dayRange;
    
    // Parse volume values
    const volume = (data.volume !== undefined && data.volume !== null && !isNaN(data.volume)) 
        ? parseInt(data.volume) 
        : 0;
    const avgVol = (data.avgVolume !== undefined && data.avgVolume !== null && !isNaN(data.avgVolume) && parseFloat(data.avgVolume) > 0) 
        ? parseInt(data.avgVolume) 
        : (volume > 0 ? volume : 0);
    
    // Format change percentage with proper sign and formatting
    const changePercentFormatted = isNaN(changePercent) ? '0.00' : Math.abs(changePercent).toFixed(2);
    const changeFormatted = isNaN(change) ? '0.00' : Math.abs(change).toFixed(2);
    
    const sourceLabel = data.source === 'finnhub' ? 'Finnhub' : data.source === 'yahoo' ? 'Yahoo Finance' : (data.source || 'Unknown');
    
    return `
        <div class="market-card ${hasError ? 'card-error' : ''}">
            <button class="delete-symbol-btn" data-symbol="${data.symbol}" title="Remove symbol">
                <i class="fas fa-times"></i>
            </button>
            <div class="market-card-top">
                <div class="market-symbol-section">
                    <div class="market-symbol-large">${data.symbol}</div>
                    ${data.source ? `<div class="market-source-label">${sourceLabel}</div>` : ''}
                </div>
                <div class="market-price-section">
                    <div class="market-price-large">${priceDisplay}</div>
                    ${!hasError ? `
                    <div class="market-change-box ${changeClass}">
                        ${changeSign}$${changeFormatted} (${changeSign}${changePercentFormatted}%)
                    </div>
                    ` : ''}
                </div>
            </div>
            ${!hasError ? `
            <div class="market-metrics-grid">
                <div class="market-metric">
                    <span class="metric-label">OPEN</span>
                    <span class="metric-value">${formatPriceShort(open)}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">HIGH</span>
                    <span class="metric-value positive">${formatPriceShort(high)}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">LOW</span>
                    <span class="metric-value negative">${formatPriceShort(low)}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">PREV</span>
                    <span class="metric-value">${formatPriceShort(prevClose)}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">VOLUME</span>
                    <span class="metric-value">${formatNumber(volume)}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">DAY</span>
                    <span class="metric-value">${dayRange}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">52W</span>
                    <span class="metric-value">${yearRange}</span>
                </div>
                <div class="market-metric">
                    <span class="metric-label">AVG VOL</span>
                    <span class="metric-value">${formatNumber(avgVol)}</span>
                </div>
            </div>
            ` : `
            <div class="error-info">
                <i class="fas fa-exclamation-triangle"></i>
                <span>Data temporarily unavailable</span>
            </div>
            `}
        </div>
    `;
}

function formatPriceShort(price) {
    if (price === null || price === undefined) return '--';
    const numPrice = parseFloat(price);
    if (isNaN(numPrice)) return '--';
    // Allow 0 as a valid price (some securities can have 0 price)
    if (numPrice < 0) return '--';
    // Always format with 2 decimal places
    return `$${numPrice.toFixed(2)}`;
}

// Format large numbers
function formatNumber(num) {
    if (num === null || num === undefined || isNaN(num)) return '--';
    const numValue = parseInt(num);
    if (isNaN(numValue)) return '--';
    if (numValue === 0) return '0';
    if (numValue >= 1000000000) {
        return (numValue / 1000000000).toFixed(2) + 'B';
    } else if (numValue >= 1000000) {
        return (numValue / 1000000).toFixed(2) + 'M';
    } else if (numValue >= 1000) {
        return (numValue / 1000).toFixed(2) + 'K';
    }
    return numValue.toLocaleString();
}

// Add new symbol
async function addSymbol(symbol) {
    if (!symbol || symbol.trim() === '') {
        alert('Please enter a valid symbol');
        return;
    }

    try {
        const response = await fetch('api/add_symbol.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ symbol: symbol.toUpperCase().trim() })
        });

        const result = await response.json();
        
        if (result.success) {
            await fetchMarketData();
            closeAddSymbolModal();
        } else {
            alert(result.message || 'Error adding symbol');
        }
    } catch (error) {
        console.error('Error adding symbol:', error);
        alert('Error adding symbol. Please try again.');
    }
}

// Delete symbol
async function deleteSymbol(symbol) {
    if (!confirm(`Are you sure you want to remove ${symbol}?`)) {
        return;
    }

    try {
        const response = await fetch('api/delete_symbol.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ symbol: symbol })
        });

        const result = await response.json();
        
        if (result.success) {
            await fetchMarketData();
        } else {
            alert(result.message || 'Error deleting symbol');
        }
    } catch (error) {
        console.error('Error deleting symbol:', error);
        alert('Error deleting symbol. Please try again.');
    }
}

// Modal functions
function openAddSymbolModal() {
    const modalElement = document.getElementById('add-symbol-modal');
    if (modalElement) {
        modalElement.classList.add('active');
        const input = document.getElementById('symbol-input');
        if (input) {
            input.value = '';
            setTimeout(() => input.focus(), 100);
        }
    }
}

function closeAddSymbolModal() {
    const modalElement = document.getElementById('add-symbol-modal');
    if (modalElement) {
        modalElement.classList.remove('active');
    }
}

// Initialize market data
function initMarketData() {
    initApiSelector();
    fetchMarketData();
    updateApiStatus();
    
    // Update every 60 seconds to respect rate limits
    marketDataInterval = setInterval(() => {
        fetchMarketData();
        updateApiStatus();
    }, 60000);
}

// Setup event listeners
function setupMarketDataListeners() {
    // Add symbol button
    const addSymbolBtn = document.getElementById('add-symbol-btn');
    if (addSymbolBtn) {
        addSymbolBtn.addEventListener('click', openAddSymbolModal);
    }

    // Refresh button
    const refreshBtn = document.getElementById('refresh-market-btn');
    if (refreshBtn) {
        refreshBtn.addEventListener('click', function() {
            this.classList.add('spinning');
            fetchMarketData().then(() => {
                setTimeout(() => this.classList.remove('spinning'), 500);
            });
        });
    }

    // Modal close button
    const closeBtn = document.getElementById('close-symbol-modal');
    if (closeBtn) {
        closeBtn.addEventListener('click', closeAddSymbolModal);
    }

    // Modal cancel button
    const cancelBtn = document.getElementById('cancel-symbol-btn');
    if (cancelBtn) {
        cancelBtn.addEventListener('click', function() {
            closeAddSymbolModal();
            const input = document.getElementById('symbol-input');
            if (input) input.value = '';
        });
    }

    // Save symbol button
    const saveSymbolBtn = document.getElementById('save-symbol-btn');
    if (saveSymbolBtn) {
        saveSymbolBtn.addEventListener('click', function() {
            const symbolInput = document.getElementById('symbol-input');
            if (symbolInput && symbolInput.value) {
                addSymbol(symbolInput.value).then(() => {
                    closeAddSymbolModal();
                });
            }
        });
    }

    // Enter key support
    const symbolInput = document.getElementById('symbol-input');
    if (symbolInput) {
        symbolInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                if (this.value) {
                    addSymbol(this.value).then(() => {
                        closeAddSymbolModal();
                    });
                }
            }
        });
    }

    // Close modal on outside click
    const modalElement = document.getElementById('add-symbol-modal');
    if (modalElement) {
        modalElement.addEventListener('click', function(e) {
            if (e.target === modalElement) {
                closeAddSymbolModal();
            }
        });
    }

    // Close modal on Escape key
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') {
            const modalElement = document.getElementById('add-symbol-modal');
            if (modalElement && modalElement.classList.contains('active')) {
                closeAddSymbolModal();
            }
        }
    });
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
        initMarketData();
        setupMarketDataListeners();
    });
} else {
    initMarketData();
    setupMarketDataListeners();
}
