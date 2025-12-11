// Todo List Management
let currentFilter = 'all';

// Fetch todos from API
async function fetchTodos(filter = 'all') {
    try {
        const response = await fetch(`api/todos.php?filter=${filter}`);
        if (!response.ok) throw new Error('Failed to fetch todos');
        
        const todos = await response.json();
        displayTodos(todos);
    } catch (error) {
        console.error('Error fetching todos:', error);
        const todoList = document.getElementById('todo-list');
        if (todoList) {
            todoList.innerHTML = '<div class="loading">Error loading todos. Please try again later.</div>';
        }
    }
}

// Display todos
function displayTodos(todos) {
    const todoList = document.getElementById('todo-list');
    if (!todoList) return;

    if (todos.length === 0) {
        todoList.innerHTML = '<div class="loading">No tasks found. Add a new task to get started!</div>';
        return;
    }

    todoList.innerHTML = todos.map(todo => createTodoItem(todo)).join('');
    
    // Attach event listeners
    attachTodoListeners();
}

// Create todo item HTML
function createTodoItem(todo) {
    const completedClass = todo.completed ? 'completed' : '';
    const checked = todo.completed ? 'checked' : '';
    
    return `
        <div class="todo-item ${completedClass}" data-id="${todo.id}">
            <input type="checkbox" class="todo-checkbox" ${checked} data-id="${todo.id}">
            <span class="todo-text">${escapeHtml(todo.task)}</span>
            <span class="todo-type-badge ${todo.type}">${todo.type}</span>
            <button class="todo-delete-btn" data-id="${todo.id}" title="Delete task">
                <i class="fas fa-trash"></i>
            </button>
        </div>
    `;
}

// Escape HTML to prevent XSS
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Attach event listeners to todo items
function attachTodoListeners() {
    // Checkbox listeners
    document.querySelectorAll('.todo-checkbox').forEach(checkbox => {
        checkbox.addEventListener('change', function() {
            const id = parseInt(this.dataset.id);
            const completed = this.checked;
            updateTodo(id, { completed: completed });
        });
    });

    // Delete button listeners
    document.querySelectorAll('.todo-delete-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            const id = parseInt(this.dataset.id);
            deleteTodo(id);
        });
    });
}

// Add new todo
async function addTodo(task, type) {
    if (!task || task.trim() === '') {
        alert('Please enter a task');
        return;
    }

    try {
        const response = await fetch('api/todos.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ task: task.trim(), type: type })
        });

        const result = await response.json();
        
        if (result.success) {
            // Clear input
            const todoInput = document.getElementById('todo-input');
            if (todoInput) {
                todoInput.value = '';
            }
            // Refresh todos
            await fetchTodos(currentFilter);
        } else {
            alert(result.message || 'Error adding todo');
        }
    } catch (error) {
        console.error('Error adding todo:', error);
        alert('Error adding todo. Please try again.');
    }
}

// Update todo
async function updateTodo(id, updates) {
    try {
        const response = await fetch('api/todos.php', {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ id: id, ...updates })
        });

        const result = await response.json();
        
        if (result.success) {
            // Refresh todos
            await fetchTodos(currentFilter);
        } else {
            alert(result.message || 'Error updating todo');
        }
    } catch (error) {
        console.error('Error updating todo:', error);
        alert('Error updating todo. Please try again.');
    }
}

// Delete todo
async function deleteTodo(id) {
    if (!confirm('Are you sure you want to delete this task?')) {
        return;
    }

    try {
        const response = await fetch('api/todos.php', {
            method: 'DELETE',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ id: id })
        });

        const result = await response.json();
        
        if (result.success) {
            // Refresh todos
            await fetchTodos(currentFilter);
        } else {
            alert(result.message || 'Error deleting todo');
        }
    } catch (error) {
        console.error('Error deleting todo:', error);
        alert('Error deleting todo. Please try again.');
    }
}

// Setup event listeners
function setupTodoListeners() {
    // Add todo button
    const addTodoBtn = document.getElementById('add-todo-btn');
    if (addTodoBtn) {
        addTodoBtn.addEventListener('click', function() {
            const todoInput = document.getElementById('todo-input');
            const todoType = document.getElementById('todo-type');
            
            if (todoInput && todoType) {
                addTodo(todoInput.value, todoType.value);
            }
        });
    }

    // Enter key support
    const todoInput = document.getElementById('todo-input');
    if (todoInput) {
        todoInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                const todoType = document.getElementById('todo-type');
                if (todoType) {
                    addTodo(this.value, todoType.value);
                }
            }
        });
    }

    // Filter buttons
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.addEventListener('click', function() {
            // Update active state
            document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
            this.classList.add('active');
            
            // Update filter
            currentFilter = this.dataset.filter;
            fetchTodos(currentFilter);
        });
    });
}

// Initialize todo list
function initTodoList() {
    fetchTodos(currentFilter);
    setupTodoListeners();
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initTodoList);
} else {
    initTodoList();
}

