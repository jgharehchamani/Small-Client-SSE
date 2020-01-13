#ifndef DEBASSERVER_H
#define DEBASSERVER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include "Server.h"
#include <iostream>
#include <sstream>
#include "mitra/Server.h"
#include "utils/Utilities.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sse/crypto/hash.hpp>
#include "Types.hpp"

struct PRFHasher {

    std::size_t operator()(const prf_type &key) const {
        std::hash<byte_t> hasher;
        size_t result = 0; // I would still seed this.
        for (size_t i = 0; i < AES_KEY_SIZE; ++i) {
            result = (result << 1) ^ hasher(key[i]); // ??
        }
        return result;
    }
};

class EachSet {
public:
    unordered_map<prf_type, prf_type, PRFHasher> setData;
};

class DeAmortizedBASServer {
private:
    vector<vector< EachSet* > > data; //OLDEST, OLDER, OLD, NEW;
    void getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result);

public:
    DeAmortizedBASServer(int dataIndex);
    void clear(int instance, int index);
    virtual ~DeAmortizedBASServer();
    void storeCiphers(int instance, int dataIndex, map<prf_type, prf_type> ciphers);
    vector<prf_type> search(int instance, int dataIndex, prf_type token);
    vector<prf_type> getAllData(int instance, int dataIndex);
    void copy(int fromInstance, int fromIndex, int toInstance, int toIndex);
    int size(int instance, int index);
    prf_type get(int instance, int index, int pos);
    void add(int instance, int index, pair<prf_type, prf_type> pair);
};


#endif /* BASSERVER_H */

