cmake_minimum_required(VERSION 3.22)

project(
    toolbox
    VERSION 1.2.0
    DESCRIPTION "A collection of generic tools"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
add_subdirectory(tests)
