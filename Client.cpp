#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <cctype>
#include <iomanip>
#include <fstream>
#include "config.h"
#include "Utils.h"
#include "HttpService.h"
#include "FileService.h"

#ifdef _WIN32
    // Windows Headers
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "iphlpapi.lib")
#else
    // Linux Headers
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <dirent.h>
    #include <unistd.h>
#endif
// ----------------------------------------------

using namespace std;
namespace fs = filesystem;

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

// Hàm lấy MAC Address đa nền tảng
string getMacAddress() {
#ifdef _WIN32
    // --- Logic cho Windows ---
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBufLen = sizeof(AdapterInfo);
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

    if (dwStatus != ERROR_SUCCESS) return "";

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    while (pAdapterInfo) {
        // Lấy MAC của interface vật lý (Ethernet/Wifi), bỏ qua Loopback
        if (pAdapterInfo->AddressLength == 6 && pAdapterInfo->Type != MIB_IF_TYPE_LOOPBACK) {
            stringstream ss;
            for (UINT i = 0; i < pAdapterInfo->AddressLength; i++) {
                ss << hex << setw(2) << setfill('0') << (int) pAdapterInfo->Address[i];
                if (i < pAdapterInfo->AddressLength - 1) ss << ":";
            }
            return ss.str();
        }
        pAdapterInfo = pAdapterInfo->Next;
    }
    return "";
#else
    // --- Logic cho Linux (như code gốc) ---
    // Helper lambda để lấy interface active
    auto getActiveInterface = []() -> string {
        DIR *dir = opendir("/sys/class/net");
        if (!dir) return "";

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            string iface = entry->d_name;
            if (iface == "." || iface == ".." || iface == "lo") continue;

            string operstate_path = "/sys/class/net/" + iface + "/operstate";
            ifstream fin(operstate_path);
            if (!fin.is_open()) continue;

            string state;
            getline(fin, state);
            fin.close();

            if (state == "up") {
                closedir(dir);
                return iface;
            }
        }
        closedir(dir);
        return "";
    };

    string iface = getActiveInterface();
    if (iface.empty()) return "";

    string path = "/sys/class/net/" + iface + "/address";
    ifstream fin(path);
    if (!fin.is_open()) return "";

    string mac;
    getline(fin, mac);
    fin.close();

    while (!mac.empty() && (mac.back() == '\n' || mac.back() == '\r'))
        mac.pop_back();

    return mac;
#endif
}


int main() {
    // Khởi tạo Winsock nếu là Windows
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed." << endl;
        return 1;
    }
#endif

    HttpService http;
    FileService fileService(http);;;;;

    string input;

    while (true) {
        cout << "\r";
        /*fs::path homePath = getenv("HOME");*/
        /*if (fs::exists(homePath))
            fs::current_path(homePath);*/
        string currentPath = fs::current_path().string();
        cout << currentPath << " > ";
        getline(cin, input);
        input = Utils::trim(input);
        if (input.empty()) continue;

        if (input == "quit") break;

        auto parts = Utils::split(input, ' ');
        string cmd = parts[0];

        if (cmd == "cd") {
            if (parts.size() < 2) {
                cout << "Ban phai chon : cd <duong dan>" << endl;
                continue;
            }
            string newPath = parts[1];
            if (!fs::exists(newPath) || !fs::is_directory(newPath)) {
                cout << "Khong tim thay url" << endl;
                continue;
            }
            fs::current_path(newPath);
        } else if (cmd == "ls") {
            fs::path pathToList = currentPath;
            if (parts.size() >= 2) pathToList = parts[1];

            if (!fs::exists(pathToList) || !fs::is_directory(pathToList)) {
                cout << "Khong tim thay url" << endl;
                continue;
            }

            for (const auto &entry: fs::directory_iterator(pathToList)) {
                cout << (entry.is_directory() ? "[Folder] " : "      ")
                        << entry.path().filename().string() << endl;
            }
        } else if (cmd == "diemdanh") {
            if (parts.size() < 2) {
                cout << "Phai la: diemdanh <ma sinh vien>" << endl;
                continue;
            }
            string response;
            http.postDiemDanh(parts[1], getMacAddress(), response);

            if (response.empty()) {
                cout << "Server khong tra gi" << endl;
            } else {
               // cout<<response<<endl;
                cout << escapeResponse(response) << endl;
            }
        } else if (cmd == "push") {
            if (parts.size() < 2) {
                cout << "Phai la: push <file>" << endl;
                continue;
            }
            string filepath = parts[1];
            if (!fs::exists(filepath)) {
                cout << "File khong ton tai" << endl;
                continue;
            }
            fileService.pushFile(filepath);
        } else if (cmd == "pushall") {
            string folder = currentPath;
            string types = "";

            if (parts.size() == 3 && parts[1] == "-t") {
                types = parts[2];
            } else if (parts.size() == 4 && parts[2] == "-t") {
                folder = parts[1];
                types = parts[3];
            } else if (parts.size() == 2 && parts[1] != "-t") {
                folder = parts[1];
            }

            fileService.pushAll(folder, types);
        }
        else {
            cout<<"Lenh khong hop le"<<endl;
        }
    }

    // Cleanup Winsock nếu là Windows
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
