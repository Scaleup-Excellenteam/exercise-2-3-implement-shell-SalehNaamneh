#ifndef SHELLBASH_SHELLHANDEL_H
#define SHELLBASH_SHELLHANDEL_H
#include "string"
#include "vector"
struct BackgroundStatus {
    pid_t pid;
    std::string command;
};
class ShellHandel{

public:
    ShellHandel();
    std::vector<std::string> TokenInput(const std::string& input);
    int ExcuteCommand(std::vector<std::string> inputs);
private:
    void SplitPath();
    std::string SearchCommand(const std::string& command);
    std::vector<std::string> paths;
    void MyJobs();
    std::vector<BackgroundStatus> BackGroundProcces;
};
#endif //SHELLBASH_SHELLHANDEL_H
