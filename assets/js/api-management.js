/**
 * API Management Module
 * Handles API configuration, monitoring, and status tracking
 */

const ApiManagementModule = (function() {
    'use strict';
    
    let statusUpdateInterval = null;
    let apiConfigs = [];
    let apiStatuses = {};
    
    // 3D Visualization variables
    let apiScene = null;
    let apiCamera = null;
    let apiRenderer = null;
    let apiAnimationId = null;
    let apiObjects = [];
    let isDragging = false;
    let previousMousePosition = { x: 0, y: 0 };
    let cameraDistance = 12;
    let cameraRotation = { theta: 0, phi: Math.PI / 3 };
    
    /**
     * Initialize API Management Module
     */
    function init() {
        try {
            loadApiConfigurations();
            loadApiStatus();
            loadApiStatistics('day'); // Load statistics on init
            setupEventListeners();
            
            // Initialize 3D visualization after a short delay to ensure DOM is ready
            setTimeout(() => {
                init3DVisualization();
            }, 500);
            
            // Auto-refresh status every 10 seconds for real-time metrics
            statusUpdateInterval = setInterval(() => {
                try {
                    loadApiStatus();
                    const timeframe = document.getElementById('api-stats-timeframe')?.value || 'day';
                    loadApiStatistics(timeframe);
                    update3DVisualization(); // Update 3D visualization with new status
                } catch (error) {
                    console.error('Error in auto-refresh:', error);
                }
            }, 10000);
        } catch (error) {
            console.error('Error initializing API Management Module:', error);
            showError('Failed to initialize API Management. Please refresh the page.');
        }
    }
    
    /**
     * Setup event listeners
     */
    function setupEventListeners() {
        // Refresh status button
        const refreshBtn = document.getElementById('refresh-api-status-btn');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', () => {
                refreshBtn.classList.add('spinning');
                loadApiStatus().finally(() => {
                    setTimeout(() => refreshBtn.classList.remove('spinning'), 500);
                });
            });
        }
        
        // Add API button
        const addBtn = document.getElementById('add-api-btn');
        if (addBtn) {
            addBtn.addEventListener('click', () => openApiModal());
        }
        
        // API modal controls
        const closeModalBtn = document.getElementById('close-api-modal');
        const cancelBtn = document.getElementById('cancel-api-btn');
        const saveBtn = document.getElementById('save-api-btn');
        const toggleKeyBtn = document.getElementById('toggle-api-key-modal');
        
        if (closeModalBtn) {
            closeModalBtn.addEventListener('click', closeApiModal);
        }
        if (cancelBtn) {
            cancelBtn.addEventListener('click', closeApiModal);
        }
        if (saveBtn) {
            saveBtn.addEventListener('click', saveApi);
        }
        if (toggleKeyBtn) {
            toggleKeyBtn.addEventListener('click', () => {
                const keyInput = document.getElementById('api-key');
                if (keyInput.type === 'password') {
                    keyInput.type = 'text';
                    toggleKeyBtn.innerHTML = '<i class="fas fa-eye-slash"></i>';
                } else {
                    keyInput.type = 'password';
                    toggleKeyBtn.innerHTML = '<i class="fas fa-eye"></i>';
                }
            });
        }
        
        // Close modal on outside click
        const modal = document.getElementById('api-modal');
        if (modal) {
            modal.addEventListener('click', (e) => {
                if (e.target === modal) {
                    closeApiModal();
                }
            });
        }
        
        // Stats timeframe selector
        const statsTimeframe = document.getElementById('api-stats-timeframe');
        if (statsTimeframe) {
            statsTimeframe.addEventListener('change', () => {
                loadApiStatistics(statsTimeframe.value);
            });
        }
    }
    
    /**
     * Load API configurations from backend
     */
    async function loadApiConfigurations() {
        try {
            const response = await fetch('api/api_management.php?action=get_configs');
            if (!response.ok) throw new Error('Failed to load API configurations');
            
            const result = await response.json();
            if (result.success && result.configs) {
                apiConfigs = result.configs;
                renderApiConfigurations();
            }
        } catch (error) {
            console.error('Error loading API configurations:', error);
            showError('Failed to load API configurations');
        }
    }
    
    /**
     * Load API status from backend
     */
    async function loadApiStatus() {
        try {
            const response = await fetch('api/get_api_status.php');
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: Failed to load API status`);
            }
            
            const status = await response.json();
            if (!status || typeof status !== 'object') {
                throw new Error('Invalid API status response');
            }
            
            apiStatuses = status;
            
            renderApiStatus();
            updateOverallStatus();
            
            // Update 3D visualization if initialized
            if (apiScene && typeof THREE !== 'undefined') {
                update3DVisualization();
            }
        } catch (error) {
            console.error('Error loading API status:', error);
            showError('Failed to load API status. Please try again.');
            
            // Set default status to prevent UI errors
            apiStatuses = {
                finnhub: { name: 'Finnhub', available: false, limit: 60, used: 0, remaining: 0 },
                yahoo: { name: 'Yahoo Finance', available: false, limit: 100, used: 0, remaining: 0 }
            };
            renderApiStatus();
            updateOverallStatus();
        }
    }
    
    /**
     * Load API usage statistics
     */
    async function loadApiStatistics(timeframe = 'day') {
        try {
            const response = await fetch(`api/api_management.php?action=get_stats&timeframe=${encodeURIComponent(timeframe)}`);
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: Failed to load statistics`);
            }
            
            const result = await response.json();
            if (result.success && result.stats) {
                renderApiStatistics(result.stats);
            } else {
                // Show empty state if no stats available
                const container = document.getElementById('api-stats-container');
                const overallSummary = document.getElementById('api-overall-summary');
                if (container) {
                    container.innerHTML = '<div class="api-stats-empty">No statistics available yet. API calls will be tracked automatically.</div>';
                }
                if (overallSummary) {
                    overallSummary.innerHTML = '';
                }
            }
        } catch (error) {
            console.error('Error loading API statistics:', error);
            const container = document.getElementById('api-stats-container');
            const overallSummary = document.getElementById('api-overall-summary');
            if (container) {
                container.innerHTML = '<div class="api-stats-empty">Unable to load statistics. Please try again.</div>';
            }
            if (overallSummary) {
                overallSummary.innerHTML = '';
            }
        }
    }
    
    /**
     * Render API status cards
     */
    function renderApiStatus() {
        const grid = document.getElementById('api-status-grid');
        if (!grid) return;
        
        if (!apiStatuses || Object.keys(apiStatuses).length === 0) {
            grid.innerHTML = '<div class="api-status-empty"><i class="fas fa-info-circle"></i><p>No API status data available. APIs will appear here once they are used.</p></div>';
            return;
        }
        
        const statusCards = [];
        
        // Render each API status
        for (const [apiId, status] of Object.entries(apiStatuses)) {
            if (apiId === 'default' || apiId === 'cache_duration') continue;
            
            const isAvailable = status.available !== false;
            const usagePercent = (status.used / status.limit) * 100;
            const remaining = status.remaining || 0;
            
            statusCards.push(`
                <div class="api-status-card ${isAvailable ? 'status-available' : 'status-limited'}">
                    <div class="api-status-card-header">
                        <div class="api-status-card-title">
                            <i class="fas fa-plug"></i>
                            <h3>${status.name || apiId}</h3>
                        </div>
                        <div class="api-status-badge ${isAvailable ? 'badge-success' : 'badge-error'}">
                            <span class="status-dot"></span>
                            ${isAvailable ? 'Available' : 'Rate Limited'}
                        </div>
                    </div>
                    <div class="api-status-card-body">
                        <div class="api-status-metric">
                            <span class="metric-label">Rate Limit</span>
                            <span class="metric-value">${status.limit} calls/min</span>
                        </div>
                        <div class="api-status-metric">
                            <span class="metric-label">Used</span>
                            <span class="metric-value">${status.used || 0} calls</span>
                        </div>
                        <div class="api-status-metric">
                            <span class="metric-label">Remaining</span>
                            <span class="metric-value ${remaining < 10 ? 'value-warning' : ''}">${remaining} calls</span>
                        </div>
                        <div class="api-status-progress">
                            <div class="progress-bar">
                                <div class="progress-fill" style="width: ${Math.min(usagePercent, 100)}%; background: ${usagePercent > 80 ? '#ef4444' : usagePercent > 60 ? '#f59e0b' : '#10b981'}"></div>
                            </div>
                            <span class="progress-text">${usagePercent.toFixed(1)}% used</span>
                        </div>
                    </div>
                    ${apiId === 'finnhub' || apiId === 'yahoo' ? `
                    <div class="api-status-card-footer">
                        <button class="btn btn-sm btn-secondary" onclick="ApiManagementModule.testApi('${apiId}')">
                            <i class="fas fa-vial"></i> Test Connection
                        </button>
                    </div>
                    ` : ''}
                </div>
            `);
        }
        
        grid.innerHTML = statusCards.join('');
    }
    
    /**
     * Render API configuration cards
     */
    function renderApiConfigurations() {
        const list = document.getElementById('api-config-list');
        if (!list) return;
        
        if (apiConfigs.length === 0) {
            list.innerHTML = `
                <div class="api-config-empty">
                    <i class="fas fa-plug"></i>
                    <p>No APIs configured yet</p>
                    <button class="btn btn-primary" onclick="ApiManagementModule.openApiModal()">
                        <i class="fas fa-plus"></i> Add Your First API
                    </button>
                </div>
            `;
            return;
        }
        
        const configCards = apiConfigs.map(config => {
            const isEnabled = config.enabled !== false;
            const priorityText = config.priority === 1 ? 'Primary' : config.priority === 2 ? 'Secondary' : 'Backup';
            const priorityColor = config.priority === 1 ? '#2563eb' : config.priority === 2 ? '#f59e0b' : '#6b7280';
            const statusColor = isEnabled ? '#10b981' : '#6b7280';
            const isDefault = config.is_default || config.id === 'finnhub' || config.id === 'yahoo';
            
            return `
                <div class="api-config-card-enhanced ${!isEnabled ? 'config-disabled' : ''}">
                    <div class="api-config-header-compact">
                        <div class="api-config-main-info">
                            <div class="api-config-icon-wrapper" style="background: ${priorityColor}20; border-color: ${priorityColor}">
                                <i class="fas fa-plug" style="color: ${priorityColor}"></i>
                            </div>
                            <div class="api-config-title-group">
                                <h3 class="api-config-name">${config.name}</h3>
                                <div class="api-config-meta">
                                    <span class="api-config-provider-compact">${config.provider}</span>
                                    <span class="api-config-type-badge">${formatApiType(config.type)}</span>
                                </div>
                            </div>
                        </div>
                        <div class="api-config-status-badges">
                            <span class="api-status-badge-compact priority-${config.priority}" style="background: ${priorityColor}20; color: ${priorityColor}; border-color: ${priorityColor}">
                                ${priorityText}
                            </span>
                            <span class="api-status-badge-compact status-${isEnabled ? 'enabled' : 'disabled'}" style="background: ${statusColor}20; color: ${statusColor}; border-color: ${statusColor}">
                                <i class="fas ${isEnabled ? 'fa-check-circle' : 'fa-times-circle'}"></i>
                                ${isEnabled ? 'Active' : 'Inactive'}
                            </span>
                        </div>
                    </div>
                    
                    <div class="api-config-details-grid">
                        <div class="api-config-detail-item">
                            <div class="detail-icon">
                                <i class="fas fa-link"></i>
                            </div>
                            <div class="detail-content">
                                <span class="detail-label">Base URL</span>
                                <span class="detail-value" title="${config.base_url}">${truncateText(config.base_url, 40)}</span>
                            </div>
                        </div>
                        
                        <div class="api-config-detail-item">
                            <div class="detail-icon">
                                <i class="fas fa-tachometer-alt"></i>
                            </div>
                            <div class="detail-content">
                                <span class="detail-label">Rate Limit</span>
                                <span class="detail-value">${config.rate_limit} calls/min</span>
                            </div>
                        </div>
                        
                        <div class="api-config-detail-item">
                            <div class="detail-icon">
                                <i class="fas fa-key"></i>
                            </div>
                            <div class="detail-content">
                                <span class="detail-label">API Key</span>
                                <span class="detail-value api-key-display" title="${config.api_key}">${maskApiKey(config.api_key)}</span>
                            </div>
                        </div>
                        
                        ${config.notes ? `
                        <div class="api-config-detail-item full-width">
                            <div class="detail-icon">
                                <i class="fas fa-sticky-note"></i>
                            </div>
                            <div class="detail-content">
                                <span class="detail-label">Notes</span>
                                <span class="detail-value">${config.notes}</span>
                            </div>
                        </div>
                        ` : ''}
                    </div>
                    
                    <div class="api-config-actions-compact">
                        ${!isDefault ? `
                        <label class="api-toggle-wrapper">
                            <span class="toggle-label-text">${isEnabled ? 'Enabled' : 'Disabled'}</span>
                            <label class="toggle-switch">
                                <input type="checkbox" ${isEnabled ? 'checked' : ''} onchange="ApiManagementModule.toggleApi('${config.id}', this.checked)">
                                <span class="toggle-slider"></span>
                            </label>
                        </label>
                        ` : '<span class="api-default-badge">Default API</span>'}
                        <div class="api-action-buttons">
                            <button class="btn-icon" onclick="ApiManagementModule.editApi('${config.id}')" title="Edit API">
                                <i class="fas fa-edit"></i>
                            </button>
                            ${!isDefault ? `
                            <button class="btn-icon btn-icon-danger" onclick="ApiManagementModule.deleteApi('${config.id}')" title="Delete API">
                                <i class="fas fa-trash"></i>
                            </button>
                            ` : ''}
                        </div>
                    </div>
                </div>
            `;
        }).join('');
        
        list.innerHTML = configCards;
    }
    
    /**
     * Render API statistics with enhanced visualizations
     */
    function renderApiStatistics(stats) {
        const container = document.getElementById('api-stats-container');
        const overallSummary = document.getElementById('api-overall-summary');
        
        if (!container || !overallSummary) return;
        
        if (!stats || Object.keys(stats).length === 0) {
            container.innerHTML = '<div class="api-stats-empty"><i class="fas fa-info-circle"></i><p>No statistics available yet.</p><p class="text-sm">API calls will be tracked automatically when you use the APIs.</p></div>';
            overallSummary.innerHTML = '<div class="api-overall-summary-card"><div class="overall-summary-header"><div class="overall-status-indicator" style="background: rgba(107, 114, 128, 0.1); border-color: #6b7280"><i class="fas fa-info-circle" style="color: #6b7280"></i></div><div class="overall-status-text"><h4>No Data Yet</h4><span style="color: #6b7280">Waiting for API calls...</span></div></div></div>';
            return;
        }
        
        // Calculate overall metrics
        let totalCalls = 0;
        let totalSuccess = 0;
        let totalErrors = 0;
        let totalResponseTime = 0;
        let apiCount = 0;
        
        Object.values(stats).forEach(data => {
            const calls = parseInt(data.total_calls) || 0;
            const success = parseInt(data.success_count) || 0;
            const errors = parseInt(data.errors) || 0;
            const responseTime = parseFloat(data.avg_response_time) || 0;
            
            totalCalls += calls;
            totalSuccess += success;
            totalErrors += errors;
            totalResponseTime += responseTime * calls; // Weighted by call count
            apiCount++;
        });
        
        // Ensure no division by zero and valid calculations
        const overallSuccessRate = totalCalls > 0 ? Math.max(0, Math.min(100, (totalSuccess / totalCalls) * 100)) : 0;
        const overallAvgResponseTime = totalCalls > 0 ? Math.max(0, totalResponseTime / totalCalls) : 0;
        
        // Determine overall status
        let overallStatusColor = '#10b981';
        let overallStatusIcon = 'fa-check-circle';
        let overallStatusText = 'All Systems Operational';
        if (overallSuccessRate < 80 || totalErrors > 0) {
            overallStatusColor = '#ef4444';
            overallStatusIcon = 'fa-exclamation-circle';
            overallStatusText = 'Issues Detected';
        } else if (overallSuccessRate < 95) {
            overallStatusColor = '#f59e0b';
            overallStatusIcon = 'fa-exclamation-triangle';
            overallStatusText = 'Degraded Performance';
        }
        
        // Render overall summary
        overallSummary.innerHTML = `
            <div class="api-overall-summary-card">
                <div class="overall-summary-header">
                    <div class="overall-status-indicator" style="background: ${overallStatusColor}20; border-color: ${overallStatusColor}">
                        <i class="fas ${overallStatusIcon}" style="color: ${overallStatusColor}"></i>
                    </div>
                    <div class="overall-status-text">
                        <h4>Overall Status</h4>
                        <span style="color: ${overallStatusColor}">${overallStatusText}</span>
                    </div>
                </div>
                
                <div class="overall-metrics-grid">
                    <div class="overall-metric-item">
                        <div class="overall-metric-icon" style="background: rgba(37, 99, 235, 0.1); color: #2563eb">
                            <i class="fas fa-chart-bar"></i>
                        </div>
                        <div class="overall-metric-content">
                            <span class="overall-metric-label">Total Calls</span>
                            <span class="overall-metric-value">${formatNumber(totalCalls)}</span>
                        </div>
                    </div>
                    
                    <div class="overall-metric-item">
                        <div class="overall-metric-icon" style="background: ${overallStatusColor}20; color: ${overallStatusColor}">
                            <i class="fas fa-percentage"></i>
                        </div>
                        <div class="overall-metric-content">
                            <span class="overall-metric-label">Success Rate</span>
                            <span class="overall-metric-value" style="color: ${overallStatusColor}">${Math.max(0, Math.min(100, overallSuccessRate)).toFixed(1)}%</span>
                        </div>
                    </div>
                    
                    <div class="overall-metric-item">
                        <div class="overall-metric-icon" style="background: rgba(16, 185, 129, 0.1); color: #10b981">
                            <i class="fas fa-clock"></i>
                        </div>
                        <div class="overall-metric-content">
                            <span class="overall-metric-label">Avg Response</span>
                            <span class="overall-metric-value">${Math.max(0, overallAvgResponseTime).toFixed(0)}ms</span>
                        </div>
                    </div>
                    
                    <div class="overall-metric-item">
                        <div class="overall-metric-icon" style="background: ${totalErrors > 0 ? 'rgba(239, 68, 68, 0.1)' : 'rgba(16, 185, 129, 0.1)'}; color: ${totalErrors > 0 ? '#ef4444' : '#10b981'}">
                            <i class="fas ${totalErrors > 0 ? 'fa-times-circle' : 'fa-check-circle'}"></i>
                        </div>
                        <div class="overall-metric-content">
                            <span class="overall-metric-label">Total Errors</span>
                            <span class="overall-metric-value" style="color: ${totalErrors > 0 ? '#ef4444' : '#10b981'}">${Math.max(0, totalErrors)}</span>
                        </div>
                    </div>
                </div>
            </div>
        `;
        
        // Render individual API stats
        const statsHTML = Object.entries(stats).map(([apiId, data]) => {
            // Ensure all values are valid numbers
            const successRate = Math.max(0, Math.min(100, parseFloat(data.success_rate) || 0));
            const totalCalls = Math.max(0, parseInt(data.total_calls) || 0);
            const avgResponseTime = Math.max(0, parseFloat(data.avg_response_time) || 0);
            const errors = Math.max(0, parseInt(data.errors) || 0);
            
            // Determine status color
            let statusColor = '#10b981'; // Green
            let statusIcon = 'fa-check-circle';
            if (successRate < 80) {
                statusColor = '#ef4444'; // Red
                statusIcon = 'fa-exclamation-circle';
            } else if (successRate < 95) {
                statusColor = '#f59e0b'; // Orange
                statusIcon = 'fa-exclamation-triangle';
            }
            
            // Response time indicator (ensure valid values)
            let responseTimeStatus = 'good';
            let responseTimeColor = '#10b981';
            const validResponseTime = Math.max(0, avgResponseTime);
            if (validResponseTime > 1000) {
                responseTimeStatus = 'slow';
                responseTimeColor = '#ef4444';
            } else if (validResponseTime > 500) {
                responseTimeStatus = 'moderate';
                responseTimeColor = '#f59e0b';
            }
            
            return `
                <div class="api-stat-card-compact">
                    <div class="api-stat-compact-header">
                        <div class="api-stat-compact-title">
                            <div class="api-stat-status-dot" style="background: ${statusColor}"></div>
                            <div>
                                <h5>${data.name || apiId}</h5>
                                <span class="api-stat-compact-id">${apiId}</span>
                            </div>
                        </div>
                        <div class="api-stat-compact-badge" style="background: ${statusColor}20; color: ${statusColor}; border-color: ${statusColor}">
                            ${Math.max(0, Math.min(100, successRate)).toFixed(1)}%
                        </div>
                    </div>
                    
                    <div class="api-stat-compact-metrics">
                        <div class="api-stat-compact-metric">
                            <i class="fas fa-chart-bar" style="color: #3b82f6"></i>
                            <span class="metric-label-compact">Calls</span>
                            <span class="metric-value-compact">${formatNumber(totalCalls)}</span>
                        </div>
                        
                        <div class="api-stat-compact-metric">
                            <i class="fas fa-clock" style="color: ${responseTimeColor}"></i>
                            <span class="metric-label-compact">Response</span>
                            <span class="metric-value-compact" style="color: ${responseTimeColor}">${Math.max(0, validResponseTime).toFixed(0)}ms</span>
                        </div>
                        
                        <div class="api-stat-compact-metric">
                            <i class="fas ${errors > 0 ? 'fa-times-circle' : 'fa-check-circle'}" style="color: ${errors > 0 ? '#ef4444' : '#10b981'}"></i>
                            <span class="metric-label-compact">Errors</span>
                            <span class="metric-value-compact" style="color: ${errors > 0 ? '#ef4444' : '#10b981'}">${Math.max(0, errors)}</span>
                        </div>
                    </div>
                </div>
            `;
        }).join('');
        
        container.innerHTML = statsHTML;
    }
    
    /**
     * Format large numbers
     */
    function formatNumber(num) {
        const validNum = Math.max(0, parseInt(num) || 0);
        if (validNum >= 1000000) {
            return (validNum / 1000000).toFixed(1) + 'M';
        } else if (validNum >= 1000) {
            return (validNum / 1000).toFixed(1) + 'K';
        }
        return validNum.toString();
    }
    
    /**
     * Update overall API status indicator
     */
    function updateOverallStatus() {
        const indicator = document.getElementById('overall-api-status');
        if (!indicator) return;
        
        let allAvailable = true;
        let hasLimited = false;
        
        for (const [apiId, status] of Object.entries(apiStatuses)) {
            if (apiId === 'default' || apiId === 'cache_duration') continue;
            if (status.available === false) {
                allAvailable = false;
                hasLimited = true;
                break;
            }
        }
        
        const dot = indicator.querySelector('.status-dot');
        const text = indicator.querySelector('.status-text');
        
        if (hasLimited) {
            indicator.className = 'api-status-indicator status-error';
            if (dot) dot.className = 'status-dot error';
            if (text) text.textContent = 'Some APIs Rate Limited';
        } else if (allAvailable) {
            indicator.className = 'api-status-indicator status-success';
            if (dot) dot.className = 'status-dot success';
            if (text) text.textContent = 'All APIs Operational';
        } else {
            indicator.className = 'api-status-indicator status-warning';
            if (dot) dot.className = 'status-dot warning';
            if (text) text.textContent = 'Checking Status...';
        }
    }
    
    /**
     * Open API modal for adding/editing
     */
    function openApiModal(apiId = null) {
        const modal = document.getElementById('api-modal');
        const form = document.getElementById('api-form');
        const title = document.getElementById('api-modal-title');
        
        if (!modal || !form) return;
        
        if (apiId) {
            // Edit mode
            const config = apiConfigs.find(c => c.id === apiId);
            if (!config) {
                showError('API configuration not found');
                return;
            }
            
            if (title) title.textContent = 'Edit API';
            document.getElementById('api-id').value = config.id;
            document.getElementById('api-name').value = config.name || '';
            document.getElementById('api-provider').value = config.provider || '';
            document.getElementById('api-type').value = config.type || 'market_data';
            document.getElementById('api-base-url').value = config.base_url || '';
            document.getElementById('api-key').value = config.api_key || '';
            document.getElementById('api-rate-limit').value = config.rate_limit || 60;
            document.getElementById('api-priority').value = config.priority || 2;
            document.getElementById('api-enabled').checked = config.enabled !== false;
            document.getElementById('api-notes').value = config.notes || '';
        } else {
            // Add mode
            if (title) title.textContent = 'Add New API';
            form.reset();
            document.getElementById('api-id').value = '';
            document.getElementById('api-enabled').checked = true;
        }
        
        modal.classList.add('active');
    }
    
    /**
     * Close API modal
     */
    function closeApiModal() {
        const modal = document.getElementById('api-modal');
        if (modal) {
            modal.classList.remove('active');
        }
    }
    
    /**
     * Save API configuration
     */
    async function saveApi() {
        const form = document.getElementById('api-form');
        if (!form) return;
        
        const formData = {
            id: document.getElementById('api-id').value || null,
            name: document.getElementById('api-name').value.trim(),
            provider: document.getElementById('api-provider').value.trim(),
            type: document.getElementById('api-type').value,
            base_url: document.getElementById('api-base-url').value.trim(),
            api_key: document.getElementById('api-key').value.trim(),
            rate_limit: parseInt(document.getElementById('api-rate-limit').value) || 60,
            priority: parseInt(document.getElementById('api-priority').value) || 2,
            enabled: document.getElementById('api-enabled').checked,
            notes: document.getElementById('api-notes').value.trim()
        };
        
        // Validation
        if (!formData.name || !formData.provider || !formData.base_url || !formData.api_key) {
            showError('Please fill in all required fields');
            return;
        }
        
        try {
            const response = await fetch('api/api_management.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    action: formData.id ? 'update' : 'create',
                    config: formData
                })
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess(formData.id ? 'API updated successfully' : 'API added successfully');
                closeApiModal();
                loadApiConfigurations();
            } else {
                showError(result.message || 'Failed to save API configuration');
            }
        } catch (error) {
            console.error('Error saving API:', error);
            showError('Failed to save API configuration');
        }
    }
    
    /**
     * Toggle API enabled/disabled
     */
    async function toggleApi(apiId, enabled) {
        try {
            const response = await fetch('api/api_management.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    action: 'toggle',
                    id: apiId,
                    enabled: enabled
                })
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess(enabled ? 'API enabled' : 'API disabled');
                loadApiConfigurations();
            } else {
                showError(result.message || 'Failed to toggle API');
            }
        } catch (error) {
            console.error('Error toggling API:', error);
            showError('Failed to toggle API');
        }
    }
    
    /**
     * Edit API
     */
    function editApi(apiId) {
        openApiModal(apiId);
    }
    
    /**
     * Delete API
     */
    async function deleteApi(apiId) {
        if (!confirm('Are you sure you want to delete this API configuration? This action cannot be undone.')) {
            return;
        }
        
        try {
            const response = await fetch('api/api_management.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    action: 'delete',
                    id: apiId
                })
            });
            
            const result = await response.json();
            
            if (result.success) {
                showSuccess('API deleted successfully');
                loadApiConfigurations();
            } else {
                showError(result.message || 'Failed to delete API');
            }
        } catch (error) {
            console.error('Error deleting API:', error);
            showError('Failed to delete API');
        }
    }
    
    /**
     * Test API connection
     */
    async function testApi(apiId) {
        try {
            showLoading('Testing API connection...');
            
            const response = await fetch('api/api_management.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    action: 'test',
                    id: apiId
                })
            });
            
            const result = await response.json();
            
            hideLoading();
            
            if (result.success) {
                showSuccess(`API test successful! Response time: ${result.response_time}ms`);
            } else {
                showError(result.message || 'API test failed');
            }
        } catch (error) {
            console.error('Error testing API:', error);
            hideLoading();
            showError('Failed to test API connection');
        }
    }
    
    /**
     * Helper functions
     */
    function formatApiType(type) {
        const types = {
            'market_data': 'Market Data',
            'news': 'News',
            'fundamental': 'Fundamental Data',
            'options': 'Options Data',
            'crypto': 'Cryptocurrency',
            'forex': 'Forex',
            'other': 'Other'
        };
        return types[type] || type;
    }
    
    function maskApiKey(key) {
        if (!key || key.length < 8) return '••••••••';
        return key.substring(0, 4) + '••••' + key.substring(key.length - 4);
    }
    
    function truncateText(text, maxLength) {
        if (!text) return '';
        if (text.length <= maxLength) return text;
        return text.substring(0, maxLength - 3) + '...';
    }
    
    function showError(message) {
        const errorEl = document.getElementById('api-error-message');
        if (errorEl) {
            const spanEl = errorEl.querySelector('span');
            if (spanEl) spanEl.textContent = message;
            errorEl.style.display = 'flex';
            setTimeout(() => {
                errorEl.style.display = 'none';
            }, 5000);
        } else {
            alert(message);
        }
    }
    
    function showSuccess(message) {
        const successEl = document.getElementById('api-success-message');
        if (successEl) {
            const spanEl = successEl.querySelector('span');
            if (spanEl) spanEl.textContent = message;
            successEl.style.display = 'flex';
            setTimeout(() => {
                successEl.style.display = 'none';
            }, 3000);
        }
    }
    
    function showLoading(message) {
        // Show loading indicator
        console.log(message);
    }
    
    function hideLoading() {
        // Hide loading indicator
    }
    
    /**
     * Initialize 3D Visualization
     */
    function init3DVisualization() {
        const canvas = document.getElementById('api-3d-canvas');
        const loading = document.getElementById('api-viz-loading');
        
        if (!canvas) return;
        
        // Check if Three.js is available
        if (typeof THREE === 'undefined') {
            console.warn('Three.js not loaded, 3D visualization disabled');
            if (loading) loading.style.display = 'none';
            return;
        }
        
        const container = canvas.parentElement;
        const width = container.clientWidth || 600;
        const height = container.clientHeight || 500;
        
        canvas.width = width;
        canvas.height = height;
        
        // Scene
        apiScene = new THREE.Scene();
        apiScene.background = new THREE.Color(0x0a0f1a);
        
        // Camera
        apiCamera = new THREE.PerspectiveCamera(50, width / height, 0.1, 1000);
        updateCameraPosition();
        
        // Renderer
        apiRenderer = new THREE.WebGLRenderer({ 
            canvas: canvas, 
            antialias: true,
            alpha: false
        });
        apiRenderer.setSize(width, height);
        apiRenderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        apiRenderer.shadowMap.enabled = true;
        apiRenderer.shadowMap.type = THREE.PCFSoftShadowMap;
        
        // Lighting
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
        apiScene.add(ambientLight);
        
        const directionalLight = new THREE.DirectionalLight(0x3b82f6, 0.8);
        directionalLight.position.set(5, 10, 5);
        directionalLight.castShadow = true;
        apiScene.add(directionalLight);
        
        const pointLight = new THREE.PointLight(0x10b981, 0.5, 100);
        pointLight.position.set(-5, 5, -5);
        apiScene.add(pointLight);
        
        // Grid helper
        const gridHelper = new THREE.GridHelper(20, 20, 0x1e293b, 0x0f172a);
        apiScene.add(gridHelper);
        
        // Setup interactive controls
        setup3DControls(canvas);
        
        // Handle window resize
        window.addEventListener('resize', () => {
            if (!apiRenderer || !apiCamera) return;
            const newWidth = container.clientWidth || 600;
            const newHeight = container.clientHeight || 500;
            apiCamera.aspect = newWidth / newHeight;
            apiCamera.updateProjectionMatrix();
            apiRenderer.setSize(newWidth, newHeight);
        });
        
        // Hide loading
        if (loading) loading.style.display = 'none';
        
        // Initial render
        update3DVisualization();
        animate();
    }
    
    /**
     * Update camera position based on rotation
     */
    function updateCameraPosition() {
        if (!apiCamera) return;
        const x = cameraDistance * Math.sin(cameraRotation.phi) * Math.sin(cameraRotation.theta);
        const y = cameraDistance * Math.cos(cameraRotation.phi);
        const z = cameraDistance * Math.sin(cameraRotation.phi) * Math.cos(cameraRotation.theta);
        apiCamera.position.set(x, y, z);
        apiCamera.lookAt(0, 0, 0);
    }
    
    /**
     * Setup 3D interactive controls
     */
    function setup3DControls(canvas) {
        if (!canvas) return;
        
        // Mouse drag to rotate
        canvas.addEventListener('mousedown', (e) => {
            isDragging = true;
            previousMousePosition = { x: e.clientX, y: e.clientY };
            canvas.style.cursor = 'grabbing';
        });
        
        canvas.addEventListener('mousemove', (e) => {
            if (!isDragging) return;
            
            const deltaX = e.clientX - previousMousePosition.x;
            const deltaY = e.clientY - previousMousePosition.y;
            
            cameraRotation.theta -= deltaX * 0.01;
            cameraRotation.phi = Math.max(0.1, Math.min(Math.PI - 0.1, cameraRotation.phi + deltaY * 0.01));
            
            updateCameraPosition();
            previousMousePosition = { x: e.clientX, y: e.clientY };
        });
        
        canvas.addEventListener('mouseup', () => {
            isDragging = false;
            canvas.style.cursor = 'grab';
        });
        
        canvas.addEventListener('mouseleave', () => {
            isDragging = false;
            canvas.style.cursor = 'grab';
        });
        
        // Mouse wheel to zoom
        canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            const zoomSpeed = 0.1;
            const direction = e.deltaY > 0 ? 1 : -1;
            cameraDistance = Math.max(5, Math.min(25, cameraDistance + (direction * zoomSpeed * cameraDistance)));
            updateCameraPosition();
        });
        
        canvas.style.cursor = 'grab';
        
        // Reset camera button
        const resetBtn = document.getElementById('reset-api-viz-camera');
        if (resetBtn) {
            resetBtn.addEventListener('click', () => {
                cameraDistance = 12;
                cameraRotation = { theta: 0, phi: Math.PI / 3 };
                updateCameraPosition();
            });
        }
    }
    
    /**
     * Update 3D visualization based on API status
     */
    function update3DVisualization() {
        if (!apiScene || !apiStatuses || typeof THREE === 'undefined') {
            // Try to initialize if not already done
            if (!apiScene && typeof THREE !== 'undefined') {
                init3DVisualization();
            }
            return;
        }
        
        // Clear existing API objects safely
        try {
            apiObjects.forEach(obj => {
                try {
                    if (obj && apiScene) {
                        apiScene.remove(obj);
                        
                        // Dispose of child objects
                        if (obj.children && obj.children.length > 0) {
                            obj.children.forEach(child => {
                                if (child.geometry && child.geometry.dispose) child.geometry.dispose();
                                if (child.material) {
                                    if (Array.isArray(child.material)) {
                                        child.material.forEach(m => m && m.dispose && m.dispose());
                                    } else if (child.material.dispose) {
                                        child.material.dispose();
                                    }
                                }
                            });
                        }
                        
                        // Dispose of object itself
                        if (obj.geometry && obj.geometry.dispose) obj.geometry.dispose();
                        if (obj.material) {
                            if (Array.isArray(obj.material)) {
                                obj.material.forEach(m => m && m.dispose && m.dispose());
                            } else if (obj.material.dispose) {
                                obj.material.dispose();
                            }
                        }
                    }
                } catch (e) {
                    console.error('Error disposing object:', e);
                }
            });
        } catch (e) {
            console.error('Error clearing objects:', e);
        }
        apiObjects = [];
        
        const apiEntries = Object.entries(apiStatuses).filter(([key]) => 
            key !== 'default' && key !== 'cache_duration'
        );
        
        if (apiEntries.length === 0) {
            // Show placeholder if no APIs
            return;
        }
        
        // Arrange APIs in a circle
        const radius = 4;
        const angleStep = (Math.PI * 2) / apiEntries.length;
        
        apiEntries.forEach(([apiId, status], index) => {
            const angle = index * angleStep;
            const x = radius * Math.cos(angle);
            const z = radius * Math.sin(angle);
            const y = 0;
            
            // Create 3D object for each API
            const apiObject = createAPI3DObject(apiId, status, { x, y, z });
            apiScene.add(apiObject);
            apiObjects.push(apiObject);
        });
    }
    
    /**
     * Create 3D object representing an API
     */
    function createAPI3DObject(apiId, status, position) {
        const group = new THREE.Group();
        group.position.set(position.x, position.y, position.z);
        
        const isAvailable = status.available !== false;
        const limit = status.limit || 60; // Default limit if not set
        const used = status.used || 0;
        const usagePercent = limit > 0 ? (used / limit) * 100 : 0;
        
        // Choose geometry based on API type
        let geometry;
        if (apiId === 'finnhub') {
            geometry = new THREE.OctahedronGeometry(1, 0);
        } else if (apiId === 'yahoo') {
            geometry = new THREE.TetrahedronGeometry(1, 0);
        } else {
            geometry = new THREE.BoxGeometry(1.5, 1.5, 1.5);
        }
        
        // Color based on status
        let color = 0x10b981; // Green for available
        if (!isAvailable) {
            color = 0xef4444; // Red for rate limited
        } else if (usagePercent > 80) {
            color = 0xf59e0b; // Orange for high usage
        } else if (usagePercent > 60) {
            color = 0x3b82f6; // Blue for normal
        }
        
        // Main object
        const material = new THREE.MeshPhongMaterial({
            color: color,
            emissive: color,
            emissiveIntensity: 0.3,
            transparent: true,
            opacity: 0.9
        });
        const mesh = new THREE.Mesh(geometry, material);
        mesh.castShadow = true;
        mesh.receiveShadow = true;
        group.add(mesh);
        
        // Wireframe overlay
        const wireframeMaterial = new THREE.MeshBasicMaterial({
            color: color,
            wireframe: true,
            transparent: true,
            opacity: 0.4
        });
        const wireframe = new THREE.Mesh(geometry, wireframeMaterial);
        group.add(wireframe);
        
        // Usage indicator (bar or ring)
        const usageHeight = Math.max(0.1, (usagePercent / 100) * 2); // Minimum height to avoid zero
        const usageGeometry = new THREE.CylinderGeometry(0.1, 0.1, usageHeight, 16);
        const usageMaterial = new THREE.MeshPhongMaterial({
            color: color,
            emissive: color,
            emissiveIntensity: 0.5
        });
        const usageBar = new THREE.Mesh(usageGeometry, usageMaterial);
        usageBar.position.y = 1 + usageHeight / 2;
        group.add(usageBar);
        
        // Label sprite
        const canvas = document.createElement('canvas');
        canvas.width = 256;
        canvas.height = 64;
        const context = canvas.getContext('2d');
        context.fillStyle = '#ffffff';
        context.font = 'Bold 24px Arial';
        context.textAlign = 'center';
        context.textBaseline = 'middle';
        context.fillText(status.name || apiId, 128, 32);
        
        const texture = new THREE.CanvasTexture(canvas);
        const spriteMaterial = new THREE.SpriteMaterial({ map: texture });
        const sprite = new THREE.Sprite(spriteMaterial);
        sprite.position.y = 2.5;
        sprite.scale.set(2, 0.5, 1);
        group.add(sprite);
        
        // Pulse animation for active APIs
        if (isAvailable) {
            mesh.userData.pulseSpeed = 0.02;
            mesh.userData.originalScale = 1;
        }
        
        return group;
    }
    
    /**
     * Animation loop
     */
    function animate() {
        if (!apiScene || !apiCamera || !apiRenderer || typeof THREE === 'undefined') {
            if (apiAnimationId) {
                cancelAnimationFrame(apiAnimationId);
                apiAnimationId = null;
            }
            return;
        }
        
        try {
            apiAnimationId = requestAnimationFrame(animate);
            
            // Rotate API objects
            apiObjects.forEach((obj, index) => {
                if (obj && obj.rotation) {
                    obj.rotation.y += 0.01;
                    obj.rotation.x += 0.005;
                    
                    // Pulse effect for active APIs
                    const mesh = obj.children.find(child => child instanceof THREE.Mesh && !child.material.wireframe);
                    if (mesh && mesh.userData && mesh.userData.pulseSpeed) {
                        const pulse = Math.sin(Date.now() * mesh.userData.pulseSpeed) * 0.1;
                        const scale = (mesh.userData.originalScale || 1) + pulse;
                        mesh.scale.set(scale, scale, scale);
                    }
                }
            });
            
            apiRenderer.render(apiScene, apiCamera);
        } catch (error) {
            console.error('Error in animation loop:', error);
            if (apiAnimationId) {
                cancelAnimationFrame(apiAnimationId);
                apiAnimationId = null;
            }
        }
    }
    
    /**
     * Cleanup 3D visualization
     */
    function cleanup3DVisualization() {
        try {
            if (apiAnimationId) {
                cancelAnimationFrame(apiAnimationId);
                apiAnimationId = null;
            }
            
            if (apiScene && typeof THREE !== 'undefined') {
                apiObjects.forEach(obj => {
                    try {
                        if (obj && obj.children) {
                            obj.children.forEach(child => {
                                if (child.geometry) child.geometry.dispose();
                                if (child.material) {
                                    if (Array.isArray(child.material)) {
                                        child.material.forEach(m => m.dispose());
                                    } else {
                                        child.material.dispose();
                                    }
                                }
                            });
                        }
                        if (obj.geometry) obj.geometry.dispose();
                        if (obj.material) {
                            if (Array.isArray(obj.material)) {
                                obj.material.forEach(m => m.dispose());
                            } else {
                                obj.material.dispose();
                            }
                        }
                        apiScene.remove(obj);
                    } catch (e) {
                        console.error('Error disposing object:', e);
                    }
                });
            }
            apiObjects = [];
            
            if (apiRenderer) {
                apiRenderer.dispose();
                apiRenderer = null;
            }
            
            apiScene = null;
            apiCamera = null;
        } catch (error) {
            console.error('Error cleaning up 3D visualization:', error);
        }
    }
    
    // Public API
    return {
        init: init,
        openApiModal: openApiModal,
        editApi: editApi,
        deleteApi: deleteApi,
        toggleApi: toggleApi,
        testApi: testApi,
        loadApiStatus: loadApiStatus,
        loadApiConfigurations: loadApiConfigurations,
        update3DVisualization: update3DVisualization,
        cleanup3DVisualization: cleanup3DVisualization
    };
})();

// Track initialization state
let apiModuleInitialized = false;

// Initialize when DOM is ready and API page is active
function initApiManagement() {
    try {
        const apiPage = document.getElementById('page-api');
        if (apiPage && apiPage.classList.contains('active')) {
            if (!apiModuleInitialized) {
                ApiManagementModule.init();
                apiModuleInitialized = true;
            } else {
                // Refresh data if already initialized
                ApiManagementModule.loadApiStatus();
                ApiManagementModule.loadApiConfigurations();
            }
        }
    } catch (error) {
        console.error('Error initializing API management:', error);
    }
}

// Initialize on DOM ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initApiManagement);
} else {
    initApiManagement();
}

// Also listen for page changes
document.addEventListener('DOMContentLoaded', () => {
    try {
        // Watch for page visibility changes
        const observer = new MutationObserver(() => {
            try {
                const apiPage = document.getElementById('page-api');
                if (apiPage && apiPage.classList.contains('active')) {
                    if (!apiModuleInitialized) {
                        ApiManagementModule.init();
                        apiModuleInitialized = true;
                    } else {
                        // Refresh data when page becomes active
                        ApiManagementModule.loadApiStatus();
                        ApiManagementModule.loadApiConfigurations();
                        // Update 3D visualization when page becomes active
                        if (typeof ApiManagementModule.update3DVisualization === 'function') {
                            ApiManagementModule.update3DVisualization();
                        }
                    }
                }
            } catch (error) {
                console.error('Error in page observer:', error);
            }
        });
        
        const apiPage = document.getElementById('page-api');
        if (apiPage) {
            observer.observe(apiPage, {
                attributes: true,
                attributeFilter: ['class']
            });
        }
    } catch (error) {
        console.error('Error setting up page observer:', error);
    }
});

