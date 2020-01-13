#ifndef HORUS_H
#define HORUS_H
#include "OMAP.h"
#include "PRFORAM.hpp"
#include <iostream>
using namespace std;

class Horus {
private:
    bool useHDD;
    map<Bid, string > setupPairs1;
    map<Bid, string > setupPairs2;
    map<Bid, int > setupPairsPos;
    map<Bid, int > setupCNTs;
    OMAP* OMAP_updt, *CNTs;
    map<string, int> UpdtCnt;
    map<string, int> SrchCnt;
    map<string, int> Access;
    map<string, int> LastIND;
    map<string, int> latestUpdatedCounter;
    double totalUpdateCommSize;
    double totalSearchCommSize;



    Bid createBid(byte_t prefix, string keyword, int number);
    Bid createBid(string keyword, int number);
    Bid createBid(string keyword, int val1, int val2, int val3);
    int generatePosition(string keyword, int updt_cnt, int src_cnt, int acc_cnt);
    inline Bid getBid(string input, string type);
    map<string, int> poses;
    int maxSize;
    PRFORAM* ORAM_srch;

public:
    void insert(string keyword, int ind);
    void setupInsert(string keyword, int ind);
    void remove(string keyword, int ind);
    void setupRemove(string keyword, int ind);
    vector<int> search(string keyword);
    Horus(bool useHDD, int maxSize);
    virtual ~Horus();
    void beginSetup();
    void endSetup();
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;

};

#endif /* HORUS_H */

