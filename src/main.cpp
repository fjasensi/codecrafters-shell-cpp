#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <algorithm>

bool findCommand(const std::vector<std::string>& v, std::string command) {
    auto it = std::find(v.begin(), v.end(), command);

    return (it != v.end()) ? true : false;
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
          if (findCommand(builtinCommands, command.substr(5))) {
              std::cout << command.substr(5) << " is a shell builtin" << std::endl;
          } else {
              std::cout << command.substr(5) << ": not found" << std::endl;
          }
      }
      else if (command != "exit") {
          std::cout << std::format("{}: command not found", command) << std::endl;
      }
  } while (command != "exit");

  return 0;
}
