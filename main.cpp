#include <iostream>
#include "include/ShellHandel.h"
using namespace std;

int main() {
    auto* shellHandel = new ShellHandel();
    string input;
    while (true) {
        cout << "saleh<Shell> ";
        getline(cin, input);
        if (input == "exit")
            break;
        vector<string> inputs = shellHandel->TokenInput(input);
        if (!inputs.empty() && inputs[0] == "myjobs") {
            shellHandel->MyJobs();
        } else {
            if (shellHandel->ExecuteCommand(inputs) == -1) {
                cerr << "Error in executing the command" << endl;
            }
        }
    }
    delete shellHandel;
    return 0;
}
