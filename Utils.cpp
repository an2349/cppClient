#include "Utils.h"
#include <sstream>
#include <random>

namespace Utils {

    vector<string> split(const string &s, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(s);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    string trim(const string &s) {
        size_t start = s.find_first_not_of(" \t\n\r");
        size_t end = s.find_last_not_of(" \t\n\r");
        if (start == string::npos || end == string::npos) return "";
        return s.substr(start, end - start + 1);
    }

    string generateBoundary() {
        const char alphanum[] =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        string boundary = "----WebKitFormBoundary";
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
        for (int i = 0; i < 16; ++i) {
            boundary += alphanum[dis(gen)];
        }
        return boundary;
    }

}
