<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');
header('Access-Control-Max-Age: 86400'); // 24 hours

// Handle preflight OPTIONS request
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once '../includes/config.php';
require_once '../includes/database.php';

$database = new Database();
$conn = $database->getConnection();

$method = $_SERVER['REQUEST_METHOD'];

switch ($method) {
    case 'GET':
        getSavedProjections();
        break;
    case 'POST':
        // Check if this is a clear all request
        $input = file_get_contents('php://input');
        $data = json_decode($input, true);
        if (isset($data['action']) && $data['action'] === 'clear_all') {
            clearAllProjections();
        } else {
            saveProjection();
        }
        break;
    case 'DELETE':
        deleteProjection();
        break;
    default:
        http_response_code(405);
        echo json_encode(['success' => false, 'message' => 'Method not allowed']);
}

function getSavedProjections() {
    global $conn;
    
    try {
        // Ensure table exists
        checkAndCreateTable();
        
        $stmt = $conn->prepare("SELECT * FROM saved_projections ORDER BY saved_at DESC");
        $stmt->execute();
        $projections = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Decode JSON data
        foreach ($projections as &$proj) {
            if (isset($proj['projection_data'])) {
                $proj['projection_data'] = json_decode($proj['projection_data'], true);
            }
            if (isset($proj['chart_data'])) {
                $proj['chart_data'] = json_decode($proj['chart_data'], true);
            }
        }
        
        echo json_encode([
            'success' => true,
            'projections' => $projections
        ]);
    } catch (PDOException $e) {
        echo json_encode([
            'success' => false,
            'message' => 'Failed to load projections: ' . $e->getMessage()
        ]);
    }
}

function saveProjection() {
    global $conn;
    
    try {
        $data = json_decode(file_get_contents('php://input'), true);
        
        if (!isset($data['symbol']) || !isset($data['projection_data'])) {
            echo json_encode([
                'success' => false,
                'message' => 'Missing required fields'
            ]);
            return;
        }
        
        // Check if table exists, create if not
        checkAndCreateTable();
        
        $symbol = strtoupper($data['symbol']);
        $projectionData = json_encode($data['projection_data']);
        $chartData = isset($data['chart_data']) ? json_encode($data['chart_data']) : null;
        $params = isset($data['params']) ? json_encode($data['params']) : null;
        $title = isset($data['title']) ? $data['title'] : $symbol . ' Projection';
        $notes = isset($data['notes']) ? $data['notes'] : '';
        
        $stmt = $conn->prepare("
            INSERT INTO saved_projections 
            (symbol, title, projection_data, chart_data, params, notes, saved_at) 
            VALUES (?, ?, ?, ?, ?, ?, datetime('now'))
        ");
        
        $stmt->execute([$symbol, $title, $projectionData, $chartData, $params, $notes]);
        
        $id = $conn->lastInsertId();
        
        echo json_encode([
            'success' => true,
            'message' => 'Projection saved successfully',
            'id' => $id
        ]);
    } catch (PDOException $e) {
        echo json_encode([
            'success' => false,
            'message' => 'Failed to save projection: ' . $e->getMessage()
        ]);
    }
}

function deleteProjection() {
    global $conn;
    
    try {
        // Ensure table exists
        checkAndCreateTable();
        
        // Get request body
        $input = file_get_contents('php://input');
        if (empty($input)) {
            http_response_code(400);
            echo json_encode([
                'success' => false,
                'message' => 'Request body is required'
            ]);
            return;
        }
        
        $data = json_decode($input, true);
        
        if (json_last_error() !== JSON_ERROR_NONE) {
            http_response_code(400);
            echo json_encode([
                'success' => false,
                'message' => 'Invalid JSON: ' . json_last_error_msg()
            ]);
            return;
        }
        
        if (!isset($data['id']) || empty($data['id'])) {
            http_response_code(400);
            echo json_encode([
                'success' => false,
                'message' => 'Projection ID is required'
            ]);
            return;
        }
        
        $id = intval($data['id']);
        
        if ($id <= 0) {
            http_response_code(400);
            echo json_encode([
                'success' => false,
                'message' => 'Invalid projection ID'
            ]);
            return;
        }
        
        // Check if projection exists (use fetch instead of rowCount for SQLite compatibility)
        $checkStmt = $conn->prepare("SELECT id FROM saved_projections WHERE id = ?");
        $checkStmt->execute([$id]);
        $exists = $checkStmt->fetch(PDO::FETCH_ASSOC);
        
        if (empty($exists) || !isset($exists['id'])) {
            http_response_code(404);
            echo json_encode([
                'success' => false,
                'message' => 'Projection not found'
            ]);
            return;
        }
        
        // Delete the projection
        $stmt = $conn->prepare("DELETE FROM saved_projections WHERE id = ?");
        $stmt->execute([$id]);
        
        // For DELETE, rowCount() works in SQLite, but verify deletion succeeded
        if ($stmt->rowCount() > 0) {
            http_response_code(200);
            echo json_encode([
                'success' => true,
                'message' => 'Projection deleted successfully'
            ]);
        } else {
            // Double-check if it still exists
            $verifyStmt = $conn->prepare("SELECT id FROM saved_projections WHERE id = ?");
            $verifyStmt->execute([$id]);
            $stillExists = $verifyStmt->fetch(PDO::FETCH_ASSOC);
            
            if (empty($stillExists) || !isset($stillExists['id'])) {
                // Projection was deleted (rowCount might not have reported correctly)
                http_response_code(200);
                echo json_encode([
                    'success' => true,
                    'message' => 'Projection deleted successfully'
                ]);
            } else {
                http_response_code(500);
                echo json_encode([
                    'success' => false,
                    'message' => 'Failed to delete projection'
                ]);
            }
        }
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Database error: ' . $e->getMessage()
        ]);
    } catch (Exception $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Error: ' . $e->getMessage()
        ]);
    }
}

function clearAllProjections() {
    global $conn;
    
    try {
        // Ensure table exists
        checkAndCreateTable();
        
        // Delete all projections
        $stmt = $conn->prepare("DELETE FROM saved_projections");
        $stmt->execute();
        
        $deletedCount = $stmt->rowCount();
        
        echo json_encode([
            'success' => true,
            'message' => "All projections cleared successfully",
            'deleted_count' => $deletedCount
        ]);
    } catch (PDOException $e) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Failed to clear projections: ' . $e->getMessage()
        ]);
    }
}

function checkAndCreateTable() {
    global $conn;
    
    try {
        $conn->exec("
            CREATE TABLE IF NOT EXISTS saved_projections (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL,
                title TEXT NOT NULL,
                projection_data TEXT NOT NULL,
                chart_data TEXT,
                params TEXT,
                notes TEXT,
                saved_at TEXT NOT NULL
            )
        ");
    } catch (PDOException $e) {
        // Table might already exist
    }
}

?>

