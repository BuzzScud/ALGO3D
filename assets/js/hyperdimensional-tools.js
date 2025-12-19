/**
 * Hyper-Dimensional Tools Module
 * Integrates tetration towers and platonic solids with projections
 */

const HyperdimensionalTools = (function() {
    'use strict';
    
    let currentTetrationTowers = [];
    let currentPlatonicSolids = [];
    let discoveredSolids = [];
    
    /**
     * Generate tetration towers
     */
    function generateTetrationTowers() {
        const bases = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31];
        const minDepth = 10;
        const maxDepth = 30;
        
        try {
            currentTetrationTowers = TetrationTowers.createTowerSet(bases, minDepth, maxDepth);
            
            const results = {
                totalTowers: currentTetrationTowers.length,
                converged: currentTetrationTowers.filter(t => t.isConverged).length,
                divergent: currentTetrationTowers.filter(t => !t.isConverged).length,
                towers: currentTetrationTowers.slice(0, 10).map(t => ({
                    base: t.base,
                    depth: t.depth,
                    converged: t.isConverged,
                    logValue: t.logValue.toFixed(4),
                    value: t.getValue() < 1e100 ? t.getValue().toExponential(4) : 'Too Large'
                }))
            };
            
            displayResults('Tetration Towers Generated', results);
            return currentTetrationTowers;
        } catch (error) {
            console.error('Error generating tetration towers:', error);
            showError('Failed to generate tetration towers');
            return [];
        }
    }
    
    /**
     * Generate platonic solids
     */
    function generatePlatonicSolids() {
        const numDimensions = 13; // Start with 13D
        
        try {
            currentPlatonicSolids = PlatonicSolidGenerator.generateAllSolids(numDimensions);
            
            const results = {
                totalSolids: currentPlatonicSolids.length,
                dimensions: numDimensions,
                solids: currentPlatonicSolids.map(s => ({
                    name: s.metadata.name,
                    vertices: s.vertices.length,
                    dimensions: s.numDimensions,
                    properties: s.calculateProperties()
                }))
            };
            
            displayResults('Platonic Solids Generated', results);
            return currentPlatonicSolids;
        } catch (error) {
            console.error('Error generating platonic solids:', error);
            showError('Failed to generate platonic solids');
            return [];
        }
    }
    
    /**
     * Discover new solids using tetration attractors
     */
    function discoverNewSolids() {
        if (currentTetrationTowers.length === 0) {
            alert('Please generate tetration towers first');
            return;
        }
        
        if (currentPlatonicSolids.length === 0) {
            alert('Please generate platonic solids first');
            return;
        }
        
        try {
            discoveredSolids = [];
            
            currentPlatonicSolids.forEach(solid => {
                const newSolid = PlatonicSolidGenerator.discoverNewSolid(
                    solid,
                    currentTetrationTowers,
                    solid.numDimensions
                );
                
                if (newSolid) {
                    discoveredSolids.push(newSolid);
                }
            });
            
            const results = {
                totalDiscovered: discoveredSolids.length,
                solids: discoveredSolids.map(s => ({
                    name: s.metadata.name,
                    baseSolid: s.metadata.baseSolid,
                    vertices: s.vertices.length,
                    dimensions: s.numDimensions,
                    properties: s.calculateProperties()
                }))
            };
            
            displayResults('New Solids Discovered', results);
            
            // Integrate discovered solids into projections
            integrateSolidsIntoProjections();
            
            return discoveredSolids;
        } catch (error) {
            console.error('Error discovering new solids:', error);
            showError('Failed to discover new solids');
            return [];
        }
    }
    
    /**
     * Integrate discovered solids into projection calculations
     */
    function integrateSolidsIntoProjections() {
        if (discoveredSolids.length === 0) return;
        
        // Store in global scope for projection engine to use
        if (typeof window !== 'undefined') {
            window.discoveredPlatonicSolids = discoveredSolids;
            window.tetrationTowers = currentTetrationTowers;
        }
        
        console.log(`Integrated ${discoveredSolids.length} discovered solids into projection system`);
    }
    
    /**
     * Switch to results tab
     */
    function switchToResultsTab() {
        // Try new unified tab system first
        if (typeof window !== 'undefined' && window.ProjectionTabs) {
            window.ProjectionTabs.switchToTab('results');
            return;
        }
        
        // Fallback to old tab system
        const toolsTab = document.getElementById('hd-tab-tools');
        const resultsTab = document.getElementById('hd-tab-results');
        const toolsBtn = document.querySelector('.hd-tab-btn[data-tab="tools"]');
        const resultsBtn = document.querySelector('.hd-tab-btn[data-tab="results"]');
        
        if (toolsTab && resultsTab && toolsBtn && resultsBtn) {
            toolsTab.classList.remove('active');
            resultsTab.classList.add('active');
            toolsBtn.classList.remove('active');
            resultsBtn.classList.add('active');
        }
    }
    
    /**
     * Display results
     */
    function displayResults(title, results) {
        const resultsPanel = document.getElementById('hd-results-panel');
        const resultsContent = document.getElementById('hd-results-content');
        
        if (!resultsPanel || !resultsContent) return;
        
        // Switch to results tab automatically
        switchToResultsTab();
        
        let html = `<div class="hd-result-section">
            <h5>${title}</h5>
            <div class="hd-result-stats">`;
        
        // Display statistics
        Object.keys(results).forEach(key => {
            if (key !== 'towers' && key !== 'solids') {
                html += `<div class="hd-stat-item">
                    <span class="hd-stat-label">${key}:</span>
                    <span class="hd-stat-value">${results[key]}</span>
                </div>`;
            }
        });
        
        html += `</div>`;
        
        // Display towers if available
        if (results.towers && results.towers.length > 0) {
            html += `<div class="hd-towers-list">
                <h6>Tetration Towers (showing first 10):</h6>
                <table class="hd-table">
                    <thead>
                        <tr>
                            <th>Base</th>
                            <th>Depth</th>
                            <th>Converged</th>
                            <th>Log Value</th>
                            <th>Value</th>
                        </tr>
                    </thead>
                    <tbody>`;
            
            results.towers.forEach(tower => {
                html += `<tr>
                    <td>${tower.base}</td>
                    <td>${tower.depth}</td>
                    <td>${tower.converged ? 'Yes' : 'No'}</td>
                    <td>${tower.logValue}</td>
                    <td>${tower.value}</td>
                </tr>`;
            });
            
            html += `</tbody></table></div>`;
        }
        
        // Display solids if available
        if (results.solids && results.solids.length > 0) {
            html += `<div class="hd-solids-list">
                <h6>Geometric Structures:</h6>
                <div class="hd-solids-grid">`;
            
            results.solids.forEach(solid => {
                html += `<div class="hd-solid-card">
                    <div class="hd-solid-name">${solid.name}</div>
                    <div class="hd-solid-info">
                        <span>Vertices: ${solid.vertices}</span>
                        <span>Dimensions: ${solid.dimensions}</span>
                    </div>`;
                
                if (solid.properties) {
                    html += `<div class="hd-solid-props">
                        <span>Avg Radius: ${solid.properties.averageRadius.toFixed(4)}</span>
                    </div>`;
                }
                
                html += `</div>`;
            });
            
            html += `</div></div>`;
        }
        
        html += `</div>`;
        
        resultsContent.innerHTML = html;
        resultsPanel.style.display = 'block';
    }
    
    /**
     * Show error message
     */
    function showError(message) {
        const resultsContent = document.getElementById('hd-results-content');
        
        if (resultsContent) {
            resultsContent.innerHTML = `<div class="hd-error">
                <i class="fas fa-exclamation-triangle"></i>
                <span>${message}</span>
            </div>`;
            // Switch to results tab to show error
            switchToResultsTab();
        }
    }
    
    /**
     * Setup tab navigation
     */
    function setupTabs() {
        const tabButtons = document.querySelectorAll('.hd-tab-btn');
        const tabContents = document.querySelectorAll('.hd-tab-content');
        
        tabButtons.forEach(btn => {
            btn.addEventListener('click', function() {
                const targetTab = this.getAttribute('data-tab');
                
                // Remove active class from all buttons and contents
                tabButtons.forEach(b => b.classList.remove('active'));
                tabContents.forEach(c => c.classList.remove('active'));
                
                // Add active class to clicked button and corresponding content
                this.classList.add('active');
                const targetContent = document.getElementById(`hd-tab-${targetTab}`);
                if (targetContent) {
                    targetContent.classList.add('active');
                }
            });
        });
    }
    
    /**
     * Initialize event listeners
     */
    function init() {
        // Setup tab navigation
        setupTabs();
        
        const tetrationBtn = document.getElementById('generate-tetration-towers-btn');
        const platonicBtn = document.getElementById('generate-platonic-solids-btn');
        const discoverBtn = document.getElementById('discover-solids-btn');
        
        if (tetrationBtn) {
            tetrationBtn.addEventListener('click', generateTetrationTowers);
        }
        
        if (platonicBtn) {
            platonicBtn.addEventListener('click', generatePlatonicSolids);
        }
        
        if (discoverBtn) {
            discoverBtn.addEventListener('click', discoverNewSolids);
        }
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    return {
        generateTetrationTowers,
        generatePlatonicSolids,
        discoverNewSolids,
        getTetrationTowers: () => currentTetrationTowers,
        getPlatonicSolids: () => currentPlatonicSolids,
        getDiscoveredSolids: () => discoveredSolids
    };
})();

// Make available globally
if (typeof window !== 'undefined') {
    window.HyperdimensionalTools = HyperdimensionalTools;
}






