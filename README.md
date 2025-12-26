# CINELOG - Movie Rating Platform

A full-stack movie rating and diary platform built with C++ backend and vanilla JavaScript frontend, inspired by Letterboxd. Features a custom B-Tree database implementation with templated data structures for high-performance disk storage.

## 🏗️ Architecture

**3-Layer Backend Architecture:**
- **Database Layer**: Custom B-Tree implementation (Order 100) with binary file storage
- **Service Layer**: Business logic and authentication (ServiceController)
- **Network Layer**: HTTP server with JSON API and RBAC authorization

**Frontend**: Single-page vanilla JavaScript application with dark Letterboxd-inspired UI

## 📁 Project Structure

```
Cinelog/
├── backend/
│   ├── include/
│   │   ├── ds/             # Data structures (BTree, HashMap)
│   │   ├── models/         # Data models (User, Film, Log, Genre, List)
│   │   ├── service/        # Service layer (ServiceController)
│   │   ├── network/        # HTTP server implementation
│   │   └── utils/          # JSON loader utilities
│   └── src/
│       └── main.cpp        # Server entry point
├── frontend/
│   ├── index.html          # Main HTML page
│   ├── styles.css          # Letterboxd-inspired styling
│   └── app.js              # Frontend application logic
├── data/                   # Database files (auto-generated)
│   ├── users.bin
│   ├── films.bin
│   ├── logs.bin
│   ├── genres.bin
│   └── *.json             # Initial data files
└── populator.py           # TMDB data fetcher
```

## 🚀 Getting Started

### Prerequisites

- **C++ Compiler**: MinGW (g++) or MSVC with C++17 support
- **Python 3.x**: For data population script
- **Python Libraries**: `requests` (install with `pip install requests`)

### Step 1: Populate the Database (OPTIONAL BUT RECOMMENDED)

This step fetches real movie data from TMDB. **Note: This takes 3-5 minutes due to API rate limiting.**

```powershell
# Install Python dependencies
pip install requests

# Run the populator script
python populator.py
```

This creates JSON files in the `data/` folder with:
- 50 popular movies from TMDB
- Genre information
- Sample users (including admin)
- Sample diary logs

**Default Admin Credentials:**
- Username: `admin`
- Password: `admin123`

### Step 2: Compile the C++ Backend

**Easy Method (Recommended):**
```powershell
# Just double-click compile.bat
# Or run it from command line:
./compile.bat
```

**Manual Method:**
```powershell
# Navigate to backend directory
cd backend

# Compile with g++ (MinGW)
g++ -std=c++17 -o server.exe src/main.cpp -lws2_32

# The -lws2_32 flag is required for Windows Sockets
```

**Alternative: Use the provided VS Code task**
- Press `Ctrl+Shift+B` in VS Code
- Select "C/C++: g++.exe build active file"

### Step 3: Run the Backend Server

**Easy Method (Recommended):**
```powershell
# Just double-click start_server.bat
# Or run it from command line:
./start_server.bat
```

**Manual Method:**
```powershell
# From the backend directory
cd backend
./server.exe

# Or from the root directory
./backend/server.exe
```

You should see:
```
==================================
    CINELOG Backend Server
==================================

Created 'data' directory
Database is empty. Loading initial data from JSON files...
Loading 4 users...
Loading 50 films...
Loading 26 genres...
Loading 34 logs...
Initial data loaded successfully!
Server started on port 8080
Server is running...
Press Ctrl+C to stop
```

### Step 4: Open the Frontend

Simply open `frontend/index.html` in your web browser. The frontend will automatically connect to `http://localhost:8080`.

**Recommended Browsers:**
- Chrome/Edge (best compatibility)
- Firefox
- Safari (CORS may require adjustments)

## 🎮 Using Cinelog

### Login Screen
- **Default Admin**: username = `admin`, password = `admin123`
- **Sample Users**: username = `john_doe`, `jane_smith`, or `cinephile`, password = `password123`
- Or create a new account

### User Features
1. **Films Page**: Browse popular movies in a grid layout
2. **Film Details**: Click any poster to view details and log a rating
3. **Diary**: View your personal film diary with ratings and reviews

### Admin Features
Admins have access to an additional **Admin Dashboard** with:
- **Add Film**: Manually add movies to the database
- **Manage Films**: View and delete films
- **Manage Users**: View and delete users (except admin)

## 🔑 API Endpoints

### Authentication
- `POST /api/login` - User login
- `POST /api/register` - User registration
- `POST /api/logout` - User logout

### Films
- `GET /api/films` - Get all films
- `GET /api/films/:id` - Get film by ID
- `POST /api/admin/add_film` - Add film (Admin only)
- `POST /api/admin/delete_film` - Delete film (Admin only)

### Logs (Diary Entries)
- `POST /api/log_entry` - Create diary entry
- `GET /api/logs/user/:id` - Get user's logs

### Admin
- `GET /api/admin/users` - Get all users (Admin only)
- `POST /api/admin/delete_user` - Delete user (Admin only)

### Genres
- `GET /api/genres` - Get all genres

## 🛠️ Technical Details

### B-Tree Implementation
- **Order**: 100 (optimized for 4KB/8KB disk pages)
- **Templated**: Supports any data type with `getId()` method
- **Operations**: Insert, Search, Delete, Update
- **File Format**: Binary serialization with header (rootPos, nextPos)

### Data Models

**User** (429 bytes)
- user_id, username, email, password_hash, bio, join_date, isAdmin

**Film** (412 bytes)
- film_id, tmdb_id, title, release_year, runtime, cast_summary, director, genre_ids[3]

**Log** (280 bytes)
- log_id, user_id, film_id, rating, review_preview, watch_date

**Genre** (36 bytes)
- genre_id, name

**List** (328 bytes)
- list_id, user_id, title, description

### Security Features
- Role-Based Access Control (RBAC)
- Admin authorization checks in backend (not just frontend)
- Session management with localStorage
- HTTP 403 responses for unauthorized requests

### Memory Management
- Fixed memory leak in HashMap destructor (string key cleanup)
- RAII principles for file handles
- Proper cleanup of dynamic allocations

## 🐛 Troubleshooting

### "Server failed to start" or "Bind failed"
- **Cause**: Port 8080 is already in use
- **Solution**: Kill the process using port 8080 or change the port in `main.cpp` and `app.js`

```powershell
# Find process using port 8080
netstat -ano | findstr :8080

# Kill the process (replace PID with actual process ID)
taskkill /F /PID <PID>
```

### "Connection error" in frontend
- **Cause**: Backend server is not running
- **Solution**: Start the backend server first
- Check that the server shows "Server started on port 8080"

### Compilation Errors
```powershell
# Error: 'std::is_same_v' not found
# Solution: Use C++17 standard
g++ -std=c++17 -o server.exe src/main.cpp -lws2_32

# Error: Cannot find -lws2_32
# Solution: Ensure you're using MinGW on Windows
# For Linux, remove -lws2_32 and use native sockets
```

### Database Issues
If the database becomes corrupted:
```powershell
# Delete all .bin files and restart
Remove-Item data/*.bin

# The server will reinitialize from JSON files
./backend/server.exe
```

### No Films Showing
- Run `populator.py` to generate film data
- Or manually add films through the Admin Dashboard

## 📊 Performance Notes

- **B-Tree Order 100**: Optimized for disk I/O with 4KB pages
- **Capacity**: Supports millions of records with logarithmic search time
- **Disk Usage**: 
  - ~50 films: ~20KB
  - ~100 films: ~40KB
  - ~1000 films: ~400KB

## 🔮 Future Enhancements

- [ ] Search functionality (by title, director, genre)
- [ ] User profiles with avatars
- [ ] Social features (follow users, activity feed)
- [ ] Movie recommendations algorithm
- [ ] RESTful authentication with JWT tokens
- [ ] Full People table (separate actors/directors)
- [ ] Custom user lists implementation
- [ ] Review comments and likes
- [ ] Movie poster image caching

## 📝 License

This project is for educational purposes. TMDB API data is used under TMDB's terms of service.

## 🙏 Credits

- **TMDB API** for movie data
- **Letterboxd** for design inspiration
- Built as a Data Structures & Algorithms project

---

**Made with ❤️ and C++**
