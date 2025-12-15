// News Feed Module - Market News from Finnhub
const NewsFeedModule = (function() {
    let currentCategory = 'general';
    let isLoading = false;
    
    // Format time ago
    function formatTimeAgo(timestamp) {
        const now = Date.now();
        const diff = now - (timestamp * 1000);
        const seconds = Math.floor(diff / 1000);
        const minutes = Math.floor(seconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);
        
        if (days > 0) {
            return `${days} day${days > 1 ? 's' : ''} ago`;
        } else if (hours > 0) {
            return `${hours} hour${hours > 1 ? 's' : ''} ago`;
        } else if (minutes > 0) {
            return `${minutes} minute${minutes > 1 ? 's' : ''} ago`;
        } else {
            return 'Just now';
        }
    }
    
    // Format date
    function formatDate(timestamp) {
        const date = new Date(timestamp * 1000);
        return date.toLocaleDateString('en-US', {
            month: 'short',
            day: 'numeric',
            year: 'numeric'
        });
    }
    
    // Truncate text
    function truncateText(text, maxLength) {
        if (!text) return '';
        if (text.length <= maxLength) return text;
        return text.substring(0, maxLength) + '...';
    }
    
    // Render news articles
    function renderNews(newsData) {
        const container = document.getElementById('news-feed-container');
        if (!container) return;
        
        if (!newsData || newsData.length === 0) {
            container.innerHTML = `
                <div class="news-empty-state">
                    <i class="fas fa-newspaper"></i>
                    <p>No news available at this time.</p>
                </div>
            `;
            return;
        }
        
        const newsHTML = newsData.map((article, index) => {
            const timeAgo = formatTimeAgo(article.datetime);
            const date = formatDate(article.datetime);
            const summary = truncateText(article.summary || article.headline, 200);
            const image = article.image || '';
            const source = article.source || 'Unknown';
            
            return `
                <div class="news-article" data-index="${index}">
                    ${image ? `
                        <div class="news-article-image">
                            <img src="${image}" alt="${article.headline}" onerror="this.style.display='none'">
                        </div>
                    ` : ''}
                    <div class="news-article-content">
                        <div class="news-article-header">
                            <span class="news-source">${source}</span>
                            <span class="news-time">${timeAgo}</span>
                        </div>
                        <h3 class="news-article-title">
                            <a href="${article.url}" target="_blank" rel="noopener noreferrer">
                                ${article.headline}
                            </a>
                        </h3>
                        ${summary ? `
                            <p class="news-article-summary">${summary}</p>
                        ` : ''}
                        <div class="news-article-footer">
                            <span class="news-date">${date}</span>
                            ${article.related ? `
                                <span class="news-related">Related: ${article.related}</span>
                            ` : ''}
                        </div>
                    </div>
                </div>
            `;
        }).join('');
        
        container.innerHTML = newsHTML;
    }
    
    // Show loading state
    function showLoading() {
        const container = document.getElementById('news-feed-container');
        if (container) {
            container.innerHTML = `
                <div class="news-loading">
                    <i class="fas fa-spinner fa-spin"></i>
                    <span>Loading market news...</span>
                </div>
            `;
        }
    }
    
    // Show error state
    function showError(message) {
        const container = document.getElementById('news-feed-container');
        if (container) {
            container.innerHTML = `
                <div class="news-error-state">
                    <i class="fas fa-exclamation-triangle"></i>
                    <p>${message || 'Failed to load news. Please try again.'}</p>
                    <button class="btn btn-primary btn-sm" onclick="NewsFeedModule.loadNews()">
                        <i class="fas fa-redo"></i>
                        Retry
                    </button>
                </div>
            `;
        }
    }
    
    // Load news from API
    async function loadNews(category = null) {
        if (isLoading) return;
        
        if (category) {
            currentCategory = category;
        }
        
        isLoading = true;
        showLoading();
        
        try {
            const response = await fetch(`api/get_news.php?category=${currentCategory}`);
            const result = await response.json();
            
            if (result.success && result.data) {
                renderNews(result.data);
            } else {
                showError(result.message || 'Failed to load news');
            }
        } catch (error) {
            console.error('Error loading news:', error);
            showError('Network error. Please check your connection.');
        } finally {
            isLoading = false;
        }
    }
    
    // Initialize news feed
    function init() {
        const categorySelect = document.getElementById('news-category-select');
        const refreshBtn = document.getElementById('refresh-news-btn');
        
        if (categorySelect) {
            categorySelect.addEventListener('change', function() {
                loadNews(this.value);
            });
        }
        
        if (refreshBtn) {
            refreshBtn.addEventListener('click', function() {
                loadNews();
            });
        }
        
        // Load initial news
        loadNews();
        
        // Auto-refresh every 5 minutes
        setInterval(() => {
            if (!isLoading) {
                loadNews();
            }
        }, 300000); // 5 minutes
    }
    
    // Public API
    return {
        init: init,
        loadNews: loadNews,
        setCategory: function(category) {
            currentCategory = category;
            loadNews();
        }
    };
})();

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', NewsFeedModule.init);
} else {
    NewsFeedModule.init();
}

