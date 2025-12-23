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
    if ($method === 'POST' && isset($input['action']) && $input['action'] === 'execute') {
        // Execute REST API call
        if (!isset($input['api_id'])) {
            echo json_encode(['success' => false, 'message' => 'Missing api_id']);
            exit;
        }

        $apiId = trim($input['api_id']);
        $customParams = $input['params'] ?? [];
        $customHeaders = $input['headers'] ?? [];
        $customBody = $input['body'] ?? null;

        $stmt = $conn->prepare("SELECT * FROM rest_apis WHERE api_id = ? AND is_active = 1");
        $stmt->execute([$apiId]);
        $apiConfig = $stmt->fetch(PDO::FETCH_ASSOC);

        if (!$apiConfig) {
            echo json_encode(['success' => false, 'message' => 'REST API not found']);
            exit;
        }

        // Build URL
        $url = rtrim($apiConfig['base_url'], '/') . '/' . ltrim($apiConfig['endpoint_path'], '/');
        
        // Add query parameters
        $queryParams = [];
        if (!empty($apiConfig['query_params'])) {
            $savedParams = json_decode($apiConfig['query_params'], true);
            if (is_array($savedParams)) {
                $queryParams = array_merge($queryParams, $savedParams);
            }
        }
        if (!empty($customParams)) {
            $queryParams = array_merge($queryParams, $customParams);
        }
        if (!empty($queryParams)) {
            $url .= '?' . http_build_query($queryParams);
        }

        // Prepare headers
        $headers = [];
        if (!empty($apiConfig['headers'])) {
            $savedHeaders = json_decode($apiConfig['headers'], true);
            if (is_array($savedHeaders)) {
                $headers = $savedHeaders;
            }
        }
        if (!empty($customHeaders)) {
            $headers = array_merge($headers, $customHeaders);
        }

        // Add authentication
        if ($apiConfig['auth_type'] !== 'none' && !empty($apiConfig['auth_value'])) {
            switch ($apiConfig['auth_type']) {
                case 'bearer':
                    $headers['Authorization'] = 'Bearer ' . $apiConfig['auth_value'];
                    break;
                case 'api_key_header':
                    $headers['X-API-Key'] = $apiConfig['auth_value'];
                    break;
                case 'api_key_query':
                    $url .= (strpos($url, '?') !== false ? '&' : '?') . 'api_key=' . urlencode($apiConfig['auth_value']);
                    break;
            }
        }

        // Prepare request body
        $body = null;
        if (in_array($apiConfig['http_method'], ['POST', 'PUT', 'PATCH'])) {
            if ($customBody !== null) {
                $body = is_string($customBody) ? $customBody : json_encode($customBody);
            } elseif (!empty($apiConfig['request_body'])) {
                $body = $apiConfig['request_body'];
            }
        }

        // Execute request
        $startTime = microtime(true);
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_TIMEOUT, (int)$apiConfig['timeout']);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        
        // Set HTTP method
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, strtoupper($apiConfig['http_method']));
        
        // Set headers
        if (!empty($headers)) {
            $headerArray = [];
            foreach ($headers as $key => $value) {
                $headerArray[] = "$key: $value";
            }
            curl_setopt($ch, CURLOPT_HTTPHEADER, $headerArray);
        }
        
        // Set body
        if ($body !== null) {
            curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
            if (!isset($headers['Content-Type'])) {
                curl_setopt($ch, CURLOPT_HTTPHEADER, array_merge(
                    $headerArray ?? [],
                    ['Content-Type: application/json']
                ));
            }
        }
        
        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        $error = curl_error($ch);
        $responseTime = (microtime(true) - $startTime) * 1000;
        curl_close($ch);

        if ($error) {
            echo json_encode([
                'success' => false,
                'message' => 'Request failed: ' . $error,
                'http_code' => $httpCode,
                'response_time' => round($responseTime, 2)
            ]);
            exit;
        }

        // Parse response
        $responseData = $response;
        if ($apiConfig['response_format'] === 'json') {
            $parsed = json_decode($response, true);
            if (json_last_error() === JSON_ERROR_NONE) {
                $responseData = $parsed;
            }
        }

        echo json_encode([
            'success' => true,
            'http_code' => $httpCode,
            'response_time' => round($responseTime, 2),
            'response' => $responseData,
            'raw_response' => $response,
            'headers_sent' => $headers,
            'url' => $url
        ]);
    } else if ($method === 'POST') {
        // Add or update REST API
        if (!isset($input['api_id']) || !isset($input['name']) || !isset($input['base_url']) || !isset($input['endpoint_path'])) {
            echo json_encode(['success' => false, 'message' => 'Missing required fields: api_id, name, base_url, endpoint_path']);
            exit;
        }

        $apiId = trim($input['api_id']);
        $name = trim($input['name']);
        $baseUrl = trim($input['base_url']);
        $endpointPath = trim($input['endpoint_path']);
        
        // Validate API ID format
        if (!preg_match('/^[a-z0-9_]+$/', $apiId)) {
            echo json_encode(['success' => false, 'message' => 'API ID must contain only lowercase letters, numbers, and underscores']);
            exit;
        }

        $description = $input['description'] ?? '';
        $httpMethod = strtoupper($input['http_method'] ?? 'GET');
        $headers = isset($input['headers']) ? json_encode($input['headers']) : null;
        $authType = $input['auth_type'] ?? 'none';
        $authValue = $input['auth_value'] ?? '';
        $requestBody = $input['request_body'] ?? null;
        $queryParams = isset($input['query_params']) ? json_encode($input['query_params']) : null;
        $responseFormat = $input['response_format'] ?? 'json';
        $timeout = isset($input['timeout']) ? (int)$input['timeout'] : 30;
        $rateLimit = isset($input['rate_limit']) ? (int)$input['rate_limit'] : 60;
        $rateLimitPeriod = isset($input['rate_limit_period']) ? (int)$input['rate_limit_period'] : 60;

        // Check if API already exists
        $stmt = $conn->prepare("SELECT id FROM rest_apis WHERE api_id = ?");
        $stmt->execute([$apiId]);
        $exists = $stmt->fetch();

        if ($exists) {
            // Update existing API
            $stmt = $conn->prepare("
                UPDATE rest_apis SET
                    name = ?,
                    description = ?,
                    base_url = ?,
                    endpoint_path = ?,
                    http_method = ?,
                    headers = ?,
                    auth_type = ?,
                    auth_value = ?,
                    request_body = ?,
                    query_params = ?,
                    response_format = ?,
                    timeout = ?,
                    rate_limit = ?,
                    rate_limit_period = ?,
                    updated_at = CURRENT_TIMESTAMP
                WHERE api_id = ?
            ");
            $stmt->execute([
                $name, $description, $baseUrl, $endpointPath, $httpMethod,
                $headers, $authType, $authValue, $requestBody, $queryParams,
                $responseFormat, $timeout, $rateLimit, $rateLimitPeriod, $apiId
            ]);
            echo json_encode(['success' => true, 'message' => 'REST API updated successfully', 'api_id' => $apiId]);
        } else {
            // Insert new API
            $stmt = $conn->prepare("
                INSERT INTO rest_apis (
                    api_id, name, description, base_url, endpoint_path, http_method,
                    headers, auth_type, auth_value, request_body, query_params,
                    response_format, timeout, rate_limit, rate_limit_period
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ");
            $stmt->execute([
                $apiId, $name, $description, $baseUrl, $endpointPath, $httpMethod,
                $headers, $authType, $authValue, $requestBody, $queryParams,
                $responseFormat, $timeout, $rateLimit, $rateLimitPeriod
            ]);
            echo json_encode(['success' => true, 'message' => 'REST API added successfully', 'api_id' => $apiId]);
        }
    } else if ($method === 'DELETE') {
        // Delete REST API
        if (!isset($input['api_id'])) {
            echo json_encode(['success' => false, 'message' => 'Missing api_id']);
            exit;
        }

        $apiId = trim($input['api_id']);
        $stmt = $conn->prepare("DELETE FROM rest_apis WHERE api_id = ?");
        $stmt->execute([$apiId]);
        
        if ($stmt->rowCount() > 0) {
            echo json_encode(['success' => true, 'message' => 'REST API deleted successfully']);
        } else {
            echo json_encode(['success' => false, 'message' => 'REST API not found']);
        }
    } else if ($method === 'GET') {
        // Get all REST APIs
        $stmt = $conn->query("SELECT * FROM rest_apis WHERE is_active = 1 ORDER BY name");
        $apis = $stmt->fetchAll(PDO::FETCH_ASSOC);
        
        // Remove sensitive data (auth_value) from response
        foreach ($apis as &$api) {
            unset($api['auth_value']);
        }
        
        echo json_encode(['success' => true, 'apis' => $apis]);
    } else {
        echo json_encode(['success' => false, 'message' => 'Method not allowed']);
    }
} catch (PDOException $e) {
    error_log("REST API management error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Database error: ' . $e->getMessage()]);
} catch (Exception $e) {
    error_log("REST API management error: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Error: ' . $e->getMessage()]);
}
?>

