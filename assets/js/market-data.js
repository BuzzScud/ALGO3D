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
    const changeClass = data.change >= 0 ? 'positive' : 'negative';
    const changeSign = data.change >= 0 ? '+' : '';
    const priceDisplay = data.price > 0 ? `$${data.price.toFixed(2)}` : 'N/A';
    const hasError = data.error || data.price === 0;
    
    return `
        <div class="market-card ${hasError ? 'card-error' : ''}">
            <button class="delete-symbol-btn" data-symbol="${data.symbol}" title="Remove symbol">
                <i class="fas fa-times"></i>
            </button>
            <div class="market-card-header">
                <div class="market-symbol-wrapper">
                    <div class="market-symbol">${data.symbol}</div>
                    ${data.source ? `<span class="source-badge">${data.source}</span>` : ''}
                </div>
            </div>
            <div class="market-price">${priceDisplay}</div>
            ${!hasError ? `
            <div class="market-change ${changeClass}">
                <i class="fas fa-${data.change >= 0 ? 'arrow-up' : 'arrow-down'}"></i>
                <span>${changeSign}${data.change.toFixed(2)} (${changeSign}${data.changePercent.toFixed(2)}%)</span>
            </div>
            <div class="market-details">
                <div class="market-detail-item">
                    <span class="market-detail-label">Open</span>
                    <span class="market-detail-value">$${data.open.toFixed(2)}</span>
                </div>
                <div class="market-detail-item">
                    <span class="market-detail-label">High</span>
                    <span class="market-detail-value">$${data.high.toFixed(2)}</span>
                </div>
                <div class="market-detail-item">
                    <span class="market-detail-label">Low</span>
                    <span class="market-detail-value">$${data.low.toFixed(2)}</span>
                </div>
                <div class="market-detail-item">
                    <span class="market-detail-label">Volume</span>
                    <span class="market-detail-value">${formatNumber(data.volume)}</span>
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

// Format large numbers
function formatNumber(num) {
    if (!num || num === 0) return '--';
    if (num >= 1000000000) {
        return (num / 1000000000).toFixed(2) + 'B';
    } else if (num >= 1000000) {
        return (num / 1000000).toFixed(2) + 'M';
    } else if (num >= 1000) {
        return (num / 1000).toFixed(2) + 'K';
    }
    return num.toString();
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
    const modal = document.getElementById('add-symbol-modal');
    if (modal) {
        modal.classList.add('active');
        const input = document.getElementById('symbol-input');
        if (input) {
            input.value = '';
            input.focus();
        }
    }
}

function closeAddSymbolModal() {
    const modal = document.getElementById('add-symbol-modal');
    if (modal) {
        modal.classList.remove('active');
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

    // Modal close buttons
    const closeModalBtn = document.getElementById('close-symbol-modal');
    if (closeModalBtn) {
        closeModalBtn.addEventListener('click', closeAddSymbolModal);
    }

    const cancelBtn = document.getElementById('cancel-symbol-btn');
    if (cancelBtn) {
        cancelBtn.addEventListener('click', closeAddSymbolModal);
    }

    // Save symbol button
    const saveSymbolBtn = document.getElementById('save-symbol-btn');
    if (saveSymbolBtn) {
        saveSymbolBtn.addEventListener('click', function() {
            const symbolInput = document.getElementById('symbol-input');
            if (symbolInput && symbolInput.value) {
                addSymbol(symbolInput.value);
            }
        });
    }

    // Enter key support
    const symbolInput = document.getElementById('symbol-input');
    if (symbolInput) {
        symbolInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                if (this.value) {
                    addSymbol(this.value);
                }
            }
        });
    }

    // Close modal on outside click
    const modal = document.getElementById('add-symbol-modal');
    if (modal) {
        modal.addEventListener('click', function(e) {
            if (e.target === modal) {
                closeAddSymbolModal();
            }
        });
    }
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
