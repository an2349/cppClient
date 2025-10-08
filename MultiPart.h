//
// Created by an on 10/7/25.
//

#ifndef CLIENT_MULTIPART_H
#define CLIENT_MULTIPART_H
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

class MultiPartModel {
public:
    MultiPartModel() {}
    int totalPart = 1;
    string name = "";
    int part = 1;
    string contentType = "";
    vector<uint8_t> value;
};


#endif //CLIENT_MULTIPART_H