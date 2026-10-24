#include <iostream>
#include <string>
#include <format>

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  do {
      std::cout << "$ ";
    
      std::string command;
      std::cin >> command;
    
      std::cout << std::format("{}: command not found", command);
  } while (1 == 1);

  return 0;
}
