#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
    vector<string> msg{"hello", "world"};

    for (const string word : msg) {
        cout << word << " ";
    }

    // cout << "Hello, World!" << endl;
    return 0;
}