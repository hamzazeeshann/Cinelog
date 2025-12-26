#pragma once

#include "../ds/BTree.h"
#include "../ds/Trie.h"
#include "../models/User.h"
#include "../models/Film.h"
#include "../models/Log.h"
#include "../models/Genre.h"
#include "../models/List.h"
#include "../models/Interaction.h"
#include "../utils/JSONLoader.h"
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std;

class ServiceController {
private:
    BTree<User>* userTree;
    BTree<Film>* filmTree;
    BTree<Log>* logTree;
    BTree<Genre>* genreTree;
    BTree<List>* listTree;
    BTree<Interaction>* interactionTree;
    Trie* searchTrie;
    
    int nextUserId;
    int nextFilmId;
    int nextLogId;
    int nextGenreId;
    int nextListId;
    int nextInteractionId;

    int currentUserId;
    bool isLoggedIn;
    bool currentUserIsAdmin;

    string escapeJson(const string& input) {
        ostringstream output;
        for (char c : input) {
            switch (c) {
                case '"': output << "\\\""; break;
                case '\\': output << "\\\\"; break;
                case '\n': output << "\\n"; break;
                case '\r': output << "\\r"; break;
                case '\t': output << "\\t"; break;
                default: output << c; break;
            }
        }
        return output.str();
    }

public:
    ServiceController() : currentUserId(0), isLoggedIn(false), currentUserIsAdmin(false) {
        userTree = new BTree<User>("data/users.bin");
        filmTree = new BTree<Film>("data/films.bin");
        logTree = new BTree<Log>("data/logs.bin");
        genreTree = new BTree<Genre>("data/genres.bin");
        listTree = new BTree<List>("data/lists.bin");
        interactionTree = new BTree<Interaction>("data/interactions.bin");
        searchTrie = new Trie();
        
        nextUserId = userTree->getMaxId() + 1;
        nextFilmId = filmTree->getMaxId() + 1;
        nextLogId = logTree->getMaxId() + 1;
        nextGenreId = genreTree->getMaxId() + 1;
        nextListId = listTree->getMaxId() + 1;
        nextInteractionId = interactionTree->getMaxId() + 1;

        loadInitialData();
        buildSearchIndex();
    }

    ~ServiceController() {
        delete userTree;
        delete filmTree;
        delete logTree;
        delete genreTree;
        delete listTree;
        delete interactionTree;
        delete searchTrie;
    }
    
    void setCurrentUser(int userId, bool loggedIn, bool isAdmin) {
        currentUserId = userId;
        isLoggedIn = loggedIn;
        currentUserIsAdmin = isAdmin;
    }

    // Authentication
    string loginUser(const string& username, const string& password) {
        vector<User> allUsers = userTree->getAllRecords();
        
        for (const auto& user : allUsers) {
            if (string(user.username) == username && string(user.password_hash) == password) {
                currentUserId = user.user_id;
                isLoggedIn = true;
                currentUserIsAdmin = user.isAdmin;
                
                ostringstream token;
                token << user.user_id << ":" << user.username << ":" << (user.isAdmin ? "1" : "0");
                
                ostringstream json;
                json << "{\"status\":\"success\",\"token\":\"" << token.str() << "\"}";
                return json.str();
            }
        }
        
        return "{\"status\":\"error\",\"error\":\"Invalid credentials\"}";
    }

    string registerUser(const string& username, const string& email, const string& password, const string& bio) {
        vector<User> allUsers = userTree->getAllRecords();
        
        for (const auto& user : allUsers) {
            if (string(user.username) == username) {
                return "{\"status\":\"error\",\"message\":\"Username already exists\"}";
            }
        }
        
        User newUser(nextUserId++, username.c_str(), email.c_str(), password.c_str(), bio.c_str(), false);
        userTree->insert(newUser);
        
        ostringstream json;
        json << "{\"status\":\"success\",\"user_id\":" << newUser.user_id << "}";
        return json.str();
    }

    // Films
    string getAllFilms() {
        vector<Film> films = filmTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"films\":[";
        
        for (size_t i = 0; i < films.size(); i++) {
            if (i > 0) json << ",";
            json << "{\"film_id\":" << films[i].film_id
                 << ",\"tmdb_id\":" << films[i].tmdb_id
                 << ",\"title\":\"" << escapeJson(films[i].title) << "\""
                 << ",\"year\":" << films[i].release_year
                 << ",\"runtime\":" << films[i].runtime
                 << ",\"cast_summary\":\"" << escapeJson(films[i].cast_summary) << "\""
                 << ",\"director\":\"" << escapeJson(films[i].director) << "\""
                 << ",\"poster_path\":\"" << escapeJson(films[i].poster_path) << "\""
                 << ",\"backdrop_path\":\"" << escapeJson(films[i].backdrop_path) << "\""
                 << ",\"tagline\":\"" << escapeJson(films[i].tagline) << "\""
                 << ",\"vote_average\":" << fixed << setprecision(1) << films[i].vote_average
                 << ",\"genre_ids\":[" << films[i].genre_ids[0] << "," 
                 << films[i].genre_ids[1] << "," << films[i].genre_ids[2] << "]}";
        }
        
        json << "]}";
        return json.str();
    }

    string getFilmById(int filmId) {
        Film film;
        if (filmTree->search(filmId, film)) {
            // Check interactions
            bool watched = false, liked = false, watchlisted = false;
            
            if (isLoggedIn) {
                vector<Log> logs = logTree->getAllRecords();
                for (const auto& log : logs) {
                    if (log.user_id == currentUserId && log.film_id == filmId) {
                        watched = true;
                        break;
                    }
                }
                
                vector<Interaction> interactions = interactionTree->getAllRecords();
                for (const auto& inter : interactions) {
                    if (inter.user_id == currentUserId && inter.film_id == filmId) {
                        if (inter.type == 1) liked = true;
                        if (inter.type == 2) watchlisted = true;
                    }
                }
            }
            
            ostringstream json;
            json << "{\"status\":\"success\",\"film\":{"
                 << "\"film_id\":" << film.film_id
                 << ",\"tmdb_id\":" << film.tmdb_id
                 << ",\"title\":\"" << escapeJson(film.title) << "\""
                 << ",\"year\":" << film.release_year
                 << ",\"runtime\":" << film.runtime
                 << ",\"cast_summary\":\"" << escapeJson(film.cast_summary) << "\""
                 << ",\"director\":\"" << escapeJson(film.director) << "\""
                 << ",\"poster_path\":\"" << escapeJson(film.poster_path) << "\""
                 << ",\"backdrop_path\":\"" << escapeJson(film.backdrop_path) << "\""
                 << ",\"tagline\":\"" << escapeJson(film.tagline) << "\""
                 << ",\"vote_average\":" << fixed << setprecision(1) << film.vote_average
                 << ",\"genre_ids\":[" << film.genre_ids[0] << "," 
                 << film.genre_ids[1] << "," << film.genre_ids[2] << "]"
                 << ",\"watched\":" << (watched ? "true" : "false")
                 << ",\"liked\":" << (liked ? "true" : "false")
                 << ",\"watchlisted\":" << (watchlisted ? "true" : "false")
                 << "}}";
            return json.str();
        }
        return "{\"status\":\"error\",\"message\":\"Film not found\"}";
    }

    string searchFilms(const string& query) {
        if (query.length() < 2) {
            return "{\"status\":\"error\",\"message\":\"Query too short\"}";
        }
        
        vector<int> filmIds = searchTrie->search(query);
        
        ostringstream json;
        json << "{\"status\":\"success\",\"films\":[";
        
        bool first = true;
        for (int filmId : filmIds) {
            Film film;
            if (filmTree->search(filmId, film)) {
                if (!first) json << ",";
                first = false;
                
                json << "{\"film_id\":" << film.film_id
                     << ",\"title\":\"" << escapeJson(film.title) << "\""
                     << ",\"year\":" << film.release_year
                     << ",\"director\":\"" << escapeJson(film.director) << "\""
                     << ",\"poster_path\":\"" << escapeJson(film.poster_path) << "\"}";
            }
        }
        
        json << "]}";
        return json.str();
    }

    // Logs
    string addLog(int filmId, float rating, const string& review) {
        if (!isLoggedIn) {
            return "{\"status\":\"error\",\"message\":\"Must be logged in\"}";
        }

        Log newLog(nextLogId++, currentUserId, filmId, rating, review.c_str());
        logTree->insert(newLog);
        
        ostringstream json;
        json << "{\"status\":\"success\",\"log_id\":" << newLog.log_id << "}";
        return json.str();
    }

    string getUserLogs(int userId) {
        vector<Log> allLogs = logTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"logs\":[";
        
        bool first = true;
        for (const auto& log : allLogs) {
            if (log.user_id == userId) {
                if (!first) json << ",";
                first = false;
                
                json << "{\"log_id\":" << log.log_id
                     << ",\"user_id\":" << log.user_id
                     << ",\"film_id\":" << log.film_id
                     << ",\"rating\":" << fixed << setprecision(1) << log.rating
                     << ",\"review_text\":\"" << escapeJson(log.review_preview) << "\""
                     << ",\"log_date\":" << log.watch_date << "}";
            }
        }
        
        json << "]}";
        return json.str();
    }

    string getRecentLogs(int limit = 10) {
        vector<Log> allLogs = logTree->getAllRecords();
        
        // Sort by watch_date descending
        sort(allLogs.begin(), allLogs.end(), [](const Log& a, const Log& b) {
            return a.watch_date > b.watch_date;
        });
        
        ostringstream json;
        json << "{\"status\":\"success\",\"logs\":[";
        
        int count = 0;
        for (const auto& log : allLogs) {
            if (count >= limit) break;
            
            User user;
            Film film;
            if (userTree->search(log.user_id, user) && filmTree->search(log.film_id, film)) {
                if (count > 0) json << ",";
                json << "{\"username\":\"" << escapeJson(user.username) << "\""
                     << ",\"film_title\":\"" << escapeJson(film.title) << "\""
                     << ",\"rating\":" << fixed << setprecision(1) << log.rating
                     << ",\"date\":" << log.watch_date << "}";
                count++;
            }
        }
        
        json << "]}";
        return json.str();
    }

    // Interactions
    string toggleInteraction(int filmId, int type) {
        if (!isLoggedIn) {
            return "{\"status\":\"error\",\"message\":\"Must be logged in\"}";
        }

        vector<Interaction> interactions = interactionTree->getAllRecords();
        
        // Check if exists
        for (const auto& inter : interactions) {
            if (inter.user_id == currentUserId && inter.film_id == filmId && inter.type == type) {
                // Remove
                interactionTree->deleteRecord(inter.interaction_id);
                return "{\"status\":\"success\",\"action\":\"removed\"}";
            }
        }
        
        // Add
        Interaction newInteraction(nextInteractionId++, currentUserId, filmId, type);
        interactionTree->insert(newInteraction);
        
        return "{\"status\":\"success\",\"action\":\"added\"}";
    }

    string getUserWatchlist(int userId) {
        vector<Interaction> interactions = interactionTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"films\":[";
        
        bool first = true;
        for (const auto& inter : interactions) {
            if (inter.user_id == userId && inter.type == 2) {
                Film film;
                if (filmTree->search(inter.film_id, film)) {
                    if (!first) json << ",";
                    first = false;
                    
                    json << "{\"film_id\":" << film.film_id
                         << ",\"title\":\"" << escapeJson(film.title) << "\""
                         << ",\"year\":" << film.release_year
                         << ",\"poster_path\":\"" << escapeJson(film.poster_path) << "\""
                         << ",\"director\":\"" << escapeJson(film.director) << "\"}";
                }
            }
        }
        
        json << "]}";
        return json.str();
    }

    string getUserFavorites(int userId) {
        vector<Interaction> interactions = interactionTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"films\":[";
        
        bool first = true;
        int count = 0;
        for (const auto& inter : interactions) {
            if (inter.user_id == userId && inter.type == 1 && count < 4) {
                Film film;
                if (filmTree->search(inter.film_id, film)) {
                    if (!first) json << ",";
                    first = false;
                    
                    json << "{\"film_id\":" << film.film_id
                         << ",\"title\":\"" << escapeJson(film.title) << "\""
                         << ",\"year\":" << film.release_year
                         << ",\"vote_average\":" << fixed << setprecision(1) << film.vote_average
                         << ",\"poster_path\":\"" << escapeJson(film.poster_path) << "\"}";
                    count++;
                }
            }
        }
        
        json << "]}";
        return json.str();
    }

    // User Profile
    string getUserProfile(int userId) {
        User user;
        if (!userTree->search(userId, user)) {
            return "{\"status\":\"error\",\"message\":\"User not found\"}";
        }
        
        // Count stats
        vector<Log> logs = logTree->getAllRecords();
        int totalFilms = 0;
        int thisYear = 0;
        time_t now = time(nullptr);
        struct tm* tm_now = localtime(&now);
        int currentYear = tm_now->tm_year + 1900;
        
        for (const auto& log : logs) {
            if (log.user_id == userId) {
                totalFilms++;
                struct tm* tm_log = localtime(&log.watch_date);
                if (tm_log->tm_year + 1900 == currentYear) {
                    thisYear++;
                }
            }
        }
        
        vector<Interaction> interactions = interactionTree->getAllRecords();
        int watchlistCount = 0;
        for (const auto& inter : interactions) {
            if (inter.user_id == userId && inter.type == 2) {
                watchlistCount++;
            }
        }
        
        ostringstream json;
        json << "{\"status\":\"success\",\"profile\":{"
             << "\"user_id\":" << user.user_id
             << ",\"username\":\"" << escapeJson(user.username) << "\""
             << ",\"bio\":\"" << escapeJson(user.bio) << "\""
             << ",\"avatar_id\":" << user.avatar_id
             << ",\"total_films\":" << totalFilms
             << ",\"this_year\":" << thisYear
             << ",\"watchlist_count\":" << watchlistCount
             << "}}";
        
        return json.str();
    }

    // Home data
    string getHomeData() {
        vector<Film> films = filmTree->getAllRecords();
        
        // Get hero film (first high-rated one)
        Film heroFilm;
        bool foundHero = false;
        for (const auto& film : films) {
            if (film.vote_average >= 8.0 && strlen(film.backdrop_path) > 0) {
                heroFilm = film;
                foundHero = true;
                break;
            }
        }
        
        if (!foundHero && !films.empty()) {
            heroFilm = films[0];
        }
        
        ostringstream json;
        json << "{\"status\":\"success\",\"hero_movie\":{"
             << "\"film_id\":" << heroFilm.film_id
             << ",\"title\":\"" << escapeJson(heroFilm.title) << "\""
             << ",\"year\":" << heroFilm.release_year
             << ",\"director\":\"" << escapeJson(heroFilm.director) << "\""
             << ",\"poster_path\":\"" << escapeJson(heroFilm.poster_path) << "\""
             << ",\"backdrop_path\":\"" << escapeJson(heroFilm.backdrop_path) << "\""
             << ",\"tagline\":\"" << escapeJson(heroFilm.tagline) << "\""
             << ",\"vote_average\":" << fixed << setprecision(1) << heroFilm.vote_average
             << "},\"popular\":[";
        
        // Get 8 popular films
        for (int i = 0; i < min(8, (int)films.size()); i++) {
            if (i > 0) json << ",";
            json << "{\"film_id\":" << films[i].film_id
                 << ",\"title\":\"" << escapeJson(films[i].title) << "\""
                 << ",\"poster_path\":\"" << escapeJson(films[i].poster_path) << "\"}";
        }
        
        json << "],\"recent_logs\":[";
        
        // Get recent logs
        vector<Log> allLogs = logTree->getAllRecords();
        sort(allLogs.begin(), allLogs.end(), [](const Log& a, const Log& b) {
            return a.watch_date > b.watch_date;
        });
        
        int logCount = 0;
        for (const auto& log : allLogs) {
            if (logCount >= 5) break;
            
            User user;
            Film film;
            if (userTree->search(log.user_id, user) && filmTree->search(log.film_id, film)) {
                if (logCount > 0) json << ",";
                json << "{\"username\":\"" << escapeJson(user.username) << "\""
                     << ",\"film_title\":\"" << escapeJson(film.title) << "\""
                     << ",\"rating\":" << fixed << setprecision(1) << log.rating
                     << ",\"date\":" << log.watch_date << "}";
                logCount++;
            }
        }
        
        json << "]}";
        
        return json.str();
    }

    string getAllGenres() {
        vector<Genre> genres = genreTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"genres\":[";
        
        for (size_t i = 0; i < genres.size(); i++) {
            if (i > 0) json << ",";
            json << "{\"genre_id\":" << genres[i].genre_id
                 << ",\"name\":\"" << escapeJson(genres[i].name) << "\"}";
        }
        
        json << "]}";
        return json.str();
    }

    bool isCurrentUserAdmin() {
        return currentUserIsAdmin;
    }

private:
    void loadInitialData() {
        vector<User> existingUsers = userTree->getAllRecords();
        
        if (existingUsers.empty()) {
            cout << "Database is empty. Loading initial data from JSON files..." << endl;
            
            ifstream userCheck("data/users.json");
            if (userCheck.good()) {
                userCheck.close();
                vector<User> users = JSONLoader::loadUsers("data/users.json");
                cout << "Loading " << users.size() << " users..." << endl;
                for (const auto& user : users) {
                    userTree->insert(user);
                }
                nextUserId = userTree->getMaxId() + 1;
            }
            
            ifstream filmCheck("data/films.json");
            if (filmCheck.good()) {
                filmCheck.close();
                vector<Film> films = JSONLoader::loadFilms("data/films.json");
                cout << "Loading " << films.size() << " films..." << endl;
                for (const auto& film : films) {
                    filmTree->insert(film);
                }
                nextFilmId = filmTree->getMaxId() + 1;
            }
            
            ifstream genreCheck("data/genres.json");
            if (genreCheck.good()) {
                genreCheck.close();
                vector<Genre> genres = JSONLoader::loadGenres("data/genres.json");
                cout << "Loading " << genres.size() << " genres..." << endl;
                for (const auto& genre : genres) {
                    genreTree->insert(genre);
                }
                nextGenreId = genreTree->getMaxId() + 1;
            }
            
            ifstream logCheck("data/logs.json");
            if (logCheck.good()) {
                logCheck.close();
                vector<Log> logs = JSONLoader::loadLogs("data/logs.json");
                cout << "Loading " << logs.size() << " logs..." << endl;
                for (const auto& log : logs) {
                    logTree->insert(log);
                }
                nextLogId = logTree->getMaxId() + 1;
            }
            
            cout << "Initial data loaded successfully!" << endl;
        } else {
            cout << "Database already contains data. Skipping initial load." << endl;
        }
    }
    
    void buildSearchIndex() {
        vector<Film> films = filmTree->getAllRecords();
        cout << "Building search index with " << films.size() << " films..." << endl;
        
        for (const auto& film : films) {
            searchTrie->insert(film.title, film.film_id);
        }
        
        cout << "Search index built successfully!" << endl;
    }
};
