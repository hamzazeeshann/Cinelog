// Cinelog v2.0 - Complete Implementation

const API_BASE = 'http://localhost:8080/api';

let currentUser = null;
let allFilms = [];
let currentRoute = 'home';

// Toast notification system
function showToast(message, type = 'success') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.textContent = message;
    document.body.appendChild(toast);
    
    setTimeout(() => toast.classList.add('show'), 10);
    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => toast.remove(), 300);
    }, 2500);
}

// Initialization
document.addEventListener('DOMContentLoaded', () => {
    checkAuth();
    setupEventListeners();
});

let currentSearchType = 'films';

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
    
    // Search tab switching
    document.querySelectorAll('.search-tab').forEach(tab => {
        tab.addEventListener('click', () => {
            document.querySelectorAll('.search-tab').forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            currentSearchType = tab.dataset.searchType;
            const input = document.getElementById('searchInput');
            input.placeholder = currentSearchType === 'films' ? 'Search films...' : 'Search people...';
            input.value = '';
            document.getElementById('searchResults').innerHTML = '';
        });
    });
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
        
        // Admin users get a completely different interface
        if (currentUser.isAdmin) {
            await showAdminDashboard();
            return;
        }
        
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
                    <div id="authMode" style="margin-bottom: 20px; text-align: center; font-weight: 500;">
                        <span id="modeText">SIGN IN</span>
                    </div>
                    <div class="form-group">
                        <label>Username</label>
                        <input type="text" id="username" required>
                    </div>
                    <div class="form-group" id="emailGroup" style="display: none;">
                        <label>Email</label>
                        <input type="email" id="email">
                    </div>
                    <div class="form-group" id="bioGroup" style="display: none;">
                        <label>Bio</label>
                        <textarea id="bio" rows="3"></textarea>
                    </div>
                    <div class="form-group">
                        <label>Password</label>
                        <input type="password" id="password" required>
                    </div>
                    <button type="submit" class="btn-primary" id="submitBtn">SIGN IN</button>
                </form>
                <div class="toggle-auth">
                    <a href="#" id="toggleMode">Create an account</a>
                </div>
            </div>
        </div>
    `;

    let isRegisterMode = false;
    
    document.getElementById('toggleMode').addEventListener('click', (e) => {
        e.preventDefault();
        isRegisterMode = !isRegisterMode;
        
        const emailGroup = document.getElementById('emailGroup');
        const bioGroup = document.getElementById('bioGroup');
        const modeText = document.getElementById('modeText');
        const submitBtn = document.getElementById('submitBtn');
        const toggleLink = document.getElementById('toggleMode');
        
        if (isRegisterMode) {
            emailGroup.style.display = 'block';
            bioGroup.style.display = 'block';
            document.getElementById('email').required = true;
            modeText.textContent = 'SIGN UP';
            submitBtn.textContent = 'CREATE ACCOUNT';
            toggleLink.textContent = 'Already have an account?';
        } else {
            emailGroup.style.display = 'none';
            bioGroup.style.display = 'none';
            document.getElementById('email').required = false;
            modeText.textContent = 'SIGN IN';
            submitBtn.textContent = 'SIGN IN';
            toggleLink.textContent = 'Create an account';
        }
    });

    document.getElementById('authForm').addEventListener('submit', (e) => handleAuth(e, isRegisterMode));
}

async function handleAuth(e, isRegister = false) {
    e.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;

    if (!username || !password) {
        showError('Username and password are required');
        return;
    }

    try {
        let endpoint = '/login';
        let bodyData = { username, password };
        
        if (isRegister) {
            endpoint = '/register';
            const email = document.getElementById('email').value;
            const bio = document.getElementById('bio').value || '';
            
            if (!email) {
                showError('Email is required for registration');
                return;
            }
            
            bodyData = { username, email, password, bio };
        }
        
        const response = await fetch(`${API_BASE}${endpoint}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(bodyData)
        });

        const data = await response.json();

        if (response.ok && data.token) {
            localStorage.setItem('token', data.token);
            showToast(isRegister ? 'Account created successfully!' : 'Logged in successfully!', 'success');
            setTimeout(() => location.reload(), 500);
        } else {
            showError(data.message || data.error || 'Authentication failed');
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
                    <button class="icon-btn" title="Watch" onclick="event.stopPropagation(); openLogModal(${film.film_id})">○</button>
                    <button class="icon-btn" title="Like" onclick="event.stopPropagation(); toggleInteraction(${film.film_id}, 1)">♥</button>
                    <button class="icon-btn" title="Watchlist" onclick="event.stopPropagation(); toggleInteraction(${film.film_id}, 2)">+</button>
                </div>
            </div>
        </div>
    `;
}

// MODULE C: FILM DETAIL PAGE
async function showFilmDetailPage(filmId) {
    try {
        const headers = {};
        if (currentUser) {
            headers['Authorization'] = localStorage.getItem('token');
        }
        
        const response = await fetch(`${API_BASE}/film/${filmId}`, { headers });
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
                                <span>○</span> ${film.watched ? 'WATCHED' : 'MARK AS WATCHED'}
                            </button>
                            <button class="action-btn ${film.liked ? 'active' : ''}" onclick="toggleInteraction(${film.film_id}, 1)">
                                <span>♥</span> ${film.liked ? 'LIKED' : 'LIKE'}
                            </button>
                            <button class="action-btn ${film.watchlisted ? 'active' : ''}" onclick="toggleInteraction(${film.film_id}, 2)">
                                <span>+</span> ${film.watchlisted ? 'IN WATCHLIST' : 'WATCHLIST'}
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
        showToast('Please select a rating', 'error');
        return;
    }

    if (!currentUser) {
        showToast('Please login first', 'error');
        return;
    }

    const review = document.getElementById('logReview').value;

    try {
        const response = await fetch(`${API_BASE}/logs`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': localStorage.getItem('token')
            },
            body: JSON.stringify({
                user_id: currentUser.userId,
                film_id: filmId,
                rating: parseFloat(selectedRating),
                review_text: review
            })
        });

        if (response.ok) {
            document.querySelector('.modal-overlay').remove();
            showToast('Film logged successfully!');
            if (currentRoute === 'film-detail') {
                navigateTo('film-detail', { filmId });
            }
        } else {
            const data = await response.json();
            showToast(data.message || 'Failed to log film', 'error');
        }
    } catch (error) {
        showToast('Error: ' + error.message, 'error');
    }
}

// MODULE E: USER PROFILE & DIARY
async function showProfilePage(userId) {
    try {
        const [profileRes, diaryRes, favoritesRes, socialRes] = await Promise.all([
            fetch(`${API_BASE}/user/${userId}/profile`),
            fetch(`${API_BASE}/user/${userId}/logs`),
            fetch(`${API_BASE}/user/${userId}/favorites`),
            fetch(`${API_BASE}/user/${userId}/social`, {
                headers: currentUser ? { 'Authorization': localStorage.getItem('token') } : {}
            })
        ]);

        const profileData = await profileRes.json();
        const diaryData = await diaryRes.json();
        const favoritesData = await favoritesRes.json();
        const socialData = await socialRes.json();

        if (!profileRes.ok) {
            document.getElementById('app').innerHTML = `<div class="empty-state"><h3>User not found</h3></div>`;
            return;
        }

        const profile = profileData.profile;
        const logs = diaryData.logs || [];
        const favorites = favoritesData.films || [];
        const isOwnProfile = currentUser && currentUser.userId === userId;
        const isFollowing = socialData.is_following || false;
        const followersCount = socialData.followers_count || 0;
        const followingCount = socialData.following_count || 0;

        document.getElementById('app').innerHTML = `
            <div class="profile-container">
                <div class="profile-header">
                    <div class="profile-avatar">👤</div>
                    <div class="profile-info">
                        <h1>${profile.username}</h1>
                        <p>${profile.bio}</p>
                        ${!isOwnProfile && currentUser ? `
                            <button class="btn-follow" onclick="toggleFollow(${userId}, '${profile.username}')">
                                ${isFollowing ? 'Unfollow' : 'Follow'}
                            </button>
                        ` : ''}
                    </div>
                </div>

                <div class="profile-tabs">
                    <button class="tab-btn active" data-tab="overview">Overview</button>
                    <button class="tab-btn" data-tab="network">Network</button>
                </div>

                <div id="overview-tab" class="tab-content active">
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
                        <div class="profile-stat">
                            <div class="profile-stat-value">${followersCount}</div>
                            <div class="profile-stat-label">FOLLOWERS</div>
                        </div>
                        <div class="profile-stat">
                            <div class="profile-stat-value">${followingCount}</div>
                            <div class="profile-stat-label">FOLLOWING</div>
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

                <div id="network-tab" class="tab-content">
                    <div id="networkContent">Loading network...</div>
                </div>
            </div>
        `;
        
        // Tab switching
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', async () => {
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
                btn.classList.add('active');
                document.getElementById(`${btn.dataset.tab}-tab`).classList.add('active');
                
                if (btn.dataset.tab === 'network') {
                    await loadUserNetwork(userId);
                }
            });
        });
    } catch (error) {
        console.error('Profile error:', error);
        document.getElementById('app').innerHTML = `<div class="empty-state"><h3>Error loading profile</h3></div>`;
    }
}

async function loadUserNetwork(userId) {
    try {
        const response = await fetch(`${API_BASE}/user/${userId}/network`);
        const data = await response.json();
        
        const followers = data.followers || [];
        const following = data.following || [];
        
        const networkHtml = `
            <div class="network-section">
                <h3>Following (${following.length})</h3>
                ${following.length > 0 ? `
                    <div class="network-grid">
                        ${following.map(user => `
                            <div class="network-card" onclick="navigateTo('profile', {userId: ${user.user_id}})">
                                <div class="network-avatar">👤</div>
                                <div class="network-username">${user.username}</div>
                            </div>
                        `).join('')}
                    </div>
                ` : '<p style="color: #9AB;">Not following anyone yet</p>'}
            </div>
            
            <div class="network-section">
                <h3>Followers (${followers.length})</h3>
                ${followers.length > 0 ? `
                    <div class="network-grid">
                        ${followers.map(user => `
                            <div class="network-card" onclick="navigateTo('profile', {userId: ${user.user_id}})">
                                <div class="network-avatar">👤</div>
                                <div class="network-username">${user.username}</div>
                            </div>
                        `).join('')}
                    </div>
                ` : '<p style="color: #9AB;">No followers yet</p>'}
            </div>
        `;
        
        document.getElementById('networkContent').innerHTML = networkHtml;
    } catch (error) {
        document.getElementById('networkContent').innerHTML = '<p>Error loading network</p>';
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
    if (!currentUser) {
        showToast('Please login first', 'error');
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/interaction`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': localStorage.getItem('token')
            },
            body: JSON.stringify({
                user_id: currentUser.userId,
                film_id: filmId,
                type: type
            })
        });

        const data = await response.json();

        if (response.ok) {
            const action = data.action === 'added' ? 'Added to' : 'Removed from';
            const list = type === 1 ? 'favorites' : 'watchlist';
            showToast(`${action} ${list}`);

            if (currentRoute === 'film-detail') {
                navigateTo('film-detail', { filmId });
            }
        } else {
            showToast(data.message || 'Action failed', 'error');
        }
    } catch (error) {
        console.error('Interaction error:', error);
        showToast('Connection error', 'error');
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
        const typeParam = currentSearchType === 'people' ? '&type=user' : '';
        const response = await fetch(`${API_BASE}/search?q=${encodeURIComponent(query)}${typeParam}`);
        const data = await response.json();
        
        if (currentSearchType === 'people') {
            const results = data.users || [];
            
            if (results.length === 0) {
                document.getElementById('searchResults').innerHTML = `
                    <div class="empty-state"><p>No people found for "${query}"</p></div>
                `;
                return;
            }
            
            document.getElementById('searchResults').innerHTML = results.map(user => {
                return `
                    <div class="search-result-item" onclick="closeSearch(); navigateTo('profile', {userId: ${user.user_id}})">
                        <div class="search-result-avatar">👤</div>
                        <div class="search-result-info">
                            <h4>${user.username}</h4>
                            <p>${user.bio || 'Cinelog user'}</p>
                        </div>
                    </div>
                `;
            }).join('');
        } else {
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
        }
    } catch (error) {
        console.error('Search error:', error);
    }
}

// ============= ADMIN DASHBOARD =============
async function showAdminDashboard() {
    document.getElementById('navbar').classList.remove('visible');
    document.getElementById('app').innerHTML = `
        <div class="admin-dashboard">
            <div class="admin-header">
                <h1>ADMIN PANEL</h1>
                <button class="btn-danger" onclick="logout()">Logout</button>
            </div>
            
            <div class="admin-tabs">
                <button class="tab-btn active" data-tab="films">Films</button>
                <button class="tab-btn" data-tab="users">Users</button>
                <button class="tab-btn" data-tab="add-film">Add Film</button>
            </div>
            
            <div class="admin-content">
                <div id="films-tab" class="tab-content active">
                    <h2>All Films</h2>
                    <div id="filmsTable">Loading...</div>
                </div>
                
                <div id="users-tab" class="tab-content">
                    <h2>All Users</h2>
                    <div id="usersTable">Loading...</div>
                </div>
                
                <div id="add-film-tab" class="tab-content">
                    <h2>Add Manual Film Entry</h2>
                    <form id="addFilmForm" class="admin-form">
                        <div class="form-row">
                            <div class="form-group">
                                <label>Title*</label>
                                <input type="text" id="filmTitle" required>
                            </div>
                            <div class="form-group">
                                <label>Year*</label>
                                <input type="number" id="filmYear" required min="1900" max="2100">
                            </div>
                        </div>
                        <div class="form-row">
                            <div class="form-group">
                                <label>Runtime (minutes)*</label>
                                <input type="number" id="filmRuntime" required min="1">
                            </div>
                            <div class="form-group">
                                <label>Rating*</label>
                                <input type="number" id="filmRating" required step="0.1" min="0" max="10">
                            </div>
                        </div>
                        <div class="form-group">
                            <label>Director*</label>
                            <input type="text" id="filmDirector" required>
                        </div>
                        <div class="form-group">
                            <label>Cast</label>
                            <input type="text" id="filmCast" placeholder="e.g., Actor 1, Actor 2">
                        </div>
                        <div class="form-group">
                            <label>Tagline</label>
                            <input type="text" id="filmTagline">
                        </div>
                        <div class="form-group">
                            <label>Overview*</label>
                            <textarea id="filmOverview" rows="4" required></textarea>
                        </div>
                        <div class="form-row">
                            <div class="form-group">
                                <label>Poster Path</label>
                                <input type="text" id="filmPoster" placeholder="/path/to/poster.jpg">
                            </div>
                            <div class="form-group">
                                <label>Backdrop Path</label>
                                <input type="text" id="filmBackdrop" placeholder="/path/to/backdrop.jpg">
                            </div>
                        </div>
                        <div class="form-group">
                            <label>Genre IDs (comma-separated)</label>
                            <input type="text" id="filmGenres" placeholder="e.g., 18,80,28">
                        </div>
                        <button type="submit" class="btn-primary">Add Film</button>
                    </form>
                </div>
            </div>
        </div>
    `;
    
    // Tab switching
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
            btn.classList.add('active');
            document.getElementById(`${btn.dataset.tab}-tab`).classList.add('active');
        });
    });
    
    // Load data
    await loadAdminFilms();
    await loadAdminUsers();
    
    // Form submission
    document.getElementById('addFilmForm').addEventListener('submit', handleAddFilm);
}

async function loadAdminFilms() {
    try {
        const response = await fetch(`${API_BASE}/films`);
        const data = await response.json();
        
        const filmsHtml = `
            <table class="admin-table">
                <thead>
                    <tr>
                        <th>ID</th>
                        <th>Title</th>
                        <th>Year</th>
                        <th>Rating</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    ${data.films.map(film => `
                        <tr>
                            <td>${film.film_id}</td>
                            <td>${film.title}</td>
                            <td>${film.release_year}</td>
                            <td>${film.vote_average}</td>
                            <td>
                                <button class="btn-danger-small" onclick="deleteFilm(${film.film_id}, '${film.title.replace(/'/g, "\\'")}')">
                                    Delete
                                </button>
                            </td>
                        </tr>
                    `).join('')}
                </tbody>
            </table>
        `;
        
        document.getElementById('filmsTable').innerHTML = filmsHtml;
    } catch (error) {
        document.getElementById('filmsTable').innerHTML = '<p>Error loading films</p>';
    }
}

async function loadAdminUsers() {
    try {
        const response = await fetch(`${API_BASE}/admin/users`, {
            headers: { 'Authorization': localStorage.getItem('token') }
        });
        const data = await response.json();
        
        const usersHtml = `
            <table class="admin-table">
                <thead>
                    <tr>
                        <th>ID</th>
                        <th>Username</th>
                        <th>Email</th>
                        <th>Role</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    ${data.users.map(user => `
                        <tr>
                            <td>${user.user_id}</td>
                            <td>${user.username}</td>
                            <td>${user.email}</td>
                            <td>${user.isAdmin ? 'Admin' : 'User'}</td>
                            <td>
                                ${user.user_id !== 1 ? `
                                    <button class="btn-danger-small" onclick="deleteUser(${user.user_id}, '${user.username.replace(/'/g, "\\'")}')">
                                        Delete
                                    </button>
                                ` : '<span style="color: #666;">Protected</span>'}
                            </td>
                        </tr>
                    `).join('')}
                </tbody>
            </table>
        `;
        
        document.getElementById('usersTable').innerHTML = usersHtml;
    } catch (error) {
        document.getElementById('usersTable').innerHTML = '<p>Error loading users</p>';
    }
}

async function handleAddFilm(e) {
    e.preventDefault();
    
    const genreStr = document.getElementById('filmGenres').value;
    const genreIds = genreStr ? genreStr.split(',').map(g => parseInt(g.trim())).filter(g => !isNaN(g)) : [];
    
    const filmData = {
        title: document.getElementById('filmTitle').value,
        year: parseInt(document.getElementById('filmYear').value),
        runtime: parseInt(document.getElementById('filmRuntime').value),
        rating: parseFloat(document.getElementById('filmRating').value),
        director: document.getElementById('filmDirector').value,
        cast: document.getElementById('filmCast').value || '',
        tagline: document.getElementById('filmTagline').value || '',
        overview: document.getElementById('filmOverview').value,
        poster_path: document.getElementById('filmPoster').value || '/default.jpg',
        backdrop_path: document.getElementById('filmBackdrop').value || '/default.jpg',
        genre_ids: genreIds
    };
    
    try {
        const response = await fetch(`${API_BASE}/admin/film`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': localStorage.getItem('token')
            },
            body: JSON.stringify(filmData)
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showToast('Film added successfully!', 'success');
            document.getElementById('addFilmForm').reset();
            await loadAdminFilms();
        } else {
            showToast(data.message || 'Failed to add film', 'error');
        }
    } catch (error) {
        showToast('Network error', 'error');
    }
}

async function deleteFilm(filmId, title) {
    if (!confirm(`Are you sure you want to delete "${title}"?`)) return;
    
    try {
        const response = await fetch(`${API_BASE}/admin/film/${filmId}`, {
            method: 'DELETE',
            headers: { 'Authorization': localStorage.getItem('token') }
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showToast('Film deleted successfully!', 'success');
            await loadAdminFilms();
        } else {
            showToast(data.message || 'Failed to delete film', 'error');
        }
    } catch (error) {
        showToast('Network error', 'error');
    }
}

async function deleteUser(userId, username) {
    if (!confirm(`Are you sure you want to delete user "${username}"?`)) return;
    
    try {
        const response = await fetch(`${API_BASE}/admin/user/${userId}`, {
            method: 'DELETE',
            headers: { 'Authorization': localStorage.getItem('token') }
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showToast('User deleted successfully!', 'success');
            await loadAdminUsers();
        } else {
            showToast(data.message || 'Failed to delete user', 'error');
        }
    } catch (error) {
        showToast('Network error', 'error');
    }
}

// ============= SOCIAL FEATURES =============
async function toggleFollow(targetId, targetUsername) {
    if (!currentUser) {
        showToast('Please login first', 'error');
        return;
    }
    
    try {
        // Check current status
        const statusResponse = await fetch(`${API_BASE}/user/${targetId}/social`, {
            headers: { 'Authorization': localStorage.getItem('token') }
        });
        const statusData = await statusResponse.json();
        
        const isFollowing = statusData.is_following;
        const endpoint = isFollowing ? '/social/unfollow' : '/social/follow';
        
        const response = await fetch(`${API_BASE}${endpoint}`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': localStorage.getItem('token')
            },
            body: JSON.stringify({ target_id: targetId })
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showToast(isFollowing ? `Unfollowed ${targetUsername}` : `Following ${targetUsername}`, 'success');
            // Refresh profile
            navigateTo('profile', { userId: targetId });
        } else {
            showToast(data.message || 'Failed to update follow status', 'error');
        }
    } catch (error) {
        showToast('Network error', 'error');
    }
}
