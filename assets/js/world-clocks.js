// World Clocks Configuration
const timezones = {
    ny: 'America/New_York',
    london: 'Europe/London',
    tokyo: 'Asia/Tokyo',
    sydney: 'Australia/Sydney'
};

// Format time with leading zeros
function formatTime(date) {
    const hours = String(date.getHours()).padStart(2, '0');
    const minutes = String(date.getMinutes()).padStart(2, '0');
    const seconds = String(date.getSeconds()).padStart(2, '0');
    return `${hours}:${minutes}:${seconds}`;
}

// Format date
function formatDate(date) {
    const options = { weekday: 'short', year: 'numeric', month: 'short', day: 'numeric' };
    return date.toLocaleDateString('en-US', options);
}

// Format date for mini clocks (matches screenshot format)
function formatMiniClockDate(date) {
    const weekdays = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    
    const weekday = weekdays[date.getDay()];
    const month = months[date.getMonth()];
    const day = date.getDate();
    const year = date.getFullYear();
    
    return `${weekday}, ${month} ${day}, ${year}`;
}

// Update clock display
function updateClock(city, timezone) {
    const clockElement = document.getElementById(`clock-${city}`);
    if (!clockElement) return;

    const now = new Date();
    const cityTime = new Date(now.toLocaleString('en-US', { timeZone: timezone }));
    
    const timeElement = clockElement.querySelector('.clock-time');
    const dateElement = clockElement.querySelector('.clock-date');
    
    if (timeElement) {
        timeElement.textContent = formatTime(cityTime);
    }
    if (dateElement) {
        dateElement.textContent = formatDate(cityTime);
    }
}

// Update mini clock display
function updateMiniClock(city, timezone) {
    const timeElement = document.getElementById(`mini-clock-${city}`);
    const dateElement = document.getElementById(`mini-clock-${city}-date`);
    
    if (!timeElement && !dateElement) return;

    const now = new Date();
    
    try {
        // Get time in city timezone - format: HH:MM:SS
        const cityTimeStr = now.toLocaleTimeString('en-US', { 
            timeZone: timezone,
            hour12: false,
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
        
        // Get date in city timezone - format: Sun, Dec 14, 2025
        const cityDateStr = now.toLocaleDateString('en-US', {
            timeZone: timezone,
            weekday: 'short',
            year: 'numeric',
            month: 'short',
            day: 'numeric'
        });
        
        if (timeElement) {
            timeElement.textContent = cityTimeStr;
        }
        
        if (dateElement) {
            dateElement.textContent = cityDateStr;
        }
    } catch (error) {
        console.error(`Error updating clock for ${city}:`, error);
    }
}

// Update all clocks
function updateAllClocks() {
    // Main dashboard clocks
    if (timezones.ny) updateClock('ny', timezones.ny);
    if (timezones.london) updateClock('london', timezones.london);
    if (timezones.tokyo) updateClock('tokyo', timezones.tokyo);
    if (timezones.sydney) updateClock('sydney', timezones.sydney);
}

// Available cities for adding clocks
const availableCities = {
    'London': { timezone: 'Europe/London', country: 'United Kingdom', label: '5 Change & Adventure', description: 'A time for exploration and new experiences' },
    'Sydney': { timezone: 'Australia/Sydney', country: 'Australia', label: '6 Nurturing & Responsibility', description: 'Focus on caring for others and home matters' },
    'Miami': { timezone: 'America/New_York', country: 'United States', label: '5 Change & Adventure', description: 'A time for exploration and new experiences' },
    'Los Angeles': { timezone: 'America/Los_Angeles', country: 'United States', label: '5 Change & Adventure', description: 'A time for exploration and new experiences' },
    'Paris': { timezone: 'Europe/Paris', country: 'France', label: '5 Change & Adventure', description: 'A time for exploration and new experiences' },
    'Dubai': { timezone: 'Asia/Dubai', country: 'UAE', label: '6 Nurturing & Responsibility', description: 'Focus on caring for others and home matters' },
    'Shanghai': { timezone: 'Asia/Shanghai', country: 'China', label: '6 Nurturing & Responsibility', description: 'Focus on caring for others and home matters' }
};

// Load saved clocks from localStorage
function loadSavedClocks() {
    const saved = localStorage.getItem('sidebarClocks');
    return saved ? JSON.parse(saved) : ['New York', 'Tokyo'];
}

// Save clocks to localStorage
function saveClocks(clocks) {
    localStorage.setItem('sidebarClocks', JSON.stringify(clocks));
}

// Add clock to sidebar
function addClockToSidebar(cityName) {
    const cityData = availableCities[cityName] || {
        timezone: timezones[cityName.toLowerCase().replace(' ', '')] || 'UTC',
        country: cityName,
        label: '5 Change & Adventure',
        description: 'A time for exploration and new experiences'
    };
    
    const miniClocksContainer = document.querySelector('.sidebar-mini-clocks');
    if (!miniClocksContainer) return;
    
    // Check if clock already exists
    const existingClock = miniClocksContainer.querySelector(`[data-city="${cityName}"]`);
    if (existingClock) {
        alert(`${cityName} is already added!`);
        return;
    }
    
    // Get the Add Clock button
    const addButton = document.getElementById('add-clock-btn');
    
    // Create new clock HTML - generate ID from city name
    const clockId = cityName.toLowerCase().replace(/\s+/g, '-');
    const clockHTML = `
        <div class="mini-clock-item" data-city="${cityName}">
            <div class="mini-clock-header">
                <span class="mini-clock-city">${cityName}</span>
                <span class="mini-clock-country">${cityData.country}</span>
                <button class="mini-clock-remove" data-city="${cityName}" title="Remove clock">
                    <i class="fas fa-times"></i>
                </button>
            </div>
            <div class="mini-clock-time" id="mini-clock-${clockId}">--:--:--</div>
            <div class="mini-clock-date" id="mini-clock-${clockId}-date">-- -- ----</div>
            <div class="mini-clock-label">${cityData.label}</div>
            <div class="mini-clock-description">${cityData.description}</div>
        </div>
    `;
    
    // Insert before Add Clock button
    addButton.insertAdjacentHTML('beforebegin', clockHTML);
    
    // Update timezones mapping
    timezones[clockId] = cityData.timezone;
    
    // Save to localStorage
    const savedClocks = loadSavedClocks();
    savedClocks.push(cityName);
    saveClocks(savedClocks);
    
    // Add remove button listener
    const removeBtn = miniClocksContainer.querySelector(`[data-city="${cityName}"] .mini-clock-remove`);
    if (removeBtn) {
        removeBtn.addEventListener('click', function() {
            removeClock(cityName);
        });
    }
    
    // Update immediately
    updateMiniClock(clockId, cityData.timezone);
}

// Remove clock from sidebar
function removeClock(cityName) {
    const clockElement = document.querySelector(`.mini-clock-item[data-city="${cityName}"]`);
    if (clockElement) {
        clockElement.remove();
        
        // Update localStorage
        const savedClocks = loadSavedClocks();
        const filtered = savedClocks.filter(c => c !== cityName);
        saveClocks(filtered);
    }
}

// Initialize Add Clock button
function initAddClockButton() {
    const addButton = document.getElementById('add-clock-btn');
    if (!addButton) return;
    
    addButton.addEventListener('click', function() {
        // Create modal or dropdown for adding clocks
        const available = Object.keys(availableCities).filter(city => {
            const saved = loadSavedClocks();
            return !saved.includes(city);
        });
        
        if (available.length === 0) {
            alert('All available cities have been added!');
            return;
        }
        
        // Simple prompt for now - could be enhanced with a modal
        const cityChoice = prompt(`Available cities:\n${available.join(', ')}\n\nEnter city name to add:`);
        if (cityChoice && availableCities[cityChoice]) {
            addClockToSidebar(cityChoice);
        } else if (cityChoice) {
            alert('City not found. Please select from the available cities.');
        }
    });
}

// Add remove button listeners to existing clocks
function initRemoveButtons() {
    document.querySelectorAll('.mini-clock-remove').forEach(btn => {
        btn.addEventListener('click', function() {
            const cityName = this.dataset.city;
            if (confirm(`Remove ${cityName} clock?`)) {
                removeClock(cityName);
            }
        });
    });
}

// Update all mini clocks dynamically
function updateAllMiniClocks() {
    // Get all mini clock items from DOM
    const miniClockItems = document.querySelectorAll('.mini-clock-item');
    
    miniClockItems.forEach(item => {
        const cityName = item.dataset.city;
        if (!cityName) return;
        
        // Get city key for ID matching - convert "New York" to "new-york", etc.
        let cityKey = cityName.toLowerCase().replace(/\s+/g, '-');
        
        // Handle special cases for existing clocks
        if (cityName === 'New York') {
            cityKey = 'ny';
        }
        
        // Get timezone
        let timezone = timezones[cityKey];
        
        // If not in timezones, check availableCities
        if (!timezone) {
            const cityData = availableCities[cityName];
            if (cityData) {
                timezone = cityData.timezone;
                // Store it for future use
                timezones[cityKey] = timezone;
            } else {
                // Default timezones for built-in cities
                if (cityName === 'New York') {
                    timezone = 'America/New_York';
                    timezones.ny = timezone;
                } else if (cityName === 'Tokyo') {
                    timezone = 'Asia/Tokyo';
                    timezones.tokyo = timezone;
                }
            }
        }
        
        if (timezone) {
            updateMiniClock(cityKey, timezone);
        }
    });
}

// Initialize clocks
function initWorldClocks() {
    // Update immediately
    updateAllClocks();
    updateAllMiniClocks();
    
    // Initialize Add Clock button
    initAddClockButton();
    initRemoveButtons();
    
    // Update every second
    setInterval(() => {
        updateAllClocks();
        updateAllMiniClocks();
    }, 1000);
}

// Start clocks when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initWorldClocks);
} else {
    initWorldClocks();
}









