#ifndef DEBASCLIENT_H
#define DEBASCLIENT_H
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
#include "DeAmortizedBASServer.h"

using namespace std;

class DeAmortizedBASClient {
private:
    DeAmortizedBASServer* server;
    void getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result);

public:
    virtual ~DeAmortizedBASClient();
    DeAmortizedBASClient(int maxUpdate);
    int totalCommunication = 0;

    vector<vector<bool> > exist;
    void destry(int instance, int index);
    void setup(int instance, int index, vector<pair<string, prf_type> >pairs, unsigned char* key);
    vector<prf_type> search(int instance, int index, string keyword, unsigned char* key);
    vector<prf_type> getAllData(int instance, int index, unsigned char* key);
    void copy(int fromInstance, int fromIndex, int toInstance, int toIndex);
    int size(int instance, int index);
    prf_type get(int instance, int index, int pos, unsigned char* key);
    void add(int instance, int index, pair<string, prf_type> pair, int cnt, unsigned char* key);
};

#endif /* DEBASCLIENT_H */

