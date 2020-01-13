#include "Client.h"
#include <sse/crypto/prg.hpp>

Client::Client() {
}

Client::~Client() {
}

Client::Client(Server* server, int N, int keywordSize) {
    this->server = server;
    bytes<Key> key{0};
    for (int i = 0; i < 16; i++) {
        IKey[i] = 1;
        DKey[i] = 2;
        secretKey[i] = 3;
    }
    this->Ocnt = new OMAP(keywordSize, key);
    this->Ostate = new OMAP(N, key);
    this->Odel = new OMAP(N, key);
    this->N = N;
    this->height = (int) ceil(log2(N)) + 1; //root is level 1
}

void Client::insert(string keyword, int id, bool setup) {
    if (!setup) {
        Ocnt->treeHandler->oram->totalRead = 0;
        Ocnt->treeHandler->oram->totalWrite = 0;
        Ostate->treeHandler->oram->totalRead = 0;
        Ostate->treeHandler->oram->totalWrite = 0;
        Odel->treeHandler->oram->totalRead = 0;
        Odel->treeHandler->oram->totalWrite = 0;
        totalUpdateCommSize = 0;
    }

    int src_cnt = 0, a_w = -1;
    Bid key = getBid(keyword);
    string result;
    if (localStorage) {
        result = LOcnt.count(keyword) == 0 ? "" : LOcnt[keyword];
    } else {
        result = setup ? setupOcnt[key] : Ocnt->incrementAwCnt(key);
    }
    if (result != "") {
        vector<string> parts = Utilities::splitData(result, "-");
        src_cnt = stoi(parts[0]);
        a_w = stoi(parts[1]);
    }

    a_w++;
    if (localStorage) {
        LOcnt[keyword] = to_string(src_cnt) + "-" + to_string(a_w);
    } else {
        if (setup) {
            setupOcnt[key] = to_string(src_cnt) + "-" + to_string(a_w);
        }
    }


    if (localStorage) {
        LOdel[keyword + "-" + to_string(id)] = to_string(a_w);
    } else {
        Bid newKey = getBid(keyword + "-" + to_string(id));
        if (setup) {
            setupOdel[newKey] = to_string(a_w);
        } else {
            Odel->insert(newKey, to_string(a_w));
        }
    }


    prf_type mapKey, mapValue;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int nodeIndex = getNodeOnPath(a_w, height);
    getAESRandomValue(k_w.data(), src_cnt, mapKey.data());
    mapKey = bitwiseXOR(IKey, mapKey);

    *(int*) (&(mapKey.data()[AES_KEY_SIZE - 5])) = nodeIndex;
    mapKey = Utilities::encode(mapKey.data());

    prf_type plaintext;
    memset(plaintext.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), plaintext.data());
    *(int*) (&(plaintext.data()[AES_KEY_SIZE - 5])) = id;
    mapValue = Utilities::encode(plaintext.data(), secretKey);

    server->insert(mapKey, mapValue);
    if (!setup) {
        totalUpdateCommSize = (Ocnt->treeHandler->oram->totalRead + Ocnt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (Ostate->treeHandler->oram->totalRead + Ostate->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (Odel->treeHandler->oram->totalRead + Odel->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (sizeof (prf_type) * 2);
    }
}

void Client::remove(string keyword, int id, bool setup) {
    if (!setup) {
        Ocnt->treeHandler->oram->totalRead = 0;
        Ocnt->treeHandler->oram->totalWrite = 0;
        Ostate->treeHandler->oram->totalRead = 0;
        Ostate->treeHandler->oram->totalWrite = 0;
        Odel->treeHandler->oram->totalRead = 0;
        Odel->treeHandler->oram->totalWrite = 0;
        totalUpdateCommSize = 0;
    }
    Bid key = getBid(keyword);
    string result;
    if (localStorage) {
        result = LOcnt[keyword];
    } else {
        result = setup ? setupOcnt[key] : Ocnt->find(key);
    }
    vector<string> parts = Utilities::splitData(result, "-");
    int src_cnt = stoi(parts[0]);

    string posStr;
    if (localStorage) {
        posStr = LOdel[keyword + "-" + to_string(id)];
    } else {
        Bid newKey = getBid(keyword + "-" + to_string(id));
        posStr = setup ? setupOdel[newKey] : Odel->find(newKey);
    }

    int pos = stoi(posStr);

    vector<int> treeNodes;
    vector<int> treeNodesSiblings;
    vector<int> nodesColor;
    vector<int> finalNodesColor;
    vector<int> siblingsColor;

    for (int i = height; i > 0; i--) {
        int index = getNodeOnPath(pos, i);
        treeNodes.push_back(index);
        if (index % 2 == 0 && index != 0) {
            treeNodesSiblings.push_back(index - 1);
        } else if (index % 2 != 0 && index != 0) {
            treeNodesSiblings.push_back(index + 1);
        }
    }

    for (unsigned int i = 0; i < treeNodes.size(); i++) {
        string colorStr;
        if (localStorage) {
            colorStr = LOstate.count(keyword + "-" + to_string(treeNodes[i])) == 0 ? "" : LOstate[keyword + "-" + to_string(treeNodes[i])];
        } else {
            Bid mapKey = getBid(keyword + "-" + to_string(treeNodes[i]));
            colorStr = setup ? setupOstate[mapKey] : Ostate->find(mapKey);
        }
        int color = stoi(colorStr == "" ? "0" : colorStr);
        nodesColor.push_back(color);
    }

    for (unsigned int i = 0; i < treeNodesSiblings.size(); i++) {
        string colorStr;
        if (localStorage) {
            colorStr = LOstate.count(keyword + "-" + to_string(treeNodesSiblings[i])) == 0 ? "" : LOstate[keyword + "-" + to_string(treeNodesSiblings[i])];
        } else {
            Bid mapKey = getBid(keyword + "-" + to_string(treeNodesSiblings[i]));
            colorStr = setup ? setupOstate[mapKey] : Ostate->find(mapKey);
        }
        int color = stoi(colorStr == "" ? "0" : colorStr);
        siblingsColor.push_back(color);
    }

    finalNodesColor.push_back(2);
    for (unsigned int i = 1; i < nodesColor.size(); i++) {
        if (finalNodesColor[i - 1] == 2 && siblingsColor[i - 1] == 2) {
            finalNodesColor.push_back(2);
        } else {
            finalNodesColor.push_back(0);
        }
    }

    bool insertedInDstate = false;
    for (int i = (treeNodes.size() - 1); i >= 0; i--) {
        if (finalNodesColor[i] != nodesColor[i] && finalNodesColor[i] == 2 && !insertedInDstate) {
            if (localStorage) {
                LOstate[keyword + "-" + to_string(treeNodes[i])] = to_string(finalNodesColor[i]);
            } else {
                Bid mapKey = getBid(keyword + "-" + to_string(treeNodes[i]));
                if (setup) {
                    setupOstate[mapKey] = to_string(finalNodesColor[i]);
                } else {
                    Ostate->insert(mapKey, to_string(finalNodesColor[i]));
                }
            }
            prf_type fkey, fvalue;

            prf_type k_w;
            memset(k_w.data(), 0, AES_KEY_SIZE);
            copy(keyword.begin(), keyword.end(), k_w.data());
            getAESRandomValue(k_w.data(), src_cnt, fkey.data());
            fkey = bitwiseXOR(DKey, fkey);

            *(int*) (&(fkey.data()[AES_KEY_SIZE - 5])) = treeNodes[i];
            fkey = Utilities::encode(fkey.data());
            fvalue = Utilities::encode("0");

            server->insertInDstate(fkey, fvalue);
            insertedInDstate = true;
            break;
        }
    }
    if (!insertedInDstate) {
        prf_type fkey, fvalue;
        fkey = Utilities::encode("dum" + to_string(rand()));
        fvalue = Utilities::encode("dm" + to_string(rand()));
        server->insertInDstate(fkey, fvalue);
    }
    if (!setup) {
        totalUpdateCommSize = (Ocnt->treeHandler->oram->totalRead + Ocnt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (Ostate->treeHandler->oram->totalRead + Ostate->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (Odel->treeHandler->oram->totalRead + Odel->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
                (sizeof (prf_type) * 2);
    }
}

vector<int> Client::search(string keyword) {
    Ocnt->treeHandler->oram->totalRead = 0;
    Ocnt->treeHandler->oram->totalWrite = 0;
    Ostate->treeHandler->oram->totalRead = 0;
    Ostate->treeHandler->oram->totalWrite = 0;
    Odel->treeHandler->oram->totalRead = 0;
    Odel->treeHandler->oram->totalWrite = 0;
    totalSearchCommSize = 0;
    Bid key = getBid(keyword);
    string result;
    if (localStorage) {
        result = LOcnt[keyword];
    } else {
        result = Ocnt->incrementSrcCnt(key);
    }

    vector<string> parts = Utilities::splitData(result, "-");
    int src_cnt = stoi(parts[0]);
    int a_w = stoi(parts[1]);

    prf_type phi, k_w, Ikey, Dkey;

    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    getAESRandomValue(k_w.data(), src_cnt, phi.data());
    Ikey = bitwiseXOR(IKey, phi);
    Dkey = bitwiseXOR(DKey, phi);
    vector<int> deletes;

    totalSearchCommSize += sizeof (IKey) + sizeof (DKey) + sizeof (a_w) + deletes.size() * sizeof (int);

    map<int, prf_type> ciphers = server->search(Ikey, Dkey, a_w, deletes);
    vector<int> finalRes;
    for (auto cipher : ciphers) {
        prf_type plaintext;
        Utilities::decode(cipher.second, plaintext, secretKey);
        int id = *(int*) (&(plaintext.data()[AES_KEY_SIZE - 5]));
        finalRes.push_back(id);
    }

    //------------------------------------------------
    //CleanUP
    //------------------------------------------------

    src_cnt++;
    if (localStorage) {
        LOcnt[keyword] = to_string(src_cnt) + "-" + to_string(a_w);
    }

    vector<prf_type> newIKey, newIvalue, newDKey, newDValue;

    int i = 0;
    for (auto cipher : ciphers) {
        prf_type mapKey, mapValue;
        prf_type k_w;
        memset(k_w.data(), 0, AES_KEY_SIZE);
        copy(keyword.begin(), keyword.end(), k_w.data());
        getAESRandomValue(k_w.data(), src_cnt, mapKey.data());
        mapKey = bitwiseXOR(IKey, mapKey);
        *(int*) (&(mapKey.data()[AES_KEY_SIZE - 5])) = cipher.first;
        mapKey = Utilities::encode(mapKey.data());

        prf_type plaintext;
        memset(plaintext.data(), 0, AES_KEY_SIZE);
        copy(keyword.begin(), keyword.end(), plaintext.data());
        *(int*) (&(plaintext.data()[AES_KEY_SIZE - 5])) = finalRes[i];
        mapValue = Utilities::encode(plaintext.data(), secretKey);

        newIKey.push_back(mapKey);
        newIvalue.push_back(mapValue);
        i++;
    }
    for (auto del : deletes) {
        prf_type k_w, fkey;
        memset(k_w.data(), 0, AES_KEY_SIZE);
        copy(keyword.begin(), keyword.end(), k_w.data());
        getAESRandomValue(k_w.data(), src_cnt, fkey.data());
        fkey = bitwiseXOR(DKey, fkey);
        *(int*) (&(fkey.data()[AES_KEY_SIZE - 5])) = del;
        fkey = Utilities::encode(fkey.data());
        prf_type fvalue = Utilities::encode("0");
        newDKey.push_back(fkey);
        newDValue.push_back(fvalue);
    }
    server->insert(newIKey, newIvalue, newDKey, newDValue);
    totalSearchCommSize += (Ocnt->treeHandler->oram->totalRead + Ocnt->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (Ostate->treeHandler->oram->totalRead + Ostate->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (Odel->treeHandler->oram->totalRead + Odel->treeHandler->oram->totalWrite)*(sizeof (prf_type) + sizeof (int))+
            (sizeof (prf_type) * 2) + newIKey.size() * 2 * sizeof (prf_type) + newDKey.size()*2 * sizeof (prf_type);
    return finalRes;
}

prf_type Client::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

prf_type Client::bitwiseXOR(unsigned char* input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1[i] ^ input2[i];
    }
    return result;
}

void Client::getAESRandomValue(unsigned char* keyword, int src_cnt, unsigned char* result) {
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = src_cnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

Bid Client::getBid(string input) {
    std::array< uint8_t, ID_SIZE> value;
    std::fill(value.begin(), value.end(), 0);
    std::copy(input.begin(), input.end(), value.begin());
    Bid res(value);
    return res;
}

int Client::getNodeOnPath(int leaf, int curDepth) {
    leaf += (int) (pow(2, height) - 1) / 2;
    for (int d = height - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

void Client::endSetup() {
    Ocnt->setupInsert(setupOcnt);
    Odel->setupInsert(setupOdel);
    for (int i = 0; i < 1000; i++) {
        cout << i << "/1000" << endl;
        Bid testBid;
        testBid.setValue(1);
        Ocnt->insert(testBid, "TEST");
    }
    for (int i = 0; i < 1000; i++) {
        cout << i << "/1000" << endl;
        Bid testBid;
        testBid.setValue(1);
        Odel->insert(testBid, "TEST");
    }
    Ostate->setupInsert(setupOstate);
    for (int i = 0; i < 1000; i++) {
        cout << i << "/1000" << endl;
        Bid testBid;
        testBid.setValue(1);
        Ostate->insert(testBid, "TEST");
    }
}

double Client::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Client::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}
