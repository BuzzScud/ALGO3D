/**
 * API Management Dashboard
 * Interactive dashboard for monitoring and managing APIs
 */

const APIDashboard = (function() {
    'use strict';

    let updateInterval = null;
    let currentTab = 'status';
    let apiData = null;

    // Initialize dashboard
    function init() {
        setupTabs();
        setupEventListeners();
        loadAPIData();
        
        // Auto-refresh every 10 seconds
        updateInterval = setInterval(loadAPIData, 10000);
        
        console.log('API Dashboard initialized');
    }

    // Setup tab switching
    function setupTabs() {
        document.querySelectorAll('.api-tab').forEach(tab => {
            tab.addEventListener('click', function() {
                const tabName = this.dataset.tab;
                switchTab(tabName);
            });
        });
    }

    // Switch tabs
    function switchTab(tabName) {
        currentTab = tabName;
        
        // Update tab buttons
        document.querySelectorAll('.api-tab').forEach(tab => {
            tab.classList.remove('active');
            if (tab.dataset.tab === tabName) {
                tab.classList.add('active');
            }
        });
        
        // Update tab content
        document.querySelectorAll('.api-tab-content').forEach(content => {
            content.classList.remove('active');
            if (content.id === `tab-${tabName}`) {
                content.classList.add('active');
            }
        });
        
        // Load tab-specific data
        if (tabName === 'status' && apiData) {
            renderMetrics(apiData);
        } else if (tabName === 'configuration' && apiData) {
            renderConfiguration(apiData);
        } else if (tabName === 'history' && apiData) {
            renderHistory(apiData);
        }
    }

    // Setup event listeners
    function setupEventListeners() {
        const refreshBtn = document.getElementById('api-refresh-all-btn');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', function() {
                this.classList.add('spinning');
                loadAPIData().finally(() => {
                    setTimeout(() => this.classList.remove('spinning'), 500);
                });
            });
        }
    }

    // Load API data
    async function loadAPIData() {
        try {
            const response = await fetch('api/get_api_dashboard.php');
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const data = await response.json();
            
            if (data.success) {
                apiData = data;
                renderOverview(data);
                
                // Render current tab content
                if (currentTab === 'status') {
                    renderMetrics(data);
                } else if (currentTab === 'configuration') {
                    renderConfiguration(data);
                } else if (currentTab === 'history') {
                    renderHistory(data);
                }
            } else {
                throw new Error(data.message || 'Failed to load API data');
            }
        } catch (error) {
            console.error('Error loading API data:', error);
            showError('Failed to load API dashboard data. Please try again.');
        }
    }

    // Render overview cards
    function renderOverview(data) {
        const grid = document.getElementById('api-overview-grid');
        if (!grid) return;

        grid.innerHTML = data.apis.map(api => createAPICard(api)).join('');
        
        // Attach click listeners to cards
        grid.querySelectorAll('.api-card').forEach(card => {
            card.addEventListener('click', function() {
                const apiId = this.dataset.apiId;
                // Could expand to show details
            });
        });
    }

    // Create API card
    function createAPICard(api) {
        const statusClass = getStatusClass(api.status);
        const usagePercent = api.usagePercent || 0;
        const usageColor = usagePercent >= 90 ? 'danger' : usagePercent >= 70 ? 'warning' : 'success';
        
        return `
            <div class="api-card ${statusClass}" data-api-id="${api.id}">
                <div class="api-card-header">
                    <div class="api-card-title-section">
                        <div class="api-card-icon ${statusClass}">
                            <i class="fas fa-plug"></i>
                        </div>
                        <div class="api-card-title">
                            <h3>${escapeHtml(api.name)}</h3>
                            <span class="api-card-subtitle">${escapeHtml(api.description)}</span>
                        </div>
                    </div>
                    ${api.isDefault ? '<span class="api-badge-default">Default</span>' : ''}
                </div>
                
                <div class="api-card-body">
                    <div class="api-status-indicator">
                        <span class="status-dot ${statusClass}"></span>
                        <span class="status-text">${getStatusText(api.status)}</span>
                    </div>
                    
                    <div class="api-metrics-row">
                        <div class="api-metric">
                            <span class="metric-label">Rate Limit</span>
                            <span class="metric-value">${api.rateLimit}/min</span>
                        </div>
                        <div class="api-metric">
                            <span class="metric-label">Used</span>
                            <span class="metric-value ${usageColor}">${api.used}</span>
                        </div>
                        <div class="api-metric">
                            <span class="metric-label">Remaining</span>
                            <span class="metric-value">${api.remaining}</span>
                        </div>
                    </div>
                    
                    <div class="api-usage-bar">
                        <div class="api-usage-fill ${usageColor}" style="width: ${Math.min(usagePercent, 100)}%"></div>
                    </div>
                    
                    ${api.health ? `
                    <div class="api-health-info">
                        <div class="health-item">
                            <i class="fas fa-clock"></i>
                            <span>Response: ${api.health.responseTime || 0}ms</span>
                        </div>
                        ${api.health.lastChecked ? `
                        <div class="health-item">
                            <i class="fas fa-check-circle"></i>
                            <span>Checked: ${formatTimeAgo(api.health.lastChecked)}</span>
                        </div>
                        ` : ''}
                    </div>
                    ` : ''}
                </div>
                
                <div class="api-card-footer">
                    ${api.requiresKey ? `
                        <div class="api-key-status">
                            ${api.hasKey ? 
                                '<i class="fas fa-key text-success"></i><span>API Key Configured</span>' :
                                '<i class="fas fa-key text-danger"></i><span>API Key Missing</span>'
                            }
                        </div>
                    ` : '<div class="api-key-status"><i class="fas fa-unlock"></i><span>No Key Required</span></div>'}
                    
                    <button class="api-action-btn" onclick="APIDashboard.setAsDefault('${api.id}')" ${api.isDefault ? 'disabled' : ''}>
                        <i class="fas fa-star"></i>
                        Set Default
                    </button>
                </div>
            </div>
        `;
    }

    // Render metrics
    function renderMetrics(data) {
        const grid = document.getElementById('api-metrics-grid');
        if (!grid) return;

        grid.innerHTML = data.apis.map(api => createMetricsCard(api)).join('');
    }

    // Create metrics card
    function createMetricsCard(api) {
        return `
            <div class="api-metrics-card">
                <div class="metrics-card-header">
                    <h3>${escapeHtml(api.name)} Metrics</h3>
                    <span class="status-badge ${getStatusClass(api.status)}">${getStatusText(api.status)}</span>
                </div>
                
                <div class="metrics-content">
                    <div class="metric-row">
                        <div class="metric-item">
                            <div class="metric-icon">
                                <i class="fas fa-tachometer-alt"></i>
                            </div>
                            <div class="metric-info">
                                <span class="metric-label">Usage Rate</span>
                                <span class="metric-value-large">${api.used}/${api.rateLimit}</span>
                                <div class="metric-progress">
                                    <div class="progress-bar" style="width: ${Math.min(api.usagePercent, 100)}%"></div>
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    <div class="metrics-grid-mini">
                        <div class="metric-mini">
                            <i class="fas fa-chart-line"></i>
                            <span class="metric-label-small">Total Calls</span>
                            <span class="metric-value-small">${formatNumber(api.totalCalls)}</span>
                        </div>
                        <div class="metric-mini">
                            <i class="fas fa-check-circle"></i>
                            <span class="metric-label-small">Success Rate</span>
                            <span class="metric-value-small">${api.successRate.toFixed(1)}%</span>
                        </div>
                        <div class="metric-mini">
                            <i class="fas fa-clock"></i>
                            <span class="metric-label-small">Avg Response</span>
                            <span class="metric-value-small">${api.avgResponseTime.toFixed(0)}ms</span>
                        </div>
                        <div class="metric-mini">
                            <i class="fas fa-exclamation-triangle"></i>
                            <span class="metric-label-small">Errors</span>
                            <span class="metric-value-small">${api.errors}</span>
                        </div>
                    </div>
                    
                    ${api.health ? `
                    <div class="health-details">
                        <h4>Health Check</h4>
                        <div class="health-grid">
                            <div class="health-item-detail">
                                <span>Status</span>
                                <span class="health-value ${getStatusClass(api.health.status)}">${getHealthStatusText(api.health.status)}</span>
                            </div>
                            <div class="health-item-detail">
                                <span>Response Time</span>
                                <span class="health-value">${api.health.responseTime || 0}ms</span>
                            </div>
                            ${api.health.lastChecked ? `
                            <div class="health-item-detail">
                                <span>Last Checked</span>
                                <span class="health-value">${formatTimeAgo(api.health.lastChecked)}</span>
                            </div>
                            ` : ''}
                            ${api.health.error ? `
                            <div class="health-item-detail">
                                <span>Error</span>
                                <span class="health-value error">${escapeHtml(api.health.error)}</span>
                            </div>
                            ` : ''}
                        </div>
                    </div>
                    ` : ''}
                </div>
            </div>
        `;
    }

    // Render configuration
    function renderConfiguration(data) {
        const selector = document.getElementById('api-source-selector');
        const keysConfig = document.getElementById('api-keys-config');
        
        if (selector) {
            selector.innerHTML = `
                <div class="api-source-cards">
                    ${data.apis.map(api => `
                        <div class="api-source-card ${api.isDefault ? 'active' : ''}" data-api-id="${api.id}">
                            <div class="source-card-header">
                                <div class="source-radio">
                                    <input type="radio" name="api-source" id="source-${api.id}" value="${api.id}" ${api.isDefault ? 'checked' : ''}>
                                    <label for="source-${api.id}"></label>
                                </div>
                                <h4>${escapeHtml(api.name)}</h4>
                            </div>
                            <p class="source-description">${escapeHtml(api.description)}</p>
                            <div class="source-details">
                                <div class="source-detail-item">
                                    <i class="fas fa-tachometer-alt"></i>
                                    <span>${api.rateLimit} calls/min</span>
                                </div>
                                <div class="source-detail-item">
                                    <i class="fas fa-key"></i>
                                    <span>${api.requiresKey ? (api.hasKey ? 'Key Configured' : 'Key Required') : 'No Key Needed'}</span>
                                </div>
                            </div>
                            <button class="btn-set-source" onclick="APIDashboard.setAsDefault('${api.id}')" ${api.isDefault ? 'disabled' : ''}>
                                ${api.isDefault ? '<i class="fas fa-check"></i> Current Default' : '<i class="fas fa-star"></i> Set as Default'}
                            </button>
                        </div>
                    `).join('')}
                </div>
            `;
        }
        
        if (keysConfig) {
            keysConfig.innerHTML = `
                <div class="api-keys-list">
                    ${data.apis.filter(api => api.requiresKey).map(api => `
                        <div class="api-key-item">
                            <div class="key-item-header">
                                <div class="key-item-info">
                                    <h4>${escapeHtml(api.name)} API Key</h4>
                                    <span class="key-status ${api.hasKey ? 'configured' : 'missing'}">
                                        ${api.hasKey ? '<i class="fas fa-check-circle"></i> Configured' : '<i class="fas fa-exclamation-circle"></i> Missing'}
                                    </span>
                                </div>
                            </div>
                            <div class="key-item-content">
                                <p class="key-description">Required for accessing ${escapeHtml(api.name)} API endpoints. Get your free API key from <a href="https://finnhub.io/register" target="_blank">finnhub.io</a></p>
                                ${api.hasKey ? `
                                    <div class="key-display">
                                        <code class="key-preview">${api.hasKey ? '••••••••••••••••' : 'Not configured'}</code>
                                        <button class="btn-key-action" onclick="APIDashboard.showKeyConfig('${api.id}')">
                                            <i class="fas fa-edit"></i> Update Key
                                        </button>
                                    </div>
                                ` : `
                                    <button class="btn-key-action primary" onclick="APIDashboard.showKeyConfig('${api.id}')">
                                        <i class="fas fa-plus"></i> Add API Key
                                    </button>
                                `}
                            </div>
                        </div>
                    `).join('')}
                </div>
            `;
        }
    }

    // Render history
    function renderHistory(data) {
        const container = document.getElementById('api-history-container');
        if (!container) return;

        container.innerHTML = `
            <div class="api-history-timeline">
                ${data.apis.map(api => `
                    <div class="history-item">
                        <div class="history-header">
                            <h4>${escapeHtml(api.name)}</h4>
                            <span class="history-status ${getStatusClass(api.status)}">${getStatusText(api.status)}</span>
                        </div>
                        <div class="history-stats">
                            <div class="history-stat">
                                <i class="fas fa-chart-bar"></i>
                                <span>Total Calls: <strong>${formatNumber(api.totalCalls)}</strong></span>
                            </div>
                            <div class="history-stat">
                                <i class="fas fa-check-circle"></i>
                                <span>Success Rate: <strong>${api.successRate.toFixed(1)}%</strong></span>
                            </div>
                            <div class="history-stat">
                                <i class="fas fa-clock"></i>
                                <span>Avg Response: <strong>${api.avgResponseTime.toFixed(0)}ms</strong></span>
                            </div>
                            ${api.lastUsed ? `
                            <div class="history-stat">
                                <i class="fas fa-history"></i>
                                <span>Last Used: <strong>${formatTimeAgo(api.lastUsed)}</strong></span>
                            </div>
                            ` : ''}
                        </div>
                    </div>
                `).join('')}
            </div>
        `;
    }

    // Set API as default
    async function setAsDefault(apiId) {
        try {
            const response = await fetch('api/set_api_source.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ source: apiId })
            });

            const result = await response.json();
            
            if (result.success) {
                showSuccess(`${apiId === 'finnhub' ? 'Finnhub' : 'Yahoo Finance'} set as default API source`);
                
                // Update localStorage to sync with session
                localStorage.setItem('apiSource', apiId);
                
                // Reload API data
                loadAPIData();
                
                // Trigger chart reload if charts module is available
                if (typeof ChartsModule !== 'undefined' && ChartsModule.reloadChart) {
                    ChartsModule.reloadChart(true); // Force refresh
                }
                
                // Trigger market data refresh if available
                if (typeof fetchMarketData === 'function') {
                    fetchMarketData();
                }
                
                // Dispatch custom event for other modules to listen
                window.dispatchEvent(new CustomEvent('apiSourceChanged', { 
                    detail: { source: apiId } 
                }));
            } else {
                throw new Error(result.message || 'Failed to set default API');
            }
        } catch (error) {
            console.error('Error setting default API:', error);
            showError('Failed to set default API. Please try again.');
        }
    }

    // Show key configuration modal
    function showKeyConfig(apiId) {
        // This would open a modal for API key configuration
        // For now, redirect to settings
        alert(`Please configure your API key in the Settings page.`);
    }

    // Helper functions
    function getStatusClass(status) {
        const statusMap = {
            'active': 'success',
            'healthy': 'success',
            'error': 'danger',
            'no_key': 'warning',
            'unavailable': 'danger',
            'unknown': 'secondary'
        };
        return statusMap[status] || 'secondary';
    }

    function getStatusText(status) {
        const statusMap = {
            'active': 'Active',
            'healthy': 'Healthy',
            'error': 'Error',
            'no_key': 'No API Key',
            'unavailable': 'Unavailable',
            'unknown': 'Unknown'
        };
        return statusMap[status] || 'Unknown';
    }

    function getHealthStatusText(status) {
        const statusMap = {
            'healthy': 'Healthy',
            'error': 'Error',
            'no_key': 'No API Key',
            'unknown': 'Unknown'
        };
        return statusMap[status] || 'Unknown';
    }

    function formatNumber(num) {
        if (num >= 1000000) {
            return (num / 1000000).toFixed(1) + 'M';
        } else if (num >= 1000) {
            return (num / 1000).toFixed(1) + 'K';
        }
        return num.toString();
    }

    function formatTimeAgo(timestamp) {
        if (!timestamp) return 'Never';
        const seconds = Math.floor((Date.now() / 1000) - timestamp);
        if (seconds < 60) return 'Just now';
        if (seconds < 3600) return Math.floor(seconds / 60) + 'm ago';
        if (seconds < 86400) return Math.floor(seconds / 3600) + 'h ago';
        return Math.floor(seconds / 86400) + 'd ago';
    }

    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    function showError(message) {
        // Simple error display
        const errorDiv = document.createElement('div');
        errorDiv.className = 'api-error-message';
        errorDiv.innerHTML = `<i class="fas fa-exclamation-circle"></i> ${escapeHtml(message)}`;
        document.querySelector('.api-dashboard-container')?.prepend(errorDiv);
        setTimeout(() => errorDiv.remove(), 5000);
    }

    function showSuccess(message) {
        // Simple success display
        const successDiv = document.createElement('div');
        successDiv.className = 'api-success-message';
        successDiv.innerHTML = `<i class="fas fa-check-circle"></i> ${escapeHtml(message)}`;
        document.querySelector('.api-dashboard-container')?.prepend(successDiv);
        setTimeout(() => successDiv.remove(), 3000);
    }

    // Watch for page activation
    function watchPageActivation() {
        const apiPage = document.getElementById('page-api');
        if (!apiPage) return;

        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (apiPage.classList.contains('active')) {
                        loadAPIData();
                    }
                }
            });
        });

        observer.observe(apiPage, {
            attributes: true,
            attributeFilter: ['class']
        });

        // Load if already active
        if (apiPage.classList.contains('active')) {
            loadAPIData();
        }
    }

    // Public API
    return {
        init: init,
        setAsDefault: setAsDefault,
        showKeyConfig: showKeyConfig,
        loadAPIData: loadAPIData,
        watchPageActivation: watchPageActivation
    };
})();

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        APIDashboard.init();
        APIDashboard.watchPageActivation();
    });
} else {
    APIDashboard.init();
    APIDashboard.watchPageActivation();
}

