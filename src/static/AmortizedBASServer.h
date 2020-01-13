#ifndef BASSERVER_H
#define BASSERVER_H
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
        size_t result = 0; 
        for (size_t i = 0; i < AES_KEY_SIZE; ++i) {
            result = (result << 1) ^ hasher(key[i]); 
        }
        return result;
    }
};

class AmortizedBASServer {
private:
    vector< unordered_map<prf_type, prf_type, PRFHasher> > data;
    void getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result);

public:
    AmortizedBASServer(int dataIndex);
    void clear(int index);
    virtual ~AmortizedBASServer();
    void storeCiphers(int dataIndex, map<prf_type, prf_type> ciphers);
    vector<prf_type> search(int dataIndex, prf_type token);
    vector<prf_type> getAllData(int dataIndex);

};


#endif /* BASSERVER_H */

