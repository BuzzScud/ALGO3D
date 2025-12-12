<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    exit(0);
}

require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

// Check and update table structure if needed
checkAndUpdateTable($conn);

$method = $_SERVER['REQUEST_METHOD'];

try {
    switch ($method) {
        case 'GET':
            handleGet($conn);
            break;
        case 'POST':
            handlePost($conn);
            break;
        case 'PUT':
            handlePut($conn);
            break;
        case 'DELETE':
            handleDelete($conn);
            break;
        default:
            echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("Todo API error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
}

function checkAndUpdateTable($conn) {
    try {
        // Check if new columns exist
        $stmt = $conn->query("PRAGMA table_info(todos)");
        $columns = $stmt->fetchAll(PDO::FETCH_COLUMN, 1);
        
        $newColumns = [
            'priority' => "ALTER TABLE todos ADD COLUMN priority VARCHAR(20) DEFAULT 'low'",
            'category' => "ALTER TABLE todos ADD COLUMN category VARCHAR(50) DEFAULT 'general'",
            'due_date' => "ALTER TABLE todos ADD COLUMN due_date DATE NULL",
            'notes' => "ALTER TABLE todos ADD COLUMN notes TEXT NULL",
            'repeat_type' => "ALTER TABLE todos ADD COLUMN repeat_type VARCHAR(20) DEFAULT 'none'",
            'completed_at' => "ALTER TABLE todos ADD COLUMN completed_at DATETIME NULL"
        ];
        
        foreach ($newColumns as $col => $sql) {
            if (!in_array($col, $columns)) {
                $conn->exec($sql);
            }
        }
    } catch (PDOException $e) {
        // Columns might already exist or table structure is different
        error_log("Table update notice: " . $e->getMessage());
    }
}

function handleGet($conn) {
    $action = $_GET['action'] ?? 'list';
    
    if ($action === 'stats') {
        getStats($conn);
        return;
    }
    
    if ($action === 'contributions') {
        getContributions($conn);
        return;
    }
    
    $filter = $_GET['filter'] ?? 'all';
    $priority = $_GET['priority'] ?? 'all';
    $search = $_GET['search'] ?? '';
    
    $query = "SELECT * FROM todos WHERE 1=1";
    $params = [];
    
    // Status filter
    if ($filter === 'active') {
        $query .= " AND completed = 0";
    } elseif ($filter === 'completed') {
        $query .= " AND completed = 1";
    }
    
    // Priority filter
    if ($priority !== 'all') {
        $query .= " AND priority = ?";
        $params[] = $priority;
    }
    
    // Search filter
    if (!empty($search)) {
        $query .= " AND (task LIKE ? OR notes LIKE ? OR category LIKE ?)";
        $searchTerm = '%' . $search . '%';
        $params[] = $searchTerm;
        $params[] = $searchTerm;
        $params[] = $searchTerm;
    }
    
    // Order by priority (high first), then by due date, then by created date
    $query .= " ORDER BY 
        CASE priority 
            WHEN 'high' THEN 1 
            WHEN 'medium' THEN 2 
            WHEN 'low' THEN 3 
            ELSE 4 
        END,
        CASE WHEN due_date IS NULL THEN 1 ELSE 0 END,
        due_date ASC,
        created_at DESC";
    
    $stmt = $conn->prepare($query);
    $stmt->execute($params);
    $todos = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Format data for frontend
    foreach ($todos as &$todo) {
        $todo['completed'] = (bool)$todo['completed'];
        $todo['priority'] = $todo['priority'] ?? 'low';
        $todo['category'] = $todo['category'] ?? 'general';
        $todo['repeat_type'] = $todo['repeat_type'] ?? 'none';
    }
    
    echo json_encode(['success' => true, 'todos' => $todos]);
}

function getStats($conn) {
    // Total tasks
    $stmt = $conn->query("SELECT COUNT(*) FROM todos");
    $total = $stmt->fetchColumn();
    
    // Active tasks
    $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 0");
    $active = $stmt->fetchColumn();
    
    // Completed tasks
    $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 1");
    $completed = $stmt->fetchColumn();
    
    // Completion rate
    $rate = $total > 0 ? round(($completed / $total) * 100) : 0;
    
    // Weekly completed (last 7 days)
    $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 1 AND completed_at >= datetime('now', '-7 days')");
    $weeklyCompleted = $stmt->fetchColumn();
    
    // Monthly completed (last 30 days)
    $stmt = $conn->query("SELECT COUNT(*) FROM todos WHERE completed = 1 AND completed_at >= datetime('now', '-30 days')");
    $monthlyCompleted = $stmt->fetchColumn();
    
    // Best streak calculation
    $stmt = $conn->query("
        SELECT DATE(completed_at) as date, COUNT(*) as count 
        FROM todos 
        WHERE completed = 1 AND completed_at IS NOT NULL 
        GROUP BY DATE(completed_at) 
        ORDER BY date DESC
    ");
    $completionDays = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    $bestStreak = 0;
    $currentStreak = 0;
    $prevDate = null;
    
    foreach ($completionDays as $day) {
        if ($prevDate === null) {
            $currentStreak = 1;
        } else {
            $diff = (strtotime($prevDate) - strtotime($day['date'])) / 86400;
            if ($diff == 1) {
                $currentStreak++;
            } else {
                $currentStreak = 1;
            }
        }
        $bestStreak = max($bestStreak, $currentStreak);
        $prevDate = $day['date'];
    }
    
    // Priority breakdown
    $stmt = $conn->query("SELECT priority, COUNT(*) as count FROM todos WHERE completed = 0 GROUP BY priority");
    $priorityBreakdown = $stmt->fetchAll(PDO::FETCH_KEY_PAIR);
    
    echo json_encode([
        'success' => true,
        'stats' => [
            'total' => (int)$total,
            'active' => (int)$active,
            'completed' => (int)$completed,
            'completionRate' => $rate,
            'weeklyCompleted' => (int)$weeklyCompleted,
            'monthlyCompleted' => (int)$monthlyCompleted,
            'bestStreak' => $bestStreak,
            'priorityBreakdown' => $priorityBreakdown
        ]
    ]);
}

function getContributions($conn) {
    // Get daily contribution data for the last 90 days
    $stmt = $conn->query("
        SELECT 
            DATE(COALESCE(completed_at, created_at)) as date,
            SUM(CASE WHEN completed = 1 THEN 1 ELSE 0 END) as completed,
            COUNT(*) as total
        FROM todos 
        WHERE created_at >= datetime('now', '-90 days')
        GROUP BY DATE(COALESCE(completed_at, created_at))
        ORDER BY date ASC
    ");
    $contributions = $stmt->fetchAll(PDO::FETCH_ASSOC);
    
    // Fill in missing days with zeros
    $result = [];
    $startDate = new DateTime('-89 days');
    $endDate = new DateTime();
    
    $contributionMap = [];
    foreach ($contributions as $c) {
        $contributionMap[$c['date']] = [
            'completed' => (int)$c['completed'],
            'total' => (int)$c['total']
        ];
    }
    
    while ($startDate <= $endDate) {
        $dateStr = $startDate->format('Y-m-d');
        $result[] = [
            'date' => $dateStr,
            'completed' => $contributionMap[$dateStr]['completed'] ?? 0,
            'total' => $contributionMap[$dateStr]['total'] ?? 0,
            'dayOfWeek' => (int)$startDate->format('w'),
            'week' => (int)$startDate->format('W')
        ];
        $startDate->modify('+1 day');
    }
    
    echo json_encode([
        'success' => true,
        'contributions' => $result
    ]);
}

function handlePost($conn) {
    $input = json_decode(file_get_contents('php://input'), true);
    
    $task = trim($input['task'] ?? '');
    $priority = $input['priority'] ?? 'low';
    $category = $input['category'] ?? 'general';
    $dueDate = !empty($input['due_date']) ? $input['due_date'] : null;
    $notes = trim($input['notes'] ?? '');
    $repeatType = $input['repeat_type'] ?? 'none';
    
    // Legacy support for 'type' field
    if (isset($input['type']) && !isset($input['repeat_type'])) {
        $repeatType = $input['type'];
    }
    
    if (empty($task)) {
        echo json_encode(['success' => false, 'message' => 'Task description is required']);
        return;
    }
    
    // Map repeat_type to valid 'type' value for database constraint
    // The 'type' column only accepts 'daily' or 'weekly'
    $typeValue = 'daily'; // Default
    if ($repeatType === 'weekly') {
        $typeValue = 'weekly';
    } elseif ($repeatType === 'daily') {
        $typeValue = 'daily';
    }
    // For 'none' and 'monthly', use 'daily' as default (stored in repeat_type for display)
    
    $stmt = $conn->prepare("
        INSERT INTO todos (task, priority, category, due_date, notes, repeat_type, type) 
        VALUES (?, ?, ?, ?, ?, ?, ?)
    ");
    $stmt->execute([$task, $priority, $category, $dueDate, $notes, $repeatType, $typeValue]);
    
    echo json_encode([
        'success' => true, 
        'id' => $conn->lastInsertId(),
        'message' => 'Task created successfully'
    ]);
}

function handlePut($conn) {
    $input = json_decode(file_get_contents('php://input'), true);
    $id = intval($input['id'] ?? 0);
    
    if ($id <= 0) {
        echo json_encode(['success' => false, 'message' => 'Invalid task ID']);
        return;
    }
    
    // Check if this is just a completion toggle
    if (isset($input['completed']) && count($input) <= 2) {
        $completed = (int)$input['completed'];
        $completedAt = $completed ? date('Y-m-d H:i:s') : null;
        
        $stmt = $conn->prepare("
            UPDATE todos 
            SET completed = ?, completed_at = ?, updated_at = CURRENT_TIMESTAMP 
            WHERE id = ?
        ");
        $stmt->execute([$completed, $completedAt, $id]);
        
        echo json_encode(['success' => true, 'message' => 'Task updated']);
        return;
    }
    
    // Full update
    $task = trim($input['task'] ?? '');
    $priority = $input['priority'] ?? null;
    $category = $input['category'] ?? null;
    $dueDate = isset($input['due_date']) ? ($input['due_date'] ?: null) : null;
    $notes = isset($input['notes']) ? trim($input['notes']) : null;
    $repeatType = $input['repeat_type'] ?? null;
    
    $updates = [];
    $params = [];
    
    if (!empty($task)) {
        $updates[] = "task = ?";
        $params[] = $task;
    }
    if ($priority !== null) {
        $updates[] = "priority = ?";
        $params[] = $priority;
    }
    if ($category !== null) {
        $updates[] = "category = ?";
        $params[] = $category;
    }
    if (array_key_exists('due_date', $input)) {
        $updates[] = "due_date = ?";
        $params[] = $dueDate;
    }
    if ($notes !== null) {
        $updates[] = "notes = ?";
        $params[] = $notes;
    }
    if ($repeatType !== null) {
        $updates[] = "repeat_type = ?";
        $params[] = $repeatType;
        
        // Map repeat_type to valid 'type' value for database constraint
        $typeValue = 'daily'; // Default
        if ($repeatType === 'weekly') {
            $typeValue = 'weekly';
        } elseif ($repeatType === 'daily') {
            $typeValue = 'daily';
        }
        
        $updates[] = "type = ?";
        $params[] = $typeValue;
    }
    
    if (empty($updates)) {
        echo json_encode(['success' => false, 'message' => 'No fields to update']);
        return;
    }
    
    $updates[] = "updated_at = CURRENT_TIMESTAMP";
    $params[] = $id;
    
    $sql = "UPDATE todos SET " . implode(', ', $updates) . " WHERE id = ?";
    $stmt = $conn->prepare($sql);
    $stmt->execute($params);
    
    echo json_encode(['success' => true, 'message' => 'Task updated successfully']);
}

function handleDelete($conn) {
    $input = json_decode(file_get_contents('php://input'), true);
    $id = intval($input['id'] ?? 0);
    
    if ($id <= 0) {
        echo json_encode(['success' => false, 'message' => 'Invalid task ID']);
        return;
    }
    
    $stmt = $conn->prepare("DELETE FROM todos WHERE id = ?");
    $stmt->execute([$id]);
    
    echo json_encode(['success' => true, 'message' => 'Task deleted successfully']);
}
?>
