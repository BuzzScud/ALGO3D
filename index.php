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
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
</head>
<body>
    <!-- Sidebar Menu -->
    <aside class="sidebar" id="sidebar">
        <div class="sidebar-header">
            <div class="sidebar-logo">
                <i class="fas fa-chart-line"></i>
                <span>ALGO3D</span>
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
                <li class="sidebar-item" data-page="charts">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-chart-candlestick"></i>
                        <span>Charts</span>
                    </a>
                </li>
                <li class="sidebar-item" data-page="notes">
                    <a href="#" class="sidebar-link">
                        <i class="fas fa-sticky-note"></i>
                        <span>Notes</span>
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
                    <i class="fas fa-moon"></i>
                    <span>Dark Mode</span>
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
                <script type="text/javascript" src="https://s3.tradingview.com/external-embedding/embed-widget-ticker-tape.js" async>
                {
                    "symbols": [
                        {"proName": "FOREXCOM:SPXUSD", "title": "S&P 500 Index"},
                        {"proName": "FOREXCOM:NSXUSD", "title": "US 100 Cash CFD"},
                        {"proName": "FX_IDC:EURUSD", "title": "EUR to USD"},
                        {"proName": "FX:GBPUSD", "title": "GBPUSD"},
                        {"proName": "FX:AUDUSD", "title": "AUDUSD"},
                        {"proName": "TVC:DXY", "title": "DXY"},
                        {"proName": "CBOE:VIX", "title": "VIX"}
                    ],
                    "colorTheme": "dark",
                    "locale": "en",
                    "largeChartUrl": "",
                    "isTransparent": true,
                    "showSymbolLogo": true,
                    "displayMode": "adaptive"
                }
                </script>
            </div>
            <!-- TradingView Widget END -->
        </div>

        <!-- Dashboard Page -->
        <main class="page-content active" id="page-dashboard">
            <div class="page-header">
                <h1 class="page-title">Dashboard</h1>
            </div>
            <div class="dashboard-container">
                <!-- World Clocks Section -->
                <section class="dashboard-section clocks-section">
                    <h2 class="section-title">
                        <i class="fas fa-clock"></i>
                        World Clocks
                    </h2>
                    <div class="clocks-grid">
                        <div class="clock-card">
                            <div class="clock-header">
                                <i class="fas fa-city"></i>
                                <h3>Miami</h3>
                            </div>
                            <div class="clock-display" id="clock-miami">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                        </div>
                        <div class="clock-card">
                            <div class="clock-header">
                                <i class="fas fa-city"></i>
                                <h3>London</h3>
                            </div>
                            <div class="clock-display" id="clock-london">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                        </div>
                        <div class="clock-card">
                            <div class="clock-header">
                                <i class="fas fa-city"></i>
                                <h3>Tokyo</h3>
                            </div>
                            <div class="clock-display" id="clock-tokyo">
                                <div class="clock-time">--:--:--</div>
                                <div class="clock-date">-- -- ----</div>
                            </div>
                        </div>
                    </div>
                </section>

                <!-- Quick Stats -->
                <section class="dashboard-section">
                    <h2 class="section-title">
                        <i class="fas fa-chart-pie"></i>
                        Quick Overview
                    </h2>
                    <div class="stats-grid">
                        <div class="stat-card">
                            <div class="stat-icon"><i class="fas fa-chart-line"></i></div>
                            <div class="stat-info">
                                <span class="stat-value" id="stat-symbols">--</span>
                                <span class="stat-label">Tracked Symbols</span>
                            </div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-icon"><i class="fas fa-tasks"></i></div>
                            <div class="stat-info">
                                <span class="stat-value" id="stat-todos">--</span>
                                <span class="stat-label">Active Tasks</span>
                            </div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-icon"><i class="fas fa-sticky-note"></i></div>
                            <div class="stat-info">
                                <span class="stat-value" id="stat-notes">--</span>
                                <span class="stat-label">Notes</span>
                            </div>
                        </div>
                        <div class="stat-card">
                            <div class="stat-icon"><i class="fas fa-server"></i></div>
                            <div class="stat-info">
                                <span class="stat-value" id="stat-api">--</span>
                                <span class="stat-label">API Status</span>
                            </div>
                        </div>
                    </div>
                </section>

                <!-- Market Data Section -->
                <section class="dashboard-section market-section">
                    <div class="section-header">
                        <h2 class="section-title">
                            <i class="fas fa-chart-line"></i>
                            Real-Time Market Data
                        </h2>
                        <div class="market-controls">
                            <div class="api-selector">
                                <label for="api-source">Data Source:</label>
                                <select id="api-source" class="api-select">
                                    <option value="finnhub">Finnhub</option>
                                    <option value="yahoo">Yahoo Finance</option>
                                </select>
                                <span class="api-status" id="api-status"></span>
                            </div>
                            <button class="btn btn-secondary btn-sm" id="refresh-market-btn" title="Refresh Data">
                                <i class="fas fa-sync-alt"></i>
                            </button>
                            <button class="btn btn-primary" id="add-symbol-btn">
                                <i class="fas fa-plus"></i>
                                Add Symbol
                            </button>
                        </div>
                    </div>
                    <div class="api-info" id="api-info">
                        <span class="api-label">Source: <strong id="current-source">--</strong></span>
                        <span class="api-label">Last Update: <strong id="last-update">--</strong></span>
                        <span class="api-label">Rate Limit: <strong id="rate-limit">--</strong></span>
                    </div>
                    <div class="market-grid" id="market-grid">
                        <!-- Market data cards will be loaded here -->
                    </div>
                </section>

                <!-- Todo List Section -->
                <section class="dashboard-section todo-section">
                    <h2 class="section-title">
                        <i class="fas fa-tasks"></i>
                        Todo List
                    </h2>
                    <div class="todo-container">
                        <div class="todo-filters">
                            <button class="filter-btn active" data-filter="all">All</button>
                            <button class="filter-btn" data-filter="daily">Daily</button>
                            <button class="filter-btn" data-filter="weekly">Weekly</button>
                            <button class="filter-btn" data-filter="completed">Completed</button>
                        </div>
                        <div class="todo-input-section">
                            <input type="text" id="todo-input" class="todo-input" placeholder="Add a new task...">
                            <select id="todo-type" class="todo-type-select">
                                <option value="daily">Daily</option>
                                <option value="weekly">Weekly</option>
                            </select>
                            <button class="btn btn-primary" id="add-todo-btn">
                                <i class="fas fa-plus"></i>
                                Add
                            </button>
                        </div>
                        <div class="todo-list" id="todo-list">
                            <!-- Todo items will be loaded here -->
                        </div>
                    </div>
                </section>
            </div>
        </main>

        <!-- Charts Page -->
        <main class="page-content" id="page-charts">
            <div class="page-header">
                <h1 class="page-title">Trading Charts</h1>
                <div class="charts-header-actions">
                    <button class="btn btn-secondary" id="save-study-btn">
                        <i class="fas fa-bookmark"></i>
                        Save Study
                    </button>
                    <div class="saved-studies-dropdown">
                        <button class="btn btn-secondary" id="toggle-studies-btn">
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
                        <button class="timeframe-btn active" data-timeframe="1D">1D</button>
                        <button class="timeframe-btn" data-timeframe="5D">5D</button>
                        <button class="timeframe-btn" data-timeframe="1M">1M</button>
                        <button class="timeframe-btn" data-timeframe="3M">3M</button>
                        <button class="timeframe-btn" data-timeframe="6M">6M</button>
                        <button class="timeframe-btn" data-timeframe="1Y">1Y</button>
                        <button class="timeframe-btn" data-timeframe="5Y">5Y</button>
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
                    </div>
                </div>

            </section>
        </main>

        <!-- Notes Page -->
        <main class="page-content" id="page-notes">
            <div class="page-header">
                <h1 class="page-title">Notes</h1>
                <button class="btn btn-primary" id="add-note-btn">
                    <i class="fas fa-plus"></i>
                    New Note
                </button>
            </div>
            <section class="dashboard-section notes-section">
                <div class="notes-container">
                    <div class="notes-search">
                        <input type="text" id="notes-search-input" class="form-input" placeholder="Search notes...">
                    </div>
                    <div class="notes-grid" id="notes-grid">
                        <!-- Notes will be loaded here -->
                    </div>
                </div>
            </section>
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
                <h3>Add Stock Symbol</h3>
                <button class="modal-close" id="close-symbol-modal">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <input type="text" id="symbol-input" class="form-input" placeholder="Enter symbol (e.g., AAPL, MSFT)">
                <div class="modal-actions">
                    <button class="btn btn-primary" id="save-symbol-btn">Add Symbol</button>
                    <button class="btn btn-secondary" id="cancel-symbol-btn">Cancel</button>
                </div>
            </div>
        </div>
    </div>

    <!-- User Profile Modal -->
    <div class="modal" id="user-profile-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>User Profile</h3>
                <button class="modal-close" id="close-profile-modal">
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
                    <button class="btn btn-primary" id="save-profile-btn">
                        <i class="fas fa-save"></i>
                        Save Profile
                    </button>
                    <button class="btn btn-secondary" id="cancel-profile-btn">Cancel</button>
                </div>
            </div>
        </div>
    </div>

    <!-- User Statistics Modal -->
    <div class="modal" id="user-stats-modal">
        <div class="modal-content modal-large">
            <div class="modal-header">
                <h3>Your Statistics</h3>
                <button class="modal-close" id="close-stats-modal">
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

    <!-- Note Editor Modal -->
    <div class="modal" id="note-modal">
        <div class="modal-content modal-large">
            <div class="modal-header">
                <h3 id="note-modal-title">New Note</h3>
                <button class="modal-close" id="close-note-modal">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="modal-body">
                <input type="hidden" id="note-id">
                <input type="text" id="note-title-input" class="form-input" placeholder="Note title...">
                <div class="note-color-picker">
                    <label>Color:</label>
                    <div class="color-options">
                        <button class="color-option active" data-color="#1e293b" style="background:#1e293b"></button>
                        <button class="color-option" data-color="#164e63" style="background:#164e63"></button>
                        <button class="color-option" data-color="#166534" style="background:#166534"></button>
                        <button class="color-option" data-color="#854d0e" style="background:#854d0e"></button>
                        <button class="color-option" data-color="#7c2d12" style="background:#7c2d12"></button>
                        <button class="color-option" data-color="#581c87" style="background:#581c87"></button>
                    </div>
                </div>
                <textarea id="note-content-input" class="form-textarea" placeholder="Write your note here..." rows="10"></textarea>
                <div class="modal-actions">
                    <button class="btn btn-primary" id="save-note-btn">Save Note</button>
                    <button class="btn btn-secondary" id="cancel-note-btn">Cancel</button>
                </div>
            </div>
        </div>
    </div>

    <!-- Save Study Modal -->
    <div class="modal" id="save-study-modal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>Save Chart Study</h3>
                <button class="modal-close" id="close-save-study-modal">
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
                    <button class="btn btn-primary" id="confirm-save-study-btn">
                        <i class="fas fa-save"></i>
                        Save Study
                    </button>
                    <button class="btn btn-secondary" id="cancel-save-study-btn">Cancel</button>
                </div>
            </div>
        </div>
    </div>

    <!-- Load Study Modal -->
    <div class="modal" id="load-study-modal">
        <div class="modal-content modal-large">
            <div class="modal-header">
                <h3>Load Saved Study</h3>
                <button class="modal-close" id="close-load-study-modal">
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

    <!-- Chart.js Library -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-chart-financial"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom@2"></script>
    <script src="assets/js/charts.js"></script>
    <script src="assets/js/world-clocks.js"></script>
    <script src="assets/js/market-data.js"></script>
    <script src="assets/js/todo-list.js"></script>
    <script src="assets/js/notes.js"></script>
    <script src="assets/js/settings-tabs.js"></script>
    <script src="assets/js/settings.js"></script>
    <script src="assets/js/user-widget.js"></script>
    <script src="assets/js/main.js"></script>
</body>
</html>
