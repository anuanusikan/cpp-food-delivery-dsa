#include "AuthService.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

string AuthService::registerUser(string username, string password) {
    if (username.empty() || password.empty()) {
        return "invalid";
    }

    if (users.containsKey(username)) {
        cout << "[Auth] Register failed - user already exists: " << username << "\n";
        return "exists";
    }

    users.insert(username, password);
    cout << "[Auth] New user registered: " << username << "\n";
    return "registered";
}

string AuthService::login(string username, string password) {
    cout << "[Auth] Login attempt: " << username << "\n";

    if (users.containsKey(username) && users.getValue(username) == password) {
        // generate a simple session id using time + random
        string session = "sess_" + to_string(time(nullptr)) + "_" + to_string(rand() % 1000000);
        sessions.insert(session, username);
        cout << "[Auth] Login successful for: " << username << "\n";
        return session;
    }

    cout << "[Auth] Login failed for: " << username << "\n";
    return "";
}

bool AuthService::checkSession(string sessionId) {
    return sessions.containsKey(sessionId);
}

string AuthService::getUsername(string sessionId) {
    if (!sessions.containsKey(sessionId)) {
        return "";
    }
    return sessions.getValue(sessionId);
}

void AuthService::logout(string sessionId) {
    string user = sessions.getValue(sessionId);
    sessions.removeKey(sessionId);
    cout << "[Auth] User logged out: " << user << "\n";
}