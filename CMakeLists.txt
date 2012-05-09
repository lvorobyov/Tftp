cmake_minimum_required(VERSION 3.12)
project(Tftp)

set(CMAKE_CXX_STANDARD 17)

include_directories($ENV{WORK}/include)

link_libraries(ws2_32)
add_executable(tftp main.cpp)
add_executable(tftps server.cpp receiver.cpp connection.cpp)
add_executable(tftp_gui WIN32 send_dialog.cpp send_dialog.h)
