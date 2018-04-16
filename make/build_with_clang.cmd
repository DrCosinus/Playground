@echo off
mkdir bin\windows\clang\release 2> nul
mkdir bin\windows\clang\debug 2> nul
rem -Wno-gnu-string-literal-operator-template
clang -Xclang -flto-visibility-public-std -Wall -Werror -pedantic -Wextra -std=c++17 %*
