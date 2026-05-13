#pragma once
#include <string>
#include <cstdlib>
#include <ctime>
#include "../ds/HashMap.h"

using namespace std;

class AuthService {
private:
    HashMap users;     // username -> password
    HashMap sessions;  // sessionId -> username

public:
    string registerUser(string username, string password) {
        if (username.empty() || password.empty()) {
            return "invalid";
        }

        if (users.containsKey(username)) {
            return "exists";
        }

        users.insert(username, password);
        return "registered";
    }

    string login(string username, string password) {
        if (users.containsKey(username) && users.getValue(username) == password) {
            string session = "sess_" + to_string(time(nullptr)) + "_" + to_string(rand() % 1000000);

            sessions.insert(session, username);

            return session;
        }

        return "";
    }

    bool checkSession(string sessionId) {
        return sessions.containsKey(sessionId);
    }

    string getUsername(string sessionId) {
        if (!sessions.containsKey(sessionId)) {
            return "";
        }

        return sessions.getValue(sessionId);
    }

    void logout(string sessionId) {
        sessions.removeKey(sessionId);
    }
};