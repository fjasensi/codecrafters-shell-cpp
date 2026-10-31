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

#define QUOTE '\''
#define DOUBLE_QUOTE '\"'
#define BACK_SLASH '\\'

namespace IO {
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
        
        while (std::getline(ss, single_path, PATH_LIST_SEPARATOR)) {
            full_path = single_path + "/" + command;
    
            if (file_exists(full_path)) {
                if (has_execute_permissions(full_path)) {
                    result = full_path;
                    break;
                }
            }
        }
    
        return result;
    }

    void run_program(std::vector<std::string> full_command, std::optional<std::string> run_path) {
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
        }
        else {
            perror("fork");
        }
    }
}

namespace Parsing {
    struct MainCommand {
        std::string name;
        int value;
    };
    
    std::vector<std::string> split(const std::string& str, char separator) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string token;
    
        while (std::getline(ss, token, separator)) {
            if (token != "") result.push_back(token);
        }
    
        return result;
    }

    bool contains_quotes(const std::string& text) {
        return (text.contains(QUOTE) || text.contains(DOUBLE_QUOTE));
    }
    
    bool is_quote(const char& c) {
        return (c == QUOTE || c == DOUBLE_QUOTE);
    }
    
    void add_char(std::string& str, char c) {   
        str += c;
    }

    bool find_command(const std::vector<std::string>& v, std::string command) {
        auto it = std::find(v.begin(), v.end(), command);
    
        return (it != v.end()) ? true : false;
    }

    bool find_vector_char(const std::vector<char>& v, char c) {
        auto it = std::find(v.begin(), v.end(), c);
    
        return (it != v.end()) ? true : false;
    }

    std::vector<std::string> tokenize_arguments(const std::string& params, std::string command, int original_length) {
        std::vector<std::string> args;
    
        // No arguments
        if (params.size() <= original_length + 1) {
            return args;
        }
    
        std::string rest = params.substr(original_length + 1);
        bool inside_quotes = false;
        bool prev_is_backslash = false;
        std::string token = "";
        char prev_quote = 'c';
    
        for(auto& param : rest) {
            if (prev_is_backslash) {
                add_char(token, param);
                prev_is_backslash = false;
                continue;
            }

            if (param == BACK_SLASH &&
                (!inside_quotes || prev_quote == DOUBLE_QUOTE)) {
                    
                add_char(token, param);
                prev_is_backslash = true;
                continue;
            }
    
            if (!prev_is_backslash) {
                if (Parsing::is_quote(param)) {
                    if (!inside_quotes) {
                        inside_quotes = true;
                        prev_quote = param;
                    }
                    else if (param == prev_quote) {
                        inside_quotes = false;
                        prev_quote = 'c';
                    }

                    Parsing::add_char(token, param);
                }
                else if (param == ' ' && !inside_quotes) {
                    if (token != "") args.push_back(token);
                    
                    token = "";
                }
                else {
                    Parsing::add_char(token, param);
                }
            }
            else {
                Parsing::add_char(token, param);
            }
    
            if (param != BACK_SLASH) prev_is_backslash = false;
        }
    
        if (token != "") args.push_back(token);
    
        return args;
    }

    std::vector<std::string> get_final_arguments(const std::vector<std::string>& args) {
        std::vector<std::string> result;
        std::vector<char> back_slash_scapes {'\\', '\"'};
    
        for (const auto& argument : args) {
            std::string result_argument;
            bool inside_single_quotes = false;
            bool inside_double_quotes = false;
            bool prev_is_backslash = false;

            for (char c : argument) {
                if (prev_is_backslash) {
                    if (inside_double_quotes && !find_vector_char(back_slash_scapes, c)) {
                        add_char(result_argument, BACK_SLASH);
                    }
    
                    add_char(result_argument, c);
                    prev_is_backslash = false;
                    continue;
                }
    
                if (inside_single_quotes) {
                    if (c == QUOTE) {
                        inside_single_quotes = false;
                    } else {
                        add_char(result_argument, c);
                    }
                    
                    continue;
                }
    
                if (c == BACK_SLASH) {
                    prev_is_backslash = true;
                    continue;
                }
    
                if (c == DOUBLE_QUOTE) {
                    inside_double_quotes = !inside_double_quotes;
                    continue;
                }
    
                if (c == QUOTE && !inside_double_quotes) {
                    inside_single_quotes = true;
                    continue;
                }
    
                // Normal text
                add_char(result_argument, c);
            }
    
            if (prev_is_backslash) {
                add_char(result_argument, BACK_SLASH);
            }
    
            result.push_back(result_argument);
        }
    
        return result;
    }

    MainCommand get_command(const std::string& line) {
        std::string command = "";
        std::vector<char> back_slash_scapes {'\\', '\"'};
        bool inside_single_quotes = false;
        bool inside_double_quotes = false;
        bool prev_is_backslash = false;
        int length = 0;

        for (const auto& c : line) {
            if (c == ' ' && !inside_single_quotes && !inside_double_quotes && !prev_is_backslash) {
                break;
            }

            length += 1;

            if (prev_is_backslash) {
                if (inside_double_quotes && !find_vector_char(back_slash_scapes, c)) {
                    add_char(command, BACK_SLASH);
                }

                add_char(command, c);
                prev_is_backslash = false;
                continue;
            }

            if (inside_single_quotes) {
                if (c == QUOTE) {
                    inside_single_quotes = false;
                } else {
                    add_char(command, c);
                }

                continue;
            }

            if (c == BACK_SLASH) {
                prev_is_backslash = true;
                continue;
            }

            if (c == DOUBLE_QUOTE) {
                inside_double_quotes = !inside_double_quotes;
                continue;
            }

            if (c == QUOTE && !inside_double_quotes) {
                inside_single_quotes = true;
                continue;
            }

            add_char(command, c);
        }

        if (prev_is_backslash) {
            add_char(command, BACK_SLASH);
        }

        return {command, length};
    }
}

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::string command;
  std::vector<std::string> builtinCommands {"exit", "echo", "type", "pwd", "cd"};
  std::optional<std::string> run_path = std::nullopt;

  do {
      std::cout << "$ ";
    
      std::getline(std::cin, command);

      // TODO: Old method, delete and replace full_command and single_command
      std::vector<std::string> full_command = Parsing::split(command, ' ');

      auto [single_command, length] = Parsing::get_command(command);

      std::vector<std::string> arguments = Parsing::tokenize_arguments(command, single_command, length);
      arguments = Parsing::get_final_arguments(arguments);

      if (single_command == "exit") return 0;

      if (single_command == "echo") {
          for (const auto& argument : arguments) {
              std::cout << argument << " ";
          }

          std::cout << std::endl;
      }
      else if (single_command == "type") {
          if (Parsing::find_command(builtinCommands, command.substr(5))) {
              std::cout << command.substr(5) << " is a shell builtin" << std::endl;
          } else {
              run_path = IO::locate_executable_files(command.substr(5));

              if (run_path.has_value()) {
                  std::cout << command.substr(5) << " is " << run_path.value() << std::endl;
              } else {
                  std::cout << command.substr(5) << ": not found" << std::endl;
              }
          }
      }
      else if (single_command == "pwd") {
          std::cout << std::filesystem::current_path().string() << std::endl;
      }
      else if (single_command == "cd") {
          if (full_command.size() > 1) {
              std::string path_to_go = full_command[1];

              if (path_to_go == "~") {
                  const char* home = std::getenv("HOME");
                  path_to_go = home;
              }
              
            if (std::filesystem::exists(path_to_go)) {
                if (std::filesystem::is_directory(path_to_go)) {
                    std::filesystem::current_path(path_to_go);
                }
            }
            else {
                std::cout << "cd: " << path_to_go << ": No such file or directory" << std::endl;
            }
          }
      }
      else {
          run_path = IO::locate_executable_files(single_command);

          if (run_path.has_value()) {
              arguments.insert(arguments.begin(), single_command);
              
              IO::run_program(arguments, run_path);
          } else {
              std::cout << std::format("{}: command not found", command) << std::endl;
          }
      }
  } while (command != "exit");

  return 0;
}
