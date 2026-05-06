#include "AuthService.h"
#include <cstdlib>
#include <ctime>

string AuthService::registerUser(string username, string password) {
    if (username.empty() || password.empty()) {
        return "invalid";
    }

    if (users.containsKey(username)) {
        return "exists";
    }

    users.insert(username, password);
    return "registered";
}

string AuthService::login(string username, string password) {
    if (users.containsKey(username) && users.getValue(username) == password) {
        string session = "sess_" + to_string(time(nullptr)) + "_" + to_string(rand() % 1000000);

        sessions.insert(session, username);

        return session;
    }

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
    sessions.removeKey(sessionId);
}