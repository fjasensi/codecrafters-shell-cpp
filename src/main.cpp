#include <iostream>
#include <string>
#include <format>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string command;

  do {
      std::cout << "$ ";
    
      std::getline(std::cin, command);

      if (command.substr(0, 5) == "echo ") {
          std::cout << command.substr(5) << std::endl;
      }
      else if (command != "exit") {
          std::cout << std::format("{}: command not found", command) << std::endl;
      }
  } while (command != "exit");

  return 0;
}
