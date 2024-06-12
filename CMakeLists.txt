cmake_minimum_required(VERSION 3.28)
project(BashShell)

set(CMAKE_CXX_STANDARD 17)

add_executable(BashShell main.cpp
        ShellHandel.cpp
        ShellHandel.h)
