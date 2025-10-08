#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

namespace Utils {
    vector<string> split(const string &s, char delimiter);
    string trim(const string &s);
    string generateBoundary();
}

#endif
