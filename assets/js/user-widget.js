// User Widget Management

let userData = null;
let userWidgetExpanded = false;

// Load user profile
async function loadUserProfile() {
    try {
        const response = await fetch('api/user.php');
        if (!response.ok) throw new Error('Failed to load user profile');
        
        userData = await response.json();
        displayUserWidget(userData);
    } catch (error) {
        console.error('Error loading user profile:', error);
        // Use default user
        userData = {
            name: 'User',
            email: 'user@algo3d.com',
            avatar_color: '#2563eb'
        };
        displayUserWidget(userData);
    }
}

// Display user widget
function displayUserWidget(user) {
    const userNameEl = document.getElementById('user-name');
    const userEmailEl = document.getElementById('user-email');
    const userInitialsEl = document.getElementById('user-initials');
    const userAvatarEl = document.getElementById('user-avatar');
    const profileInitialsEl = document.getElementById('profile-initials-large');
    const profileAvatarEl = document.getElementById('profile-avatar-large');
    
    if (userNameEl) {
        userNameEl.textContent = user.name || 'User';
    }
    
    if (userEmailEl) {
        userEmailEl.textContent = user.email || 'user@algo3d.com';
    }
    
    // Generate initials
    const initials = generateInitials(user.name || 'User');
    
    if (userInitialsEl) {
        userInitialsEl.textContent = initials;
    }
    
    if (profileInitialsEl) {
        profileInitialsEl.textContent = initials;
    }
    
    // Set avatar color
    const avatarColor = user.avatar_color || '#2563eb';
    if (userAvatarEl) {
        userAvatarEl.style.backgroundColor = avatarColor;
    }
    if (profileAvatarEl) {
        profileAvatarEl.style.backgroundColor = avatarColor;
    }
}

// Generate initials from name
function generateInitials(name) {
    const parts = name.trim().split(' ');
    if (parts.length >= 2) {
        return (parts[0][0] + parts[parts.length - 1][0]).toUpperCase();
    }
    return name.substring(0, 2).toUpperCase();
}

// Toggle user widget menu
function toggleUserWidget() {
    const menu = document.getElementById('user-widget-menu');
    const arrow = document.getElementById('user-widget-arrow');
    
    if (menu && arrow) {
        userWidgetExpanded = !userWidgetExpanded;
        menu.classList.toggle('expanded', userWidgetExpanded);
        arrow.classList.toggle('rotated', userWidgetExpanded);
    }
}

// Handle user menu actions
function handleUserMenuAction(action) {
    switch (action) {
        case 'profile':
            openProfileModal();
            toggleUserWidget();
            break;
        case 'preferences':
            // Navigate to settings page
            const settingsItem = document.querySelector('.sidebar-item[data-page="settings"]');
            if (settingsItem) {
                settingsItem.click();
            }
            toggleUserWidget();
            break;
        case 'stats':
            openStatsModal();
            toggleUserWidget();
            break;
        case 'logout':
            if (confirm('Are you sure you want to logout?')) {
                // Clear local storage
                localStorage.clear();
                // Reload page
                window.location.reload();
            }
            break;
    }
}

// Open profile modal
function openProfileModal() {
    const modal = document.getElementById('user-profile-modal');
    if (!modal || !userData) return;
    
    modal.classList.add('active');
    
    // Populate form
    document.getElementById('profile-name').value = userData.name || '';
    document.getElementById('profile-email').value = userData.email || '';
    document.getElementById('profile-timezone').value = userData.timezone || 'America/New_York';
    document.getElementById('profile-currency').value = userData.currency || 'USD';
}

// Close profile modal
function closeProfileModal() {
    const modal = document.getElementById('user-profile-modal');
    if (modal) {
        modal.classList.remove('active');
    }
}

// Save profile
async function saveProfile() {
    const name = document.getElementById('profile-name').value.trim();
    const email = document.getElementById('profile-email').value.trim();
    const timezone = document.getElementById('profile-timezone').value;
    const currency = document.getElementById('profile-currency').value;
    
    if (!name || !email) {
        alert('Please fill in all required fields');
        return;
    }
    
    try {
        const response = await fetch('api/user.php', {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                name: name,
                email: email,
                timezone: timezone,
                currency: currency,
                avatar_color: userData?.avatar_color || '#2563eb'
            })
        });

        const result = await response.json();
        
        if (result.success) {
            await loadUserProfile();
            closeProfileModal();
            alert('Profile updated successfully!');
        } else {
            alert(result.message || 'Error updating profile');
        }
    } catch (error) {
        console.error('Error saving profile:', error);
        alert('Error saving profile');
    }
}

// Open stats modal
async function openStatsModal() {
    const modal = document.getElementById('user-stats-modal');
    if (!modal) return;
    
    modal.classList.add('active');
    
    // Load stats
    try {
        const response = await fetch('api/user_stats.php');
        if (!response.ok) throw new Error('Failed to load stats');
        
        const stats = await response.json();
        
        document.getElementById('stat-total-symbols').textContent = stats.total_symbols || 0;
        document.getElementById('stat-total-todos').textContent = stats.total_todos || 0;
        document.getElementById('stat-total-notes').textContent = stats.total_notes || 0;
        document.getElementById('stat-completed-todos').textContent = stats.completed_todos || 0;
        document.getElementById('stat-account-age').textContent = stats.account_age || 0;
        document.getElementById('stat-api-calls').textContent = stats.api_calls_today || 0;
    } catch (error) {
        console.error('Error loading stats:', error);
    }
}

// Close stats modal
function closeStatsModal() {
    const modal = document.getElementById('user-stats-modal');
    if (modal) {
        modal.classList.remove('active');
    }
}

// Change avatar color
function changeAvatarColor() {
    const colors = ['#2563eb', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899'];
    const currentColor = userData?.avatar_color || '#2563eb';
    const currentIndex = colors.indexOf(currentColor);
    const nextIndex = (currentIndex + 1) % colors.length;
    const newColor = colors[nextIndex];
    
    // Update avatar immediately
    const userAvatarEl = document.getElementById('user-avatar');
    const profileAvatarEl = document.getElementById('profile-avatar-large');
    
    if (userAvatarEl) {
        userAvatarEl.style.backgroundColor = newColor;
    }
    if (profileAvatarEl) {
        profileAvatarEl.style.backgroundColor = newColor;
    }
    
    // Save to userData
    if (userData) {
        userData.avatar_color = newColor;
    }
}

// Setup event listeners
function setupUserWidgetListeners() {
    // Toggle widget
    const widgetToggle = document.getElementById('user-widget-toggle');
    if (widgetToggle) {
        widgetToggle.addEventListener('click', toggleUserWidget);
    }
    
    // Menu items
    document.querySelectorAll('.user-menu-item').forEach(item => {
        item.addEventListener('click', function() {
            const action = this.dataset.action;
            if (action) {
                handleUserMenuAction(action);
            }
        });
    });
    
    // Profile modal
    const closeProfileBtn = document.getElementById('close-profile-modal');
    if (closeProfileBtn) {
        closeProfileBtn.addEventListener('click', closeProfileModal);
    }
    
    const cancelProfileBtn = document.getElementById('cancel-profile-btn');
    if (cancelProfileBtn) {
        cancelProfileBtn.addEventListener('click', closeProfileModal);
    }
    
    const saveProfileBtn = document.getElementById('save-profile-btn');
    if (saveProfileBtn) {
        saveProfileBtn.addEventListener('click', saveProfile);
    }
    
    const changeAvatarBtn = document.getElementById('change-avatar-btn');
    if (changeAvatarBtn) {
        changeAvatarBtn.addEventListener('click', changeAvatarColor);
    }
    
    // Stats modal
    const closeStatsBtn = document.getElementById('close-stats-modal');
    if (closeStatsBtn) {
        closeStatsBtn.addEventListener('click', closeStatsModal);
    }
    
    // Close modals on outside click
    const profileModal = document.getElementById('user-profile-modal');
    if (profileModal) {
        profileModal.addEventListener('click', function(e) {
            if (e.target === profileModal) {
                closeProfileModal();
            }
        });
    }
    
    const statsModal = document.getElementById('user-stats-modal');
    if (statsModal) {
        statsModal.addEventListener('click', function(e) {
            if (e.target === statsModal) {
                closeStatsModal();
            }
        });
    }
}

// Initialize user widget
function initUserWidget() {
    loadUserProfile();
    setupUserWidgetListeners();
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initUserWidget);
} else {
    initUserWidget();
}









