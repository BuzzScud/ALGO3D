<?php
/**
 * Crystalline Math - Web Demo with REST API
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    if (isset($_GET['action'])) {
        header('Content-Type: application/json');
        echo json_encode(['error' => 'crystalline_math extension not loaded']);
        exit;
    }
    die("Error: crystalline_math extension is not loaded.");
}

// Handle API requests BEFORE any HTML output
if (isset($_GET['action'])) {
    header('Content-Type: application/json');
    
    switch ($_GET['action']) {
        case 'check':
            $number = intval($_GET['number']);
            $isPrime = crystalline_prime_is_prime($number);
            echo json_encode(['isPrime' => (bool)$isPrime]);
            exit;
            
        case 'nth':
            $n = intval($_GET['n']);
            
            // Enforce reasonable limit to prevent memory exhaustion
            if ($n > 10000000) {
                echo json_encode([
                    'error' => 'Value too large',
                    'message' => 'Maximum supported value is 10,000,000 for web requests',
                    'requested' => $n,
                    'reason' => 'Larger values require excessive memory (see LARGE_PRIME_LIMITATIONS.md)'
                ]);
                exit;
            }
            
            if ($n < 1) {
                echo json_encode([
                    'error' => 'Invalid value',
                    'message' => 'N must be positive (N >= 1)'
                ]);
                exit;
            }
            
            // Set timeout to prevent hanging
            set_time_limit(30);
            
            $prime = crystalline_prime_nth($n);
            echo json_encode(['prime' => (int)$prime]);
            exit;
            
        case 'o1':
            $position = intval($_GET['position']);
            $magnitude = intval($_GET['magnitude']);
            $result = crystalline_prime_generate_o1($position, $magnitude);
            
            // Calculate the actual value even if composite
            $base = 0;
            if ($position == 3) $base = 5;
            else if ($position == 6) $base = 7;
            else if ($position == 9) $base = 11;
            
            $value = $base + $magnitude * 12;
            $isPrime = ($result > 0);
            
            echo json_encode([
                'value' => (int)$value,
                'isPrime' => $isPrime,
                'prime' => (int)$result
            ]);
            exit;
            
        case 'reverse':
            $number = intval($_GET['number']);
            
            if ($number < 2) {
                echo json_encode([
                    'error' => 'Invalid number',
                    'message' => 'Number must be >= 2'
                ]);
                exit;
            }
            
            $result = crystalline_reverse_lookup($number);
            
            if ($result === false) {
                echo json_encode([
                    'error' => 'Invalid number',
                    'message' => 'Number does not fit clock lattice structure (mod 12 must be 2, 3, 5, 7, or 11)'
                ]);
                exit;
            }
            
            echo json_encode($result);
            exit;
            
        default:
            echo json_encode(['error' => 'Invalid action']);
            exit;
    }
}

// If we get here, render the HTML page
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Crystalline Math - O(1) Prime Generation</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        
        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }
        
        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        header p {
            font-size: 1.2em;
            opacity: 0.9;
        }
        
        .content {
            padding: 40px;
        }
        
        .calculator {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        
        .calculator h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .calc-row {
            display: flex;
            gap: 20px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        
        .calc-group {
            flex: 1;
            min-width: 200px;
        }
        
        .calc-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #333;
        }
        
        .calc-group input {
            width: 100%;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        
        .calc-group input:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .calc-buttons {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
        }
        
        button {
            padding: 12px 30px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .result {
            margin-top: 20px;
            padding: 20px;
            background: white;
            border-radius: 8px;
            border-left: 4px solid #667eea;
            display: none;
        }
        
        .result.show {
            display: block;
            animation: slideIn 0.3s ease-out;
        }
        
        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        .result h3 {
            color: #667eea;
            margin-bottom: 10px;
        }
        
        .result-value {
            font-size: 1.5em;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }
        
        .examples {
            margin-top: 40px;
        }
        
        .example-section {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
        }
        
        .example-section h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .example-section h3 {
            color: #764ba2;
            margin: 20px 0 10px 0;
            font-size: 1.3em;
        }
        
        .output {
            background: white;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
            font-family: 'Courier New', monospace;
            line-height: 1.8;
            margin-top: 15px;
        }
        
        .output-line {
            margin: 5px 0;
        }
        
        .prime {
            color: #28a745;
            font-weight: bold;
        }
        
        .composite {
            color: #dc3545;
        }
        
        .info-box {
            background: #e3f2fd;
            border-left: 4px solid #2196f3;
            padding: 20px;
            border-radius: 8px;
            margin: 20px 0;
        }
        
        .info-box h4 {
            color: #1976d2;
            margin-bottom: 10px;
        }
        
        footer {
            background: #333;
            color: white;
            text-align: center;
            padding: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🔢 Crystalline Math</h1>
            <p>O(1) Deterministic Prime Generation</p>
            <p style="font-size: 0.9em; margin-top: 10px;">Extension Version: <?php echo crystalline_version(); ?></p>
        </header>
        
        <div class="content">
            <!-- Interactive Calculator -->
            <div class="calculator">
                <h2>🧮 Interactive Prime Calculator</h2>
                
                <div class="calc-row">
                    <div class="calc-group">
                        <label for="checkNumber">Check if Prime:</label>
                        <input type="number" id="checkNumber" placeholder="Enter a number" value="157">
                    </div>
                    <div class="calc-group">
                        <label for="nthPrime">Get Nth Prime (max 10M):</label>
                        <input type="number" id="nthPrime" placeholder="Enter N (max: 10,000,000)" value="10" min="1" max="10000000">
                    </div>
                </div>
                
                <div class="calc-row">
                    <div class="calc-group">
                        <label for="position">O(1) Position:</label>
                        <input type="number" id="position" placeholder="Position (0-11)" value="3" min="0" max="11">
                    </div>
                    <div class="calc-group">
                        <label for="magnitude">O(1) Magnitude:</label>
                        <input type="number" id="magnitude" placeholder="Magnitude" value="0" min="0">
                    </div>
                </div>
                
                <div class="calc-row">
                    <div class="calc-group" style="flex: 1 1 100%;">
                        <label for="reverseNumber">Reverse Lookup (Number → Position + Magnitude):</label>
                        <input type="number" id="reverseNumber" placeholder="Enter any number" value="29" min="2">
                    </div>
                </div>
                
                <div class="calc-buttons">
                    <button onclick="checkPrime()">Check Prime</button>
                    <button onclick="getNthPrime()">Get Nth Prime</button>
                    <button onclick="generateO1()">Generate O(1)</button>
                    <button onclick="reverseLookup()">Reverse Lookup</button>
                </div>
                
                <div id="calcResult" class="result"></div>
            </div>
            
            <div class="info-box">
                <h4>ℹ️ About O(1) Prime Generation</h4>
                <p>This extension uses a revolutionary deterministic formula to generate primes in constant time O(1), without trial division or probabilistic tests. The clock lattice structure maps primes to positions on a Babylonian clock (12, 60, 60, 100).</p>
                <p style="margin-top: 10px;"><strong>Reverse Lookup:</strong> You can now convert any number back to its ring position and magnitude using Babylonian reduction mathematics. This works for both primes and composites that follow the clock lattice structure (mod 12 ∈ {2, 3, 5, 7, 11}).</p>
            </div>
            
            <div class="info-box" style="background: #fff3cd; border-left-color: #ffc107;">
                <h4>⚠️ Performance Note</h4>
                <p><strong>Nth Prime function</strong> is limited to N ≤ 10,000,000 for web requests due to memory constraints. Larger values require excessive RAM and computation time.</p>
                <p style="margin-top: 10px;"><strong>Performance guide:</strong></p>
                <ul style="margin-left: 20px; margin-top: 5px;">
                    <li>N < 1,000,000: Fast (< 1 second)</li>
                    <li>N < 10,000,000: Reasonable (< 30 seconds)</li>
                    <li>N > 10,000,000: Not recommended for web</li>
                </ul>
                <p style="margin-top: 10px;">See <a href="LARGE_PRIME_LIMITATIONS.md" style="color: #0066cc;">LARGE_PRIME_LIMITATIONS.md</a> for details.</p>
            </div>
            
            <!-- Examples -->
            <div class="examples">
                <div class="example-section">
                    <h2>📊 Live Examples</h2>
                    
                    <h3>1. O(1) Prime Generation</h3>
                    <div class="output">
                        <?php
                        $examples = [
                            [3, 0], [3, 1], [3, 2], [3, 4],
                            [6, 0], [6, 1], [6, 4],
                            [9, 0], [9, 1], [9, 4]
                        ];
                        foreach ($examples as list($pos, $mag)) {
                            $result = crystalline_prime_generate_o1($pos, $mag);
                            
                            // Calculate actual value
                            $base = 0;
                            if ($pos == 3) $base = 5;
                            else if ($pos == 6) $base = 7;
                            else if ($pos == 9) $base = 11;
                            $value = $base + $mag * 12;
                            
                            echo "<div class='output-line'>";
                            echo "Position $pos, Magnitude $mag: ";
                            if ($result > 0) {
                                echo "<span class='prime'>$value (PRIME)</span>";
                            } else {
                                echo "<span class='composite'>$value (COMPOSITE)</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>2. Primality Testing</h3>
                    <div class="output">
                        <?php
                        $test_numbers = [2, 3, 4, 5, 17, 100, 157, 997, 1009];
                        foreach ($test_numbers as $n) {
                            $is_prime = crystalline_prime_is_prime($n);
                            echo "<div class='output-line'>";
                            echo "$n is ";
                            if ($is_prime) {
                                echo "<span class='prime'>PRIME</span>";
                            } else {
                                echo "<span class='composite'>composite</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>3. First 20 Primes</h3>
                    <div class="output">
                        <?php
                        echo "<div class='output-line'>";
                        for ($i = 1; $i <= 20; $i++) {
                            $prime = crystalline_prime_nth($i);
                            echo "<span class='prime'>$prime</span>";
                            if ($i < 20) echo ", ";
                        }
                        echo "</div>";
                        ?>
                    </div>
                    
                    <h3>4. Clock Positions (Magnitude 0)</h3>
                    <div class="output">
                        <?php
                        for ($pos = 0; $pos < 12; $pos++) {
                            $result = crystalline_prime_generate_o1($pos, 0);
                            
                            // Calculate actual value for valid positions
                            $value = "N/A";
                            if ($pos == 3) $value = 5;
                            else if ($pos == 6) $value = 7;
                            else if ($pos == 9) $value = 11;
                            
                            echo "<div class='output-line'>";
                            echo "Position $pos: ";
                            if ($result > 0) {
                                echo "<span class='prime'>$value (PRIME)</span>";
                            } else if ($value !== "N/A") {
                                echo "<span class='composite'>$value (COMPOSITE)</span>";
                            } else {
                                echo "<span class='composite'>No prime at this position</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>5. Reverse Lookup Examples</h3>
                    <div class="output">
                        <?php
                        $test_numbers = [29, 65, 157, 91, 127];
                        foreach ($test_numbers as $num) {
                            $lookup = crystalline_reverse_lookup($num);
                            if ($lookup !== false) {
                                echo "<div class='output-line'>";
                                echo "$num: Ring {$lookup['ring']}, Position {$lookup['position']}, Magnitude {$lookup['magnitude']} - ";
                                if ($lookup['is_prime']) {
                                    echo "<span class='prime'>PRIME</span>";
                                } else {
                                    echo "<span class='composite'>COMPOSITE</span>";
                                }
                                echo "</div>";
                            }
                        }
                        ?>
                    </div>
                    
                    <h3>6. Performance Benchmark</h3>
                    <div class="output">
                        <?php
                        $start = microtime(true);
                        $count = 10000;
                        for ($i = 0; $i < $count; $i++) {
                            crystalline_prime_is_prime(rand(1, 100000));
                        }
                        $elapsed = microtime(true) - $start;
                        $per_second = $count / $elapsed;
                        ?>
                        <div class='output-line'>
                            Checked <?php echo number_format($count); ?> numbers in <?php echo number_format($elapsed, 4); ?> seconds
                        </div>
                        <div class='output-line'>
                            <span class='prime'>Rate: <?php echo number_format($per_second, 0); ?> checks/second</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        
        <footer>
            <p>&copy; 2024 Crystalline Math | NinjaTech AI</p>
            <p style="font-size: 0.9em; margin-top: 5px;">Powered by O(1) Deterministic Prime Generation</p>
        </footer>
    </div>
    
    <script>
        function checkPrime() {
            const number = document.getElementById('checkNumber').value;
            if (!number) {
                alert('Please enter a number');
                return;
            }
            
            fetch('?action=check&number=' + number)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>Primality Test Result</h3>
                        <div class="result-value">
                            ${number} is <span class="${data.isPrime ? 'prime' : 'composite'}">${data.isPrime ? 'PRIME' : 'COMPOSITE'}</span>
                        </div>
                    `;
                    result.classList.add('show');
                });
        }
        
        function getNthPrime() {
            const n = document.getElementById('nthPrime').value;
            if (!n || n < 1) {
                alert('Please enter a positive number');
                return;
            }
            
            if (n > 10000000) {
                alert('Value too large! Maximum supported value is 10,000,000.\n\nLarger values require excessive memory and computation time.');
                return;
            }
            
            const result = document.getElementById('calcResult');
            result.innerHTML = '<h3>Computing...</h3><p>Please wait...</p>';
            result.classList.add('show');
            
            fetch('?action=nth&n=' + n)
                .then(r => r.json())
                .then(data => {
                    if (data.error) {
                        result.innerHTML = `
                            <h3>Error</h3>
                            <div class="result-value" style="color: #dc3545;">
                                ${data.message}
                            </div>
                            ${data.reason ? `<p style="margin-top: 10px; color: #666;">${data.reason}</p>` : ''}
                        `;
                    } else {
                        result.innerHTML = `
                            <h3>Nth Prime Result</h3>
                            <div class="result-value">
                                Prime #${n} = <span class="prime">${data.prime}</span>
                            </div>
                        `;
                    }
                    result.classList.add('show');
                })
                .catch(err => {
                    result.innerHTML = `
                        <h3>Error</h3>
                        <div class="result-value" style="color: #dc3545;">
                            Request failed: ${err.message}
                        </div>
                    `;
                });
        }
        
        function generateO1() {
            const position = document.getElementById('position').value;
            const magnitude = document.getElementById('magnitude').value;
            
            if (position === '' || magnitude === '') {
                alert('Please enter both position and magnitude');
                return;
            }
            
            fetch(`?action=o1&position=${position}&magnitude=${magnitude}`)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>O(1) Generation Result</h3>
                        <div class="result-value">
                            Position ${position}, Magnitude ${magnitude} = 
                            <span class="${data.isPrime ? 'prime' : 'composite'}">${data.value}</span>
                            ${data.isPrime ? '(PRIME)' : '(COMPOSITE)'}
                        </div>
                        <p style="margin-top: 10px; color: #666;">
                            Generated in O(1) constant time using deterministic formula
                        </p>
                    `;
                    result.classList.add('show');
                });
        }
        
        function reverseLookup() {
            const number = document.getElementById('reverseNumber').value;
            
            if (!number || number < 2) {
                alert('Please enter a number >= 2');
                return;
            }
            
            fetch(`?action=reverse&number=${number}`)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    
                    if (data.error) {
                        result.innerHTML = `
                            <h3>Reverse Lookup Error</h3>
                            <div class="result-value" style="color: #dc3545;">
                                ${data.message}
                            </div>
                            <p style="margin-top: 10px; color: #666;">
                                Only numbers with mod 12 ∈ {2, 3, 5, 7, 11} fit the clock lattice structure.
                            </p>
                        `;
                    } else {
                        result.innerHTML = `
                            <h3>Reverse Lookup Result</h3>
                            <div class="result-value">
                                ${number} = <span class="${data.is_prime ? 'prime' : 'composite'}">${data.is_prime ? 'PRIME' : 'COMPOSITE'}</span>
                            </div>
                            <div style="margin-top: 15px; padding: 15px; background: #f8f9fa; border-radius: 8px;">
                                <p style="margin: 5px 0;"><strong>Ring:</strong> ${data.ring}</p>
                                <p style="margin: 5px 0;"><strong>Position:</strong> ${data.position}</p>
                                <p style="margin: 5px 0;"><strong>Magnitude:</strong> ${data.magnitude}</p>
                                <p style="margin: 10px 0 5px 0; color: #666; font-size: 0.9em;">
                                    Formula: ${number} = base + ${data.magnitude} × 12
                                </p>
                            </div>
                        `;
                    }
                    result.classList.add('show');
                })
                .catch(err => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>Error</h3>
                        <div class="result-value" style="color: #dc3545;">
                            Request failed: ${err.message}
                        </div>
                    `;
                    result.classList.add('show');
                });
        }
    </script>
</body>
</html>