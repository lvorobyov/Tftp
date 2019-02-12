cmake_minimum_required(VERSION 3.13)
project(Tftp)

set(CMAKE_CXX_STANDARD 17)

link_libraries(ws2_32)
add_executable(tftp main.cpp)
add_executable(tftps server.cpp)
