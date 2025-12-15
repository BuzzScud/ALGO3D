/**
 * Data Page Module
 * Handles the data table for saved projections
 */
const DataPageModule = (function() {
    let allProjections = [];
    let filteredProjections = [];
    
    /**
     * Load all saved projections
     */
    async function loadProjections() {
        const tableBody = document.getElementById('projections-table-body');
        const tableEmpty = document.getElementById('table-empty');
        const tableCount = document.getElementById('table-count');
        
        if (!tableBody) return;
        
        try {
            // Show loading state
            tableBody.innerHTML = `
                <tr>
                    <td colspan="11" class="table-loading">
                        <div class="loading-spinner"></div>
                        <span>Loading projections data...</span>
                    </td>
                </tr>
            `;
            
            const response = await fetch('api/projections.php');
            if (!response.ok) {
                throw new Error('Failed to fetch projections');
            }
            
            const result = await response.json();
            
            if (result.success && result.projections) {
                allProjections = result.projections;
                filteredProjections = [...allProjections];
                renderTable();
            } else {
                allProjections = [];
                filteredProjections = [];
                renderTable();
            }
        } catch (error) {
            console.error('Error loading projections:', error);
            tableBody.innerHTML = `
                <tr>
                    <td colspan="11" class="table-error">
                        <i class="fas fa-exclamation-triangle"></i>
                        <span>Error loading data. Please try again.</span>
                    </td>
                </tr>
            `;
        }
    }
    
    /**
     * Render the data table
     */
    function renderTable() {
        const tableBody = document.getElementById('projections-table-body');
        const tableEmpty = document.getElementById('table-empty');
        const tableCount = document.getElementById('table-count');
        
        if (!tableBody) return;
        
        if (filteredProjections.length === 0) {
            tableBody.innerHTML = '';
            if (tableEmpty) {
                tableEmpty.style.display = 'flex';
            }
            if (tableCount) {
                tableCount.textContent = '0';
            }
            return;
        }
        
        if (tableEmpty) {
            tableEmpty.style.display = 'none';
        }
        
        if (tableCount) {
            tableCount.textContent = filteredProjections.length;
        }
        
        tableBody.innerHTML = filteredProjections.map(proj => {
            const savedDate = new Date(proj.saved_at);
            const formattedDate = savedDate.toLocaleDateString() + ' ' + savedDate.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
            
            // Parse projection data
            let projectionData = null;
            let params = {};
            let lastPrice = '--';
            let interval = '--';
            
            try {
                if (typeof proj.projection_data === 'string') {
                    projectionData = JSON.parse(proj.projection_data);
                } else {
                    projectionData = proj.projection_data;
                }
                
                if (projectionData) {
                    if (projectionData.params) {
                        params = typeof projectionData.params === 'string' 
                            ? JSON.parse(projectionData.params) 
                            : projectionData.params;
                    } else if (proj.params) {
                        params = typeof proj.params === 'string' 
                            ? JSON.parse(proj.params) 
                            : proj.params;
                    }
                    
                    if (projectionData.lastPrice) {
                        lastPrice = '$' + parseFloat(projectionData.lastPrice).toFixed(2);
                    }
                    
                    if (projectionData.interval) {
                        interval = projectionData.interval;
                    }
                }
            } catch (e) {
                console.warn('Error parsing projection data:', e);
            }
            
            return `
                <tr data-id="${proj.id}">
                    <td>${proj.id}</td>
                    <td><strong>${proj.symbol || '--'}</strong></td>
                    <td>${proj.title || '--'}</td>
                    <td>${params.steps || '--'}</td>
                    <td>${params.base || '--'}</td>
                    <td>${params.projectionCount || '--'}</td>
                    <td>${params.depthPrime || '--'}</td>
                    <td>${interval}</td>
                    <td>${lastPrice}</td>
                    <td>${formattedDate}</td>
                    <td class="table-actions">
                        <button class="btn-icon btn-view" data-id="${proj.id}" title="View/Load">
                            <i class="fas fa-eye"></i>
                        </button>
                        <button class="btn-icon btn-export" data-id="${proj.id}" title="Export">
                            <i class="fas fa-download"></i>
                        </button>
                        <button class="btn-icon btn-delete" data-id="${proj.id}" title="Delete">
                            <i class="fas fa-trash"></i>
                        </button>
                    </td>
                </tr>
            `;
        }).join('');
        
        // Attach event listeners
        attachTableEventListeners();
    }
    
    /**
     * Attach event listeners to table actions
     */
    function attachTableEventListeners() {
        // View/Load buttons
        document.querySelectorAll('.btn-view').forEach(btn => {
            btn.addEventListener('click', (e) => {
                e.stopPropagation();
                const id = parseInt(btn.dataset.id);
                loadProjection(id);
            });
        });
        
        // Export buttons
        document.querySelectorAll('.btn-export').forEach(btn => {
            btn.addEventListener('click', (e) => {
                e.stopPropagation();
                const id = parseInt(btn.dataset.id);
                exportProjection(id);
            });
        });
        
        // Delete buttons
        document.querySelectorAll('.btn-delete').forEach(btn => {
            btn.addEventListener('click', (e) => {
                e.stopPropagation();
                const id = parseInt(btn.dataset.id);
                deleteProjection(id);
            });
        });
    }
    
    /**
     * Load projection to projections page
     */
    function loadProjection(id) {
        const projection = allProjections.find(p => p.id === id);
        if (!projection) {
            alert('Projection not found');
            return;
        }
        
        // Switch to projections page
        const projectionsPage = document.getElementById('page-projections');
        const dataPage = document.getElementById('page-data');
        const sidebarItems = document.querySelectorAll('.sidebar-item');
        
        if (projectionsPage && dataPage) {
            // Update sidebar
            sidebarItems.forEach(item => {
                item.classList.remove('active');
                if (item.dataset.page === 'projections') {
                    item.classList.add('active');
                }
            });
            
            // Switch pages
            dataPage.classList.remove('active');
            projectionsPage.classList.add('active');
            
            // Load projection data - use notes.js function if available
            setTimeout(() => {
                // Check if loadProjection is available (from notes.js)
                // It's defined in the global scope, so we can call it directly
                if (typeof loadProjection === 'function') {
                    loadProjection(id);
                } else {
                    // Fallback: manually set values and trigger search
                    const symbolInput = document.getElementById('projection-symbol-input');
                    const intervalSelect = document.getElementById('projection-interval-select');
                    const stepsInput = document.getElementById('projection-steps');
                    const baseInput = document.getElementById('projection-base');
                    const countInput = document.getElementById('projection-count');
                    const depthSelect = document.getElementById('projection-depth');
                    const stepsValueInput = document.getElementById('projection-steps-value');
                    const baseValueInput = document.getElementById('projection-base-value');
                    const countValueInput = document.getElementById('projection-count-value');
                    const depthValueInput = document.getElementById('projection-depth-value');
                    
                    let projectionData = null;
                    let params = {};
                    try {
                        projectionData = typeof projection.projection_data === 'string' 
                            ? JSON.parse(projection.projection_data) 
                            : projection.projection_data;
                        
                        if (projectionData && projectionData.params) {
                            params = typeof projectionData.params === 'string' 
                                ? JSON.parse(projectionData.params) 
                                : projectionData.params;
                        } else if (projection.params) {
                            params = typeof projection.params === 'string' 
                                ? JSON.parse(projection.params) 
                                : projection.params;
                        }
                    } catch (e) {
                        console.error('Error parsing projection data:', e);
                    }
                    
                    if (symbolInput && projectionData) {
                        symbolInput.value = projectionData.symbol || projection.symbol;
                    }
                    
                    if (intervalSelect && projectionData && projectionData.interval) {
                        intervalSelect.value = projectionData.interval;
                    }
                    
                    // Set custom mode and update inputs
                    const customPreset = document.getElementById('preset-custom');
                    if (customPreset && params.steps) {
                        customPreset.checked = true;
                        customPreset.dispatchEvent(new Event('change'));
                        
                        if (stepsInput && params.steps) stepsInput.value = params.steps;
                        if (baseInput && params.base) baseInput.value = params.base;
                        if (countInput && params.projectionCount) countInput.value = params.projectionCount;
                        if (depthSelect && params.depthPrime) depthSelect.value = params.depthPrime;
                        
                        // Also update hidden values
                        if (stepsValueInput && params.steps) stepsValueInput.value = params.steps;
                        if (baseValueInput && params.base) baseValueInput.value = params.base;
                        if (countValueInput && params.projectionCount) countValueInput.value = params.projectionCount;
                        if (depthValueInput && params.depthPrime) depthValueInput.value = params.depthPrime;
                    }
                    
                    // Trigger search
                    const searchBtn = document.getElementById('projection-search-btn');
                    if (searchBtn) {
                        setTimeout(() => {
                            searchBtn.click();
                        }, 500);
                    }
                }
            }, 300);
        }
    }
    
    /**
     * Export projection as JSON
     */
    function exportProjection(id) {
        const projection = allProjections.find(p => p.id === id);
        if (!projection) {
            alert('Projection not found');
            return;
        }
        
        try {
            const exportData = {
                id: projection.id,
                symbol: projection.symbol,
                title: projection.title,
                projection_data: typeof projection.projection_data === 'string' 
                    ? JSON.parse(projection.projection_data) 
                    : projection.projection_data,
                chart_data: projection.chart_data ? (typeof projection.chart_data === 'string' 
                    ? JSON.parse(projection.chart_data) 
                    : projection.chart_data) : null,
                params: projection.params ? (typeof projection.params === 'string' 
                    ? JSON.parse(projection.params) 
                    : projection.params) : null,
                notes: projection.notes,
                saved_at: projection.saved_at
            };
            
            const dataStr = JSON.stringify(exportData, null, 2);
            const dataBlob = new Blob([dataStr], {type: 'application/json'});
            const url = URL.createObjectURL(dataBlob);
            const link = document.createElement('a');
            link.href = url;
            link.download = `${projection.symbol}_projection_${projection.id}_${Date.now()}.json`;
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
            URL.revokeObjectURL(url);
        } catch (error) {
            console.error('Error exporting projection:', error);
            alert('Failed to export projection');
        }
    }
    
    /**
     * Delete projection
     */
    async function deleteProjection(id) {
        if (!confirm('Are you sure you want to delete this projection?')) {
            return;
        }
        
        try {
            const response = await fetch('api/projections.php', {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({id: id})
            });
            
            const result = await response.json();
            
            if (result.success) {
                // Remove from arrays
                allProjections = allProjections.filter(p => p.id !== id);
                filteredProjections = filteredProjections.filter(p => p.id !== id);
                renderTable();
            } else {
                alert(result.message || 'Failed to delete projection');
            }
        } catch (error) {
            console.error('Error deleting projection:', error);
            alert('Failed to delete projection');
        }
    }
    
    /**
     * Filter and search projections
     */
    function filterProjections() {
        const searchInput = document.getElementById('data-search-input');
        const sortSelect = document.getElementById('data-sort-select');
        
        if (!searchInput || !sortSelect) return;
        
        const searchTerm = searchInput.value.toLowerCase().trim();
        
        // Filter
        filteredProjections = allProjections.filter(proj => {
            if (!searchTerm) return true;
            
            const symbol = (proj.symbol || '').toLowerCase();
            const title = (proj.title || '').toLowerCase();
            const notes = (proj.notes || '').toLowerCase();
            
            return symbol.includes(searchTerm) || 
                   title.includes(searchTerm) || 
                   notes.includes(searchTerm);
        });
        
        // Sort
        const sortValue = sortSelect.value;
        filteredProjections.sort((a, b) => {
            switch (sortValue) {
                case 'date-desc':
                    return new Date(b.saved_at) - new Date(a.saved_at);
                case 'date-asc':
                    return new Date(a.saved_at) - new Date(b.saved_at);
                case 'symbol-asc':
                    return (a.symbol || '').localeCompare(b.symbol || '');
                case 'symbol-desc':
                    return (b.symbol || '').localeCompare(a.symbol || '');
                default:
                    return 0;
            }
        });
        
        renderTable();
    }
    
    /**
     * Export all projections
     */
    function exportAll() {
        if (filteredProjections.length === 0) {
            alert('No projections to export');
            return;
        }
        
        try {
            const exportData = {
                export_date: new Date().toISOString(),
                total_count: filteredProjections.length,
                projections: filteredProjections.map(proj => ({
                    id: proj.id,
                    symbol: proj.symbol,
                    title: proj.title,
                    projection_data: typeof proj.projection_data === 'string' 
                        ? JSON.parse(proj.projection_data) 
                        : proj.projection_data,
                    params: proj.params ? (typeof proj.params === 'string' 
                        ? JSON.parse(proj.params) 
                        : proj.params) : null,
                    notes: proj.notes,
                    saved_at: proj.saved_at
                }))
            };
            
            const dataStr = JSON.stringify(exportData, null, 2);
            const dataBlob = new Blob([dataStr], {type: 'application/json'});
            const url = URL.createObjectURL(dataBlob);
            const link = document.createElement('a');
            link.href = url;
            link.download = `all_projections_${Date.now()}.json`;
            document.body.appendChild(link);
            link.click();
            document.body.removeChild(link);
            URL.revokeObjectURL(url);
        } catch (error) {
            console.error('Error exporting all projections:', error);
            alert('Failed to export projections');
        }
    }
    
    /**
     * Watch for page activation
     */
    function watchPageActivation() {
        const pageEl = document.getElementById('page-data');
        if (!pageEl) return;
        
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (pageEl.classList.contains('active')) {
                        // Page is active, load data
                        loadProjections();
                    }
                }
            });
        });
        
        observer.observe(pageEl, {
            attributes: true,
            attributeFilter: ['class']
        });
    }
    
    /**
     * Initialize
     */
    function init() {
        const refreshBtn = document.getElementById('refresh-data-btn');
        const exportAllBtn = document.getElementById('export-all-btn');
        const searchInput = document.getElementById('data-search-input');
        const sortSelect = document.getElementById('data-sort-select');
        const clearFiltersBtn = document.getElementById('clear-filters-btn');
        
        if (refreshBtn) {
            refreshBtn.addEventListener('click', loadProjections);
        }
        
        if (exportAllBtn) {
            exportAllBtn.addEventListener('click', exportAll);
        }
        
        if (searchInput) {
            searchInput.addEventListener('input', filterProjections);
        }
        
        if (sortSelect) {
            sortSelect.addEventListener('change', filterProjections);
        }
        
        if (clearFiltersBtn) {
            clearFiltersBtn.addEventListener('click', () => {
                if (searchInput) searchInput.value = '';
                if (sortSelect) sortSelect.value = 'date-desc';
                filterProjections();
            });
        }
        
        // Watch for page activation
        watchPageActivation();
        
        // Load if page is already active
        if (document.getElementById('page-data')?.classList.contains('active')) {
            loadProjections();
        }
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    return {
        init: init,
        loadProjections: loadProjections,
        refresh: loadProjections
    };
})();

// Expose for external use
window.DataPageModule = DataPageModule;

