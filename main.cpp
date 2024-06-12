#include <iostream>
#include "ShellHandel.h"
using namespace std;
int main() {
    auto *shellHandel = new ShellHandel();
    string input;
    while(true){
        cout << "saleh<Shell>"<< endl;
        std::getline( std::cin >> std::ws, input);
        if (input == "exit")
            break;
        vector<string> inputs = shellHandel->TokenInput(input);
        if(shellHandel->ExcuteCommand(inputs) == -1 ){
            cerr << "error in excute the command "<<endl;
        }
    }
    if (input == "exit") {
        return 1;
    }
    delete shellHandel;
    return 0;
}
