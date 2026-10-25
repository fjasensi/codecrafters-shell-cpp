#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <format>
#include <filesystem>
#include <optional>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
constexpr char PATH_LIST_SEPARATOR = ';';
#define ACCESS _access
#define X_OK 0

#else
#include <unistd.h>
#define ACCESS access
constexpr char PATH_LIST_SEPARATOR = ':';
#endif

bool find_command(const std::vector<std::string>& v, std::string command) {
    auto it = std::find(v.begin(), v.end(), command);

    return (it != v.end()) ? true : false;
}

bool file_exists(std::string path) {
    return (std::filesystem::exists(path)) ? true : false;
}

bool has_execute_permissions(std::string path) {
    return std::filesystem::is_regular_file(path) && ACCESS(path.c_str(), X_OK) == 0;
}

std::optional<std::string> locate_executable_files(std::string command) {
    // The PATH environment variable specifies a list of directories where the shell should look for executable programs.
    const char* path = std::getenv("PATH");
    std::optional<std::string> result = std::nullopt;
    std::stringstream ss(path);
    std::string single_path;
    std::string full_path;
    bool found = false;
    
    while (std::getline(ss, single_path, PATH_LIST_SEPARATOR)) {
        full_path = single_path + "/" + command;

        if (file_exists(full_path)) {
            found = true;

            if (has_execute_permissions(full_path)) {
                result = full_path;
                break;
            }
        }
    }

    return result;
}

std::vector<std::string> split(const std::string& str, char separator) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, separator)) {
        result.push_back(token);
    }

    return result;
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string command;
  std::vector<std::string> builtinCommands {"exit", "echo", "type"};
  std::optional<std::string> run_path = std::nullopt;

  do {
      std::cout << "$ ";
    
      std::getline(std::cin, command);

      std::vector<std::string> full_command = split(command, ' ');
      std::string single_command = full_command[0];

      if (single_command == "exit") return 0;

      if (single_command == "echo") {
          std::cout << command.substr(5) << std::endl;
      }
      else if (single_command == "type") {
          if (find_command(builtinCommands, command.substr(5))) {
              std::cout << command.substr(5) << " is a shell builtin" << std::endl;
          } else {
              run_path = locate_executable_files(command.substr(5));

              if (run_path.has_value()) {
                  std::cout << command.substr(5) << " is " << run_path.value() << std::endl;
              } else {
                  std::cout << command.substr(5) << ": not found" << std::endl;
              }
          }
      }
      else {
          run_path = locate_executable_files(single_command);

          if (run_path.has_value()) {
              std::vector<char*> argv;
              
              for (auto& arg : full_command) {
                  argv.push_back(arg.data());
              }

              argv.push_back(nullptr);

              pid_t pid = fork();

              // Child process
              if (pid == 0) {
                  execv(run_path.value().c_str(), argv.data());

                  // only come by here if execv fails
                  perror("execv");
                  _exit(127);
              }
              else if (pid > 0) { // father process
                  int status;
                  waitpid(pid, &status, 0);
                  std::cout << std::endl;
              }
              else {
                  perror("fork");
              }
          } else {
              std::cout << std::format("{}: command not found", command) << std::endl;
          }
      }
  } while (command != "exit");

  return 0;
}
