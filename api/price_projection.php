<?php
/**
 * Price Projection API
 * 
 * This API provides price projections using CLLM with 88D threading.
 * It calls C functions from the math2 library.
 * 
 * Endpoints:
 * POST /api/price_projection.php
 * 
 * Request body:
 * {
 *   "historical_prices": [100.0, 101.0, ...],
 *   "depth_prime": 127,
 *   "base": 100.0,
 *   "steps": 120,
 *   "projection_count": 3,
 *   "omega_hz": 432.0,
 *   "decimals": 8
 * }
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Config is optional (only needed for database access, not for price projections)
if (file_exists(__DIR__ . '/../includes/config.php')) {
    require_once __DIR__ . '/../includes/config.php';
}

// Path to the C executable (will be created)
$executable_path = __DIR__ . '/../math/math 2/cllm/bin/cllm_price_projection';

// Path to shared library (alternative approach)
$library_path = __DIR__ . '/../math/math 2/cllm/lib/libcllm.so';

/**
 * Call C function using FFI (PHP 7.4+)
 */
function callCLLMviaFFI($config) {
    global $library_path;
    
    // Check if FFI is available
    if (!extension_loaded('ffi')) {
        return ['success' => false, 'error' => 'FFI extension not available'];
    }
    
    try {
        // Define FFI interface
        $ffi = FFI::cdef("
            typedef struct {
                uint32_t depth_prime;
                double base;
                uint32_t steps;
                uint32_t projection_count;
                double omega_hz;
                uint32_t decimals;
            } PriceProjectionConfig;
            
            typedef struct {
                double** projection_lines;
                uint32_t num_lines;
                uint32_t steps_per_line;
                bool success;
                char* error_message;
            } PriceProjectionResult;
            
            PriceProjectionResult* cllm_price_projection_compute(
                const PriceProjectionConfig* config,
                const double* historical_prices,
                uint32_t num_historical
            );
            
            void cllm_price_projection_free_result(PriceProjectionResult* result);
        ", $library_path);
        
        // Prepare config
        $c_config = $ffi->new('PriceProjectionConfig');
        $c_config->depth_prime = $config['depth_prime'];
        $c_config->base = $config['base'];
        $c_config->steps = $config['steps'];
        $c_config->projection_count = $config['projection_count'];
        $c_config->omega_hz = $config['omega_hz'] ?? 432.0;
        $c_config->decimals = $config['decimals'] ?? 8;
        
        // Prepare historical prices array
        $num_historical = count($config['historical_prices']);
        $c_prices = $ffi->new("double[$num_historical]");
        for ($i = 0; $i < $num_historical; $i++) {
            $c_prices[$i] = $config['historical_prices'][$i];
        }
        
        // Call C function
        $result = $ffi->cllm_price_projection_compute(
            FFI::addr($c_config),
            $c_prices,
            $num_historical
        );
        
        if (!$result || !$result->success) {
            $error = $result ? FFI::string($result->error_message) : 'Unknown error';
            return ['success' => false, 'error' => $error];
        }
        
        // Extract results
        $projection_lines = [];
        for ($i = 0; $i < $result->num_lines; $i++) {
            $line = [];
            $line_ptr = $result->projection_lines[$i];
            for ($j = 0; $j < $result->steps_per_line; $j++) {
                $line[] = $line_ptr[$j];
            }
            $projection_lines[] = $line;
        }
        
        // Free result
        $ffi->cllm_price_projection_free_result($result);
        
        return [
            'success' => true,
            'projection_lines' => $projection_lines,
            'num_lines' => $result->num_lines,
            'steps_per_line' => $result->steps_per_line
        ];
        
    } catch (Exception $e) {
        return ['success' => false, 'error' => 'FFI error: ' . $e->getMessage()];
    }
}

/**
 * Call C executable via command line
 */
function callCLLMviaExec($config) {
    global $executable_path;
    
    // Prepare input JSON
    $input = json_encode([
        'historical_prices' => $config['historical_prices'],
        'depth_prime' => $config['depth_prime'],
        'base' => $config['base'],
        'steps' => $config['steps'],
        'projection_count' => $config['projection_count'],
        'omega_hz' => $config['omega_hz'] ?? 432.0,
        'decimals' => $config['decimals'] ?? 8
    ]);
    
    // Check if executable exists
    if (!file_exists($executable_path) || !is_executable($executable_path)) {
        return ['success' => false, 'error' => 'C executable not found or not executable: ' . $executable_path];
    }
    
    // Call executable
    $descriptorspec = [
        0 => ['pipe', 'r'],  // stdin
        1 => ['pipe', 'w'],  // stdout
        2 => ['pipe', 'w']   // stderr
    ];
    
    $process = proc_open($executable_path, $descriptorspec, $pipes);
    
    if (!is_resource($process)) {
        return ['success' => false, 'error' => 'Failed to start C executable'];
    }
    
    // Write input
    fwrite($pipes[0], $input);
    fclose($pipes[0]);
    
    // Read output
    $output = stream_get_contents($pipes[1]);
    $error = stream_get_contents($pipes[2]);
    fclose($pipes[1]);
    fclose($pipes[2]);
    
    $return_code = proc_close($process);
    
    if ($return_code !== 0) {
        return ['success' => false, 'error' => 'C executable failed: ' . $error];
    }
    
    // Parse JSON output
    $result = json_decode($output, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        return ['success' => false, 'error' => 'Invalid JSON from C executable: ' . json_last_error_msg()];
    }
    
    return $result;
}

/**
 * Fallback: Use JavaScript-compatible algorithm (temporary)
 */
function computeProjectionFallback($config) {
    // This is a temporary fallback that uses PHP math
    // Should be replaced once C implementation is ready
    
    // CRITICAL: Always use the current/last price from historical_prices as the starting point
    // The 'base' parameter is for algorithm tuning, not the starting price
    $lastPrice = null;
    
    // Always use the last price from historical_prices (current price)
    if (!empty($config['historical_prices']) && is_array($config['historical_prices'])) {
        $lastPrice = end($config['historical_prices']);
    }
    
    // Validate last price
    if ($lastPrice === null || !is_numeric($lastPrice) || $lastPrice <= 0) {
        // Fallback to base parameter only if historical_prices is empty
        $lastPrice = isset($config['base']) && is_numeric($config['base']) && $config['base'] > 0 
            ? (float)$config['base'] 
            : null;
    }
    
    if ($lastPrice === null || $lastPrice <= 0) {
        return [
            'success' => false,
            'error' => 'Invalid current price: cannot project from invalid price. Last price: ' . ($lastPrice ?? 'null')
        ];
    }
    
    // Log the starting price for debugging
    error_log("Projection starting from current price: $lastPrice");
    
    $depthPrime = $config['depth_prime'];
    $omegaHz = $config['omega_hz'] ?? 432.0;
    $steps = $config['steps'];
    $projectionCount = $config['projection_count'];
    $decimals = $config['decimals'] ?? 8;
    
    // Generate triads (simplified)
    $triads = [];
    for ($i = 0; $i < $projectionCount; $i++) {
        $offset = $i - floor($projectionCount / 2);
        $center = $depthPrime + $offset;
        
        // Find primes (simplified - should use C function)
        $p1 = gmp_nextprime($center > 0 ? $center : 1);
        $p2 = gmp_nextprime(gmp_intval($p1) + 1);
        $p3 = gmp_nextprime(gmp_intval($p2) + 1);
        
        $triads[] = [
            gmp_intval($p1),
            gmp_intval($p2),
            gmp_intval($p3)
        ];
    }
    
    // Compute projections (simplified algorithm)
    $projectionLines = [];
    foreach ($triads as $triad) {
        $points = [];
        $psi = ($depthPrime % 360) * (M_PI / 180.0);
        $triProd = $triad[0] * $triad[1] * $triad[2];
        $tau = log($triProd) / log(3.0);
        $g = 1.0 + 0.01 * $tau + 0.001 * ($depthPrime % 7);
        
        // CRITICAL: First point (i=0) MUST start exactly from current price
        // Subsequent points add delta to the current price
        for ($i = 0; $i < $steps; $i++) {
            if ($i === 0) {
                // First point: start exactly from current price (no delta)
                $pricePoint = floor($lastPrice * pow(10, $decimals)) / pow(10, $decimals);
                $points[] = $pricePoint;
                continue;
            }
            
            // For subsequent points, calculate delta
            $lambda = 0.5; // Simplified
            $wHz = $omegaHz;
            $theta = (2.0 * M_PI * $i / 12.0) + $lambda * sin($wHz * $i * M_PI / 180.0 + $psi);
            $g = $g * (1.0 + 0.01 * $tau + 0.001 * cos($theta));
            
            // Simplified lattice sum
            $latticeSum = 0.0;
            for ($s = 0; $s < 12; $s++) {
                $angleBase = $i * (2.0 * M_PI / 12.0) + $s * (2.0 * M_PI / 12.0);
                $phiTerm = (($s + 1) % 360) * (M_PI / 180.0);
                $ang = $angleBase + $phiTerm;
                $base = cos($ang);
                $gNorm = tanh($g / 1e5);
                $term = $base * $psi * (1.0 + 0.5 * $gNorm);
                $latticeSum += $term;
            }
            
            $depthScale = log($depthPrime) / log(2.0);
            $triScale = max(1.0, $tau);
            $delta = floor($latticeSum * $depthScale * 0.5 * $triScale * pow(10, $decimals)) / pow(10, $decimals);
            
            // Subsequent points: current price + delta
            $pricePoint = floor(($lastPrice + $delta) * pow(10, $decimals)) / pow(10, $decimals);
            $points[] = $pricePoint;
        }
        
        $projectionLines[] = $points;
    }
    
    return [
        'success' => true,
        'projection_lines' => $projectionLines,
        'num_lines' => $projectionCount,
        'steps_per_line' => $steps,
        'method' => 'fallback'
    ];
}

// Main handler
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['success' => false, 'error' => 'Method not allowed']);
    exit;
}

try {
    // Parse input
    $input = json_decode(file_get_contents('php://input'), true);
    
    if (!$input) {
        throw new Exception('Invalid JSON input');
    }
    
    // Validate required fields
    $required = ['depth_prime', 'base', 'steps', 'projection_count'];
    foreach ($required as $field) {
        if (!isset($input[$field])) {
            throw new Exception("Missing required field: $field");
        }
    }
    
    // Prepare config
    $config = [
        'historical_prices' => $input['historical_prices'] ?? [],
        'depth_prime' => (int)$input['depth_prime'],
        'base' => (float)$input['base'],
        'steps' => (int)$input['steps'],
        'projection_count' => (int)$input['projection_count'],
        'omega_hz' => isset($input['omega_hz']) ? (float)$input['omega_hz'] : 432.0,
        'decimals' => isset($input['decimals']) ? (int)$input['decimals'] : 8
    ];
    
    // Try PHP fallback first, then FFI, then exec
    $result = null;
    
    // Try PHP fallback first (as requested)
    $result = computeProjectionFallback($config);
    
    // Try FFI if fallback failed (optional - can be enabled if needed)
    // if (!$result || !$result['success']) {
    //     if (extension_loaded('ffi') && file_exists($library_path)) {
    //         $result = callCLLMviaFFI($config);
    //         if ($result['success']) {
    //             $result['method'] = 'ffi';
    //         }
    //     }
    // }
    
    // Try exec if fallback failed (optional - can be enabled if needed)
    // if (!$result || !$result['success']) {
    //     if (file_exists($executable_path) && is_executable($executable_path)) {
    //         $result = callCLLMviaExec($config);
    //         if ($result['success']) {
    //             $result['method'] = 'exec';
    //         }
    //     }
    // }
    
    echo json_encode($result);
    
} catch (Exception $e) {
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}

