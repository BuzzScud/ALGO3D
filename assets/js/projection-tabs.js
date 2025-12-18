/**
 * Projection Configuration Tabs Module
 * Handles tab navigation for the unified projection configuration panel
 */

const ProjectionTabs = (function() {
    'use strict';
    
    /**
     * Setup tab navigation for projection configuration
     */
    function setupTabs() {
        const tabButtons = document.querySelectorAll('.projection-tab-btn');
        const tabContents = document.querySelectorAll('.projection-tab-content');
        
        tabButtons.forEach(btn => {
            btn.addEventListener('click', function() {
                const targetTab = this.getAttribute('data-tab');
                
                // Remove active class from all buttons and contents
                tabButtons.forEach(b => b.classList.remove('active'));
                tabContents.forEach(c => c.classList.remove('active'));
                
                // Add active class to clicked button and corresponding content
                this.classList.add('active');
                const targetContent = document.getElementById(`projection-tab-${targetTab}`);
                if (targetContent) {
                    targetContent.classList.add('active');
                }
            });
        });
    }
    
    /**
     * Switch to a specific tab
     */
    function switchToTab(tabName) {
        const tabBtn = document.querySelector(`.projection-tab-btn[data-tab="${tabName}"]`);
        const tabContent = document.getElementById(`projection-tab-${tabName}`);
        
        if (tabBtn && tabContent) {
            // Remove active from all
            document.querySelectorAll('.projection-tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.projection-tab-content').forEach(c => c.classList.remove('active'));
            
            // Add active to target
            tabBtn.classList.add('active');
            tabContent.classList.add('active');
        }
    }
    
    /**
     * Initialize
     */
    function init() {
        setupTabs();
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    return {
        switchToTab: switchToTab,
        init: init
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.ProjectionTabs = ProjectionTabs;
}





