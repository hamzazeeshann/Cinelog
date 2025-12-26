#pragma once

#include "../models/User.h"
#include "../models/Film.h"
#include "../models/Log.h"
#include "../models/Genre.h"
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

class JSONLoader {
private:
    static string extractStringValue(const string& json, const string& key) {
        size_t keyPos = json.find("\"" + key + "\"");
        if (keyPos == string::npos) return "";
        
        size_t colonPos = json.find(":", keyPos);
        if (colonPos == string::npos) return "";
        
        size_t quoteStart = json.find("\"", colonPos);
        if (quoteStart == string::npos) return "";
        
        size_t quoteEnd = json.find("\"", quoteStart + 1);
        if (quoteEnd == string::npos) return "";
        
        return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
    }
    
    static int extractIntValue(const string& json, const string& key) {
        size_t keyPos = json.find("\"" + key + "\"");
        if (keyPos == string::npos) return 0;
        
        size_t colonPos = json.find(":", keyPos);
        if (colonPos == string::npos) return 0;
        
        colonPos++;
        while (colonPos < json.length() && (json[colonPos] == ' ' || json[colonPos] == '\t')) {
            colonPos++;
        }
        
        string numStr;
        while (colonPos < json.length() && (isdigit(json[colonPos]) || json[colonPos] == '-')) {
            numStr += json[colonPos];
            colonPos++;
        }
        
        return numStr.empty() ? 0 : stoi(numStr);
    }
    
    static float extractFloatValue(const string& json, const string& key) {
        size_t keyPos = json.find("\"" + key + "\"");
        if (keyPos == string::npos) return 0.0f;
        
        size_t colonPos = json.find(":", keyPos);
        if (colonPos == string::npos) return 0.0f;
        
        colonPos++;
        while (colonPos < json.length() && (json[colonPos] == ' ' || json[colonPos] == '\t')) {
            colonPos++;
        }
        
        string numStr;
        while (colonPos < json.length() && (isdigit(json[colonPos]) || json[colonPos] == '-' || json[colonPos] == '.')) {
            numStr += json[colonPos];
            colonPos++;
        }
        
        return numStr.empty() ? 0.0f : stof(numStr);
    }
    
    static bool extractBoolValue(const string& json, const string& key) {
        size_t keyPos = json.find("\"" + key + "\"");
        if (keyPos == string::npos) return false;
        
        size_t colonPos = json.find(":", keyPos);
        if (colonPos == string::npos) return false;
        
        size_t truePos = json.find("true", colonPos);
        size_t falsePos = json.find("false", colonPos);
        
        if (truePos != string::npos && (falsePos == string::npos || truePos < falsePos)) {
            return true;
        }
        return false;
    }
    
    static vector<int> extractIntArray(const string& json, const string& key) {
        vector<int> result;
        size_t keyPos = json.find("\"" + key + "\"");
        if (keyPos == string::npos) return result;
        
        size_t arrayStart = json.find("[", keyPos);
        if (arrayStart == string::npos) return result;
        
        size_t arrayEnd = json.find("]", arrayStart);
        if (arrayEnd == string::npos) return result;
        
        string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        istringstream stream(arrayContent);
        string token;
        
        while (getline(stream, token, ',')) {
            // Remove whitespace
            token.erase(0, token.find_first_not_of(" \t\n\r"));
            token.erase(token.find_last_not_of(" \t\n\r") + 1);
            if (!token.empty()) {
                result.push_back(stoi(token));
            }
        }
        
        return result;
    }

public:
    static vector<User> loadUsers(const string& filename) {
        vector<User> users;
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Could not open " << filename << endl;
            return users;
        }
        
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        // Parse JSON array manually
        size_t pos = 0;
        while ((pos = content.find("{", pos)) != string::npos) {
            size_t endPos = content.find("}", pos);
            if (endPos == string::npos) break;
            
            string objStr = content.substr(pos, endPos - pos + 1);
            
            User user;
            user.user_id = extractIntValue(objStr, "user_id");
            
            string username = extractStringValue(objStr, "username");
            strncpy(user.username, username.c_str(), sizeof(user.username) - 1);
            
            string email = extractStringValue(objStr, "email");
            strncpy(user.email, email.c_str(), sizeof(user.email) - 1);
            
            string password = extractStringValue(objStr, "password_hash");
            strncpy(user.password_hash, password.c_str(), sizeof(user.password_hash) - 1);
            
            string bio = extractStringValue(objStr, "bio");
            strncpy(user.bio, bio.c_str(), sizeof(user.bio) - 1);
            
            user.join_date = extractIntValue(objStr, "join_date");
            user.isAdmin = extractBoolValue(objStr, "isAdmin");
            
            users.push_back(user);
            pos = endPos + 1;
        }
        
        return users;
    }
    
    static vector<Film> loadFilms(const string& filename) {
        vector<Film> films;
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Could not open " << filename << endl;
            return films;
        }
        
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        size_t pos = 0;
        while ((pos = content.find("{", pos)) != string::npos) {
            size_t endPos = content.find("}", pos);
            if (endPos == string::npos) break;
            
            string objStr = content.substr(pos, endPos - pos + 1);
            
            Film film;
            film.film_id = extractIntValue(objStr, "film_id");
            film.tmdb_id = extractIntValue(objStr, "tmdb_id");
            
            string title = extractStringValue(objStr, "title");
            strncpy(film.title, title.c_str(), sizeof(film.title) - 1);
            
            film.release_year = extractIntValue(objStr, "year");
            film.runtime = extractIntValue(objStr, "runtime");
            
            string cast = extractStringValue(objStr, "cast_summary");
            strncpy(film.cast_summary, cast.c_str(), sizeof(film.cast_summary) - 1);
            
            string director = extractStringValue(objStr, "director");
            strncpy(film.director, director.c_str(), sizeof(film.director) - 1);
            
            string poster = extractStringValue(objStr, "poster_path");
            strncpy(film.poster_path, poster.c_str(), sizeof(film.poster_path) - 1);
            
            string backdrop = extractStringValue(objStr, "backdrop_path");
            strncpy(film.backdrop_path, backdrop.c_str(), sizeof(film.backdrop_path) - 1);
            
            string tagline = extractStringValue(objStr, "tagline");
            strncpy(film.tagline, tagline.c_str(), sizeof(film.tagline) - 1);
            
            film.vote_average = extractFloatValue(objStr, "vote_average");
            
            vector<int> genres = extractIntArray(objStr, "genre_ids");
            for (size_t i = 0; i < 3 && i < genres.size(); i++) {
                film.genre_ids[i] = genres[i];
            }
            
            films.push_back(film);
            pos = endPos + 1;
        }
        
        return films;
    }
    
    static vector<Log> loadLogs(const string& filename) {
        vector<Log> logs;
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Could not open " << filename << endl;
            return logs;
        }
        
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        size_t pos = 0;
        while ((pos = content.find("{", pos)) != string::npos) {
            size_t endPos = content.find("}", pos);
            if (endPos == string::npos) break;
            
            string objStr = content.substr(pos, endPos - pos + 1);
            
            Log log;
            log.log_id = extractIntValue(objStr, "log_id");
            log.user_id = extractIntValue(objStr, "user_id");
            log.film_id = extractIntValue(objStr, "film_id");
            log.rating = extractFloatValue(objStr, "rating");
            
            string review = extractStringValue(objStr, "review");
            strncpy(log.review_preview, review.c_str(), sizeof(log.review_preview) - 1);
            
            log.watch_date = extractIntValue(objStr, "watch_date");
            
            logs.push_back(log);
            pos = endPos + 1;
        }
        
        return logs;
    }
    
    static vector<Genre> loadGenres(const string& filename) {
        vector<Genre> genres;
        ifstream file(filename);
        
        if (!file.is_open()) {
            cerr << "Could not open " << filename << endl;
            return genres;
        }
        
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        
        size_t pos = 0;
        while ((pos = content.find("{", pos)) != string::npos) {
            size_t endPos = content.find("}", pos);
            if (endPos == string::npos) break;
            
            string objStr = content.substr(pos, endPos - pos + 1);
            
            Genre genre;
            genre.genre_id = extractIntValue(objStr, "genre_id");
            
            string name = extractStringValue(objStr, "name");
            strncpy(genre.name, name.c_str(), sizeof(genre.name) - 1);
            
            genres.push_back(genre);
            pos = endPos + 1;
        }
        
        return genres;
    }
};
