cmake_minimum_required(VERSION 3.22)
project(Zadanie02_client)

set(CMAKE_CXX_STANDARD 20)

add_executable(Zadanie02_client Buffer.cpp ClientParameters.cpp Client.cpp)
