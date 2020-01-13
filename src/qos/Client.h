#ifndef SCHEME1_H
#define SCHEME1_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <mutex>
#include "Server.h"
#include "OMAP.h"
#include "../utils/Utilities.h"

using namespace std;

class Client {
private:
    Server* server;
    OMAP* Ocnt;
    OMAP* Ostate;
    OMAP* Odel;
    unordered_map<string, string> LOcnt;
    unordered_map<string, string> LOstate;
    unordered_map<string, string> LOdel;
    int N;
    int height;
    unsigned char IKey[16], DKey[16], secretKey[16];
    map<Bid, string> setupOcnt;
    map<Bid, string> setupOdel;
    map<Bid, string> setupOstate;
    bool localStorage = false;
    double totalUpdateCommSize;
    double totalSearchCommSize;

    void getAESRandomValue(unsigned char* keyword, int src_cnt, unsigned char* result);
    inline Bid getBid(string input);
    int getNodeOnPath(int leaf, int curDepth);
    prf_type bitwiseXOR(prf_type input1, prf_type input2);
    prf_type bitwiseXOR(unsigned char* input1, prf_type input2);

public:
    Client();
    virtual ~Client();
    void insert(string keyword, int id, bool setup);
    void remove(string keyword, int id, bool setup);
    vector<int> search(string keyword);
    Client(Server* server, int maxOMAPSize, int keywordSize);
    void endSetup();
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;

};

#endif /* SCHEME1_H */

