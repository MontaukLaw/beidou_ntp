cmake_minimum_required(VERSION 3.8)
message(STATUS "cmake version ${CMAKE_VERSION}")
project("beidou_ntp_server")

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

add_executable(ntp_server ntp_server.c beidou.c)
target_link_libraries(ntp_server pthread)

