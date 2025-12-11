// Notes Management
let currentNoteId = null;
let selectedColor = '#1e293b';

// Fetch notes from API
async function fetchNotes(search = '') {
    try {
        const url = search ? `api/notes.php?search=${encodeURIComponent(search)}` : 'api/notes.php';
        const response = await fetch(url);
        if (!response.ok) throw new Error('Failed to fetch notes');
        
        const notes = await response.json();
        displayNotes(notes);
    } catch (error) {
        console.error('Error fetching notes:', error);
        const notesGrid = document.getElementById('notes-grid');
        if (notesGrid) {
            notesGrid.innerHTML = '<div class="loading">Error loading notes. Please try again later.</div>';
        }
    }
}

// Display notes
function displayNotes(notes) {
    const notesGrid = document.getElementById('notes-grid');
    if (!notesGrid) return;

    if (!notes || notes.length === 0) {
        notesGrid.innerHTML = '<div class="empty-state"><i class="fas fa-sticky-note"></i><p>No notes yet. Create your first note!</p></div>';
        return;
    }

    notesGrid.innerHTML = notes.map(note => createNoteCard(note)).join('');
    
    // Attach event listeners
    attachNoteListeners();
}

// Create note card HTML
function createNoteCard(note) {
    const preview = note.content ? note.content.substring(0, 150) + (note.content.length > 150 ? '...' : '') : 'No content';
    const date = new Date(note.updated_at).toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' });
    
    return `
        <div class="note-card" data-id="${note.id}" style="background-color: ${note.color || '#1e293b'}">
            <div class="note-card-header">
                <h3 class="note-title">${escapeHtml(note.title)}</h3>
                <div class="note-actions">
                    <button class="note-edit-btn" data-id="${note.id}" title="Edit">
                        <i class="fas fa-edit"></i>
                    </button>
                    <button class="note-delete-btn" data-id="${note.id}" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </div>
            </div>
            <div class="note-content">${escapeHtml(preview)}</div>
            <div class="note-date">${date}</div>
        </div>
    `;
}

// Escape HTML
function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
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
    currentNoteId = null;
    selectedColor = '#1e293b';
    
    const modal = document.getElementById('note-modal');
    const titleInput = document.getElementById('note-title-input');
    const contentInput = document.getElementById('note-content-input');
    const modalTitle = document.getElementById('note-modal-title');
    
    if (modal) {
        modal.classList.add('active');
        if (modalTitle) modalTitle.textContent = 'New Note';
        if (titleInput) titleInput.value = '';
        if (contentInput) contentInput.value = '';
        
        // Reset color selection
        document.querySelectorAll('.color-option').forEach(opt => {
            opt.classList.remove('active');
            if (opt.dataset.color === selectedColor) {
                opt.classList.add('active');
            }
        });
        
        if (titleInput) titleInput.focus();
    }
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
        const titleInput = document.getElementById('note-title-input');
        const contentInput = document.getElementById('note-content-input');
        const modalTitle = document.getElementById('note-modal-title');
        
        if (modal) {
            modal.classList.add('active');
            if (modalTitle) modalTitle.textContent = 'Edit Note';
            if (titleInput) titleInput.value = note.title || '';
            if (contentInput) contentInput.value = note.content || '';
            
            // Set color selection
            document.querySelectorAll('.color-option').forEach(opt => {
                opt.classList.remove('active');
                if (opt.dataset.color === selectedColor) {
                    opt.classList.add('active');
                }
            });
            
            if (titleInput) titleInput.focus();
        }
    } catch (error) {
        console.error('Error loading note:', error);
        alert('Error loading note');
    }
}

// Save note
async function saveNote() {
    const titleInput = document.getElementById('note-title-input');
    const contentInput = document.getElementById('note-content-input');
    
    const title = titleInput?.value.trim() || 'Untitled Note';
    const content = contentInput?.value || '';
    
    try {
        const method = currentNoteId ? 'PUT' : 'POST';
        const body = {
            title: title,
            content: content,
            color: selectedColor
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
    const modal = document.getElementById('note-modal');
    if (modal) {
        modal.classList.remove('active');
    }
    currentNoteId = null;
}

// Setup event listeners
function setupNotesListeners() {
    // Add note button
    const addNoteBtn = document.getElementById('add-note-btn');
    if (addNoteBtn) {
        addNoteBtn.addEventListener('click', openNewNoteModal);
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
function initNotes() {
    fetchNotes();
    setupNotesListeners();
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initNotes);
} else {
    initNotes();
}

