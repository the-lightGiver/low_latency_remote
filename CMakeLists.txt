cmake_minimum_required(VERSION 3.16)
project(stage1_only CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server server/main.cpp)
add_executable(client client/main.cpp)