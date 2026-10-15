// Centralized API handler for the Flight Reservation System
const BASE_URL = '/api';

const API = {
    async get(endpoint) {
        try {
            const response = await fetch(`${BASE_URL}${endpoint}`);
            return await response.json();
        } catch (error) {
            console.error('API GET Error:', error);
            return { success: false, message: 'Network error occurred' };
        }
    },

    async post(endpoint, data) {
        try {
            const response = await fetch(`${BASE_URL}${endpoint}`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(data)
            });
            return await response.json();
        } catch (error) {
            console.error('API POST Error:', error);
            return { success: false, message: 'Network error occurred' };
        }
    },

    // Session management
    saveUser(user) {
        localStorage.setItem('air_india_user', JSON.stringify(user));
    },

    getUser() {
        const user = localStorage.getItem('air_india_user');
        return user ? JSON.parse(user) : null;
    },

    logout() {
        localStorage.removeItem('air_india_user');
        window.location.href = 'index.html';
    },

    updateAuthUI() {
        const user = this.getUser();
        const authLinks = document.getElementById('auth-links');
        if (authLinks && user) {
            authLinks.innerHTML = `
                <span style="font-weight: 500; margin-right: 1rem;">Hello, ${user.username}</span>
                <button onclick="API.logout()" class="nav-btn nav-link-btn">Logout</button>
            `;
        }
    }
};

// Auto-update UI on load
window.addEventListener('DOMContentLoaded', () => {
    API.updateAuthUI();
});
