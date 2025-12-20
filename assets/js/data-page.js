/**
 * Data Page Module - Enhanced with DataTables and File Upload
 * Following Preline DataTables design patterns
 */
const DataPageModule = (function() {
    let dataTable = null;
    let allProjections = [];
    let selectedFile = null;
    
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
                                const projData = typeof data === 'string' ? JSON.parse(data) : data;
                                if (projData && projData.type === 'image') {
                                    return `<span class="text-gray-600 dark:text-neutral-400"><i class="fas fa-image"></i> PNG</span>`;
                                }
                                return `<span class="text-gray-600 dark:text-neutral-400">${projData?.interval || '--'}</span>`;
                            } catch (e) {
                                return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                            }
                        }
                    },
                    {
                        data: 'params',
                        render: function(data, type, row) {
                            try {
                                const params = typeof data === 'string' ? JSON.parse(data) : data;
                                return `<span class="text-gray-600 dark:text-neutral-400">${params?.steps || '--'}</span>`;
                            } catch (e) {
                                return '<span class="text-gray-600 dark:text-neutral-400">--</span>';
                            }
                        }
                    },
                    {
                        data: null,
                        orderable: false,
                        render: function(data, type, row) {
                            const rowId = row.id || '';
                            return `
                                <div class="datatable-actions flex justify-end gap-2">
                                    <button type="button" 
                                            class="btn-action btn-view" 
                                            data-id="${rowId}" 
                                            data-action="view"
                                            title="View Projection"
                                            aria-label="View Projection">
                                        <i class="fas fa-eye"></i>
                                    </button>
                                    <button type="button" 
                                            class="btn-action btn-export" 
                                            data-id="${rowId}" 
                                            data-action="export"
                                            title="Export Projection"
                                            aria-label="Export Projection">
                                        <i class="fas fa-download"></i>
                                    </button>
                                    <button type="button" 
                                            class="btn-action btn-delete delete-projection-btn" 
                                            data-id="${rowId}" 
                                            data-action="delete"
                                            data-symbol="${escapeHtml(row.symbol || 'projection')}"
                                            title="Delete Projection"
                                            aria-label="Delete Projection">
                                        <i class="fas fa-trash-alt"></i>
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
                drawCallback: function() {
                    attachActionListeners();
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
            const response = await fetch('api/projections.php');
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.success && result.projections) {
                allProjections = result.projections;
                
                // Update DataTable safely
                try {
                    if (dataTable) {
                        dataTable.clear();
                        if (allProjections.length > 0) {
                            dataTable.rows.add(allProjections);
                        }
                        dataTable.draw();
                    } else {
                        // Initialize DataTable if not already initialized
                        initDataTable();
                        if (dataTable) {
                            dataTable.clear();
                            if (allProjections.length > 0) {
                                dataTable.rows.add(allProjections);
                            }
                            dataTable.draw();
                        }
                    }
                } catch (tableError) {
                    console.error('Error updating DataTable:', tableError);
                    // Try to reinitialize
                    try {
                        initDataTable();
                    } catch (initError) {
                        console.error('Error reinitializing DataTable:', initError);
                    }
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
                allProjections = [];
                if (dataTable) {
                    dataTable.clear();
                    dataTable.draw();
                }
                update3DVisualization(0);
            }
        } catch (error) {
            console.error('Error loading projections:', error);
            showDataTableError(error);
        } finally {
            if (loadingEl) loadingEl.style.display = 'none';
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
        
        // Attach event delegation on the new table body
        newTableBody.addEventListener('click', function(e) {
            const btn = e.target.closest('.btn-action');
            if (!btn) return;
            
            e.stopPropagation();
            e.preventDefault();
            
            const action = btn.dataset.action;
            const idStr = btn.dataset.id;
            
            if (!idStr || idStr === 'undefined' || idStr === 'null') {
                console.error('Invalid ID for action:', action, idStr);
                return;
            }
            
            const id = parseInt(idStr, 10);
            if (isNaN(id) || id <= 0) {
                console.error('Invalid ID parsed:', idStr, id);
                showErrorModal('Invalid projection ID');
                return;
            }
            
            switch (action) {
                case 'view':
                    viewProjection(id);
                    break;
                case 'export':
                    exportProjection(id);
                    break;
                case 'delete':
                    const symbol = btn.dataset.symbol || 'this projection';
                    deleteProjectionWithConfirmation(id, symbol);
                    break;
                default:
                    console.warn('Unknown action:', action);
            }
        });
    }
    
    /**
     * Delete projection with confirmation dialog
     */
    function deleteProjectionWithConfirmation(id, symbol) {
        // Create a more user-friendly confirmation dialog
        if (!confirm(`Are you sure you want to delete ${symbol}? This action cannot be undone.`)) {
            return;
        }
        deleteProjection(id);
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
            
            // Success - update UI directly (don't reload from server to avoid "Load failed" errors)
            console.log('Delete successful, updating UI directly...');
            
            // Remove from local array (numId already defined above)
            allProjections = allProjections.filter(p => {
                const pId = typeof p.id === 'string' ? parseInt(p.id, 10) : p.id;
                return pId !== numId;
            });
            
            // Update DataTable directly
            try {
                if (dataTable) {
                    // Clear and reload with updated data
                    dataTable.clear();
                    if (allProjections.length > 0) {
                        dataTable.rows.add(allProjections);
                    }
                    dataTable.draw(false); // Don't reset pagination
                }
            } catch (tableError) {
                console.error('Error updating DataTable:', tableError);
                // If table update fails, try a simple page reload approach
                // Don't call loadProjections() as it might cause "Load failed" error
                showErrorModal('Projection deleted successfully. Refreshing table...');
                // Use setTimeout to avoid blocking
                setTimeout(() => {
                    try {
                        if (dataTable) {
                            dataTable.clear();
                            if (allProjections.length > 0) {
                                dataTable.rows.add(allProjections);
                            }
                            dataTable.draw();
                        }
                    } catch (retryError) {
                        console.error('Retry update failed:', retryError);
                    }
                }, 100);
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
            
            // Show success notification
            try {
                if (typeof showNotification === 'function') {
                    showNotification('Projection deleted successfully', 'success');
                }
            } catch (notifError) {
                console.error('Error showing notification:', notifError);
                // Non-critical, continue
            }
            
        } catch (error) {
            console.error('Error deleting projection:', error);
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
                
                // Reload projections
                setTimeout(async () => {
                    await loadProjections();
                    fileStatus.style.display = 'none';
                }, 2000);
                
                showNotification('Projection uploaded successfully', 'success');
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
