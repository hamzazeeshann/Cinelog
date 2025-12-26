#pragma once

#include "../ds/BTree.h"
#include "../models/User.h"
#include "../models/Film.h"
#include "../models/Log.h"
#include "../models/Genre.h"
#include "../models/List.h"
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
    
    int nextUserId;
    int nextFilmId;
    int nextLogId;
    int nextGenreId;
    int nextListId;

    // Simple session management
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
        
        nextUserId = userTree->getMaxId() + 1;
        nextFilmId = filmTree->getMaxId() + 1;
        nextLogId = logTree->getMaxId() + 1;
        nextGenreId = genreTree->getMaxId() + 1;
        nextListId = listTree->getMaxId() + 1;

        // Load data from JSON if database is empty
        loadInitialData();
    }

    ~ServiceController() {
        delete userTree;
        delete filmTree;
        delete logTree;
        delete genreTree;
        delete listTree;
    }

    // Authentication methods
    string loginUser(const string& username, const string& password) {
        vector<User> allUsers = userTree->getAllRecords();
        
        for (const auto& user : allUsers) {
            if (string(user.username) == username && string(user.password_hash) == password) {
                currentUserId = user.user_id;
                isLoggedIn = true;
                currentUserIsAdmin = user.isAdmin;
                
                string role = user.isAdmin ? "admin" : "user";
                ostringstream json;
                json << "{\"status\":\"success\",\"role\":\"" << role 
                     << "\",\"user_id\":" << user.user_id 
                     << ",\"username\":\"" << escapeJson(user.username) << "\"}";
                return json.str();
            }
        }
        
        return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
    }

    string registerUser(const string& username, const string& email, const string& password, const string& bio) {
        vector<User> allUsers = userTree->getAllRecords();
        
        // Check if username exists
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

    string logoutUser() {
        currentUserId = 0;
        isLoggedIn = false;
        currentUserIsAdmin = false;
        return "{\"status\":\"success\",\"message\":\"Logged out\"}";
    }

    bool isUserLoggedIn() const {
        return isLoggedIn;
    }

    bool isCurrentUserAdmin() const {
        return currentUserIsAdmin;
    }

    // Film methods
    string getAllFilms() {
        vector<Film> films = filmTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"films\":[";
        
        for (size_t i = 0; i < films.size(); i++) {
            if (i > 0) json << ",";
            json << "{\"film_id\":" << films[i].film_id
                 << ",\"tmdb_id\":" << films[i].tmdb_id
                 << ",\"title\":\"" << escapeJson(films[i].title) << "\""
                 << ",\"release_year\":" << films[i].release_year
                 << ",\"runtime\":" << films[i].runtime
                 << ",\"cast\":\"" << escapeJson(films[i].cast_summary) << "\""
                 << ",\"director\":\"" << escapeJson(films[i].director) << "\""
                 << ",\"genre_ids\":[" << films[i].genre_ids[0] << "," 
                 << films[i].genre_ids[1] << "," << films[i].genre_ids[2] << "]}";
        }
        
        json << "]}";
        return json.str();
    }

    string getFilmById(int filmId) {
        Film film;
        if (filmTree->search(filmId, film)) {
            ostringstream json;
            json << "{\"status\":\"success\",\"film\":{"
                 << "\"film_id\":" << film.film_id
                 << ",\"tmdb_id\":" << film.tmdb_id
                 << ",\"title\":\"" << escapeJson(film.title) << "\""
                 << ",\"release_year\":" << film.release_year
                 << ",\"runtime\":" << film.runtime
                 << ",\"cast\":\"" << escapeJson(film.cast_summary) << "\""
                 << ",\"director\":\"" << escapeJson(film.director) << "\""
                 << ",\"genre_ids\":[" << film.genre_ids[0] << "," 
                 << film.genre_ids[1] << "," << film.genre_ids[2] << "]}}";
            return json.str();
        }
        return "{\"status\":\"error\",\"message\":\"Film not found\"}";
    }

    string addFilm(const string& title, int tmdbId, int year, int runtime, 
                   const string& cast, const string& director, int g1, int g2, int g3) {
        if (!currentUserIsAdmin) {
            return "{\"status\":\"error\",\"message\":\"Unauthorized - Admin only\"}";
        }

        Film newFilm(nextFilmId++, tmdbId, title.c_str(), year, runtime, cast.c_str(), director.c_str());
        newFilm.genre_ids[0] = g1;
        newFilm.genre_ids[1] = g2;
        newFilm.genre_ids[2] = g3;
        
        filmTree->insert(newFilm);
        
        ostringstream json;
        json << "{\"status\":\"success\",\"film_id\":" << newFilm.film_id << "}";
        return json.str();
    }

    string deleteFilm(int filmId) {
        if (!currentUserIsAdmin) {
            return "{\"status\":\"error\",\"message\":\"Unauthorized - Admin only\"}";
        }

        if (filmTree->deleteRecord(filmId)) {
            return "{\"status\":\"success\",\"message\":\"Film deleted\"}";
        }
        return "{\"status\":\"error\",\"message\":\"Film not found\"}";
    }

    // Log methods
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
                     << ",\"review\":\"" << escapeJson(log.review_preview) << "\""
                     << ",\"watch_date\":" << log.watch_date << "}";
            }
        }
        
        json << "]}";
        return json.str();
    }

    string deleteLog(int logId) {
        Log log;
        if (logTree->search(logId, log)) {
            if (log.user_id != currentUserId && !currentUserIsAdmin) {
                return "{\"status\":\"error\",\"message\":\"Unauthorized\"}";
            }
            
            if (logTree->deleteRecord(logId)) {
                return "{\"status\":\"success\",\"message\":\"Log deleted\"}";
            }
        }
        return "{\"status\":\"error\",\"message\":\"Log not found\"}";
    }

    // User management (Admin)
    string getAllUsers() {
        if (!currentUserIsAdmin) {
            return "{\"status\":\"error\",\"message\":\"Unauthorized - Admin only\"}";
        }

        vector<User> users = userTree->getAllRecords();
        
        ostringstream json;
        json << "{\"status\":\"success\",\"users\":[";
        
        for (size_t i = 0; i < users.size(); i++) {
            if (i > 0) json << ",";
            json << "{\"user_id\":" << users[i].user_id
                 << ",\"username\":\"" << escapeJson(users[i].username) << "\""
                 << ",\"email\":\"" << escapeJson(users[i].email) << "\""
                 << ",\"bio\":\"" << escapeJson(users[i].bio) << "\""
                 << ",\"join_date\":" << users[i].join_date
                 << ",\"isAdmin\":" << (users[i].isAdmin ? "true" : "false") << "}";
        }
        
        json << "]}";
        return json.str();
    }

    string deleteUser(int userId) {
        if (!currentUserIsAdmin) {
            return "{\"status\":\"error\",\"message\":\"Unauthorized - Admin only\"}";
        }

        if (userId == 1) {
            return "{\"status\":\"error\",\"message\":\"Cannot delete admin user\"}";
        }

        if (userTree->deleteRecord(userId)) {
            return "{\"status\":\"success\",\"message\":\"User deleted\"}";
        }
        return "{\"status\":\"error\",\"message\":\"User not found\"}";
    }

    // Genre methods
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

private:
    void loadInitialData() {
        // Check if database is empty
        vector<User> existingUsers = userTree->getAllRecords();
        
        if (existingUsers.empty()) {
            cout << "Database is empty. Loading initial data from JSON files..." << endl;
            
            // Load users
            ifstream userCheck("data/users.json");
            if (userCheck.good()) {
                userCheck.close();
                vector<User> users = JSONLoader::loadUsers("data/users.json");
                cout << "Loading " << users.size() << " users..." << endl;
                for (const auto& user : users) {
                    userTree->insert(user);
                }
                nextUserId = userTree->getMaxId() + 1;
            } else {
                // Create default admin if no JSON file
                cout << "No users.json found. Creating default admin..." << endl;
                User adminUser(1, "admin", "admin@cinelog.com", "admin123", "System Administrator", true);
                userTree->insert(adminUser);
                nextUserId = 2;
            }
            
            // Load films
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
            
            // Load genres
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
            
            // Load logs
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
};
