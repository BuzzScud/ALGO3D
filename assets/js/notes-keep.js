// Google Keep-like Notes Management
let allNotes = [];
let currentNoteId = null;
let selectedColor = '#cfe2ff';
let currentView = 'grid';
let searchQuery = '';

// Initialize notes page
function initKeepNotes() {
    setupEventListeners();
    fetchNotes();
}

// Setup event listeners
function setupEventListeners() {
    // Quick note input
    const quickNoteTitle = document.getElementById('quick-note-title');
    const quickNoteExpanded = document.getElementById('quick-note-expanded');
    const quickNoteContent = document.getElementById('quick-note-content');
    const quickNoteClose = document.getElementById('quick-note-close');
    
    if (quickNoteTitle) {
        quickNoteTitle.addEventListener('focus', () => {
            if (quickNoteExpanded) quickNoteExpanded.style.display = 'block';
        });
        
        quickNoteTitle.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' && !e.shiftKey) {
                e.preventDefault();
                if (quickNoteContent) quickNoteContent.focus();
            }
        });
    }
    
    // Save button
    const quickNoteSave = document.getElementById('quick-note-save');
    if (quickNoteSave) {
        quickNoteSave.addEventListener('click', () => {
            saveQuickNote();
        });
    }
    
    if (quickNoteClose) {
        quickNoteClose.addEventListener('click', () => {
            closeQuickNote();
        });
    }
    
    // Close quick note on outside click
    document.addEventListener('click', (e) => {
        const quickNoteContainer = document.getElementById('quick-note-input');
        if (quickNoteContainer && !quickNoteContainer.contains(e.target) && 
            quickNoteTitle && quickNoteTitle.value.trim() === '' && 
            (!quickNoteContent || quickNoteContent.value.trim() === '')) {
            if (quickNoteExpanded) quickNoteExpanded.style.display = 'none';
        }
    });
    
    // View toggle
    document.querySelectorAll('.view-toggle-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            document.querySelectorAll('.view-toggle-btn').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            currentView = this.dataset.view;
            displayNotes(allNotes);
        });
    });
    
    // Search
    const searchInput = document.getElementById('notes-search-input');
    const searchClear = document.getElementById('notes-search-clear');
    
    if (searchInput) {
        let searchTimeout;
        searchInput.addEventListener('input', function() {
            clearTimeout(searchTimeout);
            searchTimeout = setTimeout(() => {
                searchQuery = this.value.trim();
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
    
    // Color buttons in quick note
    document.querySelectorAll('.color-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            selectedColor = this.dataset.color;
            document.querySelectorAll('.color-btn').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
        });
    });
    
    // Modal
    const modal = document.getElementById('note-modal');
    const closeModal = document.getElementById('close-note-modal');
    const cancelBtn = document.getElementById('cancel-note-btn');
    const saveBtn = document.getElementById('save-note-btn');
    
    if (closeModal) {
        closeModal.addEventListener('click', closeNoteModal);
    }
    
    if (cancelBtn) {
        cancelBtn.addEventListener('click', closeNoteModal);
    }
    
    if (saveBtn) {
        saveBtn.addEventListener('click', saveNote);
    }
    
    if (modal) {
        modal.addEventListener('click', (e) => {
            if (e.target === modal) {
                closeNoteModal();
            }
        });
    }
    
    // Color options in modal
    document.querySelectorAll('.color-option').forEach(option => {
        option.addEventListener('click', function() {
            document.querySelectorAll('.color-option').forEach(opt => opt.classList.remove('active'));
            this.classList.add('active');
            selectedColor = this.dataset.color;
        });
    });
}

// Fetch notes from API
async function fetchNotes() {
    try {
        const url = searchQuery ? `api/notes.php?search=${encodeURIComponent(searchQuery)}` : 'api/notes.php';
        const response = await fetch(url);
        if (!response.ok) throw new Error('Failed to fetch notes');
        
        const notes = await response.json();
        allNotes = Array.isArray(notes) ? notes : [];
        displayNotes(allNotes);
    } catch (error) {
        console.error('Error fetching notes:', error);
        const notesGrid = document.getElementById('notes-grid');
        if (notesGrid) {
            notesGrid.innerHTML = '<div class="empty-state"><i class="fas fa-exclamation-triangle"></i><p>Error loading notes</p></div>';
        }
    }
}

// Display notes
function displayNotes(notes) {
    const notesGrid = document.getElementById('notes-grid');
    if (!notesGrid) return;
    
    // Filter by search
    let filtered = [...notes];
    if (searchQuery) {
        const query = searchQuery.toLowerCase();
        filtered = filtered.filter(note => 
            (note.title || '').toLowerCase().includes(query) ||
            (note.content || '').toLowerCase().includes(query) ||
            (note.labels && Array.isArray(note.labels) && note.labels.some(label => label.toLowerCase().includes(query)))
        );
    }
    
    // Sort: pinned first, then by updated date
    filtered.sort((a, b) => {
        if (a.pinned && !b.pinned) return -1;
        if (!a.pinned && b.pinned) return 1;
        return new Date(b.updated_at || b.created_at) - new Date(a.updated_at || a.created_at);
    });
    
    if (filtered.length === 0) {
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
    
    // Attach event listeners to note cards
    attachNoteCardListeners();
}

// Create note card HTML
function createNoteCard(note) {
    const title = note.title || 'Untitled Note';
    const content = note.content || '';
    const color = note.color || '#cfe2ff';
    const pinned = note.pinned || false;
    const labels = Array.isArray(note.labels) ? note.labels : [];
    const date = formatDate(note.updated_at || note.created_at);
    
    return `
        <div class="keep-note-card" data-id="${note.id}" style="background-color: ${color}">
            ${pinned ? '<div class="note-pin"><i class="fas fa-thumbtack"></i></div>' : ''}
            <div class="note-card-content" onclick="editNote(${note.id})">
                ${title ? `<div class="note-card-title">${escapeHtml(title)}</div>` : ''}
                ${content ? `<div class="note-card-text">${escapeHtml(content)}</div>` : ''}
                ${labels.length > 0 ? `<div class="note-card-labels">${labels.map(label => `<span class="note-label">${escapeHtml(label)}</span>`).join('')}</div>` : ''}
            </div>
            <div class="note-card-footer">
                <div class="note-card-date">${date}</div>
                <div class="note-card-actions">
                    <button class="note-action-btn" onclick="togglePinNote(${note.id}, event)" title="${pinned ? 'Unpin' : 'Pin'}">
                        <i class="fas fa-thumbtack"></i>
                    </button>
                    <button class="note-action-btn" onclick="editNote(${note.id}, event)" title="Edit">
                        <i class="fas fa-edit"></i>
                    </button>
                    <button class="note-action-btn" onclick="deleteNote(${note.id}, event)" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </div>
            </div>
        </div>
    `;
}

// Attach event listeners to note cards
function attachNoteCardListeners() {
    // Actions are handled via onclick in the HTML for simplicity
}

// Save quick note
async function saveQuickNote() {
    const titleInput = document.getElementById('quick-note-title');
    const contentInput = document.getElementById('quick-note-content');
    const expanded = document.getElementById('quick-note-expanded');
    
    if (!titleInput || !contentInput) return;
    
    const title = titleInput.value.trim();
    const content = contentInput.value.trim();
    
    if (!title && !content) {
        if (expanded) expanded.style.display = 'none';
        return;
    }
    
    try {
        const response = await fetch('api/notes.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                title: title || 'Untitled Note',
                content: content,
                color: selectedColor,
                pinned: 0,
                labels: [],
                notes_type: 'note'
            })
        });
        
        const result = await response.json();
        if (result.success) {
            // Clear inputs
            titleInput.value = '';
            contentInput.value = '';
            selectedColor = '#cfe2ff';
            document.querySelectorAll('.color-btn').forEach(b => b.classList.remove('active'));
            if (expanded) expanded.style.display = 'none';
            
            // Refresh notes
            fetchNotes();
        }
    } catch (error) {
        console.error('Error saving note:', error);
        alert('Error saving note');
    }
}

// Close quick note without saving
function closeQuickNote() {
    const titleInput = document.getElementById('quick-note-title');
    const contentInput = document.getElementById('quick-note-content');
    const expanded = document.getElementById('quick-note-expanded');
    
    if (titleInput) titleInput.value = '';
    if (contentInput) contentInput.value = '';
    selectedColor = '#cfe2ff';
    document.querySelectorAll('.color-btn').forEach(b => b.classList.remove('active'));
    if (expanded) expanded.style.display = 'none';
}

// Edit note
async function editNote(noteId, event) {
    if (event) event.stopPropagation();
    
    try {
        const response = await fetch('api/notes.php');
        if (!response.ok) throw new Error('Failed to fetch notes');
        
        const notes = await response.json();
        const note = Array.isArray(notes) ? notes.find(n => n.id == noteId) : null;
        
        if (!note) {
            alert('Note not found');
            return;
        }
        
        currentNoteId = note.id;
        selectedColor = note.color || '#cfe2ff';
        
        // Populate modal
        const titleInput = document.getElementById('note-title-input');
        const contentInput = document.getElementById('note-content-input');
        const labelsInput = document.getElementById('note-labels-input');
        
        if (titleInput) titleInput.value = note.title || '';
        if (contentInput) contentInput.value = note.content || '';
        if (labelsInput) {
            const labels = Array.isArray(note.labels) ? note.labels : [];
            labelsInput.value = labels.join(', ');
        }
        
        // Set color - check both exact match and closest match
        document.querySelectorAll('.color-option').forEach(opt => {
            opt.classList.remove('active');
            const optColor = opt.dataset.color.toLowerCase();
            const noteColor = (selectedColor || '#cfe2ff').toLowerCase();
            if (optColor === noteColor) {
                opt.classList.add('active');
            }
        });
        
        // If no exact match, set blue as default
        const hasActive = document.querySelector('.color-option.active');
        if (!hasActive) {
            const blueOption = document.querySelector('.color-option[data-color="#cfe2ff"]');
            if (blueOption) {
                blueOption.classList.add('active');
                selectedColor = '#cfe2ff';
            }
        }
        
        // Show modal
        const modal = document.getElementById('note-modal');
        if (modal) {
            modal.classList.add('active');
            document.body.style.overflow = 'hidden';
        }
    } catch (error) {
        console.error('Error loading note:', error);
        alert('Error loading note');
    }
}

// Save note from modal
async function saveNote() {
    const titleInput = document.getElementById('note-title-input');
    const contentInput = document.getElementById('note-content-input');
    const labelsInput = document.getElementById('note-labels-input');
    
    if (!titleInput || !contentInput) return;
    
    const title = titleInput.value.trim() || 'Untitled Note';
    const content = contentInput.value.trim();
    const labels = labelsInput ? labelsInput.value.split(',').map(l => l.trim()).filter(l => l) : [];
    
    try {
        const method = currentNoteId ? 'PUT' : 'POST';
        const body = {
                title: title,
                content: content,
                color: selectedColor || '#cfe2ff',
                labels: labels,
                pinned: 0,
                notes_type: 'note'
        };
        
        if (currentNoteId) {
            body.id = currentNoteId;
        }
        
        const response = await fetch('api/notes.php', {
            method: method,
            headers: { 'Content-Type': 'application/json' },
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

// Toggle pin
async function togglePinNote(noteId, event) {
    if (event) event.stopPropagation();
    
    try {
        const response = await fetch('api/notes.php');
        if (!response.ok) throw new Error('Failed to fetch notes');
        
        const notes = await response.json();
        const note = Array.isArray(notes) ? notes.find(n => n.id == noteId) : null;
        
        if (!note) return;
        
        const newPinned = !note.pinned;
        
        const updateResponse = await fetch('api/notes.php', {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                id: noteId,
                title: note.title || 'Untitled Note',
                content: note.content || '',
                color: note.color || '#cfe2ff',
                pinned: newPinned ? 1 : 0,
                labels: Array.isArray(note.labels) ? note.labels : [],
                notes_type: 'note'
            })
        });
        
        const result = await updateResponse.json();
        if (result.success) {
            fetchNotes();
        }
    } catch (error) {
        console.error('Error toggling pin:', error);
    }
}

// Delete note
async function deleteNote(noteId, event) {
    if (event) event.stopPropagation();
    
    if (!confirm('Are you sure you want to delete this note?')) {
        return;
    }
    
    try {
        const response = await fetch('api/notes.php', {
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
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
    const modal = document.getElementById('note-modal');
    if (modal) {
        modal.classList.remove('active');
        document.body.style.overflow = '';
    }
    currentNoteId = null;
    
    // Clear inputs
    const titleInput = document.getElementById('note-title-input');
    const contentInput = document.getElementById('note-content-input');
    const labelsInput = document.getElementById('note-labels-input');
    
    if (titleInput) titleInput.value = '';
    if (contentInput) contentInput.value = '';
    if (labelsInput) labelsInput.value = '';
}

// Format date
function formatDate(dateString) {
    if (!dateString) return '';
    const date = new Date(dateString);
    const now = new Date();
    const diffTime = Math.abs(now - date);
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    
    if (diffDays === 1) return 'Today';
    if (diffDays === 2) return 'Yesterday';
    if (diffDays <= 7) return `${diffDays - 1} days ago`;
    
    return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: date.getFullYear() !== now.getFullYear() ? 'numeric' : undefined });
}

// Escape HTML
function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Watch for page activation
function watchNotesPageActivation() {
    const notesPage = document.getElementById('page-notes');
    if (!notesPage) return;
    
    const observer = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                if (notesPage.classList.contains('active')) {
                    fetchNotes();
                }
            }
        });
    });
    
    observer.observe(notesPage, {
        attributes: true,
        attributeFilter: ['class']
    });
    
    // Initialize if already active
    if (notesPage.classList.contains('active')) {
        fetchNotes();
    }
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        initKeepNotes();
        watchNotesPageActivation();
    });
} else {
    initKeepNotes();
    watchNotesPageActivation();
}

