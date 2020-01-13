#include "static/Amortized.h"
using namespace std;

int main(int argc, char** argv) {

    Amortized client(false, 10, 100);

    client.update(OP::INS, "test", 5, false);
    client.update(OP::INS, "test", 6, false);
    client.update(OP::INS, "test", 7, false);
    client.update(OP::DEL, "test", 6, false);
    vector<int> res = client.search("test");

    for (auto item : res) {
        cout << item << endl;
    }
    
    return 0;
}
