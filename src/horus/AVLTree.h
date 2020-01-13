#ifndef AVLTREE_H
#define AVLTREE_H
#include <iostream>
#include "ORAM.hpp"
#include "RAMStore.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <array>
#include <memory>
#include <type_traits>
#include <iomanip>
#include <bits/stdc++.h>
#include "Bid.h"
#include <random>
using namespace std;

class AVLTree {
private:

    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int height(Bid N, int& leaf);
    int max(int a, int b);
    Node* newNode(Bid key, string value);
    Node* rightRotate(Node* y);
    Node* leftRotate(Node* x);
    int getBalance(Node* N);
    int RandomPath();
    int sortedArrayToBST(int start, int end, int& pos, Bid& node);
    vector<Node*> setupNodes;
    int setupProgress = 0;

public:
    ORAM *oram;
    AVLTree(int maxSize, bytes<Key> key);
    virtual ~AVLTree();
    Bid insert(Bid rootKey, int& pos, Bid key, string value);
    Node* search(Node* head, Bid key);
    void batchSearch(Node* head, vector<Bid> keys, vector<Node*>* results);
    void printTree(Node* root, int indent);
    void startOperation(bool batchWrite = false);
    void finishOperation(bool find, Bid& rootKey, int& rootPos);
    void setupInsert(Bid& rootKey, int& rootPos, map<Bid, string> pairs);
    string incrementSrcCnt(Node* head, Bid key);
    string incrementUpdtCnt(Node* head, Bid key);
    string decrementUpdtCnt(Node* head, Bid key);
};

#endif /* AVLTREE_H */




