#ifndef DEAMORTIZED_H
#define DEAMORTIZED_H

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
#include "DeAmortizedBASClient.h"
#include "OMAP.h"

using namespace std;

enum OP {
    INS, DEL
};

class DeAmortized {
private:
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Bid getBid(string str, int cnt);
    bool deleteFiles;
    vector< vector<unsigned char*> > keys;
    vector<int> cnt;
    vector<OMAP*> omaps;
    vector< map<Bid, string> > setupOMAPS;
    vector<int> setupOMAPSDummies;
    DeAmortizedBASClient* L;
    vector<vector< unordered_map<string, prf_type> > > data; //OLDEST, OLDER, OLD, NEW;
    vector<map<string, string> > localmap;
    int updateCounter = 0;
    int localSize = 10;
    int l;
    prf_type getElementAt(int instance, int index, int pos);
    double totalUpdateCommSize;
    double totalSearchCommSize;

public:
    DeAmortized(bool deleteFiles, int keyworsSize, int N);
    void update(OP op, string keyword, int ind, bool setup);
    vector<int> search(string keyword);
    virtual ~DeAmortized();
    void endSetup();
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;

};

#endif /* BAS_H */

