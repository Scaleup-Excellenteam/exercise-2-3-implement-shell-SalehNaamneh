#include  "../include/ShellHandel.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <cstring>
using namespace std;

// Constructor initializes the PATH environment variable by calling SplitPath
ShellHandel::ShellHandel() {
    SplitPath();
}

// Tokenizes the input string into individual commands and arguments
vector<string> ShellHandel::TokenInput(const std::string& input) {
    vector<string> tokens;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Splits the PATH environment variable into individual directories
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

// Searches for the command in the directories listed in paths
string ShellHandel::SearchCommand(const std::string& command) {
    for (const string& subPath : paths) {
        DIR* dir = opendir(subPath.c_str());
        if (dir != nullptr) {
            struct dirent* entity;
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

// Determines if the command involves pipes and calls the appropriate execution function
int ShellHandel::ExecuteCommand(const std::vector<std::string>& inputs) {
    vector<vector<string>> commands;
    vector<string> current_command;

    for (const auto& input : inputs) {
        if (input == "|") {
            commands.push_back(current_command);
            current_command.clear();
        } else {
            current_command.push_back(input);
        }
    }
    commands.push_back(current_command);

    if (commands.size() > 1) {
        return ExecutePipedCommands(commands);
    } else {
        return ExecuteSingleCommand(commands[0]);
    }
}

// Executes a single command with optional input/output redirection and background execution
int ShellHandel::ExecuteSingleCommand(const std::vector<std::string>& inputs) {
    if (inputs.empty()) {
        return -1;
       }
	
   // Handle 'cd' command separately
    if (inputs[0] == "cd") {
        if (inputs.size() < 2) {
            cerr << "cd: missing argument" << endl;
            return -1;
        }
        if (chdir(inputs[1].c_str()) != 0) {
            perror("cd failed");
            return -1;
        }
        return 0;
    }
    bool runInBackground = false;
    int input_fd = -1, output_fd = -1;

    vector<string> command;
    for (size_t i = 0; i < inputs.size(); ++i) {
        if (inputs[i] == "<") {
            input_fd = open(inputs[i + 1].c_str(), O_RDONLY);
            if (input_fd == -1) {
                cerr << "Failed to open input file: " << inputs[i + 1] << endl;
                return -1;
            }
            ++i;
        } else if (inputs[i] == ">") {
            output_fd = open(inputs[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                cerr << "Failed to open output file: " << inputs[i + 1] << endl;
                return -1;
            }
            ++i;
        } else if (inputs[i] == "&") {
            runInBackground = true;
        } else {
            command.push_back(inputs[i]);
        }
    }

    if (command.empty()) {
        return -1;
    }

    std::string path = SearchCommand(command[0]);
    if (path.empty()) {
        cerr << "The command can't be found: " << command[0] << endl;
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Fork failed" << endl;
        return -1;
    }

    if (pid == 0) { // Child process
        if (input_fd != -1) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        vector<char*> args;
        for (const auto& arg : command) {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);

        execv(path.c_str(), args.data());
        cerr << "execv failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    } else { // Parent process
        if (!runInBackground) {
            int status;
            waitpid(pid, &status, 0);
            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        } else {
            BackgroundStatus status{pid, command[0]};
            BackGroundProcesses.push_back(status);
            cout << "Command is running in the background with pid " << pid << endl;
        }
    }
    return 0;
}

// Executes a series of piped commands
int ShellHandel::ExecutePipedCommands(const std::vector<std::vector<std::string>>& commands) {
    int num_pipes = commands.size() - 1;
    int pipe_fds[2 * num_pipes];

    for (int i = 0; i < num_pipes; ++i) {
        if (pipe(pipe_fds + i * 2) == -1) {
            cerr << "Pipe failed" << endl;
            return -1;
        }
    }

    for (size_t i = 0; i < commands.size(); ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Fork failed" << endl;
            return -1;
        }

        if (pid == 0) { // Child process
            if (i > 0) {
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < commands.size() - 1) {
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * num_pipes; ++j) {
                close(pipe_fds[j]);
            }

            vector<char*> args;
            for (const auto& arg : commands[i]) {
                args.push_back(const_cast<char*>(arg.c_str()));
            }
            args.push_back(nullptr);

            std::string path = SearchCommand(commands[i][0]);
            if (path.empty()) {
                cerr << "The command can't be found: " << commands[i][0] << endl;
                exit(EXIT_FAILURE);
            }

            execv(path.c_str(), args.data());
            cerr << "execv failed: " << strerror(errno) << endl;
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * num_pipes; ++i) {
        close(pipe_fds[i]);
    }

    for (size_t i = 0; i < commands.size(); ++i) {
        int status;
        wait(nullptr);
    }

    return 0;
}

// Lists all background processes
void ShellHandel::MyJobs() {
    if (BackGroundProcesses.empty()) {
        cout << "There are no background processes" << endl;
        return;
    }
    for (auto it = BackGroundProcesses.begin(); it != BackGroundProcesses.end();) {
        int status;
        pid_t result = waitpid(it->pid, &status, WNOHANG);
        if (result == 0) {
            cout << "The command " << it->command << " is still running with pid " << it->pid << endl;
            ++it;
        } else if (result == -1) {
            cout << "There was an error with the command " << it->command << " PID " << it->pid << endl;
            it = BackGroundProcesses.erase(it);
        } else {
            cout << "The command " << it->command << " has terminated with pid " << it->pid << endl;
            it = BackGroundProcesses.erase(it);
        }
    }
}

