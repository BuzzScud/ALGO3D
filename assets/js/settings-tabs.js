// Settings Tabs Management

// Initialize settings tabs
function initSettingsTabs() {
    const tabs = document.querySelectorAll('.settings-tab');
    const tabContents = document.querySelectorAll('.settings-tab-content');
    
    tabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const targetTab = this.dataset.tab;
            
            // Remove active class from all tabs
            tabs.forEach(t => t.classList.remove('active'));
            // Add active class to clicked tab
            this.classList.add('active');
            
            // Hide all tab contents
            tabContents.forEach(content => {
                content.classList.remove('active');
            });
            
            // Show target tab content
            const targetContent = document.getElementById(`tab-${targetTab}`);
            if (targetContent) {
                targetContent.classList.add('active');
            }
        });
    });
}

// API Key Toggle
function initApiKeyToggle() {
    const toggleBtn = document.getElementById('toggle-api-key');
    const apiKeyInput = document.getElementById('setting-finnhub-key');
    
    if (toggleBtn && apiKeyInput) {
        toggleBtn.addEventListener('click', function() {
            const isPassword = apiKeyInput.type === 'password';
            apiKeyInput.type = isPassword ? 'text' : 'password';
            
            const icon = this.querySelector('i');
            if (icon) {
                icon.className = isPassword ? 'fas fa-eye-slash' : 'fas fa-eye';
            }
            
            this.title = isPassword ? 'Hide API Key' : 'Show API Key';
        });
    }
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
        initSettingsTabs();
        initApiKeyToggle();
    });
} else {
    initSettingsTabs();
    initApiKeyToggle();
}
















