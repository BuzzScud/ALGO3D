<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, DELETE');
header('Access-Control-Allow-Headers: Content-Type');

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
        saveProjection();
        break;
    case 'DELETE':
        deleteProjection();
        break;
    default:
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
        $data = json_decode(file_get_contents('php://input'), true);
        
        if (!isset($data['id'])) {
            echo json_encode([
                'success' => false,
                'message' => 'Projection ID required'
            ]);
            return;
        }
        
        $stmt = $conn->prepare("DELETE FROM saved_projections WHERE id = ?");
        $stmt->execute([$data['id']]);
        
        echo json_encode([
            'success' => true,
            'message' => 'Projection deleted successfully'
        ]);
    } catch (PDOException $e) {
        echo json_encode([
            'success' => false,
            'message' => 'Failed to delete projection: ' . $e->getMessage()
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

