// World Clocks Configuration
const timezones = {
    miami: 'America/New_York',
    london: 'Europe/London',
    tokyo: 'Asia/Tokyo'
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

// Update all clocks
function updateAllClocks() {
    updateClock('miami', timezones.miami);
    updateClock('london', timezones.london);
    updateClock('tokyo', timezones.tokyo);
}

// Initialize clocks
function initWorldClocks() {
    // Update immediately
    updateAllClocks();
    
    // Update every second
    setInterval(updateAllClocks, 1000);
}

// Start clocks when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initWorldClocks);
} else {
    initWorldClocks();
}

