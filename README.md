# C++ Shell

[![progress-banner](https://backend.codecrafters.io/progress/shell/ee090455-2b76-4dd3-bd65-55b1bcc960b2)](https://app.codecrafters.io/users/fjasensi?r=2qF)

A small Unix-like shell written in C++23 for the
[CodeCrafters "Build Your Own Shell" challenge](https://app.codecrafters.io/courses/shell/overview).

This is a learning project focused on command parsing, builtin commands, executable
lookup, and process creation. The implementation grows as I work through the
challenge and refactor what I learn along the way.

## Features

- Interactive read-eval-print loop with a `$ ` prompt.
- Builtin commands: `exit`, `echo`, `type`, `pwd`, and `cd`.
- Executable lookup through `PATH`, including checks for executable permissions.
- External program execution using `fork`, `execv`, and `waitpid`.
- Argument parsing for single and double quotes, adjacent quoted fragments, and
  backslash escapes. Context-specific escape rules are still being refined.

| Builtin | Behavior |
| --- | --- |
| `exit` | Exit the shell. |
| `echo` | Print the parsed arguments. |
| `type` | Identify a builtin or locate an executable in `PATH`. |
| `pwd` | Print the current working directory. |
| `cd` | Change the current directory, including `cd ~` for the home directory. |

## Build and run

Requirements:

- A POSIX environment, such as Linux or macOS.
- A compiler and standard library supporting C++23.
- CMake 3.13 or later.
- The Readline library available to the linker; the CMake target links against
  `readline`.

From the repository root:

```sh
./your_program.sh
```

The script configures CMake, builds the executable in `build/`, and starts the
shell. If `VCPKG_ROOT` is set, it uses the vcpkg toolchain from that directory;
otherwise, it uses the default toolchain.

To build and run manually with the default toolchain:

```sh
cmake -B build -S .
cmake --build build
./build/shell
```

## Try it

Once the shell is running, enter these commands at its `$ ` prompt:

```sh
echo hello shell
echo 'hello     shell'
echo 'hello''world'
echo "shell's" test
echo hello\ world
type pwd
pwd
ls
cd ..
exit
```

External programs such as `ls` and `cat` must be available in `PATH`. To read an
existing file whose name contains spaces, use `cat 'file with spaces.txt'`.

## CodeCrafters checks

With the CodeCrafters CLI configured for this repository, run the current stage:

```sh
codecrafters test
```

To include all previous stages as well:

```sh
codecrafters test --previous
```

To submit the solution:

```sh
codecrafters submit
```

## Current scope

This is an implementation in progress, with partial shell syntax support.