#include "qos/Client.h"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {

    Server server(false, 100);
    Client client(&server, 100,10);

    client.insert("test", 5, false);
    client.insert("test", 6, false);
    client.insert("test", 7, false);
    client.remove("test", 6, false);
    vector<int> res = client.search("test");

    for (auto item : res) {
        cout << item << endl;
    }
    
    return 0;
}
