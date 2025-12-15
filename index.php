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
    <link rel="stylesheet" href="assets/css/style.css">
    <link rel="stylesheet" href="assets/css/fib-page.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <script src="https://cdn.tailwindcss.com"></script>
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
                    <!-- Search and Controls Section -->
                    <div class="projections-controls-panel">
                        <div class="projections-search-section">
                            <div class="search-input-group">
                                <input type="text" id="projection-symbol-input" class="projection-input" placeholder="Enter symbol (e.g., AAPL, TSLA, SPY)" value="">
                                <button id="projection-search-btn" class="btn btn-primary">
                                    <i class="fas fa-search"></i>
                                    Search
                                </button>
                                <button id="projection-refresh-btn" class="btn btn-secondary" style="display: none;">
                                    <i class="fas fa-sync-alt"></i>
                                    Refresh
                                </button>
                            </div>
                            <div class="projection-interval-selector">
                                <label>Interval:</label>
                                <select id="projection-interval-select" class="projection-select">
                                    <option value="1d">1 Day</option>
                                    <option value="5d">5 Days</option>
                                    <option value="1mo">1 Month</option>
                                    <option value="3mo">3 Months</option>
                                    <option value="6mo">6 Months</option>
                                    <option value="1y">1 Year</option>
                                </select>
                            </div>
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
                                        <button id="reset-zoom-btn" class="btn btn-small" style="display: none;" title="Reset Zoom">
                                            <i class="fas fa-expand"></i>
                                        </button>
                                    </div>
                                    <div class="chart-export-controls">
                                        <button id="export-chart-btn" class="btn btn-small" title="Export Chart">
                                            <i class="fas fa-download"></i>
                                            Export
                                        </button>
                                        <button id="save-projection-btn" class="btn btn-primary btn-small" style="display: none;">
                                            <i class="fas fa-save"></i>
                                            Save Projection
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
            <div class="page-header">
                <h1 class="page-title">
                    <i class="fas fa-chart-line"></i>
                    Futures Trading Journal
                </h1>
            </div>
            
            <!-- Trading Performance Dashboard -->
            <div class="trading-performance-dashboard">
                <div class="metric-card trading-pnl-card">
                    <div class="metric-header">
                        <i class="fas fa-dollar-sign"></i>
                        <span>Total P&L</span>
                    </div>
                    <div class="metric-content">
                        <div class="stat-large" id="trading-total-pnl">$0.00</div>
                        <div class="stat-label">All Time</div>
                        <div class="stat-small">
                            <span id="trading-today-pnl">$0.00</span> today
                        </div>
                    </div>
                </div>
                
                <div class="metric-card trading-wins-card">
                    <div class="metric-header">
                        <i class="fas fa-trophy"></i>
                        <span>Win Rate</span>
                    </div>
                    <div class="metric-content">
                        <div class="stat-large" id="trading-win-rate">0%</div>
                        <div class="stat-label">Success Rate</div>
                        <div class="stat-small">
                            <span id="trading-wins">0</span> wins / <span id="trading-losses">0</span> losses
                        </div>
                    </div>
                </div>
                
                <div class="metric-card trading-trades-card">
                    <div class="metric-header">
                        <i class="fas fa-exchange-alt"></i>
                        <span>Total Trades</span>
                    </div>
                    <div class="metric-content">
                        <div class="stat-large" id="trading-total-trades">0</div>
                        <div class="stat-label">All Trades</div>
                        <div class="stat-small">
                            <span id="trading-today-trades">0</span> today
                        </div>
                    </div>
                </div>
                
                <div class="metric-card trading-avg-card">
                    <div class="metric-header">
                        <i class="fas fa-chart-bar"></i>
                        <span>Avg P&L</span>
                    </div>
                    <div class="metric-content">
                        <div class="stat-large" id="trading-avg-pnl">$0.00</div>
                        <div class="stat-label">Per Trade</div>
                        <div class="stat-small">
                            Best: <span id="trading-best-trade">$0.00</span>
                        </div>
                    </div>
                </div>
            </div>
            
            <!-- Main Content Grid -->
            <div class="notes-page-grid">
                <!-- Left Column: Trading Notes Management -->
                <div class="notes-management-panel">
                    <div class="panel-header">
                        <h2>
                            <i class="fas fa-book"></i>
                            Trading Journal
                        </h2>
                        <div class="view-toggle">
                            <button class="view-btn active" data-view="grid" title="Grid View">
                                <i class="fas fa-th"></i>
                            </button>
                            <button class="view-btn" data-view="list" title="List View">
                                <i class="fas fa-list"></i>
                            </button>
                        </div>
                    </div>
                    
                    <div class="notes-filters-enhanced">
                        <div class="filter-group">
                            <label>Type</label>
                            <div class="filter-buttons">
                                <button class="filter-btn active" data-type="all">All</button>
                                <button class="filter-btn" data-type="trade">Trades</button>
                                <button class="filter-btn" data-type="strategy">Strategy</button>
                                <button class="filter-btn" data-type="journal">Journal</button>
                            </div>
                        </div>
                        <div class="filter-group">
                            <label>Sort By</label>
                            <div class="filter-buttons">
                                <button class="filter-btn active" data-sort="recent">Recent</button>
                                <button class="filter-btn" data-sort="pnl">P&L</button>
                                <button class="filter-btn" data-sort="contract">Contract</button>
                                <button class="filter-btn" data-sort="date">Date</button>
                            </div>
                        </div>
                        <div class="filter-group">
                            <label>Filter</label>
                            <div class="filter-buttons">
                                <button class="filter-btn active" data-filter="all">All</button>
                                <button class="filter-btn" data-filter="today">Today</button>
                                <button class="filter-btn" data-filter="week">Week</button>
                                <button class="filter-btn" data-filter="profitable">Profitable</button>
                            </div>
                        </div>
                    </div>
                    
                    <div class="notes-search-enhanced">
                        <div class="input-wrapper">
                            <i class="fas fa-search"></i>
                            <input type="text" id="notes-search-input" class="notes-search-input" placeholder="Search by contract, strategy, or notes...">
                            <button class="search-clear-btn" id="notes-search-clear" style="display: none;">
                                <i class="fas fa-times"></i>
                            </button>
                        </div>
                <button class="btn btn-primary" id="add-note-btn">
                    <i class="fas fa-plus"></i>
                            New Trade Note
                </button>
            </div>
                    
                    <div class="notes-grid-container">
                    <div class="notes-grid" id="notes-grid">
                            <!-- Trading notes will be loaded here -->
                    </div>
                </div>
                </div>
                
                <!-- Right Column: 3D Visualization & Analytics -->
                <div class="notes-visualization-panel">
                    <div class="panel-header">
                        <h2>
                            <i class="fas fa-cube"></i>
                            3D Trading Analytics
                        </h2>
                        <div class="viz-controls">
                            <button class="viz-btn" id="notes-rotate-toggle" title="Auto Rotate">
                                <i class="fas fa-sync-alt"></i>
                            </button>
                            <button class="viz-btn" id="notes-reset-view" title="Reset View">
                                <i class="fas fa-home"></i>
                            </button>
                        </div>
                    </div>
                    
                    <div class="viz-container-wrapper">
                        <!-- Tab Navigation -->
                        <div class="viz-tabs">
                            <button class="viz-tab active" data-tab="3d-viz">
                                <i class="fas fa-cube"></i>
                                <span>3D Analytics</span>
                            </button>
                            <button class="viz-tab" data-tab="projections">
                                <i class="fas fa-project-diagram"></i>
                                <span>Saved Projections</span>
                            </button>
                        </div>
                        
                        <!-- 3D Visualization Tab -->
                        <div class="viz-tab-content active" id="tab-3d-viz">
                            <div class="notes-3d-container" id="notes-3d-container">
                                <canvas id="notes-3d-canvas"></canvas>
                                <div class="viz-loading" id="notes-viz-loading">
                                    <i class="fas fa-spinner fa-spin"></i>
                                    <span>Loading 3D visualization...</span>
                                </div>
                                <div class="viz-overlay">
                                    <div class="viz-info">
                                        <div class="viz-stat">
                                            <span class="viz-label">Total Trades</span>
                                            <span class="viz-value" id="viz-total-trades">0</span>
                                        </div>
                                        <div class="viz-stat">
                                            <span class="viz-label">Win Rate</span>
                                            <span class="viz-value" id="viz-win-rate">0%</span>
                                        </div>
                                        <div class="viz-stat">
                                            <span class="viz-label">Total P&L</span>
                                            <span class="viz-value" id="viz-total-pnl">$0</span>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        
                        <!-- Saved Projections Tab -->
                        <div class="viz-tab-content" id="tab-projections">
                            <div class="projections-tab-container">
                                <div class="saved-projections-list" id="saved-projections-list">
                                    <!-- Saved projections will be loaded here -->
                                </div>
                                <div class="empty-projections" id="empty-projections" style="display: none;">
                                    <i class="fas fa-inbox"></i>
                                    <p>No saved projections yet</p>
                                    <p class="empty-hint">Save projections from the Projections page to view them here</p>
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    <div class="analytics-section">
                        <h3>
                            <i class="fas fa-chart-line"></i>
                            Performance Metrics
                        </h3>
                        <div class="analytics-grid">
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-calendar-week"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-week-trades">0</div>
                                    <div class="analytics-label">This Week</div>
                                </div>
                            </div>
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-trophy"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-streak">0</div>
                                    <div class="analytics-label">Win Streak</div>
                                </div>
                            </div>
                            <div class="analytics-item">
                                <div class="analytics-icon">
                                    <i class="fas fa-dollar-sign"></i>
                                </div>
                                <div class="analytics-content">
                                    <div class="analytics-value" id="analytics-avg-pnl">$0</div>
                                    <div class="analytics-label">Avg P&L</div>
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    <div class="analytics-section">
                        <h3>
                            <i class="fas fa-tags"></i>
                            Top Contracts
                        </h3>
                        <div class="contracts-list" id="contracts-list">
                            <!-- Top contracts will be loaded here -->
                        </div>
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
                    <button class="btn btn-secondary" id="export-all-btn">
                        <i class="fas fa-download"></i>
                        Export All
                    </button>
                </div>
            </div>
            
            <div class="data-container">
                <!-- 3D Visualization Section -->
                <div class="data-viz-container">
                    <div class="data-viz-header">
                        <h3>
                            <i class="fas fa-cube"></i>
                            Data Visualization
                        </h3>
                        <div class="data-viz-info">
                            <span id="data-viz-count">0</span> projections
                            <span class="data-viz-hint" title="Drag to rotate, scroll to zoom">
                                <i class="fas fa-info-circle"></i>
                            </span>
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
                
                <!-- Search and Filter Section -->
                <div class="data-filters">
                    <div class="filter-group">
                        <input type="text" id="data-search-input" class="data-search-input" placeholder="Search by symbol, title, or notes...">
                        <i class="fas fa-search search-icon"></i>
                    </div>
                    <div class="filter-group">
                        <select id="data-sort-select" class="data-select">
                            <option value="date-desc">Date (Newest First)</option>
                            <option value="date-asc">Date (Oldest First)</option>
                            <option value="symbol-asc">Symbol (A-Z)</option>
                            <option value="symbol-desc">Symbol (Z-A)</option>
                        </select>
                    </div>
                    <div class="filter-group">
                        <button class="btn btn-small" id="clear-filters-btn">
                            <i class="fas fa-times"></i>
                            Clear
                        </button>
                    </div>
                </div>
                
                <!-- Saved Projections Feed Section -->
                <div class="news-feed-section">
                    <div class="news-feed-header">
                        <h2 class="widget-title">Saved Projections Data</h2>
                        <div class="news-feed-controls">
                            <p class="text-sm text-gray-600 dark:text-neutral-400">
                                <span class="font-semibold text-gray-800 dark:text-neutral-200" id="table-count">0</span> projection(s) found
                            </p>
                        </div>
                    </div>
                    <div class="news-feed-container" id="projections-feed-container">
                        <div class="news-loading">
                            <i class="fas fa-spinner fa-spin"></i>
                            <span>Loading projections data...</span>
                        </div>
                    </div>
                    <div class="news-pagination" id="data-pagination" style="display: none;">
                        <button class="pagination-btn" id="data-prev-btn" title="Previous Page">
                            <i class="fas fa-chevron-left"></i>
                        </button>
                        <div class="pagination-pages" id="data-pagination-pages"></div>
                        <button class="pagination-btn" id="data-next-btn" title="Next Page">
                            <i class="fas fa-chevron-right"></i>
                        </button>
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
                    <button class="settings-tab active" data-tab="api">
                        <i class="fas fa-plug"></i>
                        <span>API</span>
                    </button>
                    <button class="settings-tab" data-tab="display">
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
                </div>

                <div class="settings-container">
                    <!-- API Settings Tab -->
                    <div class="settings-tab-content active" id="tab-api">
                        <div class="settings-group">
                            <div class="settings-group-header">
                                <h3 class="settings-group-title">
                                    <i class="fas fa-plug"></i>
                                    API Configuration
                                </h3>
                                <p class="settings-group-description">Configure your data sources and API keys for real-time market data</p>
                            </div>
                            <div class="settings-item">
                                <div class="settings-item-header">
                                    <label class="settings-label">Finnhub API Key</label>
                                    <a href="https://finnhub.io/register" target="_blank" class="settings-link">
                                        <i class="fas fa-external-link-alt"></i> Get Free API Key
                                    </a>
                                </div>
                                <div class="api-key-container">
                                    <input type="password" id="setting-finnhub-key" class="form-input api-key-input" placeholder="Enter your Finnhub API key">
                                    <button type="button" class="api-key-toggle" id="toggle-api-key" title="Show/Hide API Key">
                                        <i class="fas fa-eye"></i>
                                    </button>
                                </div>
                                <span class="settings-hint">Required for real-time stock data. Free tier includes 60 calls per minute.</span>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Default Data Source</label>
                                <select id="setting-default-api" class="form-input">
                                    <option value="finnhub">Finnhub (Recommended)</option>
                                    <option value="yahoo">Yahoo Finance (Backup)</option>
                                </select>
                                <span class="settings-hint">Primary API for fetching market data. Yahoo Finance is used as automatic backup.</span>
                            </div>
                            <div class="settings-item">
                                <label class="settings-label">Auto-refresh Interval</label>
                                <div class="settings-range-container">
                                    <input type="range" id="setting-refresh-interval-range" class="settings-range" min="30" max="300" step="30" value="60">
                                    <input type="number" id="setting-refresh-interval" class="form-input settings-range-input" min="30" max="300" value="60">
                                    <span class="settings-range-unit">seconds</span>
                                </div>
                                <span class="settings-hint">How often to automatically refresh market data. Lower values use more API calls.</span>
                            </div>
                        </div>
                    </div>

                    <!-- Display Settings Tab -->
                    <div class="settings-tab-content" id="tab-display">
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

                    <div class="settings-actions">
                        <button class="btn btn-primary btn-large" id="save-settings-btn">
                            <i class="fas fa-save"></i>
                            Save All Settings
                        </button>
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
    <script src="assets/js/notes.js"></script>
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
    <script src="assets/js/data-page.js"></script>
    <script src="assets/js/settings-tabs.js"></script>
    <script src="assets/js/settings.js"></script>
    <script src="assets/js/user-widget.js"></script>
    <script src="assets/js/news-feed.js"></script>
    <script src="assets/js/main.js"></script>
</body>
</html>
