#include "Server.h"
#include "OMAP.h"
#include "../utils/Utilities.h"

Server::~Server() {
}

Server::Server(bool useHDD, int N) {
    this->useRocksDB = useHDD;
    this->N = N;
    this->height = (int) ceil(log2(this->N)) + 1; //root is level 1
}

void Server::insert(prf_type key, prf_type value) {
    I[key] = value;
}

void Server::insert(vector<prf_type> keys, vector<prf_type> values, vector<prf_type> Dkeys, vector<prf_type> Dvalues) {
    for (unsigned int i = 0; i < keys.size(); i++) {
        I[keys[i]] = values[i];
    }
    for (unsigned int i = 0; i < Dkeys.size(); i++) {
        Dstate[Dkeys[i]] = Dvalues[i];
    }
}

void Server::insertInDstate(prf_type key, prf_type value) {
    Dstate[key] = value;
}

map<int, prf_type> Server::search(prf_type Ikey, prf_type Dkey, int a_w, vector<int>& deletes) {
    vector<int> indexes;
    vector<int> t_i = findCoveringSet(a_w);
    for (unsigned int i = 0; i < t_i.size(); i++) {
        int currentIndex = t_i[i];
        recursiveSearch(Dkey, currentIndex, indexes, deletes);
    }
    map<int, prf_type> result;
    for (unsigned int i = 0; i < indexes.size(); i++) {
        prf_type tmpKey = Ikey;
        *(int*) (&(tmpKey.data()[AES_KEY_SIZE - 5])) = indexes[i];
        prf_type fkey = Utilities::encode(tmpKey.data());
        if (I.count(fkey) != 0) {
            result[indexes[i]] = I[fkey];
            I.erase(fkey);
        } else {
            cout << "Data is already removed!" << endl;
        }
    }
    return result;
}

void Server::recursiveSearch(prf_type key, int index, vector<int>& result, vector<int>& deletes) {

    prf_type fkey = key;
    *(int*) (&(fkey.data()[AES_KEY_SIZE - 5])) = index;
    //TODO:The encryption key should be different than I
    fkey = Utilities::encode(fkey.data());
    if (Dstate.count(fkey) == 0) {
        if (index >= (pow(2, height - 1) - 1)) {
            result.push_back(index);
        } else {
            int leftIndex = index * 2 + 1;
            int rightIndex = index * 2 + 2;
            recursiveSearch(key, leftIndex, result, deletes);
            recursiveSearch(key, rightIndex, result, deletes);
        }
    } else {
        deletes.push_back(index);
        Dstate.erase(fkey);
    }
}

vector<int> Server::findCoveringSet(int a_w) {
    vector<int> coverSet;
    int totIndex = 0;
    while (a_w >= 0) {
        int lastTreeHeight = a_w == 0 ? 1 : (int) floor(log2(a_w + 1)) + 1;
        int lastIndex = a_w == 0 ? 0 : pow(2, (int) floor(log2(a_w + 1))) - 1;
        int index = getNodeOnPath(lastIndex + totIndex, height - lastTreeHeight + 1);
        coverSet.push_back(index);
        a_w -= (lastIndex + 1);
        totIndex += lastIndex + 1;
    }
    return coverSet;
}

int Server::getNodeOnPath(int leaf, int curDepth) {
    leaf += (int) (pow(2, height) - 1) / 2;
    for (int d = height - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}