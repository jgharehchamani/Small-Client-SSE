#include "AmortizedBASClient.h"
#include <sse/crypto/prg.hpp>

AmortizedBASClient::~AmortizedBASClient() {
    delete server;
}

AmortizedBASClient::AmortizedBASClient(int numOfDataSets) {
    server = new AmortizedBASServer(numOfDataSets);
    for (int i = 0; i < numOfDataSets; i++) {
        exist.push_back(false);
    }
}

void AmortizedBASClient::setup(int index, unordered_map<string, vector<prf_type> > pairs, unsigned char* key) {
    exist[index] = true;
    map<prf_type, prf_type> ciphers;
    for (auto pair : pairs) {
        prf_type K1 = Utilities::encode(pair.first + "-1", key);
        for (unsigned int i = 0; i < pair.second.size(); i++) {
            prf_type mapKey, mapValue;
            getAESRandomValue(K1.data(), i, mapKey.data());
            mapValue = Utilities::encode(pair.second[i].data(), key);
            ciphers[mapKey] = mapValue;
        }
    }
    totalCommunication += ciphers.size() * sizeof (prf_type)*2;
    server->storeCiphers(index, ciphers);
}

vector<prf_type> AmortizedBASClient::search(int index, string keyword, unsigned char* key) {
    vector<prf_type> finalRes;
    prf_type token = Utilities::encode(keyword + "-1", key);
    vector<prf_type> ciphers = server->search(index, token);
    for (auto cipher : ciphers) {
        prf_type plaintext;
        Utilities::decode(cipher, plaintext, key);

        finalRes.push_back(plaintext);
    }
    totalCommunication += ciphers.size() * sizeof (prf_type) + sizeof (prf_type);
    return finalRes;
}

vector<prf_type> AmortizedBASClient::getAllData(int index, unsigned char* key) {
    vector<prf_type> finalRes;
    auto ciphers = server->getAllData(index);
    for (auto cipher : ciphers) {
        prf_type plaintext;
        Utilities::decode(cipher, plaintext, key);
        finalRes.push_back(plaintext);
    }
    totalCommunication += ciphers.size() * sizeof (prf_type);
    return finalRes;
}

void AmortizedBASClient::destry(int index) {
    server->clear(index);
    exist[index] = false;
    totalCommunication += sizeof (int);
}

void AmortizedBASClient::getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result) {
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = cnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}