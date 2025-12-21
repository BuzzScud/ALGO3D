/**
 * Data Page Module - Enhanced with DataTables and File Upload
 * Following Preline DataTables design patterns
 */
const DataPageModule = (function() {
    let dataTable = null;
    let allProjections = [];
    let selectedFile = null;
    let isInitializing = false; // Flag to prevent concurrent initializations
    
    /**
     * Initialize DataTables
     */
    function initDataTable() {
        const tableElement = document.getElementById('hs-datatable');
        if (!tableElement) return;
        
        // Destroy existing DataTable if it exists
        if (dataTable) {
            try {
                dataTable.destroy();
            } catch (e) {
                console.warn('Error destroying DataTable:', e);
            }
            dataTable = null;
        }
        
        // Check if DataTable is available
        if (typeof $.fn.DataTable === 'undefined') {
            console.error('DataTables is not loaded');
            showErrorModal('DataTables library is not loaded. Please refresh the page.');
            return;
        }
        
        try {
            dataTable = $(tableElement).DataTable({
                data: [],
                columns: [
                    {
                        data: 'symbol',
                        render: function(data, type, row) {
                            return `<span class="font-semibold text-primary">${data || 'N/A'}</span>`;
                        }
                    },
                    {
                        data: 'title',
                        render: function(data, type, row) {
                            const title = data || `${row.symbol || 'Untitled'} Projection`;
                            return `<span class="text-gray-900 dark:text-neutral-200">${title}</span>`;
                        }
                    },
                    {
                        data: 'saved_at',
                        render: function(data, type, row) {
                            if (!data) return '--';
                            const date = new Date(data);
                            const formatted = date.toLocaleDateString('en-US', {
                                month: 'short',
                                day: 'numeric',
                                year: 'numeric',
                                hour: '2-digit',
                                minute: '2-digit'
                            });
                            return `<span class="text-gray-600 dark:text-neutral-400">${formatted}</span>`;
                        }
                    },
                    {
                        data: 'projection_data',
                        render: function(data, type, row) {
                            try {
                                if (!data) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                
                                // Handle both string and object formats
                                const projData = typeof data === 'string' 
                                    ? (data.trim() ? JSON.parse(data) : null)
                                    : data;
                                
                                if (!projData) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                
                                if (projData.type === 'image') {
                                    return `<span class="text-gray-600 dark:text-neutral-400"><i class="fas fa-image"></i> PNG</span>`;
                                }
                                
                                // Try to get interval from various possible locations
                                const interval = projData.interval || projData.timeframe || '--';
                                return `<span class="text-gray-600 dark:text-neutral-400">${interval}</span>`;
                            } catch (e) {
                                console.warn('Error rendering projection_data:', e, data);
                                return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                            }
                        }
                    },
                    {
                        data: 'params',
                        render: function(data, type, row) {
                            try {
                                if (!data) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                
                                // Handle both string and object formats
                                const params = typeof data === 'string' 
                                    ? (data.trim() ? JSON.parse(data) : null)
                                    : data;
                                
                                if (!params) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                
                                return `<span class="text-gray-600 dark:text-neutral-400">${params.steps || '--'}</span>`;
                            } catch (e) {
                                console.warn('Error rendering params:', e, data);
                                return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                            }
                        }
                    },
                    {
                        data: null,
                        orderable: false,
                        className: 'text-center',
                        render: function(data, type, row) {
                            const rowId = row.id || '';
                            const symbol = escapeHtml(row.symbol || 'projection');
                            const title = escapeHtml(row.title || `${row.symbol || 'Untitled'} Projection`);
                            return `
                                <div class="datatable-actions flex justify-center items-center gap-2" data-row-id="${rowId}">
                                    <button type="button" 
                                            class="btn-action btn-view action-btn" 
                                            data-id="${rowId}" 
                                            data-action="view"
                                            data-symbol="${symbol}"
                                            data-title="${title}"
                                            title="View ${title}"
                                            aria-label="View Projection: ${title}"
                                            data-tooltip="View Projection">
                                        <i class="fas fa-eye"></i>
                                        <span class="btn-action-label">View</span>
                                    </button>
                                    <button type="button" 
                                            class="btn-action btn-export action-btn" 
                                            data-id="${rowId}" 
                                            data-action="export"
                                            data-symbol="${symbol}"
                                            data-title="${title}"
                                            title="Export ${title}"
                                            aria-label="Export Projection: ${title}"
                                            data-tooltip="Export Projection">
                                        <i class="fas fa-download"></i>
                                        <span class="btn-action-label">Export</span>
                                    </button>
                                    <button type="button" 
                                            class="btn-action btn-delete delete-projection-btn action-btn" 
                                            data-id="${rowId}" 
                                            data-action="delete"
                                            data-symbol="${symbol}"
                                            data-title="${title}"
                                            title="Delete ${title}"
                                            aria-label="Delete Projection: ${title}"
                                            data-tooltip="Delete Projection">
                                        <i class="fas fa-trash-alt"></i>
                                        <span class="btn-action-label">Delete</span>
                                    </button>
                                </div>
                            `;
                        }
                    }
                ],
                order: [[2, 'desc']], // Sort by saved_at descending
                pageLength: 10,
                responsive: true,
                language: {
                    search: '',
                    searchPlaceholder: 'Search projections...',
                    lengthMenu: 'Show _MENU_ entries',
                    info: 'Showing _START_ to _END_ of _TOTAL_ entries',
                    infoEmpty: 'No entries found',
                    infoFiltered: '(filtered from _MAX_ total entries)',
                    paginate: {
                        previous: '<i class="fas fa-chevron-left"></i>',
                        next: '<i class="fas fa-chevron-right"></i>'
                    }
                },
                dom: '<"flex flex-wrap items-center justify-between gap-2"<"flex items-center gap-2"l><"flex items-center gap-2"f>>rt<"flex flex-wrap items-center justify-between gap-2"<"flex items-center gap-2"i><"flex items-center gap-2"p>>',
                drawCallback: function(settings) {
                    console.log('DataTable drawCallback - visible rows:', this.api().rows({page: 'current'}).count());
                    attachActionListeners();
                },
                initComplete: function(settings, json) {
                    console.log('DataTable initialized');
                }
            });
            
            // Customize search box styling
            $('.dataTables_filter input').addClass('form-input');
            $('.dataTables_length select').addClass('form-select');
            
            // Add support for Cmd+A / Ctrl+A in search input (as per Preline docs)
            const searchInputs = document.querySelectorAll('.dataTables_filter input');
            searchInputs.forEach((input) => {
                input.addEventListener('keydown', function (evt) {
                    if ((evt.metaKey || evt.ctrlKey) && evt.key === 'a') {
                        this.select();
                    }
                });
            });
            
        } catch (error) {
            console.error('Error initializing DataTable:', error);
            showErrorModal('Failed to initialize data table. Please refresh the page.');
        }
    }
    
    /**
     * Load all saved projections
     */
    async function loadProjections() {
        const loadingEl = document.getElementById('datatable-loading');
        const errorEl = document.getElementById('datatable-error');
        const tableWrapper = document.querySelector('.datatable-wrapper');
        
        if (loadingEl) loadingEl.style.display = 'flex';
        if (errorEl) errorEl.style.display = 'none';
        
        try {
            console.log('Loading projections from API...');
            const response = await fetch('api/projections.php');
            
            if (!response.ok) {
                const errorText = await response.text();
                console.error('API response error:', response.status, errorText);
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            console.log('API response:', result);
            
            if (!result) {
                throw new Error('Invalid response from server');
            }
            
            if (result.success && result.projections) {
                allProjections = result.projections;
                console.log(`✓ Loaded ${allProjections.length} projections from API`);
                
                // Prepare data for DataTable - ensure all fields are properly formatted
                const tableData = allProjections.map(proj => {
                    return {
                        id: proj.id || null,
                        symbol: proj.symbol || 'N/A',
                        title: proj.title || 'Untitled Projection',
                        saved_at: proj.saved_at || new Date().toISOString(),
                        projection_data: proj.projection_data || null,
                        chart_data: proj.chart_data || null,
                        params: proj.params || null,
                        notes: proj.notes || null
                    };
                });
                
                console.log('✓ Prepared', tableData.length, 'rows for DataTable');
                console.log('Sample row data:', JSON.stringify(tableData[0], null, 2));
                
                // Prevent concurrent initializations
                if (isInitializing) {
                    console.log('DataTable initialization already in progress, skipping...');
                    return;
                }
                
                isInitializing = true;
                
                try {
                    // Destroy and recreate DataTable with new data
                    const tableElement = document.getElementById('hs-datatable');
                    if (!tableElement) {
                        throw new Error('Table element not found');
                    }
                    
                    // Check if DataTable is available
                    if (typeof $.fn.DataTable === 'undefined') {
                        throw new Error('DataTables library is not loaded');
                    }
                    
                    // Properly destroy existing DataTable if it exists
                    if (dataTable) {
                        try {
                            console.log('Destroying existing DataTable instance...');
                            dataTable.destroy();
                            dataTable = null;
                        } catch (destroyError) {
                            console.warn('Error destroying DataTable instance:', destroyError);
                            dataTable = null;
                        }
                    }
                    
                    // Also check if jQuery has a DataTable on this element (prevents reinitialization error)
                    if ($.fn.DataTable.isDataTable(tableElement)) {
                        try {
                            console.log('Destroying jQuery DataTable on element...');
                            $(tableElement).DataTable().destroy();
                        } catch (jqError) {
                            console.warn('Error destroying jQuery DataTable:', jqError);
                        }
                    }
                    
                    // Clear the table HTML to ensure clean state
                    $(tableElement).empty();
                    
                    // Small delay to ensure cleanup completes
                    await new Promise(resolve => setTimeout(resolve, 150));
                    
                    // Double-check that no DataTable exists before creating new one
                    if ($.fn.DataTable.isDataTable(tableElement)) {
                        console.warn('DataTable still exists after destruction, forcing cleanup...');
                        try {
                            $(tableElement).DataTable().destroy();
                            $(tableElement).empty();
                            await new Promise(resolve => setTimeout(resolve, 50));
                        } catch (forceError) {
                            console.error('Force cleanup failed:', forceError);
                            // Try to remove the table element and recreate it
                            const parent = tableElement.parentNode;
                            const newTable = tableElement.cloneNode(false);
                            parent.replaceChild(newTable, tableElement);
                            // Update reference
                            const updatedElement = document.getElementById('hs-datatable');
                            if (updatedElement) {
                                Object.assign(tableElement, updatedElement);
                            }
                        }
                    }
                    
                    // Reinitialize DataTable with the data
                    console.log('Initializing DataTable with', tableData.length, 'rows...');
                    
                    // Initialize with data directly
                    dataTable = $(tableElement).DataTable({
                        data: tableData, // Use data option directly instead of rows.add()
                        columns: [
                            {
                                data: 'symbol',
                                render: function(data, type, row) {
                                    return `<span class="font-semibold text-primary">${data || 'N/A'}</span>`;
                                }
                            },
                            {
                                data: 'title',
                                render: function(data, type, row) {
                                    const title = data || `${row.symbol || 'Untitled'} Projection`;
                                    return `<span class="text-gray-900 dark:text-neutral-200">${title}</span>`;
                                }
                            },
                            {
                                data: 'saved_at',
                                render: function(data, type, row) {
                                    if (!data) return '--';
                                    try {
                                        const date = new Date(data);
                                        const formatted = date.toLocaleDateString('en-US', {
                                            month: 'short',
                                            day: 'numeric',
                                            year: 'numeric',
                                            hour: '2-digit',
                                            minute: '2-digit'
                                        });
                                        return `<span class="text-gray-600 dark:text-neutral-400">${formatted}</span>`;
                                    } catch (e) {
                                        return `<span class="text-gray-600 dark:text-neutral-400">${data}</span>`;
                                    }
                                }
                            },
                            {
                                data: 'projection_data',
                                render: function(data, type, row) {
                                    try {
                                        if (!data) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                        
                                        const projData = typeof data === 'string' 
                                            ? (data.trim() ? JSON.parse(data) : null)
                                            : data;
                                        
                                        if (!projData) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                        
                                        if (projData.type === 'image') {
                                            return `<span class="text-gray-600 dark:text-neutral-400"><i class="fas fa-image"></i> PNG</span>`;
                                        }
                                        
                                        const interval = projData.interval || projData.timeframe || '--';
                                        return `<span class="text-gray-600 dark:text-neutral-400">${interval}</span>`;
                                    } catch (e) {
                                        console.warn('Error rendering projection_data:', e);
                                        return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                    }
                                }
                            },
                            {
                                data: 'params',
                                render: function(data, type, row) {
                                    try {
                                        if (!data) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                        
                                        const params = typeof data === 'string' 
                                            ? (data.trim() ? JSON.parse(data) : null)
                                            : data;
                                        
                                        if (!params) return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                        
                                        return `<span class="text-gray-600 dark:text-neutral-400">${params.steps || '--'}</span>`;
                                    } catch (e) {
                                        console.warn('Error rendering params:', e);
                                        return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                                    }
                                }
                            },
                            {
                                data: null,
                                orderable: false,
                                className: 'text-end',
                                render: function(data, type, row) {
                                    const rowId = row.id || '';
                                    const symbol = escapeHtml(row.symbol || 'projection');
                                    const title = escapeHtml(row.title || `${row.symbol || 'Untitled'} Projection`);
                                    return `
                                        <div class="p-3 whitespace-nowrap text-end text-sm font-medium inline-flex items-center gap-x-2" data-row-id="${rowId}">
                                            <button type="button" 
                                                    class="btn-action btn-view action-btn inline-flex items-center gap-x-2 text-sm font-semibold rounded-lg border border-transparent text-blue-600 hover:text-blue-800 focus:outline-hidden focus:text-blue-800 disabled:opacity-50 disabled:pointer-events-none dark:text-blue-500 dark:hover:text-blue-400 dark:focus:text-blue-400" 
                                                    data-id="${rowId}" 
                                                    data-action="view"
                                                    data-symbol="${symbol}"
                                                    data-title="${title}"
                                                    title="View ${title}"
                                                    aria-label="View Projection: ${title}">
                                                <i class="fas fa-eye"></i>
                                                View
                                            </button>
                                            <button type="button" 
                                                    class="btn-action btn-export action-btn inline-flex items-center gap-x-2 text-sm font-semibold rounded-lg border border-transparent text-blue-600 hover:text-blue-800 focus:outline-hidden focus:text-blue-800 disabled:opacity-50 disabled:pointer-events-none dark:text-blue-500 dark:hover:text-blue-400 dark:focus:text-blue-400" 
                                                    data-id="${rowId}" 
                                                    data-action="export"
                                                    data-symbol="${symbol}"
                                                    data-title="${title}"
                                                    title="Export ${title}"
                                                    aria-label="Export Projection: ${title}">
                                                <i class="fas fa-download"></i>
                                                Export
                                            </button>
                                            <button type="button" 
                                                    class="btn-action btn-delete delete-projection-btn action-btn inline-flex items-center gap-x-2 text-sm font-semibold rounded-lg border border-transparent text-blue-600 hover:text-blue-800 focus:outline-hidden focus:text-blue-800 disabled:opacity-50 disabled:pointer-events-none dark:text-blue-500 dark:hover:text-blue-400 dark:focus:text-blue-400" 
                                                    data-id="${rowId}" 
                                                    data-action="delete"
                                                    data-symbol="${symbol}"
                                                    data-title="${title}"
                                                    title="Delete ${title}"
                                                    aria-label="Delete Projection: ${title}">
                                                <i class="fas fa-trash-alt"></i>
                                                Delete
                                            </button>
                                        </div>
                                    `;
                                }
                            }
                        ],
                        order: [[2, 'desc']], // Sort by saved_at descending
                        pageLength: 10,
                        responsive: true,
                        language: {
                            search: '',
                            searchPlaceholder: 'Search projections...',
                            lengthMenu: 'Show _MENU_ entries',
                            info: 'Showing _START_ to _END_ of _TOTAL_ entries',
                            infoEmpty: 'No entries found',
                            infoFiltered: '(filtered from _MAX_ total entries)',
                            paginate: {
                                previous: '<i class="fas fa-chevron-left"></i>',
                                next: '<i class="fas fa-chevron-right"></i>'
                            }
                        },
                        dom: 'rt', // Remove default controls, we use Preline structure
                        drawCallback: function(settings) {
                            const api = this.api();
                            const visibleRows = api.rows({page: 'current'}).count();
                            console.log('DataTable drawCallback - visible rows:', visibleRows);
                            attachActionListeners();
                        },
                        initComplete: function(settings, json) {
                            // json parameter may not have .data when using data option directly
                            const rowCount = this.api().rows().count();
                            console.log('✓ DataTable initialized successfully with', rowCount, 'rows');
                            
                            // Connect Preline search input to DataTables
                            const searchInput = document.getElementById('hs-table-destroy-and-reinitialize-search');
                            if (searchInput) {
                                // Remove any existing listeners
                                const newSearchInput = searchInput.cloneNode(true);
                                searchInput.parentNode.replaceChild(newSearchInput, searchInput);
                                
                                // Add new listener
                                newSearchInput.addEventListener('keyup', function() {
                                    dataTable.search(this.value).draw();
                                });
                                
                                // Add support for Cmd+A / Ctrl+A
                                newSearchInput.addEventListener('keydown', function (evt) {
                                    if ((evt.metaKey || evt.ctrlKey) && evt.key === 'a') {
                                        this.select();
                                    }
                                });
                            }
                        }
                    });
                    
                    console.log('✓ DataTable created successfully');
                    const finalRowCount = dataTable.rows().count();
                    console.log('✓ Final row count:', finalRowCount);
                    
                } catch (initError) {
                    console.error('❌ Error initializing DataTable:', initError);
                    console.error('Init error stack:', initError.stack);
                    showDataTableError(new Error('Failed to initialize data table: ' + initError.message));
                    throw initError;
                } finally {
                    // Always reset the flag to allow future initializations
                    isInitializing = false;
                }
                
                // Update 3D visualization safely
                try {
                    if (typeof update3DVisualization === 'function') {
                        update3DVisualization(allProjections.length);
                    }
                } catch (vizError) {
                    console.error('Error updating 3D visualization:', vizError);
                    // Non-critical, continue
                }
            } else {
                console.log('No projections found or API returned error:', result);
                allProjections = [];
                if (dataTable) {
                    dataTable.clear();
                    dataTable.draw();
                }
                update3DVisualization(0);
                
                // Show message if there's an error
                if (result && !result.success && result.message) {
                    showDataTableError(new Error(result.message));
                }
            }
        } catch (error) {
            console.error('Error loading projections:', error);
            console.error('Error details:', {
                name: error.name,
                message: error.message,
                stack: error.stack
            });
            showDataTableError(error);
        } finally {
            if (loadingEl) loadingEl.style.display = 'none';
        }
    }
    
    /**
     * Safe reload projections - prevents crashes (DEPRECATED - not used in delete flow)
     * Kept for backward compatibility but delete now uses direct row removal
     */
    async function safeReloadProjections() {
        // Prevent concurrent reloads
        if (isInitializing) {
            console.log('Reload already in progress, skipping...');
            return;
        }
        
        try {
            console.log('Starting safe reload...');
            await loadProjections();
            console.log('✓ Safe reload completed');
        } catch (reloadError) {
            console.error('Error in safe reload:', reloadError);
            // Don't throw - just log the error
            // User can manually refresh if needed
            throw new Error('Load failed'); // This is caught and handled by caller
        }
    }
    
    /**
     * Show error in DataTable
     */
    function showDataTableError(error) {
        const errorEl = document.getElementById('datatable-error');
        const errorMsg = document.getElementById('datatable-error-message');
        
        if (errorEl && errorMsg) {
            let errorMessage = 'Error loading data. ';
            if (error.message && error.message.includes('Failed to fetch')) {
                errorMessage += 'Server connection failed. Please ensure the web server is running at http://localhost:8080';
            } else if (error.message && error.message.includes('HTTP')) {
                errorMessage += `Server error: ${error.message}`;
            } else {
                errorMessage += error.message || 'Unknown error occurred';
            }
            
            errorMsg.textContent = errorMessage;
            errorEl.style.display = 'flex';
        }
    }
    
    /**
     * Attach action button listeners using event delegation for reliability
     * Enhanced with loading states, error handling, and visual feedback
     */
    function attachActionListeners() {
        // Remove existing listeners to prevent duplicates
        const actionsContainer = document.getElementById('hs-datatable');
        if (!actionsContainer) return;
        
        // Use event delegation on the table body for better reliability
        const tableBody = actionsContainer.querySelector('tbody');
        if (!tableBody) return;
        
        // Remove old event listeners by cloning (breaks old references)
        const newTableBody = tableBody.cloneNode(true);
        tableBody.parentNode.replaceChild(newTableBody, tableBody);
        
        // Attach event delegation on the new table body - REDESIGNED for crash prevention
        newTableBody.addEventListener('click', async function(e) {
            // Wrap everything in try-catch to prevent crashes
            try {
                // Find the button - try multiple strategies for reliability
                let btn = e.target.closest('.action-btn');
                
                // If closest didn't work, try finding parent button
                if (!btn && (e.target.closest('button') || e.target.tagName === 'BUTTON')) {
                    const button = e.target.closest('button') || e.target;
                    if (button && button.classList.contains('action-btn')) {
                        btn = button;
                    }
                }
                
                // Also check if target itself is a button with action-btn class
                if (!btn && e.target.classList && e.target.classList.contains('action-btn')) {
                    btn = e.target;
                }
                
                // Also check for delete-projection-btn class specifically
                if (!btn) {
                    btn = e.target.closest('.delete-projection-btn') || 
                          e.target.closest('.btn-delete') ||
                          e.target.closest('.btn-view') ||
                          e.target.closest('.btn-export');
                }
                
                if (!btn) {
                    // Not an action button, ignore
                    console.log('No action button found for click target:', e.target);
                    return;
                }
                
                // Log for debugging
                console.log('Action button found:', btn, 'Action:', btn.dataset.action, 'ID:', btn.dataset.id);
                
                // Check if button is disabled or processing
                if (btn.disabled || btn.classList.contains('processing')) {
                    e.stopPropagation();
                    e.preventDefault();
                    return;
                }
                
                e.stopPropagation();
                e.preventDefault();
                
                const action = btn.dataset.action;
                const idStr = btn.dataset.id;
                
                // Validate ID
                if (!idStr || idStr === 'undefined' || idStr === 'null' || idStr === '') {
                    console.error('Invalid ID for action:', action, idStr);
                    showErrorModal('Invalid projection ID. Please refresh the page and try again.');
                    return;
                }
                
                const id = parseInt(idStr, 10);
                if (isNaN(id) || id <= 0) {
                    console.error('Invalid ID parsed:', idStr, id);
                    showErrorModal('Invalid projection ID. Please refresh the page and try again.');
                    return;
                }
                
                // Set button to processing state (only if button still exists)
                if (btn && document.body.contains(btn)) {
                    setButtonProcessing(btn, true);
                }
                
                // Handle action based on type
                switch (action) {
                    case 'view':
                        try {
                            await handleViewAction(id, btn);
                        } catch (error) {
                            console.error('Error in view action:', error);
                            showErrorModal('Failed to view projection: ' + (error.message || 'Unknown error'));
                            if (btn && document.body.contains(btn)) {
                                setButtonError(btn);
                                setTimeout(() => {
                                    if (btn && document.body.contains(btn)) {
                                        setButtonProcessing(btn, false);
                                    }
                                }, 2000);
                            }
                        }
                        break;
                        
                    case 'export':
                        try {
                            await handleExportAction(id, btn);
                        } catch (error) {
                            console.error('Error in export action:', error);
                            showErrorModal('Failed to export projection: ' + (error.message || 'Unknown error'));
                            if (btn && document.body.contains(btn)) {
                                setButtonError(btn);
                                setTimeout(() => {
                                    if (btn && document.body.contains(btn)) {
                                        setButtonProcessing(btn, false);
                                    }
                                }, 2000);
                            }
                        }
                        break;
                        
                    case 'delete':
                        // Delete action handles its own errors and button state
                        const symbol = btn.dataset.symbol || 'this projection';
                        await handleDeleteAction(id, symbol, btn);
                        // Don't reset button state here - delete handles it
                        break;
                        
                    default:
                        console.warn('Unknown action:', action);
                        showErrorModal('Unknown action. Please refresh the page.');
                        if (btn && document.body.contains(btn)) {
                            setButtonProcessing(btn, false);
                        }
                }
            } catch (error) {
                // Catch any unexpected errors to prevent crashes
                console.error('❌ Unexpected error in action handler:', error);
                console.error('Error stack:', error.stack);
                showErrorModal('An unexpected error occurred. Please refresh the page and try again.');
            }
        });
    }
    
    /**
     * Set button to processing state
     */
    function setButtonProcessing(btn, isProcessing) {
        if (!btn) return;
        
        if (isProcessing) {
            btn.disabled = true;
            btn.classList.add('processing');
            const icon = btn.querySelector('i');
            if (icon) {
                icon.className = 'fas fa-spinner fa-spin';
            }
        } else {
            btn.disabled = false;
            btn.classList.remove('processing', 'success', 'error');
            // Restore original icon
            const action = btn.dataset.action;
            const icon = btn.querySelector('i');
            if (icon) {
                switch (action) {
                    case 'view':
                        icon.className = 'fas fa-eye';
                        break;
                    case 'export':
                        icon.className = 'fas fa-download';
                        break;
                    case 'delete':
                        icon.className = 'fas fa-trash-alt';
                        break;
                }
            }
        }
    }
    
    /**
     * Set button to success state
     */
    function setButtonSuccess(btn) {
        if (!btn) return;
        btn.classList.add('success');
        const icon = btn.querySelector('i');
        if (icon) {
            icon.className = 'fas fa-check';
        }
    }
    
    /**
     * Set button to error state
     */
    function setButtonError(btn) {
        if (!btn) return;
        btn.classList.add('error');
        const icon = btn.querySelector('i');
        if (icon) {
            icon.className = 'fas fa-exclamation-triangle';
        }
    }
    
    /**
     * Handle view action with error handling
     */
    async function handleViewAction(id, btn) {
        try {
            const projection = allProjections.find(p => p.id === id);
            if (!projection) {
                throw new Error('Projection not found in local data');
            }
            viewProjection(id);
            setButtonSuccess(btn);
        } catch (error) {
            console.error('Error in handleViewAction:', error);
            throw error;
        }
    }
    
    /**
     * Handle export action with error handling
     */
    async function handleExportAction(id, btn) {
        try {
            const projection = allProjections.find(p => p.id === id);
            if (!projection) {
                throw new Error('Projection not found in local data');
            }
            exportProjection(id);
            setButtonSuccess(btn);
            // Show notification
            if (typeof showNotification === 'function') {
                showNotification('Projection exported successfully', 'success');
            }
        } catch (error) {
            console.error('Error in handleExportAction:', error);
            throw error;
        }
    }
    
    /**
     * Handle delete action with error handling - COMPLETELY REDESIGNED FOR CRASH PREVENTION
     */
    async function handleDeleteAction(id, symbol, btn) {
        // Validate inputs
        if (!id || !symbol || !btn) {
            console.error('Invalid parameters for delete action:', {id, symbol, btn: !!btn});
            showErrorModal('Invalid delete request. Please refresh the page and try again.');
            return;
        }
        
        // Show confirmation dialog with error handling
        let confirmed = false;
        try {
            confirmed = confirm(`Are you sure you want to delete "${symbol}"? This action cannot be undone.`);
        } catch (confirmError) {
            console.error('Error showing confirmation:', confirmError);
            if (btn && document.body.contains(btn)) {
                setButtonProcessing(btn, false);
            }
            return;
        }
        
        if (!confirmed) {
            // User cancelled - reset button state
            if (btn && document.body.contains(btn)) {
                setButtonProcessing(btn, false);
            }
            return;
        }
        
        // Set button to processing state
        if (btn && document.body.contains(btn)) {
            setButtonProcessing(btn, true);
        }
        
        try {
            // Call delete function - wrapped in try-catch for safety
            await deleteProjection(id);
            
            // Success - button will be recreated when table updates
            // Don't try to update button state here
            
        } catch (error) {
            console.error('❌ Error in handleDeleteAction:', error);
            console.error('Error stack:', error.stack);
            
            // Reset button state on error
            if (btn && document.body.contains(btn)) {
                setButtonProcessing(btn, false);
                setButtonError(btn);
                
                // Reset error state after 2 seconds
                setTimeout(() => {
                    if (btn && document.body.contains(btn)) {
                        setButtonProcessing(btn, false);
                    }
                }, 2000);
            }
            
            // Show error message if not already shown
            const errorMsg = error.message || 'Unknown error occurred';
            if (!errorMsg.includes('already shown')) {
                showErrorModal('Failed to delete projection: ' + errorMsg);
            }
        }
    }
    
    
    /**
     * View projection in modal preview
     */
    function viewProjection(id) {
        const projection = allProjections.find(p => p.id === id);
        if (!projection) {
            showErrorModal('Projection not found');
            return;
        }
        
        // Remove existing preview modal if present
        const existingModal = document.getElementById('projection-preview-modal');
        if (existingModal) {
            existingModal.remove();
        }
        
        // Create preview modal
        const modal = document.createElement('div');
        modal.id = 'projection-preview-modal';
        modal.className = 'data-error-modal';
        modal.innerHTML = `
            <div class="data-error-modal-overlay"></div>
            <div class="data-error-modal-content" style="max-width: 800px; max-height: 90vh;">
                <div class="data-error-modal-header" style="display: flex; justify-content: space-between; align-items: center; width: 100%;">
                    <div style="display: flex; align-items: center; gap: 0.75rem;">
                        <i class="fas fa-eye"></i>
                        <h3 style="margin: 0;">View Projection: ${escapeHtml(projection.title || projection.symbol + ' Projection')}</h3>
                    </div>
                    <button class="modal-close" id="close-preview-modal" title="Close">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="data-error-modal-body" id="preview-modal-content" style="padding: 1.5rem; overflow-y: auto; max-height: calc(90vh - 150px);">
                    <div style="text-align: center; padding: 2rem;">
                        <div class="spinning">
                            <i class="fas fa-spinner"></i>
                        </div>
                        <p style="margin-top: 1rem; color: var(--text-secondary);">Loading preview...</p>
                    </div>
                </div>
                <div class="data-error-modal-footer">
                    <button class="btn btn-secondary" id="download-original-btn" data-id="${projection.id}">
                        <i class="fas fa-download"></i>
                        Download Original File
                    </button>
                    <button class="btn btn-primary" id="close-preview-modal-btn">
                        Close
                    </button>
                </div>
            </div>
        `;
        
        document.body.appendChild(modal);
        
        // Show modal with animation
        setTimeout(() => {
            modal.classList.add('active');
            loadProjectionPreview(projection, modal);
        }, 10);
        
        // Close button handlers
        const closeModal = () => {
            modal.classList.remove('active');
            setTimeout(() => {
                modal.remove();
            }, 300);
        };
        
        const closeBtn = modal.querySelector('#close-preview-modal');
        const closeBtnFooter = modal.querySelector('#close-preview-modal-btn');
        const overlay = modal.querySelector('.data-error-modal-overlay');
        
        if (closeBtn) closeBtn.addEventListener('click', closeModal);
        if (closeBtnFooter) closeBtnFooter.addEventListener('click', closeModal);
        if (overlay) overlay.addEventListener('click', closeModal);
        
        // Download original file button
        const downloadBtn = modal.querySelector('#download-original-btn');
        if (downloadBtn) {
            downloadBtn.addEventListener('click', function() {
                downloadOriginalFile(projection);
            });
        }
    }
    
    /**
     * Load projection preview content
     */
    function loadProjectionPreview(projection, modal) {
        const contentEl = modal.querySelector('#preview-modal-content');
        if (!contentEl) return;
        
        try {
            // Parse projection data
            const projData = typeof projection.projection_data === 'string' 
                ? JSON.parse(projection.projection_data) 
                : projection.projection_data;
            
            // Check if it's an image type
            if (projData && projData.type === 'image' && projData.image_path) {
                // Display PNG image
                contentEl.innerHTML = `
                    <div style="text-align: center;">
                        <h4 style="color: var(--text-primary); margin-bottom: 1rem;">Projection Image Preview</h4>
                        <div style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; border: 1px solid var(--border-color); display: inline-block;">
                            <img src="${escapeHtml(projData.image_path)}" 
                                 alt="Projection Image" 
                                 style="max-width: 100%; max-height: 600px; border-radius: 0.5rem;"
                                 onerror="this.parentElement.innerHTML='<p style=\\'color: var(--danger-color);\\'>Failed to load image</p>'">
                        </div>
                        <div style="margin-top: 1rem; text-align: left; color: var(--text-secondary); font-size: 0.9rem;">
                            <p><strong>Original Filename:</strong> ${escapeHtml(projData.original_filename || 'Unknown')}</p>
                            <p><strong>File Size:</strong> ${formatFileSize(projData.file_size || 0)}</p>
                            ${projection.notes ? `<p><strong>Notes:</strong> ${escapeHtml(projection.notes)}</p>` : ''}
                        </div>
                    </div>
                `;
            } else {
                // Display JSON data in formatted view
                const chartData = projection.chart_data ? (typeof projection.chart_data === 'string' 
                    ? JSON.parse(projection.chart_data) 
                    : projection.chart_data) : null;
                const params = projection.params ? (typeof projection.params === 'string' 
                    ? JSON.parse(projection.params) 
                    : projection.params) : null;
                
                contentEl.innerHTML = `
                    <div style="color: var(--text-primary);">
                        <div style="margin-bottom: 1.5rem;">
                            <h4 style="color: var(--primary-color); margin-bottom: 0.5rem;">Projection Information</h4>
                            <div style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; border: 1px solid var(--border-color);">
                                <p><strong>Symbol:</strong> ${escapeHtml(projection.symbol)}</p>
                                <p><strong>Title:</strong> ${escapeHtml(projection.title)}</p>
                                <p><strong>Saved Date:</strong> ${escapeHtml(projection.saved_at)}</p>
                                ${projection.notes ? `<p><strong>Notes:</strong> ${escapeHtml(projection.notes)}</p>` : ''}
                            </div>
                        </div>
                        
                        ${params ? `
                        <div style="margin-bottom: 1.5rem;">
                            <h4 style="color: var(--primary-color); margin-bottom: 0.5rem;">Projection Configuration</h4>
                            <div style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; border: 1px solid var(--border-color);">
                                <pre style="margin: 0; color: var(--text-primary); font-family: 'Courier New', monospace; font-size: 0.85rem; white-space: pre-wrap; word-wrap: break-word;">${escapeHtml(JSON.stringify(params, null, 2))}</pre>
                            </div>
                        </div>
                        ` : ''}
                        
                        <div style="margin-bottom: 1.5rem;">
                            <h4 style="color: var(--primary-color); margin-bottom: 0.5rem;">Projection Data</h4>
                            <div style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; border: 1px solid var(--border-color); max-height: 400px; overflow-y: auto;">
                                <pre style="margin: 0; color: var(--text-primary); font-family: 'Courier New', monospace; font-size: 0.85rem; white-space: pre-wrap; word-wrap: break-word;">${escapeHtml(JSON.stringify(projData, null, 2))}</pre>
                            </div>
                        </div>
                        
                        ${chartData ? `
                        <div style="margin-bottom: 1.5rem;">
                            <h4 style="color: var(--primary-color); margin-bottom: 0.5rem;">Chart Data</h4>
                            <div style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; border: 1px solid var(--border-color); max-height: 400px; overflow-y: auto;">
                                <pre style="margin: 0; color: var(--text-primary); font-family: 'Courier New', monospace; font-size: 0.85rem; white-space: pre-wrap; word-wrap: break-word;">${escapeHtml(JSON.stringify(chartData, null, 2))}</pre>
                            </div>
                        </div>
                        ` : ''}
                    </div>
                `;
            }
        } catch (error) {
            console.error('Error loading preview:', error);
            contentEl.innerHTML = `
                <div style="text-align: center; padding: 2rem;">
                    <i class="fas fa-exclamation-triangle" style="font-size: 2rem; color: var(--danger-color); margin-bottom: 1rem;"></i>
                    <p style="color: var(--danger-color);">Failed to load preview: ${escapeHtml(error.message || 'Unknown error')}</p>
                </div>
            `;
        }
    }
    
    /**
     * Download original uploaded file
     */
    function downloadOriginalFile(projection) {
        try {
            const projData = typeof projection.projection_data === 'string' 
                ? JSON.parse(projection.projection_data) 
                : projection.projection_data;
            
            // If it's an image, download the image file
            if (projData && projData.type === 'image' && projData.image_path) {
                // Fetch image as blob and download
                fetch(projData.image_path)
                    .then(response => {
                        if (!response.ok) throw new Error('Failed to fetch image');
                        return response.blob();
                    })
                    .then(blob => {
                        const url = URL.createObjectURL(blob);
                        const link = document.createElement('a');
                        link.href = url;
                        link.download = projData.original_filename || `projection_${projection.id}.png`;
                        document.body.appendChild(link);
                        link.click();
                        document.body.removeChild(link);
                        URL.revokeObjectURL(url);
                        showNotification('Image downloaded successfully', 'success');
                    })
                    .catch(error => {
                        console.error('Error downloading image:', error);
                        showErrorModal('Failed to download image: ' + (error.message || 'Unknown error'));
                    });
                return; // Exit early for async image download
            } else {
                // For JSON data, export as JSON file
                const exportData = {
                    id: projection.id,
                    symbol: projection.symbol,
                    title: projection.title,
                    projection_data: projData,
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
                link.download = `${projection.symbol}_projection_${projection.id}.json`;
                document.body.appendChild(link);
                link.click();
                document.body.removeChild(link);
                URL.revokeObjectURL(url);
                showNotification('File downloaded successfully', 'success');
            }
        } catch (error) {
            console.error('Error downloading file:', error);
            showErrorModal('Failed to download file: ' + (error.message || 'Unknown error'));
        }
    }
    
    /**
     * Escape HTML to prevent XSS
     */
    function escapeHtml(text) {
        if (text === null || text === undefined) return '';
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }
    
    /**
     * Export projection (downloads original file)
     */
    function exportProjection(id) {
        const projection = allProjections.find(p => p.id === id);
        if (!projection) {
            showErrorModal('Projection not found');
            return;
        }
        
        // Use the same download logic as in the modal
        downloadOriginalFile(projection);
    }
    
    /**
     * Delete projection
     */
    async function deleteProjection(id) {
        // Validate ID
        if (!id || (typeof id !== 'number' && isNaN(parseInt(id, 10)))) {
            showErrorModal('Invalid projection ID');
            return;
        }
        
        // Convert to number
        const numId = parseInt(id, 10);
        if (isNaN(numId) || numId <= 0) {
            showErrorModal('Invalid projection ID');
            return;
        }
        
        // Confirmation is now handled in deleteProjectionWithConfirmation
        // Proceed with deletion
        try {
            console.log('Deleting projection with ID:', numId);
            
            // Make DELETE request
            const response = await fetch('api/projections.php', {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json'
                },
                body: JSON.stringify({id: numId})
            });
            
            console.log('Delete response status:', response.status);
            
            // Check if response is ok
            if (!response.ok) {
                let errorMessage = `HTTP ${response.status}: ${response.statusText}`;
                try {
                    const errorText = await response.text();
                    if (errorText) {
                        const errorData = JSON.parse(errorText);
                        if (errorData.message) {
                            errorMessage = errorData.message;
                        }
                    }
                } catch (e) {
                    // If we can't parse JSON, use the status text
                }
                throw new Error(errorMessage);
            }
            
            // Parse response
            let result;
            try {
                const responseText = await response.text();
                console.log('Delete response text:', responseText);
                if (!responseText || responseText.trim() === '') {
                    throw new Error('Empty response from server');
                }
                result = JSON.parse(responseText);
            } catch (parseError) {
                console.error('Error parsing delete response:', parseError);
                throw new Error('Invalid response from server: ' + (parseError.message || 'Parse error'));
            }
            
            // Check result
            if (!result || !result.success) {
                const errorMsg = (result && result.message) ? result.message : 'Failed to delete projection';
                showErrorModal(errorMsg);
                return;
            }
            
            // Success - deletion completed successfully
            console.log('✓ Delete successful on server');
            
            // Show success notification immediately
            try {
                if (typeof showNotification === 'function') {
                    showNotification('Projection deleted successfully', 'success');
                }
            } catch (notifError) {
                console.error('Error showing notification:', notifError);
            }
            
            // Update local array
            allProjections = allProjections.filter(p => {
                const pId = typeof p.id === 'string' ? parseInt(p.id, 10) : p.id;
                return pId !== numId;
            });
            
            // Update DataTable by removing the row directly - COMPLETELY REDESIGNED FOR CRASH PREVENTION
            try {
                if (dataTable && typeof dataTable.row === 'function' && typeof dataTable.rows === 'function') {
                    // Use DataTable API to find and remove the row
                    let rowRemoved = false;
                    
                    // Method 1: Try using rows().every() to find and remove
                    try {
                        dataTable.rows().every(function(rowIdx, tableLoop, rowLoop) {
                            const rowData = this.data();
                            if (rowData && rowData.id !== undefined) {
                                const rowId = typeof rowData.id === 'string' ? parseInt(rowData.id, 10) : rowData.id;
                                if (rowId === numId) {
                                    // Found the row - remove it
                                    this.remove();
                                    rowRemoved = true;
                                    return false; // Stop iteration
                                }
                            }
                            return true; // Continue iteration
                        });
                    } catch (everyError) {
                        console.warn('Error using rows().every():', everyError);
                        // Try alternative method
                    }
                    
                    // Method 2: If Method 1 didn't work, try searching all rows
                    if (!rowRemoved) {
                        try {
                            const allRows = dataTable.rows().nodes();
                            for (let i = 0; i < allRows.length; i++) {
                                const rowNode = allRows[i];
                                const rowData = dataTable.row(rowNode).data();
                                if (rowData && rowData.id !== undefined) {
                                    const rowId = typeof rowData.id === 'string' ? parseInt(rowData.id, 10) : rowData.id;
                                    if (rowId === numId) {
                                        // Found the row - remove it
                                        dataTable.row(rowNode).remove();
                                        rowRemoved = true;
                                        break;
                                    }
                                }
                            }
                        } catch (searchError) {
                            console.warn('Error searching rows:', searchError);
                        }
                    }
                    
                    if (rowRemoved) {
                        // Redraw table without resetting pagination
                        dataTable.draw(false);
                        console.log('✓ Row removed from DataTable successfully');
                        
                        // Reattach action listeners after redraw
                        setTimeout(() => {
                            try {
                                attachActionListeners();
                            } catch (attachError) {
                                console.warn('Error reattaching listeners:', attachError);
                            }
                        }, 100);
                    } else {
                        // Row not found in DataTable - this is OK, just log it
                        console.log('Row not found in DataTable (may have been removed already)');
                        // Don't reload - deletion succeeded on server, that's what matters
                    }
                } else {
                    console.log('DataTable not available or invalid - deletion succeeded on server');
                    // Don't reload - deletion succeeded on server
                }
            } catch (tableError) {
                console.error('Error updating DataTable:', tableError);
                // Don't throw - deletion succeeded on server, that's what matters
                // Just log the error and continue
            }
            
            // Update 3D visualization
            try {
                if (typeof update3DVisualization === 'function') {
                    update3DVisualization(allProjections.length);
                }
            } catch (vizError) {
                console.error('Error updating 3D visualization:', vizError);
            }
            
            // Return success
            return;
            
        } catch (error) {
            console.error('❌ Error deleting projection:', error);
            console.error('Error details:', {
                name: error.name,
                message: error.message,
                stack: error.stack
            });
            
            // Show user-friendly error message
            let errorMsg = 'Failed to delete projection. ';
            if (error.message) {
                if (error.message.includes('Failed to fetch') || error.message.includes('NetworkError')) {
                    errorMsg += 'Server connection failed. Please ensure the web server is running at http://localhost:8080';
                } else if (error.message.includes('HTTP')) {
                    errorMsg += error.message;
                } else if (error.message.includes('Load failed')) {
                    // Don't show "Load failed" as part of delete error - deletion might have succeeded
                    errorMsg = 'Delete operation completed, but there was an issue refreshing the page. Please refresh manually.';
                } else {
                    errorMsg += error.message;
                }
            } else {
                errorMsg += 'Unknown error occurred';
            }
            
            try {
                showErrorModal(errorMsg);
            } catch (modalError) {
                console.error('Error showing error modal:', modalError);
                alert(errorMsg); // Fallback to alert if modal fails
            }
            
            // Re-throw to be caught by handleDeleteAction
            throw error;
        }
    }
    
    /**
     * Export all projections
     */
    function exportAll() {
        if (allProjections.length === 0) {
            showErrorModal('No projections to export');
            return;
        }
        
        try {
            const exportData = {
                export_date: new Date().toISOString(),
                total_count: allProjections.length,
                projections: allProjections.map(proj => ({
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
            
            showNotification('All projections exported successfully', 'success');
        } catch (error) {
            console.error('Error exporting all projections:', error);
            showErrorModal('Failed to export projections: ' + (error.message || 'Unknown error'));
        }
    }
    
    /**
     * Initialize file upload functionality
     */
    function initFileUpload() {
        const fileInput = document.getElementById('projection-file-input');
        const fileWrapper = document.getElementById('file-upload-wrapper');
        const filePreview = document.getElementById('file-upload-preview');
        const fileName = document.getElementById('file-name');
        const fileSize = document.getElementById('file-size');
        const fileRemove = document.getElementById('file-upload-remove');
        const fileCancel = document.getElementById('file-upload-cancel');
        const fileSubmit = document.getElementById('file-upload-submit');
        const fileActions = document.getElementById('file-upload-actions');
        const fileStatus = document.getElementById('file-upload-status');
        
        if (!fileInput) return;
        
        // Handle file selection
        fileInput.addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (file) {
                handleFileSelection(file);
            }
        });
        
        // Handle drag and drop
        if (fileWrapper) {
            fileWrapper.addEventListener('dragover', function(e) {
                e.preventDefault();
                e.stopPropagation();
                fileWrapper.classList.add('dragover');
            });
            
            fileWrapper.addEventListener('dragleave', function(e) {
                e.preventDefault();
                e.stopPropagation();
                fileWrapper.classList.remove('dragover');
            });
            
            fileWrapper.addEventListener('drop', function(e) {
                e.preventDefault();
                e.stopPropagation();
                fileWrapper.classList.remove('dragover');
                
                const file = e.dataTransfer.files[0];
                if (file) {
                    handleFileSelection(file);
                    fileInput.files = e.dataTransfer.files;
                }
            });
        }
        
        // Handle file removal
        if (fileRemove) {
            fileRemove.addEventListener('click', function() {
                selectedFile = null;
                if (fileInput) fileInput.value = '';
                if (filePreview) filePreview.style.display = 'none';
                if (fileActions) fileActions.style.display = 'none';
                if (fileStatus) {
                    fileStatus.style.display = 'none';
                    fileStatus.className = 'file-upload-status';
                }
            });
        }
        
        // Handle cancel
        if (fileCancel) {
            fileCancel.addEventListener('click', function() {
                selectedFile = null;
                if (fileInput) fileInput.value = '';
                if (filePreview) filePreview.style.display = 'none';
                if (fileActions) fileActions.style.display = 'none';
                if (fileStatus) {
                    fileStatus.style.display = 'none';
                    fileStatus.className = 'file-upload-status';
                }
            });
        }
        
        // Handle submit
        if (fileSubmit) {
            fileSubmit.addEventListener('click', async function() {
                if (selectedFile) {
                    await uploadFile(selectedFile);
                }
            });
        }
        
        function handleFileSelection(file) {
            // Validate file type (JSON or PNG)
            const isJson = file.type === 'application/json' || file.name.toLowerCase().endsWith('.json');
            const isPng = file.type === 'image/png' || file.name.toLowerCase().endsWith('.png');
            
            if (!isJson && !isPng) {
                showFileUploadError('Invalid file type. Only JSON and PNG files are allowed.');
                return;
            }
            
            // Validate file size (10MB max)
            const maxSize = 10 * 1024 * 1024; // 10MB
            if (file.size > maxSize) {
                showFileUploadError('File size exceeds maximum allowed size of 10MB.');
                return;
            }
            
            selectedFile = file;
            
            // Show preview
            if (fileName) fileName.textContent = file.name;
            if (fileSize) fileSize.textContent = formatFileSize(file.size);
            
            // Update icon based on file type
            const fileIcon = filePreview?.querySelector('.file-icon');
            if (fileIcon) {
                if (isPng) {
                    fileIcon.className = 'fas fa-file-image file-icon';
                } else {
                    fileIcon.className = 'fas fa-file-code file-icon';
                }
            }
            
            if (filePreview) filePreview.style.display = 'block';
            if (fileActions) fileActions.style.display = 'flex';
            if (fileStatus) fileStatus.style.display = 'none';
        }
        
    }
    
    /**
     * Format file size for display
     */
    function formatFileSize(bytes) {
        if (!bytes || bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
    }
    
    /**
     * Upload file to server
     */
    async function uploadFile(file) {
        const fileSubmit = document.getElementById('file-upload-submit');
        const fileStatus = document.getElementById('file-upload-status');
        const fileActions = document.getElementById('file-upload-actions');
        
        if (!file || !fileSubmit || !fileStatus) return;
        
        try {
            // Show uploading state
            fileSubmit.disabled = true;
            fileSubmit.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Uploading...';
            fileStatus.style.display = 'block';
            fileStatus.className = 'file-upload-status file-upload-status-info';
            fileStatus.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Uploading file...';
            
            const formData = new FormData();
            formData.append('projection_file', file);
            
            const response = await fetch('api/upload_projection.php', {
                method: 'POST',
                body: formData
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.success) {
                // Show success
                fileStatus.className = 'file-upload-status file-upload-status-success';
                fileStatus.innerHTML = `<i class="fas fa-check-circle"></i> ${result.message}`;
                
                // Reset form
                selectedFile = null;
                const fileInput = document.getElementById('projection-file-input');
                const filePreview = document.getElementById('file-upload-preview');
                if (fileInput) fileInput.value = '';
                if (filePreview) filePreview.style.display = 'none';
                if (fileActions) fileActions.style.display = 'none';
                
                // Reload projections immediately
                console.log('Upload successful, reloading projections...');
                try {
                    await loadProjections();
                    console.log('Projections reloaded after upload');
                    
                    // Show success message with count
                    const count = allProjections.length;
                    showNotification(`Projection uploaded successfully! Total: ${count} projection${count !== 1 ? 's' : ''}`, 'success');
                } catch (reloadError) {
                    console.error('Error reloading after upload:', reloadError);
                    // Still show success for upload, but warn about reload
                    showNotification('Projection uploaded, but failed to refresh table. Please refresh the page.', 'warning');
                }
                
                // Hide status after delay
                setTimeout(() => {
                    if (fileStatus) fileStatus.style.display = 'none';
                }, 3000);
            } else {
                throw new Error(result.message || 'Upload failed');
            }
        } catch (error) {
            console.error('Error uploading file:', error);
            showFileUploadError(error.message || 'Failed to upload file');
        } finally {
            if (fileSubmit) {
                fileSubmit.disabled = false;
                fileSubmit.innerHTML = '<i class="fas fa-upload"></i> Upload & Save';
            }
        }
    }
    
    /**
     * Show file upload error
     */
    function showFileUploadError(message) {
        const fileStatus = document.getElementById('file-upload-status');
        if (fileStatus) {
            fileStatus.style.display = 'block';
            fileStatus.className = 'file-upload-status file-upload-status-error';
            fileStatus.innerHTML = `<i class="fas fa-exclamation-circle"></i> ${message}`;
        }
    }
    
    /**
     * Show error modal
     */
    function showErrorModal(message) {
        // Remove existing error modal if present
        const existingModal = document.getElementById('data-error-modal');
        if (existingModal) {
            existingModal.remove();
        }
        
        // Create error modal
        const modal = document.createElement('div');
        modal.id = 'data-error-modal';
        modal.className = 'data-error-modal';
        modal.innerHTML = `
            <div class="data-error-modal-overlay"></div>
            <div class="data-error-modal-content">
                <div class="data-error-modal-header">
                    <i class="fas fa-exclamation-triangle"></i>
                    <h3>Error</h3>
                </div>
                <div class="data-error-modal-body">
                    <p>${message}</p>
                </div>
                <div class="data-error-modal-footer">
                    <button class="btn btn-primary" id="close-error-modal-btn">Close</button>
                </div>
            </div>
        `;
        
        document.body.appendChild(modal);
        
        // Show modal with animation
        setTimeout(() => {
            modal.classList.add('active');
        }, 10);
        
        // Close button handler
        const closeBtn = document.getElementById('close-error-modal-btn');
        const overlay = modal.querySelector('.data-error-modal-overlay');
        
        const closeModal = () => {
            modal.classList.remove('active');
            setTimeout(() => {
                modal.remove();
            }, 300);
        };
        
        if (closeBtn) {
            closeBtn.addEventListener('click', closeModal);
        }
        
        if (overlay) {
            overlay.addEventListener('click', closeModal);
        }
        
        // Close on Escape key
        const escapeHandler = (e) => {
            if (e.key === 'Escape' && modal.classList.contains('active')) {
                closeModal();
                document.removeEventListener('keydown', escapeHandler);
            }
        };
        document.addEventListener('keydown', escapeHandler);
    }
    
    /**
     * Show notification (toast-style)
     */
    function showNotification(message, type = 'info') {
        const notification = document.createElement('div');
        notification.className = `data-notification data-notification-${type}`;
        notification.innerHTML = `
            <i class="fas fa-${type === 'success' ? 'check-circle' : type === 'error' ? 'exclamation-circle' : 'info-circle'}"></i>
            <span>${message}</span>
        `;
        document.body.appendChild(notification);
        
        // Show notification
        setTimeout(() => {
            notification.classList.add('active');
        }, 10);
        
        // Hide and remove after 3 seconds
        setTimeout(() => {
            notification.classList.remove('active');
            setTimeout(() => {
                notification.remove();
            }, 300);
        }, 3000);
    }
    
    // 3D Visualization (keep existing code)
    let dataScene, dataCamera, dataRenderer;
    let dataAnimationId = null;
    let dataSolid = null;
    let dataFillSolid = null;
    let dataParticles = null;
    let currentDataCount = 0;
    let targetFillLevel = 0;
    let isDragging = false;
    let previousMousePosition = { x: 0, y: 0 };
    let currentModelType = 'dodecahedron';
    
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
        
        dataScene = new THREE.Scene();
        dataScene.background = new THREE.Color(0x0a0f1a);
        
        dataCamera = new THREE.PerspectiveCamera(50, width / height, 0.1, 1000);
        dataCamera.position.set(0, 0, 8);
        dataCamera.lookAt(0, 0, 0);
        
        dataRenderer = new THREE.WebGLRenderer({ 
            canvas: canvas, 
            antialias: true,
            alpha: false
        });
        dataRenderer.setSize(width, height);
        dataRenderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
        dataScene.add(ambientLight);
        
        const directionalLight = new THREE.DirectionalLight(0x3b82f6, 0.8);
        directionalLight.position.set(5, 10, 5);
        dataScene.add(directionalLight);
        
        const pointLight = new THREE.PointLight(0x3b82f6, 0.5, 100);
        pointLight.position.set(-5, -5, 5);
        dataScene.add(pointLight);
        
        createPlatonicSolid(currentModelType);
        setupInteractiveControls(canvas);
        setupModelSelector();
        
        if (loading) loading.style.display = 'none';
        animate3D();
        
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
    
    function setupModelSelector() {
        const modelSelect = document.getElementById('data-viz-model-select');
        if (!modelSelect) return;
        
        modelSelect.value = currentModelType;
        modelSelect.addEventListener('change', function() {
            currentModelType = this.value;
            if (dataScene) {
                const savedFillLevel = targetFillLevel;
                createPlatonicSolid(currentModelType);
                targetFillLevel = savedFillLevel;
            }
        });
    }
    
    function setupInteractiveControls(canvas) {
        if (!canvas) return;
        
        canvas.addEventListener('mousedown', (e) => {
            isDragging = true;
            previousMousePosition = { x: e.clientX, y: e.clientY };
            canvas.style.cursor = 'grabbing';
        });
        
        canvas.addEventListener('mousemove', (e) => {
            if (!isDragging) return;
            
            const deltaX = e.clientX - previousMousePosition.x;
            const deltaY = e.clientY - previousMousePosition.y;
            
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
    
    function createPlatonicSolid(modelType = 'dodecahedron') {
        if (dataSolid) dataScene.remove(dataSolid);
        if (dataFillSolid) dataScene.remove(dataFillSolid);
        if (dataParticles) dataScene.remove(dataParticles);
        
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
        
        const wireframeMaterial = new THREE.MeshBasicMaterial({
            color: 0x3b82f6,
            wireframe: true,
            transparent: true,
            opacity: 0.3
        });
        dataSolid = new THREE.Mesh(geometry, wireframeMaterial);
        dataScene.add(dataSolid);
        
        const fillMaterial = new THREE.MeshPhongMaterial({
            color: 0x3b82f6,
            transparent: true,
            opacity: 0.8,
            emissive: 0x1e3a8a,
            emissiveIntensity: 0.3
        });
        dataFillSolid = new THREE.Mesh(fillGeometry, fillMaterial);
        dataScene.add(dataFillSolid);
        
        const particleCount = 100;
        const particlesGeometry = new THREE.BufferGeometry();
        const positions = new Float32Array(particleCount * 3);
        const colors = new Float32Array(particleCount * 3);
        
        for (let i = 0; i < particleCount; i++) {
            const i3 = i * 3;
            const radius = Math.random() * 1.5;
            const theta = Math.random() * Math.PI * 2;
            const phi = Math.acos(Math.random() * 2 - 1);
            
            positions[i3] = radius * Math.sin(phi) * Math.cos(theta);
            positions[i3 + 1] = radius * Math.sin(phi) * Math.sin(theta);
            positions[i3 + 2] = radius * Math.cos(phi);
            
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
    
    function update3DVisualization(count) {
        currentDataCount = count;
        targetFillLevel = Math.min(count / 20, 1);
        
        const countEl = document.getElementById('data-viz-count');
        if (countEl) {
            countEl.textContent = count;
        }
    }
    
    function animate3D() {
        if (!dataRenderer || !dataScene || !dataCamera) return;
        
        dataAnimationId = requestAnimationFrame(animate3D);
        
        if (dataSolid && !isDragging) {
            dataSolid.rotation.y += 0.005;
            dataSolid.rotation.x += 0.003;
        }
        
        if (dataFillSolid) {
            const currentScale = dataFillSolid.scale.x;
            const targetScale = 0.1 + (targetFillLevel * 1.85);
            const newScale = currentScale + (targetScale - currentScale) * 0.08;
            
            dataFillSolid.scale.set(newScale, newScale, newScale);
            dataFillSolid.material.opacity = 0.5 + (targetFillLevel * 0.5);
            dataFillSolid.material.emissiveIntensity = 0.4 + (targetFillLevel * 0.6);
            
            const intensity = 0.3 + (targetFillLevel * 0.7);
            dataFillSolid.material.emissive.setRGB(0.12 * intensity, 0.23 * intensity, 0.5 * intensity);
        }
        
        if (dataFillSolid && !isDragging) {
            dataFillSolid.rotation.y -= 0.008;
            dataFillSolid.rotation.x += 0.005;
        }
        
        if (dataParticles) {
            dataParticles.rotation.y += 0.002;
            
            const positions = dataParticles.geometry.attributes.position.array;
            const visibleCount = Math.floor(targetFillLevel * positions.length / 3);
            
            for (let i = 0; i < positions.length / 3; i++) {
                const i3 = i * 3;
                if (i < visibleCount) {
                    const radius = 0.5 + Math.random() * 1.5 * targetFillLevel;
                    const theta = Math.random() * Math.PI * 2;
                    const phi = Math.acos(Math.random() * 2 - 1);
                    
                    positions[i3] = radius * Math.sin(phi) * Math.cos(theta + dataParticles.rotation.y);
                    positions[i3 + 1] = radius * Math.sin(phi) * Math.sin(theta + dataParticles.rotation.y);
                    positions[i3 + 2] = radius * Math.cos(phi);
                } else {
                    positions[i3] = 0;
                    positions[i3 + 1] = 0;
                    positions[i3 + 2] = 0;
                }
            }
            dataParticles.geometry.attributes.position.needsUpdate = true;
        }
        
        dataRenderer.render(dataScene, dataCamera);
    }
    
    /**
     * Initialize
     */
    function init() {
        // Initialize file upload
        initFileUpload();
        
        // Initialize refresh button
        const refreshBtn = document.getElementById('refresh-data-btn');
        if (refreshBtn) {
            refreshBtn.addEventListener('click', loadProjections);
        }
        
        // Initialize export all button
        const exportAllBtn = document.getElementById('export-all-btn');
        if (exportAllBtn) {
            exportAllBtn.addEventListener('click', exportAll);
        }
        
        // Watch for page activation
        watchPageActivation();
        
        // Load if page is already active
        if (document.getElementById('page-data')?.classList.contains('active')) {
            loadProjections();
            setTimeout(() => {
                if (typeof THREE !== 'undefined' && !dataScene) {
                    init3DVisualization();
                }
            }, 300);
        }
    }
    
    function watchPageActivation() {
        const pageEl = document.getElementById('page-data');
        if (!pageEl) return;
        
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (pageEl.classList.contains('active')) {
                        loadProjections();
                        setTimeout(() => {
                            if (typeof THREE !== 'undefined' && !dataScene) {
                                init3DVisualization();
                            }
                        }, 300);
                    }
                }
            });
        });
        
        observer.observe(pageEl, {
            attributes: true,
            attributeFilter: ['class']
        });
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
        refresh: loadProjections,
        init3DVisualization: init3DVisualization,
        loadProjectionData: function(projection) {
            // This function can be called from external modules
            if (typeof window.loadProjection === 'function') {
                window.loadProjection(projection.id);
            }
        }
    };
})();

// Expose for external use
window.DataPageModule = DataPageModule;
