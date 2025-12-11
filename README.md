# ALGO3D - Trading Dashboard

A modern web application with HTML5 frontend and PHP backend featuring real-time market data, world clocks, and todo list management.

## Features

- **World Clocks**: Real-time clocks for Miami, London, and Tokyo (updates every second)
- **Real-Time Market Data**: Live stock data with dual API support
  - Primary: Finnhub API (60 calls/minute)
  - Backup: Yahoo Finance API (100 calls/minute)
  - Switchable data sources from the UI
  - Rate limiting and caching to respect API limits
  - Default symbols: SPY, QQQ, VIX, DXY
  - Add/remove custom symbols
- **Todo List**: Daily and weekly task tracking with filtering
  - Filter by: All, Daily, Weekly, Completed
  - Persistent storage with SQLite
- **Modern UI**: Responsive dark theme design
- **User-Friendly Menu**: Easy navigation with mobile support

## Requirements

- PHP 8.0 or higher
- cURL extension for PHP
- SQLite PDO extension (included in most PHP installations)

## Quick Start

1. Navigate to the project directory:
   ```bash
   cd /path/to/ALGO3D
   ```

2. Start the PHP development server:
   ```bash
   php -S localhost:8080
   ```

3. Open your browser:
   ```
   http://localhost:8080
   ```

That's it! The SQLite database is created automatically on first run.

## Project Structure

```
ALGO3D/
├── api/                      # PHP API endpoints
│   ├── get_market_data.php   # Fetch market data (Finnhub/Yahoo)
│   ├── get_api_status.php    # API rate limit status
│   ├── set_api_source.php    # Change API source
│   ├── add_symbol.php        # Add stock symbol
│   ├── delete_symbol.php     # Remove stock symbol
│   └── todos.php             # Todo CRUD operations
├── assets/
│   ├── css/
│   │   └── style.css         # Modern dark theme
│   └── js/
│       ├── main.js           # Navigation & general functionality
│       ├── world-clocks.js   # World clock updates
│       ├── market-data.js    # Market data & API switching
│       └── todo-list.js      # Todo list functionality
├── includes/
│   ├── config.php            # API keys & configuration
│   └── database.php          # SQLite database handler
├── data/                     # SQLite database (auto-created)
├── cache/                    # API response cache (auto-created)
├── index.php                 # Main dashboard page
├── setup.php                 # Setup verification script
└── README.md
```

## API Configuration

The application uses these APIs (configured in `includes/config.php`):

### Finnhub (Primary)
- API Key: Pre-configured
- Rate Limit: 60 calls/minute
- Documentation: https://finnhub.io/docs/api

### Yahoo Finance (Backup)
- No API key required
- Rate Limit: ~100 calls/minute
- Automatic fallback when Finnhub fails

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/get_market_data.php?source=finnhub` | Fetch market data |
| GET | `/api/get_api_status.php` | Get API rate limit status |
| POST | `/api/add_symbol.php` | Add stock symbol |
| POST | `/api/delete_symbol.php` | Remove stock symbol |
| GET | `/api/todos.php?filter=all` | Get todos |
| POST | `/api/todos.php` | Create todo |
| PUT | `/api/todos.php` | Update todo |
| DELETE | `/api/todos.php` | Delete todo |

## Usage

### Switching Data Sources
1. Use the "Data Source" dropdown in the Market Data section
2. Select either "Finnhub" or "Yahoo Finance"
3. Data refreshes automatically

### Adding Stock Symbols
1. Click "Add Symbol" button
2. Enter a symbol (e.g., AAPL, MSFT, TSLA)
3. Click "Add Symbol"

### Managing Todos
1. Enter a task in the input field
2. Select task type (Daily or Weekly)
3. Click "Add" or press Enter
4. Use filter buttons to view categories
5. Click checkbox to mark complete
6. Click trash icon to delete

## Technical Details

- **Database**: SQLite (no external database required)
- **Caching**: 60-second cache to respect API limits
- **Rate Limiting**: Tracks API calls per minute
- **Fallback**: Automatic switch to backup API on failures
- **Responsive**: Mobile-friendly design
- **Real-time**: Market data refreshes every 60 seconds

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Database errors | Ensure `data/` directory is writable |
| API errors | Check rate limits in UI; wait if exceeded |
| Cache errors | Ensure `cache/` directory is writable |
| Blank page | Check PHP error logs |

## License

This project is open source and available for personal and commercial use.

