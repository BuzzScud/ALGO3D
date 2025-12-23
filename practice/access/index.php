<?php
// Get external/public IP address
function getClientIP() {
    // First, try to get IP from server headers (works if behind proxy/load balancer)
    $ipKeys = ['HTTP_CLIENT_IP', 'HTTP_X_FORWARDED_FOR', 'HTTP_X_FORWARDED', 'HTTP_X_CLUSTER_CLIENT_IP', 'HTTP_FORWARDED_FOR', 'HTTP_FORWARDED', 'REMOTE_ADDR'];
    
    $detectedIP = null;
    foreach ($ipKeys as $key) {
        if (array_key_exists($key, $_SERVER) === true) {
            foreach (explode(',', $_SERVER[$key]) as $ip) {
                $ip = trim($ip);
                if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE) !== false) {
                    $detectedIP = $ip;
                    break 2; // Break out of both loops
                }
            }
        }
    }
    
    // If no public IP found, try REMOTE_ADDR
    if (!$detectedIP && isset($_SERVER['REMOTE_ADDR'])) {
        $detectedIP = $_SERVER['REMOTE_ADDR'];
    }
    
    // Check if the detected IP is localhost/private
    $isLocalhost = false;
    if ($detectedIP) {
        $isLocalhost = (
            $detectedIP === '127.0.0.1' || 
            $detectedIP === '::1' || 
            $detectedIP === 'localhost' ||
            filter_var($detectedIP, FILTER_VALIDATE_IP, FILTER_FLAG_NO_PRIV_RANGE | FILTER_FLAG_NO_RES_RANGE) === false
        );
    }
    
    // If localhost or no IP detected, get external IP from service
    if ($isLocalhost || !$detectedIP) {
        $externalIP = getExternalIP();
        if ($externalIP) {
            return $externalIP;
        }
    }
    
    // Return detected IP if it's public, otherwise return error indicator
    return $detectedIP ? $detectedIP : '0.0.0.0';
}

// Get external/public IP from external service
function getExternalIP() {
    // List of IP detection services (try multiple for reliability)
    $services = [
        'https://api.ipify.org?format=text',
        'https://icanhazip.com',
        'https://ifconfig.me/ip',
        'https://api.ip.sb/ip',
        'https://checkip.amazonaws.com'
    ];
    
    // Try each service with timeout
    foreach ($services as $service) {
        $context = stream_context_create([
            'http' => [
                'timeout' => 3,
                'method' => 'GET',
                'header' => "User-Agent: PHP\r\n"
            ]
        ]);
        
        $ip = @file_get_contents($service, false, $context);
        if ($ip !== false) {
            $ip = trim($ip);
            // Validate it's a real IP address
            if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4 | FILTER_FLAG_IPV6)) {
                return $ip;
            }
        }
    }
    
    // If all services fail, try with cURL as fallback
    if (function_exists('curl_init')) {
        $ch = curl_init();
        curl_setopt_array($ch, [
            CURLOPT_URL => 'https://api.ipify.org?format=text',
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_TIMEOUT => 3,
            CURLOPT_CONNECTTIMEOUT => 3,
            CURLOPT_USERAGENT => 'PHP'
        ]);
        
        $ip = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);
        
        if ($httpCode === 200 && $ip !== false) {
            $ip = trim($ip);
            if (filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4 | FILTER_FLAG_IPV6)) {
                return $ip;
            }
        }
    }
    
    return null;
}

$method = $_SERVER['REQUEST_METHOD'];
$ipFile = __DIR__ . '/../acl/address.txt';

// Check if client wants JSON response
$acceptHeader = $_SERVER['HTTP_ACCEPT'] ?? '';
$wantsJson = strpos($acceptHeader, 'application/json') !== false || 
             isset($_GET['format']) && $_GET['format'] === 'json';

if ($method === 'POST') {
    header('Content-Type: application/json');
    header('Access-Control-Allow-Origin: *');
    // Save IP address to file
    $ip = getClientIP();
    
    if (empty($ip) || $ip === '0.0.0.0') {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Could not determine IP address'
        ]);
        exit;
    }
    
    // Ensure directory exists
    $dir = dirname($ipFile);
    if (!is_dir($dir)) {
        mkdir($dir, 0755, true);
    }
    
    // Write IP to file
    $result = file_put_contents($ipFile, $ip . "\n", FILE_APPEND | LOCK_EX);
    
    if ($result === false) {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Failed to write IP address to file'
        ]);
        exit;
    }
    
    echo json_encode([
        'success' => true,
        'message' => 'IP address saved successfully',
        'ip' => $ip,
        'file' => $ipFile
    ]);
} else if ($method === 'GET') {
    // If JSON is requested, return IP info as JSON
    if ($wantsJson) {
        header('Content-Type: application/json');
        header('Access-Control-Allow-Origin: *');
        $ip = getClientIP();
        echo json_encode([
            'success' => true,
            'ip' => $ip,
            'message' => 'IP address retrieved'
        ]);
        exit;
    }
    
    // Return HTML page with button
    header('Content-Type: text/html; charset=UTF-8');
    $currentIP = getClientIP();
    ?>
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>IP Whitelist Tool</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box;
            }
            body {
                font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                min-height: 100vh;
                display: flex;
                align-items: center;
                justify-content: center;
                padding: 20px;
            }
            .container {
                background: white;
                border-radius: 20px;
                padding: 40px;
                box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
                max-width: 500px;
                width: 100%;
                text-align: center;
            }
            h1 {
                color: #333;
                margin-bottom: 10px;
                font-size: 2rem;
            }
            .subtitle {
                color: #666;
                margin-bottom: 30px;
                font-size: 0.95rem;
            }
            .ip-display {
                background: #f5f5f5;
                border-radius: 10px;
                padding: 20px;
                margin-bottom: 30px;
                font-family: 'Courier New', monospace;
                font-size: 1.2rem;
                color: #333;
                word-break: break-all;
            }
            .ip-label {
                font-size: 0.85rem;
                color: #666;
                margin-bottom: 8px;
                text-transform: uppercase;
                letter-spacing: 1px;
            }
            .btn {
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                border: none;
                padding: 15px 40px;
                font-size: 1.1rem;
                font-weight: 600;
                border-radius: 10px;
                cursor: pointer;
                transition: all 0.3s ease;
                box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
                width: 100%;
            }
            .btn:hover {
                transform: translateY(-2px);
                box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
            }
            .btn:active {
                transform: translateY(0);
            }
            .btn:disabled {
                opacity: 0.6;
                cursor: not-allowed;
                transform: none;
            }
            .status {
                margin-top: 20px;
                padding: 15px;
                border-radius: 10px;
                font-weight: 500;
                display: none;
            }
            .status.success {
                background: #d4edda;
                color: #155724;
                border: 1px solid #c3e6cb;
                display: block;
            }
            .status.error {
                background: #f8d7da;
                color: #721c24;
                border: 1px solid #f5c6cb;
                display: block;
            }
            .loading {
                display: inline-block;
                width: 20px;
                height: 20px;
                border: 3px solid rgba(255,255,255,.3);
                border-radius: 50%;
                border-top-color: white;
                animation: spin 1s ease-in-out infinite;
                margin-right: 10px;
            }
            @keyframes spin {
                to { transform: rotate(360deg); }
            }
        </style>
    </head>
    <body>
        <div class="container">
            <h1>🔐 IP Whitelist</h1>
            <p class="subtitle">Add your IP address to the whitelist</p>
            
            <div class="ip-display">
                <div class="ip-label">Your Current IP Address</div>
                <div id="ip-address"><?php echo htmlspecialchars($currentIP); ?></div>
            </div>
            
            <button class="btn" id="whitelist-btn" onclick="saveIP()">
                <span id="btn-text">Add My IP to Whitelist</span>
            </button>
            
            <div class="status" id="status"></div>
        </div>
        
        <script>
            async function saveIP() {
                const btn = document.getElementById('whitelist-btn');
                const btnText = document.getElementById('btn-text');
                const status = document.getElementById('status');
                
                // Reset status
                status.className = 'status';
                status.style.display = 'none';
                
                // Disable button and show loading
                btn.disabled = true;
                btnText.innerHTML = '<span class="loading"></span> Saving...';
                
                try {
                    const response = await fetch(window.location.href, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        }
                    });
                    
                    const result = await response.json();
                    
                    if (result.success) {
                        status.className = 'status success';
                        status.textContent = '✅ ' + result.message + ' (IP: ' + result.ip + ')';
                        status.style.display = 'block';
                    } else {
                        throw new Error(result.message || 'Failed to save IP address');
                    }
                } catch (error) {
                    status.className = 'status error';
                    status.textContent = '❌ Error: ' + error.message;
                    status.style.display = 'block';
                } finally {
                    btn.disabled = false;
                    btnText.textContent = 'Add My IP to Whitelist';
                }
            }
        </script>
    </body>
    </html>
    <?php
} else {
    http_response_code(405);
    echo json_encode([
        'success' => false,
        'message' => 'Method not allowed'
    ]);
}
?>

