<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/config.php';
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['error' => 'Database connection failed']);
    exit;
}

// Create user profile table if it doesn't exist
try {
    $conn->exec("
        CREATE TABLE IF NOT EXISTS user_profile (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT DEFAULT 'User',
            email TEXT DEFAULT 'user@algo3d.com',
            timezone TEXT DEFAULT 'America/New_York',
            currency TEXT DEFAULT 'USD',
            avatar_color TEXT DEFAULT '#2563eb',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ");
    
    // Insert default user if none exists
    $stmt = $conn->query("SELECT COUNT(*) FROM user_profile");
    if ($stmt->fetchColumn() == 0) {
        $conn->exec("
            INSERT INTO user_profile (name, email, timezone, currency, avatar_color)
            VALUES ('User', 'user@algo3d.com', 'America/New_York', 'USD', '#2563eb')
        ");
    }
} catch (PDOException $e) {
    // Table might already exist
}

$method = $_SERVER['REQUEST_METHOD'];

try {
    switch ($method) {
        case 'GET':
            $stmt = $conn->prepare("SELECT * FROM user_profile ORDER BY id DESC LIMIT 1");
            $stmt->execute();
            $user = $stmt->fetch(PDO::FETCH_ASSOC);
            
            if (!$user) {
                // Return default user
                $user = [
                    'id' => 1,
                    'name' => 'User',
                    'email' => 'user@algo3d.com',
                    'timezone' => 'America/New_York',
                    'currency' => 'USD',
                    'avatar_color' => '#2563eb'
                ];
            }
            
            echo json_encode($user);
            break;
            
        case 'PUT':
            $input = json_decode(file_get_contents('php://input'), true);
            
            $name = trim($input['name'] ?? 'User');
            $email = trim($input['email'] ?? 'user@algo3d.com');
            $timezone = $input['timezone'] ?? 'America/New_York';
            $currency = $input['currency'] ?? 'USD';
            $avatarColor = $input['avatar_color'] ?? '#2563eb';
            
            // Check if user exists
            $stmt = $conn->query("SELECT COUNT(*) FROM user_profile");
            $exists = $stmt->fetchColumn() > 0;
            
            if ($exists) {
                $stmt = $conn->prepare("
                    UPDATE user_profile 
                    SET name = ?, email = ?, timezone = ?, currency = ?, avatar_color = ?, updated_at = CURRENT_TIMESTAMP
                    WHERE id = (SELECT id FROM user_profile ORDER BY id DESC LIMIT 1)
                ");
                $stmt->execute([$name, $email, $timezone, $currency, $avatarColor]);
            } else {
                $stmt = $conn->prepare("
                    INSERT INTO user_profile (name, email, timezone, currency, avatar_color)
                    VALUES (?, ?, ?, ?, ?)
                ");
                $stmt->execute([$name, $email, $timezone, $currency, $avatarColor]);
            }
            
            echo json_encode(['success' => true, 'message' => 'Profile updated successfully']);
            break;
            
        default:
            echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("User API error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error']);
}
?>











