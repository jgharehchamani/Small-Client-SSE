#include "horus/Horus.h"
#include "utils/Utilities.h"
using namespace std;

int main(int argc, char** argv) {
    Horus horus(false, 100);

    horus.insert("test", 5);
    horus.insert("test", 6);
    horus.insert("test", 7);
    horus.insert("test", 8);
    horus.remove("test", 7);
    vector<int> res = horus.search("test");

    for (auto item : res) {
        cout << item << endl;
    }
    
    return 0;
}
