//
// Created by an on 10/7/25.
//
#ifndef FILESERVICE_H
#define FILESERVICE_H

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "MultiPart.h"
#include "HttpService.h"

#include <thread>
#include <future>

#include "Utils.h"

namespace std {
    class thread;
}

using namespace std;
namespace fs =  filesystem;

class FileService {
private:
    HttpService &http;

public:
    FileService(HttpService &httpService) : http(httpService) {
    }

    vector<MultiPartModel>* readFileChunks(const string &filepath);

    void pushFile(const string &filepath);

    void pushAll(const string &folderpath, const string &types);

    string escapeResponse(const string &response) {
        string escaped;
        for (char c: response) {
            if (isprint(c)) escaped += c;
            else if (c == '\r') escaped += "\\r";
            else if (c == '\n') escaped += "\\n\n";
            else {
                stringstream ss;
                ss << "\\x" << hex << setw(2) << setfill('0') << (unsigned int) (unsigned char) c;
                escaped += ss.str();
            }
        }
        return escaped;
    }
};


#endif
