# CINELOG - Movie Rating Platform

A full-stack movie rating and diary platform built with C++ backend and vanilla JavaScript frontend, inspired by Letterboxd. Features 1000 high-quality TMDB films, custom B-Tree database implementation, Trie-based search, and a modern dark UI with toast notifications.

## 🏗️ Architecture

**3-Layer Backend Architecture:**
- **Database Layer**: Custom B-Tree implementation (Order 100) with binary file storage + Trie search index
- **Service Layer**: Business logic, authentication, and interaction management (ServiceController)
- **Network Layer**: HTTP server with JSON API, CORS support, and Authorization headers

**Frontend**: Single-page vanilla JavaScript application with Letterboxd-inspired dark UI, toast notifications, and minimalist design

**Data Integration**: TMDB API with high-quality posters (w500) and backdrops (original resolution)

## 📁 Project Structure

```
Cinelog/
├── backend/
│   ├── include/
│   │   ├── core/
│   │   │   └── CinelogDB.h              # Main database initialization class
│   │   ├── ds/                           # Data Structures
│   │   │   ├── BTree.h                  # Generic B-Tree (Order 100) for disk storage
│   │   │   ├── HashMap.h                # Hash table for fast lookups
│   │   │   └── Trie.h                   # Prefix tree for film title search
│   │   ├── models/                       # Data Models (POD structs)
│   │   │   ├── User.h                   # User account data (429 bytes)
│   │   │   ├── Film.h                   # Film metadata (728 bytes with TMDB URLs)
│   │   │   ├── FilmLog.h                # Watch log (diary entry)
│   │   │   ├── Log.h                    # Log data (280 bytes)
│   │   │   ├── Genre.h                  # Genre data (36 bytes)
│   │   │   ├── List.h                   # Custom user lists (328 bytes)
│   │   │   └── Interaction.h            # User-film interactions (likes/watchlist)
│   │   ├── network/
│   │   │   └── HTTPServer.h             # HTTP server with request parsing & routing
│   │   ├── service/
│   │   │   └── ServiceController.h      # Business logic layer (600+ lines)
│   │   └── utils/
│   │       └── JSONLoader.h             # JSON file parsing utility
│   ├── src/
│   │   └── main.cpp                     # Server entry point (port 8080)
│   ├── data/                             # Database files (auto-generated)
│   │   ├── *.bin                        # Binary B-Tree files (cached)
│   │   ├── films.json                   # 1000 TMDB films with high-res images
│   │   ├── users.json                   # 4 sample users (admin, alice, bob, charlie)
│   │   ├── logs.json                    # 66+ sample watch logs
│   │   └── genres.json                  # 22 film genres
│   └── build/                            # CMake build files (optional)
├── frontend/
│   ├── index.html                        # Single page app structure
│   ├── app.js                            # 688 lines - 6 core modules + routing
│   └── styles.css                        # 1095 lines - Letterboxd-inspired dark theme
├── Some CSV/
│   ├── imdb_top_1000.csv                # Original IMDB dataset
│   └── imdb_top_1000_updated.csv        # Enhanced with TMDB poster/backdrop URLs
├── populator.py                          # Data generator from CSV to JSON (backend/data/)
├── upgrade_posters.py                    # TMDB API script for high-quality images
├── compile.bat                           # Quick compile script
├── start_server.bat                      # Quick server start script
├── README.md                             # This file
├── QUICKSTART.md                         # 5-minute setup guide
└── DATABASE.md                           # Database schema documentation
```

## 📂 File Descriptions

### Backend Core Files

**`backend/src/main.cpp`** (30 lines)
- Entry point for the server
- Creates HTTPServer instance on port 8080
- Initializes data directory if not exists

**`backend/include/network/HTTPServer.h`** (312 lines)
- HTTP request parser (method, path, body, Authorization header)
- Router with endpoint mapping
- CORS headers for frontend communication
- Authorization token parsing and user context setting
- JSON helper functions (parseJsonField, parseJsonInt, parseJsonFloat)

**`backend/include/service/ServiceController.h`** (591 lines)
- **Authentication**: loginUser(), registerUser(), setCurrentUser()
- **Film Operations**: getAllFilms(), getFilmById(), searchFilms()
- **Logging**: addLog(), getUserLogs(), getRecentLogs()
- **Interactions**: toggleInteraction() (likes/watchlist), getUserWatchlist(), getUserFavorites()
- **User Profiles**: getUserProfile() with stats
- **Home Data**: getHomeData() with hero film, popular films, recent activity
- **Data Loading**: loadInitialData() from JSON, buildSearchIndex() for Trie

### Data Structures

**`backend/include/ds/BTree.h`**
- Generic templated B-Tree with Order 100
- Disk-based storage with binary serialization
- Operations: insert(), search(), deleteRecord(), getAllRecords()
- Node structure with keys, children pointers, and disk positions
- Automatic file creation and header management

**`backend/include/ds/Trie.h`**
- Prefix tree for fast film title search
- Case-insensitive search
- Returns film IDs matching search query
- Built from all film titles on server start

**`backend/include/ds/HashMap.h`**
- Fixed-size hash table (default 1000 buckets)
- Chaining for collision resolution
- Generic key-value storage

### Data Models

**`backend/include/models/Film.h`** (728 bytes)
- film_id, tmdb_id, title (200 chars)
- release_year, runtime, vote_average
- director (100 chars), cast_summary (200 chars)
- tagline (300 chars), overview (500 chars)
- **poster_path (200 chars)** - TMDB w500 URL
- **backdrop_path (200 chars)** - TMDB original URL
- genre_ids[3] - Up to 3 genres per film

**`backend/include/models/User.h`** (429 bytes)
- user_id, username (32 chars), email (64 chars)
- password_hash (64 chars), bio (256 chars)
- join_date, isAdmin flag, avatar_id

**`backend/include/models/Log.h`** (280 bytes)
- log_id, user_id, film_id
- rating (float 0.5-5.0)
- review_preview (200 chars)
- watch_date (Unix timestamp)

**`backend/include/models/Interaction.h`** (16 bytes)
- interaction_id, user_id, film_id
- type (1 = like/favorite, 2 = watchlist)

### Frontend Files

**`frontend/app.js`** (688 lines) - **6 Core Modules:**

1. **Authentication & Setup** (Lines 1-138)
   - checkAuth(), parseToken(), showLoginPage()
   - handleAuth() with username/password validation
   - Toast notification system: showToast(message, type)
   - Event listeners and routing

2. **Home Page Module** (Lines 139-230)
   - showHomePage() with hero film, popular grid, recent activity feed
   - Dynamic poster/backdrop loading with TMDB URLs
   - Error handling with placeholder images

3. **Films Browser Module** (Lines 231-275)
   - showFilmsPage() with 1000 films grid
   - Filter options (all/popular/top-rated)
   - createFilmCard() with overlay icons (○ watch, ♥ like, + watchlist)

4. **Film Detail Page** (Lines 276-362)
   - showFilmDetailPage(filmId) with backdrop image
   - Full film metadata display
   - Action buttons with interaction state (active/inactive)
   - Authorization header for user-specific data

5. **Logging Modal Module** (Lines 363-450)
   - openLogModal(filmId) with rating stars
   - selectRating() for star selection (0.5-5.0)
   - submitLog() with toast feedback
   - Authorization and user_id in POST request

6. **Profile, Diary, Watchlist** (Lines 451-597)
   - showProfilePage() with stats and favorite films
   - showDiaryPage() with user logs table
   - showWatchlistPage() with watchlist grid
   - toggleInteraction() with Authorization header

**Search & Interactions** (Lines 598-688)
- openSearch(), closeSearch(), handleSearch()
- Search results with film poster thumbnails
- Trie-based backend search integration

**`frontend/styles.css`** (1095 lines) - **Design System:**
- CSS Variables for dark theme colors
- Navigation bar with search trigger
- Film grid layouts with hover effects
- Film detail page with backdrop blur
- Modal overlays for logging
- Profile cards and stats layout
- Toast notifications with slide-up animation
- Responsive design for mobile/tablet

### Python Scripts

**`populator.py`** (200+ lines)
- Reads `Some CSV/imdb_top_1000_updated.csv`
- Parses film data with TMDB poster/backdrop URLs
- Generates sample users (admin, alice, bob, charlie)
- Creates 66 sample logs from 3 users
- Outputs to `backend/data/*.json` (films, users, logs, genres)

**`upgrade_posters.py`** (150+ lines)
- TMDB API integration (API key: b3ed21b7e1c2a8df61c48c8db1065831)
- Searches TMDB by film title + year
- Extracts high-quality poster_path (w500) and backdrop_path (original)
- Rate limiting: 40 requests per 10 seconds
- Outputs updated CSV with Backdrop_Link column
- Processed all 1000 films successfully

## ✨ Key Features

### 🎬 Film Management
- **1000 TMDB Films** with high-quality posters (w500 resolution) and backdrops (original resolution)
- **Trie-based Search** for instant film title lookup
- **Genre Filtering** across 22 genres
- **Film Details** with full metadata (director, cast, tagline, runtime, rating)
- **Dynamic Poster Loading** with fallback placeholders

### 📝 Diary & Logging
- **Watch Logging** with 0.5-5.0 star ratings
- **Review Text** up to 200 characters
- **Watch Date** tracking with Unix timestamps
- **Personal Diary** view with all user logs
- **Recent Activity Feed** showing last 5 logs across all users

### ❤️ Interactions
- **Like/Favorite** films with toggle functionality
- **Watchlist** for films to watch later
- **User-specific States** (liked/watchlisted indicators on film pages)
- **Profile Stats** showing total films watched, liked, and watchlisted

### 🔐 Authentication
- **Login System** with username/password
- **Token-based Auth** (format: userId:username:isAdmin)
- **Authorization Headers** for protected API calls
- **Role-based Access** (admin vs regular users)
- **Session Persistence** with localStorage

### 🎨 UI/UX Features
- **Toast Notifications** instead of browser alerts
  - Success (green), Error (red), Info (blue)
  - Auto-dismiss after 2.5 seconds
  - Smooth slide-up animation
- **Minimalist Symbols** (○ watch, ♥ like, + watchlist)
- **Dark Theme** with Letterboxd-inspired design
- **Responsive Grid** for film browsing
- **Hover Effects** with film title overlays
- **Modal Dialogs** for logging films
- **Backdrop Blur** on film detail pages

### 🔍 Search Functionality
- **Real-time Search** with Trie algorithm
- **Case-insensitive** matching
- **Search Overlay** with keyboard shortcuts
- **Result Preview** with film posters
- **Instant Navigation** to film details

### 📊 Data Management
- **B-Tree Storage** (Order 100) for efficient disk I/O
- **Binary Serialization** for fast loading
- **JSON Initial Data** for easy editing
- **Automatic Index Building** on server start
- **Null-termination Safety** for C-string fields

## 🚀 Getting Started

### Prerequisites

- **C++ Compiler**: MinGW (g++) with C++17 support
- **Python 3.x**: For data generation (optional - data already included)
- **Python Libraries**: `requests` (for TMDB API if regenerating data)
- **Web Browser**: Chrome, Edge, Firefox, or Safari

### Quick Start (3 Minutes)

1. **Compile the Backend**
   ```powershell
   cd backend
   g++ -std=c++17 -o server.exe src/main.cpp -lws2_32
   ```

2. **Start the Server**
   ```powershell
   .\server.exe
   ```
   
   You should see:
   ```
   Database is empty. Loading initial data from JSON files...
   Loading 4 users...
   Loading 1000 films...
   Loading 22 genres...
   Loading 66 logs...
   Initial data loaded successfully!
   Building search index with 1000 films...
   Search index built successfully!
   Server started on port 8080
   ```

3. **Open Frontend**
   
   Open `frontend/index.html` in your browser or use:
   ```powershell
   start frontend/index.html
   ```

4. **Login**
   - Username: `admin`
   - Password: `admin123`

**That's it!** You're ready to browse 1000 films with high-quality TMDB posters!

## 🎮 Using Cinelog

### Navigation
- **Home** - Hero film, popular grid, recent activity
- **Films** - Browse all 1000 films with filters
- **Profile** - View your stats and favorite films
- **Diary** - Your complete watch history
- **Watchlist** - Films you want to watch
- **Search** - Find films by title (click search icon or press `/`)

### Sample User Credentials
- **Admin**: `admin` / `admin123`
- **Alice**: `alice` / `password123`
- **Bob**: `bob` / `password123`
- **Charlie**: `charlie` / `password123`

### Core Actions

**Logging a Film:**
1. Click any film poster or navigate to film detail page
2. Click "MARK AS WATCHED" or the ○ symbol
3. Select a rating (0.5-5.0 stars)
4. Optionally add a review (up to 200 characters)
5. Click "LOG FILM"
6. See success toast notification

**Adding to Favorites:**
- Click the ♥ symbol on any film card or detail page
- Toast notification confirms action
- Button turns green when active

**Adding to Watchlist:**
- Click the + symbol on any film card or detail page
- Toast notification confirms action
- Button turns blue when active

**Searching Films:**
1. Click the search icon in navigation
2. Type at least 2 characters
3. Results appear instantly (Trie-based search)
4. Click any result to view film details
5. Press ESC to close search

**Viewing Profile:**
- See total films watched, favorites, and watchlist count
- View your top 4 favorite films with posters
- Access your diary and watchlist from profile

**Browsing Films:**
- Grid view with 300x450px posters
- Hover to see film title and year
- Quick action buttons on hover
- Click poster for full details

## 🔑 API Endpoints

### Authentication
- **POST** `/api/login` - User authentication
  - Body: `{"username": "admin", "password": "admin123"}`
  - Returns: `{"status": "success", "token": "1:admin:1"}`
  
- **POST** `/api/register` - New user registration
  - Body: `{"username": "...", "email": "...", "password": "...", "bio": "..."}`

### Films
- **GET** `/api/films` - Get all 1000 films
  - Returns: Array of films with TMDB poster/backdrop URLs
  
- **GET** `/api/film/{id}` - Get single film by ID
  - Headers: `Authorization: userId:username:isAdmin` (optional, for interaction states)
  - Returns: Film details with watched/liked/watchlisted flags
  
- **GET** `/api/search?q={query}` - Search films by title
  - Returns: Matching films from Trie index

### Logs (Diary)
- **POST** `/api/logs` - Create watch log
  - Headers: `Authorization: token`
  - Body: `{"user_id": 1, "film_id": 5, "rating": 4.5, "review_text": "..."}`
  
- **GET** `/api/user/{id}/logs` - Get user's watch logs
  - Returns: Array of logs with ratings and reviews

- **GET** `/api/logs/recent` - Get recent activity (last 10 logs)
  - Returns: Array with username, film title, rating, date

### Interactions
- **POST** `/api/interaction` - Toggle like/watchlist
  - Headers: `Authorization: token`
  - Body: `{"user_id": 1, "film_id": 5, "type": 1}` (1=like, 2=watchlist)
  - Returns: `{"status": "success", "action": "added"}` or "removed"
  
- **GET** `/api/user/{id}/favorites` - Get user's favorite films
  - Returns: Array of liked films (top 4)
  
- **GET** `/api/user/{id}/watchlist` - Get user's watchlist
  - Returns: Array of watchlisted films

### User Profile
- **GET** `/api/user/{id}/profile` - Get user profile with stats
  - Returns: User info, watch count, favorites count, watchlist count

### Home Data
- **GET** `/api/home_data` - Get homepage data
  - Returns: Hero film, 8 popular films, 5 recent logs

### Genres
- **GET** `/api/genres` - Get all genres
  - Returns: Array of 22 genres

## 🛠️ Technical Details

### Data Structures Implementation

**B-Tree (Order 100)**
- **File**: `backend/include/ds/BTree.h`
- **Purpose**: Disk-based indexing for all data models
- **Order**: 100 keys per node (optimized for 4KB disk pages)
- **Operations**: O(log n) insert, search, delete
- **Storage**: Binary files with header (rootPos, nextPos)
- **Serialization**: Fixed-size POD structs via memcpy
- **Node Structure**: 
  - `int numKeys` - Current key count
  - `T keys[ORDER-1]` - Array of data records
  - `int children[ORDER]` - Disk positions of child nodes
  - `bool isLeaf` - Leaf node flag

**Trie (Prefix Tree)**
- **File**: `backend/include/ds/Trie.h`
- **Purpose**: Fast film title search
- **Structure**: 26 children per node (a-z, case-insensitive)
- **Storage**: In-memory, rebuilt on server start
- **Search**: O(m) where m = query length
- **Index**: Maps film titles to film IDs
- **Build Time**: ~50ms for 1000 films

**HashMap**
- **File**: `backend/include/ds/HashMap.h`
- **Purpose**: Fast key-value lookups
- **Buckets**: 1000 (configurable)
- **Collision**: Chaining with linked lists
- **Hash Function**: Simple modulo-based
- **Use Case**: Genre lookups, quick access tables

### Database Schema

**Film Table** (728 bytes per record)
```cpp
struct Film {
    int film_id;              // Primary key
    int tmdb_id;              // TMDB API reference
    char title[200];          // Film title
    int release_year;         // Release year
    int runtime;              // Minutes
    float vote_average;       // TMDB rating (0-10)
    char director[100];       // Director name
    char cast_summary[200];   // Top 3 actors
    char tagline[300];        // Film tagline
    char overview[500];       // Plot summary
    char poster_path[200];    // TMDB w500 poster URL
    char backdrop_path[200];  // TMDB original backdrop URL
    int genre_ids[3];         // Up to 3 genres
};
```

**User Table** (429 bytes per record)
```cpp
struct User {
    int user_id;              // Primary key
    char username[32];        // Unique username
    char email[64];           // Email address
    char password_hash[64];   // Plain text (demo only!)
    char bio[256];            // User biography
    long join_date;           // Unix timestamp
    bool isAdmin;             // Admin flag
    int avatar_id;            // Avatar selection (1-10)
};
```

**Log Table** (280 bytes per record)
```cpp
struct Log {
    int log_id;               // Primary key
    int user_id;              // Foreign key to User
    int film_id;              // Foreign key to Film
    float rating;             // 0.5-5.0 stars
    char review_preview[200]; // Review text
    long watch_date;          // Unix timestamp
};
```

**Interaction Table** (16 bytes per record)
```cpp
struct Interaction {
    int interaction_id;       // Primary key
    int user_id;              // Foreign key to User
    int film_id;              // Foreign key to Film
    int type;                 // 1=like, 2=watchlist
};
```

### HTTP Server Architecture

**Request Processing Flow:**
1. **Socket Accept**: Accept incoming TCP connection
2. **Parse Request**: Extract method, path, body, Authorization header
3. **Route Matching**: Map path to ServiceController method
4. **Authorization**: Parse token and set user context
5. **Execute**: Call business logic method
6. **JSON Response**: Build HTTP response with CORS headers
7. **Send**: Write response to socket and close connection

**CORS Configuration:**
```cpp
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, OPTIONS
Access-Control-Allow-Headers: Content-Type, Authorization
```

**Authorization Token Format:**
```
userId:username:isAdmin
Example: "1:admin:1"
```

### Frontend Architecture

**Routing System:**
- Hash-based routing (no page reloads)
- Routes: home, films, film-detail, profile, diary, watchlist
- Dynamic content injection into `#app` div
- State management via `currentUser` and `allFilms` globals

**Module Structure:**
1. **Auth Module**: Login, token parsing, session management
2. **Home Module**: Hero section, popular grid, activity feed
3. **Films Module**: Grid view, filters, card generation
4. **Detail Module**: Backdrop, metadata, action buttons
5. **Logging Module**: Modal, star rating, form submission
6. **Profile Module**: Stats, favorites, diary, watchlist

**API Communication:**
```javascript
// With Authorization
fetch(`${API_BASE}/logs`, {
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'Authorization': localStorage.getItem('token')
    },
    body: JSON.stringify({...})
});
```

### TMDB Integration

**Poster URLs:**
```
Base: https://image.tmdb.org/t/p/w500/
Example: https://image.tmdb.org/t/p/w500/9cqNxx0GxF0bflZmeSMuL5tnGzr.jpg
```

**Backdrop URLs:**
```
Base: https://image.tmdb.org/t/p/original/
Example: https://image.tmdb.org/t/p/original/kXfqcdQKsToO0OUXHcrrNCHDBzO.jpg
```

**API Key:** `b3ed21b7e1c2a8df61c48c8db1065831` (stored in upgrade_posters.py)

**Rate Limiting:**
- 40 requests per 10 seconds
- 0.26 second delay between requests
- Automatic retry on failure

### Performance Characteristics

**B-Tree Performance:**
- Order 100 → Height ≤ 3 for 1,000,000 records
- Max disk reads: 3 for any search
- Node size: ~40KB (100 × 400 bytes average)

**Search Performance:**
- Trie search: ~1-2ms for typical query
- B-Tree search: ~5-10ms with cold cache
- Film listing: ~50ms for 1000 films (from cache)

**Disk Usage:**
- films.bin: ~728KB (1000 × 728 bytes)
- users.bin: ~1.7KB (4 × 429 bytes)
- logs.bin: ~18KB (66 × 280 bytes)
- interactions.bin: Grows with user activity
- **Total Database**: ~750KB for 1000 films

**Memory Usage:**
- Trie index: ~1-2MB in RAM
- B-Tree nodes: Loaded on-demand
- ServiceController: ~50KB static
- **Total Server RAM**: ~5-10MB

## 🐛 Troubleshooting

### Server Won't Start

**"Port 8080 already in use"**
```powershell
# Find process using port 8080
netstat -ano | findstr :8080

# Kill the process (replace PID with actual number)
taskkill /F /PID <PID>
```

**"Cannot find -lws2_32"**
- You're not using MinGW on Windows
- Solution: Use MinGW g++ or remove `-lws2_32` for Linux

**Compilation Errors**
```powershell
# Must use C++17 standard
g++ -std=c++17 -o server.exe src/main.cpp -lws2_32

# Check g++ version (need 7.0+)
g++ --version
```

### Frontend Issues

**"Connection error" / API calls fail**
- Backend server is not running
- Solution: Start `backend/server.exe` first
- Check console for "Server started on port 8080"

**Movies not showing / only 49 films**
- Binary files are corrupted
- Solution: Delete all `.bin` files and restart server
  ```powershell
  Remove-Item backend/data/*.bin -Force
  cd backend
  .\server.exe
  ```

**Posters not loading**
- TMDB URLs might be blocked by firewall/network
- Check browser console for image loading errors
- Placeholder images should appear if URLs fail

**Login not working**
- Check username/password (case-sensitive)
- Try: `admin` / `admin123`
- Backend may need fresh data load (delete .bin files)

### Data Issues

**Reset entire database:**
```powershell
# Delete all binary files
Remove-Item backend/data/*.bin -Force

# Restart server (will reload from JSON)
cd backend
.\server.exe
```

**Regenerate film data from scratch:**
```powershell
# Run populator (takes ~1 minute)
python populator.py

# Delete binary files
Remove-Item backend/data/*.bin -Force

# Restart server
cd backend
.\server.exe
```

**Only 49 films instead of 1000:**
- Old binary files cached
- Solution: Delete .bin files before starting server
- Check that `films.json` has 1000 entries:
  ```powershell
  (Get-Content backend/data/films.json | ConvertFrom-Json).Count
  ```

### Common Error Messages

**"Database already contains data. Skipping initial load."**
- Normal message when .bin files exist
- Server loads from binary cache (faster)
- Delete .bin files to force reload from JSON

**"Building search index with X films..."**
- X should be 1000
- If less, delete .bin files and restart

**"Failed to open file"**
- Check that `backend/data/` directory exists
- Ensure JSON files are present
- Server should create directory automatically

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
