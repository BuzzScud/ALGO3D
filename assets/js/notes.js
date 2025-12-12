// Notes Management
let currentNoteId = null;
let selectedColor = '#1e293b';
let currentSort = 'recent';
let currentFilter = 'all';
let currentType = 'all';
let currentView = 'grid';
let allNotes = [];
let searchQuery = '';

// Fetch notes from API
async function fetchNotes(search = '') {
    const notesGrid = document.getElementById('notes-grid');
    if (!notesGrid) return;
    
    try {
        const url = search ? `api/notes.php?search=${encodeURIComponent(search)}` : 'api/notes.php';
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const result = await response.json();
        
        if (result.success === false) {
            throw new Error(result.message || 'Failed to fetch notes');
        }
        
        const notes = Array.isArray(result) ? result : (result.notes || []);
        allNotes = notes;
        updateNotesMetrics(notes);
        displayNotes(notes);
        
        // Ensure 3D visualization is rendered after notes are loaded
        const notesPage = document.getElementById('page-notes');
        if (notesPage && notesPage.classList.contains('active')) {
            setTimeout(() => {
                const trades = notes.filter(n => n.notes_type === 'trade' || (n.contract_symbol && n.entry_price));
                console.log('Rendering 3D visualization with', trades.length, 'trades');
                renderTrading3DVisualization(trades);
            }, 500);
        }
    } catch (error) {
        console.error('Error fetching notes:', error);
        if (notesGrid) {
            notesGrid.innerHTML = `
                <div class="loading">
                    <i class="fas fa-exclamation-triangle"></i>
                    <p>Error loading notes: ${error.message}</p>
                    <button class="btn btn-secondary btn-sm" onclick="fetchNotes('${search}')" style="margin-top: 10px;">
                        <i class="fas fa-redo"></i> Retry
                    </button>
                </div>
            `;
        }
    }
}

// Display notes
function displayNotes(notes) {
    const notesGrid = document.getElementById('notes-grid');
    if (!notesGrid) return;

    // Apply filters and sorting
    let filtered = filterAndSortNotes(notes);

    if (!filtered || filtered.length === 0) {
        notesGrid.innerHTML = '<div class="empty-state"><i class="fas fa-sticky-note"></i><p>No notes found. Create your first note!</p></div>';
        return;
    }

    // Apply view mode
    if (currentView === 'list') {
        notesGrid.classList.add('list-view');
    } else {
        notesGrid.classList.remove('list-view');
    }

    notesGrid.innerHTML = filtered.map(note => createNoteCard(note)).join('');
    
    // Attach event listeners
    attachNoteListeners();
}

// Filter and sort notes
function filterAndSortNotes(notes) {
    let filtered = [...notes];

    // Apply search filter first
    if (searchQuery) {
        const query = searchQuery.toLowerCase();
        filtered = filtered.filter(note => 
            (note.title || '').toLowerCase().includes(query) ||
            (note.content || '').toLowerCase().includes(query) ||
            (note.contract_symbol || '').toLowerCase().includes(query) ||
            (note.strategy || '').toLowerCase().includes(query)
        );
    }

    // Apply time filter
    if (currentFilter !== 'all') {
        const now = new Date();
        const today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
        const weekAgo = new Date(today);
        weekAgo.setDate(weekAgo.getDate() - 7);
        const monthAgo = new Date(today);
        monthAgo.setMonth(monthAgo.getMonth() - 1);

        filtered = filtered.filter(note => {
            const noteDate = new Date(note.updated_at || note.created_at);
            if (currentFilter === 'today') {
                return noteDate >= today;
            } else if (currentFilter === 'week') {
                return noteDate >= weekAgo;
            } else if (currentFilter === 'month') {
                return noteDate >= monthAgo;
            }
            return true;
        });
    }

    // Apply sort
    filtered.sort((a, b) => {
        if (currentSort === 'recent') {
            return new Date(b.updated_at || b.created_at) - new Date(a.updated_at || a.created_at);
        } else if (currentSort === 'oldest') {
            return new Date(a.created_at) - new Date(b.created_at);
        } else if (currentSort === 'title') {
            return (a.title || '').localeCompare(b.title || '');
        } else if (currentSort === 'updated') {
            return new Date(b.updated_at || 0) - new Date(a.updated_at || 0);
        } else if (currentSort === 'pnl') {
            const pnlA = a.pnl || (a.entry_price && a.exit_price ? 
                calculatePnL(a.entry_price, a.exit_price, a.quantity || 1, a.direction) : -Infinity);
            const pnlB = b.pnl || (b.entry_price && b.exit_price ? 
                calculatePnL(b.entry_price, b.exit_price, b.quantity || 1, b.direction) : -Infinity);
            return pnlB - pnlA;
        } else if (currentSort === 'contract') {
            return (a.contract_symbol || '').localeCompare(b.contract_symbol || '');
        } else if (currentSort === 'date') {
            return new Date(b.entry_time || b.created_at) - new Date(a.entry_time || a.created_at);
        }
        return 0;
    });

    return filtered;
}

// Update notes metrics
function updateNotesMetrics(notes) {
    const total = notes.length;
    const now = new Date();
    const weekAgo = new Date(now);
    weekAgo.setDate(weekAgo.getDate() - 7);
    const monthAgo = new Date(now);
    monthAgo.setMonth(monthAgo.getMonth() - 1);
    const today = new Date(now.getFullYear(), now.getMonth(), now.getDate());

    // Trading-specific calculations
    const trades = notes.filter(n => n.notes_type === 'trade' || (n.contract_symbol && n.entry_price));
    const totalTrades = trades.length;
    
    // Calculate P&L
    let totalPnL = 0;
    let wins = 0;
    let losses = 0;
    let bestTrade = 0;
    let todayPnL = 0;
    let todayTrades = 0;
    
    trades.forEach(trade => {
        const pnl = trade.pnl || (trade.entry_price && trade.exit_price ? 
            calculatePnL(trade.entry_price, trade.exit_price, trade.quantity || 1, trade.direction) : null);
        
        if (pnl !== null) {
            totalPnL += pnl;
            if (pnl > 0) wins++;
            else if (pnl < 0) losses++;
            if (pnl > bestTrade) bestTrade = pnl;
            
            const tradeDate = new Date(trade.entry_time || trade.created_at);
            if (tradeDate >= today) {
                todayPnL += pnl;
                todayTrades++;
            }
        }
    });
    
    const winRate = totalTrades > 0 ? ((wins / totalTrades) * 100).toFixed(1) : 0;
    const avgPnL = totalTrades > 0 ? (totalPnL / totalTrades) : 0;
    
    // Update trading metrics
    const totalPnLEl = document.getElementById('trading-total-pnl');
    if (totalPnLEl) {
        totalPnLEl.textContent = formatCurrency(totalPnL);
        totalPnLEl.className = 'stat-large ' + (totalPnL >= 0 ? 'positive' : 'negative');
    }
    
    updateElement('trading-today-pnl', formatCurrency(todayPnL));
    updateElement('trading-win-rate', winRate + '%');
    updateElement('trading-wins', wins);
    updateElement('trading-losses', losses);
    updateElement('trading-total-trades', totalTrades);
    updateElement('trading-today-trades', todayTrades);
    updateElement('trading-avg-pnl', formatCurrency(avgPnL));
    updateElement('trading-best-trade', formatCurrency(bestTrade));
    
    // Update 3D visualization stats
    updateElement('viz-total-trades', totalTrades);
    updateElement('viz-win-rate', winRate + '%');
    updateElement('viz-total-pnl', formatCurrency(totalPnL));
    
    // Update analytics
    const weekTrades = trades.filter(n => new Date(n.entry_time || n.created_at) >= weekAgo).length;
    updateElement('analytics-week-trades', weekTrades);
    updateElement('analytics-avg-pnl', formatCurrency(avgPnL));
    
    // Calculate win streak
    let streak = 0;
    const sortedTrades = [...trades].sort((a, b) => 
        new Date(b.entry_time || b.created_at) - new Date(a.entry_time || a.created_at)
    );
    for (const trade of sortedTrades) {
        const pnl = trade.pnl || (trade.entry_price && trade.exit_price ? 
            calculatePnL(trade.entry_price, trade.exit_price, trade.quantity || 1, trade.direction) : null);
        if (pnl > 0) streak++;
        else if (pnl < 0) break;
    }
    updateElement('analytics-streak', streak);
    
    // Contracts analysis
    const contracts = {};
    trades.forEach(trade => {
        const symbol = trade.contract_symbol;
        if (symbol) {
            if (!contracts[symbol]) {
                contracts[symbol] = { count: 0, pnl: 0 };
            }
            contracts[symbol].count++;
            const pnl = trade.pnl || (trade.entry_price && trade.exit_price ? 
                calculatePnL(trade.entry_price, trade.exit_price, trade.quantity || 1, trade.direction) : 0);
            contracts[symbol].pnl += pnl || 0;
        }
    });
    
    updateContractsList(contracts);
    
    // Render 3D visualization (always call, even with empty array)
    console.log('Calling renderTrading3DVisualization from updateNotesMetrics with', trades.length, 'trades');
    renderTrading3DVisualization(trades.length > 0 ? trades : []);
}

// Format currency
function formatCurrency(value) {
    if (value === null || value === undefined) return '$0.00';
    const sign = value >= 0 ? '+' : '';
    return sign + '$' + Math.abs(value).toFixed(2);
}

// Update contracts list
function updateContractsList(contracts) {
    const contractsList = document.getElementById('contracts-list');
    if (!contractsList) return;
    
    const sorted = Object.entries(contracts).sort((a, b) => b[1].count - a[1].count);
    
    if (sorted.length === 0) {
        contractsList.innerHTML = '<div class="empty-state"><p>No contracts yet</p></div>';
        return;
    }
    
    contractsList.innerHTML = sorted.map(([symbol, data]) => {
        const pnlClass = data.pnl >= 0 ? 'positive' : 'negative';
        return `
            <div class="contract-item">
                <span class="contract-name">${escapeHtml(symbol)}</span>
                <div class="contract-stats">
                    <span>${data.count} trades</span>
                    <span class="contract-pnl ${pnlClass}">${formatCurrency(data.pnl)}</span>
                </div>
            </div>
        `;
    }).join('');
}

// Update categories list
function updateCategoriesList(categories) {
    const categoriesList = document.getElementById('categories-list');
    if (!categoriesList) return;

    const sorted = Object.entries(categories).sort((a, b) => b[1] - a[1]);
    
    if (sorted.length === 0) {
        categoriesList.innerHTML = '<div class="empty-state"><p>No categories yet</p></div>';
        return;
    }

    categoriesList.innerHTML = sorted.map(([name, count]) => `
        <div class="category-item">
            <span class="category-name">${escapeHtml(name)}</span>
            <span class="category-count">${count}</span>
        </div>
    `).join('');
}

// Update element helper
function updateElement(id, value) {
    const el = document.getElementById(id);
    if (el) el.textContent = value;
}

// Create note card HTML
function createNoteCard(note) {
    const preview = note.content ? note.content.substring(0, 150) + (note.content.length > 150 ? '...' : '') : 'No content';
    const date = new Date(note.updated_at || note.created_at).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' });
    
    // Trading information
    const isTrade = note.notes_type === 'trade' || (note.contract_symbol && note.entry_price);
    const pnl = note.pnl || (note.entry_price && note.exit_price ? 
        calculatePnL(note.entry_price, note.exit_price, note.quantity || 1, note.direction) : null);
    const pnlPercent = note.pnl_percent || (note.entry_price && note.exit_price ? 
        calculatePnLPercent(note.entry_price, note.exit_price, note.direction) : null);
    const pnlClass = pnl ? (pnl >= 0 ? 'positive' : 'negative') : '';
    const cardClass = pnl ? (pnl >= 0 ? 'profitable' : 'loss') : '';
    
    return `
        <div class="note-card trading-note-card ${cardClass}" data-id="${note.id}" style="background-color: ${note.color || '#1e293b'}">
            <div class="note-card-header">
                <div class="note-title-section">
                    <h3 class="note-title">${escapeHtml(note.title)}</h3>
                    ${note.contract_symbol ? `<span class="contract-badge">${escapeHtml(note.contract_symbol)}</span>` : ''}
                </div>
                <div class="note-actions">
                    <button class="note-edit-btn" data-id="${note.id}" title="Edit">
                        <i class="fas fa-edit"></i>
                    </button>
                    <button class="note-delete-btn" data-id="${note.id}" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </div>
            </div>
            ${isTrade ? `
            <div class="note-trading-info">
                ${note.direction ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">Direction:</span>
                    <span class="direction-badge ${note.direction}">${note.direction.toUpperCase()}</span>
                </div>
                ` : ''}
                ${note.entry_price ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">Entry:</span>
                    <span class="trading-info-value">$${parseFloat(note.entry_price).toFixed(2)}</span>
                </div>
                ` : ''}
                ${note.exit_price ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">Exit:</span>
                    <span class="trading-info-value">$${parseFloat(note.exit_price).toFixed(2)}</span>
                </div>
                ` : ''}
                ${pnl !== null ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">P&L:</span>
                    <span class="trading-info-value ${pnlClass}">$${pnl >= 0 ? '+' : ''}${parseFloat(pnl).toFixed(2)}</span>
                </div>
                ` : ''}
                ${pnlPercent !== null ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">P&L %:</span>
                    <span class="trading-info-value ${pnlClass}">${pnlPercent >= 0 ? '+' : ''}${parseFloat(pnlPercent).toFixed(2)}%</span>
                </div>
                ` : ''}
                ${note.strategy ? `
                <div class="trading-info-row">
                    <span class="trading-info-label">Strategy:</span>
                    <span class="trading-info-value">${escapeHtml(note.strategy)}</span>
                </div>
                ` : ''}
            </div>
            ` : ''}
            <div class="note-content">${escapeHtml(preview)}</div>
            <div class="note-date">${date}</div>
        </div>
    `;
}

// Calculate P&L
function calculatePnL(entryPrice, exitPrice, quantity, direction) {
    if (!entryPrice || !exitPrice) return null;
    const priceDiff = direction === 'long' ? (exitPrice - entryPrice) : (entryPrice - exitPrice);
    return priceDiff * quantity;
}

// Calculate P&L Percentage
function calculatePnLPercent(entryPrice, exitPrice, direction) {
    if (!entryPrice || !exitPrice || entryPrice === 0) return null;
    const priceDiff = direction === 'long' ? (exitPrice - entryPrice) : (entryPrice - exitPrice);
    return (priceDiff / entryPrice) * 100;
}

// Escape HTML
function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Initialize notes page
function initNotesPage() {
    // View toggle buttons
    document.querySelectorAll('.view-btn[data-view]').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.view-btn').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            currentView = this.dataset.view;
            displayNotes(allNotes);
        });
    });

    // Sort buttons
    document.querySelectorAll('.filter-btn[data-sort]').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.filter-btn[data-sort]').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            currentSort = this.dataset.sort;
            displayNotes(allNotes);
        });
    });

    // Type filter buttons
    document.querySelectorAll('.filter-btn[data-type]').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.filter-btn[data-type]').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            currentType = this.dataset.type;
            displayNotes(allNotes);
        });
    });
    
    // Filter buttons
    document.querySelectorAll('.filter-btn[data-filter]').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.filter-btn[data-filter]').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            currentFilter = this.dataset.filter;
            displayNotes(allNotes);
        });
    });

    // Search input
    const searchInput = document.getElementById('notes-search-input');
    const searchClear = document.getElementById('notes-search-clear');
    
    if (searchInput) {
        let searchTimeout;
        searchInput.addEventListener('input', function() {
            clearTimeout(searchTimeout);
            searchTimeout = setTimeout(() => {
                searchQuery = this.value.trim().toLowerCase();
                if (searchClear) {
                    searchClear.style.display = searchQuery ? 'block' : 'none';
                }
                displayNotes(allNotes);
            }, 300);
        });
    }

    if (searchClear) {
        searchClear.addEventListener('click', function() {
            if (searchInput) {
                searchInput.value = '';
                searchQuery = '';
                this.style.display = 'none';
                displayNotes(allNotes);
            }
        });
    }

    // Quick actions
    const clearSearchBtn = document.getElementById('clear-search-btn');
    if (clearSearchBtn) {
        clearSearchBtn.addEventListener('click', function() {
            if (searchInput) {
                searchInput.value = '';
                searchQuery = '';
                if (searchClear) searchClear.style.display = 'none';
                displayNotes(allNotes);
            }
        });
    }

    const exportBtn = document.getElementById('export-notes-btn');
    if (exportBtn) {
        exportBtn.addEventListener('click', function() {
            exportNotes();
        });
    }
}

// Export notes
function exportNotes() {
    const dataStr = JSON.stringify(allNotes, null, 2);
    const dataBlob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(dataBlob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `notes-export-${new Date().toISOString().split('T')[0]}.json`;
    link.click();
    URL.revokeObjectURL(url);
}

// Attach event listeners
function attachNoteListeners() {
    // Edit buttons
    document.querySelectorAll('.note-edit-btn').forEach(btn => {
        btn.addEventListener('click', function(e) {
            e.stopPropagation();
            const noteId = this.dataset.id;
            editNote(noteId);
        });
    });

    // Delete buttons
    document.querySelectorAll('.note-delete-btn').forEach(btn => {
        btn.addEventListener('click', function(e) {
            e.stopPropagation();
            const noteId = this.dataset.id;
            deleteNote(noteId);
        });
    });

    // Click on card to edit
    document.querySelectorAll('.note-card').forEach(card => {
        card.addEventListener('click', function() {
            const noteId = this.dataset.id;
            editNote(noteId);
        });
    });
}

// Open note modal for new note
function openNewNoteModal() {
    console.log('openNewNoteModal called');
    currentNoteId = null;
    selectedColor = '#1e293b';
    
    const modal = document.getElementById('note-modal');
    const modalTitle = document.getElementById('note-modal-title');
    
    if (!modal) {
        console.error('Note modal not found');
        alert('Error: Note editor modal not found. Please refresh the page.');
        return;
    }
    
    console.log('Opening note modal');
    modal.classList.add('active');
    document.body.style.overflow = 'hidden'; // Prevent background scrolling
    
    if (modalTitle) modalTitle.innerHTML = '<i class="fas fa-chart-line"></i> New Trading Note';
    
    // Reset all form fields
    resetNoteForm();
    
    // Set default note type to trade
    const tradeTab = document.querySelector('.note-type-tab[data-type="trade"]');
    if (tradeTab) {
        document.querySelectorAll('.note-type-tab').forEach(tab => tab.classList.remove('active'));
        tradeTab.classList.add('active');
        toggleTradingDetailsSection('trade');
    }
    
    // Reset color selection
    document.querySelectorAll('.color-option').forEach(opt => {
        opt.classList.remove('active');
        if (opt.dataset.color === selectedColor) {
            opt.classList.add('active');
        }
    });
    
    const titleInput = document.getElementById('note-title-input');
    if (titleInput) {
        setTimeout(() => titleInput.focus(), 100);
    }
}

// Reset note form
function resetNoteForm() {
    document.getElementById('note-title-input').value = '';
    document.getElementById('note-content-input').value = '';
    document.getElementById('note-contract-symbol').value = '';
    document.getElementById('note-contract-type').value = 'futures';
    document.getElementById('note-entry-price').value = '';
    document.getElementById('note-exit-price').value = '';
    document.getElementById('note-quantity').value = '1';
    document.getElementById('note-direction').value = '';
    document.getElementById('note-entry-time').value = '';
    document.getElementById('note-exit-time').value = '';
    document.getElementById('note-stop-loss').value = '';
    document.getElementById('note-take-profit').value = '';
    document.getElementById('note-strategy').value = '';
    document.getElementById('note-timeframe').value = '';
    document.getElementById('note-category').value = '';
    document.getElementById('pnl-display').style.display = 'none';
}

// Edit existing note
async function editNote(noteId) {
    try {
        const response = await fetch('api/notes.php');
        if (!response.ok) throw new Error('Failed to fetch notes');
        
        const notes = await response.json();
        const note = notes.find(n => n.id == noteId);
        
        if (!note) {
            alert('Note not found');
            return;
        }
        
        currentNoteId = note.id;
        selectedColor = note.color || '#1e293b';
        
        const modal = document.getElementById('note-modal');
        const modalTitle = document.getElementById('note-modal-title');
        
        if (modal) {
            modal.classList.add('active');
            if (modalTitle) modalTitle.innerHTML = '<i class="fas fa-edit"></i> Edit Trading Note';
            
            // Populate form fields
            document.getElementById('note-title-input').value = note.title || '';
            document.getElementById('note-content-input').value = note.content || '';
            document.getElementById('note-contract-symbol').value = note.contract_symbol || '';
            document.getElementById('note-contract-type').value = note.contract_type || 'futures';
            document.getElementById('note-entry-price').value = note.entry_price || '';
            document.getElementById('note-exit-price').value = note.exit_price || '';
            document.getElementById('note-quantity').value = note.quantity || '1';
            document.getElementById('note-direction').value = note.direction || '';
            document.getElementById('note-entry-time').value = note.entry_time ? note.entry_time.substring(0, 16) : '';
            document.getElementById('note-exit-time').value = note.exit_time ? note.exit_time.substring(0, 16) : '';
            document.getElementById('note-stop-loss').value = note.stop_loss || '';
            document.getElementById('note-take-profit').value = note.take_profit || '';
            document.getElementById('note-strategy').value = note.strategy || '';
            document.getElementById('note-timeframe').value = note.timeframe || '';
            document.getElementById('note-category').value = note.category || '';
            
            // Set note type tab
            const noteType = note.notes_type || 'trade';
            document.querySelectorAll('.note-type-tab').forEach(tab => {
                tab.classList.remove('active');
                if (tab.dataset.type === noteType) {
                    tab.classList.add('active');
                }
            });
            toggleTradingDetailsSection(noteType);
            
            // Update P&L display if applicable
            updatePnLDisplay();
            
            // Set color selection
            document.querySelectorAll('.color-option').forEach(opt => {
                opt.classList.remove('active');
                if (opt.dataset.color === selectedColor) {
                    opt.classList.add('active');
                }
            });
            
            const titleInput = document.getElementById('note-title-input');
            if (titleInput) titleInput.focus();
        }
    } catch (error) {
        console.error('Error loading note:', error);
        alert('Error loading note');
    }
}

// Toggle trading details section based on note type
function toggleTradingDetailsSection(noteType) {
    const tradingSection = document.getElementById('trading-details-section');
    if (tradingSection) {
        if (noteType === 'trade') {
            tradingSection.classList.remove('hidden');
        } else {
            tradingSection.classList.add('hidden');
        }
    }
}

// Update P&L display
function updatePnLDisplay() {
    const entryPrice = parseFloat(document.getElementById('note-entry-price')?.value || 0);
    const exitPrice = parseFloat(document.getElementById('note-exit-price')?.value || 0);
    const quantity = parseInt(document.getElementById('note-quantity')?.value || 1);
    const direction = document.getElementById('note-direction')?.value;
    const pnlDisplay = document.getElementById('pnl-display');
    const calculatedPnL = document.getElementById('calculated-pnl');
    const calculatedPnLPercent = document.getElementById('calculated-pnl-percent');
    
    if (entryPrice && exitPrice && direction && pnlDisplay) {
        const pnl = calculatePnL(entryPrice, exitPrice, quantity, direction);
        const pnlPercent = calculatePnLPercent(entryPrice, exitPrice, direction);
        
        pnlDisplay.style.display = 'grid';
        if (calculatedPnL) {
            calculatedPnL.textContent = formatCurrency(pnl);
            calculatedPnL.className = 'pnl-value ' + (pnl >= 0 ? 'positive' : 'negative');
        }
        if (calculatedPnLPercent) {
            calculatedPnLPercent.textContent = (pnlPercent >= 0 ? '+' : '') + pnlPercent.toFixed(2) + '%';
            calculatedPnLPercent.className = 'pnl-value ' + (pnlPercent >= 0 ? 'positive' : 'negative');
        }
    } else if (pnlDisplay) {
        pnlDisplay.style.display = 'none';
    }
}

// Save note
async function saveNote() {
    const titleInput = document.getElementById('note-title-input');
    const contentInput = document.getElementById('note-content-input');
    const noteType = document.querySelector('.note-type-tab.active')?.dataset.type || 'trade';
    
    const title = titleInput?.value.trim() || 'Untitled Note';
    const content = contentInput?.value || '';
    
    // Get trading fields
    const contractSymbol = document.getElementById('note-contract-symbol')?.value.trim() || null;
    const contractType = document.getElementById('note-contract-type')?.value || 'futures';
    const entryPrice = document.getElementById('note-entry-price')?.value ? parseFloat(document.getElementById('note-entry-price').value) : null;
    const exitPrice = document.getElementById('note-exit-price')?.value ? parseFloat(document.getElementById('note-exit-price').value) : null;
    const quantity = document.getElementById('note-quantity')?.value ? parseInt(document.getElementById('note-quantity').value) : 1;
    const direction = document.getElementById('note-direction')?.value || null;
    const entryTime = document.getElementById('note-entry-time')?.value || null;
    const exitTime = document.getElementById('note-exit-time')?.value || null;
    const stopLoss = document.getElementById('note-stop-loss')?.value ? parseFloat(document.getElementById('note-stop-loss').value) : null;
    const takeProfit = document.getElementById('note-take-profit')?.value ? parseFloat(document.getElementById('note-take-profit').value) : null;
    const strategy = document.getElementById('note-strategy')?.value.trim() || null;
    const timeframe = document.getElementById('note-timeframe')?.value || null;
    const category = document.getElementById('note-category')?.value || null;
    
    // Calculate P&L if both prices exist
    let pnl = null;
    let pnlPercent = null;
    if (entryPrice && exitPrice && direction) {
        pnl = calculatePnL(entryPrice, exitPrice, quantity, direction);
        pnlPercent = calculatePnLPercent(entryPrice, exitPrice, direction);
    }
    
    try {
        const method = currentNoteId ? 'PUT' : 'POST';
        const body = {
            title: title,
            content: content,
            color: selectedColor,
            notes_type: noteType,
            contract_symbol: contractSymbol,
            contract_type: contractType,
            entry_price: entryPrice,
            exit_price: exitPrice,
            quantity: quantity,
            direction: direction,
            pnl: pnl,
            pnl_percent: pnlPercent,
            entry_time: entryTime,
            exit_time: exitTime,
            stop_loss: stopLoss,
            take_profit: takeProfit,
            strategy: strategy,
            timeframe: timeframe,
            category: category
        };
        
        if (currentNoteId) {
            body.id = currentNoteId;
        }
        
        const response = await fetch('api/notes.php', {
            method: method,
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(body)
        });

        const result = await response.json();
        
        if (result.success) {
            closeNoteModal();
            fetchNotes();
        } else {
            alert(result.message || 'Error saving note');
        }
    } catch (error) {
        console.error('Error saving note:', error);
        alert('Error saving note');
    }
}

// Delete note
async function deleteNote(noteId) {
    if (!confirm('Are you sure you want to delete this note?')) {
        return;
    }

    try {
        const response = await fetch('api/notes.php', {
            method: 'DELETE',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ id: parseInt(noteId) })
        });

        const result = await response.json();
        
        if (result.success) {
            fetchNotes();
        } else {
            alert(result.message || 'Error deleting note');
        }
    } catch (error) {
        console.error('Error deleting note:', error);
        alert('Error deleting note');
    }
}

// Close note modal
function closeNoteModal() {
    console.log('closeNoteModal called');
    const modal = document.getElementById('note-modal');
    if (modal) {
        modal.classList.remove('active');
        document.body.style.overflow = ''; // Restore scrolling
    }
    currentNoteId = null;
}

// Setup event listeners
function setupNotesListeners() {
    // Remove existing listeners to prevent duplicates
    const addNoteBtn = document.getElementById('add-note-btn');
    if (addNoteBtn) {
        // Clone and replace to remove all event listeners
        const newBtn = addNoteBtn.cloneNode(true);
        addNoteBtn.parentNode.replaceChild(newBtn, addNoteBtn);
        
        // Add fresh event listener
        newBtn.addEventListener('click', function(e) {
            e.preventDefault();
            e.stopPropagation();
            console.log('Add note button clicked');
            openNewNoteModal();
        });
        console.log('Add note button listener attached');
    } else {
        console.warn('Add note button not found');
    }

    // Close modal buttons
    const closeModalBtn = document.getElementById('close-note-modal');
    if (closeModalBtn) {
        closeModalBtn.addEventListener('click', closeNoteModal);
    }

    const cancelBtn = document.getElementById('cancel-note-btn');
    if (cancelBtn) {
        cancelBtn.addEventListener('click', closeNoteModal);
    }

    // Save note button
    const saveNoteBtn = document.getElementById('save-note-btn');
    if (saveNoteBtn) {
        saveNoteBtn.addEventListener('click', saveNote);
    }

    // Color picker
    document.querySelectorAll('.color-option').forEach(option => {
        option.addEventListener('click', function() {
            document.querySelectorAll('.color-option').forEach(opt => opt.classList.remove('active'));
            this.classList.add('active');
            selectedColor = this.dataset.color;
        });
    });
    
    // Note type tabs
    document.querySelectorAll('.note-type-tab').forEach(tab => {
        tab.addEventListener('click', function() {
            document.querySelectorAll('.note-type-tab').forEach(t => t.classList.remove('active'));
            this.classList.add('active');
            const noteType = this.dataset.type;
            toggleTradingDetailsSection(noteType);
        });
    });
    
    // P&L calculation on input change
    const entryPriceInput = document.getElementById('note-entry-price');
    const exitPriceInput = document.getElementById('note-exit-price');
    const quantityInput = document.getElementById('note-quantity');
    const directionInput = document.getElementById('note-direction');
    
    [entryPriceInput, exitPriceInput, quantityInput, directionInput].forEach(input => {
        if (input) {
            input.addEventListener('input', updatePnLDisplay);
            input.addEventListener('change', updatePnLDisplay);
        }
    });
    
    // 3D visualization controls
    const rotateToggle = document.getElementById('notes-rotate-toggle');
    if (rotateToggle) {
        rotateToggle.addEventListener('click', toggleNotesAutoRotate);
    }
    
    const resetView = document.getElementById('notes-reset-view');
    if (resetView) {
        resetView.addEventListener('click', resetNotesView);
    }

    // Search input
    const searchInput = document.getElementById('notes-search-input');
    if (searchInput) {
        let searchTimeout;
        searchInput.addEventListener('input', function() {
            clearTimeout(searchTimeout);
            searchTimeout = setTimeout(() => {
                fetchNotes(this.value);
            }, 300);
        });
    }

    // Close modal on outside click
    const modal = document.getElementById('note-modal');
    if (modal) {
        modal.addEventListener('click', function(e) {
            if (e.target === modal) {
                closeNoteModal();
            }
        });
    }
}

// Initialize notes
// 3D Visualization variables
let notesScene, notesCamera, notesRenderer, notesControls;
let notesAnimationId = null;
let notesAutoRotate = false;
let notesBars = [];

// Render 3D Trading Visualization
function renderTrading3DVisualization(trades) {
    console.log('renderTrading3DVisualization called with', trades?.length || 0, 'trades');
    
    // Check if notes page is active
    const notesPage = document.getElementById('page-notes');
    if (!notesPage || !notesPage.classList.contains('active')) {
        console.log('Notes page not active, skipping 3D visualization');
        return;
    }
    
    const canvas = document.getElementById('notes-3d-canvas');
    const loading = document.getElementById('notes-viz-loading');
    
    if (!canvas) {
        console.warn('3D canvas not found');
        return;
    }
    
    console.log('Canvas found, dimensions:', canvas.offsetWidth, 'x', canvas.offsetHeight);
    
    // Show loading
    if (loading) {
        loading.style.display = 'flex';
        loading.innerHTML = '<i class="fas fa-spinner fa-spin"></i><span>Loading 3D visualization...</span>';
    }
    
    // Wait a bit for canvas to be ready
    setTimeout(() => {
        // Check if Three.js is loaded
        if (typeof THREE === 'undefined') {
            console.log('Three.js not loaded, loading now...');
            loadThreeJsForNotes().then(() => {
                console.log('Three.js loaded, initializing scene...');
                setTimeout(() => {
                    initNotes3DScene(trades || []);
                }, 300);
            }).catch(err => {
                console.error('Failed to load Three.js:', err);
                if (loading) {
                    loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Failed to load 3D library</span>';
                }
            });
        } else {
            console.log('Three.js already available, initializing scene...');
            setTimeout(() => {
                initNotes3DScene(trades || []);
            }, 300);
        }
    }, 100);
}

// Load Three.js for notes
function loadThreeJsForNotes() {
    return new Promise((resolve, reject) => {
        if (typeof THREE !== 'undefined') {
            console.log('Three.js already loaded');
            resolve();
            return;
        }
        
        // Check if script already exists
        const existingScript = document.querySelector('script[src*="three.js"]');
        if (existingScript) {
            existingScript.addEventListener('load', resolve);
            existingScript.addEventListener('error', reject);
            return;
        }
        
        const script = document.createElement('script');
        script.src = 'https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js';
        script.onload = () => {
            console.log('Three.js loaded successfully');
            resolve();
        };
        script.onerror = (err) => {
            console.error('Failed to load Three.js:', err);
            reject(err);
        };
        document.head.appendChild(script);
    });
}

// Initialize 3D scene for notes
function initNotes3DScene(trades) {
    console.log('initNotes3DScene called');
    const canvas = document.getElementById('notes-3d-canvas');
    const loading = document.getElementById('notes-viz-loading');
    
    if (!canvas) {
        console.error('Canvas element not found');
        if (loading) loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Canvas not found</span>';
        return;
    }
    
    if (typeof THREE === 'undefined') {
        console.error('Three.js is not loaded');
        if (loading) loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Three.js failed to load</span>';
        return;
    }
    
    console.log('Initializing 3D scene with THREE version:', THREE.REVISION);
    
    try {
        // Clear existing scene
        if (notesAnimationId) {
            cancelAnimationFrame(notesAnimationId);
            notesAnimationId = null;
        }
        if (notesRenderer) {
            notesRenderer.dispose();
            notesRenderer = null;
        }
        
        // Ensure canvas has dimensions - force from container if needed
        const container = canvas.parentElement;
        let width = canvas.offsetWidth;
        let height = canvas.offsetHeight;
        
        if (width === 0 || height === 0) {
            console.warn('Canvas has no dimensions, getting from container...');
            if (container) {
                width = container.offsetWidth || 450;
                height = container.offsetHeight || 450;
                // Set explicit dimensions
                canvas.style.width = width + 'px';
                canvas.style.height = height + 'px';
                canvas.width = width;
                canvas.height = height;
                console.log('Set canvas dimensions to:', width, 'x', height);
            } else {
                console.warn('No container found, retrying...');
                setTimeout(() => initNotes3DScene(trades), 300);
                return;
            }
        }
        
        // Scene setup
        notesScene = new THREE.Scene();
        notesScene.background = new THREE.Color(0x0a0f1a);
        console.log('Scene created');
        
        // Use the dimensions we already calculated
        const aspect = width / height || 1;
        notesCamera = new THREE.PerspectiveCamera(75, aspect, 0.1, 1000);
        notesCamera.position.set(0, 5, 10);
        notesCamera.lookAt(0, 0, 0);
        console.log('Camera created with aspect:', aspect);
        
        // Renderer
        
        notesRenderer = new THREE.WebGLRenderer({ 
            canvas: canvas, 
            antialias: true,
            alpha: false,
            powerPreference: "high-performance"
        });
        notesRenderer.setSize(width, height);
        notesRenderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        notesRenderer.setClearColor(0x0a0f1a, 1);
        console.log('Renderer created with size:', width, 'x', height);
        
        // Lighting
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
        notesScene.add(ambientLight);
        
        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(5, 10, 5);
        notesScene.add(directionalLight);
        
        const pointLight = new THREE.PointLight(0x2563eb, 0.6);
        pointLight.position.set(-5, 5, -5);
        notesScene.add(pointLight);
        console.log('Lighting added');
        
        // Grid
        const gridHelper = new THREE.GridHelper(20, 20, 0x1e293b, 0x0f172a);
        notesScene.add(gridHelper);
        console.log('Grid added');
    
    // Create bars from trades
    notesBars = [];
    const sortedTrades = [...trades].sort((a, b) => 
        new Date(a.entry_time || a.created_at) - new Date(b.entry_time || b.created_at)
    );
    
    if (sortedTrades.length === 0) {
        console.log('No trades found, creating placeholder visualization');
        // Show placeholder visualization with a simple cube
        const geometry = new THREE.BoxGeometry(2, 2, 2);
        const material = new THREE.MeshPhongMaterial({ 
            color: 0x2563eb, 
            emissive: 0x2563eb, 
            emissiveIntensity: 0.3,
            transparent: true,
            opacity: 0.7
        });
        const placeholder = new THREE.Mesh(geometry, material);
        placeholder.position.set(0, 1, 0);
        notesScene.add(placeholder);
        
        // Add wireframe
        const wireframe = new THREE.WireframeGeometry(geometry);
        const line = new THREE.LineSegments(wireframe, new THREE.LineBasicMaterial({ color: 0x60a5fa }));
        line.position.set(0, 1, 0);
        notesScene.add(line);
        
        // Add a few more cubes for visual interest
        for (let i = 0; i < 3; i++) {
            const smallGeo = new THREE.BoxGeometry(0.5, 0.5, 0.5);
            const smallMat = new THREE.MeshPhongMaterial({ 
                color: 0x3b82f6, 
                emissive: 0x3b82f6, 
                emissiveIntensity: 0.2 
            });
            const smallCube = new THREE.Mesh(smallGeo, smallMat);
            smallCube.position.set((i - 1) * 1.5, 0.5, 0);
            notesScene.add(smallCube);
        }
        console.log('Placeholder visualization created');
    } else {
        console.log('Creating bars for', sortedTrades.length, 'trades');
        sortedTrades.forEach((trade, index) => {
            const pnl = trade.pnl || (trade.entry_price && trade.exit_price ? 
                calculatePnL(trade.entry_price, trade.exit_price, trade.quantity || 1, trade.direction) : 0);
            
            if (pnl === null || pnl === undefined) return;
            
            const height = Math.max(0.1, Math.min(5, Math.abs(pnl) / 100));
            const color = pnl >= 0 ? 0x10b981 : 0xef4444;
            
            const geometry = new THREE.BoxGeometry(0.3, height, 0.3);
            const material = new THREE.MeshPhongMaterial({ color: color, emissive: color, emissiveIntensity: 0.2 });
            const bar = new THREE.Mesh(geometry, material);
            
            const x = (index - sortedTrades.length / 2) * 0.5;
            bar.position.set(x, height / 2, 0);
            
            notesScene.add(bar);
            notesBars.push({ mesh: bar, pnl: pnl });
        });
    }
    
    // Controls (simple rotation)
    let mouseX = 0;
    let mouseY = 0;
    let targetRotationX = 0;
    let targetRotationY = 0;
    
    canvas.addEventListener('mousemove', (e) => {
        const rect = canvas.getBoundingClientRect();
        mouseX = ((e.clientX - rect.left) / rect.width) * 2 - 1;
        mouseY = ((e.clientY - rect.top) / rect.height) * 2 - 1;
        targetRotationY = mouseX * 0.5;
        targetRotationX = -mouseY * 0.5;
    });
    
        // Animation loop
        function animate() {
            notesAnimationId = requestAnimationFrame(animate);
            
            if (notesAutoRotate) {
                notesScene.rotation.y += 0.005;
            } else {
                notesScene.rotation.y += (targetRotationY - notesScene.rotation.y) * 0.05;
                notesScene.rotation.x += (targetRotationX - notesScene.rotation.x) * 0.05;
            }
            
            // Animate bars
            notesBars.forEach((bar, index) => {
                if (bar && bar.mesh) {
                    bar.mesh.rotation.y += 0.01;
                }
            });
            
            try {
                notesRenderer.render(notesScene, notesCamera);
            } catch (renderError) {
                console.error('Render error:', renderError);
                cancelAnimationFrame(notesAnimationId);
            }
        }
        
        // Render once immediately to ensure it works
        try {
            notesRenderer.render(notesScene, notesCamera);
            console.log('Initial render complete');
        } catch (renderError) {
            console.error('Error on initial render:', renderError);
        }
        
        animate();
        console.log('Animation started');
        
        // Hide loading after confirming render works
        setTimeout(() => {
            if (loading) {
                loading.style.display = 'none';
                console.log('Loading indicator hidden - 3D visualization ready');
            }
        }, 500);
        
        // Handle resize
        const resizeObserver = new ResizeObserver(() => {
            if (canvas && notesCamera && notesRenderer) {
                const width = canvas.offsetWidth;
                const height = canvas.offsetHeight;
                if (width > 0 && height > 0) {
                    notesCamera.aspect = width / height;
                    notesCamera.updateProjectionMatrix();
                    notesRenderer.setSize(width, height);
                }
            }
        });
        resizeObserver.observe(canvas);
        
    } catch (error) {
        console.error('Error initializing 3D scene:', error);
        console.error('Error stack:', error.stack);
        if (loading) {
            loading.innerHTML = '<i class="fas fa-exclamation-triangle"></i><span>Error: ' + error.message + '</span>';
        }
    }
}

// Toggle auto rotate for notes 3D
function toggleNotesAutoRotate() {
    notesAutoRotate = !notesAutoRotate;
    const btn = document.getElementById('notes-rotate-toggle');
    if (btn) {
        btn.classList.toggle('active', notesAutoRotate);
    }
}

// Reset notes 3D view
function resetNotesView() {
    if (notesScene) {
        notesScene.rotation.x = 0;
        notesScene.rotation.y = 0;
    }
    if (notesCamera) {
        notesCamera.position.set(0, 5, 10);
        notesCamera.lookAt(0, 0, 0);
    }
}

// Watch for page activation
function watchNotesPageActivation() {
    const notesPage = document.getElementById('page-notes');
    if (!notesPage) {
        console.warn('Notes page element not found');
        return;
    }

    const observer = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                if (notesPage.classList.contains('active')) {
                    console.log('Notes page activated - reinitializing');
                    // Re-setup listeners when page becomes active
                    setupNotesListeners();
                    
                    // Re-fetch notes and render 3D when page becomes active
                    setTimeout(() => {
                        fetchNotes();
                        // Also ensure 3D visualization is initialized
                        setTimeout(() => {
                            const trades = allNotes.filter(n => n.notes_type === 'trade' || (n.contract_symbol && n.entry_price));
                            console.log('Rendering 3D visualization on page activation with', trades.length, 'trades');
                            renderTrading3DVisualization(trades.length > 0 ? trades : []);
                        }, 500);
                    }, 200);
                }
            }
        });
    });

    observer.observe(notesPage, {
        attributes: true,
        attributeFilter: ['class']
    });
    
    // Also check if page is already active on init
    if (notesPage.classList.contains('active')) {
        console.log('Notes page is already active on init');
        setTimeout(() => {
            setupNotesListeners();
            const trades = allNotes.filter(n => n.notes_type === 'trade' || (n.contract_symbol && n.entry_price));
            console.log('Initial render with', trades.length, 'trades from', allNotes.length, 'total notes');
            renderTrading3DVisualization(trades.length > 0 ? trades : []);
        }, 500);
    }
}

function initNotes() {
    console.log('Initializing notes module');
    
    // Initialize page controls (filters, views, etc.)
    initNotesPage();
    
    // Setup listeners immediately
    setupNotesListeners();
    
    // Watch for page activation
    watchNotesPageActivation();
    
    // Check if page is already active and initialize
    const notesPage = document.getElementById('page-notes');
    if (notesPage && notesPage.classList.contains('active')) {
        console.log('Notes page is active on init, setting up immediately');
        // Fetch notes and initialize 3D
        setTimeout(() => {
            fetchNotes();
            setTimeout(() => {
                const trades = allNotes.filter(n => n.notes_type === 'trade' || (n.contract_symbol && n.entry_price));
                console.log('Initializing 3D with', trades.length, 'trades');
                renderTrading3DVisualization(trades.length > 0 ? trades : []);
            }, 800);
        }, 500);
    } else {
        // Fetch notes after a short delay to ensure DOM is ready
        setTimeout(() => {
            fetchNotes();
        }, 300);
    }
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initNotes);
} else {
    initNotes();
}






