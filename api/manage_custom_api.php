<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
require_once '../includes/database.php';

$db = new Database();
$conn = $db->getConnection();

if (!$conn) {
    echo json_encode(['success' => false, 'message' => 'Database connection failed']);
    exit;
}

$method = $_SERVER['REQUEST_METHOD'];
$input = json_decode(file_get_contents('php://input'), true);

try {
    if ($method === 'POST') {
        // Add or update custom API
        if (!isset($input['api_id']) || !isset($input['name']) || !isset($input['base_url']) || !isset($input['quote_url_template'])) {
            echo json_encode(['success' => false, 'message' => 'Missing required fields: api_id, name, base_url, quote_url_template']);
            exit;
        }

        $apiId = trim($input['api_id']);
        $name = trim($input['name']);
        $baseUrl = trim($input['base_url']);
        $quoteUrlTemplate = trim($input['quote_url_template']);
        
        // Validate API ID format (alphanumeric and underscore only)
        if (!preg_match('/^[a-z0-9_]+$/', $apiId)) {
            echo json_encode(['success' => false, 'message' => 'API ID must contain only lowercase letters, numbers, and underscores']);
            exit;
        }
        
        // Check if API ID conflicts with built-in APIs
        if (in_array($apiId, ['finnhub', 'yahoo'])) {
            echo json_encode(['success' => false, 'message' => 'API ID cannot be "finnhub" or "yahoo"']);
            exit;
        }

        $description = $input['description'] ?? '';
        $apiKey = $input['api_key'] ?? '';
        $rateLimit = isset($input['rate_limit']) ? (int)$input['rate_limit'] : 60;
        $rateLimitPeriod = isset($input['rate_limit_period']) ? (int)$input['rate_limit_period'] : 60;
        $requiresKey = isset($input['requires_key']) ? (int)$input['requires_key'] : 0;
        $responseFormat = $input['response_format'] ?? 'json';
        $quotePath = $input['quote_path'] ?? '';
        
        // Field mappings for parsing response
        $priceField = $input['price_field'] ?? 'c';
        $changeField = $input['change_field'] ?? 'd';
        $changePercentField = $input['change_percent_field'] ?? 'dp';
        $volumeField = $input['volume_field'] ?? 'v';
        $highField = $input['high_field'] ?? 'h';
        $lowField = $input['low_field'] ?? 'l';
        $openField = $input['open_field'] ?? 'o';
        $previousCloseField = $input['previous_close_field'] ?? 'pc';

        // Check if API already exists
        $stmt = $conn->prepare("SELECT id FROM custom_apis WHERE api_id = ?");
        $stmt->execute([$apiId]);
        $exists = $stmt->fetch();

        if ($exists) {
            // Update existing API
            $stmt = $conn->prepare("
                UPDATE custom_apis SET
                    name = ?,
                    description = ?,
                    base_url = ?,
                    quote_url_template = ?,
                    api_key = ?,
                    rate_limit = ?,
                    rate_limit_period = ?,
                    requires_key = ?,
                    response_format = ?,
                    quote_path = ?,
                    price_field = ?,
                    change_field = ?,
                    change_percent_field = ?,
                    volume_field = ?,
                    high_field = ?,
                    low_field = ?,
                    open_field = ?,
                    previous_close_field = ?,
                    updated_at = CURRENT_TIMESTAMP
                WHERE api_id = ?
            ");
            $stmt->execute([
                $name, $description, $baseUrl, $quoteUrlTemplate, $apiKey,
                $rateLimit, $rateLimitPeriod, $requiresKey, $responseFormat, $quotePath,
                $priceField, $changeField, $changePercentField, $volumeField,
                $highField, $lowField, $openField, $previousCloseField, $apiId
            ]);
            echo json_encode(['success' => true, 'message' => 'API updated successfully', 'api_id' => $apiId]);
        } else {
            // Insert new API
            $stmt = $conn->prepare("
                INSERT INTO custom_apis (
                    api_id, name, description, base_url, quote_url_template, api_key,
                    rate_limit, rate_limit_period, requires_key, response_format, quote_path,
                    price_field, change_field, change_percent_field, volume_field,
                    high_field, low_field, open_field, previous_close_field
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ");
            $stmt->execute([
                $apiId, $name, $description, $baseUrl, $quoteUrlTemplate, $apiKey,
                $rateLimit, $rateLimitPeriod, $requiresKey, $responseFormat, $quotePath,
                $priceField, $changeField, $changePercentField, $volumeField,
                $highField, $lowField, $openField, $previousCloseField
            ]);
            echo json_encode(['success' => true, 'message' => 'API added successfully', 'api_id' => $apiId]);
        }
    } else if ($method === 'DELETE') {
        // Delete custom API
        if (!isset($input['api_id'])) {
            echo json_encode(['success' => false, 'message' => 'Missing api_id']);
            exit;
        }

        $apiId = trim($input['api_id']);
        
        // Prevent deletion of built-in APIs
        if (in_array($apiId, ['finnhub', 'yahoo'])) {
            echo json_encode(['success' => false, 'message' => 'Cannot delete built-in APIs']);
            exit;
        }

        $stmt = $conn->prepare("DELETE FROM custom_apis WHERE api_id = ?");
        $stmt->execute([$apiId]);
        
        if ($stmt->rowCount() > 0) {
            echo json_encode(['success' => true, 'message' => 'API deleted successfully']);
        } else {
            echo json_encode(['success' => false, 'message' => 'API not found']);
        }
    } else if ($method === 'GET') {
        // Get all custom APIs
        $stmt = $conn->query("SELECT * FROM custom_apis WHERE is_active = 1 ORDER BY name");
        $apis = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Remove sensitive data (api_key) from response
        foreach ($apis as &$api) {
            unset($api['api_key']);
        }
        
        echo json_encode(['success' => true, 'apis' => $apis]);
    } else {
        echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("Custom API management error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    error_log("Custom API management error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Error: ' . $e->getMessage()]);
}
?>

