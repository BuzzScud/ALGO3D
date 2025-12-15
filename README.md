# ALGO3D - Trading Dashboard

A modern web application with HTML5 frontend and PHP backend featuring real-time market data, interactive trading charts, world clocks, notes, and todo list management.

## Features

- **Interactive Trading Charts**: Fully functional candlestick charts with pan and zoom capabilities
  - Real-time price data with technical indicators (SMA, EMA, RSI, MACD, Bollinger Bands)
  - Pan by clicking and dragging
  - Zoom with mouse wheel or control buttons
  - Save and load chart studies
  - Multiple chart types (Candlestick, Line, Area)
  - Multiple timeframes (1D, 5D, 1M, 3M, 6M, 1Y, 5Y)
- **World Clocks**: Real-time clocks for Miami, London, and Tokyo (updates every second)
- **Real-Time Market Data**: Live stock data with dual API support
  - Primary: Finnhub API (60 calls/minute)
  - Backup: Yahoo Finance API (100 calls/minute)
  - Switchable data sources from the UI
  - Rate limiting and caching to respect API limits
  - Default symbols: SPY, QQQ, VIX, DXY
  - Add/remove custom symbols
- **Notes**: Full-featured note-taking system with search and color coding
- **Todo List**: Daily and weekly task tracking with filtering
  - Filter by: All, Daily, Weekly, Completed
  - Persistent storage with SQLite
- **User Profile**: Customizable user widget with statistics
- **Settings**: Comprehensive settings page with tabs for API, Display, Notifications, and Data management
- **Modern UI**: Responsive dark/light theme design with sidebar navigation
- **TradingView Integration**: Ticker tape widget for market overview

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
│   ├── charts.php            # Chart data and studies
│   ├── fib_calc.php          # Fibonacci calculator API
│   ├── notes.php             # Notes CRUD operations
│   ├── todos.php             # Todo CRUD operations
│   ├── settings.php          # Settings management
│   ├── user.php              # User profile management
│   └── stats.php             # Dashboard statistics
├── assets/
│   ├── css/
│   │   ├── style.css         # Main stylesheet
│   │   └── fib-page.css      # Fibonacci calculator styles
│   └── js/
│       ├── main.js           # Navigation & general functionality
│       ├── charts.js         # Interactive charts with pan/zoom
│       ├── fib-calc.js       # Fibonacci calculator
│       ├── projections.js    # Price projection engine
│       ├── world-clocks.js   # World clock updates
│       ├── market-data.js    # Market data & API switching
│       ├── todo-list.js      # Todo list functionality
│       ├── notes.js          # Notes functionality
│       ├── settings.js       # Settings management
│       └── user-widget.js    # User profile widget
├── docs/                     # Documentation
│   ├── features/             # Feature documentation
│   ├── implementation/       # Implementation guides
│   ├── fixes/                # Bug fix documentation
│   └── README.md             # Documentation index
├── includes/
│   ├── config.php            # API keys & configuration (not in repo)
│   └── database.php          # SQLite database handler
├── math/                     # Mathematical algorithms and models
│   ├── algorithms/           # C-based algorithms
│   ├── src/                  # Mathematical source files
│   └── THESIS.md             # Mathematical thesis
├── data/                     # SQLite database (auto-created, not in repo)
├── cache/                    # API response cache (auto-created)
├── index.php                 # Main dashboard page
├── setup.php                 # Setup verification script
└── README.md
```

## Documentation

For detailed documentation, see the [`docs/`](docs/) directory:
- **Features**: Implementation details for chart interactivity, hyperdimensional tools, and more
- **Implementation**: Step-by-step guides for projection integration and validation metrics
- **Fixes**: Bug fix documentation and resolution notes

See [`docs/README.md`](docs/README.md) for a complete documentation index.

## API Configuration

The application uses these APIs (configured in `includes/config.php`):

### Finnhub (Primary)
- API Key: Configure in `includes/config.php`
- Rate Limit: 60 calls/minute
- Documentation: https://finnhub.io/docs/api

### Yahoo Finance (Backup)
- No API key required
- Rate Limit: ~100 calls/minute
- Automatic fallback when Finnhub fails

## Key Features

### Interactive Charts
- Pan: Click and drag to move around the chart
- Zoom: Scroll with mouse wheel or use zoom buttons
- Technical Indicators: Toggle SMA, EMA, RSI, MACD, Bollinger Bands
- Save Studies: Save chart configurations for later analysis
- Multiple Timeframes: 1D, 5D, 1M, 3M, 6M, 1Y, 5Y

### Market Data
- Real-time price updates
- Change percentages
- Volume information
- API source switching
- Custom symbol tracking

### Notes System
- Create, edit, and delete notes
- Search functionality
- Color coding
- Persistent storage

## License

GPL-3.0 License
