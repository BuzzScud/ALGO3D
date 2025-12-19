/**
 * Data Page Module
 * Handles the data table for saved projections
 */
const DataPageModule = (function() {
    let allProjections = [];
    let filteredProjections = [];
    let currentPage = 1;
    const itemsPerPage = 10;
    let modalChart = null; // Chart instance for the modal
    
    /**
     * Get element by ID, checking both regular and tab versions
     */
    function getElement(id, tabId) {
        return document.getElementById(id) || document.getElementById(tabId);
    }
    
    /**
     * Load all saved projections
     */
    async function loadProjections() {
        const container = document.getElementById('projections-feed-container');
        const tableCount = getElement('table-count', 'table-count-tab');
        
        if (!container) return;
        
        try {
            // Show loading state
            container.innerHTML = `
                <div class="news-loading">
                    <i class="fas fa-spinner fa-spin"></i>
                    <span>Loading projections data...</span>
                </div>
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
                update3DVisualization(allProjections.length);
            } else {
                allProjections = [];
                filteredProjections = [];
                renderTable();
                update3DVisualization(0);
            }
        } catch (error) {
            console.error('Error loading projections:', error);
            container.innerHTML = `
                <div class="news-error-state">
                    <i class="fas fa-exclamation-triangle"></i>
                    <p>Error loading data. Please try again.</p>
                    <button class="btn btn-primary btn-sm" onclick="DataPageModule.refresh()">
                        <i class="fas fa-redo"></i>
                        Retry
                    </button>
                </div>
            `;
        }
    }
    
    /**
     * Render the data cards (replacing table)
     */
    function renderTable() {
        const container = document.getElementById('projections-feed-container');
        const tableCount = getElement('table-count', 'table-count-tab');
        
        if (!container) return;
        
        // Calculate pagination
        const totalPages = Math.ceil(filteredProjections.length / itemsPerPage);
        const startIndex = (currentPage - 1) * itemsPerPage;
        const endIndex = startIndex + itemsPerPage;
        const pageProjections = filteredProjections.slice(startIndex, endIndex);
        
        if (tableCount) {
            tableCount.textContent = filteredProjections.length;
        }
        
        if (filteredProjections.length === 0 || pageProjections.length === 0) {
            container.innerHTML = `
                <div class="news-empty-state">
                    <i class="fas fa-inbox"></i>
                    <p>No saved projections found</p>
                    <span class="text-sm">Save a projection from the Projections page to see it here</span>
                </div>
            `;
            renderPagination(totalPages);
            return;
        }
        
        // Format time ago helper
        function formatTimeAgo(dateString) {
            const now = Date.now();
            const date = new Date(dateString);
            const diff = now - date.getTime();
            const seconds = Math.floor(diff / 1000);
            const minutes = Math.floor(seconds / 60);
            const hours = Math.floor(minutes / 60);
            const days = Math.floor(hours / 24);
            
            if (days > 0) {
                return `${days} day${days > 1 ? 's' : ''} ago`;
            } else if (hours > 0) {
                return `${hours} hour${hours > 1 ? 's' : ''} ago`;
            } else if (minutes > 0) {
                return `${minutes} minute${minutes > 1 ? 's' : ''} ago`;
            } else {
                return 'Just now';
            }
        }
        
        // Format date helper
        function formatDate(dateString) {
            const date = new Date(dateString);
            return date.toLocaleDateString('en-US', {
                month: 'short',
                day: 'numeric',
                year: 'numeric'
            });
        }
        
        container.innerHTML = pageProjections.map(proj => {
            const savedDate = new Date(proj.saved_at);
            const timeAgo = formatTimeAgo(proj.saved_at);
            const formattedDate = formatDate(proj.saved_at);
            
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
            
            // Build params summary
            const paramsSummary = [];
            if (params.steps) paramsSummary.push(`Steps: ${params.steps}`);
            if (params.base) paramsSummary.push(`Base: ${params.base}`);
            if (params.projectionCount) paramsSummary.push(`Count: ${params.projectionCount}`);
            if (params.depthPrime) paramsSummary.push(`Depth: ${params.depthPrime}`);
            
            // Build description
            const description = proj.notes 
                ? (proj.notes.length > 100 ? proj.notes.substring(0, 100) + '...' : proj.notes)
                : `Projection for ${proj.symbol || 'N/A'}${params.steps ? ` with ${params.steps} steps` : ''}${params.base ? `, base ${params.base}` : ''}${params.depthPrime ? `, depth ${params.depthPrime}` : ''}`;
            
            return `
                <div class="news-article" data-id="${proj.id}">
                    <div class="news-article-content">
                        <div class="news-article-header">
                            <span class="news-source" style="color: var(--primary-color); font-weight: 600;">${proj.symbol || 'N/A'}</span>
                            <span class="news-time">${timeAgo}</span>
                        </div>
                        <h3 class="news-article-title">
                            ${proj.title || `${proj.symbol || 'Untitled'}${interval !== '--' ? ` - ${interval} Projection` : ' Projection'}`}
                        </h3>
                        ${paramsSummary.length > 0 || interval !== '--' || lastPrice !== '--' ? `
                            <p class="news-article-summary">
                                ${paramsSummary.join(' • ')}${interval !== '--' ? ` • Interval: ${interval}` : ''}${lastPrice !== '--' ? ` • Price: ${lastPrice}` : ''}
                            </p>
                        ` : ''}
                        <p class="news-article-summary" style="margin-top: 0.5rem; font-style: italic; opacity: 0.8;">
                            ${description}
                        </p>
                        <div class="news-article-footer">
                            <span class="news-date">${formattedDate}</span>
                            <div class="projection-actions" style="display: flex; gap: 0.5rem;">
                                <button type="button" class="btn-view btn btn-primary btn-sm" data-id="${proj.id}" title="Load Projection">
                                    <i class="fas fa-check"></i>
                                    Load
                                </button>
                                <button type="button" class="btn-export btn btn-secondary btn-sm" data-id="${proj.id}" title="Export">
                                    <i class="fas fa-download"></i>
                                </button>
                                <button type="button" class="btn-delete btn btn-secondary btn-sm" data-id="${proj.id}" title="Delete">
                                    <i class="fas fa-trash"></i>
                                </button>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        }).join('');
        
        // Attach event listeners after DOM update
        // Use setTimeout to ensure DOM is fully updated
        setTimeout(() => {
            attachTableEventListeners();
        }, 0);
        
        // Render pagination
        renderPagination(totalPages);
    }
    
    /**
     * Render pagination controls (matching news feed style)
     */
    function renderPagination(totalPages) {
        const paginationEl = document.getElementById('data-pagination');
        const pagesEl = document.getElementById('data-pagination-pages');
        const prevBtn = document.getElementById('data-prev-btn');
        const nextBtn = document.getElementById('data-next-btn');
        
        if (!paginationEl || !pagesEl || !prevBtn || !nextBtn) return;
        
        if (totalPages <= 1) {
            paginationEl.style.display = 'none';
            return;
        }
        
        paginationEl.style.display = 'flex';
        
        // Update prev/next buttons
        prevBtn.disabled = currentPage === 1;
        nextBtn.disabled = currentPage === totalPages;
        
        // Render page numbers
        let pagesHTML = '';
        for (let i = 1; i <= totalPages; i++) {
            if (i === 1 || i === totalPages || (i >= currentPage - 1 && i <= currentPage + 1)) {
                pagesHTML += `
                    <button class="pagination-page ${i === currentPage ? 'active' : ''}" data-page="${i}">
                        ${i}
                    </button>
                `;
            } else if (i === currentPage - 2 || i === currentPage + 2) {
                pagesHTML += `<span class="pagination-ellipsis">...</span>`;
            }
        }
        
        pagesEl.innerHTML = pagesHTML;
        
        // Add event listeners
        pagesEl.querySelectorAll('.pagination-page').forEach(btn => {
            btn.addEventListener('click', () => {
                currentPage = parseInt(btn.dataset.page);
                renderTable();
            });
        });
        
        prevBtn.onclick = () => {
            if (currentPage > 1) {
                currentPage--;
                renderTable();
            }
        };
        
        nextBtn.onclick = () => {
            if (currentPage < totalPages) {
                currentPage++;
                renderTable();
            }
        };
    }
    
    /**
     * Attach event listeners to table actions
     */
    function attachTableEventListeners() {
        // View/Load buttons
        document.querySelectorAll('.btn-view').forEach(btn => {
            // Remove any existing listeners by cloning (cleaner than trying to track them)
            const newBtn = btn.cloneNode(true);
            btn.parentNode.replaceChild(newBtn, btn);
            
            // Add event listener to the new button
            newBtn.addEventListener('click', function(e) {
                e.stopPropagation();
                e.preventDefault();
                
                const id = parseInt(this.dataset.id);
                console.log('Load button clicked for projection ID:', id);
                
                if (id && !isNaN(id)) {
                    loadProjection(id);
                } else {
                    console.error('Invalid projection ID:', this.dataset.id);
                    alert('Invalid projection ID: ' + this.dataset.id);
                }
                return false;
            }, true); // Use capture phase to ensure it fires
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
     * Show modal to load projection
     */
    function loadProjection(id) {
        console.log('loadProjection called with ID:', id);
        
        if (!id || isNaN(id)) {
            console.error('Invalid projection ID:', id);
            alert('Invalid projection ID');
            return;
        }
        
        const projectionId = parseInt(id);
        console.log('Showing load projection modal for ID:', projectionId);
        
        // Show modal
        const modal = document.getElementById('load-projection-modal');
        const loadingDiv = document.getElementById('load-projection-loading');
        const contentDiv = document.getElementById('load-projection-content');
        const errorDiv = document.getElementById('load-projection-error');
        const confirmBtn = document.getElementById('confirm-load-projection-btn');
        
        if (!modal) {
            console.error('Load projection modal not found');
            // Fallback to direct load
            loadProjectionDirectly(projectionId);
            return;
        }
        
        // Reset modal state
        loadingDiv.style.display = 'block';
        contentDiv.style.display = 'none';
        errorDiv.style.display = 'none';
        confirmBtn.disabled = true;
        confirmBtn.dataset.projectionId = projectionId;
        
        // Show modal
        modal.classList.add('active');
        
        // Fetch projection data
        const projection = allProjections.find(p => p.id === projectionId);
        if (projection) {
            displayProjectionInModal(projection);
        } else {
            // Fetch from API
            fetch('api/projections.php')
                .then(response => response.json())
                .then(result => {
                    if (result.success && result.projections) {
                        const proj = result.projections.find(p => p.id === projectionId);
                        if (proj) {
                            displayProjectionInModal(proj);
                        } else {
                            showProjectionError('Projection not found');
                        }
                    } else {
                        showProjectionError('Failed to load projection data');
                    }
                })
                .catch(error => {
                    console.error('Error fetching projection:', error);
                    showProjectionError('Failed to load projection: ' + error.message);
                });
        }
    }
    
    /**
     * Display projection data in the modal
     */
    function displayProjectionInModal(projection) {
        const loadingDiv = document.getElementById('load-projection-loading');
        const contentDiv = document.getElementById('load-projection-content');
        const errorDiv = document.getElementById('load-projection-error');
        const confirmBtn = document.getElementById('confirm-load-projection-btn');
        const modal = document.getElementById('load-projection-modal');
        
        loadingDiv.style.display = 'none';
        errorDiv.style.display = 'none';
        contentDiv.style.display = 'block';
        confirmBtn.disabled = false;
        
        // Store projection data on modal for later use
        if (modal) {
            modal.dataset.projectionData = JSON.stringify(projection);
        }
        confirmBtn.dataset.projectionId = projection.id;
        
        // Parse projection data
        let projectionData = null;
        let params = {};
        
        try {
            if (typeof projection.projection_data === 'string') {
                projectionData = JSON.parse(projection.projection_data);
            } else {
                projectionData = projection.projection_data;
            }
            
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
            console.warn('Error parsing projection data:', e);
        }
        
        // Display data
        document.getElementById('modal-projection-symbol').textContent = projectionData?.symbol || projection.symbol || '--';
        document.getElementById('modal-projection-title').textContent = projection.title || projection.symbol || 'Untitled Projection';
        document.getElementById('modal-projection-interval').textContent = projectionData?.interval || '--';
        
        // Build params display
        const paramsArray = [];
        if (params.steps) paramsArray.push(`Steps: ${params.steps}`);
        if (params.base) paramsArray.push(`Base: ${params.base}`);
        if (params.projectionCount) paramsArray.push(`Count: ${params.projectionCount}`);
        if (params.depthPrime) paramsArray.push(`Depth: ${params.depthPrime}`);
        document.getElementById('modal-projection-params').textContent = paramsArray.length > 0 ? paramsArray.join(' • ') : '--';
        
        // Notes
        if (projection.notes) {
            document.getElementById('modal-projection-notes-group').style.display = 'block';
            document.getElementById('modal-projection-notes').textContent = projection.notes;
        } else {
            document.getElementById('modal-projection-notes-group').style.display = 'none';
        }
        
        // Date
        const savedDate = new Date(projection.saved_at);
        document.getElementById('modal-projection-date').textContent = savedDate.toLocaleDateString('en-US', {
            month: 'short',
            day: 'numeric',
            year: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
        });
        
        // Render chart
        renderModalChart(projection, projectionData, params);
    }
    
    /**
     * Render projection chart in modal
     */
    function renderModalChart(projection, projectionData, params) {
        const canvas = document.getElementById('modal-projection-chart');
        if (!canvas) {
            console.warn('Modal chart canvas not found');
            return;
        }
        
        // Destroy existing chart
        if (modalChart) {
            modalChart.destroy();
            modalChart = null;
        }
        
        // Get chart data from projection
        let historicalPrices = [];
        let historicalLabels = [];
        let projectionLines = [];
        
        try {
            // Priority 1: Get from projection_data (this is what's saved)
            if (projectionData) {
                if (projectionData.historicalPrices && Array.isArray(projectionData.historicalPrices)) {
                    historicalPrices = projectionData.historicalPrices.map(p => typeof p === 'string' ? parseFloat(p) : p);
                }
                if (projectionData.historicalLabels && Array.isArray(projectionData.historicalLabels)) {
                    historicalLabels = projectionData.historicalLabels;
                }
                if (projectionData.projectionLines && Array.isArray(projectionData.projectionLines)) {
                    projectionLines = projectionData.projectionLines;
                }
            }
            
            // Priority 2: If no projection_data, try chart_data
            if (historicalPrices.length === 0 && projection.chart_data) {
                const chartData = typeof projection.chart_data === 'string' 
                    ? JSON.parse(projection.chart_data) 
                    : projection.chart_data;
                
                // chart_data might have Chart.js structure (datasets with labels)
                if (chartData && chartData.datasets && chartData.datasets.length > 0) {
                    // Extract from first dataset (historical data)
                    const historicalDataset = chartData.datasets.find(ds => ds.label && ds.label.includes('Historical'));
                    if (historicalDataset && historicalDataset.data) {
                        historicalPrices = historicalDataset.data.filter(d => d !== null && d !== undefined);
                        historicalLabels = chartData.labels || [];
                    }
                    
                    // Extract projection lines from other datasets
                    chartData.datasets.forEach((ds, idx) => {
                        if (ds.label && !ds.label.includes('Historical') && ds.data) {
                            const points = ds.data.filter(d => d !== null && d !== undefined);
                            if (points.length > 0) {
                                projectionLines.push({
                                    points: points,
                                    triad: [ds.label]
                                });
                            }
                        }
                    });
                }
            }
            
            // Priority 3: Fallback - use lastPrice if available
            if (historicalPrices.length === 0 && projectionData && projectionData.lastPrice) {
                historicalPrices = [parseFloat(projectionData.lastPrice)];
                historicalLabels = ['Last Price'];
            }
        } catch (e) {
            console.warn('Error parsing chart data:', e);
        }
        
        if (historicalPrices.length === 0) {
            console.warn('No chart data available for projection');
            // Show message on canvas
            const ctx = canvas.getContext('2d');
            ctx.fillStyle = '#9ca3af';
            ctx.font = '14px Inter, sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('Chart data not available', canvas.width / 2, canvas.height / 2);
            return;
        }
        
        // Chart colors
        const chartColors = {
            grid: 'rgba(255, 255, 255, 0.1)',
            text: '#9ca3af',
            primary: '#3b82f6',
            bullish: '#22c55e',
            bearish: '#ef4444'
        };
        
        const projectionColors = [
            '#8b5cf6', '#10b981', '#f59e0b', '#ef4444', '#ec4899',
            '#14b8a6', '#f97316', '#6366f1', '#84cc16', '#06b6d4',
            '#a855f7', '#22c55e'
        ];
        
        const steps = params.steps || 20;
        const symbol = projectionData?.symbol || projection.symbol || 'Symbol';
        
        // Generate projection labels if not available
        if (historicalLabels.length === 0 || historicalLabels.length < historicalPrices.length) {
            historicalLabels = historicalPrices.map((_, idx) => `Point ${idx + 1}`);
        }
        
        // Generate projection labels
        const projectedLabels = [];
        for (let i = 1; i <= steps; i++) {
            projectedLabels.push(`Step ${i}`);
        }
        
        const allLabels = [...historicalLabels, ...projectedLabels];
        const lastPrice = historicalPrices[historicalPrices.length - 1];
        
        // Build datasets
        const datasets = [
            {
                label: `${symbol} Historical`,
                data: [...historicalPrices, ...new Array(steps).fill(null)],
                borderColor: chartColors.primary,
                backgroundColor: 'transparent',
                tension: 0.1,
                fill: false,
                pointRadius: 0,
                borderWidth: 2,
                yAxisID: 'y'
            }
        ];
        
        // Add projection lines
        if (projectionLines && projectionLines.length > 0) {
            projectionLines.forEach((line, idx) => {
                if (!line.points || line.points.length === 0) return;
                
                const lineData = [];
                // Fill with nulls for historical period
                for (let i = 0; i < historicalPrices.length; i++) {
                    lineData.push(null);
                }
                
                // Connect first point to last historical
                if (line.points.length > 0) {
                    const firstProjected = line.points[0];
                    const connectionPrice = lastPrice * 0.8 + firstProjected * 0.2;
                    lineData.push(connectionPrice);
                }
                
                // Add rest of projection points
                for (let i = 1; i < line.points.length; i++) {
                    lineData.push(line.points[i]);
                }
                
                // Fill remaining with nulls
                while (lineData.length < allLabels.length) {
                    lineData.push(null);
                }
                
                const isEnsemble = line.isEnsemble || (line.triad && line.triad[0] === 'Ensemble');
                const triadLabel = line.triad ? `Triad [${line.triad.join('-')}]` : `Projection ${idx + 1}`;
                
                datasets.push({
                    label: triadLabel,
                    data: lineData,
                    borderColor: projectionColors[idx % projectionColors.length],
                    backgroundColor: 'transparent',
                    borderWidth: isEnsemble ? 2.5 : 1.5,
                    pointRadius: 0,
                    tension: 0.1,
                    borderDash: isEnsemble ? [] : [5, 5],
                    yAxisID: 'y'
                });
            });
        }
        
        // Create chart
        modalChart = new Chart(canvas, {
            type: 'line',
            data: {
                labels: allLabels,
                datasets: datasets
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index'
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top',
                        labels: {
                            color: chartColors.text,
                            usePointStyle: true,
                            padding: 8,
                            boxWidth: 10,
                            font: {
                                size: 10
                            }
                        }
                    },
                    tooltip: {
                        enabled: true,
                        mode: 'index',
                        intersect: false,
                        backgroundColor: 'rgba(0, 0, 0, 0.8)',
                        titleColor: '#fff',
                        bodyColor: '#fff',
                        borderColor: 'rgba(255, 255, 255, 0.1)',
                        borderWidth: 1,
                        padding: 10,
                        callbacks: {
                            label: function(context) {
                                const datasetLabel = context.dataset.label || '';
                                const value = context.parsed.y;
                                if (value === null || value === undefined) return '';
                                return `${datasetLabel}: $${value.toFixed(2)}`;
                            }
                        }
                    }
                },
                scales: {
                    x: {
                        display: true,
                        grid: {
                            color: chartColors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: chartColors.text,
                            maxRotation: 45,
                            padding: 6,
                            font: {
                                size: 10
                            }
                        }
                    },
                    y: {
                        position: 'right',
                        display: true,
                        grid: {
                            color: chartColors.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: chartColors.text,
                            padding: 6,
                            font: {
                                size: 10
                            },
                            callback: function(value) {
                                return '$' + value.toFixed(2);
                            }
                        }
                    }
                },
                animation: {
                    duration: 300
                }
            }
        });
    }
    
    /**
     * Show error in modal
     */
    function showProjectionError(message) {
        const loadingDiv = document.getElementById('load-projection-loading');
        const contentDiv = document.getElementById('load-projection-content');
        const errorDiv = document.getElementById('load-projection-error');
        const confirmBtn = document.getElementById('confirm-load-projection-btn');
        
        loadingDiv.style.display = 'none';
        contentDiv.style.display = 'none';
        errorDiv.style.display = 'block';
        confirmBtn.disabled = true;
        document.getElementById('load-projection-error-text').textContent = message;
    }
    
    /**
     * Load projection directly (fallback if modal fails)
     */
    function loadProjectionDirectly(projectionId) {
        // Use the global loadProjection function from notes.js if available
        if (typeof window.loadProjection === 'function') {
            console.log('Using window.loadProjection from notes.js');
            try {
                window.loadProjection(projectionId);
                return;
            } catch (error) {
                console.error('Error calling window.loadProjection:', error);
            }
        }
        
        // Fallback implementation
        const projection = allProjections.find(p => p.id === projectionId);
        if (!projection) {
            fetch('api/projections.php')
                .then(response => response.json())
                .then(result => {
                    if (result.success && result.projections) {
                        const proj = result.projections.find(p => p.id === projectionId);
                        if (proj) {
                            loadProjectionData(proj);
                        } else {
                            alert('Projection not found');
                        }
                    }
                })
                .catch(error => {
                    console.error('Error fetching projection:', error);
                    alert('Failed to load projection');
                });
            return;
        }
        
        loadProjectionData(projection);
    }
    
    /**
     * Helper function to load projection data into the projections page
     */
    function loadProjectionData(projection) {
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
            
            // Load projection data
            setTimeout(() => {
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
                
                if (symbolInput) {
                    symbolInput.value = projectionData?.symbol || projection.symbol || '';
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
            }, 300);
        } else {
            alert('Could not switch to projections page');
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
                
                // Update 3D visualization
                update3DVisualization(allProjections.length);
                
                // Adjust page if needed
                const totalPages = Math.ceil(filteredProjections.length / itemsPerPage);
                if (currentPage > totalPages && totalPages > 0) {
                    currentPage = totalPages;
                }
                
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
        
        // Reset to first page when filtering
        currentPage = 1;
        
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
     * Watch for tab activation on projections page
     */
    function watchProjectionsTabActivation() {
        const tabContent = document.getElementById('tab-saved-projections');
        if (!tabContent) return;
        
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (tabContent.classList.contains('active')) {
                        // Tab is active, load data
                        loadProjections();
                    }
                }
            });
        });
        
        observer.observe(tabContent, {
            attributes: true,
            attributeFilter: ['class']
        });
    }
    
    /**
     * Initialize modal handlers
     */
    function initLoadProjectionModal() {
        const modal = document.getElementById('load-projection-modal');
        const closeBtn = document.getElementById('close-load-projection-modal');
        const cancelBtn = document.getElementById('cancel-load-projection-btn');
        const confirmBtn = document.getElementById('confirm-load-projection-btn');
        
        // Close modal handlers
        const closeModal = () => {
            if (modal) {
                modal.classList.remove('active');
                // Clean up chart when closing modal
                if (modalChart) {
                    modalChart.destroy();
                    modalChart = null;
                }
            }
        };
        
        if (closeBtn) {
            closeBtn.addEventListener('click', closeModal);
        }
        
        if (cancelBtn) {
            cancelBtn.addEventListener('click', closeModal);
        }
        
        // Close on background click
        if (modal) {
            modal.addEventListener('click', (e) => {
                if (e.target === modal) {
                    closeModal();
                }
            });
        }
        
        // Confirm button handler
        if (confirmBtn) {
            confirmBtn.addEventListener('click', () => {
                const modal = document.getElementById('load-projection-modal');
                const projectionId = parseInt(confirmBtn.dataset.projectionId);
                
                if (projectionId && !isNaN(projectionId)) {
                    // Get stored projection data if available
                    let projection = null;
                    if (modal && modal.dataset.projectionData) {
                        try {
                            projection = JSON.parse(modal.dataset.projectionData);
                        } catch (e) {
                            console.warn('Error parsing stored projection data:', e);
                        }
                    }
                    
                    closeModal();
                    
                    // Use stored projection if available, otherwise fetch
                    if (projection) {
                        loadProjectionData(projection);
                    } else {
                        loadProjectionDirectly(projectionId);
                    }
                }
            });
        }
        
        // Close on Escape key
        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape' && modal && modal.classList.contains('active')) {
                closeModal();
            }
        });
    }
    
    /**
     * Initialize
     */
    function init() {
        const refreshBtn = getElement('refresh-data-btn', 'refresh-data-btn-tab');
        const exportAllBtn = getElement('export-all-btn', 'export-all-btn-tab');
        const searchInput = getElement('data-search-input', 'data-search-input-tab');
        const sortSelect = getElement('data-sort-select', 'data-sort-select-tab');
        const clearFiltersBtn = getElement('clear-filters-btn', 'clear-filters-btn-tab');
        
        // Initialize modal handlers
        initLoadProjectionModal();
        
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
            clearFiltersBtn.addEventListener('click', (e) => {
                e.preventDefault();
                e.stopPropagation();
                if (searchInput) searchInput.value = '';
                if (sortSelect) sortSelect.value = 'date-desc';
                currentPage = 1; // Reset to first page
                filterProjections();
            });
        }
        
        // Watch for page activation
        watchPageActivation();
        
        // Watch for tab activation on projections page
        watchProjectionsTabActivation();
        
        // Load if page is already active
        if (document.getElementById('page-data')?.classList.contains('active')) {
            loadProjections();
            // Initialize 3D visualization
            setTimeout(() => {
                if (typeof THREE !== 'undefined' && !dataScene) {
                    init3DVisualization();
                }
            }, 300);
        }
        
        // Load if projections page tab is already active
        const projectionsPage = document.getElementById('page-projections');
        const savedProjectionsTab = document.getElementById('tab-saved-projections');
        if (projectionsPage?.classList.contains('active') && savedProjectionsTab?.classList.contains('active')) {
            loadProjections();
        }
    }
    
    // 3D Visualization
    let dataScene, dataCamera, dataRenderer;
    let dataAnimationId = null;
    let dataSolid = null;
    let dataFillSolid = null;
    let dataParticles = null;
    let currentDataCount = 0;
    let targetFillLevel = 0;
    let dataControls = null;
    let isDragging = false;
    let previousMousePosition = { x: 0, y: 0 };
    let currentModelType = 'dodecahedron';
    
    /**
     * Initialize 3D visualization
     */
    function init3DVisualization() {
        const canvas = document.getElementById('data-3d-canvas');
        const loading = document.getElementById('data-viz-loading');
        
        if (!canvas || typeof THREE === 'undefined') {
            if (loading) loading.style.display = 'none';
            return;
        }
        
        const container = canvas.parentElement;
        const width = container.clientWidth || 400;
        const height = container.clientHeight || 400;
        
        canvas.width = width;
        canvas.height = height;
        
        // Scene
        dataScene = new THREE.Scene();
        dataScene.background = new THREE.Color(0x0a0f1a);
        
        // Camera
        dataCamera = new THREE.PerspectiveCamera(50, width / height, 0.1, 1000);
        dataCamera.position.set(0, 0, 8);
        dataCamera.lookAt(0, 0, 0);
        
        // Renderer
        dataRenderer = new THREE.WebGLRenderer({ 
            canvas: canvas, 
            antialias: true,
            alpha: false
        });
        dataRenderer.setSize(width, height);
        dataRenderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        
        // Lighting
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
        dataScene.add(ambientLight);
        
        const directionalLight = new THREE.DirectionalLight(0x3b82f6, 0.8);
        directionalLight.position.set(5, 10, 5);
        dataScene.add(directionalLight);
        
        const pointLight = new THREE.PointLight(0x3b82f6, 0.5, 100);
        pointLight.position.set(-5, -5, 5);
        dataScene.add(pointLight);
        
        // Create platonic solid (default: Dodecahedron)
        createPlatonicSolid(currentModelType);
        
        // Setup interactive controls (mouse drag to rotate, wheel to zoom)
        setupInteractiveControls(canvas);
        
        // Setup model selector
        setupModelSelector();
        
        // Hide loading
        if (loading) loading.style.display = 'none';
        
        // Start animation
        animate3D();
        
        // Handle resize
        window.addEventListener('resize', () => {
            if (container && dataRenderer && dataCamera) {
                const newWidth = container.clientWidth || 400;
                const newHeight = container.clientHeight || 400;
                dataCamera.aspect = newWidth / newHeight;
                dataCamera.updateProjectionMatrix();
                dataRenderer.setSize(newWidth, newHeight);
            }
        });
    }
    
    /**
     * Setup model selector
     */
    function setupModelSelector() {
        const modelSelect = document.getElementById('data-viz-model-select');
        if (!modelSelect) return;
        
        // Set initial value
        modelSelect.value = currentModelType;
        
        // Add change listener
        modelSelect.addEventListener('change', function() {
            currentModelType = this.value;
            if (dataScene) {
                // Save current fill level
                const savedFillLevel = targetFillLevel;
                createPlatonicSolid(currentModelType);
                // Restore fill level
                targetFillLevel = savedFillLevel;
            }
        });
    }
    
    /**
     * Setup interactive controls for 3D visualization
     */
    function setupInteractiveControls(canvas) {
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
            
            // Rotate camera around the scene using spherical coordinates
            const radius = dataCamera.position.length();
            const currentTheta = Math.atan2(dataCamera.position.x, dataCamera.position.z);
            const currentPhi = Math.acos(dataCamera.position.y / radius);
            
            const newTheta = currentTheta - deltaX * 0.01;
            const newPhi = Math.max(0.1, Math.min(Math.PI - 0.1, currentPhi + deltaY * 0.01));
            
            dataCamera.position.x = radius * Math.sin(newPhi) * Math.sin(newTheta);
            dataCamera.position.y = radius * Math.cos(newPhi);
            dataCamera.position.z = radius * Math.sin(newPhi) * Math.cos(newTheta);
            
            dataCamera.lookAt(0, 0, 0);
            
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
            
            const distance = dataCamera.position.length();
            const newDistance = distance + (direction * zoomSpeed * distance);
            const clampedDistance = Math.max(3, Math.min(15, newDistance));
            
            dataCamera.position.normalize().multiplyScalar(clampedDistance);
        });
        
        canvas.style.cursor = 'grab';
    }
    
    /**
     * Create platonic solid based on model type
     */
    function createPlatonicSolid(modelType = 'dodecahedron') {
        // Remove existing solids
        if (dataSolid) dataScene.remove(dataSolid);
        if (dataFillSolid) dataScene.remove(dataFillSolid);
        if (dataParticles) dataScene.remove(dataParticles);
        
        // Create geometry based on model type
        let geometry, fillGeometry;
        const outerSize = 2;
        const innerSize = 0.1;
        
        switch (modelType) {
            case 'tetrahedron':
                geometry = new THREE.TetrahedronGeometry(outerSize, 0);
                fillGeometry = new THREE.TetrahedronGeometry(innerSize, 0);
                break;
            case 'octahedron':
                geometry = new THREE.OctahedronGeometry(outerSize, 0);
                fillGeometry = new THREE.OctahedronGeometry(innerSize, 0);
                break;
            case 'icosahedron':
                geometry = new THREE.IcosahedronGeometry(outerSize, 0);
                fillGeometry = new THREE.IcosahedronGeometry(innerSize, 0);
                break;
            case 'cube':
                geometry = new THREE.BoxGeometry(outerSize * 1.2, outerSize * 1.2, outerSize * 1.2);
                fillGeometry = new THREE.BoxGeometry(innerSize * 1.2, innerSize * 1.2, innerSize * 1.2);
                break;
            case 'sphere':
                geometry = new THREE.SphereGeometry(outerSize, 32, 32);
                fillGeometry = new THREE.SphereGeometry(innerSize, 16, 16);
                break;
            case 'torus':
                geometry = new THREE.TorusGeometry(outerSize, 0.4, 16, 100);
                fillGeometry = new THREE.TorusGeometry(innerSize, 0.02, 16, 100);
                break;
            case 'dodecahedron':
            default:
                geometry = new THREE.DodecahedronGeometry(outerSize, 0);
                fillGeometry = new THREE.DodecahedronGeometry(innerSize, 0);
                break;
        }
        
        // Outer wireframe solid
        const wireframeMaterial = new THREE.MeshBasicMaterial({
            color: 0x3b82f6,
            wireframe: true,
            transparent: true,
            opacity: 0.3
        });
        dataSolid = new THREE.Mesh(geometry, wireframeMaterial);
        dataScene.add(dataSolid);
        
        // Inner fill solid (starts empty)
        const fillMaterial = new THREE.MeshPhongMaterial({
            color: 0x3b82f6,
            transparent: true,
            opacity: 0.8,
            emissive: 0x1e3a8a,
            emissiveIntensity: 0.3
        });
        dataFillSolid = new THREE.Mesh(fillGeometry, fillMaterial);
        dataScene.add(dataFillSolid);
        
        // Particles system
        const particleCount = 100;
        const particlesGeometry = new THREE.BufferGeometry();
        const positions = new Float32Array(particleCount * 3);
        const colors = new Float32Array(particleCount * 3);
        
        for (let i = 0; i < particleCount; i++) {
            const i3 = i * 3;
            // Random position inside sphere
            const radius = Math.random() * 1.5;
            const theta = Math.random() * Math.PI * 2;
            const phi = Math.acos(Math.random() * 2 - 1);
            
            positions[i3] = radius * Math.sin(phi) * Math.cos(theta);
            positions[i3 + 1] = radius * Math.sin(phi) * Math.sin(theta);
            positions[i3 + 2] = radius * Math.cos(phi);
            
            // Blue color
            colors[i3] = 0.2;
            colors[i3 + 1] = 0.5;
            colors[i3 + 2] = 1.0;
        }
        
        particlesGeometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
        particlesGeometry.setAttribute('color', new THREE.BufferAttribute(colors, 3));
        
        const particlesMaterial = new THREE.PointsMaterial({
            size: 0.1,
            vertexColors: true,
            transparent: true,
            opacity: 0.6
        });
        
        dataParticles = new THREE.Points(particlesGeometry, particlesMaterial);
        dataScene.add(dataParticles);
    }
    
    /**
     * Update 3D visualization based on data count
     */
    function update3DVisualization(count) {
        currentDataCount = count;
        // Calculate fill level (0 to 1) - max at 20 projections for better fill effect
        targetFillLevel = Math.min(count / 20, 1);
        
        // Update count display
        const countEl = document.getElementById('data-viz-count');
        if (countEl) {
            countEl.textContent = count;
        }
    }
    
    /**
     * Animate 3D scene
     */
    function animate3D() {
        if (!dataRenderer || !dataScene || !dataCamera) return;
        
        dataAnimationId = requestAnimationFrame(animate3D);
        
        // Rotate outer solid (only if not dragging)
        if (dataSolid && !isDragging) {
            dataSolid.rotation.y += 0.005;
            dataSolid.rotation.x += 0.003;
        }
        
        // Update fill solid size based on data count
        if (dataFillSolid) {
            const currentScale = dataFillSolid.scale.x;
            // Scale from 0.1 to 1.95 (almost fills the outer solid) - more aggressive fill
            const targetScale = 0.1 + (targetFillLevel * 1.85);
            const newScale = currentScale + (targetScale - currentScale) * 0.08; // Faster interpolation for more responsive fill
            
            dataFillSolid.scale.set(newScale, newScale, newScale);
            
            // Update opacity and emissive based on fill - more dramatic changes
            dataFillSolid.material.opacity = 0.5 + (targetFillLevel * 0.5);
            dataFillSolid.material.emissiveIntensity = 0.4 + (targetFillLevel * 0.6);
            
            // Change color intensity based on fill
            const intensity = 0.3 + (targetFillLevel * 0.7);
            dataFillSolid.material.emissive.setRGB(0.12 * intensity, 0.23 * intensity, 0.5 * intensity);
        }
        
        // Rotate fill solid (only if not dragging)
        if (dataFillSolid && !isDragging) {
            dataFillSolid.rotation.y -= 0.008;
            dataFillSolid.rotation.x += 0.005;
        }
        
        // Animate particles
        if (dataParticles) {
            dataParticles.rotation.y += 0.002;
            
            // Show/hide particles based on fill level
            const positions = dataParticles.geometry.attributes.position.array;
            const visibleCount = Math.floor(targetFillLevel * positions.length / 3);
            
            for (let i = 0; i < positions.length / 3; i++) {
                const i3 = i * 3;
                if (i < visibleCount) {
                    // Make particle visible and animate
                    const radius = 0.5 + Math.random() * 1.5 * targetFillLevel;
                    const theta = Math.random() * Math.PI * 2;
                    const phi = Math.acos(Math.random() * 2 - 1);
                    
                    positions[i3] = radius * Math.sin(phi) * Math.cos(theta + dataParticles.rotation.y);
                    positions[i3 + 1] = radius * Math.sin(phi) * Math.sin(theta + dataParticles.rotation.y);
                    positions[i3 + 2] = radius * Math.cos(phi);
                } else {
                    // Hide particle
                    positions[i3] = 0;
                    positions[i3 + 1] = 0;
                    positions[i3 + 2] = 0;
                }
            }
            dataParticles.geometry.attributes.position.needsUpdate = true;
        }
        
        dataRenderer.render(dataScene, dataCamera);
    }
    
    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
    
    // Initialize 3D visualization when data page becomes active
    const dataPage = document.getElementById('page-data');
    if (dataPage) {
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (dataPage.classList.contains('active')) {
                        setTimeout(() => {
                            if (typeof THREE !== 'undefined' && !dataScene) {
                                init3DVisualization();
                            }
                            loadProjections();
                        }, 300);
                    }
                }
            });
        });
        observer.observe(dataPage, { attributes: true, attributeFilter: ['class'] });
    }
    
    return {
        init: init,
        loadProjections: loadProjections,
        refresh: loadProjections,
        init3DVisualization: init3DVisualization
    };
})();

// Expose for external use
window.DataPageModule = DataPageModule;

