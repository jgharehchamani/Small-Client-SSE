#include "DeAmortizedBASServer.h"
#include <string.h>
#include <sse/crypto/prg.hpp>

DeAmortizedBASServer::DeAmortizedBASServer(int dataIndex) {
    for (int j = 0; j < 4; j++) {
        data.push_back(vector<EachSet*>());
        for (int i = 0; i < dataIndex; i++) {
            EachSet* curData = new EachSet();
            data[j].push_back(curData);
        }
    }
}

DeAmortizedBASServer::~DeAmortizedBASServer() {
}

void DeAmortizedBASServer::storeCiphers(int instance, int dataIndex, map<prf_type, prf_type> ciphers) {
    data[instance][dataIndex]->setData.insert(ciphers.begin(), ciphers.end());
}

vector<prf_type> DeAmortizedBASServer::search(int instance, int dataIndex, prf_type token) {
    vector<prf_type> results;
    bool exist = false;
    int cnt = 1;
    do {
        prf_type curToken = token, mapKey;
        getAESRandomValue(curToken.data(), cnt, mapKey.data());
        if (data[instance][dataIndex]->setData.count(mapKey) != 0) {
            results.push_back(data[instance][dataIndex]->setData[mapKey]);
            exist = true;
            cnt++;
        } else {
            exist = false;
        }
    } while (exist);
    return results;
}

void DeAmortizedBASServer::getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result) {
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = cnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

vector<prf_type> DeAmortizedBASServer::getAllData(int instance, int dataIndex) {
    vector<prf_type> results;
    for (auto item : data[instance][dataIndex]->setData) {
        results.push_back(item.second);
    }
    return results;
}

void DeAmortizedBASServer::clear(int instance, int index) {
    data[instance][index] = new EachSet();
}

void DeAmortizedBASServer::copy(int fromInstance, int fromIndex, int toInstance, int toIndex) {
    delete data[toInstance][toIndex];
    data[toInstance][toIndex] = data[fromInstance][fromIndex];
}

int DeAmortizedBASServer::size(int instance, int index) {
    return data[instance][index]->setData.size();
}

prf_type DeAmortizedBASServer::get(int instance, int index, int pos) {
    auto iter = data[instance][index]->setData.begin();
    for (int i = 0; i < pos; i++) {
        iter++;
    }
    return (*iter).second;
}

void DeAmortizedBASServer::add(int instance, int index, pair<prf_type, prf_type> keyValue) {
    data[instance][index]->setData[keyValue.first] = keyValue.second;
}

