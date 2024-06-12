#include "ShellHandel.h"
#include "string"
#include "vector"
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <cstring>
using namespace std;

ShellHandel::ShellHandel() {
}
/*
 * this func to split the input that has been received from the user
 *
 * Parameters
 *
 * input : is the input form the user
 *
 * return
 * vector<string> that's contain all the input but the input is cut for every white space
 */
vector<string> ShellHandel::TokenInput(const std::string& input) {
    vector<string> tokens;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}
/*
 * this func to split the PATH linux environment variable that contains paths each path is separated by :
 *
 * result:
 * the func will  fill  a vector<string > called paths with every path in the path environment variable
 */
void ShellHandel::SplitPath() {
    const char* path = getenv("PATH");
    if (path == nullptr) {
        cerr << "Failed to retrieve PATH variable" << endl;
        return;
    }

    string subPath;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p != ':') {
            subPath += *p;
        } else {
            paths.push_back(subPath);
            subPath.clear();
        }
    }
    paths.push_back(subPath);
}
/*
 * this func is for searching for the command that the user has entered every path in the path environment variable
 *
 * return :
 *
 * return a string it is a path were the command has been found
 * an empty string if not found
 */

string ShellHandel::SearchCommand(const std::string& command) {
    this->SplitPath();
    struct dirent* entity;
    for (const string& subPath : paths) {
        DIR* dir = opendir(subPath.c_str());
        if (dir != nullptr) {
            while ((entity = readdir(dir)) != nullptr) {
                if (strcmp(entity->d_name, command.c_str()) == 0) {
                    closedir(dir);
                    return subPath + "/" + command;
                }
            }
            closedir(dir);
        }
    }
    return "";
}


/*
 * this func is for excue the command using the excve func
 *
 * return :
 * positive number if succsees negative if fail
 */
int ShellHandel::ExcuteCommand(std::vector<std::string> inputs) {

    if (inputs[0] == "myjobs")
    {
        this->MyJobs();
        return 0;
    }
    std::string path = this->SearchCommand(inputs[0]);
    bool runInBackground = false;
    if (path.empty()) {
        cerr << "The command can't be found: " << inputs[0] << endl;
        return -1;
    }
    if (!inputs.empty() && inputs.back() == "&")
    {
        runInBackground = true;
        inputs.pop_back();
    }
    // Fork a new process
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Fork has failed" << std::endl;
        return -1;
    }

    // Child process
    if (pid == 0) {
        // Convert vector of strings to array of C-style strings
        std::vector<char*> args;
        for (const std::string& arg : inputs) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr); // Null-terminate the array

        // Execute the command in the child process
        execv(path.c_str(), args.data());

        // execv only returns if an error occurs
        std::cerr << "execv failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else {
        if (!runInBackground) {
            int status;
            waitpid(pid, &status, 0); // Wait for the child process to complete
            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        }
        else {
            BackgroundStatus status{pid,inputs[0]};
            BackGroundProcces.push_back(status);
            cout << "the command is runing in the background with pid " << pid << endl ;
        }
    }
}
/*
 * this func is for handle the background proccess and for displaying the status of them
 */
void ShellHandel::MyJobs()
{
    if (BackGroundProcces.empty())
    {
        cout << "there no background procceses " << endl;
        return;
    }
    for (vector<BackgroundStatus>::iterator it = BackGroundProcces.begin(); it!=BackGroundProcces.end();)
    {
        int stauts;
        pid_t result = waitpid(it->pid,&stauts,WNOHANG);
        if (result==0)
        {
            cout << "The command " << it->command << "is still runing with pid " << it->pid << endl ;
        }
        else if (result == -1 )
        {
            cout << "The has been error in the command " << it->command << "PID" << it->pid << endl;
            BackGroundProcces.erase(it);
        }
        else
        {
            cout << "the command " << it ->command << " has  been terminated pid "  << it->pid<<endl;
            BackGroundProcces.erase(it);
        }
    }
}

