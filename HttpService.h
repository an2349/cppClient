//
// Created by an on 10/7/25.
//

#ifndef CLIENT_HTTPSERVICE_H
#define CLIENT_HTTPSERVICE_H

// Fix cho MinGW: phai dinh nghia phien ban Windows truoc khi include
#ifdef _WIN32
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
#endif

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring>
#include "MultiPart.h"
#include "config.h"

// --- Phần xử lý đa nền tảng cho Socket ---
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    
    #define CLOSE_SOCKET closesocket
    #define IS_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
    // Dinh nghia ssize_t cho MinGW neu thieu
    #ifdef __MINGW32__
        #include <unistd.h>
    #endif
    #ifndef SSIZE_T
        typedef int ssize_t;
    #endif
#else
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
    #define IS_VALID_SOCKET(s) ((s) >= 0)
#endif
// -----------------------------------------

using namespace std;

class HttpService {
private:

public:
    HttpService() {
    }

    int sendRawRequest(vector<uint8_t> *requestData, string &response) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (!IS_VALID_SOCKET(sock)) {
            delete requestData;
            return -1;
        }

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            delete requestData;
            CLOSE_SOCKET(sock);
            return  -1;
        }

        size_t totalSent = 0;
        while (totalSent < requestData->size()) {
            int sent = send(sock, (const char*)(requestData->data() + totalSent), (int)(requestData->size() - totalSent), 0);
            if (sent < 0) {
                delete requestData;
                CLOSE_SOCKET(sock);
                return -1;
            }
            totalSent += sent;
        }

        char buffer[4096];
        response.clear();
        int bytes;
        while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes] = '\0';
            response += buffer;
            if (bytes < sizeof(buffer) - 1) break;
        }
        delete requestData;
        CLOSE_SOCKET(sock);
        return 0;
    }

    int sendRawRequest(const string &request, string &response) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (!IS_VALID_SOCKET(sock)) {
            return -1;
        }

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            CLOSE_SOCKET(sock);
            return -1;
        }

        send(sock, request.c_str(), (int)request.size(), 0);

        char buffer[4096];
        response.clear();
        int bytes;
        while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes] = '\0';
            response += buffer;
            if (bytes < sizeof(buffer) - 1) break;
        }

        CLOSE_SOCKET(sock);
        return 0;
    }

    int postDiemDanh(const string &masv, const string &mac, string &response) {
        string serverip = SERVER_IP;
        string body = "{\"MaSv\":\"" + masv + "\",\"Mac\":\"" + mac + "\"}";
        string req = "POST /maso HTTP/1.1\r\n";
        req += "Host: " + serverip + "\r\n";
        req += "Content-Type: application/json\r\n";
        req += "Content-Length: " + to_string(body.size()) + "\r\n";
        req += "Connection: close\r\n\r\n";
        req += body;
        cout << req << endl;
        return sendRawRequest(req, response);
    }


    int putFileChunk(const MultiPartModel &mp, const string &boundary, size_t totalSize, string &response) {
        stringstream ss;
        ss << "--" << boundary << "\r\n";
        ss << "Content-Disposition: form-data; name=\"file\"; filename=\"" << mp.name
                << "\"; part=\"" << to_string(mp.part) << "\"; total=\"" << to_string(mp.totalPart)
                << "\"; size=\"" << to_string(totalSize) << "\"\r\n";
        ss << "Content-Type: " << mp.contentType << "\r\n\r\n";

        string headerStr = ss.str();
        vector<uint8_t> header(headerStr.begin(), headerStr.end());

        const vector<uint8_t> &body = mp.value;

        string footerStr = "\r\n";
        vector<uint8_t> footer(footerStr.begin(), footerStr.end());

        vector<uint8_t> *payload = new vector<uint8_t>();
        payload->reserve(header.size() + body.size() + footer.size());
        payload->insert(payload->end(), header.begin(), header.end());
        payload->insert(payload->end(), body.begin(), body.end());
        payload->insert(payload->end(), footer.begin(), footer.end());

        stringstream req;
        req << "PUT /upload HTTP/1.1\r\n";
        req << "Host: " << SERVER_IP << "\r\n";
        req << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";
        req << "Content-Length: " << payload->size() << "\r\n";
        req << "Connection: close\r\n\r\n";

        string headerHttp = req.str();
        vector<uint8_t> *requestData = new vector<uint8_t>(headerHttp.begin(), headerHttp.end());
        requestData->insert(requestData->end(), payload->begin(), payload->end());
        delete payload;
        return sendRawRequest(requestData, response);
    }
};
#endif //CLIENT_HTTPSERVICE_H
