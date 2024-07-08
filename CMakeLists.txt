cmake_minimum_required(VERSION 3.2)
project(BashShell)

set(CMAKE_CXX_STANDARD 17)

# Set the build type to Debug if it's not set from the command line
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_executable(BashShell main.cpp
        src/ShellHandel.cpp
        include/ShellHandel.h)

