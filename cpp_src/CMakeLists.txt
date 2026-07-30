cmake_minimum_required(VERSION 3.5)
project(temene_inmem_db)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC "${CMAKE_SOURCE_DIR}/*.cpp")

add_executable(cli_db ${SRC})
