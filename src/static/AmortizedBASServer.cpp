#include "AmortizedBASServer.h"
#include <string.h>
#include <sse/crypto/prg.hpp>

AmortizedBASServer::AmortizedBASServer(int dataIndex) {
    for (int i = 0; i < dataIndex; i++) {
        unordered_map<prf_type, prf_type, PRFHasher> curData;
        data.push_back(curData);
    }
}

AmortizedBASServer::~AmortizedBASServer() {
}

void AmortizedBASServer::storeCiphers(int dataIndex, map<prf_type, prf_type> ciphers) {
    data[dataIndex].insert(ciphers.begin(), ciphers.end());
}

vector<prf_type> AmortizedBASServer::search(int dataIndex, prf_type token) {
    vector<prf_type> results;
    bool exist = false;
    int cnt = 0;
    do {
        prf_type curToken = token, mapKey;
        getAESRandomValue(curToken.data(), cnt, mapKey.data());
        if (data[dataIndex].count(mapKey) != 0) {
            results.push_back(data[dataIndex][mapKey]);
            exist = true;
            cnt++;
        } else {
            exist = false;
        }
    } while (exist);
    return results;
}

void AmortizedBASServer::getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result) {
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = cnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

vector<prf_type> AmortizedBASServer::getAllData(int dataIndex) {
    vector<prf_type> results;
    for (auto item : data[dataIndex]) {
        results.push_back(item.second);
    }
    return results;
}

void AmortizedBASServer::clear(int index) {
    data[index].clear();
}
