#include "Amortized.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>

using namespace std;
using namespace boost::algorithm;

Amortized::Amortized(bool deleteFiles, int keyworsSize, int N) {
    this->deleteFiles = deleteFiles;
    L = new AmortizedBASClient(ceil(log2(N)));
    for (int i = 0; i < ceil(log2(N)); i++) {
        keys.push_back(NULL);
    }
    for (int i = 0; i < localSize; i++) {
        data.push_back(unordered_map<string, vector<prf_type> >());
    }
}

Amortized::~Amortized() {
}

void Amortized::update(OP op, string keyword, int ind, bool setup) {
    totalUpdateCommSize = 0;
    L->totalCommunication = 0;
    int rm0 = log2((~updateCounter & (updateCounter + 1)));
    updateCounter++;
    unordered_map<string, vector<prf_type> > previousData;


    for (int i = 0; i < min(rm0, localSize); i++) {
        for (auto item : data[i]) {
            if (previousData.count(item.first) == 0) {
                previousData[item.first] = vector<prf_type>();
            }
            previousData[item.first].insert(previousData[item.first].end(), item.second.begin(), item.second.end());
        }
        data[i].clear();
    }
    for (int i = localSize; i < rm0; i++) {
        vector<prf_type> curData = L->getAllData(i, keys[i]);
        for (auto item : curData) {
            string curKeyword((char*) item.data());
            if (previousData.count(curKeyword) == 0) {
                previousData[curKeyword] = vector < prf_type>();
            }
            previousData[curKeyword].push_back(item);
        }

        L->destry(i);
        delete keys[i];
        keys[i] = NULL;
    }
    prf_type value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(keyword.begin(), keyword.end(), value.begin());
    *(int*) (&(value.data()[AES_KEY_SIZE - 5])) = ind;
    value.data()[AES_KEY_SIZE - 6] = (byte) (op == OP::INS ? 0 : 1);

    if (previousData.count(keyword) == 0) {
        previousData[keyword] = vector<prf_type>();
    }
    previousData[keyword].push_back(value);
    if (rm0 < localSize) {
        data[rm0].insert(previousData.begin(), previousData.end());
    } else {
        unsigned char* newKey = new unsigned char[16];
        keys[rm0] = newKey;
        L->setup(rm0, previousData, newKey);
        totalUpdateCommSize += L->totalCommunication;
    }
}

vector<int> Amortized::search(string keyword) {
    totalSearchCommSize = 0;
    L->totalCommunication = 0;
    vector<int> finalRes;
    vector<prf_type> encIndexes;
    int s = data.size();
    for (int i = 0; i < min(localSize, s); i++) {
        if (data[i][keyword].size() > 0) {
            encIndexes.insert(encIndexes.end(), data[i][keyword].begin(), data[i][keyword].end());
        }
    }
    for (unsigned int i = localSize; i < L->exist.size(); i++) {
        if (L->exist[i]) {
            auto tmpRes = L->search(i, keyword, keys[i]);
            encIndexes.insert(encIndexes.end(), tmpRes.begin(), tmpRes.end());
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
    totalSearchCommSize += L->totalCommunication;
    return finalRes;
}

prf_type Amortized::bitwiseXOR(int input1, int op, prf_type input2) {
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

prf_type Amortized::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void Amortized::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

double Amortized::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Amortized::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

void Amortized::endSetup() {

}
