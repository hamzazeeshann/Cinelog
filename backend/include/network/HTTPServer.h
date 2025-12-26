#pragma once

#include "../service/ServiceController.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct HTTPRequest {
    string method;
    string path;
    string body;
    string authToken;
};

class HTTPServer {
private:
    int port;
    SOCKET serverSocket;
    ServiceController* controller;
    bool running;

    HTTPRequest parseRequest(const string& rawRequest) {
        HTTPRequest req;
        istringstream stream(rawRequest);
        stream >> req.method >> req.path;
        
        // Extract Authorization header
        size_t authPos = rawRequest.find("Authorization:");
        if (authPos != string::npos) {
            size_t authStart = authPos + 14; // Skip "Authorization:"
            while (authStart < rawRequest.length() && rawRequest[authStart] == ' ') {
                authStart++;
            }
            if (authStart < rawRequest.length()) {
                size_t authEnd = rawRequest.find("\r\n", authStart);
                if (authEnd != string::npos) {
                    req.authToken = rawRequest.substr(authStart, authEnd - authStart);
                }
            }
        }
        
        size_t bodyStart = rawRequest.find("\r\n\r\n");
        if (bodyStart != string::npos) {
            req.body = rawRequest.substr(bodyStart + 4);
        }
        
        return req;
    }
    
    void setAuthFromToken(const string& token) {
        if (token.empty()) {
            controller->setCurrentUser(0, false, false);
            return;
        }
        
        // Token format: "userId:username:isAdmin"
        size_t firstColon = token.find(':');
        if (firstColon == string::npos) {
            controller->setCurrentUser(0, false, false);
            return;
        }
        
        size_t secondColon = token.find(':', firstColon + 1);
        if (secondColon == string::npos) {
            controller->setCurrentUser(0, false, false);
            return;
        }
        
        try {
            int userId = stoi(token.substr(0, firstColon));
            bool isAdmin = (token.substr(secondColon + 1) == "1");
            controller->setCurrentUser(userId, true, isAdmin);
        } catch (...) {
            controller->setCurrentUser(0, false, false);
        }
    }

    string parseJsonField(const string& json, const string& field) {
        size_t pos = json.find("\"" + field + "\"");
        if (pos == string::npos) return "";
        
        size_t valueStart = json.find(":", pos) + 1;
        while (json[valueStart] == ' ' || json[valueStart] == '\"') valueStart++;
        
        size_t valueEnd = valueStart;
        if (json[valueStart - 1] == '\"') {
            valueEnd = json.find("\"", valueStart);
        } else {
            valueEnd = json.find_first_of(",}", valueStart);
        }
        
        return json.substr(valueStart, valueEnd - valueStart);
    }

    int parseJsonInt(const string& json, const string& field) {
        string value = parseJsonField(json, field);
        try {
            return stoi(value);
        } catch (...) {
            return 0;
        }
    }

    float parseJsonFloat(const string& json, const string& field) {
        string value = parseJsonField(json, field);
        try {
            return stof(value);
        } catch (...) {
            return 0.0f;
        }
    }

    string buildHTTPResponse(int statusCode, const string& statusText, const string& body) {
        ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        response << "Content-Type: application/json\r\n";
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Access-Control-Allow-Origin: *\r\n";
        response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        response << "\r\n";
        response << body;
        return response.str();
    }

    string handleRequest(const HTTPRequest& req) {
        cout << req.method << " " << req.path << endl;

        if (req.method == "OPTIONS") {
            return buildHTTPResponse(200, "OK", "");
        }

        // Authentication endpoints
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
        
        // Film endpoints
        else if (req.path == "/api/films" && req.method == "GET") {
            string result = controller->getAllFilms();
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/film/") == 0 && req.method == "GET") {
            setAuthFromToken(req.authToken);
            int filmId = stoi(req.path.substr(10));
            string result = controller->getFilmById(filmId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/search?q=") == 0 && req.method == "GET") {
            size_t qPos = req.path.find("q=");
            if (qPos != string::npos) {
                string query = req.path.substr(qPos + 2);
                size_t pos = 0;
                while ((pos = query.find("%20")) != string::npos) {
                    query.replace(pos, 3, " ");
                }
                string result = controller->searchFilms(query);
                return buildHTTPResponse(200, "OK", result);
            }
            return buildHTTPResponse(400, "Bad Request", "{\"status\":\"error\",\"message\":\"Invalid query\"}");
        }
        
        // Log endpoints
        else if (req.path == "/api/logs" && req.method == "POST") {
            setAuthFromToken(req.authToken);
            int filmId = parseJsonInt(req.body, "film_id");
            float rating = parseJsonFloat(req.body, "rating");
            string review = parseJsonField(req.body, "review_text");
            
            string result = controller->addLog(filmId, rating, review);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/user/") == 0 && req.path.find("/logs") != string::npos && req.method == "GET") {
            size_t userStart = 10; // "/api/user/"
            size_t userEnd = req.path.find("/", userStart);
            int userId = stoi(req.path.substr(userStart, userEnd - userStart));
            string result = controller->getUserLogs(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path == "/api/logs/recent" && req.method == "GET") {
            string result = controller->getRecentLogs(10);
            return buildHTTPResponse(200, "OK", result);
        }
        
        // Interaction endpoints
        else if (req.path == "/api/interaction" && req.method == "POST") {
            setAuthFromToken(req.authToken);
            int filmId = parseJsonInt(req.body, "film_id");
            int type = parseJsonInt(req.body, "type");
            
            string result = controller->toggleInteraction(filmId, type);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/user/") == 0 && req.path.find("/watchlist") != string::npos && req.method == "GET") {
            size_t userStart = 10;
            size_t userEnd = req.path.find("/", userStart);
            int userId = stoi(req.path.substr(userStart, userEnd - userStart));
            string result = controller->getUserWatchlist(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/user/") == 0 && req.path.find("/favorites") != string::npos && req.method == "GET") {
            size_t userStart = 10;
            size_t userEnd = req.path.find("/", userStart);
            int userId = stoi(req.path.substr(userStart, userEnd - userStart));
            string result = controller->getUserFavorites(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        else if (req.path.find("/api/user/") == 0 && req.path.find("/profile") != string::npos && req.method == "GET") {
            size_t userStart = 10;
            size_t userEnd = req.path.find("/", userStart);
            int userId = stoi(req.path.substr(userStart, userEnd - userStart));
            string result = controller->getUserProfile(userId);
            return buildHTTPResponse(200, "OK", result);
        }
        
        // Home data
        else if (req.path == "/api/home_data" && req.method == "GET") {
            string result = controller->getHomeData();
            return buildHTTPResponse(200, "OK", result);
        }
        
        // Genres
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
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
        delete controller;
    }

    bool start() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed" << endl;
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

        cout << "Server started on port " << port << endl;
        running = true;
        return true;
    }

    void run() {
        while (running) {
            SOCKET clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                continue;
            }

            char buffer[4096];
            int bytesReceived = recv(clientSocket, buffer, 4096, 0);
            
            if (bytesReceived > 0) {
                string rawRequest(buffer, bytesReceived);
                HTTPRequest req = parseRequest(rawRequest);
                string response = handleRequest(req);
                
                send(clientSocket, response.c_str(), response.length(), 0);
            }

            closesocket(clientSocket);
        }
    }
};
