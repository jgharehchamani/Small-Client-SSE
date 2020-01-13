#ifndef BASCLIENT_H
#define BASCLIENT_H
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
#include "AmortizedBASServer.h"

using namespace std;

class AmortizedBASClient {
private:
    AmortizedBASServer* server;
    void getAESRandomValue(unsigned char* keyword, int cnt, unsigned char* result);

public:
    virtual ~AmortizedBASClient();
    AmortizedBASClient(int maxUpdate);
    int totalCommunication = 0;

    vector<bool> exist;
    void destry(int index);
    void setup(int index, unordered_map<string, vector<prf_type> >pairs, unsigned char* key);
    vector<prf_type> search(int index, string keyword, unsigned char* key);
    vector<prf_type> getAllData(int index, unsigned char* key);

};

#endif /* BASCLIENT_H */

