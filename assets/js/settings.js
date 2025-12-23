// Settings Management

// Toast Notification System
function showToast(message, type = 'success', duration = 3000) {
    const container = document.getElementById('toast-container');
    if (!container) return;
    
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    
    const icons = {
        success: 'fa-check-circle',
        error: 'fa-exclamation-circle',
        warning: 'fa-exclamation-triangle',
        info: 'fa-info-circle'
    };
    
    toast.innerHTML = `
        <i class="fas ${icons[type] || icons.info}"></i>
        <span>${message}</span>
        <button class="toast-close" onclick="this.parentElement.remove()">
            <i class="fas fa-times"></i>
        </button>
    `;
    
    container.appendChild(toast);
    
    // Trigger animation
    setTimeout(() => toast.classList.add('show'), 10);
    
    // Auto remove
    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => toast.remove(), 300);
    }, duration);
}

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
    
    // Font size
    const fontSizeInput = document.getElementById('setting-font-size');
    const fontSizeRange = document.getElementById('setting-font-size-range');
    if (fontSizeInput && settings.font_size) {
        fontSizeInput.value = settings.font_size;
        if (fontSizeRange) {
            fontSizeRange.value = settings.font_size;
        }
        applyFontSize(settings.font_size);
    }
    
    // Animations
    const animationsCheckbox = document.getElementById('setting-animations');
    if (animationsCheckbox) {
        animationsCheckbox.checked = settings.animations !== 'false';
        applyAnimations(settings.animations !== 'false');
    }
    
    // Tooltips
    const tooltipsCheckbox = document.getElementById('setting-tooltips');
    if (tooltipsCheckbox) {
        tooltipsCheckbox.checked = settings.tooltips !== 'false';
    }
    
    // Notification position
    const notificationPositionSelect = document.getElementById('setting-notification-position');
    if (notificationPositionSelect && settings.notification_position) {
        notificationPositionSelect.value = settings.notification_position;
    }
    
    // Notification duration
    const notificationDurationInput = document.getElementById('setting-notification-duration');
    const notificationDurationRange = document.getElementById('setting-notification-duration-range');
    if (notificationDurationInput && settings.notification_duration) {
        notificationDurationInput.value = settings.notification_duration;
        if (notificationDurationRange) {
            notificationDurationRange.value = settings.notification_duration;
        }
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

// Auto-save settings with debounce
let autoSaveTimeout = null;
function autoSaveSettings() {
    clearTimeout(autoSaveTimeout);
    autoSaveTimeout = setTimeout(() => {
        saveSettings(true); // true = silent save
    }, 2000); // Wait 2 seconds after last change
    
    // Show indicator
    const indicator = document.getElementById('auto-save-indicator');
    if (indicator) {
        indicator.style.display = 'flex';
        setTimeout(() => {
            indicator.style.display = 'none';
        }, 3000);
    }
}

// Save settings
async function saveSettings(silent = false) {
    const settings = {
        theme: document.getElementById('setting-theme')?.value || 'dark',
        show_ticker: document.getElementById('setting-show-ticker')?.checked ? 'true' : 'false',
        compact_cards: document.getElementById('setting-compact-cards')?.checked ? 'true' : 'false',
        notifications: document.getElementById('setting-notifications')?.checked ? 'true' : 'false',
        sound_alerts: document.getElementById('setting-sound-alerts')?.checked ? 'true' : 'false',
        font_size: document.getElementById('setting-font-size')?.value || '14',
        animations: document.getElementById('setting-animations')?.checked ? 'true' : 'false',
        tooltips: document.getElementById('setting-tooltips')?.checked ? 'true' : 'false',
        notification_position: document.getElementById('setting-notification-position')?.value || 'top-right',
        notification_duration: document.getElementById('setting-notification-duration')?.value || '5'
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
            applyFontSize(settings.font_size);
            applyAnimations(settings.animations === 'true');
            
            if (!silent) {
                showToast('Settings saved successfully!', 'success');
            }
        } else {
            if (!silent) {
                showToast(result.message || 'Error saving settings', 'error');
            }
        }
    } catch (error) {
        console.error('Error saving settings:', error);
        // Save locally even if API fails
        localStorage.setItem('algo3d_settings', JSON.stringify(settings));
        if (!silent) {
            showToast('Settings saved locally', 'warning');
        }
    }
}

// Apply font size
function applyFontSize(size) {
    if (!size) return;
    document.documentElement.style.setProperty('--base-font-size', `${size}px`);
}

// Apply animations
function applyAnimations(enabled) {
    if (enabled) {
        document.body.classList.remove('no-animations');
    } else {
        document.body.classList.add('no-animations');
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
            showToast(result.message || 'Cache cleared successfully', 'success');
        } else {
            showToast(result.message || 'Error clearing cache', 'error');
        }
    } catch (error) {
        console.error('Error clearing cache:', error);
        showToast('Error clearing cache', 'error');
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
        
        showToast('Data exported successfully!', 'success');
    } catch (error) {
        console.error('Error exporting data:', error);
        showToast('Error exporting data', 'error');
    }
}

// Export settings
function exportSettings() {
    try {
        const settings = {
            theme: document.getElementById('setting-theme')?.value || 'dark',
            show_ticker: document.getElementById('setting-show-ticker')?.checked ? 'true' : 'false',
            compact_cards: document.getElementById('setting-compact-cards')?.checked ? 'true' : 'false',
            notifications: document.getElementById('setting-notifications')?.checked ? 'true' : 'false',
            sound_alerts: document.getElementById('setting-sound-alerts')?.checked ? 'true' : 'false',
            font_size: document.getElementById('setting-font-size')?.value || '14',
            animations: document.getElementById('setting-animations')?.checked ? 'true' : 'false',
            tooltips: document.getElementById('setting-tooltips')?.checked ? 'true' : 'false',
            notification_position: document.getElementById('setting-notification-position')?.value || 'top-right',
            notification_duration: document.getElementById('setting-notification-duration')?.value || '5',
            exported_at: new Date().toISOString()
        };
        
        const blob = new Blob([JSON.stringify(settings, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `algo3d_settings_${new Date().toISOString().split('T')[0]}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        showToast('Settings exported successfully!', 'success');
    } catch (error) {
        console.error('Error exporting settings:', error);
        showToast('Error exporting settings', 'error');
    }
}

// Import settings
function importSettings() {
    const fileInput = document.getElementById('import-settings-file');
    if (!fileInput) return;
    
    fileInput.click();
    fileInput.onchange = function(e) {
        const file = e.target.files[0];
        if (!file) return;
        
        const reader = new FileReader();
        reader.onload = function(event) {
            try {
                const settings = JSON.parse(event.target.result);
                
                // Apply imported settings
                if (settings.theme) {
                    document.getElementById('setting-theme').value = settings.theme;
                }
                if (settings.show_ticker !== undefined) {
                    document.getElementById('setting-show-ticker').checked = settings.show_ticker === 'true';
                }
                if (settings.compact_cards !== undefined) {
                    document.getElementById('setting-compact-cards').checked = settings.compact_cards === 'true';
                }
                if (settings.notifications !== undefined) {
                    document.getElementById('setting-notifications').checked = settings.notifications === 'true';
                }
                if (settings.sound_alerts !== undefined) {
                    document.getElementById('setting-sound-alerts').checked = settings.sound_alerts === 'true';
                }
                if (settings.font_size) {
                    document.getElementById('setting-font-size').value = settings.font_size;
                    document.getElementById('setting-font-size-range').value = settings.font_size;
                }
                if (settings.animations !== undefined) {
                    document.getElementById('setting-animations').checked = settings.animations === 'true';
                }
                if (settings.tooltips !== undefined) {
                    document.getElementById('setting-tooltips').checked = settings.tooltips === 'true';
                }
                if (settings.notification_position) {
                    document.getElementById('setting-notification-position').value = settings.notification_position;
                }
                if (settings.notification_duration) {
                    document.getElementById('setting-notification-duration').value = settings.notification_duration;
                    document.getElementById('setting-notification-duration-range').value = settings.notification_duration;
                }
                
                // Apply settings immediately
                applySettings(settings);
                saveSettings();
                
                showToast('Settings imported successfully!', 'success');
            } catch (error) {
                console.error('Error importing settings:', error);
                showToast('Error importing settings. Invalid file format.', 'error');
            }
        };
        reader.readAsText(file);
    };
}

// Reset settings to defaults
function resetSettingsToDefaults() {
    if (!confirm('Are you sure you want to reset all settings to their default values?')) {
        return;
    }
    
    // Reset to defaults
    document.getElementById('setting-theme').value = 'dark';
    document.getElementById('setting-show-ticker').checked = true;
    document.getElementById('setting-compact-cards').checked = false;
    document.getElementById('setting-notifications').checked = false;
    document.getElementById('setting-sound-alerts').checked = false;
    document.getElementById('setting-font-size').value = '14';
    document.getElementById('setting-font-size-range').value = '14';
    document.getElementById('setting-animations').checked = true;
    document.getElementById('setting-tooltips').checked = true;
    document.getElementById('setting-notification-position').value = 'top-right';
    document.getElementById('setting-notification-duration').value = '5';
    document.getElementById('setting-notification-duration-range').value = '5';
    
    // Apply and save
    const defaults = {
        theme: 'dark',
        show_ticker: 'true',
        compact_cards: 'false',
        notifications: 'false',
        sound_alerts: 'false',
        font_size: '14',
        animations: 'true',
        tooltips: 'true',
        notification_position: 'top-right',
        notification_duration: '5'
    };
    
    applySettings(defaults);
    saveSettings();
    showToast('Settings reset to defaults', 'success');
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
            showToast('All data has been reset. The page will reload.', 'success', 2000);
            setTimeout(() => window.location.reload(), 2000);
        } else {
            showToast(result.message || 'Error resetting data', 'error');
        }
    } catch (error) {
        console.error('Error resetting data:', error);
        showToast('Error resetting data', 'error');
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
            autoSaveSettings();
        });
    }
    
    // Font size range sync
    const fontSizeRange = document.getElementById('setting-font-size-range');
    const fontSizeInput = document.getElementById('setting-font-size');
    if (fontSizeRange && fontSizeInput) {
        fontSizeRange.addEventListener('input', function() {
            fontSizeInput.value = this.value;
            applyFontSize(this.value);
            autoSaveSettings();
        });
        fontSizeInput.addEventListener('input', function() {
            let value = parseInt(this.value);
            if (value < 12) value = 12;
            if (value > 18) value = 18;
            this.value = value;
            fontSizeRange.value = value;
            applyFontSize(value);
            autoSaveSettings();
        });
    }
    
    // Notification duration range sync
    const notificationDurationRange = document.getElementById('setting-notification-duration-range');
    const notificationDurationInput = document.getElementById('setting-notification-duration');
    if (notificationDurationRange && notificationDurationInput) {
        notificationDurationRange.addEventListener('input', function() {
            notificationDurationInput.value = this.value;
            autoSaveSettings();
        });
        notificationDurationInput.addEventListener('input', function() {
            let value = parseInt(this.value);
            if (value < 2) value = 2;
            if (value > 10) value = 10;
            this.value = value;
            notificationDurationRange.value = value;
            autoSaveSettings();
        });
    }
    
    // Add auto-save listeners for all inputs
    const autoSaveInputs = [
        'setting-theme',
        'setting-show-ticker',
        'setting-compact-cards',
        'setting-notifications',
        'setting-sound-alerts',
        'setting-animations',
        'setting-tooltips',
        'setting-notification-position'
    ];
    
    autoSaveInputs.forEach(id => {
        const element = document.getElementById(id);
        if (element) {
            element.addEventListener('change', autoSaveSettings);
        }
    });
    
    // Export/Import settings buttons
    const exportSettingsBtn = document.getElementById('export-settings-btn');
    if (exportSettingsBtn) {
        exportSettingsBtn.addEventListener('click', exportSettings);
    }
    
    const importSettingsBtn = document.getElementById('import-settings-btn');
    if (importSettingsBtn) {
        importSettingsBtn.addEventListener('click', importSettings);
    }
    
    // Reset settings button
    const resetSettingsBtn = document.getElementById('reset-settings-btn');
    const resetSettingsQuickBtn = document.getElementById('reset-settings-quick-btn');
    if (resetSettingsBtn) {
        resetSettingsBtn.addEventListener('click', resetSettingsToDefaults);
    }
    if (resetSettingsQuickBtn) {
        resetSettingsQuickBtn.addEventListener('click', resetSettingsToDefaults);
    }
    
    // Initialize About tab
    initAboutTab();
    
    // Initialize keyboard shortcuts
    initKeyboardShortcuts();
}

// Initialize About tab
function initAboutTab() {
    // Detect browser
    const browserInfo = navigator.userAgent;
    let browserName = 'Unknown';
    if (browserInfo.includes('Chrome')) browserName = 'Chrome';
    else if (browserInfo.includes('Firefox')) browserName = 'Firefox';
    else if (browserInfo.includes('Safari')) browserName = 'Safari';
    else if (browserInfo.includes('Edge')) browserName = 'Edge';
    
    const browserEl = document.getElementById('app-browser');
    if (browserEl) {
        browserEl.textContent = browserName;
    }
    
    // Detect screen resolution
    const resolutionEl = document.getElementById('app-resolution');
    if (resolutionEl) {
        resolutionEl.textContent = `${window.screen.width}x${window.screen.height}`;
    }
    
    // Check localStorage
    const storageEl = document.getElementById('app-storage');
    if (storageEl) {
        try {
            let totalSize = 0;
            for (let key in localStorage) {
                if (localStorage.hasOwnProperty(key)) {
                    totalSize += localStorage[key].length + key.length;
                }
            }
            storageEl.textContent = `${(totalSize / 1024).toFixed(2)} KB used`;
        } catch (e) {
            storageEl.textContent = 'Unable to calculate';
        }
    }
    
    // Copy system info
    const copySystemInfoBtn = document.getElementById('copy-system-info-btn');
    if (copySystemInfoBtn) {
        copySystemInfoBtn.addEventListener('click', function() {
            const systemInfo = {
                browser: browserName,
                userAgent: navigator.userAgent,
                screenResolution: `${window.screen.width}x${window.screen.height}`,
                viewport: `${window.innerWidth}x${window.innerHeight}`,
                platform: navigator.platform,
                language: navigator.language,
                localStorage: storageEl.textContent,
                timestamp: new Date().toISOString()
            };
            
            const text = JSON.stringify(systemInfo, null, 2);
            navigator.clipboard.writeText(text).then(() => {
                showToast('System info copied to clipboard', 'success');
            }).catch(() => {
                showToast('Failed to copy system info', 'error');
            });
        });
    }
}

// Initialize keyboard shortcuts
function initKeyboardShortcuts() {
    document.addEventListener('keydown', function(e) {
        // Ctrl/Cmd + S to save settings
        if ((e.ctrlKey || e.metaKey) && e.key === 's') {
            const settingsPage = document.getElementById('page-settings');
            if (settingsPage && settingsPage.classList.contains('active')) {
                e.preventDefault();
                saveSettings();
            }
        }
        
        // Ctrl/Cmd + , to open settings
        if ((e.ctrlKey || e.metaKey) && e.key === ',') {
            e.preventDefault();
            const settingsItem = document.querySelector('.sidebar-item[data-page="settings"]');
            if (settingsItem) {
                settingsItem.click();
            }
        }
        
        // Ctrl/Cmd + D to go to dashboard
        if ((e.ctrlKey || e.metaKey) && e.key === 'd') {
            e.preventDefault();
            const dashboardItem = document.querySelector('.sidebar-item[data-page="dashboard"]');
            if (dashboardItem) {
                dashboardItem.click();
            }
        }
    });
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

