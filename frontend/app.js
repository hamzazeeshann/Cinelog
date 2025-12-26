// Cinelog Frontend Application
const API_BASE = 'http://localhost:8080/api';

// State Management
let currentUser = null;
let currentFilm = null;
let selectedRating = 0;

// View Management
function showView(viewId) {
    document.querySelectorAll('.view').forEach(view => {
        view.classList.remove('active');
    });
    document.getElementById(viewId).classList.add('active');
}

function showToast(message, type = 'success') {
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.className = `toast ${type} show`;
    
    setTimeout(() => {
        toast.classList.remove('show');
    }, 3000);
}

// Authentication
async function login() {
    const username = document.getElementById('loginUsername').value;
    const password = document.getElementById('loginPassword').value;
    
    if (!username || !password) {
        showToast('Please fill in all fields', 'error');
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            currentUser = {
                user_id: data.user_id,
                username: data.username,
                role: data.role
            };
            
            localStorage.setItem('user', JSON.stringify(currentUser));
            
            document.getElementById('navbar').style.display = 'block';
            
            if (data.role === 'admin') {
                document.getElementById('navAdmin').style.display = 'block';
            }
            
            showView('homeView');
            loadFilms();
            showToast(`Welcome back, ${data.username}!`);
        } else {
            showToast(data.message || 'Login failed', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

async function register() {
    const username = document.getElementById('regUsername').value;
    const email = document.getElementById('regEmail').value;
    const password = document.getElementById('regPassword').value;
    const bio = document.getElementById('regBio').value;
    
    if (!username || !email || !password) {
        showToast('Please fill in all required fields', 'error');
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/register`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, email, password, bio })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            showToast('Account created! Please log in.');
            showView('loginView');
        } else {
            showToast(data.message || 'Registration failed', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

function logout() {
    currentUser = null;
    localStorage.removeItem('user');
    document.getElementById('navbar').style.display = 'none';
    document.getElementById('navAdmin').style.display = 'none';
    showView('loginView');
    showToast('Logged out successfully');
}

// Film Management
async function loadFilms() {
    try {
        const response = await fetch(`${API_BASE}/films`);
        const data = await response.json();
        
        if (data.status === 'success') {
            displayFilms(data.films);
        }
    } catch (error) {
        showToast('Failed to load films', 'error');
        console.error(error);
    }
}

function displayFilms(films) {
    const grid = document.getElementById('filmGrid');
    grid.innerHTML = '';
    
    films.forEach(film => {
        const card = document.createElement('div');
        card.className = 'film-card';
        card.onclick = () => openFilmModal(film);
        
        const posterUrl = `https://image.tmdb.org/t/p/w500${getPosterPath(film.tmdb_id)}`;
        
        card.innerHTML = `
            <img class="film-poster" src="${posterUrl}" 
                 alt="${film.title}" 
                 onerror="this.src='https://via.placeholder.com/200x300?text=${encodeURIComponent(film.title)}'" />
            <div class="film-title">${film.title} (${film.release_year})</div>
        `;
        
        grid.appendChild(card);
    });
}

function getPosterPath(tmdbId) {
    // This is a placeholder - in production, you'd fetch this from TMDB
    return `/movie/${tmdbId}.jpg`;
}

function openFilmModal(film) {
    currentFilm = film;
    selectedRating = 0;
    
    document.getElementById('modalTitle').textContent = film.title;
    document.getElementById('modalYear').textContent = `Year: ${film.release_year}`;
    document.getElementById('modalRuntime').textContent = `Runtime: ${film.runtime} minutes`;
    document.getElementById('modalDirector').innerHTML = `<strong>Director:</strong> ${film.director}`;
    document.getElementById('modalCast').innerHTML = `<strong>Cast:</strong> ${film.cast}`;
    
    const posterUrl = `https://image.tmdb.org/t/p/w500${getPosterPath(film.tmdb_id)}`;
    document.getElementById('modalPoster').src = posterUrl;
    document.getElementById('modalPoster').onerror = function() {
        this.src = `https://via.placeholder.com/300x450?text=${encodeURIComponent(film.title)}`;
    };
    
    // Reset stars
    document.querySelectorAll('.star-rating span').forEach(star => {
        star.classList.remove('active');
    });
    document.getElementById('reviewText').value = '';
    
    document.getElementById('movieModal').classList.add('active');
}

function closeModal() {
    document.getElementById('movieModal').classList.remove('active');
    currentFilm = null;
    selectedRating = 0;
}

async function logFilm() {
    if (!currentUser) {
        showToast('Please log in first', 'error');
        return;
    }
    
    if (selectedRating === 0) {
        showToast('Please select a rating', 'error');
        return;
    }
    
    const review = document.getElementById('reviewText').value;
    
    try {
        const response = await fetch(`${API_BASE}/log_entry`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                film_id: currentFilm.film_id,
                rating: selectedRating,
                review: review
            })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            showToast('Film logged successfully!');
            closeModal();
        } else {
            showToast(data.message || 'Failed to log film', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

// Diary Management
async function loadDiary() {
    if (!currentUser) return;
    
    try {
        const response = await fetch(`${API_BASE}/logs/user/${currentUser.user_id}`);
        const data = await response.json();
        
        if (data.status === 'success') {
            displayDiary(data.logs);
        }
    } catch (error) {
        showToast('Failed to load diary', 'error');
        console.error(error);
    }
}

async function displayDiary(logs) {
    const list = document.getElementById('diaryList');
    list.innerHTML = '';
    
    if (logs.length === 0) {
        list.innerHTML = '<p style="color: #99AABB; text-align: center; padding: 2rem;">No diary entries yet. Start logging films!</p>';
        return;
    }
    
    for (const log of logs) {
        // Fetch film details
        const filmResponse = await fetch(`${API_BASE}/films/${log.film_id}`);
        const filmData = await filmResponse.json();
        
        if (filmData.status === 'success') {
            const film = filmData.film;
            const entry = document.createElement('div');
            entry.className = 'diary-entry';
            
            const posterUrl = `https://image.tmdb.org/t/p/w200${getPosterPath(film.tmdb_id)}`;
            const stars = '★'.repeat(Math.floor(log.rating)) + '☆'.repeat(5 - Math.floor(log.rating));
            
            entry.innerHTML = `
                <img class="diary-poster" src="${posterUrl}" 
                     alt="${film.title}"
                     onerror="this.src='https://via.placeholder.com/100x150?text=${encodeURIComponent(film.title)}'" />
                <div class="diary-info">
                    <div class="diary-title">${film.title} (${film.release_year})</div>
                    <div class="diary-rating">${stars} ${log.rating.toFixed(1)}/5</div>
                    <div class="diary-review">${log.review || 'No review'}</div>
                    <div style="color: #99AABB; font-size: 0.85rem; margin-top: 0.5rem;">
                        Watched: ${new Date(log.watch_date * 1000).toLocaleDateString()}
                    </div>
                </div>
            `;
            
            list.appendChild(entry);
        }
    }
}

// Admin Functions
async function loadAdminData() {
    if (!currentUser || currentUser.role !== 'admin') return;
    
    loadAdminFilms();
    loadAdminUsers();
}

async function loadAdminFilms() {
    try {
        const response = await fetch(`${API_BASE}/films`);
        const data = await response.json();
        
        if (data.status === 'success') {
            const container = document.getElementById('adminFilmList');
            container.innerHTML = `
                <table>
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Title</th>
                            <th>Year</th>
                            <th>Director</th>
                            <th>Action</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${data.films.map(film => `
                            <tr>
                                <td>${film.film_id}</td>
                                <td>${film.title}</td>
                                <td>${film.release_year}</td>
                                <td>${film.director}</td>
                                <td><button onclick="deleteFilm(${film.film_id})">Delete</button></td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            `;
        }
    } catch (error) {
        console.error(error);
    }
}

async function loadAdminUsers() {
    try {
        const response = await fetch(`${API_BASE}/admin/users`);
        const data = await response.json();
        
        if (data.status === 'success') {
            const container = document.getElementById('adminUserList');
            container.innerHTML = `
                <table>
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Username</th>
                            <th>Email</th>
                            <th>Admin</th>
                            <th>Action</th>
                        </tr>
                    </thead>
                    <tbody>
                        ${data.users.map(user => `
                            <tr>
                                <td>${user.user_id}</td>
                                <td>${user.username}</td>
                                <td>${user.email}</td>
                                <td>${user.isAdmin ? 'Yes' : 'No'}</td>
                                <td>
                                    ${user.user_id !== 1 ? `<button onclick="deleteUser(${user.user_id})">Delete</button>` : '-'}
                                </td>
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            `;
        }
    } catch (error) {
        console.error(error);
    }
}

async function addFilmAdmin() {
    const title = document.getElementById('adminTitle').value;
    const tmdbId = document.getElementById('adminTmdbId').value;
    const year = document.getElementById('adminYear').value;
    const runtime = document.getElementById('adminRuntime').value;
    const cast = document.getElementById('adminCast').value;
    const director = document.getElementById('adminDirector').value;
    const g1 = document.getElementById('adminGenre1').value;
    const g2 = document.getElementById('adminGenre2').value;
    const g3 = document.getElementById('adminGenre3').value;
    
    if (!title || !year || !director) {
        showToast('Please fill required fields', 'error');
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/admin/add_film`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                title, tmdb_id: parseInt(tmdbId) || 0,
                release_year: parseInt(year), runtime: parseInt(runtime) || 0,
                cast, director,
                genre_id_1: parseInt(g1) || 0,
                genre_id_2: parseInt(g2) || 0,
                genre_id_3: parseInt(g3) || 0
            })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            showToast('Film added successfully!');
            document.querySelectorAll('.admin-form input').forEach(input => input.value = '');
            loadAdminFilms();
            loadFilms();
        } else {
            showToast(data.message || 'Failed to add film', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

async function deleteFilm(filmId) {
    if (!confirm('Are you sure you want to delete this film?')) return;
    
    try {
        const response = await fetch(`${API_BASE}/admin/delete_film`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ film_id: filmId })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            showToast('Film deleted');
            loadAdminFilms();
            loadFilms();
        } else {
            showToast(data.message || 'Failed to delete', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

async function deleteUser(userId) {
    if (!confirm('Are you sure you want to delete this user?')) return;
    
    try {
        const response = await fetch(`${API_BASE}/admin/delete_user`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ user_id: userId })
        });
        
        const data = await response.json();
        
        if (data.status === 'success') {
            showToast('User deleted');
            loadAdminUsers();
        } else {
            showToast(data.message || 'Failed to delete', 'error');
        }
    } catch (error) {
        showToast('Connection error', 'error');
        console.error(error);
    }
}

// Event Listeners
document.addEventListener('DOMContentLoaded', () => {
    // Login/Register
    document.getElementById('loginBtn').addEventListener('click', login);
    document.getElementById('registerBtn').addEventListener('click', register);
    document.getElementById('showRegister').addEventListener('click', (e) => {
        e.preventDefault();
        showView('registerView');
    });
    document.getElementById('showLogin').addEventListener('click', (e) => {
        e.preventDefault();
        showView('loginView');
    });
    
    // Navigation
    document.getElementById('navHome').addEventListener('click', (e) => {
        e.preventDefault();
        showView('homeView');
        loadFilms();
    });
    document.getElementById('navDiary').addEventListener('click', (e) => {
        e.preventDefault();
        showView('diaryView');
        loadDiary();
    });
    document.getElementById('navAdmin').addEventListener('click', (e) => {
        e.preventDefault();
        showView('adminView');
        loadAdminData();
    });
    document.getElementById('navLogout').addEventListener('click', (e) => {
        e.preventDefault();
        logout();
    });
    
    // Modal
    document.querySelector('.close').addEventListener('click', closeModal);
    document.getElementById('logFilmBtn').addEventListener('click', logFilm);
    
    // Star rating
    document.querySelectorAll('.star-rating span').forEach(star => {
        star.addEventListener('click', function() {
            selectedRating = parseInt(this.dataset.rating);
            document.querySelectorAll('.star-rating span').forEach(s => {
                s.classList.remove('active');
                if (parseInt(s.dataset.rating) <= selectedRating) {
                    s.classList.add('active');
                }
            });
        });
    });
    
    // Admin
    document.getElementById('adminAddFilmBtn').addEventListener('click', addFilmAdmin);
    
    // Check for saved session
    const savedUser = localStorage.getItem('user');
    if (savedUser) {
        currentUser = JSON.parse(savedUser);
        document.getElementById('navbar').style.display = 'block';
        if (currentUser.role === 'admin') {
            document.getElementById('navAdmin').style.display = 'block';
        }
        showView('homeView');
        loadFilms();
    }
    
    // Enter key for login
    document.getElementById('loginPassword').addEventListener('keypress', (e) => {
        if (e.key === 'Enter') login();
    });
});
