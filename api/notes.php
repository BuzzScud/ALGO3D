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
    
    // Add futures trading columns if they don't exist
    $stmt = $conn->query("PRAGMA table_info(notes)");
    $columns = $stmt->fetchAll(PDO::FETCH_COLUMN, 1);
    
    $tradingColumns = [
        'contract_symbol' => "ALTER TABLE notes ADD COLUMN contract_symbol TEXT",
        'contract_type' => "ALTER TABLE notes ADD COLUMN contract_type TEXT DEFAULT 'futures'",
        'entry_price' => "ALTER TABLE notes ADD COLUMN entry_price REAL",
        'exit_price' => "ALTER TABLE notes ADD COLUMN exit_price REAL",
        'quantity' => "ALTER TABLE notes ADD COLUMN quantity INTEGER DEFAULT 1",
        'direction' => "ALTER TABLE notes ADD COLUMN direction TEXT CHECK(direction IN ('long', 'short'))",
        'pnl' => "ALTER TABLE notes ADD COLUMN pnl REAL",
        'pnl_percent' => "ALTER TABLE notes ADD COLUMN pnl_percent REAL",
        'entry_time' => "ALTER TABLE notes ADD COLUMN entry_time DATETIME",
        'exit_time' => "ALTER TABLE notes ADD COLUMN exit_time DATETIME",
        'stop_loss' => "ALTER TABLE notes ADD COLUMN stop_loss REAL",
        'take_profit' => "ALTER TABLE notes ADD COLUMN take_profit REAL",
        'strategy' => "ALTER TABLE notes ADD COLUMN strategy TEXT",
        'timeframe' => "ALTER TABLE notes ADD COLUMN timeframe TEXT",
        'notes_type' => "ALTER TABLE notes ADD COLUMN notes_type TEXT DEFAULT 'trade'",
        'category' => "ALTER TABLE notes ADD COLUMN category TEXT",
        'pinned' => "ALTER TABLE notes ADD COLUMN pinned INTEGER DEFAULT 0",
        'labels' => "ALTER TABLE notes ADD COLUMN labels TEXT"
    ];
    
    foreach ($tradingColumns as $col => $sql) {
        if (!in_array($col, $columns)) {
            try {
                $conn->exec($sql);
            } catch (PDOException $e) {
                // Column might already exist
            }
        }
    }
} catch (PDOException $e) {
    // Table might already exist
}

$method = $_SERVER['REQUEST_METHOD'];

try {
    switch ($method) {
        case 'GET':
            $search = $_GET['search'] ?? '';
            
            if (!empty($search)) {
                $stmt = $conn->prepare("SELECT * FROM notes WHERE title LIKE ? OR content LIKE ? ORDER BY pinned DESC, updated_at DESC");
                $searchTerm = '%' . $search . '%';
                $stmt->execute([$searchTerm, $searchTerm]);
            } else {
                $stmt = $conn->prepare("SELECT * FROM notes ORDER BY pinned DESC, updated_at DESC");
                $stmt->execute();
            }
            
            $notes = $stmt->fetchAll(PDO::FETCH_ASSOC);
            // Parse labels JSON
            foreach ($notes as &$note) {
                if (!empty($note['labels'])) {
                    $note['labels'] = json_decode($note['labels'], true) ?: [];
                } else {
                    $note['labels'] = [];
                }
                $note['pinned'] = (bool)($note['pinned'] ?? 0);
            }
            echo json_encode($notes);
            break;
            
        case 'POST':
            $input = json_decode(file_get_contents('php://input'), true);
            $title = trim($input['title'] ?? 'Untitled Note');
            $content = $input['content'] ?? '';
            $color = $input['color'] ?? '#2563eb';
            $pinned = isset($input['pinned']) ? intval($input['pinned']) : 0;
            $labels = isset($input['labels']) && is_array($input['labels']) ? json_encode($input['labels']) : null;
            
            // Trading fields
            $contractSymbol = $input['contract_symbol'] ?? null;
            $contractType = $input['contract_type'] ?? 'futures';
            $entryPrice = isset($input['entry_price']) ? floatval($input['entry_price']) : null;
            $exitPrice = isset($input['exit_price']) ? floatval($input['exit_price']) : null;
            $quantity = isset($input['quantity']) ? intval($input['quantity']) : 1;
            $direction = $input['direction'] ?? null;
            $pnl = isset($input['pnl']) ? floatval($input['pnl']) : null;
            $pnlPercent = isset($input['pnl_percent']) ? floatval($input['pnl_percent']) : null;
            $entryTime = $input['entry_time'] ?? null;
            $exitTime = $input['exit_time'] ?? null;
            $stopLoss = isset($input['stop_loss']) ? floatval($input['stop_loss']) : null;
            $takeProfit = isset($input['take_profit']) ? floatval($input['take_profit']) : null;
            $strategy = $input['strategy'] ?? null;
            $timeframe = $input['timeframe'] ?? null;
            $notesType = $input['notes_type'] ?? 'note';
            $category = $input['category'] ?? null;
            
            $stmt = $conn->prepare("INSERT INTO notes (title, content, color, pinned, labels, contract_symbol, contract_type, entry_price, exit_price, quantity, direction, pnl, pnl_percent, entry_time, exit_time, stop_loss, take_profit, strategy, timeframe, notes_type, category) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            $stmt->execute([$title, $content, $color, $pinned, $labels, $contractSymbol, $contractType, $entryPrice, $exitPrice, $quantity, $direction, $pnl, $pnlPercent, $entryTime, $exitTime, $stopLoss, $takeProfit, $strategy, $timeframe, $notesType, $category]);
            
            echo json_encode(['success' => true, 'id' => $conn->lastInsertId()]);
            break;
            
        case 'PUT':
            $input = json_decode(file_get_contents('php://input'), true);
            $id = intval($input['id'] ?? 0);
            $title = trim($input['title'] ?? 'Untitled Note');
            $content = $input['content'] ?? '';
            $color = $input['color'] ?? '#2563eb';
            $pinned = isset($input['pinned']) ? intval($input['pinned']) : 0;
            $labels = isset($input['labels']) && is_array($input['labels']) ? json_encode($input['labels']) : null;
            
            // Trading fields
            $contractSymbol = $input['contract_symbol'] ?? null;
            $contractType = $input['contract_type'] ?? 'futures';
            $entryPrice = isset($input['entry_price']) ? floatval($input['entry_price']) : null;
            $exitPrice = isset($input['exit_price']) ? floatval($input['exit_price']) : null;
            $quantity = isset($input['quantity']) ? intval($input['quantity']) : 1;
            $direction = $input['direction'] ?? null;
            $pnl = isset($input['pnl']) ? floatval($input['pnl']) : null;
            $pnlPercent = isset($input['pnl_percent']) ? floatval($input['pnl_percent']) : null;
            $entryTime = $input['entry_time'] ?? null;
            $exitTime = $input['exit_time'] ?? null;
            $stopLoss = isset($input['stop_loss']) ? floatval($input['stop_loss']) : null;
            $takeProfit = isset($input['take_profit']) ? floatval($input['take_profit']) : null;
            $strategy = $input['strategy'] ?? null;
            $timeframe = $input['timeframe'] ?? null;
            $notesType = $input['notes_type'] ?? 'note';
            $category = $input['category'] ?? null;
            
            if ($id > 0) {
                $stmt = $conn->prepare("UPDATE notes SET title = ?, content = ?, color = ?, pinned = ?, labels = ?, contract_symbol = ?, contract_type = ?, entry_price = ?, exit_price = ?, quantity = ?, direction = ?, pnl = ?, pnl_percent = ?, entry_time = ?, exit_time = ?, stop_loss = ?, take_profit = ?, strategy = ?, timeframe = ?, notes_type = ?, category = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?");
                $stmt->execute([$title, $content, $color, $pinned, $labels, $contractSymbol, $contractType, $entryPrice, $exitPrice, $quantity, $direction, $pnl, $pnlPercent, $entryTime, $exitTime, $stopLoss, $takeProfit, $strategy, $timeframe, $notesType, $category, $id]);
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






