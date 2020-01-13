#ifndef CLIENT_H
#define CLIENT_H
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
#include "OMAP.h"
using namespace std;

enum OP {
    INS, DEL
};

class Client {
public:
    string Wg;
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int counter, unsigned char* result);
    Server* server;
    bool deleteFiles;
    double totalUpdateCommSize;
    double totalSearchCommSize;
    bool localStorage = false;
    OMAP* omap;
    map<prf_type, int> FileCnt;
    map<prf_type, int> SrcCnt;
    map<Bid, string> setupOMAP;
    inline Bid getBid(string input);

public:
    Client(Server* server, bool deleteFiles, int keyworsSize);
    Client(bool deleteFiles, int keyworsSize);
    void update(OP op, string keyword, int ind, bool setup);
    vector<int> search(string keyword);
    void updateRequest(OP op, string keyword, int ind, prf_type& address, prf_type& value);
    prf_type searchRequest(string keyword, vector<prf_type>& tokens);
    void searchProcess(vector<prf_type> tokens, prf_type k_w, vector<int>& ids, map<prf_type, prf_type>& cleaningPairs, string keyword);
    virtual ~Client();
    bool isSetupMode() const;
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    void endSetup();

};

#endif /* CLIENT_H */
