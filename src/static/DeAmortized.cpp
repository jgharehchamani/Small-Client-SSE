#include "DeAmortized.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>
#include <vector>
using namespace std;
using namespace boost::algorithm;

DeAmortized::DeAmortized(bool deleteFiles, int keyworsSize, int N) {
    this->deleteFiles = deleteFiles;
    l = ceil(log2(N));
    L = new DeAmortizedBASClient(l);
    for (int j = 0; j < 4; j++) {
        keys.push_back(vector<unsigned char*> ());
        for (int i = 0; i < l; i++) {
            unsigned char* tmpKey = new unsigned char[16];
            keys[j].push_back(tmpKey);
        }
    }
    for (int i = 0; i < l; i++) {
        cnt.push_back(0);
        bytes<Key> key{0};
        OMAP* omap = new OMAP(max((int) pow(2, i), 4), key);
        omaps.push_back(omap);
        setupOMAPS.push_back(map<Bid, string>());
        setupOMAPSDummies.push_back(0);
    }
    for (int i = 0; i < localSize; i++) {
        localmap.push_back(map<string, string>());
    }
    for (int j = 0; j < 4; j++) {
        vector< unordered_map<string, prf_type> > curVec;
        for (int i = 0; i < localSize; i++) {
            curVec.push_back(unordered_map<string, prf_type>());
        }
        data.push_back(curVec);
    }
}

DeAmortized::~DeAmortized() {
}

void DeAmortized::update(OP op, string keyword, int ind, bool setup) {
    if (!setup) {
        for (int i = 0; i < l; i++) {
            omaps[i]->treeHandler->oram->totalRead = 0;
            omaps[i]->treeHandler->oram->totalWrite = 0;
        }
        L->totalCommunication = 0;
        totalUpdateCommSize = 0;
    }
    updateCounter++;
    for (int i = l - 1; i > 0; i--) {
        if ((i > localSize && L->exist[0][i - 1] && L->exist[1][i - 1]) || (i <= localSize && data[0][i - 1].size() > 0 && data[1][i - 1].size() > 0)) {
            prf_type x;
            if (cnt[i] < pow(2, i - 1)) {
                x = (i <= localSize ? getElementAt(0, i - 1, cnt[i]) : L->get(0, i - 1, cnt[i], keys[0][i - 1]));
            } else {
                x = (i <= localSize ? getElementAt(1, i - 1, cnt[i] % (int) pow(2, i - 1)) : L->get(1, i - 1, cnt[i] % (int) pow(2, i - 1), keys[1][i - 1]));
            }
            cnt[i]++;
            string curKeyword((char*) x.data());
            int upCnt = (int) ceil((updateCounter - (6 * pow(2, i - 1) - 2)) / pow(2, i)) + 1;
            string c;
            c = (i < localSize ? (localmap[i].count(curKeyword + "-" + to_string(upCnt)) == 0 ? "" : localmap[i][curKeyword + "-" + to_string(upCnt)])
                    : (setup ? (setupOMAPS[i].count(getBid(curKeyword, upCnt)) == 0 ? "" : setupOMAPS[i][getBid(curKeyword, upCnt)]) : omaps[i]->incrementCnt(getBid(curKeyword, upCnt))));
            if (c == "") {
                if (i < localSize) {
                    localmap[i][curKeyword + "-" + to_string(upCnt)] = "1";
                } else {
                    if (setup) {
                        setupOMAPS[i][getBid(curKeyword, upCnt)] = "1"; //The else condition is satisfied by omaps[i]->incrementCnt
                    }
                }
                c = "1";
            } else {
                c = to_string(stoi(c) + 1);
                if (i < localSize) {
                    localmap[i][curKeyword + "-" + to_string(upCnt)] = c;
                } else {
                    if (setup) {
                        setupOMAPS[i][getBid(curKeyword, upCnt)] = c; //The else condition is satisfied by omaps[i]->incrementCnt
                    }
                }
            }

            if (i < localSize) {
                data[3][i][curKeyword + "-" + c] = x;
            } else {
                L->add(3, i, pair<string, prf_type>(curKeyword, x), stoi(c), keys[3][i]);
            }

            if ((i >= localSize && L->size(3, i) == pow(2, i)) || (i < localSize && data[3][i].size() == pow(2, i))) {
                if (i <= localSize) {
                    if (i == localSize) {
                        if (setup) {
                            setupOMAPSDummies[i] = upCnt;
                        } else {
                            omaps[i]->setDummy(upCnt);
                        }
                    } else {
                        localmap[i].erase(curKeyword + "-" + to_string(upCnt));
                    }
                    data[0][i - 1].clear();
                    data[1][i - 1].clear();
                    data[0][i - 1].insert(data[2][i - 1].begin(), data[2][i - 1].end());
                    data[2][i - 1].clear();
                } else {
                    if (setup) {
                        setupOMAPSDummies[i] = upCnt;
                    } else {
                        omaps[i]->setDummy(upCnt);
                    }
                    L->destry(0, i - 1);
                    L->destry(1, i - 1);
                    L->copy(2, i - 1, 0, i - 1);
                    L->destry(2, i - 1);
                }

                memcpy(keys[0][i - 1], keys[2][i - 1], 16);
                cnt[i] = 0;
                if ((i >= localSize && L->exist[0][i] == false) || (i < localSize && data[0][i].size() == 0)) {
                    if (i < localSize) {
                        data[0][i].clear();
                        data[0][i].insert(data[3][i].begin(), data[3][i].end());
                        data[3][i].clear();
                    } else {
                        L->copy(3, i, 0, i);
                        L->destry(3, i);
                    }
                    memcpy(keys[0][i], keys[3][i], 16);
                } else if ((i >= localSize && L->exist[1][i] == false) || (i < localSize && data[1][i].size() == 0)) {
                    if (i < localSize) {
                        data[1][i].clear();
                        data[1][i].insert(data[3][i].begin(), data[3][i].end());
                        data[3][i].clear();
                    } else {
                        L->copy(3, i, 1, i);
                        L->destry(3, i);
                    }
                    memcpy(keys[1][i], keys[3][i], 16);
                } else if ((i >= localSize && L->exist[2][i] == false) || (i < localSize && data[2][i].size() == 0)) {
                    if (i < localSize) {
                        data[2][i].clear();
                        data[2][i].insert(data[3][i].begin(), data[3][i].end());
                        data[3][i].clear();
                    } else {
                        L->copy(3, i, 2, i);
                        L->destry(3, i);
                    }
                    memcpy(keys[2][i], keys[3][i], 16);
                }
                if (i >= localSize) {
                    for (int j = 0; j < 16; j++) {
                        keys[3][i][j] = (unsigned char) rand() % 256;
                    }
                }
            }
        }
    }

    prf_type value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(keyword.begin(), keyword.end(), value.begin());
    *(int*) (&(value.data()[AES_KEY_SIZE - 5])) = ind;
    value.data()[AES_KEY_SIZE - 6] = (byte) (op == OP::INS ? 0 : 1);

    data[3][0][keyword + "-1"] = value;

    if (data[0][0].size() == 0) {
        data[0][0].insert(data[3][0].begin(), data[3][0].end());
        data[3][0].clear();
    } else if (data[1][0].size() == 0) {
        data[1][0].insert(data[3][0].begin(), data[3][0].end());
        data[3][0].clear();
    } else {
        data[2][0].insert(data[3][0].begin(), data[3][0].end());
        data[3][0].clear();
    }
    if (!setup) {
        for (int i = 0; i < l; i++) {
            totalUpdateCommSize += (omaps[i]->treeHandler->oram->totalRead + omaps[i]->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
        }
        totalUpdateCommSize += L->totalCommunication;
    }
}

vector<int> DeAmortized::search(string keyword) {
    for (int i = 0; i < l; i++) {
        omaps[i]->treeHandler->oram->totalRead = 0;
        omaps[i]->treeHandler->oram->totalWrite = 0;
    }
    L->totalCommunication = 0;
    totalSearchCommSize = 0;
    vector<int> finalRes;
    vector<prf_type> encIndexes;
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < localSize; i++) {
            if (data[j][i].size() > 0) {
                int curCounter = 1;
                bool exist = true;
                do {
                    if (data[j][i].count(keyword + "-" + to_string(curCounter)) != 0) {
                        encIndexes.push_back(data[j][i][keyword + "-" + to_string(curCounter)]);
                        curCounter++;
                    } else {
                        exist = false;
                    }
                } while (exist);
            }
        }
    }
    for (int j = 0; j < 3; j++) {
        for (int i = localSize; i < l; i++) {
            if (L->exist[j][i]) {
                auto tmpRes = L->search(j, i, keyword, keys[j][i]);
                encIndexes.insert(encIndexes.end(), tmpRes.begin(), tmpRes.end());
            }
        }
    }

    map<int, int> remove;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) {
        prf_type decodedString = *i;
        int plaintext = *(int*) (&(decodedString.data()[AES_KEY_SIZE - 5]));
        remove[plaintext] += (2 * ((byte) decodedString.data()[AES_KEY_SIZE - 6]) - 1);
    }
    for (auto const& cur : remove) {
        if (cur.second < 0) {
            finalRes.emplace_back(cur.first);
        }
    }
    for (int i = 0; i < l; i++) {
        totalSearchCommSize += (omaps[i]->treeHandler->oram->totalRead + omaps[i]->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int));
    }
    totalSearchCommSize += L->totalCommunication;
    return finalRes;
}

prf_type DeAmortized::bitwiseXOR(int input1, int op, prf_type input2) {
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

prf_type DeAmortized::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void DeAmortized::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

Bid DeAmortized::getBid(string input, int cnt) {
    std::array< uint8_t, ID_SIZE> value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(input.begin(), input.end(), value.begin());
    *(int*) (&value[AES_KEY_SIZE - 4]) = cnt;
    Bid res(value);
    return res;
}

prf_type DeAmortized::getElementAt(int instance, int index, int pos) {
    auto iter = data[instance][index].begin();
    for (int i = 0; i < pos; i++) {
        iter++;
    }
    return (*iter).second;
}

void DeAmortized::endSetup() {
    for (unsigned int i = 0; i < setupOMAPS.size(); i++) {
        omaps[i]->setDummy(setupOMAPSDummies[i]);
        omaps[i]->setupInsert(setupOMAPS[i]);
    }
}

double DeAmortized::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double DeAmortized::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

