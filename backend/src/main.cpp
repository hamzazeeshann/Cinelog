#include "../include/network/HTTPServer.h"
#include <iostream>
#include <direct.h>

using namespace std;

int main() {
    // Create data directory if it doesn't exist
    _mkdir("data");
    
    cout << "==================================" << endl;
    cout << "    CINELOG Backend Server" << endl;
    cout << "==================================" << endl;
    cout << endl;
    
    HTTPServer server(8080);
    
    if (!server.start()) {
        cerr << "Failed to start server!" << endl;
        return 1;
    }
    
    cout << "Server is running..." << endl;
    cout << "Press Ctrl+C to stop" << endl;
    cout << endl;
    
    server.run();
    
    return 0;
}
