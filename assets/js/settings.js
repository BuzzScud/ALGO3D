// Settings Management

// Load settings from API
async function loadSettings() {
    try {
        const response = await fetch('api/settings.php');
        if (!response.ok) throw new Error('Failed to load settings');
        
        const settings = await response.json();
        applySettings(settings);
    } catch (error) {
        console.error('Error loading settings:', error);
        // Load from localStorage as fallback
        loadLocalSettings();
    }
    
    // Load theme from localStorage (priority)
    const savedTheme = localStorage.getItem('algo3d_theme') || 'dark';
    applyTheme(savedTheme);
    
    // Update settings page theme select
    const themeSelect = document.getElementById('setting-theme');
    if (themeSelect) {
        themeSelect.value = savedTheme;
    }
}

// Apply settings to UI
function applySettings(settings) {
    // Finnhub API Key (keep as password type for security)
    const finnhubKeyInput = document.getElementById('setting-finnhub-key');
    if (finnhubKeyInput && settings.finnhub_key) {
        // Only set value if it's different to avoid showing it
        if (finnhubKeyInput.value !== settings.finnhub_key) {
            finnhubKeyInput.value = settings.finnhub_key;
        }
        // Ensure it stays as password type
        finnhubKeyInput.type = 'password';
    }
    
    // Default API
    const defaultApiSelect = document.getElementById('setting-default-api');
    if (defaultApiSelect && settings.default_api) {
        defaultApiSelect.value = settings.default_api;
    }
    
    // Refresh interval
    const refreshIntervalInput = document.getElementById('setting-refresh-interval');
    const refreshIntervalRange = document.getElementById('setting-refresh-interval-range');
    if (refreshIntervalInput && settings.refresh_interval) {
        refreshIntervalInput.value = settings.refresh_interval;
        if (refreshIntervalRange) {
            refreshIntervalRange.value = settings.refresh_interval;
        }
    }
    
    // Theme
    const themeSelect = document.getElementById('setting-theme');
    if (themeSelect && settings.theme) {
        themeSelect.value = settings.theme;
        applyTheme(settings.theme);
    }
    
    // Show ticker
    const showTickerCheckbox = document.getElementById('setting-show-ticker');
    if (showTickerCheckbox) {
        showTickerCheckbox.checked = settings.show_ticker === 'true';
        toggleTickerTape(showTickerCheckbox.checked);
    }
    
    // Compact cards
    const compactCardsCheckbox = document.getElementById('setting-compact-cards');
    if (compactCardsCheckbox) {
        compactCardsCheckbox.checked = settings.compact_cards === 'true';
    }
    
    // Notifications
    const notificationsCheckbox = document.getElementById('setting-notifications');
    if (notificationsCheckbox) {
        notificationsCheckbox.checked = settings.notifications === 'true';
    }
    
    // Sound alerts
    const soundAlertsCheckbox = document.getElementById('setting-sound-alerts');
    if (soundAlertsCheckbox) {
        soundAlertsCheckbox.checked = settings.sound_alerts === 'true';
    }
    
    // Store in localStorage for quick access
    localStorage.setItem('algo3d_settings', JSON.stringify(settings));
}

// Load settings from localStorage
function loadLocalSettings() {
    const stored = localStorage.getItem('algo3d_settings');
    if (stored) {
        try {
            const settings = JSON.parse(stored);
            applySettings(settings);
        } catch (e) {
            console.error('Error parsing stored settings');
        }
    }
}

// Save settings
async function saveSettings() {
    const settings = {
        finnhub_key: document.getElementById('setting-finnhub-key')?.value || '',
        default_api: document.getElementById('setting-default-api')?.value || 'finnhub',
        refresh_interval: document.getElementById('setting-refresh-interval')?.value || '60',
        theme: document.getElementById('setting-theme')?.value || 'dark',
        show_ticker: document.getElementById('setting-show-ticker')?.checked ? 'true' : 'false',
        compact_cards: document.getElementById('setting-compact-cards')?.checked ? 'true' : 'false',
        notifications: document.getElementById('setting-notifications')?.checked ? 'true' : 'false',
        sound_alerts: document.getElementById('setting-sound-alerts')?.checked ? 'true' : 'false'
    };
    
    try {
        const response = await fetch('api/settings.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(settings)
        });

        const result = await response.json();
        
        if (result.success) {
            // Store locally
            localStorage.setItem('algo3d_settings', JSON.stringify(settings));
            
            // Apply settings
            applyTheme(settings.theme);
            toggleTickerTape(settings.show_ticker === 'true');
            
            alert('Settings saved successfully!');
        } else {
            alert(result.message || 'Error saving settings');
        }
    } catch (error) {
        console.error('Error saving settings:', error);
        // Save locally even if API fails
        localStorage.setItem('algo3d_settings', JSON.stringify(settings));
        alert('Settings saved locally');
    }
}

// Apply theme
function applyTheme(theme) {
    document.body.setAttribute('data-theme', theme);
    
    // Sync sidebar toggle
    const sidebarToggle = document.getElementById('sidebar-theme-toggle');
    if (sidebarToggle) {
        sidebarToggle.checked = theme === 'light';
        updateThemeToggleLabel(theme === 'dark');
    }
    
    // Update TradingView widget theme
    const tradingViewScript = document.querySelector('script[src*="ticker-tape"]');
    if (tradingViewScript && tradingViewScript.innerHTML) {
        try {
            const config = JSON.parse(tradingViewScript.innerHTML.trim());
            config.colorTheme = theme;
            tradingViewScript.innerHTML = JSON.stringify(config);
        } catch (e) {
            // Script might not be ready yet
        }
    }
}

// Update theme toggle label (shared function)
function updateThemeToggleLabel(isDark) {
    const themeLabel = document.querySelector('.theme-toggle-label span');
    const themeIcon = document.querySelector('.theme-toggle-label i');
    
    if (themeLabel) {
        themeLabel.textContent = isDark ? 'Dark Mode' : 'Light Mode';
    }
    
    if (themeIcon) {
        themeIcon.className = isDark ? 'fas fa-moon' : 'fas fa-sun';
    }
}

// Toggle ticker tape visibility
function toggleTickerTape(show) {
    const tickerTape = document.querySelector('.ticker-tape-container');
    if (tickerTape) {
        tickerTape.style.display = show ? 'block' : 'none';
    }
}

// Clear cache
async function clearCache() {
    try {
        const response = await fetch('api/clear_cache.php');
        const result = await response.json();
        
        if (result.success) {
            alert(result.message);
        } else {
            alert(result.message || 'Error clearing cache');
        }
    } catch (error) {
        console.error('Error clearing cache:', error);
        alert('Error clearing cache');
    }
}

// Export data
async function exportData() {
    try {
        const response = await fetch('api/export_data.php');
        const data = await response.json();
        
        // Create download
        const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `algo3d_export_${new Date().toISOString().split('T')[0]}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        alert('Data exported successfully!');
    } catch (error) {
        console.error('Error exporting data:', error);
        alert('Error exporting data');
    }
}

// Reset all data
async function resetData() {
    if (!confirm('Are you sure you want to reset ALL data? This cannot be undone!')) {
        return;
    }
    
    if (!confirm('This will delete all your todos, notes, symbols, and settings. Are you absolutely sure?')) {
        return;
    }

    try {
        const response = await fetch('api/reset_data.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ confirm: true })
        });

        const result = await response.json();
        
        if (result.success) {
            localStorage.clear();
            alert('All data has been reset. The page will reload.');
            window.location.reload();
        } else {
            alert(result.message || 'Error resetting data');
        }
    } catch (error) {
        console.error('Error resetting data:', error);
        alert('Error resetting data');
    }
}

// Setup event listeners
function setupSettingsListeners() {
    // Save settings button
    const saveSettingsBtn = document.getElementById('save-settings-btn');
    if (saveSettingsBtn) {
        saveSettingsBtn.addEventListener('click', saveSettings);
    }

    // Clear cache button
    const clearCacheBtn = document.getElementById('clear-cache-btn');
    if (clearCacheBtn) {
        clearCacheBtn.addEventListener('click', clearCache);
    }

    // Export data button
    const exportDataBtn = document.getElementById('export-data-btn');
    if (exportDataBtn) {
        exportDataBtn.addEventListener('click', exportData);
    }

    // Reset data button
    const resetDataBtn = document.getElementById('reset-data-btn');
    if (resetDataBtn) {
        resetDataBtn.addEventListener('click', resetData);
    }

    // Theme change
    const themeSelect = document.getElementById('setting-theme');
    if (themeSelect) {
        themeSelect.addEventListener('change', function() {
            applyTheme(this.value);
            localStorage.setItem('algo3d_theme', this.value);
            
            // Sync sidebar toggle
            const sidebarToggle = document.getElementById('sidebar-theme-toggle');
            if (sidebarToggle) {
                sidebarToggle.checked = this.value === 'light';
            }
        });
    }

    // Show ticker change
    const showTickerCheckbox = document.getElementById('setting-show-ticker');
    if (showTickerCheckbox) {
        showTickerCheckbox.addEventListener('change', function() {
            toggleTickerTape(this.checked);
        });
    }

    // Range slider sync
    const refreshIntervalRange = document.getElementById('setting-refresh-interval-range');
    const refreshIntervalInput = document.getElementById('setting-refresh-interval');
    
    if (refreshIntervalRange && refreshIntervalInput) {
        // Sync range to input
        refreshIntervalRange.addEventListener('input', function() {
            refreshIntervalInput.value = this.value;
        });
        
        // Sync input to range
        refreshIntervalInput.addEventListener('input', function() {
            let value = parseInt(this.value);
            if (value < 30) value = 30;
            if (value > 300) value = 300;
            this.value = value;
            refreshIntervalRange.value = value;
        });
    }
}

// Initialize IP Whitelist functionality
function initIPWhitelist() {
    const whitelistBtn = document.getElementById('whitelist-ip-btn');
    const ipDisplay = document.getElementById('current-ip-display');
    const statusMessage = document.getElementById('ip-status-message');
    const btnText = document.getElementById('whitelist-btn-text');
    
    if (!whitelistBtn) return;
    
    // Get current IP on page load
    async function getCurrentIP() {
        try {
            const response = await fetch('practice/access/index.php?format=json', {
                method: 'GET',
                headers: {
                    'Accept': 'application/json'
                }
            });
            if (response.ok) {
                const result = await response.json();
                if (result.success && result.ip && ipDisplay) {
                    ipDisplay.textContent = result.ip;
                    ipDisplay.style.color = 'var(--primary-color)';
                }
            }
        } catch (error) {
            console.log('Could not detect IP initially:', error);
            if (ipDisplay) {
                ipDisplay.textContent = 'Click button to detect';
                ipDisplay.style.color = 'var(--text-secondary)';
            }
        }
    }
    
    // Try to get IP when tab is first shown
    getCurrentIP();
    
    // Handle button click
    whitelistBtn.addEventListener('click', async function() {
        if (this.disabled) return;
        
        // Disable button and show loading
        this.disabled = true;
        if (btnText) {
            btnText.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';
        }
        
        // Hide previous status
        if (statusMessage) {
            statusMessage.style.display = 'none';
            statusMessage.className = 'ip-status-message';
        }
        
        try {
            const response = await fetch('practice/access/index.php', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.success) {
                // Show success message
                if (statusMessage) {
                    statusMessage.className = 'ip-status-message success';
                    statusMessage.innerHTML = `<i class="fas fa-check-circle"></i> ${result.message} - IP: ${result.ip}`;
                    statusMessage.style.display = 'block';
                }
                
                // Update IP display
                if (ipDisplay && result.ip) {
                    ipDisplay.textContent = result.ip;
                    ipDisplay.style.color = 'var(--primary-color)';
                }
            } else {
                throw new Error(result.message || 'Failed to save IP address');
            }
        } catch (error) {
            console.error('Error saving IP:', error);
            if (statusMessage) {
                statusMessage.className = 'ip-status-message error';
                statusMessage.innerHTML = `<i class="fas fa-exclamation-circle"></i> Error: ${error.message}`;
                statusMessage.style.display = 'block';
            }
        } finally {
            // Re-enable button
            this.disabled = false;
            if (btnText) {
                btnText.innerHTML = 'Add My IP to Whitelist';
            }
        }
    });
    
    // Try to detect IP when tab is shown
    const ipButtonTab = document.querySelector('.settings-tab[data-tab="ip-button"]');
    if (ipButtonTab) {
        ipButtonTab.addEventListener('click', function() {
            // When tab is clicked, try to get IP
            setTimeout(() => {
                getCurrentIP();
            }, 100);
        });
    }
}

// Initialize settings
function initSettings() {
    loadSettings();
    setupSettingsListeners();
    initIPWhitelist();
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initSettings);
} else {
    initSettings();
}

