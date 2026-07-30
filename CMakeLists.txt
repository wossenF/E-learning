cmake_minimum_required(VERSION 3.10)
project(temene_cpp_substrate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB CPP_SOURCES cpp_src/*.cpp)
add_library(temene_substrate STATIC ${CPP_SOURCES})

add_executable(temene_example examples/example_main.cpp)
target_link_libraries(temene_example PRIVATE temene_substrate)
