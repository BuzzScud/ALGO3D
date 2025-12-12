<?php
class Database {
    private $conn;
    private $dbFile;

    public function __construct() {
        $this->dbFile = __DIR__ . '/../data/algo3d.db';
    }

    public function getConnection() {
        if ($this->conn !== null) {
            return $this->conn;
        }

        try {
            // Ensure data directory exists
            $dataDir = dirname($this->dbFile);
            if (!is_dir($dataDir)) {
                mkdir($dataDir, 0755, true);
            }

            // Create SQLite connection
            $this->conn = new PDO("sqlite:" . $this->dbFile);
            $this->conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
            
            // Create tables if they don't exist
            $this->createTables();
            
            return $this->conn;
        } catch(PDOException $e) {
            error_log("Database connection error: " . $e->getMessage());
            return null;
        }
    }

    private function createTables() {
        // Create stock_symbols table
        $this->conn->exec("
            CREATE TABLE IF NOT EXISTS stock_symbols (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL UNIQUE,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");

        // Create todos table
        $this->conn->exec("
            CREATE TABLE IF NOT EXISTS todos (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                task TEXT NOT NULL,
                type TEXT CHECK(type IN ('daily', 'weekly')) NOT NULL DEFAULT 'daily',
                completed INTEGER DEFAULT 0,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");

        // Create user_profile table
        $this->conn->exec("
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
        $stmt = $this->conn->query("SELECT COUNT(*) FROM user_profile");
        if ($stmt->fetchColumn() == 0) {
            $this->conn->exec("
                INSERT INTO user_profile (name, email, timezone, currency, avatar_color)
                VALUES ('User', 'user@algo3d.com', 'America/New_York', 'USD', '#2563eb')
            ");
        }

        // Create chart_studies table
        $this->conn->exec("
            CREATE TABLE IF NOT EXISTS chart_studies (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                symbol TEXT NOT NULL,
                timeframe TEXT NOT NULL,
                chart_type TEXT DEFAULT 'candlestick',
                indicators TEXT,
                notes TEXT,
                price_data TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");

        // Create watchlist table
        $this->conn->exec("
            CREATE TABLE IF NOT EXISTS watchlist (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                symbol TEXT NOT NULL UNIQUE,
                added_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");

        // Create notes table
        $this->conn->exec("
            CREATE TABLE IF NOT EXISTS notes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                content TEXT,
                color TEXT DEFAULT '#1e293b',
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ");

        // Insert default symbols if none exist
        $stmt = $this->conn->query("SELECT COUNT(*) FROM stock_symbols");
        if ($stmt->fetchColumn() == 0) {
            $this->conn->exec("
                INSERT INTO stock_symbols (symbol) VALUES 
                ('SPY'), ('QQQ'), ('AAPL'), ('MSFT')
            ");
        }
    }
}
?>
