@echo off
mkdir bin\windows\clang\release 2> nul
mkdir bin\windows\clang\debug 2> nul
clang magic.cpp -Xclang -flto-visibility-public-std -Wno-gnu-string-literal-operator-template -std=c++17 %*
