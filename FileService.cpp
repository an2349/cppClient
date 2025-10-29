//
// Created by an on 10/7/25.
//

#include "FileService.h"


vector<MultiPartModel>* FileService::readFileChunks(const string &filepath) {
    vector<MultiPartModel>* parts = new vector<MultiPartModel>();
    ifstream fin(filepath, ios::binary);
    if (!fin) return parts;

    fin.seekg(0, ios::end);
    size_t filesize = fin.tellg();
    fin.seekg(0, ios::beg);

    int total = (filesize + MAX_CONTENT_SIZE - 1) / MAX_CONTENT_SIZE;
    for (int i = 0; i < total; i++) {
        MultiPartModel* mp  = new MultiPartModel();
        mp->name = fs::path(filepath).filename().string();
        mp->totalPart = total;
        mp->part = i + 1;
        mp->contentType = "application/octet-stream";

        size_t size = min((size_t) MAX_CONTENT_SIZE, filesize - i * MAX_CONTENT_SIZE);
        mp->value.resize(size);
        fin.read((char *) mp->value.data(), size);
        parts->push_back(*mp);
        delete mp;
    }
    return parts;
}


void FileService::pushFile(const string &filepath) {
    string boundary = Utils::generateBoundary();
    auto chunks = readFileChunks(filepath);
    size_t totalSize = 0;
    for (auto &chunk: *chunks)
        totalSize += chunk.value.size();
    string response = "";
    bool isSuccess = false;
    for (size_t i = 0; i < chunks->size(); ++i) {
        isSuccess = true;
        auto &chunk = ((*chunks)[i]);
        http.putFileChunk(chunk, boundary, totalSize, response);
        string code = "";
        if (response.size() >= 12) {
            code = response.substr(9, 3);
            if (code == "203") {
                isSuccess = false;
            }
        } else {
            //cout << "Warning " << endl;
            isSuccess = false;
           break;
        }}
    if (isSuccess) {
        cout<< escapeResponse(response)<<endl;
    }
    else {
        cout<<"Upload that bai"<<endl;
    }
    delete chunks;
    }

void FileService::pushAll(const string &folderpath, const string &types = "") {
    vector<string> exts;
    if (!types.empty()) exts = Utils::split(types, ',');
    vector<thread> threads;

    for (auto &p: fs::directory_iterator(folderpath)) {
        if (!p.is_regular_file()) continue;
        string ext = p.path().extension().string();
        if (!exts.empty()) {
            bool ok = false;
            for (auto &e: exts) if (ext == ("." + e)) ok = true;
            if (!ok) continue;
        }

        threads.emplace_back([this, path=p.path().string()] {
            pushFile(path);
        });
    }

    for (auto &t: threads) t.join();
}

