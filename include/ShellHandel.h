#ifndef SHELLBASH_SHELLHANDEL_H
#define SHELLBASH_SHELLHANDEL_H
#include <string>
#include <vector>
#include <sys/types.h>

struct BackgroundStatus {
    pid_t pid;
    std::string command;
};

class ShellHandel {
public:
    ShellHandel();
    std::vector<std::string> TokenInput(const std::string& input);
    int ExecuteCommand(const std::vector<std::string>& inputs);
    void MyJobs();
private:
    void SplitPath();
    std::string SearchCommand(const std::string& command);
    std::vector<std::string> paths;
    std::vector<BackgroundStatus> BackGroundProcesses;
    int ExecuteSingleCommand(const std::vector<std::string>& inputs);
    int ExecutePipedCommands(const std::vector<std::vector<std::string>>& commands);
};
#endif //SHELLBASH_SHELLHANDEL_H
