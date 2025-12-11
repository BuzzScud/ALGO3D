<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['error' => 'Database connection failed', 'notes' => []]);
    exit;
}

// Create notes table if it doesn't exist
try {
    $conn->exec("
        CREATE TABLE IF NOT EXISTS notes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT,
            color TEXT DEFAULT '#1e293b',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ");
} catch (PDOException $e) {
    // Table might already exist
}

$method = $_SERVER['REQUEST_METHOD'];

try {
    switch ($method) {
        case 'GET':
            $search = $_GET['search'] ?? '';
            
            if (!empty($search)) {
                $stmt = $conn->prepare("SELECT * FROM notes WHERE title LIKE ? OR content LIKE ? ORDER BY updated_at DESC");
                $searchTerm = '%' . $search . '%';
                $stmt->execute([$searchTerm, $searchTerm]);
            } else {
                $stmt = $conn->prepare("SELECT * FROM notes ORDER BY updated_at DESC");
                $stmt->execute();
            }
            
            $notes = $stmt->fetchAll(PDO::FETCH_ASSOC);
            echo json_encode($notes);
            break;
            
        case 'POST':
            $input = json_decode(file_get_contents('php://input'), true);
            $title = trim($input['title'] ?? 'Untitled Note');
            $content = $input['content'] ?? '';
            $color = $input['color'] ?? '#1e293b';
            
            $stmt = $conn->prepare("INSERT INTO notes (title, content, color) VALUES (?, ?, ?)");
            $stmt->execute([$title, $content, $color]);
            
            echo json_encode(['success' => true, 'id' => $conn->lastInsertId()]);
            break;
            
        case 'PUT':
            $input = json_decode(file_get_contents('php://input'), true);
            $id = intval($input['id'] ?? 0);
            $title = trim($input['title'] ?? 'Untitled Note');
            $content = $input['content'] ?? '';
            $color = $input['color'] ?? '#1e293b';
            
            if ($id > 0) {
                $stmt = $conn->prepare("UPDATE notes SET title = ?, content = ?, color = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
                $stmt->execute([$title, $content, $color, $id]);
                echo json_encode(['success' => true]);
            } else {
                echo json_encode(['success' => false, 'message' => 'Invalid ID']);
            }
            break;
            
        case 'DELETE':
            $input = json_decode(file_get_contents('php://input'), true);
            $id = intval($input['id'] ?? 0);
            
            if ($id > 0) {
                $stmt = $conn->prepare("DELETE FROM notes WHERE id = ?");
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
    error_log("Notes API error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error']);
}
?>

