#ifndef BAS_H
#define BAS_H

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
#include "AmortizedBASClient.h"
#include <set>

using namespace std;

enum OP {
    INS, DEL
};

class Amortized {
private:
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    bool deleteFiles;
    vector<unsigned char*> keys;
    AmortizedBASClient* L;
    int updateCounter = 0;
    double totalUpdateCommSize;
    double totalSearchCommSize;
    vector< unordered_map< string, vector<prf_type > > > data;
    int localSize = 10;

public:
    Amortized(bool deleteFiles, int keyworsSize, int N);
    void update(OP op, string keyword, int ind, bool setup);
    vector<int> search(string keyword);
    virtual ~Amortized();
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    void endSetup();

};

#endif /* BAS_H */

