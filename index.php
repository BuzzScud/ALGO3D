<?php
session_start();
require_once 'includes/config.php';
require_once 'includes/database.php';
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ALGO3D - Trading Dashboard</title>
    <link rel="stylesheet" href="assets/css/tailwind.css">
    <link rel="stylesheet" href="assets/css/style.css">
    <link rel="stylesheet" href="assets/css/fib-page.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
</head>
<body>
    <!-- Sidebar Menu -->
    <aside class="sidebar" id="sidebar">
        <div class="sidebar-header">
            <div class="sidebar-logo">
                <span>App</span>
            </div>
            <button class="sidebar-toggle" id="sidebar-toggle">
                <i class="fas fa-bars"></i>
            </button>
        </div>
        <nav class="sidebar-nav">
            <ul class="sidebar-menu">
                <li class="sidebar-item active" data-page="dashboard">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-home"></i>
                        <span>Dashboard</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="news">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-newspaper"></i>
                        <span>News</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="notes">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-sticky-note"></i>
                        <span>Notes</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="charts">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-chart-line"></i>
                        <span>Charts</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="data">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-database"></i>
                        <span>Data</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="api">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-plug"></i>
                        <span>API</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="settings">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-cog"></i>
                        <span>Settings</span>
                    </a>
                </li>
            </ul>
        </nav>
        <!-- Mini Clock Widgets -->
        <div class="sidebar-mini-clocks">
            <div class="mini-clock-item" data-city="New York">
                <div class="mini-clock-header">
                    <span class="mini-clock-city">New York</span>
                    <span class="mini-clock-country">United States</span>
                    <button class="mini-clock-remove" data-city="New York" title="Remove clock">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="mini-clock-time" id="mini-clock-ny">--:--:--</div>
                <div class="mini-clock-date" id="mini-clock-ny-date">-- -- ----</div>
                <div class="mini-clock-label">5 Change & Adventure</div>
                <div class="mini-clock-description">A time for exploration and new experiences</div>
            </div>
            <div class="mini-clock-item" data-city="Tokyo">
                <div class="mini-clock-header">
                    <span class="mini-clock-city">Tokyo</span>
                    <span class="mini-clock-country">Japan</span>
                    <button class="mini-clock-remove" data-city="Tokyo" title="Remove clock">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="mini-clock-time" id="mini-clock-tokyo">--:--:--</div>
                <div class="mini-clock-date" id="mini-clock-tokyo-date">-- -- ----</div>
                <div class="mini-clock-label">6 Nurturing & Responsibility</div>
                <div class="mini-clock-description">Focus on caring for others and home matters</div>
            </div>
            <button class="btn-add-clock" id="add-clock-btn">
                <i class="fas fa-plus"></i> Add Clock
            </button>
        </div>
        <!-- User Widget -->
        <div class="sidebar-user-widget">
            <div class="user-widget-header" id="user-widget-toggle">
                <div class="user-avatar" id="user-avatar">
                    <span id="user-initials">U</span>
                </div>
                <div class="user-info">
                    <div class="user-name" id="user-name">User</div>
                    <div class="user-email" id="user-email">user@algo3d.com</div>
                </div>
                <i class="fas fa-chevron-up user-widget-arrow" id="user-widget-arrow"></i>
            </div>
            <div class="user-widget-menu" id="user-widget-menu">
                <div class="user-menu-item" data-action="profile">
                    <i class="fas fa-user"></i>
                    <span>Profile</span>
                </div>
                <div class="user-menu-item" data-action="preferences">
                    <i class="fas fa-sliders-h"></i>
                    <span>Preferences</span>
                </div>
                <div class="user-menu-item" data-action="stats">
                    <i class="fas fa-chart-bar"></i>
                    <span>Statistics</span>
                </div>
                <div class="user-menu-divider"></div>
                <div class="user-menu-item" data-action="logout">
                    <i class="fas fa-sign-out-alt"></i>
                    <span>Logout</span>
                </div>
            </div>
        </div>
        <div class="sidebar-footer">
            <div class="theme-toggle-container">
                <div class="theme-toggle-label">
                    <i class="fas fa-sun"></i>
                    <span>Light Mode</span>
                </div>
                <label class="theme-toggle-switch">
                    <input type="checkbox" id="sidebar-theme-toggle">
                    <span class="theme-toggle-slider"></span>
                </label>
            </div>
            <span class="version">v1.0.0</span>
        </div>
    </aside>

    <!-- Main Content -->
    <div class="main-wrapper">
        <!-- TradingView Ticker Tape -->
        <div class="ticker-tape-container">
            <!-- TradingView Widget BEGIN -->
            <div class="tradingview-widget-container">
                <div class="tradingview-widget-container__widget"></div>
                <div class="tradingview-widget-copyright">
                    <a href="https://www.tradingview.com/markets/" rel="noopener nofollow" target="_blank">
                        <span class="blue-text">Ticker tape</span>
                    </a>
                    <span class="trademark"> by TradingView</span>
                </div>
                <script type="text/javascript" src="https://s3.tradingview.com/external-embedding/embed-widget-ticker-tape.js" async>
                {
                    "symbols": [
                        {
                            "proName": "FOREXCOM:SPXUSD",
                            "title": "ES"
                        },
                        {
                            "proName": "FOREXCOM:NSXUSD",
                            "title": "NQ"
                        },
                        {
                            "proName": "FX:US30",
                            "title": "YM"
                        },
                        {
                            "proName": "OANDA:AUDUSD",
                            "title": "AU"
                        },
                        {
                            "proName": "OANDA:EURUSD",
                            "title": "EU"
                        },
                        {
                            "proName": "OANDA:GBPUSD",
                            "title": "GU"
                        },
                        {
                            "proName": "NASDAQ:QQQ",
                            "title": "QQQ"
                        },
                        {
                            "proName": "AMEX:SPY",
                            "title": "SPY"
                        },
                        {
                            "proName": "CAPITALCOM:VIX",
                            "title": "VIX"
                        },
                        {
                            "proName": "CAPITALCOM:DXY",
                            "title": "DXY"
                        }
                    ],
                    "colorTheme": "dark",
                    "locale": "en",
                    "largeChartUrl": "",
                    "isTransparent": true,
                    "showSymbolLogo": true,
                    "displayMode": "regular"
                }
                </script>
            </div>
            <!-- TradingView Widget END -->
        </div>

        <!-- Dashboard Page -->
        <main class="page-content active" id="page-dashboard">
            <div class="dashboard-container">
                <!-- World Clocks Section -->
                <section class="dashboard-section clocks-section clocks-section-top">
                    <h2 class="section-title">
                        <i class="fas fa-clock"></i>
                        World Clocks
                    </h2>
                    <div class="clocks-grid">
                        <div class="clock-card" data-city="New York">
                            <div class="clock-header">
                                <i class="fas fa-bars"></i>
                                <h3>New York</h3>
                            </div>
                            <div class="clock-display" id="clock-ny">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                            <div class="clock-mystical">
                                <div class="mystical-label">5 Change & Adventure</div>
                                <div class="mystical-description">A time for exploration and new experiences</div>
                            </div>
                        </div>
                        <div class="clock-card" data-city="London">
                            <div class="clock-header">
                                <i class="fas fa-bars"></i>
                                <h3>London</h3>
                            </div>
                            <div class="clock-display" id="clock-london">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                            <div class="clock-mystical">
                                <div class="mystical-label">5 Change & Adventure</div>
                                <div class="mystical-description">A time for exploration and new experiences</div>
                            </div>
                        </div>
                        <div class="clock-card" data-city="Tokyo">
                            <div class="clock-header">
                                <i class="fas fa-bars"></i>
                                <h3>Tokyo</h3>
                            </div>
                            <div class="clock-display" id="clock-tokyo">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                            <div class="clock-mystical">
                                <div class="mystical-label">6 Nurturing & Responsibility</div>
                                <div class="mystical-description">Focus on caring for others and home matters</div>
                            </div>
                        </div>
                        <div class="clock-card" data-city="Sydney">
                            <div class="clock-header">
                                <i class="fas fa-bars"></i>
                                <h3>Sydney</h3>
                            </div>
                            <div class="clock-display" id="clock-sydney">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                            <div class="clock-mystical">
                                <div class="mystical-label">6 Nurturing & Responsibility</div>
                                <div class="mystical-description">Focus on caring for others and home matters</div>
                            </div>
                        </div>
                    </div>
                </section>

                <!-- Market Data Section -->
                <section class="dashboard-section market-section market-section-full">
                    <div class="section-header">
                        <h2 class="section-title">Market</h2>
                        <div class="market-header-right">
                            <span class="last-updated-text">Last updated: <strong id="last-update">--</strong></span>
                            <button class="btn btn-secondary btn-sm" id="refresh-market-btn" title="Refresh Data">
                                Refresh
                            </button>
                        </div>
                    </div>
                    <div class="market-input-section">
                        <input type="text" id="symbol-input-inline" class="market-symbol-input" placeholder="Enter symbol (e.g., AAPL, TSLA, MSFT)">
                        <button class="btn btn-primary" id="add-symbol-btn-inline">
                            <i class="fas fa-plus"></i>
                            Add Symbol
                        </button>
                    </div>
                    <div class="market-grid" id="market-grid">
                        <!-- Market data cards will be loaded here -->
                    </div>
                </section>

                <!-- Today's To-Do List Section -->
                <section class="dashboard-section todo-section">
                    <h2 class="section-title">Today's To-Do List</h2>
                    <div class="todo-input-section-dashboard">
                        <input type="text" id="todo-input-dashboard" class="todo-input-dashboard" placeholder="Add a new task...">
                        <button class="btn btn-primary btn-sm" id="add-todo-btn-dashboard">
                            <i class="fas fa-plus"></i>
                            Add
                        </button>
                    </div>
                    <div class="todo-list-dashboard" id="todo-list-dashboard">
                        <div class="todo-empty-state">
                            <i class="fas fa-clipboard-list"></i>
                            <p>No tasks for today. Add one above to get started!</p>
                        </div>
                    </div>
                </section>

            </div>
        </main>

        <!-- News Page -->
        <main class="page-content" id="page-news">
            <div class="news-container">
                <div class="news-widgets-grid">
                    <!-- Top Stories Widget -->
                    <div class="news-widget-card">
                        <h2 class="widget-title">Top Stories</h2>
                        <div class="tradingview-widget-container">
                            <div class="tradingview-widget-container__widget"></div>
                            <div class="tradingview-widget-copyright">
                                <a href="https://www.tradingview.com/news/top-providers/tradingview/" rel="noopener nofollow" target="_blank">
                                    <span class="blue-text">Top stories</span>
                                </a>
                                <span class="trademark"> by TradingView</span>
                            </div>
                            <script type="text/javascript" src="https://s3.tradingview.com/external-embedding/embed-widget-timeline.js" async>
                            {
                                "displayMode": "regular",
                                "feedMode": "all_symbols",
                                "colorTheme": "dark",
                                "isTransparent": true,
                                "locale": "en",
                                "width": "100%",
                                "height": 550
                            }
                            </script>
                        </div>
                    </div>

                    <!-- Economic Calendar Widget -->
                    <div class="news-widget-card">
                        <h2 class="widget-title">Economic Calendar</h2>
                        <div class="tradingview-widget-container">
                            <div class="tradingview-widget-container__widget"></div>
                            <div class="tradingview-widget-copyright">
                                <a href="https://www.tradingview.com/economic-calendar/" rel="noopener nofollow" target="_blank">
                                    <span class="blue-text">Economic Calendar</span>
                                </a>
                                <span class="trademark"> by TradingView</span>
                            </div>
                            <script type="text/javascript" src="https://s3.tradingview.com/external-embedding/embed-widget-events.js" async>
                            {
                                "colorTheme": "dark",
                                "isTransparent": true,
                                "locale": "en",
                                "countryFilter": "ar,au,br,ca,cn,fr,de,in,id,it,jp,kr,mx,ru,sa,za,tr,gb,us,eu",
                                "importanceFilter": "0,1",
                                "width": "100%",
                                "height": 550
                            }
                            </script>
                        </div>
                    </div>
                </div>
                
                <!-- Market News Feed Section -->
                <div class="news-feed-section">
                    <div class="news-feed-header">
                        <h2 class="widget-title">Market News Feed</h2>
                        <div class="news-feed-controls">
                            <select id="news-category-select" class="news-category-select">
                                <option value="general">General</option>
                                <option value="forex">Forex</option>
                                <option value="crypto">Crypto</option>
                                <option value="merger">Merger</option>
                            </select>
                            <button class="btn btn-secondary btn-sm" id="refresh-news-btn" title="Refresh News">
                                <i class="fas fa-sync-alt"></i>
                                Refresh
                            </button>
                        </div>
                    </div>
                    <div class="news-feed-container" id="news-feed-container">
                        <div class="news-loading">
                            <i class="fas fa-spinner fa-spin"></i>
                            <span>Loading market news...</span>
                        </div>
                    </div>
                    <div class="news-pagination" id="news-pagination" style="display: none;">
                        <button class="pagination-btn" id="news-prev-btn" title="Previous Page">
                            <i class="fas fa-chevron-left"></i>
                        </button>
                        <div class="pagination-pages" id="news-pagination-pages"></div>
                        <button class="pagination-btn" id="news-next-btn" title="Next Page">
                            <i class="fas fa-chevron-right"></i>
                        </button>
                    </div>
                </div>
            </div>
        </main>

        <!-- Charts Page -->
        <main class="page-content" id="page-charts">
            <!-- Tab Navigation -->
            <div class="charts-tabs">
                <button class="chart-tab-btn active" data-tab="trading-charts">
                    <i class="fas fa-chart-line"></i>
                    Trading Charts
                </button>
                <button class="chart-tab-btn" data-tab="fibonacci">
                    <i class="fas fa-calculator"></i>
                    Fibonacci Calculator
                </button>
                <button class="chart-tab-btn" data-tab="projections">
                    <i class="fas fa-project-diagram"></i>
                    Price Projections
                </button>
            </div>
            
            <!-- Trading Charts Tab -->
            <section class="chart-tab-content active" id="tab-trading-charts">
            <section class="charts-page-content">
                <!-- Chart Controls -->
                <div class="chart-controls-panel">
                    <div class="chart-symbol-search">
                        <div class="symbol-input-wrapper">
                            <i class="fas fa-search"></i>
                            <input type="text" id="chart-symbol-input" class="chart-symbol-input" placeholder="Enter symbol (e.g., AAPL, SPY)">
                            <button class="btn btn-primary btn-sm" id="load-chart-btn">Load</button>
                        </div>
                    </div>
                    <div class="chart-timeframe-selector">
                        <button class="timeframe-btn" data-timeframe="15MIN">15 MIN</button>
                        <button class="timeframe-btn" data-timeframe="1H">1H</button>
                        <button class="timeframe-btn" data-timeframe="4H">4H</button>
                        <button class="timeframe-btn active" data-timeframe="1D">1D</button>
                    </div>
                    <div class="chart-type-selector">
                        <button class="chart-type-btn active" data-type="candlestick" title="Candlestick">
                            <i class="fas fa-chart-bar"></i>
                        </button>
                        <button class="chart-type-btn" data-type="line" title="Line">
                            <i class="fas fa-chart-line"></i>
                        </button>
                        <button class="chart-type-btn" data-type="area" title="Area">
                            <i class="fas fa-chart-area"></i>
                        </button>
                    </div>
                </div>

                <div class="charts-main-container">
                    <!-- Left Side - Chart Area -->
                    <div class="chart-area">
                        <!-- Enhanced Symbol Info Bar -->
                        <div class="chart-symbol-info-enhanced" id="chart-symbol-info">
                            <div class="symbol-header-row">
                                <div class="symbol-name-large">
                                    <span class="symbol-ticker" id="chart-ticker">--</span>
                                    <span class="symbol-company" id="chart-company">Select a symbol to view chart</span>
                                </div>
                                <div class="symbol-price-main">
                                    <span class="current-price" id="chart-current-price">--</span>
                                    <span class="price-change positive" id="chart-price-change">--</span>
                                </div>
                            </div>
                            <div class="symbol-details-row">
                                <div class="detail-item">
                                    <span class="detail-label">Open</span>
                                    <span class="detail-value" id="detail-open">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">High</span>
                                    <span class="detail-value positive" id="detail-high">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">Low</span>
                                    <span class="detail-value negative" id="detail-low">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">Prev Close</span>
                                    <span class="detail-value" id="detail-prev-close">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">Volume</span>
                                    <span class="detail-value" id="detail-volume">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">Day Range</span>
                                    <span class="detail-value" id="detail-day-range">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">52W Range</span>
                                    <span class="detail-value" id="detail-52w-range">--</span>
                                </div>
                                <div class="detail-item">
                                    <span class="detail-label">Avg Vol</span>
                                    <span class="detail-value" id="detail-avg-vol">--</span>
                                </div>
                            </div>
                        </div>

                        <!-- Main Chart -->
                        <div class="chart-container" id="main-chart-container">
                            <canvas id="main-chart"></canvas>
                            <div class="chart-zoom-controls">
                                <button class="zoom-btn" id="zoom-in-btn" title="Zoom In">
                                    <i class="fas fa-search-plus"></i>
                                </button>
                                <button class="zoom-btn" id="zoom-out-btn" title="Zoom Out">
                                    <i class="fas fa-search-minus"></i>
                                </button>
                                <button class="zoom-btn" id="zoom-reset-btn" title="Reset Zoom">
                                    <i class="fas fa-compress-arrows-alt"></i>
                                </button>
                            </div>
                            <div class="chart-loading" id="chart-loading">
                                <i class="fas fa-spinner fa-spin"></i>
                                <span>Loading chart data...</span>
                            </div>
                            <div class="chart-empty" id="chart-empty">
                                <i class="fas fa-chart-line"></i>
                                <p>Enter a symbol above to load chart</p>
                            </div>
                        </div>

                        <!-- Volume Chart -->
                        <div class="volume-chart-container" id="volume-chart-container">
                            <canvas id="volume-chart"></canvas>
                        </div>

                        <!-- Market Signals & Quick Actions - Under Chart -->
                        <div class="chart-bottom-panels">
                            <!-- Market Signals -->
                            <div class="analysis-section bottom-panel">
                                <h3 class="analysis-title">
                                    <i class="fas fa-signal"></i>
                                    Market Signals
                                </h3>
                                <div class="signals-grid">
                                    <div class="signal-card">
                                        <span class="signal-name">RSI (14)</span>
                                        <span class="signal-value" id="signal-rsi">--</span>
                                        <span class="signal-indicator neutral" id="signal-rsi-indicator">--</span>
                                    </div>
                                    <div class="signal-card">
                                        <span class="signal-name">MACD</span>
                                        <span class="signal-value" id="signal-macd">--</span>
                                        <span class="signal-indicator neutral" id="signal-macd-indicator">--</span>
                                    </div>
                                    <div class="signal-card">
                                        <span class="signal-name">Trend</span>
                                        <span class="signal-value" id="signal-trend">--</span>
                                        <span class="signal-indicator neutral" id="signal-trend-indicator">--</span>
                                    </div>
                                    <div class="signal-card">
                                        <span class="signal-name">Momentum</span>
                                        <span class="signal-value" id="signal-momentum">--</span>
                                        <span class="signal-indicator neutral" id="signal-momentum-indicator">--</span>
                                    </div>
                                </div>
                            </div>

                            <!-- Quick Actions -->
                            <div class="analysis-section bottom-panel">
                                <h3 class="analysis-title">
                                    <i class="fas fa-bolt"></i>
                                    Quick Actions
                                </h3>
                                <div class="quick-actions-row">
                                    <button class="btn btn-secondary btn-sm" id="refresh-chart-btn">
                                        <i class="fas fa-sync-alt"></i>
                                        Refresh
                                    </button>
                                    <button class="btn btn-secondary btn-sm" id="add-to-watchlist-btn">
                                        <i class="fas fa-star"></i>
                                        Watchlist
                                    </button>
                                    <button class="btn btn-secondary btn-sm" id="create-note-btn">
                                        <i class="fas fa-sticky-note"></i>
                                        Note
                                    </button>
                                    <button class="btn btn-secondary btn-sm" id="export-chart-btn">
                                        <i class="fas fa-download"></i>
                                        Export
                                    </button>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Right Side - Analysis Panel -->
                    <div class="chart-analysis-panel">
                        <!-- Price Statistics -->
                        <div class="analysis-section">
                            <h3 class="analysis-title">
                                <i class="fas fa-info-circle"></i>
                                Price Statistics
                            </h3>
                            <div class="stats-list">
                                <div class="stat-row">
                                    <span class="stat-label">Open</span>
                                    <span class="stat-value" id="stat-open">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">High</span>
                                    <span class="stat-value positive" id="stat-high">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">Low</span>
                                    <span class="stat-value negative" id="stat-low">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">Close</span>
                                    <span class="stat-value" id="stat-close">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">Volume</span>
                                    <span class="stat-value" id="stat-volume">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">Avg Volume</span>
                                    <span class="stat-value" id="stat-avg-volume">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">52W High</span>
                                    <span class="stat-value positive" id="stat-52w-high">--</span>
                                </div>
                                <div class="stat-row">
                                    <span class="stat-label">52W Low</span>
                                    <span class="stat-value negative" id="stat-52w-low">--</span>
                                </div>
                            </div>
                        </div>

                        <!-- Technical Indicators -->
                        <div class="analysis-section">
                            <h3 class="analysis-title">
                                <i class="fas fa-calculator"></i>
                                Technical Indicators
                            </h3>
                            <div class="indicators-controls">
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-sma20" data-indicator="sma20">
                                    <span class="toggle-label">SMA (20)</span>
                                    <span class="indicator-value" id="value-sma20">--</span>
                                </label>
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-sma50" data-indicator="sma50">
                                    <span class="toggle-label">SMA (50)</span>
                                    <span class="indicator-value" id="value-sma50">--</span>
                                </label>
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-ema12" data-indicator="ema12">
                                    <span class="toggle-label">EMA (12)</span>
                                    <span class="indicator-value" id="value-ema12">--</span>
                                </label>
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-ema26" data-indicator="ema26">
                                    <span class="toggle-label">EMA (26)</span>
                                    <span class="indicator-value" id="value-ema26">--</span>
                                </label>
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-bb" data-indicator="bollinger">
                                    <span class="toggle-label">Bollinger Bands</span>
                                    <span class="indicator-value" id="value-bb">--</span>
                                </label>
                                <label class="indicator-toggle">
                                    <input type="checkbox" id="indicator-volume" data-indicator="volume" checked>
                                    <span class="toggle-label">Volume</span>
                                    <span class="indicator-value" id="value-volume">--</span>
                                </label>
                            </div>
                        </div>

                        <!-- Saved Studies -->
                        <div class="analysis-section">
                            <h3 class="analysis-title">
                                <i class="fas fa-bookmark"></i>
                                Saved Studies
                            </h3>
                            <div class="saved-studies-controls">
                                <button class="btn btn-secondary btn-sm" id="save-study-btn">
                                    <i class="fas fa-bookmark"></i>
                                    Save Study
                                </button>
                                <div class="saved-studies-dropdown">
                                    <button class="btn btn-secondary btn-sm" id="toggle-studies-btn">
                                        <i class="fas fa-folder-open"></i>
                                        Saved Studies
                                        <i class="fas fa-chevron-down dropdown-arrow"></i>
                                    </button>
                                    <div class="saved-studies-panel" id="saved-studies-panel">
                                        <div class="studies-panel-header">
                                            <span>Your Saved Studies</span>
                                            <button class="btn btn-sm" id="close-studies-panel">
                                                <i class="fas fa-times"></i>
                                            </button>
                                        </div>
                                        <div class="studies-panel-content" id="studies-panel-content">
                                            <!-- Studies will be loaded here -->
                                        </div>
                                        <div class="studies-panel-empty" id="studies-panel-empty">
                                            <i class="fas fa-folder-open"></i>
                                            <p>No saved studies yet</p>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>

            </section>
            </section>
            
            <!-- Fibonacci Calculator Tab -->
            <section class="chart-tab-content" id="tab-fibonacci">
                <div class="fib-container">
                    <!-- Top Section: Market Data + Fibonacci Levels -->
                    <div class="fib-top-section">
                        <!-- Left Column: Market Data Input + Current Market Data -->
                        <div class="fib-left-column">
                            <div class="fib-card">
                                <h2>
                                    <i class="fas fa-chart-line"></i>
                                    Market Data Input
                                </h2>
                                
                                <div class="fib-input-group">
                                    <label for="fib-symbol">Symbol (Stock/Crypto)</label>
                                    <input type="text" id="fib-symbol" class="fib-input" placeholder="e.g., AAPL, TSLA, BTC-USD" value="AAPL">
                                    <small class="fib-hint">Enter a stock symbol (e.g., AAPL, GOOGL) or crypto symbol (e.g., BTC-USD, ETH-USD)</small>
                                </div>

                                <div class="fib-input-group">
                                    <label>Time Period</label>
                                    <div class="fib-info-highlight">
                                        <i class="fas fa-calendar"></i>
                                        Year to Date (YTD)
                                    </div>
                                </div>

                                <div class="fib-input-group">
                                    <label>Fibonacci Anchor</label>
                                    <div class="fib-info-highlight">
                                        <i class="fas fa-anchor"></i>
                                        First Trading Day of Year
                                    </div>
                                </div>

                                <div class="fib-input-group">
                                    <label>Decimal Precision</label>
                                    <div class="fib-toggle-group" id="fib-precision-toggle">
                                        <button class="fib-toggle-btn active" data-value="2">2</button>
                                        <button class="fib-toggle-btn" data-value="3">3</button>
                                        <button class="fib-toggle-btn" data-value="4">4</button>
                                        <button class="fib-toggle-btn" data-value="5">5</button>
                                    </div>
                                </div>

                                <button class="btn btn-primary" id="fib-calculate-btn">
                                    <i class="fas fa-calculator"></i>
                                    Calculate Fibonacci Levels
                                </button>

                                <div class="fib-info-box">
                                    <i class="fas fa-info-circle"></i>
                                    <p><strong>How to use:</strong> Enter a stock symbol (e.g., AAPL, GOOGL) or crypto symbol (e.g., BTC-USD, ETH-USD) and click calculate. Fibonacci levels will be anchored to the first trading day of the current year.</p>
                                </div>
                            </div>
                        </div>

                        <!-- Right Column: Fibonacci Extension Levels -->
                        <div class="fib-right-column" id="fib-levels-container" style="display: none;">
                            <div class="fib-card fib-levels-card">
                                <h2>
                                    <i class="fas fa-layer-group"></i>
                                    Fibonacci Extension Levels
                                </h2>
                                
                                <div class="fib-levels-container">
                                    <!-- Positive Levels - Column 1 -->
                                    <div class="fib-levels-column fib-positive-col-1">
                                        <h3>
                                            <i class="fas fa-arrow-up"></i>
                                            <span>↑ Positive Levels</span>
                                        </h3>
                                        <div id="fib-positive-levels-1" class="fib-levels-list"></div>
                                    </div>

                                    <!-- Positive Levels - Column 2 -->
                                    <div class="fib-levels-column fib-positive-col-2">
                                        <h3 class="fib-levels-col-header-hidden">
                                            <i class="fas fa-arrow-up"></i>
                                            <span>↑ Positive Levels</span>
                                        </h3>
                                        <div id="fib-positive-levels-2" class="fib-levels-list"></div>
                                    </div>

                                    <!-- Negative Levels - Column 1 -->
                                    <div class="fib-levels-column fib-negative-col-1">
                                        <h3>
                                            <i class="fas fa-arrow-down"></i>
                                            <span>↓ Negative Levels</span>
                                        </h3>
                                        <div id="fib-negative-levels-1" class="fib-levels-list"></div>
                                    </div>

                                    <!-- Negative Levels - Column 2 -->
                                    <div class="fib-levels-column fib-negative-col-2">
                                        <h3 class="fib-levels-col-header-hidden">
                                            <i class="fas fa-arrow-down"></i>
                                            <span>↓ Negative Levels</span>
                                        </h3>
                                        <div id="fib-negative-levels-2" class="fib-levels-list"></div>
                                    </div>
                                </div>

                                <div class="fib-info-box">
                                    <i class="fas fa-info-circle"></i>
                                    <p><strong>Fibonacci Anchoring:</strong> BULLISH candle: 0 = Low, 1 = High. BEARISH candle: 0 = High, 1 = Low. Levels extend from these anchor points.</p>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Bottom Section: Price Chart (Full Width) -->
                    <div class="fib-chart-section" id="fib-chart-container" style="display: none;">
                        <div class="fib-card fib-chart-card">
                            <!-- Current Market Data - At Top of Chart -->
                            <div class="fib-market-data-inline" id="fib-market-data" style="display: none;">
                                <div class="fib-price-info-inline">
                                    <div class="fib-market-data-header">
                                        <h3 id="fib-symbol-name">Loading...</h3>
                                        <div class="fib-market-data-header-right">
                                            <div class="fib-chart-controls-inline">
                                                <button class="fib-chart-type-btn active" id="fib-chart-type-line-inline" title="Line Chart">
                                                    <i class="fas fa-chart-line"></i>
                                                </button>
                                                <button class="fib-chart-type-btn" id="fib-chart-type-candlestick-inline" title="Candlestick Chart">
                                                    <i class="fas fa-chart-bar"></i>
                                                </button>
                                            </div>
                                            <div class="fib-market-data-stats">
                                                <div class="fib-market-stat">
                                                    <span class="fib-stat-label">Current Price:</span>
                                                    <span class="fib-stat-value" id="fib-current-price">-</span>
                                                </div>
                                                <div class="fib-market-stat">
                                                    <span class="fib-stat-label">Period High:</span>
                                                    <span class="fib-stat-value" id="fib-ytd-high">-</span>
                                                </div>
                                                <div class="fib-market-stat">
                                                    <span class="fib-stat-label">Period Low:</span>
                                                    <span class="fib-stat-value" id="fib-ytd-low">-</span>
                                                </div>
                                                <div class="fib-market-stat">
                                                    <span class="fib-stat-label">Range:</span>
                                                    <span class="fib-stat-value" id="fib-range">-</span>
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                    <div id="fib-anchor-info" class="fib-anchor-info-inline"></div>
                                </div>
                            </div>
                            
                            <div class="fib-chart-wrapper">
                                <canvas id="fib-chart"></canvas>
                                <div id="fib-chart-loading" class="fib-chart-loading" style="display: none;">
                                    <i class="fas fa-spinner fa-spin"></i>
                                    <span>Loading chart...</span>
                                </div>
                                <div id="fib-chart-empty" class="fib-chart-empty">
                                    <i class="fas fa-chart-line"></i>
                                    <p>Chart will appear here after calculation</p>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Loading & Error States -->
                    <div id="fib-loading" class="fib-loading" style="display: none;">
                        <i class="fas fa-spinner fa-spin"></i>
                        <span>Fetching market data...</span>
                    </div>
                    <div id="fib-error" class="fib-error" style="display: none;"></div>
                </div>
            </section>
            
            <!-- Price Projections Tab -->
            <section class="chart-tab-content" id="tab-projections">
                <div class="projections-container">
                    <!-- Search and Controls Section (Matching Trading Charts Tab) -->
                    <div class="chart-controls-panel">
                        <div class="chart-symbol-search">
                            <div class="symbol-input-wrapper">
                                <i class="fas fa-search"></i>
                                <input type="text" id="projection-symbol-input" class="chart-symbol-input" placeholder="Enter symbol (e.g., AAPL, SPY)">
                                <button class="btn btn-primary btn-sm" id="projection-load-btn">Load</button>
                            </div>
                        </div>
                        <div class="chart-timeframe-selector">
                            <button class="timeframe-btn" data-timeframe="15MIN" disabled style="opacity: 0.5; cursor: not-allowed;">15 MIN</button>
                            <button class="timeframe-btn active" data-timeframe="1H">1H</button>
                            <button class="timeframe-btn" data-timeframe="4H" disabled style="opacity: 0.5; cursor: not-allowed;">4H</button>
                            <button class="timeframe-btn" data-timeframe="1D" disabled style="opacity: 0.5; cursor: not-allowed;">1D</button>
                        </div>
                        <button id="projection-refresh-btn" class="btn btn-secondary btn-sm" style="display: none;">
                            <i class="fas fa-sync-alt"></i>
                            Refresh
                        </button>
                    </div>
                        
                        <!-- Two Column Layout -->
                        <div class="projections-two-column-layout">
                            <!-- Column 1: Projection Configuration -->
                            <div class="projections-column-left">
                                <div class="projection-config-panel">
                                    <h3>Projection Configuration</h3>
                                    
                                    <!-- Tab Navigation -->
                                    <div class="projection-tabs">
                                        <button class="projection-tab-btn active" data-tab="parameters">
                                            <i class="fas fa-sliders-h"></i>
                                            Parameters
                                        </button>
                                        <button class="projection-tab-btn" data-tab="hyperdimensional">
                                            <i class="fas fa-cube"></i>
                                            Hyper-Dimensional
                                        </button>
                                        <button class="projection-tab-btn" data-tab="results">
                                            <i class="fas fa-chart-line"></i>
                                            Results
                                        </button>
                                    </div>
                            
                            <!-- Parameters Tab Content -->
                            <div class="projection-tab-content active" id="projection-tab-parameters">
                                <div class="projection-parameters-content">
                                    
                                    <!-- Computation Method Status -->
                                    <div id="projection-computation-info" style="background: var(--card-bg, #2a2a2a); border: 1px solid var(--border-color, #444); border-radius: 8px; padding: 12px; margin-bottom: 16px; display: none;">
                                        <div style="display: flex; align-items: center; gap: 10px; font-size: 13px;">
                                            <span id="computation-method-icon" style="font-size: 18px;">📊</span>
                                            <div style="flex: 1;">
                                                <div style="font-weight: 600; color: var(--text-color, #e0e0e0);">
                                                    Computation Method: <span id="computation-method-text">--</span>
                                                </div>
                                                <div id="computation-88d-badge" style="font-size: 11px; margin-top: 4px; color: #22c55e; display: none;">
                                                    ⚡ 88D Threading Active (96 threads, 8 layers × 12 dimensions)
                                                </div>
                                            </div>
                                        </div>
                                    </div>
                                    
                                    <div class="params-toggle-grid">
                                        <!-- Standard Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Standard</span>
                                                    <span class="param-toggle-desc">Steps: 20, Base: 3, Count: 12, Depth: 31</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-standard" value="standard" checked>
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                        
                                        <!-- Extended Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Extended</span>
                                                    <span class="param-toggle-desc">Steps: 40, Base: 3.5, Count: 18, Depth: 47</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-extended" value="extended">
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                        
                                        <!-- Deep Analysis Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Deep Analysis</span>
                                                    <span class="param-toggle-desc">Steps: 60, Base: 4, Count: 24, Depth: 61</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-deep" value="deep">
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                        
                                        <!-- Quick Preview Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Quick Preview</span>
                                                    <span class="param-toggle-desc">Steps: 10, Base: 2.5, Count: 6, Depth: 17</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-quick" value="quick">
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                        
                                        <!-- Maximum Precision Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Maximum Precision</span>
                                                    <span class="param-toggle-desc">Steps: 100, Base: 5, Count: 36, Depth: 97</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-maximum" value="maximum">
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                        
                                        <!-- Custom Preset -->
                                        <div class="param-toggle-item">
                                            <div class="param-toggle-header">
                                                <label class="param-toggle-label">
                                                    <span class="param-toggle-name">Custom</span>
                                                    <span class="param-toggle-desc">Set your own parameters</span>
                                                </label>
                                                <label class="toggle-switch">
                                                    <input type="radio" name="projection-preset" id="preset-custom" value="custom">
                                                    <span class="toggle-slider"></span>
                                                </label>
                                            </div>
                                        </div>
                                    </div>
                                    
                                    <!-- Custom Parameters (hidden by default, shown when Custom is selected) -->
                                    <div class="custom-params-panel" id="custom-params-panel" style="display: none;">
                                        <div class="params-grid">
                                            <div class="param-group">
                                                <label for="projection-steps">Projection Steps</label>
                                                <input type="number" id="projection-steps" class="projection-input" min="1" max="200" value="20">
                                            </div>
                                            <div class="param-group">
                                                <label for="projection-base">Base</label>
                                                <input type="number" id="projection-base" class="projection-input" min="2" max="10" value="3" step="0.1">
                                            </div>
                                            <div class="param-group">
                                                <label for="projection-count">Projection Count</label>
                                                <input type="number" id="projection-count" class="projection-input" min="1" max="50" value="12">
                                            </div>
                                            <div class="param-group">
                                                <label for="projection-depth">Prime Depth</label>
                                                <select id="projection-depth" class="projection-select">
                                                    <option value="11">11</option>
                                                    <option value="13">13</option>
                                                    <option value="17">17</option>
                                                    <option value="29">29</option>
                                                    <option value="31" selected>31</option>
                                                    <option value="47">47</option>
                                                    <option value="59">59</option>
                                                    <option value="61">61</option>
                                                    <option value="97">97</option>
                                                    <option value="101">101</option>
                                                </select>
                                            </div>
                                        </div>
                                    </div>
                                    
                                    <!-- Hidden inputs to store current values (for compatibility) -->
                                    <input type="hidden" id="projection-steps-value" value="20">
                                    <input type="hidden" id="projection-base-value" value="3">
                                    <input type="hidden" id="projection-count-value" value="12">
                                    <input type="hidden" id="projection-depth-value" value="31">
                                    
                                    <!-- Active Parameters Display -->
                                    <div class="active-params-display" id="active-params-display" style="display: none;">
                                        <div class="active-params-label">Active Parameters:</div>
                                        <div class="active-params-values">
                                            <span class="param-badge">Steps: <strong id="active-steps">20</strong></span>
                                            <span class="param-badge">Base: <strong id="active-base">3</strong></span>
                                            <span class="param-badge">Count: <strong id="active-count">12</strong></span>
                                            <span class="param-badge">Depth: <strong id="active-depth">31</strong></span>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            
                            <!-- Hyper-Dimensional Tab Content -->
                            <div class="projection-tab-content" id="projection-tab-hyperdimensional">
                                <div class="hyperdimensional-content">
                                    <p class="hd-integration-note">
                                        <i class="fas fa-info-circle"></i>
                                        Tetration towers and platonic solids are now fully integrated into each step of the projection calculation, ensuring geometrically consistent models.
                                    </p>
                                    <div class="hd-tools-grid">
                                        <div class="hd-tool-item">
                                            <label class="hd-tool-label">
                                                <span class="hd-tool-name">Tetration Towers</span>
                                                <span class="hd-tool-desc">Generate tetration towers - influences growth factor, theta, and price attractors at each step</span>
                                            </label>
                                            <button class="btn btn-small" id="generate-tetration-towers-btn">
                                                <i class="fas fa-cube"></i>
                                                Generate
                                            </button>
                                        </div>
                                        <div class="hd-tool-item">
                                            <label class="hd-tool-label">
                                                <span class="hd-tool-name">Platonic Solids</span>
                                                <span class="hd-tool-desc">Generate geometric structures - influences lattice angles and scaling at each step</span>
                                            </label>
                                            <button class="btn btn-small" id="generate-platonic-solids-btn">
                                                <i class="fas fa-shapes"></i>
                                                Generate
                                            </button>
                                        </div>
                                        <div class="hd-tool-item">
                                            <label class="hd-tool-label">
                                                <span class="hd-tool-name">Discover New Solids</span>
                                                <span class="hd-tool-desc">Use tetration attractors to discover new structures - automatically integrated into step calculations</span>
                                            </label>
                                            <button class="btn btn-small" id="discover-solids-btn">
                                                <i class="fas fa-search"></i>
                                                Discover
                                            </button>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            
                            <!-- Results Tab Content -->
                            <div class="projection-tab-content" id="projection-tab-results">
                                <div class="hd-results-panel" id="hd-results-panel">
                                    <div id="hd-results-content">
                                        <div class="hd-empty-state">
                                            <i class="fas fa-info-circle"></i>
                                            <p>No results yet. Generate tetration towers or platonic solids to see results here.</p>
                                        </div>
                                    </div>
                                </div>
                            </div>
                                </div>
                            </div>
                            
                            <!-- Column 2: Metrics -->
                            <div class="projections-column-right">
                                <!-- Metrics Section (Visible by default, shows placeholders until data is loaded) -->
                                <div class="projections-metrics-section" id="projections-metrics-section">
                                    <h3>Price Metrics</h3>
                                    <div class="metrics-grid">
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Current Price</div>
                                            <div class="metric-value" id="metric-current-price">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Historical Change</div>
                                            <div class="metric-value" id="metric-historical-change">--</div>
                                            <div class="metric-percent" id="metric-historical-percent">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Projected Price</div>
                                            <div class="metric-value" id="metric-projected-price">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Projected Change</div>
                                            <div class="metric-value" id="metric-projected-change">--</div>
                                            <div class="metric-percent" id="metric-projected-percent">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Period High</div>
                                            <div class="metric-value" id="metric-period-high">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Period Low</div>
                                            <div class="metric-value" id="metric-period-low">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Average Volume</div>
                                            <div class="metric-value" id="metric-average-volume">--</div>
                                        </div>
                                        <div class="metric-card-projection">
                                            <div class="metric-label">Actual Volume</div>
                                            <div class="metric-value" id="metric-actual-volume">--</div>
                                        </div>
                                    </div>
                                    
                                    <!-- Validation Metrics Section (Visible by default, shows placeholders until data is loaded) -->
                                    <div class="validation-metrics-section" id="validation-metrics-section">
                                        <h3>Validation Metrics</h3>
                                        <div class="metrics-grid">
                                            <div class="metric-card-projection">
                                                <div class="metric-label">MAE</div>
                                                <div class="metric-value" id="validation-mae">--</div>
                                                <div class="metric-description">Mean Absolute Error</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">RMSE</div>
                                                <div class="metric-value" id="validation-rmse">--</div>
                                                <div class="metric-description">Root Mean Squared Error</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">MAPE</div>
                                                <div class="metric-value" id="validation-mape">--</div>
                                                <div class="metric-description">Mean Absolute Percentage Error</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">Confidence</div>
                                                <div class="metric-value" id="validation-confidence">--</div>
                                                <div class="metric-description">Model Confidence Score</div>
                                            </div>
                                        </div>
                                    </div>
                                    
                                    <!-- Accuracy Metrics Section (Shown when comparing saved projection to actual prices) -->
                                    <div class="validation-metrics-section" id="accuracy-metrics-section" style="display: none;">
                                        <h3>Projection Accuracy (vs Actual Price)</h3>
                                        <div class="metrics-grid">
                                            <div class="metric-card-projection">
                                                <div class="metric-label">MAE</div>
                                                <div class="metric-value" id="accuracy-mae">--</div>
                                                <div class="metric-description">Mean Absolute Error</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">MAPE</div>
                                                <div class="metric-value" id="accuracy-mape">--</div>
                                                <div class="metric-description">Mean Absolute % Error</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">Bias</div>
                                                <div class="metric-value" id="accuracy-bias">--</div>
                                                <div class="metric-description">Over/Under Estimate</div>
                                            </div>
                                            <div class="metric-card-projection">
                                                <div class="metric-label">Max Error</div>
                                                <div class="metric-value" id="accuracy-max-error">--</div>
                                                <div class="metric-description">Maximum Deviation</div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    
                    <!-- Chart Section -->
                    <div class="projections-chart-section">
                        <div class="projections-chart-container">
                            <div class="chart-header">
                                <h3 id="projection-chart-title">Price Projection Chart</h3>
                                <div class="chart-controls">
                                    <div class="chart-zoom-controls">
                                        <button id="zoom-in-btn" class="btn btn-small" title="Zoom In" style="display: none;">
                                            <i class="fas fa-search-plus"></i>
                                        </button>
                                        <button id="zoom-out-btn" class="btn btn-small" title="Zoom Out" style="display: none;">
                                            <i class="fas fa-search-minus"></i>
                                        </button>
                                        <button id="reset-zoom-btn" class="btn btn-small" title="Reset Zoom">
                                            <i class="fas fa-expand-arrows-alt"></i>
                                            Reset Zoom
                                        </button>
                                    </div>
                                    <div class="chart-export-controls">
                                        <button id="export-chart-btn" class="btn btn-small" title="Export Chart" type="button">
                                            <i class="fas fa-download"></i>
                                            Export
                                        </button>
                                    </div>
                                </div>
                            </div>
                            <div class="chart-legend-controls" id="chart-legend-controls" style="display: none;">
                                <div class="legend-header">
                                    <h4>Toggle Data Series</h4>
                                    <div class="chart-help-text">
                                        <i class="fas fa-info-circle"></i>
                                        <span>Click legend items or checkboxes to show/hide series. Hold Shift and drag to select zoom area.</span>
                                    </div>
                                </div>
                                <div class="legend-checkboxes" id="legend-checkboxes"></div>
                            </div>
                            <div class="chart-wrapper">
                                <canvas id="projection-chart"></canvas>
                                <div id="projection-loading" class="projection-loading" style="display: none;">
                                    <div class="loading-spinner"></div>
                                    <p>Loading projection data...</p>
                                </div>
                                <div id="projection-error" class="projection-error" style="display: none;">
                                    <i class="fas fa-exclamation-triangle"></i>
                                    <p></p>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </section>
        </main>

        <!-- Notes Page -->
        <main class="page-content" id="page-notes">
            <div class="notes-page-redesign">
                <!-- Header Section -->
                <div class="notes-header">
                    <div class="notes-header-content">
                        <h1 class="notes-page-title">
                            <i class="fas fa-sticky-note"></i>
                            Notes
                        </h1>
                        <div class="notes-header-actions">
                            <div class="notes-view-toggle">
                                <button class="view-toggle-btn active" data-view="grid" title="Grid View">
                                    <i class="fas fa-th"></i>
                                </button>
                                <button class="view-toggle-btn" data-view="list" title="List View">
                                    <i class="fas fa-list"></i>
                                </button>
                            </div>
                        </div>
                    </div>
                    
                    <!-- Enhanced Search Bar -->
                    <div class="notes-search-bar-modern">
                        <div class="search-input-wrapper-modern">
                            <i class="fas fa-search search-icon"></i>
                            <input type="text" id="notes-search-input" class="notes-search-input-modern" placeholder="Search notes...">
                            <button class="search-clear-btn-modern" id="notes-search-clear" style="display: none;">
                                <i class="fas fa-times"></i>
                            </button>
                        </div>
                    </div>
                </div>

                <!-- Quick Note Input - Modern Design -->
                <div class="quick-note-modern-container">
                    <div class="quick-note-modern" id="quick-note-input">
                        <div>
                            <div class="quick-note-icon">
                                <i class="fas fa-plus"></i>
                            </div>
                            <input type="text" id="quick-note-title" placeholder="Take a note..." class="quick-note-title-modern">
                        </div>
                        <div class="quick-note-expanded-modern" id="quick-note-expanded" style="display: none;">
                            <textarea id="quick-note-content" placeholder="Note content..." class="quick-note-content-modern"></textarea>
                            <div class="quick-note-footer-modern">
                                <div class="quick-note-colors-modern">
                                    <span class="color-label">Color:</span>
                                    <button class="color-btn-modern active" data-color="#2563eb" style="background-color: #2563eb;" title="Blue">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-btn-modern" data-color="#eab308" style="background-color: #eab308;" title="Yellow">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-btn-modern" data-color="#dc2626" style="background-color: #dc2626;" title="Red">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-btn-modern" data-color="#059669" style="background-color: #059669;" title="Green">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-btn-modern" data-color="#9333ea" style="background-color: #9333ea;" title="Purple">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-btn-modern" data-color="#db2777" style="background-color: #db2777;" title="Pink">
                                        <i class="fas fa-check"></i>
                                    </button>
                                </div>
                                <div class="quick-note-buttons-modern">
                                    <button class="quick-note-close-modern" id="quick-note-close">
                                        Cancel
                                    </button>
                                    <button class="quick-note-save-modern" id="quick-note-save">
                                        <i class="fas fa-check"></i>
                                        Save
                                    </button>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- Notes Grid -->
                <div class="notes-grid-modern-wrapper">
                    <div class="notes-grid-modern" id="notes-grid">
                        <!-- Notes will be loaded here -->
                    </div>
                </div>
            </div>

            <!-- Note Edit Modal - Modern Design -->
            <div class="note-modal-modern" id="note-modal">
                <div class="note-modal-overlay"></div>
                <div class="note-modal-content-modern">
                    <div class="note-modal-header-modern">
                        <h2 id="note-modal-title">
                            <i class="fas fa-edit"></i>
                            Edit Note
                        </h2>
                        <button class="note-modal-close-modern" id="close-note-modal">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="note-modal-body-modern">
                        <div class="note-input-group-modern">
                            <input type="text" id="note-title-input" class="note-title-input-modern" placeholder="Title">
                        </div>
                        <div class="note-input-group-modern">
                            <textarea id="note-content-input" class="note-content-input-modern" placeholder="Note content..."></textarea>
                        </div>
                        
                        <div class="note-modal-options-modern">
                            <div class="note-option-group-modern">
                                <label class="option-label">
                                    <i class="fas fa-palette"></i>
                                    Color
                                </label>
                                <div class="color-options-modern">
                                    <button class="color-option-modern active" data-color="#2563eb" style="background-color: #2563eb;" title="Blue">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-option-modern" data-color="#eab308" style="background-color: #eab308;" title="Yellow">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-option-modern" data-color="#dc2626" style="background-color: #dc2626;" title="Red">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-option-modern" data-color="#059669" style="background-color: #059669;" title="Green">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-option-modern" data-color="#9333ea" style="background-color: #9333ea;" title="Purple">
                                        <i class="fas fa-check"></i>
                                    </button>
                                    <button class="color-option-modern" data-color="#db2777" style="background-color: #db2777;" title="Pink">
                                        <i class="fas fa-check"></i>
                                    </button>
                                </div>
                            </div>
                            <div class="note-option-group-modern">
                                <label class="option-label">
                                    <i class="fas fa-tags"></i>
                                    Labels
                                </label>
                                <div class="labels-input-wrapper-modern">
                                    <input type="text" id="note-labels-input" class="labels-input-modern" placeholder="Add labels (comma separated)">
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="note-modal-footer-modern">
                        <button class="btn-modern btn-secondary-modern" id="cancel-note-btn">
                            <i class="fas fa-times"></i>
                            Cancel
                        </button>
                        <button class="btn-modern btn-primary-modern" id="save-note-btn">
                            <i class="fas fa-check"></i>
                            Save
                        </button>
                    </div>
                </div>
            </div>
        </main>

        <!-- To Do List Page -->
        <main class="page-content" id="page-todo">
            <div class="page-header">
                <h1 class="page-title">
                    <i class="fas fa-tasks"></i>
                    Trading Task Manager
                </h1>
            </div>
            
            <!-- Trading Metrics Dashboard -->
            <div class="trading-metrics-dashboard">
                <div class="metric-card market-status-card">
                    <div class="metric-header">
                        <i class="fas fa-clock"></i>
                        <span>Market Status</span>
                    </div>
                    <div class="metric-content">
                        <div class="current-time-display" id="current-time-display">--:--:--</div>
                        <div class="market-status" id="market-status">Market Closed</div>
                        <div class="market-countdown" id="market-countdown">--</div>
                    </div>
                </div>
                
                <div class="metric-card trading-session-card">
                    <div class="metric-header">
                        <i class="fas fa-chart-line"></i>
                        <span>Trading Session</span>
                    </div>
                    <div class="metric-content">
                        <div class="session-info" id="session-info">Pre-Market</div>
                        <div class="session-time" id="session-time">4:00 AM - 9:30 AM ET</div>
                        <div class="session-progress">
                            <div class="progress-bar">
                                <div class="progress-fill" id="session-progress"></div>
                            </div>
                        </div>
                    </div>
                </div>
                
                <div class="metric-card task-stats-card">
                    <div class="metric-header">
                        <i class="fas fa-check-circle"></i>
                        <span>Today's Progress</span>
                    </div>
                    <div class="metric-content">
                        <div class="stat-large" id="stat-today-completed">0</div>
                        <div class="stat-label">Completed</div>
                        <div class="stat-small">
                            <span id="stat-today-total">0</span> total tasks
                        </div>
                    </div>
                </div>
                
                <div class="metric-card productivity-card">
                    <div class="metric-header">
                        <i class="fas fa-fire"></i>
                        <span>Productivity Score</span>
                    </div>
                    <div class="metric-content">
                        <div class="score-circle" id="productivity-score">
                            <svg class="score-svg" viewBox="0 0 100 100">
                                <circle class="score-bg" cx="50" cy="50" r="45"></circle>
                                <circle class="score-fill" cx="50" cy="50" r="45" id="score-circle"></circle>
                            </svg>
                            <div class="score-text" id="score-text">0%</div>
                        </div>
                    </div>
                </div>
            </div>
            
            <!-- Main Content Grid -->
            <div class="todo-page-grid">
                <!-- Left Column: Task Management -->
                <div class="todo-management-panel">
                    <div class="panel-header">
                        <h2>
                            <i class="fas fa-list-check"></i>
                            Task Management
                        </h2>
                        <div class="view-toggle">
                            <button class="view-btn active" data-view="list" title="List View">
                                <i class="fas fa-list"></i>
                            </button>
                            <button class="view-btn" data-view="grid" title="Grid View">
                                <i class="fas fa-th"></i>
                            </button>
                        </div>
                    </div>
                    
                    <div class="todo-filters-enhanced">
                        <div class="filter-group">
                            <label>Status</label>
                            <div class="filter-buttons">
                                <button class="filter-btn active" data-filter="all">All</button>
                                <button class="filter-btn" data-filter="daily">Daily</button>
                                <button class="filter-btn" data-filter="weekly">Weekly</button>
                                <button class="filter-btn" data-filter="completed">Done</button>
                            </div>
                        </div>
                        <div class="filter-group">
                            <label>Priority</label>
                            <div class="filter-buttons">
                                <button class="filter-btn" data-priority="all">All</button>
                                <button class="filter-btn" data-priority="high">High</button>
                                <button class="filter-btn" data-priority="medium">Med</button>
                                <button class="filter-btn" data-priority="low">Low</button>
                            </div>
                        </div>
                    </div>
                    
                    <div class="todo-input-section-enhanced">
                        <div class="input-wrapper">
                            <i class="fas fa-plus-circle"></i>
                            <input type="text" id="todo-input" class="todo-input" placeholder="Add a new trading task...">
                        </div>
                        <div class="input-options">
                            <select id="todo-type" class="todo-type-select">
                                <option value="daily">Daily</option>
                                <option value="weekly">Weekly</option>
                            </select>
                            <select id="todo-priority" class="todo-priority-select">
                                <option value="medium">Medium</option>
                                <option value="high">High</option>
                                <option value="low">Low</option>
                            </select>
                            <button class="btn btn-primary" id="add-todo-btn">
                                <i class="fas fa-plus"></i>
                                Add Task
                            </button>
                        </div>
                    </div>
                    
                    <div class="todo-list-container">
                        <div class="todo-list" id="todo-list">
                            <!-- Todo items will be loaded here -->
                        </div>
                    </div>
                </div>
                
                <!-- Right Column: 3D Visualization & Analytics -->
                <div class="todo-visualization-panel">
                    <div class="panel-header">
                        <h2>
                            <i class="fas fa-cube"></i>
                            3D Analytics
                        </h2>
                        <div class="viz-controls">
                            <button class="viz-btn" id="rotate-toggle" title="Auto Rotate">
                                <i class="fas fa-sync-alt"></i>
                            </button>
                            <button class="viz-btn" id="reset-view" title="Reset View">
                                <i class="fas fa-home"></i>
                            </button>
                        </div>
                    </div>
                    
                    <div class="viz-container-wrapper">
                        <div class="todo-3d-container" id="todo-3d-container">
                            <canvas id="todo-3d-canvas"></canvas>
                            <div class="viz-loading" id="viz-loading">
                                <i class="fas fa-spinner fa-spin"></i>
                                <span>Loading 3D visualization...</span>
                            </div>
                            <div class="viz-overlay">
                                <div class="viz-info">
                                    <div class="viz-stat">
                                        <span class="viz-label">Tasks Completed</span>
                                        <span class="viz-value" id="viz-completed">0</span>
                                    </div>
                                    <div class="viz-stat">
                                        <span class="viz-label">Active Tasks</span>
                                        <span class="viz-value" id="viz-active">0</span>
                                    </div>
                                    <div class="viz-stat">
                                        <span class="viz-label">Completion Rate</span>
                                        <span class="viz-value" id="viz-rate">0%</span>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    <div class="analytics-section">
                        <h3>
                            <i class="fas fa-chart-bar"></i>
                            Performance Metrics
                        </h3>
                        <div class="analytics-grid">
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-calendar-check"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-weekly">0</div>
                                    <div class="analytics-label">This Week</div>
                                </div>
                            </div>
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-trophy"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-streak">0</div>
                                    <div class="analytics-label">Day Streak</div>
                                </div>
                            </div>
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-bolt"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-avg">0</div>
                                    <div class="analytics-label">Avg/Day</div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </main>

        <!-- API Management Dashboard Page -->
        <main class="page-content" id="page-api">
            <div class="api-dashboard-container">
                <!-- Header -->
                <div class="api-dashboard-header">
                    <div class="api-header-content">
                        <h1 class="api-page-title">
                            <i class="fas fa-plug"></i>
                            API Management Dashboard
                        </h1>
                        <div class="api-header-actions">
                            <button class="btn-api-refresh" id="api-refresh-all-btn" title="Refresh All APIs">
                                <i class="fas fa-sync-alt"></i>
                                Refresh
                            </button>
                        </div>
                    </div>
                    <p class="api-page-subtitle">Monitor and manage your connected APIs, view real-time metrics, and control data sources</p>
                </div>

                <!-- API Status Overview Cards -->
                <div class="api-overview-grid" id="api-overview-grid">
                    <!-- Cards will be dynamically generated -->
                </div>

                <!-- API Details Section -->
                <div class="api-details-section">
                    <div class="api-tabs">
                        <button class="api-tab active" data-tab="status">
                            <i class="fas fa-chart-line"></i>
                            Status & Metrics
                        </button>
                        <button class="api-tab" data-tab="configuration">
                            <i class="fas fa-cog"></i>
                            Configuration
                        </button>
                        <button class="api-tab" data-tab="history">
                            <i class="fas fa-history"></i>
                            Usage History
                        </button>
                    </div>

                    <!-- Status & Metrics Tab -->
                    <div class="api-tab-content active" id="tab-status">
                        <div class="api-metrics-grid" id="api-metrics-grid">
                            <!-- Metrics will be dynamically generated -->
                        </div>
                    </div>

                    <!-- Configuration Tab -->
                    <div class="api-tab-content" id="tab-configuration">
                        <div class="api-config-section">
                            <h3 class="api-section-title">
                                <i class="fas fa-key"></i>
                                API Keys & Settings
                            </h3>
                            <div class="api-settings-form">
                                <div class="api-setting-item">
                                    <div class="api-setting-header">
                                        <div class="api-setting-header-left">
                                            <label class="api-setting-label">Finnhub API Key</label>
                                            <span class="api-key-status" id="api-finnhub-key-status" style="display: none;">
                                                <i class="fas fa-check-circle"></i>
                                                <span>Configured</span>
                                            </span>
                                        </div>
                                        <a href="https://finnhub.io/register" target="_blank" class="api-setting-link">
                                            <i class="fas fa-external-link-alt"></i> Get Free API Key
                                        </a>
                                    </div>
                                    <div class="api-key-container">
                                        <input type="password" id="api-finnhub-key" class="api-key-input-field" placeholder="Enter your Finnhub API key">
                                        <button type="button" class="api-key-toggle" id="api-toggle-finnhub-key" title="Show/Hide API Key">
                                            <i class="fas fa-eye"></i>
                                        </button>
                                    </div>
                                    <p class="api-setting-hint">Required for accessing Finnhub API endpoints. Get your free API key from <a href="https://finnhub.io/register" target="_blank">finnhub.io</a>. Free tier includes 60 calls per minute.</p>
                                </div>
                                
                                <div class="api-setting-item">
                                    <label class="api-setting-label">Default Data Source</label>
                                    <select id="api-default-source" class="api-key-input-field">
                                        <option value="finnhub">Finnhub (Recommended)</option>
                                        <option value="yahoo">Yahoo Finance (Backup)</option>
                                    </select>
                                    <p class="api-setting-hint">Primary API for fetching market data. Yahoo Finance is used as automatic backup.</p>
                                </div>
                                
                                <div class="api-setting-item">
                                    <label class="api-setting-label">Auto-refresh Interval</label>
                                    <div class="api-range-container">
                                        <input type="range" id="api-refresh-interval-range" class="api-range-input" min="30" max="300" step="30" value="60">
                                        <input type="number" id="api-refresh-interval" class="api-key-input-field api-range-number" min="30" max="300" value="60">
                                        <span class="api-range-unit">seconds</span>
                                    </div>
                                    <p class="api-setting-hint">How often to automatically refresh market data. Lower values use more API calls.</p>
                                </div>
                                
                                <div class="api-setting-actions">
                                    <button class="btn btn-primary" id="api-save-settings-btn">
                                        <i class="fas fa-save"></i>
                                        Save API Settings
                                    </button>
                                </div>
                            </div>
                        </div>
                        <div class="api-config-section">
                            <h3 class="api-section-title">
                                <i class="fas fa-sliders-h"></i>
                                API Source Selection
                            </h3>
                            <div class="api-source-selector" id="api-source-selector">
                                <!-- Source selector will be generated -->
                            </div>
                        </div>
                        <div class="api-config-section">
                            <div class="api-section-header-with-action">
                                <h3 class="api-section-title">
                                    <i class="fas fa-plus-circle"></i>
                                    Custom APIs
                                </h3>
                                <button class="btn btn-primary btn-sm" id="add-custom-api-btn">
                                    <i class="fas fa-plus"></i>
                                    Add Custom API
                                </button>
                            </div>
                            <p class="api-section-description">Add and configure custom API sources for market data</p>
                            <div class="custom-apis-list" id="custom-apis-list">
                                <!-- Custom APIs will be generated -->
                            </div>
                        </div>
                        <div class="api-config-section">
                            <div class="api-section-header-with-action">
                                <h3 class="api-section-title">
                                    <i class="fas fa-network-wired"></i>
                                    REST APIs
                                </h3>
                                <button class="btn btn-primary btn-sm" id="add-rest-api-btn">
                                    <i class="fas fa-plus"></i>
                                    Add REST API
                                </button>
                            </div>
                            <p class="api-section-description">Configure and execute REST API calls for any endpoint</p>
                            <div class="rest-apis-list" id="rest-apis-list">
                                <!-- REST APIs will be generated -->
                            </div>
                        </div>
                    </div>

                    <!-- Usage History Tab -->
                    <div class="api-tab-content" id="tab-history">
                        <div class="api-history-container" id="api-history-container">
                            <!-- History will be dynamically generated -->
                        </div>
                    </div>
                </div>
            </div>
            
            <!-- Add/Edit Custom API Modal -->
            <div class="api-key-modal" id="custom-api-modal">
                <div class="api-key-modal-overlay"></div>
                <div class="api-key-modal-content" style="max-width: 800px; max-height: 90vh; overflow-y: auto;">
                    <div class="api-key-modal-header">
                        <h3 id="custom-api-modal-title">
                            <i class="fas fa-plus-circle"></i>
                            <span id="custom-api-modal-action">Add Custom API</span>
                        </h3>
                        <button class="api-key-modal-close" id="custom-api-modal-close">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="api-key-modal-body">
                        <div class="custom-api-form" id="custom-api-form">
                            <div class="api-key-form-group">
                                <label for="custom-api-id">API ID <span class="required">*</span></label>
                                <input type="text" id="custom-api-id" class="api-key-input-field" placeholder="e.g., alpha_vantage" pattern="[a-z0-9_]+" required>
                                <p class="api-key-help-text">Lowercase letters, numbers, and underscores only. Cannot be "finnhub" or "yahoo".</p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-name">API Name <span class="required">*</span></label>
                                <input type="text" id="custom-api-name" class="api-key-input-field" placeholder="e.g., Alpha Vantage" required>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-description">Description</label>
                                <textarea id="custom-api-description" class="api-key-input-field" rows="2" placeholder="Brief description of the API"></textarea>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-base-url">Base URL <span class="required">*</span></label>
                                <input type="url" id="custom-api-base-url" class="api-key-input-field" placeholder="https://api.example.com" required>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-quote-url">Quote URL Template <span class="required">*</span></label>
                                <input type="text" id="custom-api-quote-url" class="api-key-input-field" placeholder="https://api.example.com/quote?symbol={symbol}&apikey={api_key}" required>
                                <p class="api-key-help-text">Use {symbol} for symbol placeholder and {api_key} for API key placeholder</p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-key">API Key</label>
                                <div class="api-key-input-wrapper">
                                    <input type="password" id="custom-api-key" class="api-key-input-field" placeholder="Enter API key if required">
                                    <button type="button" class="api-key-toggle-btn" id="custom-api-key-toggle">
                                        <i class="fas fa-eye"></i>
                                    </button>
                                </div>
                            </div>
                            
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-rate-limit">Rate Limit (calls)</label>
                                    <input type="number" id="custom-api-rate-limit" class="api-key-input-field" value="60" min="1" required>
                                </div>
                                
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-rate-period">Rate Period (seconds)</label>
                                    <input type="number" id="custom-api-rate-period" class="api-key-input-field" value="60" min="1" required>
                                </div>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label>
                                    <input type="checkbox" id="custom-api-requires-key">
                                    Requires API Key
                                </label>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-response-format">Response Format</label>
                                <select id="custom-api-response-format" class="api-key-input-field">
                                    <option value="json">JSON</option>
                                    <option value="xml">XML</option>
                                </select>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="custom-api-quote-path">Quote Path (optional)</label>
                                <input type="text" id="custom-api-quote-path" class="api-key-input-field" placeholder="e.g., data.quote or result.0">
                                <p class="api-key-help-text">JSON path to navigate to quote data (use dots for nested objects, e.g., "data.quote")</p>
                            </div>
                            
                            <div class="form-section-title">Field Mappings</div>
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-price-field">Price Field</label>
                                    <input type="text" id="custom-api-price-field" class="api-key-input-field" value="c" placeholder="c">
                                </div>
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-change-field">Change Field</label>
                                    <input type="text" id="custom-api-change-field" class="api-key-input-field" value="d" placeholder="d">
                                </div>
                            </div>
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-change-percent-field">Change % Field</label>
                                    <input type="text" id="custom-api-change-percent-field" class="api-key-input-field" value="dp" placeholder="dp">
                                </div>
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-volume-field">Volume Field</label>
                                    <input type="text" id="custom-api-volume-field" class="api-key-input-field" value="v" placeholder="v">
                                </div>
                            </div>
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-high-field">High Field</label>
                                    <input type="text" id="custom-api-high-field" class="api-key-input-field" value="h" placeholder="h">
                                </div>
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-low-field">Low Field</label>
                                    <input type="text" id="custom-api-low-field" class="api-key-input-field" value="l" placeholder="l">
                                </div>
                            </div>
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-open-field">Open Field</label>
                                    <input type="text" id="custom-api-open-field" class="api-key-input-field" value="o" placeholder="o">
                                </div>
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="custom-api-prev-close-field">Previous Close Field</label>
                                    <input type="text" id="custom-api-prev-close-field" class="api-key-input-field" value="pc" placeholder="pc">
                                </div>
                            </div>
                            
                            <div class="api-key-modal-status" id="custom-api-modal-status"></div>
                        </div>
                    </div>
                    <div class="api-key-modal-footer">
                        <button class="api-key-btn api-key-btn-cancel" id="custom-api-cancel-btn">Cancel</button>
                        <button class="api-key-btn api-key-btn-save" id="custom-api-save-btn">
                            <i class="fas fa-save"></i>
                            Save API
                        </button>
                    </div>
                </div>
            </div>
            
            <!-- Add/Edit REST API Modal -->
            <div class="api-key-modal" id="rest-api-modal">
                <div class="api-key-modal-overlay"></div>
                <div class="api-key-modal-content" style="max-width: 900px; max-height: 90vh; overflow-y: auto;">
                    <div class="api-key-modal-header">
                        <h3 id="rest-api-modal-title">
                            <i class="fas fa-network-wired"></i>
                            <span id="rest-api-modal-action">Add REST API</span>
                        </h3>
                        <button class="api-key-modal-close" id="rest-api-modal-close">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="api-key-modal-body">
                        <div class="custom-api-form" id="rest-api-form">
                            <div class="api-key-form-group">
                                <label for="rest-api-id">API ID <span class="required">*</span></label>
                                <input type="text" id="rest-api-id" class="api-key-input-field" placeholder="e.g., github_api" pattern="[a-z0-9_]+" required>
                                <p class="api-key-help-text">Lowercase letters, numbers, and underscores only.</p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-name">API Name <span class="required">*</span></label>
                                <input type="text" id="rest-api-name" class="api-key-input-field" placeholder="e.g., GitHub API" required>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-description">Description</label>
                                <textarea id="rest-api-description" class="api-key-input-field" rows="2" placeholder="Brief description of the REST API"></textarea>
                            </div>
                            
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 2;">
                                    <label for="rest-api-base-url">Base URL <span class="required">*</span></label>
                                    <input type="url" id="rest-api-base-url" class="api-key-input-field" placeholder="https://api.example.com" required>
                                </div>
                                
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-method">HTTP Method</label>
                                    <select id="rest-api-method" class="api-key-input-field">
                                        <option value="GET">GET</option>
                                        <option value="POST">POST</option>
                                        <option value="PUT">PUT</option>
                                        <option value="PATCH">PATCH</option>
                                        <option value="DELETE">DELETE</option>
                                    </select>
                                </div>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-endpoint">Endpoint Path <span class="required">*</span></label>
                                <input type="text" id="rest-api-endpoint" class="api-key-input-field" placeholder="/users/{id}" required>
                                <p class="api-key-help-text">Path relative to base URL. Use {param} for dynamic parameters.</p>
                            </div>
                            
                            <div class="form-section-title">Authentication</div>
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-auth-type">Auth Type</label>
                                    <select id="rest-api-auth-type" class="api-key-input-field">
                                        <option value="none">None</option>
                                        <option value="bearer">Bearer Token</option>
                                        <option value="api_key_header">API Key (Header)</option>
                                        <option value="api_key_query">API Key (Query)</option>
                                    </select>
                                </div>
                                
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-auth-value">Auth Value</label>
                                    <div class="api-key-input-wrapper">
                                        <input type="password" id="rest-api-auth-value" class="api-key-input-field" placeholder="Token or API key">
                                        <button type="button" class="api-key-toggle-btn" id="rest-api-auth-toggle">
                                            <i class="fas fa-eye"></i>
                                        </button>
                                    </div>
                                </div>
                            </div>
                            
                            <div class="form-section-title">Request Configuration</div>
                            <div class="api-key-form-group">
                                <label for="rest-api-headers">Headers (JSON)</label>
                                <textarea id="rest-api-headers" class="api-key-input-field" rows="3" placeholder='{"Content-Type": "application/json", "Accept": "application/json"}'></textarea>
                                <p class="api-key-help-text">JSON object with header key-value pairs</p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-query-params">Query Parameters (JSON)</label>
                                <textarea id="rest-api-query-params" class="api-key-input-field" rows="2" placeholder='{"page": "1", "limit": "10"}'></textarea>
                                <p class="api-key-help-text">JSON object with query parameter key-value pairs</p>
                            </div>
                            
                            <div class="api-key-form-group" id="rest-api-body-group" style="display: none;">
                                <label for="rest-api-request-body">Request Body</label>
                                <textarea id="rest-api-request-body" class="api-key-input-field" rows="4" placeholder='{"key": "value"}'></textarea>
                                <p class="api-key-help-text">JSON body for POST/PUT/PATCH requests</p>
                            </div>
                            
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-timeout">Timeout (seconds)</label>
                                    <input type="number" id="rest-api-timeout" class="api-key-input-field" value="30" min="1" max="300" required>
                                </div>
                                
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-response-format">Response Format</label>
                                    <select id="rest-api-response-format" class="api-key-input-field">
                                        <option value="json">JSON</option>
                                        <option value="text">Text</option>
                                        <option value="xml">XML</option>
                                    </select>
                                </div>
                            </div>
                            
                            <div class="form-row">
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-rate-limit">Rate Limit (calls)</label>
                                    <input type="number" id="rest-api-rate-limit" class="api-key-input-field" value="60" min="1" required>
                                </div>
                                
                                <div class="api-key-form-group" style="flex: 1;">
                                    <label for="rest-api-rate-period">Rate Period (seconds)</label>
                                    <input type="number" id="rest-api-rate-period" class="api-key-input-field" value="60" min="1" required>
                                </div>
                            </div>
                            
                            <div class="api-key-modal-status" id="rest-api-modal-status"></div>
                        </div>
                    </div>
                    <div class="api-key-modal-footer">
                        <button class="api-key-btn api-key-btn-cancel" id="rest-api-cancel-btn">Cancel</button>
                        <button class="api-key-btn api-key-btn-save" id="rest-api-save-btn">
                            <i class="fas fa-save"></i>
                            Save REST API
                        </button>
                    </div>
                </div>
            </div>
            
            <!-- REST API Execute Modal -->
            <div class="api-key-modal" id="rest-api-execute-modal">
                <div class="api-key-modal-overlay"></div>
                <div class="api-key-modal-content" style="max-width: 1000px; max-height: 90vh; overflow-y: auto;">
                    <div class="api-key-modal-header">
                        <h3 id="rest-api-execute-title">
                            <i class="fas fa-play"></i>
                            <span>Execute REST API</span>
                        </h3>
                        <button class="api-key-modal-close" id="rest-api-execute-close">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="api-key-modal-body">
                        <div id="rest-api-execute-form">
                            <div class="api-key-form-group">
                                <label>API: <strong id="rest-api-execute-name"></strong></label>
                                <p class="api-key-help-text" id="rest-api-execute-description"></p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-execute-params">Query Parameters (JSON)</label>
                                <textarea id="rest-api-execute-params" class="api-key-input-field" rows="3" placeholder='{"param": "value"}'></textarea>
                                <p class="api-key-help-text">Override or add query parameters</p>
                            </div>
                            
                            <div class="api-key-form-group">
                                <label for="rest-api-execute-headers">Additional Headers (JSON)</label>
                                <textarea id="rest-api-execute-headers" class="api-key-input-field" rows="3" placeholder='{"Header-Name": "value"}'></textarea>
                                <p class="api-key-help-text">Override or add headers</p>
                            </div>
                            
                            <div class="api-key-form-group" id="rest-api-execute-body-group" style="display: none;">
                                <label for="rest-api-execute-body">Request Body</label>
                                <textarea id="rest-api-execute-body" class="api-key-input-field" rows="6" placeholder='{"key": "value"}'></textarea>
                                <p class="api-key-help-text">Request body for POST/PUT/PATCH</p>
                            </div>
                            
                            <div class="api-key-modal-status" id="rest-api-execute-status"></div>
                            
                            <div id="rest-api-execute-result" style="display: none;">
                                <div class="form-section-title">Response</div>
                                <div class="api-key-form-group">
                                    <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                        <label>Status: <span id="rest-api-execute-status-code" class="badge"></span></label>
                                        <label>Time: <span id="rest-api-execute-time"></span>ms</label>
                                    </div>
                                    <pre id="rest-api-execute-response" style="background: var(--dark-bg); padding: 1rem; border-radius: 0.5rem; overflow-x: auto; max-height: 400px; overflow-y: auto; font-size: 0.875rem;"></pre>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="api-key-modal-footer">
                        <button class="api-key-btn api-key-btn-cancel" id="rest-api-execute-cancel-btn">Close</button>
                        <button class="api-key-btn api-key-btn-save" id="rest-api-execute-btn">
                            <i class="fas fa-play"></i>
                            Execute
                        </button>
                    </div>
                </div>
            </div>
            
            <!-- API Key Configuration Modal -->
            <div class="api-key-modal" id="api-key-modal">
                <div class="api-key-modal-overlay"></div>
                <div class="api-key-modal-content">
                    <div class="api-key-modal-header">
                        <h3 id="api-key-modal-title">
                            <i class="fas fa-key"></i>
                            <span id="api-key-modal-api-name">Configure API Key</span>
                        </h3>
                        <button class="api-key-modal-close" id="api-key-modal-close">
                            <i class="fas fa-times"></i>
                        </button>
                    </div>
                    <div class="api-key-modal-body">
                        <div class="api-key-form-group">
                            <label for="api-key-input">API Key</label>
                            <div class="api-key-input-wrapper">
                                <input 
                                    type="password" 
                                    id="api-key-input" 
                                    class="api-key-input-field" 
                                    placeholder="Enter your API key"
                                    autocomplete="off"
                                >
                                <button type="button" class="api-key-toggle-btn" id="api-key-toggle">
                                    <i class="fas fa-eye"></i>
                                </button>
                            </div>
                            <p class="api-key-help-text">
                                <i class="fas fa-info-circle"></i>
                                Get your free API key from <a href="https://finnhub.io/register" target="_blank">finnhub.io</a>
                            </p>
                        </div>
                        <div class="api-key-modal-status" id="api-key-modal-status"></div>
                    </div>
                    <div class="api-key-modal-footer">
                        <button class="api-key-btn api-key-btn-cancel" id="api-key-cancel-btn">Cancel</button>
                        <button class="api-key-btn api-key-btn-save" id="api-key-save-btn">
                            <i class="fas fa-save"></i>
                            Save API Key
                        </button>
                    </div>
                </div>
            </div>
        </main>

        <!-- Data Page -->
        <main class="page-content" id="page-data">
            <div class="page-header">
                <h1 class="page-title">
                    <i class="fas fa-database"></i>
                    Saved Projections Data
                </h1>
                <div class="data-page-controls">
                    <button class="btn btn-primary" id="refresh-data-btn">
                        <i class="fas fa-sync-alt"></i>
                        Refresh
                    </button>
                </div>
            </div>
            
            <div class="data-container">
                <!-- File Upload Section -->
                <div class="dashboard-section" style="margin-bottom: 2rem;">
                    <div class="section-header">
                        <h2 class="section-title">
                            <i class="fas fa-upload"></i>
                            Upload Projection File
                        </h2>
                        <p class="section-description">Upload a JSON projection file or PNG chart image to save and review later</p>
                    </div>
                    
                    <div class="file-upload-container">
                        <div class="file-upload-wrapper" id="file-upload-wrapper">
                            <input type="file" 
                                   id="projection-file-input" 
                                   class="file-upload-input" 
                                   accept=".json,.png,application/json,image/png"
                                   style="display: none;">
                            <label for="projection-file-input" class="file-upload-label">
                                <div class="file-upload-icon">
                                    <i class="fas fa-cloud-upload-alt"></i>
                                </div>
                                <div class="file-upload-content">
                                    <h3>Drop files to upload</h3>
                                    <p>or <span class="file-upload-browse">browse files</span></p>
                                    <p class="file-upload-hint">Supported formats: JSON, PNG (Max 10MB)</p>
                                </div>
                            </label>
                            <div class="file-upload-preview" id="file-upload-preview" style="display: none;">
                                <div class="file-upload-file">
                                    <div class="file-upload-file-info">
                                        <i class="fas fa-file-code file-icon"></i>
                                        <div class="file-info">
                                            <span class="file-name" id="file-name"></span>
                                            <span class="file-size" id="file-size"></span>
                                        </div>
                                    </div>
                                    <button type="button" class="file-upload-remove" id="file-upload-remove">
                                        <i class="fas fa-times"></i>
                                    </button>
                                </div>
                            </div>
                        </div>
                        <div class="file-upload-actions" id="file-upload-actions" style="display: none;">
                            <button type="button" class="btn btn-secondary" id="file-upload-cancel">
                                Cancel
                            </button>
                            <button type="button" class="btn btn-primary" id="file-upload-submit">
                                <i class="fas fa-upload"></i>
                                Upload & Save
                            </button>
                        </div>
                        <div class="file-upload-status" id="file-upload-status" style="display: none;"></div>
                    </div>
                </div>
                
                <!-- 3D Visualization Section -->
                <div class="data-viz-container" style="margin-bottom: 2rem;">
                    <div class="data-viz-header">
                        <h3>
                            <i class="fas fa-cube"></i>
                            Data Visualization
                        </h3>
                        <div class="data-viz-controls">
                            <div class="data-viz-info">
                                <span id="data-viz-count">0</span> projections
                                <span class="data-viz-hint" title="Drag to rotate, scroll to zoom">
                                    <i class="fas fa-info-circle"></i>
                                </span>
                            </div>
                            <div class="data-viz-model-selector">
                                <label for="data-viz-model-select" class="data-viz-model-label">
                                    <i class="fas fa-shapes"></i>
                                    Model:
                                </label>
                                <select id="data-viz-model-select" class="data-viz-model-select">
                                    <option value="dodecahedron">Dodecahedron</option>
                                    <option value="icosahedron">Icosahedron</option>
                                    <option value="octahedron">Octahedron</option>
                                    <option value="tetrahedron">Tetrahedron</option>
                                    <option value="cube">Cube</option>
                                    <option value="sphere">Sphere</option>
                                    <option value="torus">Torus</option>
                                </select>
                            </div>
                        </div>
                    </div>
                    <div class="data-viz-canvas-wrapper">
                        <canvas id="data-3d-canvas"></canvas>
                        <div class="data-viz-loading" id="data-viz-loading">
                            <i class="fas fa-spinner fa-spin"></i>
                            <span>Loading visualization...</span>
                        </div>
                    </div>
                </div>
                
                <!-- DataTable Section -->
                <div class="dashboard-section">
                    <div class="section-header">
                        <h2 class="section-title">
                            <i class="fas fa-table"></i>
                            Saved Projections
                        </h2>
                        <div class="section-controls">
                            <button class="btn btn-secondary btn-sm" id="export-all-btn">
                                <i class="fas fa-download"></i>
                                Export All
                            </button>
                            <button class="btn btn-secondary btn-sm" id="hide-all-btn" style="margin-left: 0.5rem;">
                                <i class="fas fa-eye-slash"></i>
                                Hide All
                            </button>
                        </div>
                    </div>
                    
                    <div class="datatable-container">
                        <div id="datatable-loading" class="datatable-loading" style="display: none;">
                            <i class="fas fa-spinner fa-spin"></i>
                            <span>Loading data...</span>
                        </div>
                        
                        <div id="datatable-error" class="datatable-error" style="display: none;">
                            <i class="fas fa-exclamation-triangle"></i>
                            <span id="datatable-error-message"></span>
                            <button class="btn btn-primary btn-sm" onclick="DataPageModule.refresh()">
                                <i class="fas fa-redo"></i>
                                Retry
                            </button>
                        </div>
                        
                        <div id="hs-datatable-to-destroy" class="flex flex-col" data-hs-datatable='{
                          "pageLength": 10,
                          "pagingOptions": {
                            "pageBtnClasses": "min-w-10 flex justify-center items-center text-gray-800 hover:bg-gray-100 focus:outline-hidden focus:bg-gray-100 py-2.5 text-sm rounded-full disabled:opacity-50 disabled:pointer-events-none dark:text-white dark:focus:bg-neutral-700 dark:hover:bg-neutral-700"
                          },
                          "language": {
                            "zeroRecords": "<div class=\"py-10 px-5 flex flex-col justify-center items-center text-center\"><svg class=\"shrink-0 size-6 text-gray-500 dark:text-neutral-500\" xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><circle cx=\"11\" cy=\"11\" r=\"8\"/><path d=\"m21 21-4.3-4.3\"/></svg><div class=\"max-w-sm mx-auto\"><p class=\"mt-2 text-sm text-gray-600 dark:text-neutral-400\">No search results</p></div></div>"
                          },
                          "layout": {
                            "topStart": {
                              "buttons": ["copy", "csv", "excel", "pdf", "print"]
                            }
                          }
                        }'>
                          <div class="flex flex-wrap items-center gap-2 mb-4">
                            <div class="grow">
                              <div class="relative max-w-xs w-full">
                                <label for="hs-table-destroy-and-reinitialize-search" class="sr-only">Search</label>
                                <input type="text" name="hs-table-destroy-and-reinitialize-search" id="hs-table-destroy-and-reinitialize-search" class="py-1.5 sm:py-2 px-3 ps-9 block w-full border-gray-200 shadow-2xs rounded-lg sm:text-sm focus:z-10 focus:border-blue-500 focus:ring-blue-500 disabled:opacity-50 disabled:pointer-events-none dark:bg-neutral-900 dark:border-neutral-700 dark:text-neutral-400 dark:placeholder-neutral-500 dark:focus:ring-neutral-600" placeholder="Search projections..." data-hs-datatable-search="">
                                <div class="absolute inset-y-0 start-0 flex items-center pointer-events-none ps-3">
                                  <svg class="size-4 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                    <circle cx="11" cy="11" r="8"></circle>
                                    <path d="m21 21-4.3-4.3"></path>
                                  </svg>
                                </div>
                              </div>
                            </div>

                            <div class="flex-1 flex items-center justify-end space-x-2">
                              <div id="hs-dropdown-datatable-destroy-and-reinitialize" class="hs-dropdown [--placement:bottom-right] relative inline-flex">
                                <button id="hs-datatable-destroy-and-reinitialize-dropdown" type="button" class="hs-dropdown-toggle py-2 px-3 inline-flex items-center gap-x-2 text-sm rounded-lg border border-gray-200 bg-white text-gray-800 shadow-2xs hover:bg-gray-50 focus:outline-hidden focus:bg-gray-50 disabled:opacity-50 disabled:pointer-events-none dark:bg-neutral-900 dark:border-neutral-700 dark:text-white dark:hover:bg-neutral-800 dark:focus:bg-neutral-800" aria-haspopup="menu" aria-expanded="false" aria-label="Dropdown">
                                  <svg class="shrink-0 size-4" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                    <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
                                    <polyline points="7 10 12 15 17 10"></polyline>
                                    <line x1="12" x2="12" y1="15" y2="3"></line>
                                  </svg>
                                  Export
                                  <svg class="hs-dropdown-open:rotate-180 size-4" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                    <path d="m6 9 6 6 6-6"></path>
                                  </svg>
                                </button>

                                <div class="hs-dropdown-menu hs-dropdown-open:opacity-100 w-32 transition-[opacity,margin] duration opacity-0 hidden z-20 bg-white rounded-xl shadow-xl dark:bg-neutral-900" role="menu" aria-orientation="vertical" aria-labelledby="hs-datatable-destroy-and-reinitialize-dropdown">
                                  <div class="p-1 space-y-0.5">
                                    <button type="button" class="flex w-full items-center gap-x-2 py-2 px-3 rounded-lg text-sm text-gray-800 hover:bg-gray-100 focus:outline-hidden focus:bg-gray-100 dark:text-neutral-400 dark:hover:bg-neutral-700 dark:hover:text-neutral-300 dark:focus:bg-neutral-700" data-hs-datatable-action-type="copy">
                                      <svg class="shrink-0 size-3.5" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                        <rect width="14" height="14" x="8" y="8" rx="2" ry="2"></rect>
                                        <path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"></path>
                                      </svg>
                                      Copy
                                    </button>
                                    <button type="button" class="flex w-full items-center gap-x-2 py-2 px-3 rounded-lg text-sm text-gray-800 hover:bg-gray-100 focus:outline-hidden focus:bg-gray-100 dark:text-neutral-400 dark:hover:bg-neutral-700 dark:hover:text-neutral-300 dark:focus:bg-neutral-700" data-hs-datatable-action-type="csv">
                                      <svg class="shrink-0 size-3.5" width="400" height="461" viewBox="0 0 400 461" fill="none" xmlns="http://www.w3.org/2000/svg">
                                        <g clip-path="url(#clip2_0)">
                                          <path d="M342.544 460.526H57.4563C39.2545 460.526 24.5615 445.833 24.5615 427.632V32.8947C24.5615 14.693 39.2545 0 57.4563 0H265.79L375.439 109.649V427.632C375.439 445.833 360.746 460.526 342.544 460.526Z" fill="#E7EAF3"></path>
                                          <path d="M375.439 109.649H265.79V0L375.439 109.649Z" fill="#F8FAFD"></path>
                                          <path d="M265.79 109.649L375.439 219.298V109.649H265.79Z" fill="#BDC5D1"></path>
                                          <path d="M387.5 389.035H12.5C5.70175 389.035 0 383.553 0 376.535V272.807C0 266.009 5.48246 260.307 12.5 260.307H387.5C394.298 260.307 400 265.79 400 272.807V376.535C400 383.553 394.298 389.035 387.5 389.035Z" fill="#377DFF"></path>
                                          <path d="M91.7529 306.759C91.7529 305.748 92.0236 304.984 92.565 304.467C93.1064 303.95 93.8944 303.691 94.929 303.691C95.9275 303.691 96.6975 303.956 97.2388 304.485C97.7922 305.015 98.0689 305.772 98.0689 306.759C98.0689 307.709 97.7922 308.461 97.2388 309.015C96.6854 309.556 95.9155 309.827 94.929 309.827C93.9184 309.827 93.1365 309.562 92.5831 309.033C92.0296 308.491 91.7529 307.733 91.7529 306.759ZM114.707 287.233C112.602 287.233 110.972 288.028 109.817 289.616C108.662 291.192 108.084 293.393 108.084 296.22C108.084 302.103 110.292 305.045 114.707 305.045C116.56 305.045 118.803 304.581 121.438 303.655V308.347C119.273 309.249 116.855 309.7 114.184 309.7C110.346 309.7 107.411 308.539 105.377 306.218C103.344 303.884 102.328 300.539 102.328 296.184C102.328 293.441 102.827 291.041 103.826 288.984C104.824 286.915 106.256 285.333 108.12 284.238C109.997 283.131 112.193 282.578 114.707 282.578C117.27 282.578 119.844 283.197 122.431 284.436L120.626 288.984C119.64 288.515 118.647 288.106 117.649 287.757C116.65 287.408 115.67 287.233 114.707 287.233ZM142.642 302.013C142.642 304.395 141.782 306.272 140.061 307.643C138.353 309.015 135.971 309.7 132.915 309.7C130.1 309.7 127.61 309.171 125.444 308.112V302.915C127.225 303.709 128.729 304.269 129.956 304.593C131.195 304.918 132.326 305.081 133.348 305.081C134.575 305.081 135.514 304.846 136.163 304.377C136.825 303.908 137.156 303.21 137.156 302.284C137.156 301.766 137.012 301.309 136.723 300.912C136.434 300.503 136.007 300.112 135.442 299.739C134.888 299.366 133.751 298.771 132.031 297.953C130.419 297.195 129.21 296.467 128.404 295.769C127.598 295.071 126.954 294.259 126.473 293.333C125.992 292.407 125.751 291.324 125.751 290.085C125.751 287.751 126.539 285.916 128.115 284.581C129.703 283.245 131.893 282.578 134.684 282.578C136.055 282.578 137.36 282.74 138.6 283.065C139.851 283.39 141.156 283.847 142.516 284.436L140.711 288.785C139.303 288.208 138.136 287.805 137.21 287.576C136.296 287.348 135.393 287.233 134.503 287.233C133.445 287.233 132.632 287.48 132.067 287.973C131.502 288.467 131.219 289.11 131.219 289.904C131.219 290.398 131.333 290.831 131.562 291.204C131.79 291.564 132.151 291.919 132.645 292.268C133.15 292.605 134.335 293.219 136.2 294.109C138.666 295.288 140.356 296.473 141.27 297.664C142.185 298.843 142.642 300.293 142.642 302.013ZM162.474 282.957H168.122L159.154 309.339H153.054L144.104 282.957H149.752L154.714 298.656C154.991 299.583 155.274 300.666 155.563 301.905C155.863 303.132 156.05 303.986 156.122 304.467C156.254 303.36 156.705 301.423 157.475 298.656L162.474 282.957Z" fill="white"></path>
                                        </g>
                                        <defs>
                                          <clipPath id="clip2_0">
                                            <rect width="400" height="460.526" fill="white"></rect>
                                          </clipPath>
                                        </defs>
                                      </svg>
                                      CSV
                                    </button>
                                  </div>
                                </div>
                              </div>
                            </div>
                          </div>

                          <div class="min-h-130 overflow-x-auto">
                            <div class="min-w-full inline-block align-middle">
                              <div class="overflow-hidden">
                                <table id="hs-datatable" class="min-w-full">
                                  <thead class="border-b border-gray-200 dark:border-neutral-700">
                                    <tr>
                                      <th scope="col" class="py-1 group text-start font-normal focus:outline-hidden">
                                        <div class="py-1 px-2.5 inline-flex items-center border border-transparent text-sm text-gray-500 rounded-md hover:border-gray-200 dark:text-neutral-500 dark:hover:border-neutral-700">
                                          Symbol
                                          <svg class="size-3.5 ms-1 -me-0.5 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                            <path class="hs-datatable-ordering-desc:text-blue-600 dark:hs-datatable-ordering-desc:text-blue-500" d="m7 15 5 5 5-5"></path>
                                            <path class="hs-datatable-ordering-asc:text-blue-600 dark:hs-datatable-ordering-asc:text-blue-500" d="m7 9 5-5 5 5"></path>
                                          </svg>
                                        </div>
                                      </th>

                                      <th scope="col" class="py-1 group text-start font-normal focus:outline-hidden">
                                        <div class="py-1 px-2.5 inline-flex items-center border border-transparent text-sm text-gray-500 rounded-md hover:border-gray-200 dark:text-neutral-500 dark:hover:border-neutral-700">
                                          Title
                                          <svg class="size-3.5 ms-1 -me-0.5 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                            <path class="hs-datatable-ordering-desc:text-blue-600 dark:hs-datatable-ordering-desc:text-blue-500" d="m7 15 5 5 5-5"></path>
                                            <path class="hs-datatable-ordering-asc:text-blue-600 dark:hs-datatable-ordering-asc:text-blue-500" d="m7 9 5-5 5 5"></path>
                                          </svg>
                                        </div>
                                      </th>

                                      <th scope="col" class="py-1 group text-start font-normal focus:outline-hidden">
                                        <div class="py-1 px-2.5 inline-flex items-center border border-transparent text-sm text-gray-500 rounded-md hover:border-gray-200 dark:text-neutral-500 dark:hover:border-neutral-700">
                                          Saved Date
                                          <svg class="size-3.5 ms-1 -me-0.5 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                            <path class="hs-datatable-ordering-desc:text-blue-600 dark:hs-datatable-ordering-desc:text-blue-500" d="m7 15 5 5 5-5"></path>
                                            <path class="hs-datatable-ordering-asc:text-blue-600 dark:hs-datatable-ordering-asc:text-blue-500" d="m7 9 5-5 5 5"></path>
                                          </svg>
                                        </div>
                                      </th>

                                      <th scope="col" class="py-1 group text-start font-normal focus:outline-hidden">
                                        <div class="py-1 px-2.5 inline-flex items-center border border-transparent text-sm text-gray-500 rounded-md hover:border-gray-200 dark:text-neutral-500 dark:hover:border-neutral-700">
                                          Interval
                                          <svg class="size-3.5 ms-1 -me-0.5 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                            <path class="hs-datatable-ordering-desc:text-blue-600 dark:hs-datatable-ordering-desc:text-blue-500" d="m7 15 5 5 5-5"></path>
                                            <path class="hs-datatable-ordering-asc:text-blue-600 dark:hs-datatable-ordering-asc:text-blue-500" d="m7 9 5-5 5 5"></path>
                                          </svg>
                                        </div>
                                      </th>

                                      <th scope="col" class="py-1 group text-start font-normal focus:outline-hidden">
                                        <div class="py-1 px-2.5 inline-flex items-center border border-transparent text-sm text-gray-500 rounded-md hover:border-gray-200 dark:text-neutral-500 dark:hover:border-neutral-700">
                                          Steps
                                          <svg class="size-3.5 ms-1 -me-0.5 text-gray-400 dark:text-neutral-500" xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                            <path class="hs-datatable-ordering-desc:text-blue-600 dark:hs-datatable-ordering-desc:text-blue-500" d="m7 15 5 5 5-5"></path>
                                            <path class="hs-datatable-ordering-asc:text-blue-600 dark:hs-datatable-ordering-asc:text-blue-500" d="m7 9 5-5 5 5"></path>
                                          </svg>
                                        </div>
                                      </th>

                                      <th scope="col" class="py-2 px-3 text-end font-normal text-sm text-gray-500 --exclude-from-ordering dark:text-neutral-500">Actions</th>
                                    </tr>
                                  </thead>

                                  <tbody class="divide-y divide-gray-200 dark:divide-neutral-700" id="datatable-tbody">
                                    <!-- Data will be populated by DataTables -->
                                  </tbody>
                                </table>
                              </div>
                            </div>
                          </div>

                          <div class="flex items-center space-x-1 mt-4 hidden" data-hs-datatable-paging="">
                            <button type="button" class="p-2.5 min-w-10 inline-flex justify-center items-center gap-x-2 text-sm rounded-full text-gray-800 hover:bg-gray-100 focus:outline-hidden focus:bg-gray-100 disabled:opacity-50 disabled:pointer-events-none dark:text-white dark:hover:bg-neutral-700 dark:focus:bg-neutral-700" data-hs-datatable-paging-prev="">
                              <span aria-hidden="true">«</span>
                              <span class="sr-only">Previous</span>
                            </button>
                            <div class="flex items-center space-x-1 [&>.active]:bg-gray-100 dark:[&>.active]:bg-neutral-700" data-hs-datatable-paging-pages=""></div>
                            <button type="button" class="p-2.5 min-w-10 inline-flex justify-center items-center gap-x-2 text-sm rounded-full text-gray-800 hover:bg-gray-100 focus:outline-hidden focus:bg-gray-100 disabled:opacity-50 disabled:pointer-events-none dark:text-white dark:hover:bg-neutral-700 dark:focus:bg-neutral-700" data-hs-datatable-paging-next="">
                              <span class="sr-only">Next</span>
                              <span aria-hidden="true">»</span>
                            </button>
                          </div>
                        </div>
                    </div>
                </div>
            </div>
        </main>

        <!-- Settings Page -->
        <main class="page-content" id="page-settings">
            <div class="page-header">
                <h1 class="page-title">Settings</h1>
            </div>
            <section class="dashboard-section settings-section">
                <!-- Settings Tabs -->
                <div class="settings-tabs">
                    <button class="settings-tab active" data-tab="display">
                        <i class="fas fa-desktop"></i>
                        <span>Display</span>
                    </button>
                    <button class="settings-tab" data-tab="notifications">
                        <i class="fas fa-bell"></i>
                        <span>Notifications</span>
                    </button>
                    <button class="settings-tab" data-tab="data">
                        <i class="fas fa-database"></i>
                        <span>Data</span>
                    </button>
                    <button class="settings-tab" data-tab="ip-button">
                        <i class="fas fa-network-wired"></i>
                        <span>IP Button</span>
                    </button>
                    <button class="settings-tab" data-tab="shortcuts">
                        <i class="fas fa-keyboard"></i>
                        <span>Shortcuts</span>
                    </button>
                    <button class="settings-tab" data-tab="about">
                        <i class="fas fa-info-circle"></i>
                        <span>About</span>
                    </button>
                </div>

                <div class="settings-container">
                    <!-- Display Settings Tab -->
                    <div class="settings-tab-content active" id="tab-display">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-desktop"></i>
                                    Display Preferences
                                </h3>
                                <p class="settings-group-description">Customize how the dashboard looks and behaves</p>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Show Ticker Tape</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-show-ticker" checked>
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Display the TradingView ticker tape at the top of the dashboard</span>
                                </div>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Compact Market Cards</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-compact-cards">
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Show a more compact view of market data cards</span>
                                </div>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Theme</label>
                                <select id="setting-theme" class="form-input">
                                    <option value="dark">Dark Mode</option>
                                    <option value="light">Light Mode</option>
                                    <option value="auto">Auto (System)</option>
                                </select>
                                <span class="settings-hint">Choose your preferred color theme. Auto follows your system preference.</span>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Font Size</label>
                                <div class="settings-range-container">
                                    <input type="range" id="setting-font-size-range" class="settings-range" min="12" max="18" step="1" value="14">
                                    <input type="number" id="setting-font-size" class="form-input settings-range-input" min="12" max="18" value="14">
                                    <span class="settings-range-unit">px</span>
                                </div>
                                <span class="settings-hint">Adjust the base font size for better readability</span>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Enable Animations</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-animations" checked>
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Enable smooth transitions and animations throughout the app</span>
                                </div>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Show Tooltips</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-tooltips" checked>
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Display helpful tooltips on hover for buttons and controls</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Notification Settings Tab -->
                    <div class="settings-tab-content" id="tab-notifications">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-bell"></i>
                                    Notifications & Alerts
                                </h3>
                                <p class="settings-group-description">Control how you receive alerts and notifications</p>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Browser Notifications</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-notifications">
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Receive browser notifications for important updates</span>
                                </div>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Sound Alerts</label>
                                <div class="settings-toggle-wrapper">
                                    <label class="toggle-switch">
                                        <input type="checkbox" id="setting-sound-alerts">
                                        <span class="toggle-slider"></span>
                                    </label>
                                    <span class="settings-hint">Play sounds for alerts and notifications</span>
                                </div>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Notification Position</label>
                                <select id="setting-notification-position" class="form-input">
                                    <option value="top-right">Top Right</option>
                                    <option value="top-left">Top Left</option>
                                    <option value="bottom-right">Bottom Right</option>
                                    <option value="bottom-left">Bottom Left</option>
                                    <option value="top-center">Top Center</option>
                                    <option value="bottom-center">Bottom Center</option>
                                </select>
                                <span class="settings-hint">Choose where notifications appear on screen</span>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Notification Duration</label>
                                <div class="settings-range-container">
                                    <input type="range" id="setting-notification-duration-range" class="settings-range" min="2" max="10" step="1" value="5">
                                    <input type="number" id="setting-notification-duration" class="form-input settings-range-input" min="2" max="10" value="5">
                                    <span class="settings-range-unit">seconds</span>
                                </div>
                                <span class="settings-hint">How long notifications stay visible</span>
                            </div>
                        </div>
                    </div>

                    <!-- Data Management Tab -->
                    <div class="settings-tab-content" id="tab-data">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-database"></i>
                                    Data Management
                                </h3>
                                <p class="settings-group-description">Manage your data, cache, and backups</p>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Clear API Cache</label>
                                </div>
                                <button class="btn btn-secondary" id="clear-cache-btn">
                                    <i class="fas fa-broom"></i>
                                    Clear Cache Now
                                </button>
                                <span class="settings-hint">Removes cached market data to force fresh API calls. Useful if data seems outdated.</span>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Export All Data</label>
                                </div>
                                <button class="btn btn-secondary" id="export-data-btn">
                                    <i class="fas fa-download"></i>
                                    Download Backup
                                </button>
                                <span class="settings-hint">Download all your data (todos, notes, symbols, settings) as a JSON file for backup.</span>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Import Settings</label>
                                </div>
                                <div class="settings-file-upload">
                                    <input type="file" id="import-settings-file" accept=".json" style="display: none;">
                                    <button class="btn btn-secondary" id="import-settings-btn">
                                        <i class="fas fa-upload"></i>
                                        Import Settings
                                    </button>
                                </div>
                                <span class="settings-hint">Import settings from a previously exported JSON file</span>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Export Settings</label>
                                </div>
                                <button class="btn btn-secondary" id="export-settings-btn">
                                    <i class="fas fa-download"></i>
                                    Export Settings
                                </button>
                                <span class="settings-hint">Download your current settings as a JSON file for backup</span>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Reset to Defaults</label>
                                </div>
                                <button class="btn btn-secondary" id="reset-settings-btn">
                                    <i class="fas fa-undo"></i>
                                    Reset Settings
                                </button>
                                <span class="settings-hint">Reset all settings to their default values</span>
                            </div>
                            <div class="settings-item danger-zone">
                                <div class="settings-item-header">
                                    <label class="settings-label danger-label">Reset All Data</label>
                                </div>
                                <button class="btn btn-danger" id="reset-data-btn">
                                    <i class="fas fa-exclamation-triangle"></i>
                                    Reset Everything
                                </button>
                                <span class="settings-hint danger-hint">⚠️ Warning: This will permanently delete all your todos, notes, symbols, and settings. This action cannot be undone!</span>
                            </div>
                        </div>
                    </div>
                    
                    <!-- Keyboard Shortcuts Tab -->
                    <div class="settings-tab-content" id="tab-shortcuts">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-keyboard"></i>
                                    Keyboard Shortcuts
                                </h3>
                                <p class="settings-group-description">Speed up your workflow with these keyboard shortcuts</p>
                            </div>
                            
                            <div class="shortcuts-list">
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>Ctrl</kbd> + <kbd>K</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Quick Search</strong>
                                        <span>Open global search</span>
                                    </div>
                                </div>
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>Ctrl</kbd> + <kbd>S</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Save Settings</strong>
                                        <span>Save all current settings</span>
                                    </div>
                                </div>
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>Esc</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Close Modal</strong>
                                        <span>Close any open modal or dialog</span>
                                    </div>
                                </div>
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>Ctrl</kbd> + <kbd>D</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Dashboard</strong>
                                        <span>Navigate to dashboard</span>
                                    </div>
                                </div>
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>Ctrl</kbd> + <kbd>,</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Settings</strong>
                                        <span>Open settings page</span>
                                    </div>
                                </div>
                                <div class="shortcut-item">
                                    <div class="shortcut-keys">
                                        <kbd>/</kbd>
                                    </div>
                                    <div class="shortcut-description">
                                        <strong>Search</strong>
                                        <span>Focus search input (when available)</span>
                                    </div>
                                </div>
                            </div>
                            
                            <div class="settings-hint" style="margin-top: 1.5rem; padding: 1rem; background: var(--card-bg); border-radius: 0.5rem;">
                                <i class="fas fa-info-circle"></i>
                                <strong>Note:</strong> Shortcuts may vary depending on your browser and operating system. On Mac, use <kbd>Cmd</kbd> instead of <kbd>Ctrl</kbd>.
                            </div>
                        </div>
                    </div>
                    
                    <!-- About Tab -->
                    <div class="settings-tab-content" id="tab-about">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-info-circle"></i>
                                    About ALGO3D
                                </h3>
                                <p class="settings-group-description">Application information and system details</p>
                            </div>
                            
                            <div class="about-section">
                                <div class="about-item">
                                    <div class="about-label">Application Name</div>
                                    <div class="about-value">ALGO3D Trading Dashboard</div>
                                </div>
                                <div class="about-item">
                                    <div class="about-label">Version</div>
                                    <div class="about-value" id="app-version">1.0.0</div>
                                </div>
                                <div class="about-item">
                                    <div class="about-label">Build Date</div>
                                    <div class="about-value" id="app-build-date"><?php echo date('Y-m-d'); ?></div>
                                </div>
                                <div class="about-item">
                                    <div class="about-label">Browser</div>
                                    <div class="about-value" id="app-browser">Detecting...</div>
                                </div>
                                <div class="about-item">
                                    <div class="about-label">Screen Resolution</div>
                                    <div class="about-value" id="app-resolution">Detecting...</div>
                                </div>
                                <div class="about-item">
                                    <div class="about-label">Local Storage</div>
                                    <div class="about-value" id="app-storage">Checking...</div>
                                </div>
                            </div>
                            
                            <div class="settings-item" style="margin-top: 1.5rem;">
                                <div class="settings-item-header">
                                    <label class="settings-label">System Information</label>
                                </div>
                                <button class="btn btn-secondary" id="copy-system-info-btn">
                                    <i class="fas fa-copy"></i>
                                    Copy System Info
                                </button>
                                <span class="settings-hint">Copy system information for troubleshooting</span>
                            </div>
                            
                            <div class="settings-hint" style="margin-top: 1.5rem; padding: 1rem; background: var(--card-bg); border-radius: 0.5rem;">
                                <i class="fas fa-heart"></i>
                                <strong>Thank you for using ALGO3D!</strong> This application is designed to provide real-time market data and advanced trading analysis tools.
                            </div>
                        </div>
                    </div>
                    
                    <!-- IP Button Tab -->
                    <div class="settings-tab-content" id="tab-ip-button">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-network-wired"></i>
                                    IP Whitelist Tool
                                </h3>
                                <p class="settings-group-description">Add your current IP address to the whitelist file for your friend's cron job</p>
                            </div>
                            
                            <div class="settings-item">
                                <div class="ip-whitelist-container">
                                    <div class="ip-display-box">
                                        <div class="ip-label">Your Current IP Address</div>
                                        <div class="ip-address-value" id="current-ip-display">Detecting...</div>
                                    </div>
                                    
                                    <button class="btn btn-primary btn-large" id="whitelist-ip-btn" style="margin-top: 1.5rem; width: 100%;">
                                        <i class="fas fa-plus-circle"></i>
                                        <span id="whitelist-btn-text">Add My IP to Whitelist</span>
                                    </button>
                                    
                                    <div class="ip-status-message" id="ip-status-message" style="display: none; margin-top: 1rem; padding: 1rem; border-radius: 0.5rem;"></div>
                                    
                                    <div class="settings-hint" style="margin-top: 1rem;">
                                        <i class="fas fa-info-circle"></i>
                                        Click the button to save your IP address to <code>practice/acl/address.txt</code>. Your friend's cron job will read this file and whitelist your IP, then clear the file.
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div class="settings-actions">
                        <div class="settings-actions-left">
                            <div class="auto-save-indicator" id="auto-save-indicator" style="display: none;">
                                <i class="fas fa-check-circle"></i>
                                <span>Settings auto-saved</span>
                            </div>
                        </div>
                        <div class="settings-actions-right">
                            <button class="btn btn-secondary" id="reset-settings-quick-btn" title="Reset to defaults">
                                <i class="fas fa-undo"></i>
                                Reset
                            </button>
                            <button class="btn btn-primary btn-large" id="save-settings-btn">
                                <i class="fas fa-save"></i>
                                Save All Settings
                            </button>
                        </div>
                    </div>
                </div>
            </section>
        </main>
    </div>

    <!-- Add Symbol Modal -->
    <div class="modal" id="add-symbol-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-plus-circle"></i>
                    Add Stock Symbol
                </h3>
                <button class="modal-close" id="close-symbol-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="form-group">
                    <label class="form-label">Symbol</label>
                    <input type="text" id="symbol-input" class="form-input" placeholder="e.g., AAPL, MSFT, TSLA" autofocus>
                    <small class="form-hint">Enter a stock ticker symbol</small>
                </div>
                <div class="modal-actions">
                    <button class="btn btn-secondary" id="cancel-symbol-btn">Cancel</button>
                    <button class="btn btn-primary" id="save-symbol-btn">
                        <i class="fas fa-plus"></i>
                        Add Symbol
                    </button>
                </div>
            </div>
        </div>
    </div>

    <!-- User Profile Modal -->
    <div class="modal" id="user-profile-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-user"></i>
                    User Profile
                </h3>
                <button class="modal-close" id="close-profile-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="profile-avatar-section">
                    <div class="profile-avatar-large" id="profile-avatar-large">
                        <span id="profile-initials-large">U</span>
                    </div>
                    <button class="btn btn-secondary btn-sm" id="change-avatar-btn">
                        <i class="fas fa-camera"></i>
                        Change Avatar
                    </button>
                </div>
                <div class="profile-form">
                    <div class="form-group">
                        <label class="form-label">Full Name</label>
                        <input type="text" id="profile-name" class="form-input" placeholder="Enter your name">
                    </div>
                    <div class="form-group">
                        <label class="form-label">Email</label>
                        <input type="email" id="profile-email" class="form-input" placeholder="Enter your email">
                    </div>
                    <div class="form-group">
                        <label class="form-label">Time Zone</label>
                        <select id="profile-timezone" class="form-input">
                            <option value="America/New_York">Eastern Time (ET)</option>
                            <option value="America/Chicago">Central Time (CT)</option>
                            <option value="America/Denver">Mountain Time (MT)</option>
                            <option value="America/Los_Angeles">Pacific Time (PT)</option>
                            <option value="Europe/London">London (GMT)</option>
                            <option value="Asia/Tokyo">Tokyo (JST)</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label class="form-label">Currency</label>
                        <select id="profile-currency" class="form-input">
                            <option value="USD">USD ($)</option>
                            <option value="EUR">EUR (€)</option>
                            <option value="GBP">GBP (£)</option>
                            <option value="JPY">JPY (¥)</option>
                        </select>
                    </div>
                </div>
                <div class="modal-actions">
                    <button class="btn btn-secondary" id="cancel-profile-btn">Cancel</button>
                    <button class="btn btn-primary" id="save-profile-btn">
                        <i class="fas fa-save"></i>
                        Save Profile
                    </button>
                </div>
            </div>
        </div>
    </div>

    <!-- User Statistics Modal -->
    <div class="modal" id="user-stats-modal">
        <div class="modal-content modal-large">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-chart-bar"></i>
                    Your Statistics
                </h3>
                <button class="modal-close" id="close-stats-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="stats-grid-modal">
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-chart-line"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-total-symbols">0</span>
                            <span class="stat-label-modal">Total Symbols Tracked</span>
                        </div>
                    </div>
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-tasks"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-total-todos">0</span>
                            <span class="stat-label-modal">Total Tasks Created</span>
                        </div>
                    </div>
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-sticky-note"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-total-notes">0</span>
                            <span class="stat-label-modal">Total Notes</span>
                        </div>
                    </div>
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-check-circle"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-completed-todos">0</span>
                            <span class="stat-label-modal">Completed Tasks</span>
                        </div>
                    </div>
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-clock"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-account-age">0</span>
                            <span class="stat-label-modal">Days Active</span>
                        </div>
                    </div>
                    <div class="stat-card-modal">
                        <div class="stat-icon-modal"><i class="fas fa-sync"></i></div>
                        <div class="stat-info-modal">
                            <span class="stat-value-modal" id="stat-api-calls">0</span>
                            <span class="stat-label-modal">API Calls Today</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <!-- Trading Note Editor Modal -->
    <div class="modal" id="note-modal">
        <div class="modal-content modal-extra-large">
            <div class="modal-header">
                <h3 id="note-modal-title">
                    <i class="fas fa-sticky-note"></i>
                    Trading Note
                </h3>
                <button class="modal-close" id="close-note-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body trading-note-form">
                <input type="hidden" id="note-id">
                
                <!-- Note Type Tabs -->
                <div class="note-type-tabs">
                    <button class="note-type-tab active" data-type="trade">
                        <i class="fas fa-exchange-alt"></i>
                        Trade
                    </button>
                    <button class="note-type-tab" data-type="strategy">
                        <i class="fas fa-chess"></i>
                        Strategy
                    </button>
                    <button class="note-type-tab" data-type="journal">
                        <i class="fas fa-book"></i>
                        Journal
                    </button>
                </div>
                
                <!-- Basic Info Section -->
                <div class="form-section">
                    <h4 class="form-section-title">
                        <i class="fas fa-info-circle"></i>
                        Basic Information
                    </h4>
                    <div class="form-row">
                        <div class="form-group">
                            <label class="form-label">Title *</label>
                            <input type="text" id="note-title-input" class="form-input" placeholder="e.g., ES Long Entry - Morning Session">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Category</label>
                            <select id="note-category" class="form-input">
                                <option value="">Select Category</option>
                                <option value="scalp">Scalping</option>
                                <option value="swing">Swing Trading</option>
                                <option value="day">Day Trading</option>
                                <option value="breakout">Breakout</option>
                                <option value="reversal">Reversal</option>
                                <option value="trend">Trend Following</option>
                                <option value="other">Other</option>
                            </select>
                        </div>
                    </div>
                </div>
                
                <!-- Trading Details Section (for Trade type) -->
                <div class="form-section trading-details-section" id="trading-details-section">
                    <h4 class="form-section-title">
                        <i class="fas fa-chart-candlestick"></i>
                        Trading Details
                    </h4>
                    <div class="form-row">
                        <div class="form-group">
                            <label class="form-label">Contract Symbol *</label>
                            <input type="text" id="note-contract-symbol" class="form-input" placeholder="e.g., ES, NQ, YM, CL">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Contract Type</label>
                            <select id="note-contract-type" class="form-input">
                                <option value="futures">Futures</option>
                                <option value="options">Options</option>
                                <option value="forex">Forex</option>
                                <option value="crypto">Crypto</option>
                            </select>
                        </div>
                        <div class="form-group">
                            <label class="form-label">Direction *</label>
                            <select id="note-direction" class="form-input">
                                <option value="">Select</option>
                                <option value="long">Long</option>
                                <option value="short">Short</option>
                            </select>
                        </div>
                    </div>
                    
                    <div class="form-row">
                        <div class="form-group">
                            <label class="form-label">Entry Price *</label>
                            <input type="number" id="note-entry-price" class="form-input" step="0.01" placeholder="0.00">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Exit Price</label>
                            <input type="number" id="note-exit-price" class="form-input" step="0.01" placeholder="0.00">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Quantity</label>
                            <input type="number" id="note-quantity" class="form-input" value="1" min="1">
                        </div>
                    </div>
                    
                    <div class="form-row">
                        <div class="form-group">
                            <label class="form-label">Entry Time</label>
                            <input type="datetime-local" id="note-entry-time" class="form-input">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Exit Time</label>
                            <input type="datetime-local" id="note-exit-time" class="form-input">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Timeframe</label>
                            <select id="note-timeframe" class="form-input">
                                <option value="">Select</option>
                                <option value="1m">1 Minute</option>
                                <option value="5m">5 Minutes</option>
                                <option value="15m">15 Minutes</option>
                                <option value="30m">30 Minutes</option>
                                <option value="1h">1 Hour</option>
                                <option value="4h">4 Hours</option>
                                <option value="1d">Daily</option>
                            </select>
                        </div>
                    </div>
                    
                    <div class="form-row">
                        <div class="form-group">
                            <label class="form-label">Stop Loss</label>
                            <input type="number" id="note-stop-loss" class="form-input" step="0.01" placeholder="0.00">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Take Profit</label>
                            <input type="number" id="note-take-profit" class="form-input" step="0.01" placeholder="0.00">
                        </div>
                        <div class="form-group">
                            <label class="form-label">Strategy</label>
                            <input type="text" id="note-strategy" class="form-input" placeholder="e.g., Breakout, Reversal">
                        </div>
                    </div>
                    
                    <!-- P&L Calculation Display -->
                    <div class="pnl-display" id="pnl-display" style="display: none;">
                        <div class="pnl-row">
                            <span class="pnl-label">P&L:</span>
                            <span class="pnl-value" id="calculated-pnl">$0.00</span>
                        </div>
                        <div class="pnl-row">
                            <span class="pnl-label">P&L %:</span>
                            <span class="pnl-value" id="calculated-pnl-percent">0.00%</span>
                        </div>
                    </div>
                </div>
                
                <!-- Notes Content Section -->
                <div class="form-section">
                    <h4 class="form-section-title">
                        <i class="fas fa-sticky-note"></i>
                        Notes & Analysis
                    </h4>
                    <textarea id="note-content-input" class="form-textarea" placeholder="Write your trading notes, analysis, lessons learned, market observations..." rows="8"></textarea>
                </div>
                
                <!-- Color Picker -->
                <div class="form-section">
                <div class="note-color-picker">
                        <label>Note Color:</label>
                    <div class="color-options">
                            <button class="color-option active" data-color="#1e293b" style="background:#1e293b" title="Default"></button>
                            <button class="color-option" data-color="#164e63" style="background:#164e63" title="Blue"></button>
                            <button class="color-option" data-color="#166534" style="background:#166534" title="Green"></button>
                            <button class="color-option" data-color="#854d0e" style="background:#854d0e" title="Yellow"></button>
                            <button class="color-option" data-color="#7c2d12" style="background:#7c2d12" title="Red"></button>
                            <button class="color-option" data-color="#581c87" style="background:#581c87" title="Purple"></button>
                    </div>
                </div>
                </div>
                
                <div class="modal-actions">
                    <button class="btn btn-secondary" id="cancel-note-btn">Cancel</button>
                    <button class="btn btn-primary" id="save-note-btn">
                        <i class="fas fa-save"></i>
                        Save Note
                    </button>
                </div>
            </div>
        </div>
    </div>

    <!-- Save Study Modal -->
    <div class="modal" id="save-study-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-bookmark"></i>
                    Save Chart Study
                </h3>
                <button class="modal-close" id="close-save-study-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="form-group">
                    <label class="form-label">Study Name</label>
                    <input type="text" id="study-name-input" class="form-input" placeholder="Enter a name for this study">
                </div>
                <div class="form-group">
                    <label class="form-label">Notes</label>
                    <textarea id="study-notes-input" class="form-textarea" rows="4" placeholder="Add notes about this setup..."></textarea>
                </div>
                <div class="study-preview">
                    <div class="preview-item">
                        <span class="preview-label">Symbol:</span>
                        <span class="preview-value" id="preview-symbol">--</span>
                    </div>
                    <div class="preview-item">
                        <span class="preview-label">Timeframe:</span>
                        <span class="preview-value" id="preview-timeframe">--</span>
                    </div>
                    <div class="preview-item">
                        <span class="preview-label">Indicators:</span>
                        <span class="preview-value" id="preview-indicators">--</span>
                    </div>
                </div>
                <div class="modal-actions">
                    <button class="btn btn-secondary" id="cancel-save-study-btn">Cancel</button>
                    <button class="btn btn-primary" id="confirm-save-study-btn">
                        <i class="fas fa-save"></i>
                        Save Study
                    </button>
                </div>
            </div>
        </div>
    </div>

    <!-- Load Study Modal -->
    <div class="modal" id="load-study-modal">
        <div class="modal-content modal-large">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-folder-open"></i>
                    Load Saved Study
                </h3>
                <button class="modal-close" id="close-load-study-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div class="saved-studies-list" id="saved-studies-list">
                    <!-- Studies will be loaded here -->
                </div>
                <div class="empty-studies" id="empty-studies">
                    <i class="fas fa-folder-open"></i>
                    <p>No saved studies yet</p>
                </div>
            </div>
        </div>
    </div>

    <!-- Load Projection Modal -->
    <div class="modal" id="load-projection-modal">
        <div class="modal-content modal-extra-large">
            <div class="modal-header">
                <h3>
                    <i class="fas fa-chart-line"></i>
                    Load Projection
                </h3>
                <button class="modal-close" id="close-load-projection-modal" title="Close">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <div id="load-projection-loading" class="loading" style="display: none;">
                    <i class="fas fa-spinner fa-spin"></i>
                    <span>Loading projection data...</span>
                </div>
                <div id="load-projection-content" style="display: none;">
                    <div class="form-group">
                        <label class="form-label">Symbol</label>
                        <div class="projection-info-value" id="modal-projection-symbol">--</div>
                    </div>
                    <div class="form-group">
                        <label class="form-label">Title</label>
                        <div class="projection-info-value" id="modal-projection-title">--</div>
                    </div>
                    <div class="form-group">
                        <label class="form-label">Interval</label>
                        <div class="projection-info-value" id="modal-projection-interval">--</div>
                    </div>
                    <div class="form-group">
                        <label class="form-label">Parameters</label>
                        <div class="projection-info-value" id="modal-projection-params">--</div>
                    </div>
                    <div class="form-group" id="modal-projection-notes-group" style="display: none;">
                        <label class="form-label">Notes</label>
                        <div class="projection-info-value" id="modal-projection-notes">--</div>
                    </div>
                    <div class="form-group">
                        <label class="form-label">Saved Date</label>
                        <div class="projection-info-value" id="modal-projection-date">--</div>
                    </div>
                    <div class="form-group" style="margin-top: 1.5rem;">
                        <label class="form-label">Price Projection Chart</label>
                        <div style="position: relative; height: 400px; width: 100%; margin-top: 0.75rem; background: var(--dark-bg); border-radius: 0.5rem; padding: 1rem; border: 1px solid var(--border-color);">
                            <canvas id="modal-projection-chart"></canvas>
                        </div>
                    </div>
                </div>
                <div id="load-projection-error" class="error-message" style="display: none;">
                    <i class="fas fa-exclamation-triangle"></i>
                    <span id="load-projection-error-text">Failed to load projection</span>
                </div>
                <div class="modal-actions">
                    <button class="btn btn-secondary" id="cancel-load-projection-btn">Cancel</button>
                    <button class="btn btn-primary" id="confirm-load-projection-btn">
                        <i class="fas fa-check"></i>
                        Load Projection
                    </button>
                </div>
            </div>
        </div>
    </div>

    <!-- Chart.js Library -->
    <!-- Suppress source map warnings (non-critical) -->
    <script>
        // Suppress source map 404 errors (non-critical warnings)
        // These warnings don't affect functionality - source maps are only for debugging
        const originalError = console.error;
        console.error = function(...args) {
            // Filter out source map loading errors
            const errorMessage = args.join(' ');
            if (errorMessage && typeof errorMessage === 'string' && 
                (errorMessage.includes('.map') || 
                 errorMessage.includes('Source Map') ||
                 errorMessage.includes('Failed to load resource') && errorMessage.includes('.map') ||
                 errorMessage.includes('chart.umd.min.js.map') ||
                 errorMessage.includes('cdn.jsdelivr.net') && errorMessage.includes('.map'))) {
                // Suppress source map errors - they're non-critical
                return;
            }
            originalError.apply(console, args);
        };
        
        // Suppress network errors for source maps (404s)
        // This catches resource loading failures before they reach console
        window.addEventListener('error', function(event) {
            // Check if it's a source map loading error
            if (event.target) {
                const src = event.target.src || event.target.href || '';
                if (src && (src.includes('.map') || src.includes('chart.umd.min.js.map'))) {
                    event.preventDefault();
                    event.stopPropagation();
                    event.stopImmediatePropagation();
                    return false;
                }
            }
            // Check error message for source map references
            const errorMsg = (event.message || event.filename || event.target?.src || '').toString();
            if (errorMsg.includes('.map') || 
                errorMsg.includes('chart.umd.min.js.map') ||
                errorMsg.includes('cdn.jsdelivr.net/npm/chart') && errorMsg.includes('.map') ||
                (errorMsg.includes('Failed to load') && (errorMsg.includes('404') || errorMsg.includes('.map')))) {
                event.preventDefault();
                event.stopPropagation();
                event.stopImmediatePropagation();
                return false;
            }
        }, true);
        
        // Also intercept fetch/XMLHttpRequest for source maps (if any)
        const originalFetch = window.fetch;
        window.fetch = function(...args) {
            const url = args[0]?.toString() || '';
            if (url.includes('.map') || url.includes('chart.umd.min.js.map')) {
                // Silently fail for source map requests
                return Promise.reject(new Error('Source map request suppressed'));
            }
            return originalFetch.apply(this, args);
        };
        
        // Suppress console warnings from third-party scripts (TradingView widgets, Tailwind CDN)
        const originalWarn = console.warn;
        console.warn = function(...args) {
            // Filter out TradingView/snowplow warnings and Tailwind CDN warnings
            const message = args.join(' ');
            if (message.includes('Invalid environment') || 
                message.includes('snowplow-embed-widget-tracker') ||
                message.includes('cdn.tailwindcss.com should not be used') ||
                message.includes('cdn.tailwindcss.com') ||
                message.includes('Source Map loading errors')) {
                // Suppress these warnings - they're from third-party scripts
                return;
            }
            // Allow other warnings
            originalWarn.apply(console, args);
        };
    </script>
    <!-- jQuery and DataTables Libraries -->
    <script src="https://code.jquery.com/jquery-3.7.1.min.js"></script>
    <script src="https://cdn.datatables.net/1.13.7/js/jquery.dataTables.min.js"></script>
    <link rel="stylesheet" href="https://cdn.datatables.net/1.13.7/css/jquery.dataTables.min.css">
    
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-chart-financial"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom@2"></script>
    <script>
        // Register zoom plugin globally when Chart.js is ready
        (function() {
            function registerZoomPlugin() {
                if (typeof Chart !== 'undefined' && Chart.register) {
                    // Check if plugin is already registered
                    if (Chart.registry && Chart.registry.getPlugin('zoom')) {
                        console.log('Zoom plugin already registered');
                        return;
                    }
                    
                    // Try to register from various possible locations
                    if (typeof zoomPlugin !== 'undefined') {
                        Chart.register(zoomPlugin);
                        console.log('Zoom plugin registered from zoomPlugin');
                    } else if (window.zoomPlugin) {
                        Chart.register(window.zoomPlugin);
                        console.log('Zoom plugin registered from window.zoomPlugin');
                    } else {
                        // Plugin might auto-register, check after a moment
                        setTimeout(function() {
                            if (Chart.registry && Chart.registry.getPlugin('zoom')) {
                                console.log('Zoom plugin auto-registered');
                            } else {
                                console.warn('Zoom plugin not found - zoom/pan features may not work');
                            }
                        }, 100);
                    }
                } else {
                    // Chart.js not loaded yet, try again
                    setTimeout(registerZoomPlugin, 100);
                }
            }
            
            // Try immediately and also on DOM ready
            if (document.readyState === 'loading') {
                document.addEventListener('DOMContentLoaded', registerZoomPlugin);
            } else {
                registerZoomPlugin();
            }
        })();
    </script>
    <!-- Three.js Library for 3D Visualizations -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="assets/js/charts.js"></script>
    <script src="assets/js/world-clocks.js"></script>
    <script src="assets/js/market-data.js"></script>
    <script src="assets/js/todo-list.js"></script>
    <script src="assets/js/notes-keep.js"></script>
    <!-- Phase 1 & Phase 2: Enhanced Projection Modules -->
    <!-- Hyper-Dimensional Tools -->
    <script src="assets/js/tetration-towers.js"></script>
    <script src="assets/js/platonic-solid-generator.js"></script>
    <script src="assets/js/hyperdimensional-tools.js"></script>
    <!-- Projection Configuration Tabs -->
    <script src="assets/js/projection-tabs.js"></script>
    <!-- Unified Projection Engine (Final Solution) -->
    <script src="assets/js/unified-projection-engine.js"></script>
    <!-- Legacy modules (kept for compatibility, but unified engine is primary) -->
    <script src="assets/js/projection-validation.js"></script>
    <script src="assets/js/oscillation-analysis.js"></script>
    <script src="assets/js/projection-models/base-model.js"></script>
    <script src="assets/js/projection-models/crystalline-model.js"></script>
    <script src="assets/js/projection-models/harmonic-model.js"></script>
    <script src="assets/js/projection-models/wave-based-model.js"></script>
    <script src="assets/js/projection-models/statistical-model.js"></script>
    <script src="assets/js/projection-ensemble.js"></script>
    <script src="assets/js/projections.js"></script>
    <script src="assets/js/fib-calc.js"></script>
    <script src="assets/js/data-page.js"></script>
    <script src="assets/js/settings-tabs.js"></script>
    <script src="assets/js/settings.js"></script>
    <script src="assets/js/user-widget.js"></script>
    <script src="assets/js/news-feed.js"></script>
    <script src="assets/js/api-dashboard.js"></script>
    <script src="assets/js/main.js"></script>
    
    <!-- Toast Notification Container -->
    <div id="toast-container" class="toast-container"></div>
</body>
</html>
