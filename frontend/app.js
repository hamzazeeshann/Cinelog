// Cinelog v2.0 - Complete Implementation

const API_BASE = 'http://localhost:8080/api';

let currentUser = null;
let allFilms = [];
let currentRoute = 'home';

// Initialization
document.addEventListener('DOMContentLoaded', () => {
    checkAuth();
    setupEventListeners();
});

function setupEventListeners() {
    document.querySelectorAll('[data-route]').forEach(el => {
        el.addEventListener('click', (e) => {
            e.preventDefault();
            navigateTo(el.dataset.route);
        });
    });

    document.querySelector('.search-trigger')?.addEventListener('click', openSearch);
    document.querySelector('.search-close')?.addEventListener('click', closeSearch);
    document.getElementById('searchInput')?.addEventListener('input', handleSearch);
    document.getElementById('searchOverlay')?.addEventListener('click', (e) => {
        if (e.target.id === 'searchOverlay') closeSearch();
    });

    document.getElementById('logoutBtn')?.addEventListener('click', logout);
}

// Authentication
async function checkAuth() {
    const token = localStorage.getItem('token');
    if (!token) {
        showLoginPage();
        return;
    }

    try {
        currentUser = parseToken(token);
        document.getElementById('navUsername').textContent = currentUser.username;
        document.getElementById('navbar').classList.add('visible');
        await loadAllFilms();
        navigateTo('home');
    } catch (error) {
        console.error('Auth failed:', error);
        localStorage.removeItem('token');
        showLoginPage();
    }
}

function parseToken(token) {
    const parts = token.split(':');
    return {
        userId: parseInt(parts[0]),
        username: parts[1],
        isAdmin: parts[2] === '1'
    };
}

function showLoginPage() {
    document.getElementById('navbar').classList.remove('visible');
    document.getElementById('app').innerHTML = `
        <div class="login-container">
            <div class="login-box">
                <h1>CINELOG</h1>
                <div id="authError" class="error-message" style="display: none;"></div>
                <form id="authForm">
                    <div class="form-group">
                        <label>Username</label>
                        <input type="text" id="username" required>
                    </div>
                    <div class="form-group">
                        <label>Email</label>
                        <input type="email" id="email" required>
                    </div>
                    <div class="form-group">
                        <label>Password</label>
                        <input type="password" id="password" required>
                    </div>
                    <button type="submit" class="btn-primary">SIGN IN</button>
                </form>
                <div class="toggle-auth">
                    <span>New user? Registration creates account automatically</span>
                </div>
            </div>
        </div>
    `;

    document.getElementById('authForm').addEventListener('submit', handleAuth);
}

async function handleAuth(e) {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const email = document.getElementById('email').value;
    const password = document.getElementById('password').value;

    try {
        const response = await fetch(`${API_BASE}/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, email, password })
        });

        const data = await response.json();

        if (response.ok && data.token) {
            localStorage.setItem('token', data.token);
            location.reload();
        } else {
            showError(data.error || 'Authentication failed');
        }
    } catch (error) {
        showError('Network error: ' + error.message);
    }
}

function showError(message) {
    const errorEl = document.getElementById('authError');
    errorEl.textContent = message;
    errorEl.style.display = 'block';
}

function logout() {
    localStorage.removeItem('token');
    currentUser = null;
    location.reload();
}

// Data Loading
async function loadAllFilms() {
    try {
        const response = await fetch(`${API_BASE}/films`);
        if (response.ok) {
            const data = await response.json();
            allFilms = data.films || [];
        }
    } catch (error) {
        console.error('Failed to load films:', error);
    }
}

// Routing
function navigateTo(route, params = {}) {
    currentRoute = route;

    document.querySelectorAll('[data-route]').forEach(el => {
        el.classList.toggle('active', el.dataset.route === route);
    });

    switch (route) {
        case 'home':
            showHomePage();
            break;
        case 'films':
            showFilmsPage();
            break;
        case 'diary':
            showDiaryPage();
            break;
        case 'lists':
            showWatchlistPage();
            break;
        case 'profile':
            showProfilePage(params.userId || currentUser.userId);
            break;
        case 'film-detail':
            showFilmDetailPage(params.filmId);
            break;
    }
}

// MODULE A & B: HOME PAGE (Hero + Popular + Recent Activity)
async function showHomePage() {
    try {
        const response = await fetch(`${API_BASE}/home_data`);
        const data = await response.json();

        if (!response.ok || !data.hero_movie) {
            document.getElementById('app').innerHTML = `<div class="empty-state"><h3>No data available</h3></div>`;
            return;
        }

        const hero = data.hero_movie;
        const backdropUrl = hero.backdrop_path || hero.poster_path || 'https://via.placeholder.com/1280x720/2C3440/FFFFFF?text=No+Image';
        const posterUrl = hero.poster_path || 'https://via.placeholder.com/300x450/2C3440/FFFFFF?text=No+Poster';

        document.getElementById('app').innerHTML = `
            <div class="hero-container">
                <div class="hero-backdrop" style="background-image: url('${backdropUrl}')"></div>
                <div class="hero-content">
                    <img src="${posterUrl}" alt="${hero.title}" class="hero-poster" onerror="this.src='https://via.placeholder.com/200x300/2C3440/FFFFFF?text=No+Image'">
                    <h1 class="hero-title">${hero.title}</h1>
                    <div class="hero-meta">${hero.year} • ${hero.director} • ⭐ ${hero.vote_average.toFixed(1)}</div>
                    ${hero.tagline ? `<p class="hero-tagline">"${hero.tagline}"</p>` : ''}
                    <div class="hero-actions">
                        <button class="btn-hero btn-log" onclick="openLogModal(${hero.film_id})">LOG THIS FILM</button>
                        <button class="btn-hero btn-details" onclick="navigateTo('film-detail', {filmId: ${hero.film_id}})">VIEW DETAILS</button>
                    </div>
                </div>
            </div>

            <div class="home-section">
                <div class="section-header">
                    <h2>Popular This Week</h2>
                    <a href="#" onclick="navigateTo('films'); return false;">View All →</a>
                </div>
                <div class="popular-row">
                    ${data.popular.map(film => `
                        <div class="popular-card" onclick="navigateTo('film-detail', {filmId: ${film.film_id}})">
                            <img src="${film.poster_path || 'https://via.placeholder.com/150x225/2C3440/FFFFFF?text=No+Poster'}" 
                                 alt="${film.title}" onerror="this.src='https://via.placeholder.com/150x225/2C3440/FFFFFF?text=No+Image'">
                        </div>
                    `).join('')}
                </div>
            </div>

            <div class="home-section">
                <div class="section-header">
                    <h2>Recent Activity</h2>
                </div>
                <div class="activity-feed">
                    ${data.recent_logs && data.recent_logs.length > 0 ? data.recent_logs.map(log => `
                        <div class="activity-item">
                            <div class="activity-avatar">👤</div>
                            <div class="activity-text">
                                <strong>${log.username}</strong> watched <em>${log.film_title}</em>
                                <span class="activity-rating">${'⭐'.repeat(Math.floor(log.rating))}</span>
                            </div>
                        </div>
                    `).join('') : '<div class="empty-state"><p>No recent activity</p></div>'}
                </div>
            </div>
        `;
    } catch (error) {
        console.error('Home page error:', error);
        document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Error loading home page</h3></div>`;
    }
}

// Films Browser
function showFilmsPage() {
    document.getElementById('app').innerHTML = `
        <div class="films-header">
            <h1>Browse Films</h1>
            <div class="films-filters">
                <button class="filter-btn active" onclick="filterFilms('all')">All Films</button>
                <button class="filter-btn" onclick="filterFilms('top')">Top Rated</button>
                <button class="filter-btn" onclick="filterFilms('recent')">Recently Added</button>
            </div>
        </div>
        <div class="films-grid" id="filmsGrid">
            ${allFilms.slice(0, 50).map(film => createFilmCard(film)).join('')}
        </div>
    `;
}

function filterFilms(type) {
    document.querySelectorAll('.filter-btn').forEach(btn => btn.classList.remove('active'));
    event.target.classList.add('active');

    let filtered = [...allFilms];
    
    if (type === 'top') {
        filtered = filtered.sort((a, b) => (b.vote_average || 0) - (a.vote_average || 0)).slice(0, 100);
    } else if (type === 'recent') {
        filtered = filtered.slice(-100).reverse();
    }

    document.getElementById('filmsGrid').innerHTML = filtered.slice(0, 50).map(film => createFilmCard(film)).join('');
}

function createFilmCard(film) {
    const posterUrl = film.poster_path || 'https://via.placeholder.com/300x450/2C3440/FFFFFF?text=No+Poster';

    return `
        <div class="film-card" onclick="navigateTo('film-detail', {filmId: ${film.film_id}})">
            <img src="${posterUrl}" alt="${film.title}" class="film-poster" onerror="this.src='https://via.placeholder.com/300x450/2C3440/FFFFFF?text=No+Image'">
            <div class="film-overlay">
                <div class="film-overlay-title">${film.title}</div>
                <div class="film-overlay-year">${film.year}</div>
                <div class="film-overlay-icons">
                    <button class="icon-btn" title="Watch" onclick="event.stopPropagation(); openLogModal(${film.film_id})">👁</button>
                    <button class="icon-btn" title="Like" onclick="event.stopPropagation(); toggleInteraction(${film.film_id}, 1)">❤</button>
                    <button class="icon-btn" title="Watchlist" onclick="event.stopPropagation(); toggleInteraction(${film.film_id}, 2)">🕐</button>
                </div>
            </div>
        </div>
    `;
}

// MODULE C: FILM DETAIL PAGE
async function showFilmDetailPage(filmId) {
    try {
        const response = await fetch(`${API_BASE}/film/${filmId}`);
        const data = await response.json();

        if (!response.ok || !data.film) {
            document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Film not found</h3></div>`;
            return;
        }

        const film = data.film;
        const backdropUrl = film.backdrop_path || film.poster_path || 'https://via.placeholder.com/1280x720/2C3440/FFFFFF?text=No+Backdrop';
        const posterUrl = film.poster_path || 'https://via.placeholder.com/230x345/2C3440/FFFFFF?text=No+Poster';

        document.getElementById('app').innerHTML = `
            <div class="detail-backdrop" style="background-image: url('${backdropUrl}')"></div>
            <div class="detail-content">
                <div class="detail-hero">
                    <img src="${posterUrl}" alt="${film.title}" class="detail-poster" onerror="this.src='https://via.placeholder.com/230x345/2C3440/FFFFFF?text=No+Image'">
                    <div class="detail-info">
                        <h1 class="detail-title">${film.title}</h1>
                        <div class="detail-year">${film.year}</div>
                        ${film.tagline ? `<p class="detail-tagline">"${film.tagline}"</p>` : ''}
                        
                        <div class="detail-stats">
                            <div class="stat-item">
                                <span>Rating:</span>
                                <span class="stat-value">⭐ ${film.vote_average.toFixed(1)}</span>
                            </div>
                            <div class="stat-item">
                                <span>Runtime:</span>
                                <span class="stat-value">${film.runtime} min</span>
                            </div>
                        </div>

                        <div class="detail-actions">
                            <button class="action-btn ${film.watched ? 'active' : ''}" onclick="openLogModal(${film.film_id})">
                                <span>👁</span> ${film.watched ? 'WATCHED' : 'MARK AS WATCHED'}
                            </button>
                            <button class="action-btn ${film.liked ? 'active' : ''}" onclick="toggleInteraction(${film.film_id}, 1)">
                                <span>❤</span> ${film.liked ? 'LIKED' : 'LIKE'}
                            </button>
                            <button class="action-btn ${film.watchlisted ? 'active' : ''}" onclick="toggleInteraction(${film.film_id}, 2)">
                                <span>🕐</span> ${film.watchlisted ? 'IN WATCHLIST' : 'WATCHLIST'}
                            </button>
                        </div>

                        <div class="detail-section">
                            <h3>Director</h3>
                            <p>${film.director}</p>
                        </div>

                        <div class="detail-section">
                            <h3>Cast</h3>
                            <p>${film.cast_summary || 'No cast information available'}</p>
                        </div>
                    </div>
                </div>
            </div>
        `;
    } catch (error) {
        console.error('Film detail error:', error);
        document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Error loading film</h3></div>`;
    }
}

// MODULE D: LOGGING MODAL
function openLogModal(filmId) {
    const film = allFilms.find(f => f.film_id === filmId);
    const filmTitle = film ? film.title : 'Film';

    const modal = document.createElement('div');
    modal.className = 'modal-overlay';
    modal.innerHTML = `
        <div class="modal-content">
            <div class="modal-header">
                <h2>Log "${filmTitle}"</h2>
                <button class="modal-close" onclick="this.closest('.modal-overlay').remove()">&times;</button>
            </div>
            <div class="modal-body">
                <div class="form-group">
                    <label>I watched this on:</label>
                    <input type="date" id="logDate" value="${new Date().toISOString().split('T')[0]}">
                </div>
                <div class="form-group">
                    <label>Rating:</label>
                    <div class="star-rating">
                        ${[1, 2, 3, 4, 5].map(n => `<span class="star" data-rating="${n}" onclick="selectRating(${n})">☆</span>`).join('')}
                    </div>
                </div>
                <div class="form-group">
                    <label>Review:</label>
                    <textarea id="logReview" placeholder="Add a review..." rows="4"></textarea>
                </div>
                <button class="btn-primary" onclick="submitLog(${filmId})">LOG FILM</button>
            </div>
        </div>
    `;

    document.body.appendChild(modal);
}

let selectedRating = 0;

function selectRating(rating) {
    selectedRating = rating;
    document.querySelectorAll('.star').forEach((star, index) => {
        star.textContent = index < rating ? '★' : '☆';
    });
}

async function submitLog(filmId) {
    if (selectedRating === 0) {
        alert('Please select a rating');
        return;
    }

    const review = document.getElementById('logReview').value;

    try {
        const response = await fetch(`${API_BASE}/logs`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                film_id: filmId,
                rating: parseFloat(selectedRating),
                review_text: review
            })
        });

        if (response.ok) {
            document.querySelector('.modal-overlay').remove();
            alert('✓ Film logged successfully!');
            if (currentRoute === 'film-detail') {
                navigateTo('film-detail', { filmId });
            }
        } else {
            const data = await response.json();
            alert('Failed: ' + (data.message || 'Unknown error'));
        }
    } catch (error) {
        alert('Error: ' + error.message);
    }
}

// MODULE E: USER PROFILE & DIARY
async function showProfilePage(userId) {
    try {
        const [profileRes, diaryRes, favoritesRes] = await Promise.all([
            fetch(`${API_BASE}/user/${userId}/profile`),
            fetch(`${API_BASE}/user/${userId}/logs`),
            fetch(`${API_BASE}/user/${userId}/favorites`)
        ]);

        const profileData = await profileRes.json();
        const diaryData = await diaryRes.json();
        const favoritesData = await favoritesRes.json();

        if (!profileRes.ok) {
            document.getElementById('app').innerHTML = `<div class="empty-state"><h3>User not found</h3></div>`;
            return;
        }

        const profile = profileData.profile;
        const logs = diaryData.logs || [];
        const favorites = favoritesData.films || [];

        document.getElementById('app').innerHTML = `
            <div class="profile-container">
                <div class="profile-header">
                    <div class="profile-avatar">👤</div>
                    <div class="profile-info">
                        <h1>${profile.username}</h1>
                        <p>${profile.bio}</p>
                    </div>
                </div>

                <div class="profile-stats">
                    <div class="profile-stat">
                        <div class="profile-stat-value">${profile.total_films}</div>
                        <div class="profile-stat-label">FILMS</div>
                    </div>
                    <div class="profile-stat">
                        <div class="profile-stat-value">${profile.this_year}</div>
                        <div class="profile-stat-label">THIS YEAR</div>
                    </div>
                    <div class="profile-stat">
                        <div class="profile-stat-value">${profile.watchlist_count}</div>
                        <div class="profile-stat-label">WATCHLIST</div>
                    </div>
                </div>

                ${favorites.length > 0 ? `
                    <div class="profile-section">
                        <h2>Favorite Films</h2>
                        <div class="favorites-grid">
                            ${favorites.map(film => `
                                <div class="favorite-card" onclick="navigateTo('film-detail', {filmId: ${film.film_id}})">
                                    <img src="${film.poster_path || 'https://via.placeholder.com/150x225'}" alt="${film.title}" 
                                         onerror="this.src='https://via.placeholder.com/150x225/2C3440/FFFFFF?text=No+Image'">
                                </div>
                            `).join('')}
                        </div>
                    </div>
                ` : ''}

                <div class="profile-section">
                    <h2>Diary</h2>
                    ${logs.length > 0 ? `
                        <table class="diary-table">
                            <thead>
                                <tr>
                                    <th>DATE</th>
                                    <th>FILM</th>
                                    <th>RATING</th>
                                    <th>REVIEW</th>
                                </tr>
                            </thead>
                            <tbody>
                                ${logs.map(log => {
                                    const film = allFilms.find(f => f.film_id === log.film_id);
                                    const filmTitle = film ? film.title : 'Unknown';
                                    const date = new Date(log.log_date * 1000).toLocaleDateString();
                                    return `
                                        <tr>
                                            <td>${date}</td>
                                            <td class="diary-film-title">${filmTitle}</td>
                                            <td class="diary-rating">${'⭐'.repeat(Math.floor(log.rating))}</td>
                                            <td>${log.review_text || '-'}</td>
                                        </tr>
                                    `;
                                }).join('')}
                            </tbody>
                        </table>
                    ` : '<div class="empty-state"><p>No diary entries yet</p></div>'}
                </div>
            </div>
        `;
    } catch (error) {
        console.error('Profile error:', error);
        document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Error loading profile</h3></div>`;
    }
}

function showDiaryPage() {
    showProfilePage(currentUser.userId);
}

// MODULE F: WATCHLIST
async function showWatchlistPage() {
    try {
        const response = await fetch(`${API_BASE}/user/${currentUser.userId}/watchlist`);
        const data = await response.json();

        const films = data.films || [];

        document.getElementById('app').innerHTML = `
            <div class="watchlist-container">
                <div class="watchlist-header">
                    <h1>My Watchlist</h1>
                    <p>${films.length} films</p>
                </div>
                ${films.length > 0 ? `
                    <div class="films-grid">
                        ${films.map(film => createFilmCard(film)).join('')}
                    </div>
                ` : '<div class="empty-state"><h3>Your watchlist is empty</h3><p>Start adding films you want to watch!</p></div>'}
            </div>
        `;
    } catch (error) {
        console.error('Watchlist error:', error);
        document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Error loading watchlist</h3></div>`;
    }
}

// Interactions
async function toggleInteraction(filmId, type) {
    try {
        const response = await fetch(`${API_BASE}/interaction`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ film_id: filmId, type: type })
        });

        const data = await response.json();

        if (response.ok) {
            const action = data.action === 'added' ? 'Added to' : 'Removed from';
            const list = type === 1 ? 'favorites' : 'watchlist';
            alert(`${action} ${list}!`);

            if (currentRoute === 'film-detail') {
                navigateTo('film-detail', { filmId });
            }
        }
    } catch (error) {
        console.error('Interaction error:', error);
    }
}

// Search
function openSearch() {
    document.getElementById('searchOverlay').classList.add('active');
    document.getElementById('searchInput').focus();
}

function closeSearch() {
    document.getElementById('searchOverlay').classList.remove('active');
    document.getElementById('searchInput').value = '';
    document.getElementById('searchResults').innerHTML = '';
}

async function handleSearch(e) {
    const query = e.target.value.trim();
    
    if (query.length < 2) {
        document.getElementById('searchResults').innerHTML = '';
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/search?q=${encodeURIComponent(query)}`);
        const data = await response.json();
        const results = data.films || [];

        if (results.length === 0) {
            document.getElementById('searchResults').innerHTML = `
                <div class="empty-state"><p>No films found for "${query}"</p></div>
            `;
            return;
        }

        document.getElementById('searchResults').innerHTML = results.map(film => {
            const posterUrl = film.poster_path || 'https://via.placeholder.com/50x75/2C3440/FFFFFF?text=?';

            return `
                <div class="search-result-item" onclick="closeSearch(); navigateTo('film-detail', {filmId: ${film.film_id}})">
                    <img src="${posterUrl}" class="search-result-poster" onerror="this.src='https://via.placeholder.com/50x75/2C3440/FFFFFF?text=?'">
                    <div class="search-result-info">
                        <h4>${film.title}</h4>
                        <p>${film.year} • ${film.director}</p>
                    </div>
                </div>
            `;
        }).join('');
    } catch (error) {
        console.error('Search error:', error);
    }
}
