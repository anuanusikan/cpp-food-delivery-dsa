#pragma once
#include <string>
#include "../ds/HashMap.h"

using namespace std;

class AuthService {
private:
    HashMap users;
    HashMap sessions;

public:
    string registerUser(string username, string password);
    string login(string username, string password);
    bool checkSession(string sessionId);
    string getUsername(string sessionId);
    void logout(string sessionId);
};