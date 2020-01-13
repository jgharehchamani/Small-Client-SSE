#include "Client.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>

using namespace std;
using namespace boost::algorithm;

Client::Client(Server* server, bool deleteFiles, int keyworsSize) {
    this->server = server;
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
    if (!localStorage) {
        omap = new OMAP(keyworsSize, key);
    }
}

Client::Client(bool deleteFiles, int keyworsSize) {
    this->deleteFiles = deleteFiles;
    bytes<Key> key{0};
    if (!localStorage) {
        omap = new OMAP(keyworsSize, key);
    }
}

Client::~Client() {
}

void Client::update(OP op, string keyword, int ind, bool setup) {
    if (!localStorage) {
        omap->treeHandler->oram->totalRead = 0;
        omap->treeHandler->oram->totalWrite = 0;
    }
    totalUpdateCommSize = 0;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int fileCnt = 0, srcCnt = 0;

    if (localStorage) {
        if (FileCnt.find(k_w) == FileCnt.end()) {
            FileCnt[k_w] = 1;
            fileCnt = 1;
        } else {
            FileCnt[k_w]++;
            fileCnt = FileCnt[k_w];
        }
        if (deleteFiles) {
            if (SrcCnt.find(k_w) == SrcCnt.end()) {
                SrcCnt[k_w] = 0;
                srcCnt = 0;
            } else {
                srcCnt = SrcCnt[k_w];
            }
        }
    } else {
        Bid mapKey = getBid(keyword);
        string fileCntStr = setup ? setupOMAP[mapKey] : omap->incrementFileCnt(mapKey);
        if (fileCntStr != "") {
            auto parts = Utilities::splitData(fileCntStr, "-");
            fileCnt = stoi(parts[0]);
            srcCnt = stoi(parts[1]);
        }
        fileCnt++;
        if (setup) {
            setupOMAP[mapKey] = to_string(fileCnt) + "-" + to_string(srcCnt);
        }
    }

    prf_type addr, rnd;
    getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
    getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
    prf_type val = bitwiseXOR(ind, op, rnd);
    if (!localStorage) {
        totalUpdateCommSize = (sizeof (prf_type) * 2) + (omap->treeHandler->oram->totalRead + omap->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    server->update(addr, val);

}

vector<int> Client::search(string keyword) {
    if (!localStorage) {
        omap->treeHandler->oram->totalRead = 0;
        omap->treeHandler->oram->totalWrite = 0;
    }
    totalSearchCommSize = 0;
    vector<int> finalRes;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    Bid mapKey = getBid(keyword);
    int fileCnt = 0, srcCnt = 0;
    if (localStorage) {
        if (FileCnt.find(k_w) == FileCnt.end()) {
            return finalRes;
        }
        fileCnt = FileCnt[k_w];
        if (deleteFiles) {
            srcCnt = SrcCnt[k_w];
        }
    } else {

        string fileCntStr = omap->find(mapKey);
        if (fileCntStr != "") {
            auto parts = Utilities::splitData(fileCntStr, "-");
            fileCnt = stoi(parts[0]);
            srcCnt = stoi(parts[1]);
        } else {
            return finalRes;
        }
    }

    KList.reserve(fileCnt);
    finalRes.reserve(fileCnt);
    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
        KList.emplace_back(rnd);
    }
    vector<prf_type> encIndexes = server->search(KList);
    map<int, int> remove;
    int cnt = 1;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
        cnt++;
    }
    if (deleteFiles) {
        if (localStorage) {
            SrcCnt[k_w]++;
        }
        fileCnt = 0;
        srcCnt++;
    }
    for (auto const& cur : remove) {
        if (cur.second < 0) {
            finalRes.emplace_back(cur.first);
            if (deleteFiles) {
                fileCnt++;
                prf_type addr, rnd;
                getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
                getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
                prf_type val = bitwiseXOR(cur.first, OP::INS, rnd);
                server->update(addr, val);
            }
        }
    }
    if (deleteFiles) {
        totalSearchCommSize += (fileCnt * 2 * sizeof (prf_type));
        if (localStorage) {
            FileCnt[k_w] = fileCnt;
        } else {
            omap->insert(mapKey, to_string(fileCnt) + "-" + to_string(srcCnt));
        }
    }
    if (!localStorage) {
        totalSearchCommSize += sizeof (prf_type) * KList.size() + encIndexes.size() * sizeof (prf_type) + (omap->treeHandler->oram->totalRead + omap->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    return finalRes;
}

prf_type Client::bitwiseXOR(int input1, int op, prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    result[4] = input2[4] ^ (op & 0xFF);
    for (int i = 5; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type Client::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void Client::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

double Client::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Client::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

Bid Client::getBid(string input) {
    std::array< uint8_t, ID_SIZE> value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(input.begin(), input.end(), value.begin());
    Bid res(value);
    return res;
}

void Client::endSetup() {
    if (!localStorage) {
        omap->setupInsert(setupOMAP);
        cout << "setup size:" << setupOMAP.size() << endl;

        for (int i = 0; i < 500; i++) {
            cout << i << "/500" << endl;
            Bid testBid;
            testBid.setValue(1);
            omap->insert(testBid, "TEST");
        }
    }
}