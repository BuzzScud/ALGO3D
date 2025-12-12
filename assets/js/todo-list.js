// Todo List Management - Enhanced Version with 3D Visualization
const TodoModule = (function() {
    // State
    let currentFilter = 'all';
    let currentPriority = 'all';
    let currentCategory = 'all';
    let currentTimeFilter = 'all';
    let currentSort = 'priority';
    let currentListView = 'list';
    let searchQuery = '';
    let allTodos = [];
    let contributions = [];
    let scene, camera, renderer, controls;
    let contributionBars = [];
    let animationId = null;
    let isEditing = false;
    let currentView = '3d';
    let marketTimeInterval = null;

    // DOM Elements cache
    const elements = {};

    // Initialize the module
    function init() {
        cacheElements();
        if (!elements.todoList) return;
        
        setupEventListeners();
        initMarketTime();
        fetchTodos();
        fetchStats();
        fetchContributions();
        watchPageActivation();
        
        console.log('Todo Module initialized');
    }
    
    // Initialize market time display
    function initMarketTime() {
        updateMarketTime();
        marketTimeInterval = setInterval(updateMarketTime, 1000);
    }
    
    // Update market time and status
    function updateMarketTime() {
        if (!elements.currentTimeDisplay || !elements.marketStatus) return;
        
        const now = new Date();
        const hours = String(now.getHours()).padStart(2, '0');
        const minutes = String(now.getMinutes()).padStart(2, '0');
        const seconds = String(now.getSeconds()).padStart(2, '0');
        
        elements.currentTimeDisplay.textContent = `${hours}:${minutes}:${seconds}`;
        
        // Get ET time (simplified - in production, use proper timezone conversion)
        const etHour = now.getHours(); // Adjust for ET if needed
        const day = now.getDay(); // 0 = Sunday, 6 = Saturday
        const isWeekday = day >= 1 && day <= 5;
        
        // Trading sessions (ET)
        let session = 'Pre-Market';
        let sessionTime = '4:00 AM - 9:30 AM ET';
        let isMarketOpen = false;
        let progress = 0;
        let countdown = '';
        
        if (isWeekday) {
            if (etHour >= 4 && etHour < 9) {
                session = 'Pre-Market';
                sessionTime = '4:00 AM - 9:30 AM ET';
                progress = ((etHour - 4) * 60 + now.getMinutes()) / 330 * 100;
                const minutesToOpen = (9 * 60 + 30) - (etHour * 60 + now.getMinutes());
                countdown = `${Math.floor(minutesToOpen / 60)}h ${minutesToOpen % 60}m to open`;
            } else if (etHour >= 9 && (etHour < 16 || (etHour === 9 && now.getMinutes() >= 30))) {
                session = 'Regular Trading';
                sessionTime = '9:30 AM - 4:00 PM ET';
                isMarketOpen = true;
                const sessionStart = 9 * 60 + 30;
                const sessionEnd = 16 * 60;
                const current = etHour * 60 + now.getMinutes();
                progress = ((current - sessionStart) / (sessionEnd - sessionStart)) * 100;
                const minutesToClose = sessionEnd - current;
                countdown = `${Math.floor(minutesToClose / 60)}h ${minutesToClose % 60}m to close`;
            } else if (etHour >= 16 && etHour < 20) {
                session = 'After Hours';
                sessionTime = '4:00 PM - 8:00 PM ET';
                progress = ((etHour - 16) * 60 + now.getMinutes()) / 240 * 100;
                countdown = 'Extended hours';
            } else {
                session = 'Market Closed';
                sessionTime = 'Closed until 4:00 AM ET';
                const minutesToOpen = (24 - etHour + 4) * 60 - now.getMinutes();
                countdown = `${Math.floor(minutesToOpen / 60)}h ${minutesToOpen % 60}m to pre-market`;
            }
        } else {
            session = 'Weekend';
            sessionTime = 'Market Closed';
            countdown = 'Closed for weekend';
        }
        
        if (elements.sessionInfo) elements.sessionInfo.textContent = session;
        if (elements.sessionTime) elements.sessionTime.textContent = sessionTime;
        if (elements.sessionProgress) elements.sessionProgress.style.width = Math.min(100, Math.max(0, progress)) + '%';
        if (elements.marketCountdown) elements.marketCountdown.textContent = countdown;
        
        if (isMarketOpen) {
            elements.marketStatus.textContent = 'Market Open';
            elements.marketStatus.classList.add('open');
        } else {
            elements.marketStatus.textContent = 'Market Closed';
            elements.marketStatus.classList.remove('open');
        }
    }

    // Cache DOM elements
    function cacheElements() {
        elements.todoList = document.getElementById('todo-list');
        elements.todoSearch = document.getElementById('todo-search');
        elements.searchClear = document.getElementById('search-clear');
        elements.quickInput = document.getElementById('todo-quick-input');
        elements.quickAddBtn = document.getElementById('quick-add-btn');
        elements.openModalBtn = document.getElementById('open-add-todo-modal');
        elements.modal = document.getElementById('todo-modal');
        elements.closeModalBtn = document.getElementById('close-todo-modal');
        elements.cancelBtn = document.getElementById('cancel-todo-btn');
        elements.saveBtn = document.getElementById('save-todo-btn');
        elements.modalTitle = document.getElementById('todo-modal-title');
        elements.taskInput = document.getElementById('todo-task-input');
        elements.editId = document.getElementById('edit-todo-id');
        elements.categorySelect = document.getElementById('todo-category');
        elements.dueDateInput = document.getElementById('todo-due-date');
        elements.repeatSelect = document.getElementById('todo-repeat');
        elements.notesInput = document.getElementById('todo-notes');
        elements.canvas3d = document.getElementById('todo-3d-canvas');
        elements.vizContainer = document.getElementById('todo-3d-container');
        elements.vizLoading = document.getElementById('viz-loading');
        elements.todoSort = document.getElementById('todo-sort');
        elements.currentTimeDisplay = document.getElementById('current-time-display');
        elements.marketStatus = document.getElementById('market-status');
        elements.marketCountdown = document.getElementById('market-countdown');
        elements.sessionInfo = document.getElementById('session-info');
        elements.sessionTime = document.getElementById('session-time');
        elements.sessionProgress = document.getElementById('session-progress');
        elements.statTodayCompleted = document.getElementById('stat-today-completed');
        elements.statTodayTotal = document.getElementById('stat-today-total');
        elements.productivityScore = document.getElementById('productivity-score');
        elements.scoreCircle = document.getElementById('score-circle');
        elements.scoreText = document.getElementById('score-text');
        elements.vizCompleted = document.getElementById('viz-completed');
        elements.vizActive = document.getElementById('viz-active');
        elements.vizRate = document.getElementById('viz-rate');
        elements.analyticsWeekly = document.getElementById('analytics-weekly');
        elements.analyticsStreak = document.getElementById('analytics-streak');
        elements.analyticsAvg = document.getElementById('analytics-avg');
        elements.rotateToggle = document.getElementById('rotate-toggle');
        elements.resetView = document.getElementById('reset-view');
    }

    // Setup all event listeners
    function setupEventListeners() {
        // Enhanced filter buttons (new design)
        document.querySelectorAll('.filter-btn[data-filter]').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.filter-btn[data-filter]').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentFilter = this.dataset.filter;
                filterAndDisplayTodos();
            });
        });
        
        // Priority filter buttons (new design)
        document.querySelectorAll('.filter-btn[data-priority]').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.filter-btn[data-priority]').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentPriority = this.dataset.priority;
                filterAndDisplayTodos();
            });
        });
        
        // View toggle buttons
        document.querySelectorAll('.view-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.view-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentListView = this.dataset.view;
                filterAndDisplayTodos();
            });
        });
        
        // 3D visualization controls
        if (elements.rotateToggle) {
            elements.rotateToggle.addEventListener('click', function() {
                autoRotate = !autoRotate;
                this.classList.toggle('active', autoRotate);
            });
        }
        
        if (elements.resetView) {
            elements.resetView.addEventListener('click', function() {
                if (camera && controls) {
                    camera.position.set(20, 15, 25);
                    camera.lookAt(0, 0, 0);
                    if (controls.target) {
                        controls.target.set(0, 0, 0);
                        controls.update();
                    }
                }
            });
        }
        
        // Status filter buttons (legacy support)
        document.querySelectorAll('.status-filter-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.status-filter-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentFilter = this.dataset.filter;
                filterAndDisplayTodos();
            });
        });

        // Priority filter buttons
        document.querySelectorAll('.priority-filter-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.priority-filter-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentPriority = this.dataset.priority;
                filterAndDisplayTodos();
            });
        });

        // Category filter buttons
        document.querySelectorAll('.category-filter-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.category-filter-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentCategory = this.dataset.category;
                filterAndDisplayTodos();
            });
        });

        // Time filter buttons
        document.querySelectorAll('.time-filter-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.time-filter-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentTimeFilter = this.dataset.time;
                filterAndDisplayTodos();
            });
        });

        // Search input
        if (elements.todoSearch) {
            elements.todoSearch.addEventListener('input', debounce(function() {
                searchQuery = this.value.trim().toLowerCase();
                if (elements.searchClear) {
                    elements.searchClear.style.display = searchQuery ? 'block' : 'none';
                }
                filterAndDisplayTodos();
            }, 300));
        }

        // Search clear button
        if (elements.searchClear) {
            elements.searchClear.addEventListener('click', function() {
                if (elements.todoSearch) {
                    elements.todoSearch.value = '';
                    searchQuery = '';
                    this.style.display = 'none';
                    filterAndDisplayTodos();
                }
            });
        }

        // Sort select
        if (elements.todoSort) {
            elements.todoSort.addEventListener('change', function() {
                currentSort = this.value;
                filterAndDisplayTodos();
            });
        }

        // View toggle buttons
        document.querySelectorAll('.view-toggle-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.view-toggle-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentListView = this.dataset.view;
                filterAndDisplayTodos();
            });
        });

        // Quick add input
        if (elements.quickInput) {
            elements.quickInput.addEventListener('keypress', function(e) {
                if (e.key === 'Enter' && this.value.trim()) {
                    quickAddTodo(this.value.trim());
                    this.value = '';
                }
            });
        }

        // Quick add button
        if (elements.quickAddBtn) {
            elements.quickAddBtn.addEventListener('click', function() {
                if (elements.quickInput && elements.quickInput.value.trim()) {
                    quickAddTodo(elements.quickInput.value.trim());
                    elements.quickInput.value = '';
                }
            });
        }
        
        // Add todo button (enhanced input)
        const addTodoBtn = document.getElementById('add-todo-btn');
        const todoInput = document.getElementById('todo-input');
        const todoType = document.getElementById('todo-type');
        const todoPriority = document.getElementById('todo-priority');
        
        if (addTodoBtn && todoInput) {
            addTodoBtn.addEventListener('click', function() {
                const text = todoInput.value.trim();
                if (text) {
                    const type = todoType ? todoType.value : 'daily';
                    const priority = todoPriority ? todoPriority.value : 'medium';
                    addTodo(text, type, priority);
                    todoInput.value = '';
                }
            });
        }
        
        if (todoInput) {
            todoInput.addEventListener('keypress', function(e) {
                if (e.key === 'Enter' && this.value.trim()) {
                    const type = todoType ? todoType.value : 'daily';
                    const priority = todoPriority ? todoPriority.value : 'medium';
                    addTodo(this.value.trim(), type, priority);
                    this.value = '';
                }
            });
        }

        // Modal controls
        if (elements.openModalBtn) {
            elements.openModalBtn.addEventListener('click', () => openModal());
        }
        if (elements.closeModalBtn) {
            elements.closeModalBtn.addEventListener('click', closeModal);
        }
        if (elements.cancelBtn) {
            elements.cancelBtn.addEventListener('click', closeModal);
        }
        if (elements.saveBtn) {
            elements.saveBtn.addEventListener('click', saveTodo);
        }

        // Close modal on outside click
        if (elements.modal) {
            elements.modal.addEventListener('click', function(e) {
                if (e.target === this) {
                    closeModal();
                }
            });
        }

        // Visualization view toggle
        document.querySelectorAll('.viz-control-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                document.querySelectorAll('.viz-control-btn').forEach(b => b.classList.remove('active'));
                this.classList.add('active');
                currentView = this.dataset.view;
                renderVisualization();
            });
        });

        // Keyboard shortcuts
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape' && elements.modal?.classList.contains('active')) {
                closeModal();
            }
        });
    }

    // Debounce utility
    function debounce(func, wait) {
        let timeout;
        return function(...args) {
            clearTimeout(timeout);
            timeout = setTimeout(() => func.apply(this, args), wait);
        };
    }

    // Fetch todos from API
    async function fetchTodos() {
        if (!elements.todoList) return;

        try {
            const response = await fetch('api/todos.php?action=list');
            const result = await response.json();

            if (result.success) {
                allTodos = result.todos || [];
                filterAndDisplayTodos();
            } else {
                showError(result.message || 'Failed to fetch todos');
            }
        } catch (error) {
            console.error('Error fetching todos:', error);
            showError('Failed to load tasks. Please try again.');
        }
    }

    // Fetch stats
    async function fetchStats() {
        try {
            const response = await fetch('api/todos.php?action=stats');
            const result = await response.json();

            if (result.success && result.stats) {
                updateStats(result.stats);
            }
        } catch (error) {
            console.error('Error fetching stats:', error);
        }
    }

    // Fetch contributions for visualization
    async function fetchContributions() {
        try {
            const response = await fetch('api/todos.php?action=contributions');
            const result = await response.json();

            if (result.success) {
                contributions = result.contributions || [];
                renderVisualization();
            }
        } catch (error) {
            console.error('Error fetching contributions:', error);
        }
    }

    // Update stats display
    function updateStats(stats) {
        updateElement('stat-total-tasks', stats.total);
        updateElement('stat-active-tasks', stats.active);
        updateElement('stat-active-tasks-sidebar', stats.active);
        updateElement('stat-completed-tasks', stats.completed);
        updateElement('stat-completed-tasks-sidebar', stats.completed);
        updateElement('stat-completion-rate', stats.completionRate + '%');
        updateElement('weekly-completed', stats.weeklyCompleted);
        updateElement('monthly-completed', stats.monthlyCompleted);
        updateElement('best-streak', stats.bestStreak);
        
        // Calculate additional stats
        const highPriority = allTodos.filter(t => !t.completed && t.priority === 'high').length;
        const today = new Date().toISOString().split('T')[0];
        const todayTasks = allTodos.filter(t => !t.completed && t.due_date === today).length;
        
        updateElement('stat-high-priority', highPriority);
        updateElement('stat-today-tasks', todayTasks);
        
        // Update new trading metrics
        updateTradingMetrics(stats);
        updateProductivityScore(stats);
        updateAnalytics(stats);
        updateVizStats(stats);
    }
    
    // Update trading-specific metrics
    function updateTradingMetrics(stats) {
        const today = new Date().toISOString().split('T')[0];
        const todayTodos = allTodos.filter(t => {
            const todoDate = t.completed_at ? t.completed_at.split('T')[0] : null;
            return todoDate === today && t.completed;
        });
        
        const todayTotal = allTodos.filter(t => {
            const todoDate = t.created_at ? t.created_at.split('T')[0] : null;
            return todoDate === today;
        }).length;
        
        if (elements.statTodayCompleted) {
            elements.statTodayCompleted.textContent = todayTodos.length;
        }
        if (elements.statTodayTotal) {
            elements.statTodayTotal.textContent = todayTotal;
        }
    }
    
    // Update productivity score
    function updateProductivityScore(stats) {
        if (!elements.scoreCircle || !elements.scoreText) return;
        
        const completionRate = stats.completionRate || 0;
        const score = Math.min(100, Math.max(0, completionRate));
        
        // Update circle progress
        const circumference = 2 * Math.PI * 45;
        const offset = circumference - (score / 100) * circumference;
        elements.scoreCircle.style.strokeDashoffset = offset;
        
        // Update text
        elements.scoreText.textContent = Math.round(score) + '%';
        
        // Change color based on score
        if (score >= 80) {
            elements.scoreCircle.style.stroke = 'var(--success-color)';
        } else if (score >= 50) {
            elements.scoreCircle.style.stroke = 'var(--warning-color)';
        } else {
            elements.scoreCircle.style.stroke = 'var(--danger-color)';
        }
    }
    
    // Update analytics
    function updateAnalytics(stats) {
        if (elements.analyticsWeekly) {
            elements.analyticsWeekly.textContent = stats.weeklyCompleted || 0;
        }
        if (elements.analyticsStreak) {
            elements.analyticsStreak.textContent = stats.bestStreak || 0;
        }
        if (elements.analyticsAvg) {
            const avg = stats.weeklyCompleted ? Math.round(stats.weeklyCompleted / 7) : 0;
            elements.analyticsAvg.textContent = avg;
        }
    }
    
    // Update visualization stats
    function updateVizStats(stats) {
        if (elements.vizCompleted) {
            elements.vizCompleted.textContent = stats.completed || 0;
        }
        if (elements.vizActive) {
            elements.vizActive.textContent = stats.active || 0;
        }
        if (elements.vizRate) {
            elements.vizRate.textContent = (stats.completionRate || 0) + '%';
        }
    }

    function updateElement(id, value) {
        const el = document.getElementById(id);
        if (el) el.textContent = value;
    }

    // Filter and display todos
    function filterAndDisplayTodos() {
        let filtered = [...allTodos];

        // Apply status filter
        if (currentFilter === 'active') {
            filtered = filtered.filter(t => !t.completed);
        } else if (currentFilter === 'completed') {
            filtered = filtered.filter(t => t.completed);
        }

        // Apply priority filter
        if (currentPriority !== 'all') {
            filtered = filtered.filter(t => t.priority === currentPriority);
        }

        // Apply category filter
        if (currentCategory !== 'all') {
            filtered = filtered.filter(t => t.category === currentCategory);
        }

        // Apply time filter
        const now = new Date();
        const today = now.toISOString().split('T')[0];
        const weekStart = new Date(now.setDate(now.getDate() - now.getDay()));
        const weekStartStr = weekStart.toISOString().split('T')[0];
        
        if (currentTimeFilter === 'today') {
            filtered = filtered.filter(t => t.due_date === today);
        } else if (currentTimeFilter === 'week') {
            filtered = filtered.filter(t => t.due_date && t.due_date >= weekStartStr);
        } else if (currentTimeFilter === 'overdue') {
            filtered = filtered.filter(t => t.due_date && !t.completed && t.due_date < today);
        }

        // Apply search filter
        if (searchQuery) {
            filtered = filtered.filter(t => 
                t.task.toLowerCase().includes(searchQuery) ||
                (t.notes && t.notes.toLowerCase().includes(searchQuery)) ||
                (t.category && t.category.toLowerCase().includes(searchQuery))
            );
        }

        // Sort todos
        filtered = sortTodos(filtered);

        // Update count
        updateElement('todo-list-count', filtered.length);

        displayTodos(filtered);
    }

    // Sort todos
    function sortTodos(todos) {
        const sorted = [...todos];
        
        switch (currentSort) {
            case 'priority':
                sorted.sort((a, b) => {
                    const priorityOrder = { high: 1, medium: 2, low: 3 };
                    const aPriority = priorityOrder[a.priority || 'low'] || 4;
                    const bPriority = priorityOrder[b.priority || 'low'] || 4;
                    if (aPriority !== bPriority) return aPriority - bPriority;
                    return new Date(b.created_at || 0) - new Date(a.created_at || 0);
                });
                break;
            case 'due_date':
                sorted.sort((a, b) => {
                    if (!a.due_date && !b.due_date) return 0;
                    if (!a.due_date) return 1;
                    if (!b.due_date) return -1;
                    return new Date(a.due_date) - new Date(b.due_date);
                });
                break;
            case 'created':
                sorted.sort((a, b) => {
                    return new Date(b.created_at || 0) - new Date(a.created_at || 0);
                });
                break;
            case 'category':
                sorted.sort((a, b) => {
                    const aCat = a.category || 'general';
                    const bCat = b.category || 'general';
                    return aCat.localeCompare(bCat);
                });
                break;
        }
        
        return sorted;
    }

    // Display todos
    function displayTodos(todos) {
        if (!elements.todoList) return;

        if (todos.length === 0) {
            elements.todoList.innerHTML = `
                <div class="todo-empty-state">
                    <i class="fas fa-clipboard-list"></i>
                    <h3>No tasks found</h3>
                    <p>${searchQuery ? 'Try a different search term' : 'Click "New Task" to create your first task!'}</p>
                </div>
            `;
            return;
        }

        // Apply view mode
        if (currentListView === 'grid') {
            elements.todoList.className = 'todo-list todo-list-grid';
            elements.todoList.style.display = 'grid';
            elements.todoList.style.gridTemplateColumns = 'repeat(auto-fill, minmax(280px, 1fr))';
            elements.todoList.style.gap = '1rem';
        } else if (currentListView === 'compact') {
            elements.todoList.className = 'todo-list todo-list-compact';
            elements.todoList.style.display = 'flex';
            elements.todoList.style.flexDirection = 'column';
            elements.todoList.style.gap = '0.5rem';
        } else {
            elements.todoList.className = 'todo-list todo-list-list';
            elements.todoList.style.display = 'flex';
            elements.todoList.style.flexDirection = 'column';
            elements.todoList.style.gap = '0.75rem';
        }

        elements.todoList.innerHTML = todos.map(todo => createTodoItem(todo)).join('');
        attachTodoListeners();
    }

    // Create todo item HTML
    function createTodoItem(todo) {
        const completedClass = todo.completed ? 'completed' : '';
        const priorityClass = `priority-${todo.priority || 'low'}`;
        const isOverdue = todo.due_date && !todo.completed && new Date(todo.due_date) < new Date();
        const overdueClass = isOverdue ? 'overdue' : '';
        const categoryIcon = getCategoryIcon(todo.category);

        return `
            <div class="todo-item ${completedClass} ${priorityClass} ${overdueClass}" data-id="${todo.id}">
                <div class="todo-item-left">
                    <label class="todo-checkbox-wrapper">
                        <input type="checkbox" class="todo-checkbox" ${todo.completed ? 'checked' : ''} data-id="${todo.id}">
                        <span class="todo-checkmark">
                            <i class="fas fa-check"></i>
                        </span>
                    </label>
                    <div class="todo-content">
                        <div class="todo-header-row">
                            <span class="todo-text">${escapeHtml(todo.task)}</span>
                            <span class="todo-priority-badge ${todo.priority || 'low'}" title="${(todo.priority || 'low').charAt(0).toUpperCase() + (todo.priority || 'low').slice(1)} Priority">
                                ${(todo.priority || 'low').charAt(0).toUpperCase()}
                            </span>
                        </div>
                        <div class="todo-meta">
                            ${todo.category ? `<span class="todo-category-badge category-${todo.category}"><i class="${categoryIcon}"></i> ${todo.category}</span>` : ''}
                            ${todo.due_date ? `<span class="todo-due-date ${isOverdue ? 'overdue' : ''}"><i class="fas fa-calendar"></i> ${formatDate(todo.due_date)}</span>` : ''}
                            ${todo.repeat_type && todo.repeat_type !== 'none' ? `<span class="todo-repeat"><i class="fas fa-redo"></i> ${todo.repeat_type}</span>` : ''}
                        </div>
                        ${todo.notes ? `<div class="todo-notes-preview">${escapeHtml(todo.notes.substring(0, 100))}${todo.notes.length > 100 ? '...' : ''}</div>` : ''}
                    </div>
                </div>
                <div class="todo-item-right">
                    <button class="todo-action-btn edit-btn" data-id="${todo.id}" title="Edit">
                        <i class="fas fa-pen"></i>
                    </button>
                    <button class="todo-action-btn delete-btn" data-id="${todo.id}" title="Delete">
                        <i class="fas fa-trash"></i>
                    </button>
                </div>
            </div>
        `;
    }

    // Get category icon
    function getCategoryIcon(category) {
        const icons = {
            trading: 'fas fa-chart-line',
            research: 'fas fa-search',
            learning: 'fas fa-book',
            personal: 'fas fa-user',
            general: 'fas fa-circle'
        };
        return icons[category] || icons.general;
    }

    // Attach event listeners to todo items
    function attachTodoListeners() {
        // Checkbox listeners
        document.querySelectorAll('.todo-checkbox').forEach(checkbox => {
            checkbox.addEventListener('change', async function() {
                const id = parseInt(this.dataset.id);
                await toggleComplete(id, this.checked);
            });
        });

        // Edit button listeners
        document.querySelectorAll('.edit-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                const id = parseInt(this.dataset.id);
                editTodo(id);
            });
        });

        // Delete button listeners
        document.querySelectorAll('.delete-btn').forEach(btn => {
            btn.addEventListener('click', function() {
                const id = parseInt(this.dataset.id);
                deleteTodo(id);
            });
        });
    }

    // Quick add todo
    async function quickAddTodo(task) {
        try {
            const response = await fetch('api/todos.php', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ task, priority: 'low', category: 'general' })
            });

            const result = await response.json();
            if (result.success) {
                await fetchTodos();
                await fetchStats();
                await fetchContributions();
                showNotification('Task added!', 'success');
            } else {
                showNotification(result.message || 'Error adding task', 'error');
            }
        } catch (error) {
            console.error('Error adding todo:', error);
            showNotification('Error adding task', 'error');
        }
    }
    
    async function addTodo(task, type = 'daily', priority = 'medium') {
        try {
            const response = await fetch('api/todos.php', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    task, 
                    type,
                    priority: priority || 'medium',
                    category: 'trading'
                })
            });

            const result = await response.json();
            if (result.success) {
                await fetchTodos();
                await fetchStats();
                await fetchContributions();
                showNotification('Task added!', 'success');
            } else {
                showNotification(result.message || 'Error adding task', 'error');
            }
        } catch (error) {
            console.error('Error adding todo:', error);
            showNotification('Error adding task', 'error');
        }
    }

    // Toggle complete status
    async function toggleComplete(id, completed) {
        try {
            const response = await fetch('api/todos.php', {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, completed })
            });

            const result = await response.json();
            if (result.success) {
                // Update local state
                const todo = allTodos.find(t => t.id === id);
                if (todo) todo.completed = completed;
                
                filterAndDisplayTodos();
                await fetchStats();
                await fetchContributions();
                
                if (completed) {
                    showNotification('Task completed! 🎉', 'success');
                }
            }
        } catch (error) {
            console.error('Error updating todo:', error);
        }
    }

    // Open modal for new/edit
    function openModal(todo = null) {
        isEditing = !!todo;
        
        if (elements.modalTitle) {
            elements.modalTitle.innerHTML = isEditing 
                ? '<i class="fas fa-edit"></i> Edit Task' 
                : '<i class="fas fa-plus-circle"></i> New Task';
        }

        // Reset form
        if (elements.taskInput) elements.taskInput.value = todo?.task || '';
        if (elements.editId) elements.editId.value = todo?.id || '';
        if (elements.categorySelect) elements.categorySelect.value = todo?.category || 'general';
        if (elements.dueDateInput) elements.dueDateInput.value = todo?.due_date || '';
        if (elements.repeatSelect) elements.repeatSelect.value = todo?.repeat_type || 'none';
        if (elements.notesInput) elements.notesInput.value = todo?.notes || '';

        // Set priority
        const priority = todo?.priority || 'low';
        document.querySelectorAll('input[name="todo-priority"]').forEach(radio => {
            radio.checked = radio.value === priority;
        });

        if (elements.modal) {
            elements.modal.classList.add('active');
            elements.taskInput?.focus();
        }
    }

    // Close modal
    function closeModal() {
        if (elements.modal) {
            elements.modal.classList.remove('active');
        }
        isEditing = false;
    }

    // Edit todo
    function editTodo(id) {
        const todo = allTodos.find(t => t.id === id);
        if (todo) {
            openModal(todo);
        }
    }

    // Save todo (create or update)
    async function saveTodo() {
        const task = elements.taskInput?.value.trim();
        if (!task) {
            showNotification('Please enter a task description', 'error');
            return;
        }

        const priority = document.querySelector('input[name="todo-priority"]:checked')?.value || 'low';
        const category = elements.categorySelect?.value || 'general';
        const dueDate = elements.dueDateInput?.value || null;
        const repeatType = elements.repeatSelect?.value || 'none';
        const notes = elements.notesInput?.value.trim() || '';

        const data = {
            task,
            priority,
            category,
            due_date: dueDate,
            repeat_type: repeatType,
            notes
        };

        try {
            let response;
            if (isEditing && elements.editId?.value) {
                data.id = parseInt(elements.editId.value);
                response = await fetch('api/todos.php', {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                });
            } else {
                response = await fetch('api/todos.php', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(data)
                });
            }

            const result = await response.json();
            if (result.success) {
                closeModal();
                await fetchTodos();
                await fetchStats();
                showNotification(isEditing ? 'Task updated!' : 'Task created!', 'success');
            } else {
                showNotification(result.message || 'Error saving task', 'error');
            }
        } catch (error) {
            console.error('Error saving todo:', error);
            showNotification('Error saving task', 'error');
        }
    }

    // Delete todo
    async function deleteTodo(id) {
        if (!confirm('Are you sure you want to delete this task?')) return;

        try {
            const response = await fetch('api/todos.php', {
                method: 'DELETE',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id })
            });

            const result = await response.json();
            if (result.success) {
                await fetchTodos();
                await fetchStats();
                await fetchContributions();
                showNotification('Task deleted', 'success');
            }
        } catch (error) {
            console.error('Error deleting todo:', error);
            showNotification('Error deleting task', 'error');
        }
    }

    // 3D Visualization Functions
    function renderVisualization() {
        if (currentView === '3d') {
            render3DVisualization();
        } else {
            renderChartVisualization();
        }
    }

    function render3DVisualization() {
        if (!elements.canvas3d || !elements.vizContainer) return;
        if (elements.vizLoading) elements.vizLoading.style.display = 'flex';

        // Clean up existing scene
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }
        if (renderer) {
            renderer.dispose();
        }

        // Check if Three.js is available
        if (typeof THREE === 'undefined') {
            // Load Three.js dynamically
            loadThreeJs().then(() => {
                initThreeScene();
            }).catch(() => {
                // Fallback to chart view
                renderChartVisualization();
            });
        } else {
            initThreeScene();
        }
    }

    function loadThreeJs() {
        return new Promise((resolve, reject) => {
            if (typeof THREE !== 'undefined') {
                resolve();
                return;
            }

            const script = document.createElement('script');
            script.src = 'https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js';
            script.onload = resolve;
            script.onerror = reject;
            document.head.appendChild(script);
        });
    }

    function initThreeScene() {
        const container = elements.vizContainer;
        const canvas = elements.canvas3d;
        
        const width = container.clientWidth;
        const height = container.clientHeight || 300;

        // Scene setup
        scene = new THREE.Scene();
        scene.background = new THREE.Color(0x0f172a);

        // Camera
        camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 1000);
        camera.position.set(20, 15, 25);
        camera.lookAt(0, 0, 0);

        // Renderer
        renderer = new THREE.WebGLRenderer({ 
            canvas: canvas, 
            antialias: true,
            alpha: true 
        });
        renderer.setSize(width, height);
        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));

        // Lighting
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
        scene.add(ambientLight);

        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(10, 20, 10);
        scene.add(directionalLight);

        // Create contribution bars
        createContributionBars();

        // Add grid floor
        const gridHelper = new THREE.GridHelper(30, 30, 0x3b82f6, 0x1e293b);
        gridHelper.position.y = -0.01;
        scene.add(gridHelper);

        // Hide loading
        if (elements.vizLoading) elements.vizLoading.style.display = 'none';

        // Animation
        let autoRotate = true;
        let isDragging = false;
        let previousMousePosition = { x: 0, y: 0 };

        canvas.addEventListener('mousedown', () => { isDragging = true; autoRotate = false; });
        canvas.addEventListener('mouseup', () => { isDragging = false; });
        canvas.addEventListener('mouseleave', () => { isDragging = false; });
        canvas.addEventListener('mousemove', (e) => {
            if (isDragging) {
                const deltaX = e.clientX - previousMousePosition.x;
                const deltaY = e.clientY - previousMousePosition.y;
                
                // Rotate camera around scene
                const rotationSpeed = 0.01;
                const theta = deltaX * rotationSpeed;
                
                camera.position.x = camera.position.x * Math.cos(theta) + camera.position.z * Math.sin(theta);
                camera.position.z = camera.position.z * Math.cos(theta) - camera.position.x * Math.sin(theta);
                camera.lookAt(0, 0, 0);
            }
            previousMousePosition = { x: e.clientX, y: e.clientY };
        });

        function animate() {
            animationId = requestAnimationFrame(animate);

            if (autoRotate) {
                camera.position.x = camera.position.x * Math.cos(0.002) + camera.position.z * Math.sin(0.002);
                camera.position.z = camera.position.z * Math.cos(0.002) - camera.position.x * Math.sin(0.002);
                camera.lookAt(0, 0, 0);
            }

            // Animate bars
            contributionBars.forEach((bar, i) => {
                if (bar.userData.targetHeight !== undefined) {
                    bar.scale.y += (bar.userData.targetHeight - bar.scale.y) * 0.1;
                    bar.position.y = bar.scale.y / 2;
                }
            });

            renderer.render(scene, camera);
        }
        animate();

        // Handle resize
        const resizeObserver = new ResizeObserver(() => {
            const newWidth = container.clientWidth;
            const newHeight = container.clientHeight || 300;
            camera.aspect = newWidth / newHeight;
            camera.updateProjectionMatrix();
            renderer.setSize(newWidth, newHeight);
        });
        resizeObserver.observe(container);
    }

    function createContributionBars() {
        // Clear existing bars
        contributionBars.forEach(bar => scene.remove(bar));
        contributionBars = [];

        if (!contributions.length) return;

        // Create 13 weeks x 7 days grid
        const weeksToShow = 13;
        const daysPerWeek = 7;
        const barSize = 0.8;
        const gap = 0.2;
        const totalWidth = (barSize + gap) * weeksToShow;
        const totalDepth = (barSize + gap) * daysPerWeek;

        // Get last 91 days
        const recentContributions = contributions.slice(-91);

        recentContributions.forEach((contrib, index) => {
            const weekIndex = Math.floor(index / 7);
            const dayIndex = index % 7;

            const x = weekIndex * (barSize + gap) - totalWidth / 2;
            const z = dayIndex * (barSize + gap) - totalDepth / 2;

            // Determine height and color based on activity
            const activity = contrib.completed;
            const maxHeight = 5;
            const height = Math.max(0.1, (activity / 5) * maxHeight);

            // Color based on activity level
            let color;
            if (activity === 0) {
                color = 0x1e293b; // Dark - no activity
            } else if (activity <= 1) {
                color = 0x166534; // Low - dark green
            } else if (activity <= 3) {
                color = 0x22c55e; // Medium - green
            } else {
                color = 0x4ade80; // High - bright green
            }

            const geometry = new THREE.BoxGeometry(barSize, 1, barSize);
            const material = new THREE.MeshLambertMaterial({ color });
            const bar = new THREE.Mesh(geometry, material);

            bar.position.set(x, 0.05, z);
            bar.scale.y = 0.1;
            bar.userData.targetHeight = height;
            bar.userData.contribution = contrib;

            scene.add(bar);
            contributionBars.push(bar);
        });
    }

    function renderChartVisualization() {
        if (!elements.vizContainer) return;
        if (elements.vizLoading) elements.vizLoading.style.display = 'none';

        // Create a simple canvas-based chart
        const canvas = elements.canvas3d;
        if (!canvas) return;

        const ctx = canvas.getContext('2d');
        const width = elements.vizContainer.clientWidth;
        const height = elements.vizContainer.clientHeight || 300;

        canvas.width = width;
        canvas.height = height;

        // Clear canvas
        ctx.fillStyle = '#0f172a';
        ctx.fillRect(0, 0, width, height);

        if (!contributions.length) {
            ctx.fillStyle = '#64748b';
            ctx.font = '14px Inter, sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('No contribution data available', width / 2, height / 2);
            return;
        }

        // Draw contribution grid
        const weeks = 13;
        const days = 7;
        const cellSize = Math.min((width - 80) / weeks, (height - 60) / days);
        const gap = 2;
        const startX = (width - (weeks * (cellSize + gap))) / 2;
        const startY = 40;

        const recentContributions = contributions.slice(-91);

        // Day labels
        const dayLabels = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
        ctx.fillStyle = '#64748b';
        ctx.font = '10px Inter, sans-serif';
        ctx.textAlign = 'right';
        
        for (let d = 0; d < days; d++) {
            ctx.fillText(dayLabels[d], startX - 8, startY + d * (cellSize + gap) + cellSize / 2 + 3);
        }

        // Draw cells
        recentContributions.forEach((contrib, index) => {
            const weekIndex = Math.floor(index / 7);
            const dayIndex = index % 7;

            const x = startX + weekIndex * (cellSize + gap);
            const y = startY + dayIndex * (cellSize + gap);

            // Color based on activity
            let color;
            if (contrib.completed === 0) {
                color = '#1e293b';
            } else if (contrib.completed <= 1) {
                color = '#166534';
            } else if (contrib.completed <= 3) {
                color = '#22c55e';
            } else {
                color = '#4ade80';
            }

            ctx.fillStyle = color;
            ctx.beginPath();
            ctx.roundRect(x, y, cellSize, cellSize, 2);
            ctx.fill();
        });

        // Title
        ctx.fillStyle = '#e2e8f0';
        ctx.font = 'bold 12px Inter, sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('Last 13 Weeks Activity', width / 2, 20);
    }

    // Utility functions
    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    function formatDate(dateStr) {
        if (!dateStr) return '';
        const date = new Date(dateStr);
        const today = new Date();
        const tomorrow = new Date(today);
        tomorrow.setDate(tomorrow.getDate() + 1);

        if (date.toDateString() === today.toDateString()) return 'Today';
        if (date.toDateString() === tomorrow.toDateString()) return 'Tomorrow';

        return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
    }

    function showError(message) {
        if (elements.todoList) {
            elements.todoList.innerHTML = `
                <div class="todo-error-state">
                    <i class="fas fa-exclamation-triangle"></i>
                    <p>${message}</p>
                    <button class="btn btn-secondary btn-sm" onclick="TodoModule.refresh()">
                        <i class="fas fa-redo"></i> Retry
                    </button>
                </div>
            `;
        }
    }

    function showNotification(message, type = 'info') {
        const notification = document.createElement('div');
        notification.className = `notification notification-${type}`;
        notification.innerHTML = `
            <i class="fas fa-${type === 'success' ? 'check-circle' : type === 'error' ? 'exclamation-circle' : 'info-circle'}"></i>
            <span>${message}</span>
        `;
        notification.style.cssText = `
            position: fixed;
            top: 80px;
            right: 20px;
            padding: 12px 20px;
            background: ${type === 'success' ? '#22c55e' : type === 'error' ? '#ef4444' : '#3b82f6'};
            color: white;
            border-radius: 8px;
            z-index: 10000;
            display: flex;
            align-items: center;
            gap: 8px;
            animation: slideIn 0.3s ease;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        `;
        document.body.appendChild(notification);

        setTimeout(() => {
            notification.style.animation = 'slideOut 0.3s ease';
            setTimeout(() => notification.remove(), 300);
        }, 3000);
    }

    // Watch for page activation
    function watchPageActivation() {
        const todosPage = document.getElementById('page-todo');
        if (!todosPage) return;

        const observer = new MutationObserver((mutations) => {
            mutations.forEach((mutation) => {
                if (mutation.type === 'attributes' && mutation.attributeName === 'class') {
                    if (todosPage.classList.contains('active')) {
                        fetchTodos();
                        fetchStats();
                        fetchContributions();
                    }
                }
            });
        });

        observer.observe(todosPage, {
            attributes: true,
            attributeFilter: ['class']
        });
    }

    // Public API
    function refresh() {
        fetchTodos();
        fetchStats();
        fetchContributions();
    }

    // Cleanup
    function cleanup() {
        if (marketTimeInterval) {
            clearInterval(marketTimeInterval);
            marketTimeInterval = null;
        }
        if (animationId) {
            cancelAnimationFrame(animationId);
            animationId = null;
        }
    }

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }

    return {
        init,
        refresh,
        fetchTodos,
        fetchStats,
        cleanup
    };
})();

// Expose globally for onclick handlers
window.TodoModule = TodoModule;

