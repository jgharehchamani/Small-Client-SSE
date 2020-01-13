#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <vector>
#include "OMAP.h"
using namespace std;

#define AES_KEY_SIZE 16

typedef array<uint8_t, AES_KEY_SIZE> prf_type;

struct PRFHasher {

    std::size_t operator()(const prf_type &key) const {
        std::hash<uint8_t> hasher;
        size_t result = 0;
        for (size_t i = 0; i < AES_KEY_SIZE; ++i) {
            result = (result << 1) ^ hasher(key[i]);
        }
        return result;
    }
};

class Server {
private:
    bool useRocksDB;
    int N, height;
    unordered_map<prf_type, prf_type, PRFHasher> I, Dstate;
    vector<int> findCoveringSet(int a_w);
    void recursiveSearch(prf_type key, int index, vector<int>& result, vector<int>& deletes);
    int getNodeOnPath(int leaf, int curDepth);

public:
    virtual ~Server();
    Server(bool useHDD, int N);
    void insert(prf_type key, prf_type value);
    void insertInDstate(prf_type key, prf_type value);
    map<int, prf_type> search(prf_type keyI, prf_type KeyD, int a_w, vector<int>& deletes);
    void insert(vector<prf_type> keys, vector<prf_type> values, vector<prf_type> Dkeys, vector<prf_type> Dvalues);
};

#endif /* SERVER_H */

