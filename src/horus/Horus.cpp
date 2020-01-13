#include "Horus.h"
#include "utils/Utilities.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sse/crypto/prg.hpp>

typedef array<uint8_t, 16> prf_type;

using namespace boost::algorithm;

#define INC_FACTOR 4

Horus::Horus(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    bytes<Key> key3{2};
    OMAP_updt = new OMAP(maxSize * INC_FACTOR, key1);
    ORAM_srch = new PRFORAM(maxSize * INC_FACTOR, key2);
    CNTs = new OMAP(maxSize, key3);
    this->maxSize = maxSize;
}

Horus::~Horus() {
    delete OMAP_updt;
    delete ORAM_srch;
    delete CNTs;
}

void Horus::insert(string keyword, int ind) {
    OMAP_updt->treeHandler->oram->totalRead = 0;
    OMAP_updt->treeHandler->oram->totalWrite = 0;
    ORAM_srch->totalRead = 0;
    ORAM_srch->totalWrite = 0;
    CNTs->treeHandler->oram->totalRead = 0;
    CNTs->treeHandler->oram->totalWrite = 0;
    totalUpdateCommSize = 0;

    Bid mapKey = createBid(0, keyword, ind);
    string resStr = OMAP_updt->find(mapKey);
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    int updtCnt = 0, accessCnt = 1, srcCnt = 0, lastUpdtCnt = 0;
    if (resStr == "" || (tokens.size() > 0 && tokens[0] == "-1")) {
        string updtCntStr = CNTs->incrementUpdtCnt(getBid(keyword, "UpdtCnt"));
        if (updtCntStr == "") {
            CNTs->insert(getBid(keyword, "UpdtCnt"), to_string(1));
            CNTs->insert(getBid(keyword, "latestUpdatedCounter"), to_string(0));
            CNTs->insert(getBid(keyword, "SrchCnt"), to_string(0));
        } else {
            updtCnt = stoi(updtCntStr);
            srcCnt = stoi(CNTs->find(getBid(keyword, "SrchCnt")));
            lastUpdtCnt = stoi(CNTs->find(getBid(keyword, "latestUpdatedCounter")));
        }
        string accessCntStr = CNTs->find(getBid(keyword, "Access"));
        if (accessCntStr == "") {
            CNTs->insert(getBid(keyword, "Access"), to_string(1));
        } else {
            accessCnt = stoi(accessCntStr);
        }
        updtCnt++;
        Bid mapKey2 = createBid(1, keyword, updtCnt);
        resStr = OMAP_updt->find(mapKey2);
        int acc_cnt = 1;
        if (resStr != "") {
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            acc_cnt = stoi(tokens[1]) + 1;
            if (acc_cnt > accessCnt) {
                accessCnt = acc_cnt;
                CNTs->insert(getBid(keyword, "Access"), to_string(accessCnt));
            }
        }
        OMAP_updt->insert(mapKey, to_string(updtCnt) + "#" + to_string(acc_cnt));
        OMAP_updt->insert(mapKey2, to_string(ind) + "#" + to_string(acc_cnt));
        Bid oramID = createBid(keyword, updtCnt, srcCnt, acc_cnt);
        int pos = generatePosition(keyword, updtCnt, srcCnt, acc_cnt);
        ORAM_srch->WriteBox(oramID, to_string(ind), pos);
        if (lastUpdtCnt < updtCnt) {
            lastUpdtCnt++;
            CNTs->insert(getBid(keyword, "latestUpdatedCounter"), to_string(lastUpdtCnt));
            if (acc_cnt > 1) {
                Bid oramID2 = createBid(keyword, updtCnt, srcCnt, 1);
                int pos2 = generatePosition(keyword, updtCnt, srcCnt, 1);
                ORAM_srch->WriteBox(oramID2, "@" + to_string(acc_cnt), pos2);
            }
        }
        CNTs->insert(getBid(keyword, "LastIND"), to_string(ind));
    }
    totalUpdateCommSize = (OMAP_updt->treeHandler->oram->totalRead + OMAP_updt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (ORAM_srch->totalRead + ORAM_srch->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (CNTs->treeHandler->oram->totalRead + CNTs->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
}

/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup().
 */
void Horus::setupInsert(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    std::vector<std::string> tokens;
    if (UpdtCnt.count(keyword) == 0) {
        UpdtCnt[keyword] = 0;
    }
    if (Access.count(keyword) == 0) {
        Access[keyword] = 1;
    }
    UpdtCnt[keyword]++;
    Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
    int acc_cnt = 1;

    setupPairs1[mapKey] = to_string(UpdtCnt[keyword]) + "#" + to_string(acc_cnt);
    setupPairs1[mapKey2] = to_string(ind) + "#" + to_string(acc_cnt);
    Bid oramID = createBid(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
    int pos = generatePosition(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
    setupPairs2[oramID] = to_string(ind);
    setupPairsPos[oramID] = pos;
    LastIND[keyword] = ind;
}

void Horus::remove(string keyword, int ind) {
    OMAP_updt->treeHandler->oram->totalRead = 0;
    OMAP_updt->treeHandler->oram->totalWrite = 0;
    ORAM_srch->totalRead = 0;
    ORAM_srch->totalWrite = 0;
    CNTs->treeHandler->oram->totalRead = 0;
    CNTs->treeHandler->oram->totalWrite = 0;
    totalUpdateCommSize = 0;

    Bid mapKey = createBid(0, keyword, ind);
    string resStr = OMAP_updt->find(mapKey);
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    if (tokens.size() > 0 && stoi(tokens[0]) > 0) {
        int updt_cnt = stoi(tokens[0]);
        int acc_cnt = stoi(tokens[1]);
        OMAP_updt->insert(mapKey, "-1#" + to_string(acc_cnt + 1));
        int updtCnt = stoi(CNTs->decrementUpdtCnt(getBid(keyword, "UpdtCnt")));
        updtCnt--;
        if (updtCnt > 0) {
            int srcCnt = stoi(CNTs->find(getBid(keyword, "SrchCnt")));
            int lastind = stoi(CNTs->find(getBid(keyword, "LastIND")));
            if (updtCnt + 1 != updt_cnt) {
                acc_cnt++;
                int accessCnt = stoi(CNTs->find(getBid(keyword, "Access")));
                if (acc_cnt > accessCnt) {
                    accessCnt = acc_cnt;
                    CNTs->insert(getBid(keyword, "Access"), to_string(accessCnt));
                }

                Bid oramID = createBid(keyword, updt_cnt, srcCnt, acc_cnt);
                int pos = generatePosition(keyword, updt_cnt, srcCnt, acc_cnt);
                ORAM_srch->WriteBox(oramID, to_string(lastind), pos);
                mapKey = createBid(0, keyword, lastind);
                OMAP_updt->insert(mapKey, to_string(updt_cnt) + "#" + to_string(acc_cnt));
                mapKey = createBid(1, keyword, updt_cnt);
                OMAP_updt->insert(mapKey, to_string(lastind) + "#" + to_string(acc_cnt));
            }
            Bid mapKey2 = createBid(1, keyword, updtCnt);
            resStr = OMAP_updt->find(mapKey2);
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            lastind = stoi(tokens[0]);
            CNTs->insert(getBid(keyword, "LastIND"), to_string(lastind));
        } else {
            CNTs->insert(getBid(keyword, "LastIND"), "");
        }
    }
    totalUpdateCommSize = (OMAP_updt->treeHandler->oram->totalRead + OMAP_updt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (ORAM_srch->totalRead + ORAM_srch->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (CNTs->treeHandler->oram->totalRead + CNTs->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
}

/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup().
 */
void Horus::setupRemove(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    string resStr = setupPairs1[mapKey];
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    if (tokens.size() > 0 && stoi(tokens[0]) > 0) {
        int updt_cnt = stoi(tokens[0]);
        int acc_cnt = stoi(tokens[1]);
        setupPairs1[mapKey] = "-1#" + to_string(acc_cnt + 1);
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != updt_cnt) {
                acc_cnt++;
                if (acc_cnt > Access[keyword]) {
                    Access[keyword] = acc_cnt;
                }
                Bid oramID = createBid(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                int pos = generatePosition(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                setupPairs2[oramID] = to_string(LastIND[keyword]);
                setupPairsPos[oramID] = pos;
                mapKey = createBid(0, keyword, LastIND[keyword]);
                setupPairs1[mapKey] = to_string(updt_cnt) + "#" + to_string(acc_cnt);
                mapKey = createBid(1, keyword, updt_cnt);
                setupPairs1[mapKey] = to_string(LastIND[keyword]) + "#" + to_string(acc_cnt);
            }
            Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
            resStr = setupPairs1[mapKey2];
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            LastIND[keyword] = stoi(tokens[0]);
        } else {
            LastIND.erase(keyword);
        }
    }
}

vector<int> Horus::search(string keyword) {
    OMAP_updt->treeHandler->oram->totalRead = 0;
    OMAP_updt->treeHandler->oram->totalWrite = 0;
    ORAM_srch->totalRead = 0;
    ORAM_srch->totalWrite = 0;
    CNTs->treeHandler->oram->totalRead = 0;
    CNTs->treeHandler->oram->totalWrite = 0;
    totalSearchCommSize = 0;

    vector<int> result;
    vector<int> left, right, curValue, lastAcc;
    int updtCnt = stoi(CNTs->find(getBid(keyword, "UpdtCnt")));
    int accessCnt = stoi(CNTs->find(getBid(keyword, "Access")));
    int srcCnt = stoi(CNTs->incrementSrcCnt(getBid(keyword, "SrchCnt")));
    string* lastID = new string[updtCnt];
    vector<bool> foundItem;
    map<Bid, int> batchWriteQuery;
    map<Bid, string> batchWriteQueryID;
    for (int i = 1; i <= updtCnt; i++) {
        foundItem.push_back(false);
        right.push_back(accessCnt);
        left.push_back(1);
        curValue.push_back(1);
        lastAcc.push_back(0);
    }
    bool firstRun = true;
    bool foundAll = false;
    while (!foundAll) {
        foundAll = true;
        vector<pair<Bid, int> > batchReadQuery;
        for (int i = 1; i <= updtCnt; i++) {
            if (foundItem[i - 1] == false) {
                foundAll = false;
                int pos = generatePosition(keyword, i, srcCnt, curValue[i - 1]);
                Bid oramID = createBid(keyword, i, srcCnt, curValue[i - 1]);
                batchReadQuery.push_back(make_pair(oramID, pos));
            }
        }
        if (foundAll == false) {
            vector<string> ids = ORAM_srch->batchRead(batchReadQuery);
            int tmpCnt = 0;
            for (int i = 0; i < updtCnt; i++) {
                if (foundItem[i] == false) {
                    string id = ids[tmpCnt];
                    tmpCnt++;
                    if (firstRun) {
                        if (id != "" && id.at(0) == '@' && left[i] == 1) {
                            left[i] = stoi(id.substr(1, id.length() - 1));
                            curValue[i] = (left[i] + right[i]) / 2;
                            lastAcc[i] = curValue[i];
                        } else {
                            curValue[i] = (left[i] + right[i]) / 2;
                        }
                        continue;
                    }

                    if (id == "" || left[i] >= right[i]) {
                        if (right[i] <= left[i]) {
                            foundItem[i] = true;
                            if (lastID[i] == "") {
                                lastID[i] = id;
                                lastAcc[i] = curValue[i];
                            }
                            result.push_back(stoi(lastID[i]));
                            Bid oramID = createBid(keyword, i + 1, srcCnt + 1, lastAcc[i]);
                            int pos = generatePosition(keyword, i + 1, srcCnt + 1, lastAcc[i]);
                            batchWriteQuery.insert(make_pair(oramID, pos));
                            batchWriteQueryID.insert(make_pair(oramID, lastID[i]));
                            if (lastAcc[i] > 1) {
                                oramID = createBid(keyword, i + 1, srcCnt + 1, 1);
                                pos = generatePosition(keyword, i + 1, srcCnt + 1, 1);
                                batchWriteQuery.insert(make_pair(oramID, pos));
                                batchWriteQueryID.insert(make_pair(oramID, "@" + to_string(lastAcc[i])));
                            }
                        } else {

                            if (curValue[i] == right[i]) {
                                curValue[i] = (int) floor((double) (left[i] + right[i]) / 2.0);
                                right[i] = curValue[i];
                            } else {
                                right[i] = curValue[i];
                                curValue[i] = (int) ceil((double) (left[i] + right[i]) / 2.0);
                            }
                        }
                    } else {
                        lastID[i] = id;
                        lastAcc[i] = curValue[i];
                        if (curValue[i] == left[i]) {
                            left[i] = curValue[i];
                            curValue[i] = (int) ceil((double) (left[i] + right[i]) / 2.0);
                        } else {
                            left[i] = curValue[i];
                            curValue[i] = (int) floor((double) (left[i] + right[i]) / 2.0);
                        }
                    }
                }
            }
            firstRun = false;
        }
    }
    srcCnt++;
    ORAM_srch->batchWrite(batchWriteQueryID, batchWriteQuery);
    CNTs->insert(getBid(keyword, "latestUpdatedCounter"), to_string(result.size()));
    delete[] lastID;
    totalSearchCommSize = (OMAP_updt->treeHandler->oram->totalRead + OMAP_updt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (ORAM_srch->totalRead + ORAM_srch->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (CNTs->treeHandler->oram->totalRead + CNTs->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    return result;
}

/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Horus::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Horus::endSetup() {
    cout << "inserting OMAP_updt" << endl;
    OMAP_updt->setupInsert(setupPairs1);
    for (int i = 0; i < 500; i++) {
        Bid testBid;
        testBid.setValue(1);
        OMAP_updt->insert(testBid, "TEST");
    }
    cout << "inserting ORAM_srch" << endl;
    ORAM_srch->setupInsert(setupPairs2, setupPairsPos);
    cout << "Creating CNTs pairs" << endl;
    map<Bid, string> pairs;
    for (auto item : UpdtCnt) {
        Bid bid = getBid(item.first, "UpdtCnt");
        pairs[bid] = to_string(item.second);
    }
    for (auto item : SrchCnt) {
        Bid bid = getBid(item.first, "SrchCnt");
        pairs[bid] = to_string(item.second);
    }
    for (auto item : Access) {
        Bid bid = getBid(item.first, "Access");
        pairs[bid] = to_string(item.second);
    }
    for (auto item : LastIND) {
        Bid bid = getBid(item.first, "LastIND");
        pairs[bid] = to_string(item.second);
    }
    for (auto item : latestUpdatedCounter) {
        Bid bid = getBid(item.first, "latestUpdatedCounter");
        pairs[bid] = to_string(item.second);
    }
    UpdtCnt.clear();
    SrchCnt.clear();
    Access.clear();
    LastIND.clear();
    latestUpdatedCounter.clear();
    cout << "Inserting in CNTs" << endl;
    CNTs->setupInsert(pairs);
    for (int i = 0; i < 500; i++) {
        Bid testBid;
        testBid.setValue(1);
        CNTs->insert(testBid, "TEST");
    }
}

Bid Horus::createBid(byte_t prefix, string keyword, int number) {
    Bid bid;
    bid.id[0] = prefix;
    std::copy(keyword.begin(), keyword.end(), bid.id.begin() + 1);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid Horus::createBid(string keyword, int val1, int val2, int val3) {
    Bid bid;
    auto arr = to_bytes(val1);
    std::copy(arr.begin(), arr.end(), bid.id.begin());
    arr = to_bytes(val2);
    std::copy(arr.begin(), arr.end(), bid.id.begin() + 4);
    arr = to_bytes(val3);
    std::copy(arr.begin(), arr.end(), bid.id.begin() + 8);
    std::copy(keyword.begin(), keyword.end(), bid.id.begin() + 12);
    return bid;
}

Bid Horus::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

/*
 * This function generates the corresponding position of Path-ORAM using a AES PRF
 */
int Horus::generatePosition(string keyword, int updt_cnt, int src_cnt, int acc_cnt) {
    string keyStr = keyword + "-" + to_string(updt_cnt) + "-" + to_string(src_cnt) + "-" + to_string(acc_cnt);
    unsigned int result = 0;
    unsigned char key[32];
    memset(key, 0, 32);
    for (unsigned int i = 0; i < keyStr.length() && i < 31; i++) {
        key[i] = keyStr.at(i);
    }
    sse::crypto::Prg::derive(key, 0, 4, (unsigned char*) &result);
    int pos = (result) % (int) ((pow(2, floor(log2(maxSize * INC_FACTOR / Z)) + 1) - 1) / 2);
    return pos;
}

Bid Horus::getBid(string input, string type) {
    std::array< uint8_t, ID_SIZE> value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(input.begin(), input.end(), value.begin());
    if (type == "UpdtCnt") {
        *(int*) (&(value.data()[ID_SIZE - 4])) = 1;
    } else if (type == "SrchCnt") {
        *(int*) (&(value.data()[ID_SIZE - 4])) = 2;
    } else if (type == "Access") {
        *(int*) (&(value.data()[ID_SIZE - 4])) = 3;
    } else if (type == "LastIND") {
        *(int*) (&(value.data()[ID_SIZE - 4])) = 4;
    } else if (type == "latestUpdatedCounter") {
        *(int*) (&(value.data()[ID_SIZE - 4])) = 5;
    }
    Bid res(value);
    return res;
}

double Horus::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Horus::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}