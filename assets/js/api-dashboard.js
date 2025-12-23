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
        
        // API Key Modal event listeners
        const modal = document.getElementById('api-key-modal');
        const modalClose = document.getElementById('api-key-modal-close');
        const modalCancel = document.getElementById('api-key-cancel-btn');
        const modalSave = document.getElementById('api-key-save-btn');
        const modalOverlay = modal?.querySelector('.api-key-modal-overlay');
        const apiKeyInput = document.getElementById('api-key-input');
        const apiKeyToggle = document.getElementById('api-key-toggle');
        
        // Close modal handlers
        if (modalClose) {
            modalClose.addEventListener('click', closeKeyConfig);
        }
        if (modalCancel) {
            modalCancel.addEventListener('click', closeKeyConfig);
        }
        if (modalOverlay) {
            modalOverlay.addEventListener('click', closeKeyConfig);
        }
        
        // Save API key
        if (modalSave) {
            modalSave.addEventListener('click', function() {
                const apiId = modal?.dataset?.apiId;
                const apiKey = apiKeyInput?.value;
                if (apiId && apiKey) {
                    saveAPIKey(apiId, apiKey);
                }
            });
        }
        
        // Enter key to save
        if (apiKeyInput) {
            apiKeyInput.addEventListener('keypress', function(e) {
                if (e.key === 'Enter') {
                    const apiId = modal?.dataset?.apiId;
                    const apiKey = this.value;
                    if (apiId && apiKey) {
                        saveAPIKey(apiId, apiKey);
                    }
                }
            });
        }
        
        // Toggle password visibility
        if (apiKeyToggle) {
            apiKeyToggle.addEventListener('click', function() {
                const input = apiKeyInput;
                const icon = this.querySelector('i');
                if (input.type === 'password') {
                    input.type = 'text';
                    if (icon) {
                        icon.classList.remove('fa-eye');
                        icon.classList.add('fa-eye-slash');
                    }
                } else {
                    input.type = 'password';
                    if (icon) {
                        icon.classList.remove('fa-eye-slash');
                        icon.classList.add('fa-eye');
                    }
                }
            });
        }
        
        // Close on Escape key
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape' && modal?.classList.contains('active')) {
                closeKeyConfig();
            }
        });
        
        // Custom API Modal event listeners - use event delegation for dynamic content
        const customApiModal = document.getElementById('custom-api-modal');
        const customApiModalClose = document.getElementById('custom-api-modal-close');
        const customApiModalCancel = document.getElementById('custom-api-cancel-btn');
        const customApiModalSave = document.getElementById('custom-api-save-btn');
        const customApiModalOverlay = customApiModal?.querySelector('.api-key-modal-overlay');
        const customApiKeyToggle = document.getElementById('custom-api-key-toggle');
        
        // Use event delegation for the add button (in case it's added dynamically)
        document.addEventListener('click', function(e) {
            if (e.target && (e.target.id === 'add-custom-api-btn' || e.target.closest('#add-custom-api-btn'))) {
                e.preventDefault();
                e.stopPropagation();
                console.log('Add Custom API button clicked (delegated)');
                // Small delay to ensure DOM is ready
                setTimeout(() => {
                    showCustomApiModal();
                }, 10);
            }
        });
        
        // Also set up direct listener if button exists
        const addCustomApiBtn = document.getElementById('add-custom-api-btn');
        if (addCustomApiBtn) {
            addCustomApiBtn.addEventListener('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                console.log('Add Custom API button clicked (direct)');
                // Small delay to ensure DOM is ready
                setTimeout(() => {
                    showCustomApiModal();
                }, 10);
            });
        } else {
            console.log('Add Custom API button not found during initialization - will use event delegation');
        }
        
        // Use event delegation for close buttons (they might not exist during init)
        // This ensures clicks work even if the button is clicked via its icon
        document.addEventListener('click', function(e) {
            // Close button - check if click is on button or icon inside it
            const closeBtn = e.target.closest('#custom-api-modal-close');
            if (closeBtn || e.target.id === 'custom-api-modal-close') {
                e.preventDefault();
                e.stopPropagation();
                console.log('Close button clicked');
                closeCustomApiModal();
                return;
            }
            
            // Cancel button
            const cancelBtn = e.target.closest('#custom-api-cancel-btn');
            if (cancelBtn || e.target.id === 'custom-api-cancel-btn') {
                e.preventDefault();
                e.stopPropagation();
                console.log('Cancel button clicked');
                closeCustomApiModal();
                return;
            }
            
            // Overlay click - only close if clicking directly on overlay, not on modal content
            if (e.target && e.target.classList.contains('api-key-modal-overlay')) {
                const modal = e.target.closest('#custom-api-modal');
                if (modal && modal.classList.contains('active')) {
                    e.preventDefault();
                    e.stopPropagation();
                    console.log('Overlay clicked');
                    closeCustomApiModal();
                    return;
                }
            }
        });
        
        // Also set up direct listeners if elements exist
        if (customApiModalClose) {
            customApiModalClose.addEventListener('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                closeCustomApiModal();
            });
        }
        if (customApiModalCancel) {
            customApiModalCancel.addEventListener('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                closeCustomApiModal();
            });
        }
        if (customApiModalOverlay) {
            customApiModalOverlay.addEventListener('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                closeCustomApiModal();
            });
        }
        
        if (customApiModalSave) {
            customApiModalSave.addEventListener('click', saveCustomAPI);
        }
        
        if (customApiKeyToggle) {
            customApiKeyToggle.addEventListener('click', function() {
                const input = document.getElementById('custom-api-key');
                const icon = this.querySelector('i');
                if (input.type === 'password') {
                    input.type = 'text';
                    if (icon) {
                        icon.classList.remove('fa-eye');
                        icon.classList.add('fa-eye-slash');
                    }
                } else {
                    input.type = 'password';
                    if (icon) {
                        icon.classList.remove('fa-eye-slash');
                        icon.classList.add('fa-eye');
                    }
                }
            });
        }
        
        // Close custom API modal on Escape
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') {
                const customApiModal = document.getElementById('custom-api-modal');
                if (customApiModal && customApiModal.classList.contains('active')) {
                    e.preventDefault();
                    e.stopPropagation();
                    closeCustomApiModal();
                }
                const restApiModal = document.getElementById('rest-api-modal');
                if (restApiModal && restApiModal.classList.contains('active')) {
                    e.preventDefault();
                    e.stopPropagation();
                    closeRESTApiModal();
                }
                const restApiExecuteModal = document.getElementById('rest-api-execute-modal');
                if (restApiExecuteModal && restApiExecuteModal.classList.contains('active')) {
                    e.preventDefault();
                    e.stopPropagation();
                    closeRESTApiExecuteModal();
                }
            }
        });
        
        // REST API Modal event listeners
        const addRestApiBtn = document.getElementById('add-rest-api-btn');
        if (addRestApiBtn) {
            addRestApiBtn.addEventListener('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                showRESTApiModal();
            });
        }
        
        // REST API modal close buttons
        document.addEventListener('click', function(e) {
            if (e.target && (e.target.id === 'rest-api-modal-close' || e.target.closest('#rest-api-modal-close'))) {
                e.preventDefault();
                e.stopPropagation();
                closeRESTApiModal();
            }
            if (e.target && (e.target.id === 'rest-api-cancel-btn' || e.target.closest('#rest-api-cancel-btn'))) {
                e.preventDefault();
                e.stopPropagation();
                closeRESTApiModal();
            }
            if (e.target && (e.target.id === 'rest-api-execute-close' || e.target.closest('#rest-api-execute-close'))) {
                e.preventDefault();
                e.stopPropagation();
                closeRESTApiExecuteModal();
            }
            if (e.target && (e.target.id === 'rest-api-execute-cancel-btn' || e.target.closest('#rest-api-execute-cancel-btn'))) {
                e.preventDefault();
                e.stopPropagation();
                closeRESTApiExecuteModal();
            }
        });
        
        const restApiSaveBtn = document.getElementById('rest-api-save-btn');
        if (restApiSaveBtn) {
            restApiSaveBtn.addEventListener('click', saveRESTAPI);
        }
        
        const restApiExecuteBtn = document.getElementById('rest-api-execute-btn');
        if (restApiExecuteBtn) {
            restApiExecuteBtn.addEventListener('click', executeRESTAPI);
        }
        
        // Toggle request body field based on HTTP method
        const restApiMethod = document.getElementById('rest-api-method');
        if (restApiMethod) {
            restApiMethod.addEventListener('change', function() {
                const bodyGroup = document.getElementById('rest-api-body-group');
                if (['POST', 'PUT', 'PATCH'].includes(this.value)) {
                    if (bodyGroup) bodyGroup.style.display = 'block';
                } else {
                    if (bodyGroup) bodyGroup.style.display = 'none';
                }
            });
        }
        
        // Auth value toggle
        const restApiAuthToggle = document.getElementById('rest-api-auth-toggle');
        if (restApiAuthToggle) {
            restApiAuthToggle.addEventListener('click', function() {
                const input = document.getElementById('rest-api-auth-value');
                const icon = this.querySelector('i');
                if (input.type === 'password') {
                    input.type = 'text';
                    if (icon) {
                        icon.classList.remove('fa-eye');
                        icon.classList.add('fa-eye-slash');
                    }
                } else {
                    input.type = 'password';
                    if (icon) {
                        icon.classList.remove('fa-eye-slash');
                        icon.classList.add('fa-eye');
                    }
                }
            });
        }
    }

    // Load API data
    async function loadAPIData() {
        try {
            const response = await fetch('api/get_api_dashboard.php');
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
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
            
            // Provide more specific error messages
            let errorMessage = 'Failed to load API dashboard data. ';
            if (error.message && (error.message.includes('Failed to fetch') || error.message.includes('Load failed'))) {
                errorMessage += 'Server connection failed. Please ensure the web server is running at http://localhost:8080';
            } else if (error.message && error.message.includes('HTTP')) {
                errorMessage += `Server error: ${error.message}`;
            } else {
                errorMessage += 'Please try again.';
            }
            
            showError(errorMessage);
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
        
        // Render custom APIs list
        renderCustomApisList(data);
        
        // Render REST APIs list
        loadRESTApis();
    }
    
    
    // Render REST APIs list
    function renderRESTApisList(apis) {
        const restApisList = document.getElementById('rest-apis-list');
        if (!restApisList) return;
        
        if (apis.length === 0) {
            restApisList.innerHTML = `
                <div class="empty-state">
                    <i class="fas fa-inbox"></i>
                    <p>No REST APIs configured yet. Click "Add REST API" to get started.</p>
                </div>
            `;
            return;
        }
        
        restApisList.innerHTML = `
            <div class="custom-apis-grid">
                ${apis.map(api => `
                    <div class="custom-api-card">
                        <div class="custom-api-header">
                            <div class="custom-api-info">
                                <h4>${escapeHtml(api.name)}</h4>
                                <span class="custom-api-id">${escapeHtml(api.api_id)}</span>
                            </div>
                            <div class="custom-api-actions">
                                <button class="btn-icon" onclick="APIDashboard.executeRESTAPI('${api.api_id}')" title="Execute">
                                    <i class="fas fa-play"></i>
                                </button>
                                <button class="btn-icon" onclick="APIDashboard.editRESTAPI('${api.api_id}')" title="Edit">
                                    <i class="fas fa-edit"></i>
                                </button>
                                <button class="btn-icon btn-danger" onclick="APIDashboard.deleteRESTAPI('${api.api_id}')" title="Delete">
                                    <i class="fas fa-trash"></i>
                                </button>
                            </div>
                        </div>
                        <p class="custom-api-description">${escapeHtml(api.description || 'No description')}</p>
                        <div class="custom-api-details">
                            <div class="custom-api-detail">
                                <i class="fas fa-link"></i>
                                <span>${escapeHtml(api.base_url)}${escapeHtml(api.endpoint_path)}</span>
                            </div>
                            <div class="custom-api-detail">
                                <i class="fas fa-code"></i>
                                <span>${api.http_method}</span>
                            </div>
                            <div class="custom-api-detail">
                                <i class="fas fa-key"></i>
                                <span>${api.auth_type !== 'none' ? api.auth_type : 'No Auth'}</span>
                            </div>
                        </div>
                    </div>
                `).join('')}
            </div>
        `;
    }
    
    // Render custom APIs list
    function renderCustomApisList(data) {
        const customApisList = document.getElementById('custom-apis-list');
        if (!customApisList) return;
        
        const customApis = data.apis.filter(api => api.isCustom);
        
        if (customApis.length === 0) {
            customApisList.innerHTML = `
                <div class="empty-state">
                    <i class="fas fa-inbox"></i>
                    <p>No custom APIs configured yet. Click "Add Custom API" to get started.</p>
                </div>
            `;
            return;
        }
        
        customApisList.innerHTML = `
            <div class="custom-apis-grid">
                ${customApis.map(api => `
                    <div class="custom-api-card">
                        <div class="custom-api-header">
                            <div class="custom-api-info">
                                <h4>${escapeHtml(api.name)}</h4>
                                <span class="custom-api-id">${escapeHtml(api.id)}</span>
                            </div>
                            <div class="custom-api-actions">
                                <button class="btn-icon" onclick="APIDashboard.editCustomAPI('${api.id}')" title="Edit">
                                    <i class="fas fa-edit"></i>
                                </button>
                                <button class="btn-icon btn-danger" onclick="APIDashboard.deleteCustomAPI('${api.id}')" title="Delete">
                                    <i class="fas fa-trash"></i>
                                </button>
                            </div>
                        </div>
                        <p class="custom-api-description">${escapeHtml(api.description || 'No description')}</p>
                        <div class="custom-api-details">
                            <div class="custom-api-detail">
                                <i class="fas fa-link"></i>
                                <span>${escapeHtml(api.baseUrl)}</span>
                            </div>
                            <div class="custom-api-detail">
                                <i class="fas fa-tachometer-alt"></i>
                                <span>${api.rateLimit} calls/${api.rateLimitPeriod}s</span>
                            </div>
                            <div class="custom-api-detail">
                                <i class="fas fa-key"></i>
                                <span>${api.requiresKey ? (api.hasKey ? 'Key Configured' : 'Key Required') : 'No Key'}</span>
                            </div>
                        </div>
                        ${api.isDefault ? `
                            <div class="custom-api-badge">
                                <i class="fas fa-star"></i> Default API
                            </div>
                        ` : `
                            <button class="btn-set-source btn-sm" onclick="APIDashboard.setAsDefault('${api.id}')">
                                <i class="fas fa-star"></i> Set as Default
                            </button>
                        `}
                    </div>
                `).join('')}
            </div>
        `;
    }
    
    // Show custom API modal
    function showCustomApiModal(apiId = null) {
        try {
            const modal = document.getElementById('custom-api-modal');
            if (!modal) {
                console.error('Custom API modal not found in DOM');
                showError('Custom API modal not found. Please refresh the page.');
                return;
            }
            
            // Show modal first
            modal.dataset.apiId = apiId || '';
            modal.classList.add('active');
            modal.style.display = 'flex';
            modal.style.opacity = '1';
            modal.style.visibility = 'visible';
            
            // Clear any previous error messages
            const statusDiv = document.getElementById('custom-api-modal-status');
            if (statusDiv) {
                statusDiv.innerHTML = '';
                statusDiv.className = 'api-key-modal-status';
            }
            
            const modalTitle = document.getElementById('custom-api-modal-action');
            const form = document.getElementById('custom-api-form');
            
            if (!form) {
                console.warn('Custom API form not found, but modal is showing');
            }
            
            // Helper function to safely set value
            const setValue = (id, value) => {
                try {
                    const el = document.getElementById(id);
                    if (el) {
                        if (el.type === 'checkbox') {
                            el.checked = Boolean(value);
                        } else if (el.tagName === 'SELECT') {
                            el.value = value || '';
                        } else {
                            el.value = value || '';
                        }
                    }
                } catch (e) {
                    console.warn(`Could not set value for ${id}:`, e);
                }
            };
            
            // Helper function to safely set disabled
            const setDisabled = (id, disabled) => {
                try {
                    const el = document.getElementById(id);
                    if (el) {
                        el.disabled = disabled;
                    }
                } catch (e) {
                    console.warn(`Could not set disabled for ${id}:`, e);
                }
            };
            
            // Clear form safely - reset individual fields instead of form.reset()
            // This is more reliable and won't throw errors if some elements are missing
            setValue('custom-api-id', '');
            setValue('custom-api-name', '');
            setValue('custom-api-description', '');
            setValue('custom-api-base-url', '');
            setValue('custom-api-quote-url', '');
            setValue('custom-api-key', '');
            setValue('custom-api-rate-limit', '60');
            setValue('custom-api-rate-period', '60');
            setValue('custom-api-response-format', 'json');
            setValue('custom-api-quote-path', '');
            setValue('custom-api-price-field', 'c');
            setValue('custom-api-change-field', 'd');
            setValue('custom-api-change-percent-field', 'dp');
            setValue('custom-api-volume-field', 'v');
            setValue('custom-api-high-field', 'h');
            setValue('custom-api-low-field', 'l');
            setValue('custom-api-open-field', 'o');
            setValue('custom-api-prev-close-field', 'pc');
            setValue('custom-api-requires-key', false);
            
            // Try form.reset() as a backup, but don't fail if it errors
            if (form) {
                try {
                    form.reset();
                } catch (e) {
                    // Ignore - we've already set values manually
                    console.debug('Form reset not available, using manual reset');
                }
            }
            
            if (apiId) {
                // Edit mode - load API data
                const api = apiData?.apis?.find(a => a.id === apiId && a.isCustom);
                if (api) {
                    setValue('custom-api-id', api.id);
                    setDisabled('custom-api-id', true);
                    setValue('custom-api-name', api.name);
                    setValue('custom-api-description', api.description || '');
                    setValue('custom-api-base-url', api.baseUrl);
                    setValue('custom-api-quote-url', api.quoteUrlTemplate || '');
                    setValue('custom-api-key', '');
                    setValue('custom-api-rate-limit', api.rateLimit);
                    setValue('custom-api-rate-period', api.rateLimitPeriod);
                    setValue('custom-api-requires-key', api.requiresKey);
                    setValue('custom-api-response-format', api.responseFormat || 'json');
                    setValue('custom-api-quote-path', api.quotePath || '');
                    setValue('custom-api-price-field', api.priceField || 'c');
                    setValue('custom-api-change-field', api.changeField || 'd');
                    setValue('custom-api-change-percent-field', api.changePercentField || 'dp');
                    setValue('custom-api-volume-field', api.volumeField || 'v');
                    setValue('custom-api-high-field', api.highField || 'h');
                    setValue('custom-api-low-field', api.lowField || 'l');
                    setValue('custom-api-open-field', api.openField || 'o');
                    setValue('custom-api-prev-close-field', api.previousCloseField || 'pc');
                    
                    if (modalTitle) modalTitle.textContent = 'Edit Custom API';
                }
            } else {
                // Add mode
                setDisabled('custom-api-id', false);
                if (modalTitle) modalTitle.textContent = 'Add Custom API';
            }
            
            console.log('Custom API modal opened successfully');
        } catch (error) {
            console.error('Error showing custom API modal:', error);
            console.error('Error stack:', error.stack);
            showError('Failed to open custom API form: ' + error.message);
        }
    }
    
    // Close custom API modal
    function closeCustomApiModal() {
        try {
            const modal = document.getElementById('custom-api-modal');
            if (modal) {
                modal.classList.remove('active');
                // Also remove inline styles that might have been added
                modal.style.display = '';
                modal.style.opacity = '';
                modal.style.visibility = '';
                
                // Clear any error messages
                const statusDiv = document.getElementById('custom-api-modal-status');
                if (statusDiv) {
                    statusDiv.innerHTML = '';
                    statusDiv.className = 'api-key-modal-status';
                }
                
                console.log('Custom API modal closed');
            } else {
                console.warn('Custom API modal not found when trying to close');
            }
        } catch (error) {
            console.error('Error closing custom API modal:', error);
        }
    }
    
    // Save custom API
    async function saveCustomAPI() {
        const modal = document.getElementById('custom-api-modal');
        const statusDiv = document.getElementById('custom-api-modal-status');
        const saveBtn = document.getElementById('custom-api-save-btn');
        
        if (!modal) return;
        
        const apiId = document.getElementById('custom-api-id').value.trim();
        const name = document.getElementById('custom-api-name').value.trim();
        const baseUrl = document.getElementById('custom-api-base-url').value.trim();
        const quoteUrl = document.getElementById('custom-api-quote-url').value.trim();
        
        if (!apiId || !name || !baseUrl || !quoteUrl) {
            if (statusDiv) {
                statusDiv.innerHTML = '<div class="error-message"><i class="fas fa-exclamation-circle"></i> Please fill in all required fields</div>';
            }
            return;
        }
        
        if (saveBtn) {
            saveBtn.disabled = true;
            saveBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';
        }
        
        if (statusDiv) {
            statusDiv.innerHTML = '';
        }
        
        try {
            const apiData = {
                api_id: apiId,
                name: name,
                description: document.getElementById('custom-api-description').value.trim(),
                base_url: baseUrl,
                quote_url_template: quoteUrl,
                api_key: document.getElementById('custom-api-key').value.trim(),
                rate_limit: parseInt(document.getElementById('custom-api-rate-limit').value) || 60,
                rate_limit_period: parseInt(document.getElementById('custom-api-rate-period').value) || 60,
                requires_key: document.getElementById('custom-api-requires-key').checked ? 1 : 0,
                response_format: document.getElementById('custom-api-response-format').value,
                quote_path: document.getElementById('custom-api-quote-path').value.trim(),
                price_field: document.getElementById('custom-api-price-field').value.trim() || 'c',
                change_field: document.getElementById('custom-api-change-field').value.trim() || 'd',
                change_percent_field: document.getElementById('custom-api-change-percent-field').value.trim() || 'dp',
                volume_field: document.getElementById('custom-api-volume-field').value.trim() || 'v',
                high_field: document.getElementById('custom-api-high-field').value.trim() || 'h',
                low_field: document.getElementById('custom-api-low-field').value.trim() || 'l',
                open_field: document.getElementById('custom-api-open-field').value.trim() || 'o',
                previous_close_field: document.getElementById('custom-api-prev-close-field').value.trim() || 'pc'
            };
            
            const response = await fetch('api/manage_custom_api.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(apiData)
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess(result.message || 'Custom API saved successfully');
                closeCustomApiModal();
                loadAPIData();
            } else {
                throw new Error(result.message || 'Failed to save custom API');
            }
        } catch (error) {
            console.error('Error saving custom API:', error);
            if (statusDiv) {
                statusDiv.innerHTML = `<div class="error-message"><i class="fas fa-exclamation-circle"></i> ${escapeHtml(error.message)}</div>`;
            }
        } finally {
            if (saveBtn) {
                saveBtn.disabled = false;
                saveBtn.innerHTML = '<i class="fas fa-save"></i> Save API';
            }
        }
    }
    
    // Edit custom API
    function editCustomAPI(apiId) {
        showCustomApiModal(apiId);
    }
    
    // Delete custom API
    async function deleteCustomAPI(apiId) {
        if (!confirm(`Are you sure you want to delete the custom API "${apiId}"? This action cannot be undone.`)) {
            return;
        }
        
        try {
            const response = await fetch('api/manage_custom_api.php', {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ api_id: apiId })
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess('Custom API deleted successfully');
                loadAPIData();
            } else {
                throw new Error(result.message || 'Failed to delete custom API');
            }
        } catch (error) {
            console.error('Error deleting custom API:', error);
            showError(error.message || 'Failed to delete custom API');
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
                const apiName = apiData?.apis?.find(a => a.id === apiId)?.name || apiId;
                showSuccess(`${apiName} set as default API source`);
                
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
        const modal = document.getElementById('api-key-modal');
        const apiName = apiData?.apis?.find(api => api.id === apiId)?.name || apiId;
        const modalTitle = document.getElementById('api-key-modal-api-name');
        const apiKeyInput = document.getElementById('api-key-input');
        const saveBtn = document.getElementById('api-key-save-btn');
        const statusDiv = document.getElementById('api-key-modal-status');
        
        if (!modal) {
            console.error('API key modal not found');
            return;
        }
        
        // Set API name in modal
        if (modalTitle) {
            modalTitle.textContent = `${apiName} API Key`;
        }
        
        // Clear previous values
        if (apiKeyInput) {
            apiKeyInput.value = '';
            apiKeyInput.type = 'password';
        }
        
        if (statusDiv) {
            statusDiv.innerHTML = '';
            statusDiv.className = 'api-key-modal-status';
        }
        
        // Store current API ID for save action
        modal.dataset.apiId = apiId;
        
        // Show modal
        modal.classList.add('active');
        
        // Focus on input
        setTimeout(() => {
            if (apiKeyInput) {
                apiKeyInput.focus();
            }
        }, 100);
    }
    
    // Close key configuration modal
    function closeKeyConfig() {
        const modal = document.getElementById('api-key-modal');
        if (modal) {
            modal.classList.remove('active');
            const statusDiv = document.getElementById('api-key-modal-status');
            if (statusDiv) {
                statusDiv.innerHTML = '';
                statusDiv.className = 'api-key-modal-status';
            }
        }
    }
    
    // Save API key
    async function saveAPIKey(apiId, apiKey) {
        const statusDiv = document.getElementById('api-key-modal-status');
        const saveBtn = document.getElementById('api-key-save-btn');
        
        // Check if it's a custom API
        const isCustomApi = apiData?.apis?.find(a => a.id === apiId && a.isCustom);
        
        if (isCustomApi) {
            // For custom APIs, update via manage_custom_api.php
            const api = apiData.apis.find(a => a.id === apiId);
            if (!api) {
                if (statusDiv) {
                    statusDiv.innerHTML = '<i class="fas fa-exclamation-circle"></i> API not found';
                    statusDiv.className = 'api-key-modal-status error';
                }
                return;
            }
            
            // Show loading state
            if (saveBtn) {
                saveBtn.disabled = true;
                saveBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';
            }
            
            if (statusDiv) {
                statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving API key...';
                statusDiv.className = 'api-key-modal-status info';
            }
            
            // Update the custom API with new key
            try {
                const response = await fetch('api/manage_custom_api.php', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        api_id: apiId,
                        name: api.name,
                        description: api.description || '',
                        base_url: api.baseUrl,
                        quote_url_template: api.quoteUrlTemplate || '',
                        api_key: apiKey.trim(),
                        rate_limit: api.rateLimit,
                        rate_limit_period: api.rateLimitPeriod,
                        requires_key: api.requiresKey ? 1 : 0,
                        response_format: api.responseFormat || 'json',
                        quote_path: api.quotePath || '',
                        price_field: api.priceField || 'c',
                        change_field: api.changeField || 'd',
                        change_percent_field: api.changePercentField || 'dp',
                        volume_field: api.volumeField || 'v',
                        high_field: api.highField || 'h',
                        low_field: api.lowField || 'l',
                        open_field: api.openField || 'o',
                        previous_close_field: api.previousCloseField || 'pc'
                    })
                });
                
                const result = await response.json();
                
                if (result.success) {
                    if (statusDiv) {
                        statusDiv.innerHTML = '<i class="fas fa-check-circle"></i> API key saved successfully!';
                        statusDiv.className = 'api-key-modal-status success';
                    }
                    
                    setTimeout(() => {
                        loadAPIData().then(() => {
                            setTimeout(() => {
                                closeKeyConfig();
                            }, 1000);
                        });
                    }, 500);
                } else {
                    throw new Error(result.message || 'Failed to update API key');
                }
            } catch (error) {
                console.error('Error updating custom API key:', error);
                if (statusDiv) {
                    statusDiv.innerHTML = `<i class="fas fa-exclamation-circle"></i> ${escapeHtml(error.message)}`;
                    statusDiv.className = 'api-key-modal-status error';
                }
            } finally {
                if (saveBtn) {
                    saveBtn.disabled = false;
                    saveBtn.innerHTML = '<i class="fas fa-save"></i> Save API Key';
                }
            }
            return;
        }
        
        // For built-in APIs (Finnhub)
        if (!apiKey || apiKey.trim().length < 10) {
            if (statusDiv) {
                statusDiv.innerHTML = '<i class="fas fa-exclamation-circle"></i> Please enter a valid API key';
                statusDiv.className = 'api-key-modal-status error';
            }
            return;
        }
        
        // Show loading state
        if (saveBtn) {
            saveBtn.disabled = true;
            saveBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';
        }
        
        if (statusDiv) {
            statusDiv.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving API key...';
            statusDiv.className = 'api-key-modal-status info';
        }
        
        try {
            const response = await fetch('api/update_api_key.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    api_id: apiId,
                    api_key: apiKey.trim()
                })
            });
            
            const result = await response.json();
            
            if (result.success) {
                if (statusDiv) {
                    statusDiv.innerHTML = '<i class="fas fa-check-circle"></i> API key saved successfully!';
                    statusDiv.className = 'api-key-modal-status success';
                }
                
                // Reload API data after a short delay
                setTimeout(() => {
                    loadAPIData().then(() => {
                        setTimeout(() => {
                            closeKeyConfig();
                        }, 1000);
                    });
                }, 500);
            } else {
                throw new Error(result.message || 'Failed to save API key');
            }
        } catch (error) {
            console.error('Error saving API key:', error);
            if (statusDiv) {
                statusDiv.innerHTML = `<i class="fas fa-exclamation-circle"></i> ${escapeHtml(error.message)}`;
                statusDiv.className = 'api-key-modal-status error';
            }
        } finally {
            if (saveBtn) {
                saveBtn.disabled = false;
                saveBtn.innerHTML = '<i class="fas fa-save"></i> Save API Key';
            }
        }
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
        try {
            // Simple error display
            const errorDiv = document.createElement('div');
            errorDiv.className = 'api-error-message';
            errorDiv.innerHTML = `<i class="fas fa-exclamation-circle"></i> ${escapeHtml(message)}`;
            const container = document.querySelector('.api-dashboard-container');
            if (container) {
                container.prepend(errorDiv);
                setTimeout(() => {
                    try {
                        errorDiv.remove();
                    } catch (e) {
                        console.warn('Error removing error message:', e);
                    }
                }, 5000);
            } else {
                // Fallback: use alert if container not found
                console.error('Error message:', message);
                alert('Error: ' + message);
            }
        } catch (e) {
            console.error('Error displaying error message:', e);
            console.error('Original error:', message);
        }
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

    // REST API Functions
    let restApiData = null;
    
    // Show REST API modal
    function showRESTApiModal(apiId = null) {
        try {
            const modal = document.getElementById('rest-api-modal');
            if (!modal) {
                console.error('REST API modal not found');
                return;
            }
            
            modal.dataset.apiId = apiId || '';
            modal.classList.add('active');
            modal.style.display = 'flex';
            modal.style.opacity = '1';
            modal.style.visibility = 'visible';
            
            const modalTitle = document.getElementById('rest-api-modal-action');
            const statusDiv = document.getElementById('rest-api-modal-status');
            if (statusDiv) {
                statusDiv.innerHTML = '';
            }
            
            // Clear form
            document.getElementById('rest-api-id').value = '';
            document.getElementById('rest-api-name').value = '';
            document.getElementById('rest-api-description').value = '';
            document.getElementById('rest-api-base-url').value = '';
            document.getElementById('rest-api-endpoint').value = '';
            document.getElementById('rest-api-method').value = 'GET';
            document.getElementById('rest-api-auth-type').value = 'none';
            document.getElementById('rest-api-auth-value').value = '';
            document.getElementById('rest-api-headers').value = '';
            document.getElementById('rest-api-query-params').value = '';
            document.getElementById('rest-api-request-body').value = '';
            document.getElementById('rest-api-timeout').value = '30';
            document.getElementById('rest-api-response-format').value = 'json';
            document.getElementById('rest-api-rate-limit').value = '60';
            document.getElementById('rest-api-rate-period').value = '60';
            document.getElementById('rest-api-body-group').style.display = 'none';
            
            if (apiId) {
                // Edit mode - load API data
                loadRESTApis().then(() => {
                    const api = restApiData?.find(a => a.api_id === apiId);
                    if (api) {
                        document.getElementById('rest-api-id').value = api.api_id;
                        document.getElementById('rest-api-id').disabled = true;
                        document.getElementById('rest-api-name').value = api.name;
                        document.getElementById('rest-api-description').value = api.description || '';
                        document.getElementById('rest-api-base-url').value = api.base_url;
                        document.getElementById('rest-api-endpoint').value = api.endpoint_path;
                        document.getElementById('rest-api-method').value = api.http_method;
                        document.getElementById('rest-api-auth-type').value = api.auth_type;
                        document.getElementById('rest-api-auth-value').value = '';
                        document.getElementById('rest-api-headers').value = api.headers ? JSON.stringify(JSON.parse(api.headers), null, 2) : '';
                        document.getElementById('rest-api-query-params').value = api.query_params ? JSON.stringify(JSON.parse(api.query_params), null, 2) : '';
                        document.getElementById('rest-api-request-body').value = api.request_body || '';
                        document.getElementById('rest-api-timeout').value = api.timeout;
                        document.getElementById('rest-api-response-format').value = api.response_format;
                        document.getElementById('rest-api-rate-limit').value = api.rate_limit;
                        document.getElementById('rest-api-rate-period').value = api.rate_limit_period;
                        
                        if (['POST', 'PUT', 'PATCH'].includes(api.http_method)) {
                            document.getElementById('rest-api-body-group').style.display = 'block';
                        }
                        
                        if (modalTitle) modalTitle.textContent = 'Edit REST API';
                    }
                });
            } else {
                document.getElementById('rest-api-id').disabled = false;
                if (modalTitle) modalTitle.textContent = 'Add REST API';
            }
        } catch (error) {
            console.error('Error showing REST API modal:', error);
            showError('Failed to open REST API form: ' + error.message);
        }
    }
    
    // Close REST API modal
    function closeRESTApiModal() {
        const modal = document.getElementById('rest-api-modal');
        if (modal) {
            modal.classList.remove('active');
            modal.style.display = '';
            modal.style.opacity = '';
            modal.style.visibility = '';
        }
    }
    
    // Save REST API
    async function saveRESTAPI() {
        const modal = document.getElementById('rest-api-modal');
        const statusDiv = document.getElementById('rest-api-modal-status');
        const saveBtn = document.getElementById('rest-api-save-btn');
        
        if (!modal) return;
        
        const apiId = document.getElementById('rest-api-id').value.trim();
        const name = document.getElementById('rest-api-name').value.trim();
        const baseUrl = document.getElementById('rest-api-base-url').value.trim();
        const endpoint = document.getElementById('rest-api-endpoint').value.trim();
        
        if (!apiId || !name || !baseUrl || !endpoint) {
            if (statusDiv) {
                statusDiv.innerHTML = '<div class="error-message"><i class="fas fa-exclamation-circle"></i> Please fill in all required fields</div>';
            }
            return;
        }
        
        if (saveBtn) {
            saveBtn.disabled = true;
            saveBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';
        }
        
        try {
            // Parse JSON fields
            let headers = null;
            let queryParams = null;
            try {
                const headersText = document.getElementById('rest-api-headers').value.trim();
                if (headersText) {
                    headers = JSON.parse(headersText);
                }
            } catch (e) {
                throw new Error('Invalid JSON in Headers field');
            }
            
            try {
                const queryParamsText = document.getElementById('rest-api-query-params').value.trim();
                if (queryParamsText) {
                    queryParams = JSON.parse(queryParamsText);
                }
            } catch (e) {
                throw new Error('Invalid JSON in Query Parameters field');
            }
            
            const apiData = {
                api_id: apiId,
                name: name,
                description: document.getElementById('rest-api-description').value.trim(),
                base_url: baseUrl,
                endpoint_path: endpoint,
                http_method: document.getElementById('rest-api-method').value,
                headers: headers,
                auth_type: document.getElementById('rest-api-auth-type').value,
                auth_value: document.getElementById('rest-api-auth-value').value.trim(),
                request_body: document.getElementById('rest-api-request-body').value.trim() || null,
                query_params: queryParams,
                response_format: document.getElementById('rest-api-response-format').value,
                timeout: parseInt(document.getElementById('rest-api-timeout').value) || 30,
                rate_limit: parseInt(document.getElementById('rest-api-rate-limit').value) || 60,
                rate_limit_period: parseInt(document.getElementById('rest-api-rate-period').value) || 60
            };
            
            const response = await fetch('api/manage_rest_api.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(apiData)
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess(result.message || 'REST API saved successfully');
                closeRESTApiModal();
                loadRESTApis();
            } else {
                throw new Error(result.message || 'Failed to save REST API');
            }
        } catch (error) {
            console.error('Error saving REST API:', error);
            if (statusDiv) {
                statusDiv.innerHTML = `<div class="error-message"><i class="fas fa-exclamation-circle"></i> ${escapeHtml(error.message)}</div>`;
            }
        } finally {
            if (saveBtn) {
                saveBtn.disabled = false;
                saveBtn.innerHTML = '<i class="fas fa-save"></i> Save REST API';
            }
        }
    }
    
    // Edit REST API
    function editRESTAPI(apiId) {
        showRESTApiModal(apiId);
    }
    
    // Delete REST API
    async function deleteRESTAPI(apiId) {
        if (!confirm(`Are you sure you want to delete the REST API "${apiId}"? This action cannot be undone.`)) {
            return;
        }
        
        try {
            const response = await fetch('api/manage_rest_api.php', {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ api_id: apiId })
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess('REST API deleted successfully');
                loadRESTApis();
            } else {
                throw new Error(result.message || 'Failed to delete REST API');
            }
        } catch (error) {
            console.error('Error deleting REST API:', error);
            showError(error.message || 'Failed to delete REST API');
        }
    }
    
    // Execute REST API
    async function executeRESTAPI(apiId) {
        try {
            // Load API config
            const response = await fetch('api/manage_rest_api.php');
            const result = await response.json();
            
            if (!result.success) {
                throw new Error('Failed to load REST APIs');
            }
            
            restApiData = result.apis;
            const api = restApiData.find(a => a.api_id === apiId);
            
            if (!api) {
                throw new Error('REST API not found');
            }
            
            // Show execute modal
            const modal = document.getElementById('rest-api-execute-modal');
            if (!modal) {
                throw new Error('Execute modal not found');
            }
            
            modal.dataset.apiId = apiId;
            document.getElementById('rest-api-execute-name').textContent = api.name;
            document.getElementById('rest-api-execute-description').textContent = api.description || '';
            document.getElementById('rest-api-execute-params').value = api.query_params ? JSON.stringify(JSON.parse(api.query_params), null, 2) : '';
            document.getElementById('rest-api-execute-headers').value = '';
            document.getElementById('rest-api-execute-body').value = api.request_body || '';
            
            const bodyGroup = document.getElementById('rest-api-execute-body-group');
            if (['POST', 'PUT', 'PATCH'].includes(api.http_method)) {
                if (bodyGroup) bodyGroup.style.display = 'block';
            } else {
                if (bodyGroup) bodyGroup.style.display = 'none';
            }
            
            document.getElementById('rest-api-execute-result').style.display = 'none';
            document.getElementById('rest-api-execute-status').innerHTML = '';
            
            modal.classList.add('active');
            modal.style.display = 'flex';
            modal.style.opacity = '1';
            modal.style.visibility = 'visible';
        } catch (error) {
            console.error('Error opening execute modal:', error);
            showError(error.message);
        }
    }
    
    // Execute REST API call
    async function executeRESTAPI() {
        const modal = document.getElementById('rest-api-execute-modal');
        const statusDiv = document.getElementById('rest-api-execute-status');
        const executeBtn = document.getElementById('rest-api-execute-btn');
        const resultDiv = document.getElementById('rest-api-execute-result');
        
        if (!modal) return;
        
        const apiId = modal.dataset.apiId;
        if (!apiId) return;
        
        if (executeBtn) {
            executeBtn.disabled = true;
            executeBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Executing...';
        }
        
        if (statusDiv) {
            statusDiv.innerHTML = '<div class="info-message"><i class="fas fa-spinner fa-spin"></i> Executing request...</div>';
        }
        
        resultDiv.style.display = 'none';
        
        try {
            // Parse parameters
            let params = null;
            let headers = null;
            let body = null;
            
            try {
                const paramsText = document.getElementById('rest-api-execute-params').value.trim();
                if (paramsText) {
                    params = JSON.parse(paramsText);
                }
            } catch (e) {
                throw new Error('Invalid JSON in Query Parameters');
            }
            
            try {
                const headersText = document.getElementById('rest-api-execute-headers').value.trim();
                if (headersText) {
                    headers = JSON.parse(headersText);
                }
            } catch (e) {
                throw new Error('Invalid JSON in Headers');
            }
            
            const bodyText = document.getElementById('rest-api-execute-body').value.trim();
            if (bodyText) {
                try {
                    body = JSON.parse(bodyText);
                } catch (e) {
                    body = bodyText; // Use as plain text if not JSON
                }
            }
            
            const response = await fetch('api/manage_rest_api.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    action: 'execute',
                    api_id: apiId,
                    params: params,
                    headers: headers,
                    body: body
                })
            });
            
            const result = await response.json();
            
            if (result.success) {
                // Display result
                document.getElementById('rest-api-execute-status-code').textContent = result.http_code;
                document.getElementById('rest-api-execute-status-code').className = 'badge ' + (result.http_code >= 200 && result.http_code < 300 ? 'success' : 'error');
                document.getElementById('rest-api-execute-time').textContent = result.response_time;
                
                const responsePre = document.getElementById('rest-api-execute-response');
                if (typeof result.response === 'object') {
                    responsePre.textContent = JSON.stringify(result.response, null, 2);
                } else {
                    responsePre.textContent = result.response;
                }
                
                resultDiv.style.display = 'block';
                if (statusDiv) {
                    statusDiv.innerHTML = '<div class="success-message"><i class="fas fa-check-circle"></i> Request executed successfully</div>';
                }
            } else {
                throw new Error(result.message || 'Request failed');
            }
        } catch (error) {
            console.error('Error executing REST API:', error);
            if (statusDiv) {
                statusDiv.innerHTML = `<div class="error-message"><i class="fas fa-exclamation-circle"></i> ${escapeHtml(error.message)}</div>`;
            }
        } finally {
            if (executeBtn) {
                executeBtn.disabled = false;
                executeBtn.innerHTML = '<i class="fas fa-play"></i> Execute';
            }
        }
    }
    
    // Close REST API execute modal
    function closeRESTApiExecuteModal() {
        const modal = document.getElementById('rest-api-execute-modal');
        if (modal) {
            modal.classList.remove('active');
            modal.style.display = '';
            modal.style.opacity = '';
            modal.style.visibility = '';
        }
    }
    
    // Load REST APIs and store data
    async function loadRESTApis() {
        try {
            const response = await fetch('api/manage_rest_api.php');
            const result = await response.json();
            
            if (result.success) {
                restApiData = result.apis || [];
                renderRESTApisList(restApiData);
            }
        } catch (error) {
            console.error('Error loading REST APIs:', error);
        }
    }

    // Public API
    return {
        init: init,
        setAsDefault: setAsDefault,
        showKeyConfig: showKeyConfig,
        closeKeyConfig: closeKeyConfig,
        saveAPIKey: saveAPIKey,
        loadAPIData: loadAPIData,
        watchPageActivation: watchPageActivation,
        editCustomAPI: editCustomAPI,
        deleteCustomAPI: deleteCustomAPI,
        closeCustomApiModal: closeCustomApiModal,
        editRESTAPI: editRESTAPI,
        deleteRESTAPI: deleteRESTAPI,
        executeRESTAPI: executeRESTAPI
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

