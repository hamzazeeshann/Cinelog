#pragma once

#include "../service/ServiceController.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class HTTPServer {
private:
    ServiceController* controller;
    SOCKET serverSocket;
    int port;
    bool running;

    struct HTTPRequest {
        string method;
        string path;
        map<string, string> headers;
        string body;
    };

    HTTPRequest parseRequest(const string& rawRequest) {
        HTTPRequest req;
        istringstream stream(rawRequest);
        string line;
        
        // Parse request line
        if (getline(stream, line)) {
            istringstream lineStream(line);
            lineStream >> req.method >> req.path;
        }
        
        // Parse headers
        while (getline(stream, line) && line != "\r" && !line.empty()) {
            size_t colonPos = line.find(':');
            if (colonPos != string::npos) {
                string key = line.substr(0, colonPos);
                string value = line.substr(colonPos + 2);
                if (!value.empty() && value.back() == '\r') {
                    value.pop_back();
                }
                req.headers[key] = value;
            }
        }
        
        // Parse body
        string bodyContent;
        while (getline(stream, line)) {
            bodyContent += line;
        }
        req.body = bodyContent;
        
        return req;
    }

    string parseJsonField(const string& json, const string& field) {
        size_t pos = json.find("\"" + field + "\"");
        if (pos == string::npos) return "";
        
        pos = json.find(":", pos);
        if (pos == string::npos) return "";
        
        pos = json.find("\"", pos);
        if (pos == string::npos) return "";
        
        size_t endPos = json.find("\"", pos + 1);
        if (endPos == string::npos) return "";
        
        return json.substr(pos + 1, endPos - pos - 1);
    }

    int parseJsonInt(const string& json, const string& field) {
        size_t pos = json.find("\"" + field + "\"");
        if (pos == string::npos) return 0;
        
        pos = json.find(":", pos);
        if (pos == string::npos) return 0;
        
        // Skip whitespace
        pos++;
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) {
            pos++;
        }
        
        string numStr;
        while (pos < json.length() && (isdigit(json[pos]) || json[pos] == '-' || json[pos] == '.')) {
            numStr += json[pos];
            pos++;
        }
        
        return numStr.empty() ? 0 : stoi(numStr);
    }

    float parseJsonFloat(const string& json, const string& field) {
        size_t pos = json.find("\"" + field + "\"");
        if (pos == string::npos) return 0.0f;
        
        pos = json.find(":", pos);
        if (pos == string::npos) return 0.0f;
        
        pos++;
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) {
            pos++;
        }
        
        string numStr;
        while (pos < json.length() && (isdigit(json[pos]) || json[pos] == '-' || json[pos] == '.')) {
            numStr += json[pos];
            pos++;
        }
        
        return numStr.empty() ? 0.0f : stof(numStr);
    }

    string buildHTTPResponse(int statusCode, const string& statusText, const string& body, const string& contentType = "application/json") {
        ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        response << "Content-Type: " << contentType << "\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;
        return response.str();
    }

    string handleRequest(const HTTPRequest& req) {
        // Handle CORS preflight
        if (req.method == "OPTIONS") {
            return buildHTTPResponse(200, "OK", "");
        }

        // Route handling
        if (req.path == "/api/login" && req.method == "POST") {
            string username = parseJsonField(req.body, "username");
            string password = parseJsonField(req.body, "password");
            string result = controller->loginUser(username, password);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/register" && req.method == "POST") {
            string username = parseJsonField(req.body, "username");
            string email = parseJsonField(req.body, "email");
            string password = parseJsonField(req.body, "password");
            string bio = parseJsonField(req.body, "bio");
            string result = controller->registerUser(username, email, password, bio);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/logout" && req.method == "POST") {
            string result = controller->logoutUser();
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/films" && req.method == "GET") {
            string result = controller->getAllFilms();
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/films/") == 0 && req.method == "GET") {
            int filmId = stoi(req.path.substr(11));
            string result = controller->getFilmById(filmId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/admin/add_film" && req.method == "POST") {
            if (!controller->isCurrentUserAdmin()) {
                return buildHTTPResponse(403, "Forbidden", "{\"status\":\"error\",\"message\":\"Admin only\"}");
            }
            
            string title = parseJsonField(req.body, "title");
            int tmdbId = parseJsonInt(req.body, "tmdb_id");
            int year = parseJsonInt(req.body, "release_year");
            int runtime = parseJsonInt(req.body, "runtime");
            string cast = parseJsonField(req.body, "cast");
            string director = parseJsonField(req.body, "director");
            int g1 = parseJsonInt(req.body, "genre_id_1");
            int g2 = parseJsonInt(req.body, "genre_id_2");
            int g3 = parseJsonInt(req.body, "genre_id_3");
            
            string result = controller->addFilm(title, tmdbId, year, runtime, cast, director, g1, g2, g3);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/admin/delete_film" && req.method == "POST") {
            if (!controller->isCurrentUserAdmin()) {
                return buildHTTPResponse(403, "Forbidden", "{\"status\":\"error\",\"message\":\"Admin only\"}");
            }
            
            int filmId = parseJsonInt(req.body, "film_id");
            string result = controller->deleteFilm(filmId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/log_entry" && req.method == "POST") {
            int filmId = parseJsonInt(req.body, "film_id");
            float rating = parseJsonFloat(req.body, "rating");
            string review = parseJsonField(req.body, "review");
            
            string result = controller->addLog(filmId, rating, review);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/logs/user/") == 0 && req.method == "GET") {
            int userId = stoi(req.path.substr(15));
            string result = controller->getUserLogs(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/admin/users" && req.method == "GET") {
            if (!controller->isCurrentUserAdmin()) {
                return buildHTTPResponse(403, "Forbidden", "{\"status\":\"error\",\"message\":\"Admin only\"}");
            }
            
            string result = controller->getAllUsers();
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/admin/delete_user" && req.method == "POST") {
            if (!controller->isCurrentUserAdmin()) {
                return buildHTTPResponse(403, "Forbidden", "{\"status\":\"error\",\"message\":\"Admin only\"}");
            }
            
            int userId = parseJsonInt(req.body, "user_id");
            string result = controller->deleteUser(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/genres" && req.method == "GET") {
            string result = controller->getAllGenres();
            return buildHTTPResponse(200, "OK", result);
        }
        
        // 404 Not Found
        return buildHTTPResponse(404, "Not Found", "{\"status\":\"error\",\"message\":\"Endpoint not found\"}");
    }

public:
    HTTPServer(int p = 8080) : port(p), running(false), serverSocket(INVALID_SOCKET) {
        controller = new ServiceController();
    }

    ~HTTPServer() {
        stop();
        delete controller;
    }

    bool start() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            cerr << "WSAStartup failed: " << result << endl;
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed" << endl;
            WSACleanup();
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Bind failed" << endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "Listen failed" << endl;
            closesocket(serverSocket);
            WSACleanup();
            return false;
        }

        running = true;
        cout << "Server started on port " << port << endl;
        return true;
    }

    void run() {
        while (running) {
            SOCKET clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                if (running) {
                    cerr << "Accept failed" << endl;
                }
                continue;
            }

            char buffer[8192];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                string rawRequest(buffer);
                
                HTTPRequest req = parseRequest(rawRequest);
                cout << req.method << " " << req.path << endl;
                
                string response = handleRequest(req);
                send(clientSocket, response.c_str(), response.length(), 0);
            }

            closesocket(clientSocket);
        }
    }

    void stop() {
        running = false;
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        WSACleanup();
    }
};
