// Main JavaScript - Navigation and general functionality

// Page navigation
function initPageNavigation() {
    const sidebarItems = document.querySelectorAll('.sidebar-item');
    const pages = document.querySelectorAll('.page-content');
    
    sidebarItems.forEach(item => {
        item.addEventListener('click', function(e) {
            e.preventDefault();
            
            const pageName = this.dataset.page;
            
            // Update active sidebar item
            sidebarItems.forEach(i => i.classList.remove('active'));
            this.classList.add('active');
            
            // Show corresponding page
            pages.forEach(page => {
                page.classList.remove('active');
                if (page.id === `page-${pageName}`) {
                    page.classList.add('active');
                    
                    // Auto-load QQQ on Charts page
                    if (pageName === 'charts') {
                        setTimeout(() => {
                            // Try ChartsModule first, then fallback to direct function call
                            const symbolInput = document.getElementById('chart-symbol-input');
                            if (symbolInput) {
                                symbolInput.value = 'QQQ';
                                
                                if (typeof ChartsModule !== 'undefined' && ChartsModule.loadChart) {
                                    ChartsModule.loadChart('QQQ');
                                } else {
                                    // Fallback: trigger the load button click
                                    const loadBtn = document.getElementById('load-chart-btn');
                                    if (loadBtn) {
                                        loadBtn.click();
                                    }
                                }
                            }
                        }, 300);
                    }
                    
                    // Auto-load news feed on News page
                    if (pageName === 'news') {
                        setTimeout(() => {
                            if (typeof NewsFeedModule !== 'undefined' && NewsFeedModule.loadNews) {
                                NewsFeedModule.loadNews();
                            }
                        }, 300);
                    }
                }
            });
            
            // Close sidebar on mobile
            const sidebar = document.getElementById('sidebar');
            if (window.innerWidth <= 768 && sidebar) {
                sidebar.classList.remove('open');
            }
        });
    });
}

// Sidebar toggle
function initSidebarToggle() {
    const sidebarToggle = document.getElementById('sidebar-toggle');
    const sidebar = document.getElementById('sidebar');
    
    if (sidebarToggle && sidebar) {
        sidebarToggle.addEventListener('click', function() {
            sidebar.classList.toggle('collapsed');
            sidebar.classList.toggle('open');
            
            // Store preference
            const isCollapsed = sidebar.classList.contains('collapsed');
            localStorage.setItem('sidebar_collapsed', isCollapsed);
        });
    }
    
    // Restore sidebar state
    const savedState = localStorage.getItem('sidebar_collapsed');
    if (savedState === 'true' && sidebar) {
        sidebar.classList.add('collapsed');
    }
}

// Theme toggle in sidebar
function initSidebarThemeToggle() {
    const themeToggle = document.getElementById('sidebar-theme-toggle');
    const themeLabel = document.querySelector('.theme-toggle-label span');
    const themeIcon = document.querySelector('.theme-toggle-label i');
    
    if (!themeToggle) return;
    
    // Load saved theme
    const savedTheme = localStorage.getItem('algo3d_theme') || 'dark';
    const isDark = savedTheme === 'dark';
    
    // Set initial state
    themeToggle.checked = !isDark; // Checked = light mode
    updateTheme(isDark ? 'dark' : 'light');
    updateThemeToggleLabel(isDark);
    
    // Handle toggle change
    themeToggle.addEventListener('change', function() {
        const newTheme = this.checked ? 'light' : 'dark';
        updateTheme(newTheme);
        updateThemeToggleLabel(newTheme === 'dark');
        
        // Save to localStorage
        localStorage.setItem('algo3d_theme', newTheme);
        
        // Sync with settings page if it exists
        const settingsThemeSelect = document.getElementById('setting-theme');
        if (settingsThemeSelect) {
            settingsThemeSelect.value = newTheme;
        }
    });
}

// Update theme
function updateTheme(theme) {
    document.body.setAttribute('data-theme', theme);
    
    // Update TradingView widget theme
    const tradingViewScript = document.querySelector('script[src*="ticker-tape"]');
    if (tradingViewScript && tradingViewScript.innerHTML) {
        try {
            const config = JSON.parse(tradingViewScript.innerHTML.trim());
            config.colorTheme = theme;
            tradingViewScript.innerHTML = JSON.stringify(config);
        } catch (e) {
            // Script might not be ready yet
        }
    }
}

// Update theme toggle label
function updateThemeToggleLabel(isDark) {
    const themeLabel = document.querySelector('.theme-toggle-label span');
    const themeIcon = document.querySelector('.theme-toggle-label i');
    
    if (themeLabel) {
        themeLabel.textContent = isDark ? 'Dark Mode' : 'Light Mode';
    }
    
    if (themeIcon) {
        themeIcon.className = isDark ? 'fas fa-moon' : 'fas fa-sun';
    }
}

// Load dashboard stats
async function loadDashboardStats() {
    try {
        const response = await fetch('api/stats.php');
        if (!response.ok) throw new Error('Failed to fetch stats');
        
        const stats = await response.json();
        
        // Update stats only if elements exist (Quick Overview section may be removed)
        const statSymbols = document.getElementById('stat-symbols');
        const statTodos = document.getElementById('stat-todos');
        const statNotes = document.getElementById('stat-notes');
        const statApi = document.getElementById('stat-api');
        
        if (statSymbols) statSymbols.textContent = stats.symbols || '--';
        if (statTodos) statTodos.textContent = stats.todos || '--';
        if (statNotes) statNotes.textContent = stats.notes || '--';
        if (statApi) statApi.textContent = stats.api_status || '--';
    } catch (error) {
        console.error('Error loading stats:', error);
    }
}

// Handle window resize
function initResizeHandler() {
    let resizeTimeout;
    window.addEventListener('resize', function() {
        clearTimeout(resizeTimeout);
        resizeTimeout = setTimeout(function() {
            const sidebar = document.getElementById('sidebar');
            if (window.innerWidth > 768 && sidebar) {
                sidebar.classList.remove('open');
            }
        }, 100);
    });
}

// Dashboard Todo List
async function loadDashboardTodos() {
    const todoListEl = document.getElementById('todo-list-dashboard');
    if (!todoListEl) return;

    try {
        const response = await fetch(`api/todos.php?action=list&filter=active`);
        const result = await response.json();

        if (result.success && result.todos) {
            const todos = result.todos.filter(todo => !todo.completed);
            
            if (todos.length === 0) {
                todoListEl.innerHTML = `
                    <div class="todo-empty-state">
                        <i class="fas fa-clipboard-list"></i>
                        <p>No tasks for today. Add one above to get started!</p>
                    </div>
                `;
            } else {
                todoListEl.innerHTML = todos.slice(0, 5).map(todo => `
                    <div class="todo-item-dashboard" data-id="${todo.id}">
                        <input type="checkbox" class="todo-checkbox" ${todo.completed ? 'checked' : ''} data-id="${todo.id}">
                        <span class="todo-text ${todo.completed ? 'completed' : ''}">${todo.task}</span>
                    </div>
                `).join('');
                
                // Add checkbox event listeners
                todoListEl.querySelectorAll('.todo-checkbox').forEach(checkbox => {
                    checkbox.addEventListener('change', async function() {
                        const todoId = this.dataset.id;
                        const completed = this.checked;
                        await updateTodoStatus(todoId, completed);
                        loadDashboardTodos();
                    });
                });
            }
        }
    } catch (error) {
        console.error('Error loading dashboard todos:', error);
    }
}

async function updateTodoStatus(id, completed) {
    try {
        const response = await fetch('api/todos.php', {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id, completed })
        });
        return await response.json();
    } catch (error) {
        console.error('Error updating todo:', error);
        return { success: false };
    }
}

async function addDashboardTodo() {
    const input = document.getElementById('todo-input-dashboard');
    if (!input || !input.value.trim()) return;

    try {
        const response = await fetch('api/todos.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                task: input.value.trim(),
                priority: 'medium',
                repeat_type: 'none'
            })
        });
        
        const result = await response.json();
        if (result.success) {
            input.value = '';
            loadDashboardTodos();
        }
    } catch (error) {
        console.error('Error adding todo:', error);
    }
}

// Setup dashboard todo listeners
function setupDashboardTodos() {
    const addBtn = document.getElementById('add-todo-btn-dashboard');
    const input = document.getElementById('todo-input-dashboard');
    
    if (addBtn) {
        addBtn.addEventListener('click', addDashboardTodo);
    }
    
    if (input) {
        input.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                addDashboardTodo();
            }
        });
    }
    
    // Load todos when dashboard is active
    const dashboardPage = document.getElementById('page-dashboard');
    if (dashboardPage) {
        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (dashboardPage.classList.contains('active')) {
                        loadDashboardTodos();
                    }
                }
            });
        });
        observer.observe(dashboardPage, { attributes: true, attributeFilter: ['class'] });
    }
}

// Initialize all functionality
function init() {
    initPageNavigation();
    initSidebarToggle();
    initSidebarThemeToggle();
    initResizeHandler();
    loadDashboardStats();
    setupDashboardTodos();
    loadDashboardTodos();
    
    // Refresh stats periodically
    setInterval(loadDashboardStats, 60000);
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
