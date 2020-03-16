cmake_minimum_required(VERSION 3.12)
project(Tftp)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(CsoiTEX)
include(CsoiWin)

set(CMAKE_CXX_STANDARD 17)

include_directories($ENV{WORK}/include)
add_definitions(-D_UNICODE -DUNICODE)

link_libraries(ws2_32)
add_executable(tftp main.cpp)
add_executable(tftps server.cpp receiver.cpp connection.cpp)
add_executable(tftp_gui WIN32 send_dialog.cpp send_dialog.h
        detector.cpp tftp.rc resource.h)
target_link_libraries(tftp_gui comctl32)

add_tex(report README.ad)