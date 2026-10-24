#include <iostream>
#include <sstream>
#include <string>
#include <format>
#include <filesystem>
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
    return std::filesystem::is_regular_file(path) && access(path.c_str(), X_OK) == 0;
}

void locate_executable_files(std::string command) {
    // The PATH environment variable specifies a list of directories where the shell should look for executable programs.
    const char* path = std::getenv("PATH");
    std::stringstream ss(path);
    std::string single_path;
    std::string full_path;
    bool found = false;
    
    while (std::getline(ss, single_path, PATH_LIST_SEPARATOR)) {
        full_path = single_path + "/" + command;

        if (file_exists(full_path)) {
            found = true;

            if (has_execute_permissions(full_path)) {
                std::cout << command << " is " << full_path << std::endl;
                break;
            }
        }
    }

    if (!found) std::cout << command << ": not found" << std::endl;
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string command;
  std::vector<std::string> builtinCommands {"exit", "echo", "type"};

  do {
      std::cout << "$ ";
    
      std::getline(std::cin, command);

      if (command.substr(0, 5) == "echo ") {
          std::cout << command.substr(5) << std::endl;
      }
      else if (command.substr(0, 5) == "type ") {
          if (find_command(builtinCommands, command.substr(5))) {
              std::cout << command.substr(5) << " is a shell builtin" << std::endl;
          } else {
              locate_executable_files(command.substr(5));
          }
      }
      else if (command != "exit") {
          std::cout << std::format("{}: command not found", command) << std::endl;
      }
  } while (command != "exit");

  return 0;
}
