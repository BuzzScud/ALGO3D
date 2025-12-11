<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['error' => 'Database connection failed', 'todos' => []]);
    exit;
}

$method = $_SERVER['REQUEST_METHOD'];

try {
    switch ($method) {
        case 'GET':
            $filter = $_GET['filter'] ?? 'all';
            $query = "SELECT * FROM todos";
            
            if ($filter === 'daily') {
                $query .= " WHERE type = 'daily'";
            } elseif ($filter === 'weekly') {
                $query .= " WHERE type = 'weekly'";
            } elseif ($filter === 'completed') {
                $query .= " WHERE completed = 1";
            } elseif ($filter === 'active') {
                $query .= " WHERE completed = 0";
            }
            
            $query .= " ORDER BY created_at DESC";
            
            $stmt = $conn->prepare($query);
            $stmt->execute();
            $todos = $stmt->fetchAll(PDO::FETCH_ASSOC);
            
            // Convert completed to boolean for frontend
            foreach ($todos as &$todo) {
                $todo['completed'] = (bool)$todo['completed'];
            }
            
            echo json_encode($todos);
            break;
            
        case 'POST':
            $input = json_decode(file_get_contents('php://input'), true);
            $task = trim($input['task'] ?? '');
            $type = $input['type'] ?? 'daily';
            
            if (empty($task)) {
                echo json_encode(['success' => false, 'message' => 'Task is required']);
                exit;
            }
            
            $stmt = $conn->prepare("INSERT INTO todos (task, type) VALUES (?, ?)");
            $stmt->execute([$task, $type]);
            
            echo json_encode(['success' => true, 'id' => $conn->lastInsertId()]);
            break;
            
        case 'PUT':
            $input = json_decode(file_get_contents('php://input'), true);
            $id = intval($input['id'] ?? 0);
            $completed = isset($input['completed']) ? (int)$input['completed'] : null;
            $task = trim($input['task'] ?? '');
            
            if ($id > 0) {
                if ($completed !== null) {
                    $stmt = $conn->prepare("UPDATE todos SET completed = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
                    $stmt->execute([$completed, $id]);
                } elseif (!empty($task)) {
                    $stmt = $conn->prepare("UPDATE todos SET task = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
                    $stmt->execute([$task, $id]);
                }
                echo json_encode(['success' => true]);
            } else {
                echo json_encode(['success' => false, 'message' => 'Invalid ID']);
            }
            break;
            
        case 'DELETE':
            $input = json_decode(file_get_contents('php://input'), true);
            $id = intval($input['id'] ?? 0);
            
            if ($id > 0) {
                $stmt = $conn->prepare("DELETE FROM todos WHERE id = ?");
                $stmt->execute([$id]);
                echo json_encode(['success' => true]);
            } else {
                echo json_encode(['success' => false, 'message' => 'Invalid ID']);
            }
            break;
            
        default:
            echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("Todo API error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error']);
}
?>
