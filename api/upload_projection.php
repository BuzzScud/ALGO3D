<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

require_once '../includes/config.php';
require_once '../includes/database.php';

$database = new Database();
$conn = $database->getConnection();

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    exit;
}

try {
    // Check if file was uploaded
    if (!isset($_FILES['projection_file']) || $_FILES['projection_file']['error'] !== UPLOAD_ERR_OK) {
        throw new Exception('No file uploaded or upload error occurred');
    }
    
    $file = $_FILES['projection_file'];
    $fileName = $file['name'];
    $fileTmpName = $file['tmp_name'];
    $fileSize = $file['size'];
    $fileError = $file['error'];
    
    // Validate file size (max 10MB)
    $maxFileSize = 10 * 1024 * 1024; // 10MB
    if ($fileSize > $maxFileSize) {
        throw new Exception('File size exceeds maximum allowed size of 10MB');
    }
    
    // Validate file type (JSON or PNG)
    $fileExt = strtolower(pathinfo($fileName, PATHINFO_EXTENSION));
    if ($fileExt !== 'json' && $fileExt !== 'png') {
        throw new Exception('Invalid file type. Only JSON and PNG files are allowed');
    }
    
    // Ensure table exists
    checkAndCreateTable();
    
    // Handle PNG files differently
    if ($fileExt === 'png') {
        // Create uploads directory if it doesn't exist
        $uploadDir = __DIR__ . '/../data/uploaded_projections/';
        if (!is_dir($uploadDir)) {
            mkdir($uploadDir, 0755, true);
        }
        
        // Generate unique filename
        $timestamp = date('YmdHis');
        $uniqueFileName = $timestamp . '_' . preg_replace('/[^a-zA-Z0-9._-]/', '_', $fileName);
        $targetPath = $uploadDir . $uniqueFileName;
        
        // Move uploaded file
        if (!move_uploaded_file($fileTmpName, $targetPath)) {
            throw new Exception('Failed to save uploaded PNG file');
        }
        
        // Extract symbol from filename if possible (format: SYMBOL_INTERVAL_projection_DATE_TIME.png)
        $symbol = 'UNKNOWN';
        if (preg_match('/^([A-Z]+)_/', strtoupper($fileName), $matches)) {
            $symbol = $matches[1];
        }
        
        // Create projection entry for PNG
        $projectionDataJson = json_encode([
            'type' => 'image',
            'image_path' => 'data/uploaded_projections/' . $uniqueFileName,
            'original_filename' => $fileName,
            'file_size' => $fileSize
        ]);
        
        $title = $symbol . ' Projection Image (Uploaded)';
        $notes = 'PNG image uploaded from file: ' . $fileName;
        
        $stmt = $conn->prepare("
            INSERT INTO saved_projections 
            (symbol, title, projection_data, chart_data, params, notes, saved_at) 
            VALUES (?, ?, ?, ?, ?, ?, datetime('now'))
        ");
        
        $stmt->execute([$symbol, $title, $projectionDataJson, null, null, $notes]);
        
        $id = $conn->lastInsertId();
        
        echo json_encode([
            'success' => true,
            'message' => 'PNG image uploaded and saved successfully',
            'id' => $id,
            'symbol' => $symbol,
            'title' => $title,
            'file_type' => 'png'
        ]);
        
    } else {
        // Handle JSON files (original logic)
        $fileContent = file_get_contents($fileTmpName);
        if ($fileContent === false) {
            throw new Exception('Failed to read uploaded file');
        }
        
        $projectionData = json_decode($fileContent, true);
        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new Exception('Invalid JSON format: ' . json_last_error_msg());
        }
        
        // Validate required fields
        if (!isset($projectionData['symbol']) || !isset($projectionData['projection_data'])) {
            throw new Exception('Missing required fields: symbol and projection_data are required');
        }
        
        $symbol = strtoupper($projectionData['symbol']);
        $projectionDataJson = json_encode($projectionData['projection_data']);
        $chartDataJson = isset($projectionData['chart_data']) ? json_encode($projectionData['chart_data']) : null;
        $paramsJson = isset($projectionData['params']) ? json_encode($projectionData['params']) : null;
        $title = isset($projectionData['title']) ? $projectionData['title'] : $symbol . ' Projection (Uploaded)';
        $notes = isset($projectionData['notes']) ? $projectionData['notes'] : 'Uploaded from file: ' . $fileName;
        
        // Save to database
        $stmt = $conn->prepare("
            INSERT INTO saved_projections 
            (symbol, title, projection_data, chart_data, params, notes, saved_at) 
            VALUES (?, ?, ?, ?, ?, ?, datetime('now'))
        ");
        
        $stmt->execute([$symbol, $title, $projectionDataJson, $chartDataJson, $paramsJson, $notes]);
        
        $id = $conn->lastInsertId();
        
        echo json_encode([
            'success' => true,
            'message' => 'Projection uploaded and saved successfully',
            'id' => $id,
            'symbol' => $symbol,
            'title' => $title,
            'file_type' => 'json'
        ]);
    }
    
} catch (Exception $e) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => $e->getMessage()
    ]);
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

